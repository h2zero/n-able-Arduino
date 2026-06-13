#include "variant.h"
#include "wiring_constants.h"
#include "wiring_digital.h"
#include "nrf.h"

const uint32_t g_ADigitalPinMap[] =
{
    // D0 .. D13
    35, // D0  is P1.03 (TX)
    42, // D1  is P1.10 (RX)
    43, // D2  is P1.11
    44, // D3  is P1.12
    47, // D4  is P1.15
    45, // D5  is P1.13
    46, // D6  is P1.14
    23, // D7  is P0.23
    21, // D8  is P0.21
    27, // D9  is P0.27
    34, // D10 is P1.02 (SS)
    33, // D11 is P1.01 (MOSI)
    40, // D12 is P1.08 (MISO)
    13, // D13 is P0.13 (SCK / LED_BUILTIN)

    // A0 .. A7  (D14 .. D21)
     4, // D14 is P0.04 (A0)
     5, // D15 is P0.05 (A1)
    30, // D16 is P0.30 (A2)
    29, // D17 is P0.29 (A3)
    31, // D18 is P0.31 (A4 / SDA)
     2, // D19 is P0.02 (A5 / SCL)
    28, // D20 is P0.28 (A6)
     3, // D21 is P0.03 (A7)

    // LEDs  (D22 .. D25)
    24, // D22 is P0.24 (LED_RED)
    16, // D23 is P0.16 (LED_GREEN)
     6, // D24 is P0.06 (LED_BLUE)
    41, // D25 is P1.09 (LED_PWR)

    // Interrupt  (D26)
    19, // D26 is P0.19 (INT_APDS)

    // PDM microphone  (D27 .. D29)
    17, // D27 is P0.17 (PDM_PWR)
    26, // D28 is P0.26 (PDM_CLK)
    25, // D29 is P0.25 (PDM_DIN)

    // Internal I2C for sensors  (D30 .. D31)
    14, // D30 is P0.14 (Wire1 SDA)
    15, // D31 is P0.15 (Wire1 SCL)

    // Internal power control  (D32 .. D33)
    32, // D32 is P1.00 (I2C pull-up enable)
    22, // D33 is P0.22 (VDD_ENV / sensors 3V3 enable)
};

void initVariant()
{
    // Turn on the power LED
    pinMode(LED_PWR, OUTPUT);
    digitalWrite(LED_PWR, HIGH);

    // Enable 3.3V power rail and I2C pull-ups for the onboard sensors
    pinMode(PIN_ENABLE_SENSORS_3V3, OUTPUT);
    pinMode(PIN_ENABLE_I2C_PULLUP, OUTPUT);
    digitalWrite(PIN_ENABLE_SENSORS_3V3, HIGH);
    digitalWrite(PIN_ENABLE_I2C_PULLUP, HIGH);
}
