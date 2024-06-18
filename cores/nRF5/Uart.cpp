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

#include "Uart.h"
#include "Arduino.h"
#include "wiring_private.h"

// these nRF52s have different UART PSEL definition
#if defined(NRF52810_XXAA) || defined(NRF52805_XXAA) \
  || defined(NRF52811_XXAA) || defined(NRF52833_XXAA) \
  || defined(NRF52840_XXAA)
#define PSELTXD  PSEL.TXD
#define PSELRXD  PSEL.RXD
#define PSELCTS  PSEL.CTS
#define PSELRTS  PSEL.RTS
#endif

void serialEventRun(void)
{
  if (serialEvent && Serial.available() ) serialEvent();

#if defined(PIN_SERIAL1_RX) && defined(PIN_SERIAL1_TX)
  if (serialEvent1 && Serial1.available() ) serialEvent1();
#endif
}

Uart::Uart(NRF_UART_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX)
{
  nrfUart = _nrfUart;
  IRQn = _IRQn;
  uc_pinRX = g_ADigitalPinMap[_pinRX];
  uc_pinTX = g_ADigitalPinMap[_pinTX];
  uc_hwFlow = 0;
}

Uart::Uart(NRF_UART_Type *_nrfUart, IRQn_Type _IRQn, uint8_t _pinRX, uint8_t _pinTX, uint8_t _pinCTS, uint8_t _pinRTS)
{
  nrfUart = _nrfUart;
  IRQn = _IRQn;
  uc_pinRX = g_ADigitalPinMap[_pinRX];
  uc_pinTX = g_ADigitalPinMap[_pinTX];
  uc_pinCTS = g_ADigitalPinMap[_pinCTS];
  uc_pinRTS = g_ADigitalPinMap[_pinRTS];
  uc_hwFlow = 1;
}

void Uart::setPins(uint8_t _pinRX, uint8_t _pinTX)
{
  uc_pinRX = g_ADigitalPinMap[_pinRX];
  uc_pinTX = g_ADigitalPinMap[_pinTX];
}

void Uart::setPins(uint8_t _pinRX, uint8_t _pinTX, uint8_t _pinCTS, uint8_t _pinRTS)
{
  uc_pinRX = g_ADigitalPinMap[_pinRX];
  uc_pinTX = g_ADigitalPinMap[_pinTX];
  uc_pinCTS = g_ADigitalPinMap[_pinCTS];
  uc_pinRTS = g_ADigitalPinMap[_pinRTS];
}

void Uart::begin(unsigned long baudrate)
{
  begin(baudrate, (uint8_t)SERIAL_8N1);
}

void Uart::begin(unsigned long baudrate, uint16_t /*config*/)
{
  nrfUart->PSELTXD = uc_pinTX;
  nrfUart->PSELRXD = uc_pinRX;

  if (uc_hwFlow == 1) {
    nrfUart->PSELCTS = uc_pinCTS;
    nrfUart->PSELRTS = uc_pinRTS;
    nrfUart->CONFIG = (UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos) | UART_CONFIG_HWFC_Enabled;
  } else {
    nrfUart->CONFIG = (UART_CONFIG_PARITY_Excluded << UART_CONFIG_PARITY_Pos) | UART_CONFIG_HWFC_Disabled;
  }


  uint32_t nrfBaudRate;

#if defined(NRF52_SERIES)
  if (baudrate <= 1200) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud1200;
  } else if (baudrate <= 2400) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud2400;
  } else if (baudrate <= 4800) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud4800;
  } else if (baudrate <= 9600) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud9600;
  } else if (baudrate <= 14400) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud14400;
  } else if (baudrate <= 19200) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud19200;
  } else if (baudrate <= 28800) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud28800;
  } else if (baudrate <= 38400) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud38400;
  } else if (baudrate <= 57600) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud57600;
  } else if (baudrate <= 76800) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud76800;
  } else if (baudrate <= 115200) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud115200;
  } else if (baudrate <= 230400) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud230400;
  } else if (baudrate <= 250000) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud250000;
  } else if (baudrate <= 460800) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud460800;
  } else if (baudrate <= 921600) {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud921600;
  } else {
    nrfBaudRate = UARTE_BAUDRATE_BAUDRATE_Baud1M;
  }
#else
  if (baudrate <= 1200) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud1200;
  } else if (baudrate <= 2400) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud2400;
  } else if (baudrate <= 4800) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud4800;
  } else if (baudrate <= 9600) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud9600;
  } else if (baudrate <= 14400) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud14400;
  } else if (baudrate <= 19200) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud19200;
  } else if (baudrate <= 28800) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud28800;
  } else if (baudrate <= 38400) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud38400;
  } else if (baudrate <= 57600) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud57600;
  } else if (baudrate <= 76800) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud76800;
  } else if (baudrate <= 115200) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud115200;
  } else if (baudrate <= 230400) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud230400;
  } else if (baudrate <= 250000) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud250000;
  } else if (baudrate <= 460800) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud460800;
  } else if (baudrate <= 921600) {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud921600;
  } else {
    nrfBaudRate = UART_BAUDRATE_BAUDRATE_Baud1M;
  }
#endif

  nrfUart->BAUDRATE = nrfBaudRate;

  nrfUart->ENABLE = UART_ENABLE_ENABLE_Enabled;

  nrfUart->EVENTS_RXDRDY = 0x0UL;
  nrfUart->EVENTS_TXDRDY = 0x0UL;

  nrfUart->TASKS_STARTRX = 0x1UL;
  nrfUart->TASKS_STARTTX = 0x1UL;

  nrfUart->INTENSET = UART_INTENSET_RXDRDY_Msk;

  NVIC_ClearPendingIRQ(IRQn);
  NVIC_SetPriority(IRQn, 3);
  NVIC_EnableIRQ(IRQn);
}

void Uart::end()
{
  NVIC_DisableIRQ(IRQn);

  nrfUart->INTENCLR = UART_INTENCLR_RXDRDY_Msk;

  nrfUart->TASKS_STOPRX = 0x1UL;
  nrfUart->TASKS_STOPTX = 0x1UL;

  nrfUart->ENABLE = UART_ENABLE_ENABLE_Disabled;

  nrfUart->PSELTXD = 0xFFFFFFFF;
  nrfUart->PSELRXD = 0xFFFFFFFF;

  nrfUart->PSELRTS = 0xFFFFFFFF;
  nrfUart->PSELCTS = 0xFFFFFFFF;

  rxBuffer.clear();
}

void Uart::flush()
{
}

void Uart::IrqHandler()
{
  if (nrfUart->EVENTS_RXDRDY)
  {
    rxBuffer.store_char(nrfUart->RXD);

    nrfUart->EVENTS_RXDRDY = 0x0UL;
  }
}

int Uart::available()
{
  return rxBuffer.available();
}

int Uart::peek()
{
  return rxBuffer.peek();
}

int Uart::read()
{
  return rxBuffer.read_char();
}

size_t Uart::write(const uint8_t data)
{
  if (!nrfUart->ENABLE) {
    return 0;
  }

  nrfUart->TXD = data;

  while(!nrfUart->EVENTS_TXDRDY);

  nrfUart->EVENTS_TXDRDY = 0x0UL;

  return 1;
}

#if defined(NRF52_SERIES)
  #define NRF_UART0_IRQn UARTE0_UART0_IRQn
#elif defined(NRF51_SERIES)
  #define NRF_UART0_IRQn UART0_IRQn
#endif

#if !defined(USB_CDC_DEFAULT_SERIAL)
#  if defined(PIN_SERIAL_CTS) && defined(PIN_SERIAL_RTS)
      Uart Serial( NRF_UART0, NRF_UART0_IRQn, PIN_SERIAL_RX, PIN_SERIAL_TX, PIN_SERIAL_CTS, PIN_SERIAL_RTS );
#  else
      Uart Serial( NRF_UART0, NRF_UART0_IRQn, PIN_SERIAL_RX, PIN_SERIAL_TX );
#  endif
#endif

#if defined(PIN_SERIAL1_RX) && defined(PIN_SERIAL1_TX)
#  if defined(PIN_SERIAL1_CTS) && defined(PIN_SERIAL1_RTS)
      Uart Serial1( NRF_UART0, NRF_UART0_IRQn, PIN_SERIAL1_RX, PIN_SERIAL1_TX, PIN_SERIAL1_CTS, PIN_SERIAL1_RTS );
#  else
      Uart Serial1( NRF_UART0, NRF_UART0_IRQn, PIN_SERIAL1_RX, PIN_SERIAL1_TX );
#  endif
#endif

extern "C"
{
#if defined(NRF52_SERIES)
  void UARTE0_UART0_IRQHandler()
#elif defined(NRF51_SERIES)
  void UART0_IRQHandler()
#endif
  {
    SERIAL_PORT_HARDWARE.IrqHandler();
  }
}
