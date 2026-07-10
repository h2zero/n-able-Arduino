#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include "nrf.h"

const uint32_t g_ADigitalPinMap[] =
{
  // nice!nano v2 edge castellations
   8, // D0  is P0.08
   6, // D1  is P0.06
  17, // D2  is P0.17
  20, // D3  is P0.20
  22, // D4  is P0.22
  24, // D5  is P0.24
  32, // D6  is P1.00
  11, // D7  is P0.11
  36, // D8  is P1.04
  38, // D9  is P1.06
   9, // D10 is P0.09

  // Exposed bottom pads
  33, // D11 is P1.01
  34, // D12 is P1.02
  39, // D13 is P1.07

  // nice!nano v2 edge castellations
  43, // D14 is P1.11
  45, // D15 is P1.13
  10, // D16 is P0.10
  42, // D17 is P1.10
  47, // D18 is P1.15
   2, // D19 is P0.02 (AIN0)
  29, // D20 is P0.29 (AIN5)
  31, // D21 is P0.31 (AIN7)

  15, // D22 is P0.15 (status LED)
   4, // D23 is P0.04 (battery voltage)
  13, // D24 is P0.13 (VCC cutoff)
};

void initVariant()
{
  pinMode(PIN_LED, OUTPUT);
  ledOff(PIN_LED);
}
