/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x7C000
  USER_STORE (rw) : ORIGIN = 0x7C000, LENGTH = 0x2000
  BOND_STORE (rw) : ORIGIN = 0x7E000, LENGTH = 0x2000
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x10000
  CODE_RAM (rwx) : ORIGIN = 0x800000, LENGTH = 0x10000
}


INCLUDE "nrf_common.ld"
