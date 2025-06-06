#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "stm32u5xx.h"
#include <furi/core/common_defines.h>

uint32_t furi_hal_flash_get_page_size(void) {
    // STM32U5: FLASH_PAGE_SIZE is 8 KB (0x2000)
    return FLASH_PAGE_SIZE;
}

uint32_t furi_hal_flash_get_base(void) {
    return FLASH_BASE_NS;
}

size_t furi_hal_flash_get_free_end_address(void) {
    // End of flash: base + (page size * number of pages)
    return FLASH_BASE_NS + (FLASH_PAGE_SIZE * FLASH_PAGE_NB);
}

int16_t furi_hal_flash_get_page_number(size_t address) {
    // STM32U5: FLASH_BASE_NS is 0x08000000UL, page size is 8 KB
    if(address < FLASH_BASE_NS) return -1;
    size_t offset = address - FLASH_BASE_NS;
    if(offset >= FLASH_PAGE_SIZE * FLASH_PAGE_NB) return -1;
    return offset / FLASH_PAGE_SIZE;
}

void furi_hal_flash_program_page(const uint8_t page, const uint8_t* data, uint16_t length) {
    FURI_CRITICAL_ENTER();
    // Unlock non-secure flash
    FLASH->NSKEYR = 0x45670123U;
    FLASH->NSKEYR = 0xCDEF89ABU;

    uint32_t page_addr = FLASH_BASE_NS + (page * FLASH_PAGE_SIZE);
    for(uint32_t offset = 0; offset < length; offset += 8) {
        uint64_t dword = 0xFFFFFFFFFFFFFFFFULL;
        memcpy(&dword, data + offset, (length - offset >= 8) ? 8 : (length - offset));
        // Wait for any previous operation to finish
        while(FLASH->NSSR & FLASH_NSSR_BSY) {
        }
        // Set programming
        FLASH->NSCR |= FLASH_NSCR_PG;
        // Write double word
        *(volatile uint64_t*)(page_addr + offset) = dword;
        // Wait for completion
        while(FLASH->NSSR & FLASH_NSSR_BSY) {
        }
        // Clear programming
        FLASH->NSCR &= ~FLASH_NSCR_PG;
    }
    // Lock flash
    FLASH->NSCR |= FLASH_NSCR_LOCK;
    FURI_CRITICAL_EXIT();
}
