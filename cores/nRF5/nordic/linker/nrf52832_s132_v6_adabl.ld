/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x26000, LENGTH = 0x74000 - 0x26000 - 0x4000
  USER_STORE (rw) : ORIGIN = 0x74000 - 0x4000, LENGTH = 0x2000
  BOND_STORE (rw) : ORIGIN = 0x74000 - 0x2000, LENGTH = 0x2000
  RAM (rwx) :  ORIGIN = 0x20003600, LENGTH = 0x10000 - 0x3600
}