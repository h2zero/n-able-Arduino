/*
  Copyright (c) 2014-2015 Arduino LLC.  All right reserved.
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

#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include "nrf.h"

const uint32_t g_ADigitalPinMap[] = {
  // D0 - D7
  0,  // xtal 1
  1,  // xtal 2
  2,  // a0
  3,  // a1
  4,  // a2
  5,  // a3
  6,  // a4
  7,  // AREF

  // D8 - D13
  8,  // GPIO #8

  9,  // NFC1
  10, // NFC2

  11, // GPIO #11

  12, // GPIO #12
  13, // GPIO #13
  14, // GPIO #14

  15, // GPIO #15
  16, // GPIO #16

  // function set pins
  17,
  18, // TXD SWO
  19, // GPIO #19
  20, // GPIO #20
  21, // Reset
  (uint32_t)-1, // N/A
  (uint32_t)-1, // N/A
  (uint32_t)-1, // N/A

  25, // MISO
  26, // MOSI
  27, // SS
  28, // SCK
  29, // BUTTON 1 = DISC = DFU
  30, // LED #2 (blue - also a red)
  31, // LED #1 (red)
};

void initVariant()
{
  // LED1 & LED2
  pinMode(PIN_LED1, OUTPUT);
  ledOff(PIN_LED1);

  pinMode(PIN_LED2, OUTPUT);
  ledOff(PIN_LED2);
}