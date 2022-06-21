/* Pins for nRF52833_DK */

#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include "nrf.h"

const uint32_t g_ADigitalPinMap[] = {
  // P0
  0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 ,
  8 , 9 , 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23,
  24, 25, 26, 27, 28, 29, 30, 31,

  // P1
  32, 33, 34, 35, 36, 37, 38, 39,
  40, 41,
};

void initVariant()
{
  // init all 4 onboard LEDs
  pinMode(PIN_LED1, OUTPUT);
  ledOff(PIN_LED1);

  pinMode(PIN_LED2, OUTPUT);
  ledOff(PIN_LED2);

  pinMode(PIN_LED3, OUTPUT);
  ledOff(PIN_LED3);

  pinMode(PIN_LED4, OUTPUT);
  ledOff(PIN_LED4);

  // init all 4 onboard buttons
  pinMode(PIN_BUTTON1, INPUT_PULLUP);

  pinMode(PIN_BUTTON2, INPUT_PULLUP);

  pinMode(PIN_BUTTON3, INPUT_PULLUP);

  pinMode(PIN_BUTTON4, INPUT_PULLUP);
}
