#ifdef NRF5

#include "nvm_store.h"
#include "nrf.h"
#include "nrfx_nvmc.h"

#if defined(USE_ADA_BL)
#  define BOOTLOADER_PAGE_SIZE 12
#elif defined(USE_NORDIC_BL)
#  define BOOTLOADER_PAGE_SIZE 32
#else
#  define BOOTLOADER_PAGE_SIZE 0
#endif

#define FLASH_WAIT_READY while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

inline void flash_page_erase(uint32_t address)
{
  nrfx_nvmc_page_erase(address);
}


inline void flash_write_word(uint32_t address, uint32_t value)
{
  nrfx_nvmc_word_write(address, value);
}

inline void flash_write_byte(uint32_t address, uint8_t value)
{
  nrfx_nvmc_byte_write(address, value);
}

inline void flash_write_bytes(uint32_t address, const uint8_t * src, uint32_t num_bytes)
{
  nrfx_nvmc_bytes_write(address, src, num_bytes);
}

inline void flash_write_words(uint32_t address, const uint32_t * src, uint32_t num_words)
{
    nrfx_nvmc_words_write(address, src, num_words);
}

inline uint32_t flash_read_unaligned_32(uint32_t address) {
    uint32_t ua32 = 0;
    ua32 = *(uint8_t*)address;
    ua32 |= *(uint8_t*)(address + 1) << 8;
    ua32 |= *(uint8_t*)(address + 2) << 16;
    ua32 |= *(uint8_t*)(address + 3) << 24;

    return ua32;
}

inline uint32_t flash_get_store_page(uint32_t bytes) {
    if (bytes < NRF_FICR->CODEPAGESIZE * 2) {
        bytes = NRF_FICR->CODEPAGESIZE * 2;
    } else if (bytes % NRF_FICR->CODEPAGESIZE != 0) {
        bytes += NRF_FICR->CODEPAGESIZE;
    }

    return (uint32_t)(NRF_FICR->CODEPAGESIZE * (NRF_FICR->CODESIZE - 2 - BOOTLOADER_PAGE_SIZE - (bytes / NRF_FICR->CODEPAGESIZE)));
}

inline uint32_t flash_get_bond_store_page() {
    return (uint32_t)(NRF_FICR->CODEPAGESIZE * (NRF_FICR->CODESIZE - 2 - BOOTLOADER_PAGE_SIZE));
}

inline uint32_t flash_get_page_size(void) {
    return (uint32_t)NRF_FICR->CODEPAGESIZE;
}

#endif /* NRF5 */
