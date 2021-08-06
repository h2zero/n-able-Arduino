/*
  Copyright (c) 2015 Arduino LLC.  All right reserved.
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

/*
 * Modified work Copyright (c) 2021 Ryan Powell.  All right reserved.
 */

#define ARDUINO_MAIN
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"

#ifdef CONFIG_MAIN_TASK_STACK_SIZE
#define MAIN_TASK_STACK_SIZE CONFIG_MAIN_TASK_STACK_SIZE
#else
#define MAIN_TASK_STACK_SIZE 1024
#endif

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

static TaskHandle_t _loopTaskHandle = NULL;
static StackType_t _mainStack[ (MAIN_TASK_STACK_SIZE / 4) ];
static StaticTask_t _mainTaskBuffer;

void loopTask(void *pvParameters)
{
/*
    if(get_reset_reason() == 0xdeadbeef) {
        Serial.begin(115200);
        Serial.printf("Chip was reset by watchdog!\n");
    }
*/
    setup();

    for(;;) {
        loop();
        yield();
        if (serialEventRun) serialEventRun();
    }
}

/*
 * \brief Main entry point of Arduino application
 */
int main( void )
{
  init();

  initVariant();

  #ifdef USE_TINYUSB
  Adafruit_TinyUSB_Core_init();
  #endif

  _loopTaskHandle = xTaskCreateStatic(loopTask, "mlt", (MAIN_TASK_STACK_SIZE / 4),
                                     NULL, 1, _mainStack, &_mainTaskBuffer);

  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  vTaskStartScheduler();

  NVIC_SystemReset();

  return 0;
}
