/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.
  Copyright (c) 2016 Sandeep Mistry All right reserved.
  Copyright (c) 2016 Frank Holtz. All right reserved.

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

#ifndef _VARIANT_PCA1000X_
#define _VARIANT_PCA1000X_

/** Master clock frequency */
#define VARIANT_MCK       (16000000)

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define PINS_COUNT           (31)
#define NUM_DIGITAL_PINS     (31)
#define NUM_ANALOG_INPUTS    (6)
#define NUM_ANALOG_OUTPUTS   (0)

// Analog pins
#define PIN_AREF             (0)
#define PIN_A0				       (1)
#define PIN_A1				       (2)
#define PIN_A2				       (3)
#define PIN_A3				       (4)
#define PIN_A4				       (5)
#define PIN_A5				       (6)
#define PIN_A6               (0xff)
#define PIN_A7               (0xff)
#define ADC_RESOLUTION 14

// LEDs
#define PIN_LED1             (21)
#define PIN_LED2             (22)
#define PIN_LED3             (23)
#define PIN_LED4             (24)
#define LED_BUILTIN          PIN_LED1
#define LED_STATE_ON         (0)

// Buttons
#define PIN_BUTTON1          (17)
#define PIN_BUTTON2          (18)
#define PIN_BUTTON3          (19)
#define PIN_BUTTON4          (20)

/*
  * Serial interfaces
  */
#define PIN_SERIAL_RX        (11)
#define PIN_SERIAL_CTS       (10)
#define PIN_SERIAL_TX        (9)
#define PIN_SERIAL_RTS       (8)

// SPI
#define SPI_INTERFACES_COUNT 1

#define PIN_SPI_MISO		     (28)
#define PIN_SPI_MOSI		     (29)
#define PIN_SPI_SCK			     (25)

/*
 * Wire Interfaces
 */
#define WIRE_INTERFACES_COUNT 1

#define PIN_WIRE_SDA         (7)
#define PIN_WIRE_SCL         (30)


#ifdef __cplusplus
}
#endif

#endif
