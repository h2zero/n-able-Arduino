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
struct __attribute__((packed)) fault_data_t {
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

extern "C" {

// Declare stack symbols from linker
extern uint32_t __StackLimit;

#if ( configRECORD_STACK_HIGH_ADDRESS == 1 )
// Minimal TCB mirror to access stack bounds for current task
typedef struct tskTaskControlBlock
{
    volatile StackType_t * pxTopOfStack;
#if ( portUSING_MPU_WRAPPERS == 1 )
    xMPU_SETTINGS xMPUSettings;
#endif
    ListItem_t xStateListItem;
    ListItem_t xEventListItem;
    UBaseType_t uxPriority;
    StackType_t * pxStack; // stack start
    char pcTaskName[ configMAX_TASK_NAME_LEN ];
#if ( ( portSTACK_GROWTH > 0 ) || ( configRECORD_STACK_HIGH_ADDRESS == 1 ) )
    StackType_t * pxEndOfStack; // stack end (high address for downward growth)
#endif
} TCB_t;

extern TCB_t * volatile pxCurrentTCB;
#endif

// Place fault data 256 bytes below stack limit for safety
#define FAULT_DATA_ADDR ((volatile fault_data_t*)((uint32_t)&__StackLimit - 0x200))

// Code regions (derived from hardware registers / device memory map)
static inline uint32_t flash_end_address(void)
{
    // CODESIZE * CODEPAGESIZE gives total flash size in bytes
    return NRF_FICR->CODEPAGESIZE * NRF_FICR->CODESIZE;
}

#define CODE_RAM_START 0x00800000u
#define CODE_RAM_SIZE  0x00040000u // 256 KB code RAM window on nRF52
#define CODE_RAM_END   (CODE_RAM_START + CODE_RAM_SIZE)

// Check if address is in valid code region (FLASH or CODE_RAM)
static inline bool is_valid_code_address(uint32_t addr)
{
    uint32_t flash_end = flash_end_address();
    bool in_flash = (addr < flash_end);
    bool in_code_ram = (addr >= CODE_RAM_START) && (addr < CODE_RAM_END);
    return in_flash || in_code_ram;
}

// Unwind the stack to capture backtrace
static void capture_backtrace(volatile fault_data_t* fault, uint32_t sp_value)
{
    uint32_t* sp = (uint32_t*)sp_value;
    extern uint32_t __StackTop;
    uint32_t stack_low = (uint32_t)&__StackLimit;
    uint32_t stack_high = (uint32_t)&__StackTop;

#if ( configRECORD_STACK_HIGH_ADDRESS == 1 )
    // If PSP was active (bit 2 of EXC_RETURN = 1), use current task stack bounds
    if (fault->exc_return & 0x4)
    {
        if (pxCurrentTCB != NULL)
        {
            stack_low = (uint32_t)pxCurrentTCB->pxStack;
            stack_high = (uint32_t)pxCurrentTCB->pxEndOfStack;
        }
    }
#else
    // Without pxEndOfStack, fall back to full RAM range to avoid early exit
    if (fault->exc_return & 0x4)
    {
        stack_low = 0x20000000u;
        stack_high = 0x20040000u; // 256 KB RAM max on nRF52
    }
#endif

    fault->backtrace_len = 0;

    // Add PC and LR as first entries
    if (fault->backtrace_len < BACKTRACE_DEPTH) {
        fault->backtrace[fault->backtrace_len] = fault->pc;
        fault->backtrace_len = fault->backtrace_len + 1; // avoid volatile warning
    }
    if (fault->backtrace_len < BACKTRACE_DEPTH && is_valid_code_address(fault->lr)) {
        fault->backtrace[fault->backtrace_len] = fault->lr & ~1; // Clear thumb bit
        fault->backtrace_len = fault->backtrace_len + 1; // avoid volatile warning
    }

    // Walk the stack looking for valid return addresses
    // Stack grows downward, so walk from current SP towards stack top
    for (uint32_t i = 0; i < 256 && fault->backtrace_len < BACKTRACE_DEPTH; i++) {
        // Bounds check
        if ((uint32_t)sp < stack_low || (uint32_t)sp >= stack_high) {
            break;
        }

        uint32_t value = *sp;

        // Check if this looks like a return address (in code region, thumb bit set)
        if ((value & 0x1) && is_valid_code_address(value & ~1)) {
            uint32_t addr = value & ~1; // Clear thumb bit for display

            // Avoid duplicates
            bool duplicate = false;
            for (uint8_t j = 0; j < fault->backtrace_len; j++) {
                if (fault->backtrace[j] == addr) {
                    duplicate = true;
                    break;
                }
            }

            if (!duplicate) {
                fault->backtrace[fault->backtrace_len] = addr;
                fault->backtrace_len = fault->backtrace_len + 1; // avoid volatile warning
            }
        }

        sp++;
    }
}

// Store exception data to retention RAM
static void store_fault_to_ram(exception_frame* ef, uint32_t exc_return_val)
{
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

    // Capture stack backtrace
    uint32_t sp = (exc_return_val & 0x4) ? fault->psp : fault->msp;
    capture_backtrace(fault, sp);
}

// Helper function to safely print exception data
static void print_exception_data(exception_frame* ef)
{
#ifndef USE_TINYUSB
    char buffer[128];

    if (Serial)
    {
        Serial.println("\n=== HARD FAULT ===");
        snprintf(buffer, sizeof(buffer), "PC:  0x%08lx\n", ef->pc);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "LR:  0x%08lx\n", ef->lr);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "R0:  0x%08lx  R1: 0x%08lx\n", ef->r0, ef->r1);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "R2:  0x%08lx  R3: 0x%08lx\n", ef->r2, ef->r3);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "R12: 0x%08lx\n", ef->r12);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "PSR: 0x%08lx\n", ef->psr);
        Serial.print(buffer);
        Serial.println("=== END FAULT ===\n");

        // Wait briefly for UART to flush
        delay(10);
    }
#endif
}

__attribute__ ((__weak__))
void Hardfault_handler_cpp( uint32_t *p_stack_address, uint32_t lr_value )
{
    exception_frame* ef = (exception_frame*)p_stack_address;

    // Store fault data to retention RAM (survives reset)
    store_fault_to_ram(ef, lr_value);

    print_exception_data(ef);

    NVIC_SystemReset();
    while(1) {} // Ensure we don't continue executing
}

__attribute__ ((__weak__))
void __assert_func(const char *file, int line, const char *func, const char *e)
{
    // Serial.printf("Assertion Failed: %s at line %d , in function: %s\n", file, line, func);
    yield();
    NVIC_SystemReset();
    while (1){} // silence compiler
}

__attribute__ ((__weak__))
void __cxa_pure_virtual(void) {
    // Serial.println("Pure virtual function called");
    NVIC_SystemReset();
    while(1) {}
}

__attribute__ ((__weak__))
void __cxa_deleted_virtual(void) {
    // Serial.println("Deleted virtual function called");
    NVIC_SystemReset();
    while(1) {}
}

// Function to check for and report saved fault data
// Call this from setup() to retrieve fault information from previous crash
//__attribute__ ((__weak__))
void check_and_report_fault(void)
{
    volatile fault_data_t* fault = FAULT_DATA_ADDR;

    if (fault->magic == FAULT_DATA_MAGIC)
    {
        // Valid fault data found
        Serial.println("\n======== HARD FAULT DETECTED ========");
        char buffer[80];

        // Exception frame
        snprintf(buffer, sizeof(buffer), "PC:  0x%08lx  LR:  0x%08lx\n", fault->pc, fault->lr);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "PSR: 0x%08lx  SP:  0x%08lx\n", fault->psr,
                 (fault->exc_return & 0x4) ? fault->psp : fault->msp);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "R0:  0x%08lx  R1:  0x%08lx\n", fault->r0, fault->r1);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "R2:  0x%08lx  R3:  0x%08lx\n", fault->r2, fault->r3);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "R12: 0x%08lx\n", fault->r12);
        Serial.print(buffer);

        // Fault status registers
        Serial.println("\nFault Status Registers:");
        snprintf(buffer, sizeof(buffer), "CFSR:  0x%08lx\n", fault->cfsr);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "  UFSR: 0x%04x  BFSR: 0x%02x  MMFSR: 0x%02x\n",
                 (uint16_t)(fault->cfsr >> 16),
                 (uint8_t)(fault->cfsr >> 8),
                 (uint8_t)fault->cfsr);
        Serial.print(buffer);
        snprintf(buffer, sizeof(buffer), "HFSR:  0x%08lx\n", fault->hfsr);
        Serial.print(buffer);

        // Decode CFSR bits
        Serial.println("\nFault Analysis:");
        if (fault->cfsr & 0x0080) { // MMARVALID
            snprintf(buffer, sizeof(buffer), "MemManage fault at: 0x%08lx\n", fault->mmfar);
            Serial.print(buffer);
        }
        if (fault->cfsr & 0x8000) { // BFARVALID
            snprintf(buffer, sizeof(buffer), "BusFault at: 0x%08lx\n", fault->bfar);
            Serial.print(buffer);
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
        if (fault->backtrace_len > 0) {
            Serial.println("\nCall Stack Backtrace:");
            for (uint8_t i = 0; i < fault->backtrace_len; i++) {
                snprintf(buffer, sizeof(buffer), "  #%d: 0x%08lx\n", i, fault->backtrace[i]);
                Serial.print(buffer);
            }
            Serial.println("\nTo resolve addresses, use:");
            Serial.println("  addr2line -e firmware.elf -a -f -C <address>");
            Serial.println("If no line is shown (??:?), try <address>-1:");
            Serial.println("  addr2line -e firmware.elf -a -f -C <address_minus_1>");
        }

        Serial.println("======================================\n");

        // Clear the fault data
        fault->magic = 0;
    }
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
