/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00001000, LENGTH = 0x6F000
  USER_STORE (rw) : ORIGIN = 0x70000, LENGTH = 0x2000
  BOND_STORE (rw) : ORIGIN = 0x72000, LENGTH = 0x2000
  RAM (rwx) : ORIGIN = 0x20000008, LENGTH = 0x1FFF8
}


INCLUDE "nrf_common.ld"
