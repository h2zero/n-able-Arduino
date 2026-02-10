/*
  Copyright (c) 2021 Ryan Powell.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>
#include "nrf.h"
#include "compiler_abstraction.h"
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

struct exception_frame {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

// Retention RAM structure for storing fault data (survives soft reset)
// Placed below stack limit to avoid stack/heap collisions
#define FAULT_DATA_MAGIC 0xDEADBEEF
#define BACKTRACE_DEPTH 16
struct fault_data_t {
    uint32_t magic;      // Magic number to detect valid fault data
    uint32_t pc;
    uint32_t lr;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t psr;
    uint32_t cfsr;       // Configurable Fault Status Register
    uint32_t hfsr;       // Hard Fault Status Register
    uint32_t mmfar;      // MemManage Fault Address Register
    uint32_t bfar;       // BusFault Address Register
    uint32_t afsr;       // Auxiliary Fault Status Register
    uint32_t msp;        // Main Stack Pointer
    uint32_t psp;        // Process Stack Pointer
    uint32_t exc_return; // EXC_RETURN value (LR at exception)
    uint32_t backtrace[BACKTRACE_DEPTH]; // Stack backtrace
    uint8_t backtrace_len; // Number of valid backtrace entries
};
// Place fault data 256 bytes below stack limit for safety
#define FAULT_DATA_ADDR ((volatile fault_data_t*)((uint32_t)&__StackLimit - 0x200))

// Declare stack symbols from linker
extern uint32_t __StackTop;
extern uint32_t __StackLimit;
extern uint32_t __etext;

/**
 * @brief Checks if the instruction before 'LR' is 'BL' or 'BLX'
 * @param lr Link Register value at time of call
 * @param callee Address of the called function to verify against
 * @returns caller address or -1 if not valid
 */
static uint32_t find_caller(uint32_t lr, uint32_t callee) {
    lr -= 1; // Clear thumb bit
    if (lr < 4 || lr > (uint32_t)&__etext) {
        return -1;
    }

    lr -= 4;  // Now lr points to potential BL instruction
    uint16_t first_hw = *((uint16_t *)lr);      // Read 1st halfword from memory
    uint16_t second_hw = *((uint16_t *)(lr + 2)); // Read 2nd halfword from memory

    // Check for BLX (2-byte) instruction
    if ((second_hw & 0xFF80) == 0x4780) {
        return lr + 2; // BLX register (2-byte) is at lr+2
    }

    // Check for BL (4-byte) instruction
    if ((first_hw & 0xF800) == 0xF000 && (second_hw & 0xF800) == 0xF800) {
        // Extract offset and compute target address
        uint32_t S = (first_hw >> 10) & 0x1;
        uint32_t J1 = (second_hw >> 13) & 0x1;
        uint32_t J2 = (second_hw >> 11) & 0x1;
        uint32_t I1 = ~(J1 ^ S) & 0x1;
        uint32_t I2 = ~(J2 ^ S) & 0x1;
        int32_t imm10 = first_hw & 0x03FF;
        int32_t imm11 = second_hw & 0x07FF;
        int32_t offset = (S << 24) | (I1 << 23) | (I2 << 22) | (imm10 << 12) | (imm11 << 1);
        // Sign-extend if needed
        if (S) {
            offset |= 0xFE000000;
        }

        // verify if this BL targets the callee
        // Compute target: PC is at 'lr', so PC+4 is at lr+4
        uint32_t target = (lr + 4) + offset;

        // Relaxed validation: accept if target is close to callee (within 32 bytes)
        // This accounts for function alignment, prologue code, and multiple entry points
        int32_t diff = (int32_t)(target - callee);
        if (diff >= -32 && diff <= 32) {
            return lr;
        }
    }

    return -1; // Not a valid caller
}

/**
 * @brief Capture stack backtrace into fault_data_t structure
 * @param fault Pointer to fault_data_t structure to populate
 * @param sp Stack pointer at time of fault
 * @returns number of backtrace entries captured
 */
static size_t capture_backtrace(uint32_t* backtrace_buffer, size_t buffer_size, uint32_t sp, bool is_psp)
{
    uint32_t stack_low = (uint32_t)&__StackLimit;
    uint32_t stack_high = (uint32_t)&__StackTop;

#if ( configRECORD_STACK_HIGH_ADDRESS == 1 )
    // If PSP was active use current task stack bounds
    if (is_psp)
    {
        stack_low = ulGetCurrentStackLowAddress();
        stack_high = ulGetCurrentStackHighAddress();
    }
#endif

    if (sp < stack_low) {
        sp = stack_low;
    }

    size_t capture_count = 0;

    // Walk the stack looking for valid return addresses
    // Stack grows downward, so walk from current SP towards stack top
    for (;sp < stack_high && capture_count < buffer_size; sp += sizeof(size_t)) {
        uint32_t lr_value = *((uint32_t *) sp);
        // the Cortex-M using thumb instruction, so the LR must be an odd number
        if ((lr_value & 1) == 0) {
            continue;
        }

        // Use previous frame as expected callee for validation
        // When capture_count=0, look at entry before buffer (last pre-filled entry)
        // When capture_count>0, look at last captured entry
        uint32_t expected_callee = (capture_count > 0) ? backtrace_buffer[capture_count - 1] : backtrace_buffer[-1];
        uint32_t pc = find_caller(lr_value, expected_callee);
        if (pc == (uint32_t)-1) {
            continue;
        }

        backtrace_buffer[capture_count++] = pc;
    }

    return capture_count;
}

/**
 * @brief Store fault data into retention RAM
 * @param ef Pointer to exception frame on stack
 * @param exc_return_val EXC_RETURN value from LR at time of fault
 */
static void store_fault_to_ram(exception_frame* ef, uint32_t exc_return_val)
{
#if USB_CDC_DEFAULT_SERIAL
    volatile fault_data_t* fault = FAULT_DATA_ADDR;
    fault->magic = FAULT_DATA_MAGIC;
    fault->pc = ef->pc;
    fault->lr = ef->lr;
    fault->r0 = ef->r0;
    fault->r1 = ef->r1;
    fault->r2 = ef->r2;
    fault->r3 = ef->r3;
    fault->r12 = ef->r12;
    fault->psr = ef->psr;

    // Capture ARM Cortex-M fault status registers
    fault->cfsr = SCB->CFSR;   // Shows UsageFault, BusFault, MemManage details
    fault->hfsr = SCB->HFSR;   // Shows if fault escalated to HardFault
    fault->mmfar = SCB->MMFAR; // Address that caused MemManage fault (if valid)
    fault->bfar = SCB->BFAR;   // Address that caused BusFault (if valid)
    fault->afsr = SCB->AFSR;   // Auxiliary/implementation-defined info

    // Capture stack pointers
    __asm volatile ("mrs %0, msp" : "=r" (fault->msp));
    __asm volatile ("mrs %0, psp" : "=r" (fault->psp));
    fault->exc_return = exc_return_val;
    uint32_t sp = (exc_return_val & 0x4) ? fault->psp : fault->msp;
    // skip R0~R3, R12, LR,PC,xPSR registers space
    sp += sizeof(size_t) * 8;
#if defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)
    // Check if FPU context was saved (bit 4 of EXC_RETURN)
    if (!((exc_return_val >> 4) & 0x1)) {
        // skip extra saved FPU registers S0~S15, FPSCR, reserved
        sp += sizeof(size_t) * 18;
    }
#endif

    // Was the stack pointer adjusted to ensure 8-byte alignment when the exception was taken?
    // In that case, bit 9 of stacked xPSR/RETPSR is set.
    if (fault->psr & (1UL << 9)) {
        sp += 4;
    }

    uint32_t pc = fault->pc;
    fault->backtrace[0] = pc;
    fault->backtrace_len = 1;
    pc = find_caller(fault->lr, pc);

    if (pc != (uint32_t)-1) {
        fault->backtrace[1] = pc;
        fault->backtrace_len = 2;
    }

    // Continue capturing from the stack, starting after existing entries
    size_t additional = capture_backtrace((uint32_t*)&fault->backtrace[fault->backtrace_len],
                                          BACKTRACE_DEPTH - fault->backtrace_len,
                                          sp,
                                          (exc_return_val & 0x4));
    fault->backtrace_len = fault->backtrace_len + additional;
#endif
}

// Helper function to safely print exception data
static void print_exception_data(exception_frame* ef)
{
// If USB is used for Serial we must avoid using it in HardFault handler
#if !USB_CDC_DEFAULT_SERIAL
    if (Serial)
    {
        Serial.println("\n======== HARD FAULT DETECTED ========");

        // Exception frame registers
        Serial.printf("PC:  0x%08lx\n", ef->pc);
        Serial.printf("LR:  0x%08lx\n", ef->lr);
        Serial.printf("R0:  0x%08lx  R1: 0x%08lx\n", ef->r0, ef->r1);
        Serial.printf("R2:  0x%08lx  R3: 0x%08lx\n", ef->r2, ef->r3);
        Serial.printf("R12: 0x%08lx\n", ef->r12);
        Serial.printf("PSR: 0x%08lx\n", ef->psr);

        // Stack pointers
        uint32_t msp, psp;
        __asm volatile ("mrs %0, msp" : "=r" (msp));
        __asm volatile ("mrs %0, psp" : "=r" (psp));
        Serial.printf("MSP: 0x%08lx  PSP: 0x%08lx\n", msp, psp);

#if (__CORTEX_M >= 0x03U)
        // Fault status registers
        Serial.println("\nFault Status:");
        Serial.printf("HFSR:  0x%08lx\n", SCB->HFSR);
        if (SCB->HFSR & 0x40000000) Serial.println("  - Forced (escalated from configurable fault)");
        if (SCB->HFSR & 0x80000000) Serial.println("  - Debug event");
        if (SCB->HFSR & 0x00000002) Serial.println("  - Vector table read fault");

        Serial.printf("CFSR:  0x%08lx\n", SCB->CFSR);
        Serial.printf("  UFSR: 0x%04x  BFSR: 0x%02x  MMFSR: 0x%02x\n",
                     (uint16_t)(SCB->CFSR >> 16),
                     (uint8_t)(SCB->CFSR >> 8),
                     (uint8_t)SCB->CFSR);

        // MemManage faults
        if (SCB->CFSR & 0x0080) { // MMARVALID
            Serial.printf("MMFAR: 0x%08lx (MemManage fault address)\n", SCB->MMFAR);
        }
        if (SCB->CFSR & 0x01) Serial.println("  - Instruction access violation");
        if (SCB->CFSR & 0x02) Serial.println("  - Data access violation");
        if (SCB->CFSR & 0x08) Serial.println("  - MemManage fault on unstacking");
        if (SCB->CFSR & 0x10) Serial.println("  - MemManage fault on stacking");
        if (SCB->CFSR & 0x20) Serial.println("  - MemManage fault on FP lazy state");

        // BusFaults
        if (SCB->CFSR & 0x8000) { // BFARVALID
            Serial.printf("BFAR:  0x%08lx (BusFault address)\n", SCB->BFAR);
        }
        if (SCB->CFSR & 0x0100) Serial.println("  - Bus fault on instruction fetch");
        if (SCB->CFSR & 0x0200) Serial.println("  - Precise data bus error");
        if (SCB->CFSR & 0x0400) Serial.println("  - Imprecise data bus error");
        if (SCB->CFSR & 0x0800) Serial.println("  - Bus fault on unstacking");
        if (SCB->CFSR & 0x1000) Serial.println("  - Bus fault on stacking");
        if (SCB->CFSR & 0x2000) Serial.println("  - Bus fault on FP lazy state");

        // UsageFaults
        if (SCB->CFSR & 0x010000) Serial.println("  - Undefined instruction");
        if (SCB->CFSR & 0x020000) Serial.println("  - Invalid state (EPSR)");
        if (SCB->CFSR & 0x040000) Serial.println("  - Invalid PC on exception return");
        if (SCB->CFSR & 0x080000) Serial.println("  - No coprocessor");
        if (SCB->CFSR & 0x100000) Serial.println("  - Unaligned access");
        if (SCB->CFSR & 0x200000) Serial.println("  - Divide by zero");

        Serial.printf("SHCSR: 0x%08lx\n", SCB->SHCSR);
        if (SCB->AFSR) {
            Serial.printf("AFSR:  0x%08lx (Auxiliary fault info)\n", SCB->AFSR);
        }
#endif

        dump_call_stack();
    }
#endif
}

/**
 * @brief Function to check for and report saved fault data.
 * @details Serial.prints fault information from previous crash if any.
 * Can only be used when using USB for serial logging output.
 */
void check_and_report_fault()
{
#if USB_CDC_DEFAULT_SERIAL
    volatile fault_data_t* fault = FAULT_DATA_ADDR;

    if (fault->magic == FAULT_DATA_MAGIC)
    {
        // Valid fault data found
        Serial.println("\n======== HARD FAULT DETECTED ========");

        // Exception frame
        Serial.printf("PC:  0x%08lx  LR:  0x%08lx\n", fault->pc, fault->lr);
        Serial.printf("PSR: 0x%08lx  SP:  0x%08lx\n", fault->psr,
                      (fault->exc_return & 0x4) ? fault->psp : fault->msp);
        Serial.printf("R0:  0x%08lx  R1:  0x%08lx\n", fault->r0, fault->r1);
        Serial.printf("R2:  0x%08lx  R3:  0x%08lx\n", fault->r2, fault->r3);
        Serial.printf("R12: 0x%08lx\n", fault->r12);

        // Fault status registers
        Serial.println("\nFault Status Registers:");
        Serial.printf("CFSR:  0x%08lx\n", fault->cfsr);
        Serial.printf("  UFSR: 0x%04x  BFSR: 0x%02x  MMFSR: 0x%02x\n",
                      (uint16_t)(fault->cfsr >> 16),
                      (uint8_t)(fault->cfsr >> 8),
                      (uint8_t)fault->cfsr);
        Serial.printf("HFSR:  0x%08lx\n", fault->hfsr);

        // Decode CFSR bits
        Serial.println("\nFault Analysis:");
        if (fault->cfsr & 0x0080) { // MMARVALID
            Serial.printf("MemManage fault at: 0x%08lx\n", fault->mmfar);
        }
        if (fault->cfsr & 0x8000) { // BFARVALID
            Serial.printf("BusFault at: 0x%08lx\n", fault->bfar);
        }
        if (fault->cfsr & 0x02) Serial.println("- Data access violation");
        if (fault->cfsr & 0x01) Serial.println("- Instruction access violation");
        if (fault->cfsr & 0x10) Serial.println("- Stacking error");
        if (fault->cfsr & 0x08) Serial.println("- Unstacking error");
        if (fault->cfsr & 0x0100) Serial.println("- Bus fault on instruction fetch");
        if (fault->cfsr & 0x0400) Serial.println("- Bus fault on stacking");
        if (fault->cfsr & 0x0800) Serial.println("- Bus fault on unstacking");
        if (fault->cfsr & 0x1000) Serial.println("- Imprecise bus fault");
        if (fault->cfsr & 0x2000) Serial.println("- Precise bus fault");
        if (fault->cfsr & 0x10000) Serial.println("- Undefined instruction");
        if (fault->cfsr & 0x20000) Serial.println("- Invalid state");
        if (fault->cfsr & 0x40000) Serial.println("- Invalid PC");
        if (fault->cfsr & 0x80000) Serial.println("- No coprocessor");
        if (fault->cfsr & 0x1000000) Serial.println("- Unaligned access");
        if (fault->cfsr & 0x2000000) Serial.println("- Divide by zero");
        if (fault->hfsr & 0x40000000) Serial.println("- Forced HardFault (escalated)");
        if (fault->hfsr & 0x80000000) Serial.println("- Debug event");

        // Print backtrace
        Serial.println("\nCall Stack Backtrace:");
        for (uint8_t i = 0; i < fault->backtrace_len; i++) {
            Serial.printf("  #%d: 0x%08lx\n", i, fault->backtrace[i]);
        }

        Serial.println("======================================\n");

        // Clear the fault data
        fault->magic = 0;
    }
#endif
}

/**
 * @brief Dump current call stack from application code.
 * @details Uses current SP and LR to capture and print backtrace to Serial.
 */
void dump_call_stack()
{
    // Get current stack pointer and link register
    uint32_t sp, lr;
    __asm volatile ("mov %0, sp" : "=r" (sp));
    __asm volatile ("mov %0, lr" : "=r" (lr));

    // backtrace buffer
    uint32_t backtrace[BACKTRACE_DEPTH];
    uint8_t depth = 0;

    // Add first frame from LR
    auto pc = find_caller(lr, reinterpret_cast<uint32_t>(dump_call_stack));
    if (pc == (uint32_t)-1) {
        Serial.println("Unable to find caller from LR");
        return;
    }

    backtrace[depth++] = pc;

    // Check if we're using PSP (task context)
    uint32_t control;
    __asm volatile ("mrs %0, control" : "=r" (control));
    bool is_psp = (control & 0x2) != 0;

    // Capture remaining frames from stack
    depth += capture_backtrace(&backtrace[depth],
                               BACKTRACE_DEPTH - depth,
                               sp,
                               is_psp);

    // Print backtrace
    Serial.println("\n======== CALL STACK ========\n");
    if (depth > 0) {
        Serial.println("Call Stack Backtrace:");
        for (uint8_t i = 0; i < depth; i++) {
            // Ensure thread-safe printing so the monitor filter can find and decode it the address.
            char buffer[19];
            snprintf(buffer, sizeof(buffer), "  #%d: 0x%08lx", i, backtrace[i]);
            Serial.println(buffer);
        }
    }

    Serial.println("============================\n");
}

extern "C" {
__attribute__ ((__weak__))
void Hardfault_handler_cpp( uint32_t *p_stack_address, uint32_t lr_value )
{
    exception_frame* ef = (exception_frame*)p_stack_address;

    // Store fault data to retention RAM (survives reset)
    store_fault_to_ram(ef, lr_value);

    print_exception_data(ef);

    NVIC_SystemReset();
    while(1) {}
}

__attribute__ ((__weak__))
void __assert_func(const char *file, int line, const char *func, const char *e)
{
    Serial.printf("\n\n\n"); // 3 newlines needed due as tinyusb will not print any without it?
    Serial.printf("=== ASSERTION FAILED ===\n");
    Serial.printf("  Expression: %s\n", e);
    Serial.printf("  File:       %s\n", file);
    Serial.printf("  Line:       %d\n", line);
    Serial.printf("  Function:   %s\n", func);
    Serial.printf("========================\n");
    dump_call_stack();
    delay(50);
    NVIC_SystemReset();
    while (1){}
}

__attribute__ ((__weak__))
void __cxa_pure_virtual(void) {
    Serial.println("Pure virtual function called");
    dump_call_stack();
    delay(100);
    NVIC_SystemReset();
    while(1) {}
}

__attribute__ ((__weak__))
void __cxa_deleted_virtual(void) {
    Serial.println("Deleted virtual function called");
    dump_call_stack();
    delay(100);
    NVIC_SystemReset();
    while(1) {}
}

#if __CORTEX_M == 0x00
__attribute__(( naked ))
void HardFault_Handler(void)
{
    __asm volatile
    (
    "   .syntax unified                        \n"

    "   mov   r1, lr                           \n"  // Save LR (EXC_RETURN) to r1
    "   ldr   r0, =0xFFFFFFFD                  \n"
    "   cmp   r0, lr                           \n"
    "   bne   _MSP                             \n"
    "   mrs   r0, PSP                          \n"
    "   b     _Done                            \n"
    "_MSP:                                     \n"
    "   mrs   r0, MSP                          \n"
    "_Done:                                    \n"
    "   ldr r3, =Hardfault_handler_cpp         \n"
    "   bx r3                                  \n"

    "   .align                                 \n"
    );
}
#endif

#if __CORTEX_M == 0x04
__attribute__(( naked ))
void HardFault_Handler(void)
{
    __asm volatile
    (
        " mov r1, lr                           \n"  // Save LR (EXC_RETURN) to r1
        " tst lr, #4                           \n"
        " ite eq                               \n"
        " mrseq r0, msp                        \n"
        " mrsne r0, psp                        \n"
        " b Hardfault_handler_cpp              \n"
    );
}
#endif

} // extern "C"
