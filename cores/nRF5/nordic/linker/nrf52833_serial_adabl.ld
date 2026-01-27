/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00001000, LENGTH = 0x74000 - 0x1000 - 0x4000
  USER_STORE (rw) : ORIGIN = 0x70000, LENGTH = 0x2000
  BOND_STORE (rw) : ORIGIN = 0x72000, LENGTH = 0x2000
  RAM (rwx) : ORIGIN = 0x20000008, LENGTH = 0x20000 - 0x8 - 0x100
}


INCLUDE "nrf_common.ld"
