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

extern "C" {

__attribute__ ((__weak__))
void Hardfault_handler_cpp( uint32_t *p_stack_address )
{
    exception_frame* ef = (exception_frame*)p_stack_address;

    Serial.printf("Unhandled exception 0x%08x ", SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk);
    Serial.printf(", exception sp 0x%08x\n", (uint32_t)p_stack_address);
    Serial.printf("R0: 0x%08x, , R1: 0x%08x , R2: 0x%08x, R3: 0x%08x, R12: 0x%08x\n", ef->r0, ef->r1, ef->r2, ef->r3, ef->r12);
    Serial.printf("LR: 0x%08x, PC: 0x%08x, PSR: 0x%08x\n", ef->lr, ef->pc, ef->psr);

    NVIC_SystemReset();
}

__attribute__ ((__weak__))
void __assert_func(const char *file, int line, const char *func, const char *e)
{
    Serial.printf("Assertion Failed: %s at line %d , in function: %s", file, line, func);
    yield();
    NVIC_SystemReset();
    while (1){} // silence compiler
}

__attribute__ ((__weak__))
void __cxa_pure_virtual(void) {
    Serial.println("Pure virtual function called");
    NVIC_SystemReset();
}

__attribute__ ((__weak__))
void __cxa_deleted_virtual(void) {
    Serial.println("Deleted virtual function called");
    NVIC_SystemReset();
}

#if __CORTEX_M == 0x00
__attribute__(( naked ))
void HardFault_Handler(void)
{
    __asm volatile
    (
    "   .syntax unified                        \n"

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
        " tst lr, #4                           \n"
        " ite eq                               \n"
        " mrseq r0, msp                        \n"
        " mrsne r0, psp                        \n"
        " b Hardfault_handler_cpp              \n"
    );
}
#endif

} // extern "C"
