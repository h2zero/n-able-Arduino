#if defined(NRF51)
#include "nrf51_nimconfig.h"
#elif defined(NRF52810_XXAA)
#include "nrf52810_nimconfig.h"
#elif defined(NRF52832_XXAA) || defined(NRF52832_XXAB)
#include "nrf52832_nimconfig.h"
#elif defined(NRF52833_XXAA)
#include "nrf52833_nimconfig.h"
#elif defined(NRF52840_XXAA)
#include "nrf52840_nimconfig.h"
#else
#error No supported mcu config specified
#endif

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif