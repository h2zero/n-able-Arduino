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
#include <nrfx_clock.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_WDT_TIMEOUT_SECONDS
#define CONFIG_WDT_TIMEOUT_SECONDS 5
#endif

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

#if !defined(USE_LFXO) && !defined(USE_LFRC) && !defined(USE_LFSYNT)
#warning Low frequency clock source not defined - using RC
#endif

static uint32_t _resetReason;
static uint8_t hw_clock_hfxo_refcnt;

static void hw_clock_evt_handler(nrfx_clock_evt_type_t event) {
    switch(event){
#if defined(USE_LFRC)
        case NRFX_CLOCK_EVT_CTTO:
            hw_clock_hfxo_request();
            if (nrfx_clock_calibration_start() != NRFX_SUCCESS) {
                nrfx_clock_calibration_timer_start(16);
            }
            break;
        case NRFX_CLOCK_EVT_CAL_DONE:
            hw_clock_hfxo_release();
            // Calibrate every 4 seconds
            nrfx_clock_calibration_timer_start(16);
            break;
#endif
        default:
            break;
    }
}

void init( void )
{
    _resetReason = NRF_POWER->RESETREAS;
    NRF_POWER->RESETREAS |= NRF_POWER->RESETREAS;

    nrfx_clock_init(hw_clock_evt_handler);
    nrfx_clock_enable();
#if defined(USE_LFSYNT)
    // LFSYNT requires the HF XTAL to always be running.
    hw_clock_hfxo_request();
#endif
    nrfx_clock_start(NRF_CLOCK_DOMAIN_LFCLK);
    while(!nrfx_clock_is_running(NRF_CLOCK_DOMAIN_LFCLK, NULL)){}

#if defined(USE_LFRC)
    nrfx_clock_calibration_timer_start(0);
#endif

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

    // Set vendor IRQ's to default priority level
    for (unsigned i = 0; i < NUM_IRQS; i++) {
        NVIC_SetPriority((IRQn_Type) i, DEFAULT_IRQ_PRIO);
    }

#if CONFIG_WDT_TIMEOUT_SECONDS
    //Configure Watchdog. Pause watchdog while the CPU is halted by the debugger or sleeping.
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Pause << WDT_CONFIG_SLEEP_Pos);
	NRF_WDT->CRV = CONFIG_WDT_TIMEOUT_SECONDS*32768;
	NRF_WDT->RREN |= WDT_RREN_RR0_Msk;
	NRF_WDT->TASKS_START = 1;  //Start the Watchdog timer
#endif
}

uint32_t getResetReason(void) {
    return _resetReason;
}

void systemPowerOff(void) {
    nrf_power_system_off(NRF_POWER);
}

void systemRestart(void) {
    NVIC_SystemReset();
}

__attribute__ ((__weak__))
void enterSerialDfu(void) {
    NRF_POWER->GPREGRET = DFU_MAGIC_SERIAL_ONLY_RESET;
    NVIC_SystemReset();
}

int hw_clock_hfxo_request(void) {
    int started = 0;
    nrf_clock_hfclk_t clk_src;

    portENTER_CRITICAL();
    assert(hw_clock_hfxo_refcnt < 0xff);
    if (hw_clock_hfxo_refcnt == 0) {
        if (!nrfx_clock_is_running(NRF_CLOCK_DOMAIN_HFCLK, &clk_src) ||
           (clk_src != NRF_CLOCK_HFCLK_HIGH_ACCURACY)) {
            nrfx_clock_start(NRF_CLOCK_DOMAIN_HFCLK);
            while (!nrfx_clock_is_running(NRF_CLOCK_DOMAIN_HFCLK, &clk_src) ||
                (clk_src != NRF_CLOCK_HFCLK_HIGH_ACCURACY)){}
        }
        started = 1;
    }
    ++hw_clock_hfxo_refcnt;
    portEXIT_CRITICAL();

    return started;
}

int hw_clock_hfxo_release(void) {
    int stopped = 0;

    portENTER_CRITICAL();
    assert(hw_clock_hfxo_refcnt != 0);
    --hw_clock_hfxo_refcnt;
    if (hw_clock_hfxo_refcnt == 0) {
        nrfx_clock_stop(NRF_CLOCK_DOMAIN_HFCLK);
        stopped = 1;
    }
    portEXIT_CRITICAL();

    return stopped;
}

#ifdef __cplusplus
}
#endif
