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

#include "nrf.h"

#include "Arduino.h"

#ifdef __cplusplus
extern "C" {
#endif

void pinMode( uint32_t ulPin, uint32_t ulMode )
{
  if (ulPin >= PINS_COUNT) {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  // Set pin mode according to chapter '22.6.3 I/O Pin Configuration'
  switch ( ulMode )
  {
    case INPUT:
      // Set pin to input mode
      nrf_gpio_cfg_input(ulPin, NRF_GPIO_PIN_NOPULL);
    break ;

    case INPUT_PULLUP:
      // Set pin to input mode with pull-up resistor enabled
      nrf_gpio_cfg_input(ulPin, NRF_GPIO_PIN_PULLUP);
    break ;

    case INPUT_PULLDOWN:
      // Set pin to input mode with pull-down resistor enabled
      nrf_gpio_cfg_input(ulPin, NRF_GPIO_PIN_PULLDOWN);
    break ;

    case OUTPUT:
      // Set pin to output mode
      nrf_gpio_cfg_output(ulPin);
    break ;

    default:
      // do nothing
    break ;
  }
}

void digitalWrite( uint32_t ulPin, uint32_t ulVal )
{
  if (ulPin >= PINS_COUNT) {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  if (ulVal == LOW) {
      nrf_gpio_pin_clear(ulPin);
  } else {
      nrf_gpio_pin_set(ulPin);
  }

  return ;
}

int digitalRead( uint32_t ulPin )
{
  if (ulPin >= PINS_COUNT) {
    return 0;
  }

  ulPin = g_ADigitalPinMap[ulPin];

  if (nrf_gpio_pin_dir_get(ulPin) == NRF_GPIO_PIN_DIR_INPUT) {
    return nrf_gpio_pin_read(ulPin);
  } else {
    return nrf_gpio_pin_out_read(ulPin);
  }
}

void digitalToggle( uint32_t ulPin )
{
  if (ulPin >= PINS_COUNT) {
    return;
  }

  ulPin = g_ADigitalPinMap[ulPin];
  nrf_gpio_pin_toggle(ulPin);
}

#ifdef LED_STATE_ON
void ledOn(uint32_t pin)
{
  digitalWrite(pin, LED_STATE_ON);
}

void ledOff(uint32_t pin)
{
  digitalWrite(pin, 1-LED_STATE_ON);
}
#endif


#ifdef __cplusplus
}
#endif
