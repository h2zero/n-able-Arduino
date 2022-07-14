/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x3F000
  USER_STORE (rw) : ORIGIN = 0x3F000, LENGTH = 0x800
  BOND_STORE (rw) : ORIGIN = 0x3F800, LENGTH = 0x800
  RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x4000
}


INCLUDE "nrf_common.ld"
