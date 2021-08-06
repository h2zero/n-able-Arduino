/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.

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

#include "Arduino.h"
#include <nrf.h>
#include <stdio.h>

#define DFU_MAGIC_SERIAL_ONLY_RESET   0x4e

#ifdef NRF52_SERIES
    #define DEFAULT_IRQ_PRIO      (2U)

    #if defined(NRF52805_XXAA)
        #define NUM_IRQS          (26U)
    #elif defined(NRF52810_XXAA)
        #define NUM_IRQS          (30U)
    #elif defined(NRF52811_XXAA)
        #define NUM_IRQS          (30U)
    #elif defined(NRF52820_XXAA)
        #define NUM_IRQS          (40U)
    #elif defined(NRF52832_XXAA)
        #define NUM_IRQS          (39U)
    #elif defined(NRF52833_XXAA)
        #define NUM_IRQS          (48U)
    #elif defined(NRF52840_XXAA)
        #define NUM_IRQS          (48U)
    #endif
#elif defined NRF51
    #define DEFAULT_IRQ_PRIO      (1U)
    #define NUM_IRQS              (26U)
#endif


#ifdef __cplusplus
extern "C" {
#endif

static uint32_t _resetReason;

void init( void )
{
  #if defined(USE_LFXO)
    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
  #elif defined(USE_LFSYNT)
    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_Synth << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
  #else //USE_LFRC
    NRF_CLOCK->LFCLKSRC = (uint32_t)((CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos) & CLOCK_LFCLKSRC_SRC_Msk);
  #endif
  NRF_CLOCK->TASKS_LFCLKSTART = 1UL;

  #if defined(RESET_PIN)
  if (((NRF_UICR->PSELRESET[0] & UICR_PSELRESET_CONNECT_Msk) != (UICR_PSELRESET_CONNECT_Connected << UICR_PSELRESET_CONNECT_Pos)) ||
      ((NRF_UICR->PSELRESET[1] & UICR_PSELRESET_CONNECT_Msk) != (UICR_PSELRESET_CONNECT_Connected << UICR_PSELRESET_CONNECT_Pos))){
      NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
      while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
      NRF_UICR->PSELRESET[0] = RESET_PIN;
      while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
      NRF_UICR->PSELRESET[1] = RESET_PIN;
      while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
      NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
      while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
      NVIC_SystemReset();
  }
  #endif

    if(NRF_POWER->RESETREAS & (POWER_RESETREAS_DOG_Detected << POWER_RESETREAS_DOG_Pos))
    {
        _resetReason = 0xdeadbeef;
        NRF_POWER->RESETREAS = 0xffffffff;
    }

    // Set vendor IRQ's to default priority level
    for (unsigned i = 0; i < NUM_IRQS; i++) {
        NVIC_SetPriority((IRQn_Type) i, DEFAULT_IRQ_PRIO);
    }

     //Configure Watchdog. Pause watchdog while the CPU is halted by the debugger or sleeping.
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = 5*32768; //5 second timeout
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;
	NRF_WDT->TASKS_START = 1;  //Start the Watchdog timer
}

uint32_t get_reset_reason(void) {
    return _resetReason;
}

void systemPowerOff(void) {
    nrf_power_system_off(NRF_POWER);
}

__attribute__ ((__weak__))
void enterSerialDfu(void) {
  NRF_POWER->GPREGRET = DFU_MAGIC_SERIAL_ONLY_RESET;
  NVIC_SystemReset();
}

#ifdef __cplusplus
}
#endif
