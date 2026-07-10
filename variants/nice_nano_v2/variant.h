#ifndef _VARIANT_NICE_NANO_V2_
#define _VARIANT_NICE_NANO_V2_

#define TARGET_NICE_NANO_V2

/** Master clock frequency */
#define VARIANT_MCK       (64000000ul)

#define USE_LFXO

/*----------------------------------------------------------------------------
 *        Headers
 *----------------------------------------------------------------------------*/

#include "WVariant.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define PINS_COUNT              (25)
#define NUM_DIGITAL_PINS        (25)
#define NUM_ANALOG_INPUTS       (4)
#define NUM_ANALOG_OUTPUTS      (0)

// LEDs
#define PIN_LED                 (22)
#define LED_BUILTIN             PIN_LED
#define LED_BLUE                PIN_LED
#define LED_STATE_ON            (1)

// Power control
#define PIN_VCC_CUTOFF          (24)

// Buttons
#define PIN_BUTTON1             (PINS_COUNT)

// Digital pins, matching the nice!nano v2 pinout labels.
static const uint8_t D0  = 0;
static const uint8_t D1  = 1;
static const uint8_t D2  = 2;
static const uint8_t D3  = 3;
static const uint8_t D4  = 4;
static const uint8_t D5  = 5;
static const uint8_t D6  = 6;
static const uint8_t D7  = 7;
static const uint8_t D8  = 8;
static const uint8_t D9  = 9;
static const uint8_t D10 = 10;
static const uint8_t D11 = 11;
static const uint8_t D12 = 12;
static const uint8_t D13 = 13;
static const uint8_t D14 = 14;
static const uint8_t D15 = 15;
static const uint8_t D16 = 16;
static const uint8_t D17 = 17;
static const uint8_t D18 = 18;
static const uint8_t D19 = 19;
static const uint8_t D20 = 20;
static const uint8_t D21 = 21;

// Analog pins
#define PIN_VBAT                (23)    // P0.04/AIN2, battery voltage
#define PIN_A0                  (D19)   // P0.02/AIN0
#define PIN_A1                  (D20)   // P0.29/AIN5
#define PIN_A2                  (D21)   // P0.31/AIN7
#define PIN_A3                  (PIN_VBAT)

static const uint8_t A0 = PIN_A0;
static const uint8_t A1 = PIN_A1;
static const uint8_t A2 = PIN_A2;
static const uint8_t A3 = PIN_A3;

#define ADC_RESOLUTION          (14)

// Serial interfaces
#ifndef USB_CDC_DEFAULT_SERIAL
  #define USB_CDC_DEFAULT_SERIAL (1)
#endif

#if USB_CDC_DEFAULT_SERIAL
  #define PIN_SERIAL1_RX         (D0)
  #define PIN_SERIAL1_TX         (D1)
#else
  #define PIN_SERIAL_RX          (D0)
  #define PIN_SERIAL_TX          (D1)
#endif

// SPI interfaces
#define SPI_INTERFACES_COUNT    (1)

#define PIN_SPI_MISO            (D14)
#define PIN_SPI_MOSI            (D16)
#define PIN_SPI_SCK             (D15)

static const uint8_t SS   = D10;
static const uint8_t MOSI = PIN_SPI_MOSI;
static const uint8_t MISO = PIN_SPI_MISO;
static const uint8_t SCK  = PIN_SPI_SCK;

// Wire interfaces
#define WIRE_INTERFACES_COUNT   (1)

#define PIN_WIRE_SDA            (D2)
#define PIN_WIRE_SCL            (D3)

static const uint8_t SDA = PIN_WIRE_SDA;
static const uint8_t SCL = PIN_WIRE_SCL;

#ifdef __cplusplus
}
#endif

#endif
