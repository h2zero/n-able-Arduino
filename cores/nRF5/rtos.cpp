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

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    Serial.printf("Task: %s, stack overflow\n", pcTaskName);
    NVIC_SystemReset();
}

void vApplicationMallocFailedHook(void)
{
    Serial.println("Malloc failed");
    NVIC_SystemReset();
}

void vApplicationIdleHook(void) {
    NRF_WDT->RR[0] = WDT_RR_RR_Reload;
#ifdef USE_TINYUSB
extern uint32_t tud_cdc_write_flush (void);
    tud_cdc_write_flush();
#endif
}

}