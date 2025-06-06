#include "furi_hal_flash.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <core/common_defines.h>
#include <furi.h>
#include <furi_hal_cortex.h>

#include "stm32u5xx.h"

#define FURI_HAL_FLASH_TOTAL_PAGES (2 * FLASH_PAGE_NB)
#define FURI_HAL_FLASH_BUSY_WAIT_TIMEOUT_US \
    (1000000) // 1 second, for waiting for BSY flag to clear before an operation
#define FURI_HAL_FLASH_PROGRAM_TIMEOUT_US \
    (1000000) // 1 second, for the programming operation itself
#define FURI_HAL_FLASH_ERASE_TIMEOUT_US (3000000) // 3 seconds, for the erase operation itself

static void furi_hal_flash_unlock() {
    furi_check(FLASH->NSCR & FLASH_NSCR_LOCK); // Ensure flash is locked before unlocking
    // Unlock non-secure flash
    FLASH->NSKEYR = 0x45670123U;
    FLASH->NSKEYR = 0xCDEF89ABU;
    furi_check(!(FLASH->NSCR & FLASH_NSCR_LOCK)); // Ensure flash is unlocked
}

static void furi_hal_flash_lock() {
    // Lock flash
    FLASH->NSCR |= FLASH_NSCR_LOCK;
    furi_check(FLASH->NSCR & FLASH_NSCR_LOCK); // Ensure flash is locked
}

size_t furi_hal_flash_get_page_size(void) {
    // STM32U5: FLASH_PAGE_SIZE is 8 KB (0x2000)
    return FLASH_PAGE_SIZE;
}

size_t furi_hal_flash_get_base(void) {
    // Always return the base of the first bank (single region abstraction)
    return FLASH_BASE_NS;
}

const void* furi_hal_flash_get_free_end_address(void) {
    // Abstract both banks as a single region
    return (void*)(FLASH_BASE_NS + (FLASH_PAGE_SIZE * FURI_HAL_FLASH_TOTAL_PAGES));
}

int16_t furi_hal_flash_get_page_number(size_t address) {
    // Abstract both banks as a single region
    if(address < FLASH_BASE_NS) return -1;
    size_t offset = address - FLASH_BASE_NS;
    if(offset >= FLASH_PAGE_SIZE * FURI_HAL_FLASH_TOTAL_PAGES) return -1;
    return offset / FLASH_PAGE_SIZE;
}

void furi_hal_flash_program_page(const uint8_t page, const uint8_t* data, uint16_t length) {
    // Abstract both banks as a single region: page index is global
    furi_check(
        page < FURI_HAL_FLASH_TOTAL_PAGES); // Ensure page number is valid (across both banks)
    furi_check(data != NULL); // Ensure data pointer is not NULL
    furi_check(length > 0 && length <= FLASH_PAGE_SIZE); // Ensure length is valid

    // Erase the page before programming.
    // furi_hal_flash_erase is expected to handle its own flash unlocking/locking
    // and leave the flash locked on completion.
    furi_hal_flash_erase(page);

    FURI_CRITICAL_ENTER();
    // Unlock non-secure flash
    furi_hal_flash_unlock();

    uint32_t page_addr = FLASH_BASE_NS + (page * FLASH_PAGE_SIZE);
    for(uint32_t offset = 0; offset < length; offset += 8) {
        uint64_t dword = 0xFFFFFFFFFFFFFFFFULL;
        memcpy(&dword, data + offset, (length - offset >= 8) ? 8 : (length - offset));
        // Wait for any previous operation to finish
        FuriHalCortexTimer timer = furi_hal_cortex_timer_get(FURI_HAL_FLASH_BUSY_WAIT_TIMEOUT_US);
        while(FLASH->NSSR & FLASH_NSSR_BSY) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                furi_crash("Flash busy timeout");
            }
        }
        // Set programming
        FLASH->NSCR |= FLASH_NSCR_PG;
        // Write double word
        *(volatile uint64_t*)(page_addr + offset) = dword;
        // Wait for completion
        timer = furi_hal_cortex_timer_get(FURI_HAL_FLASH_PROGRAM_TIMEOUT_US);
        while(FLASH->NSSR & FLASH_NSSR_BSY) {
            if(furi_hal_cortex_timer_is_expired(timer)) {
                furi_crash("Flash program timeout");
            }
        }
        // Check for errors (only standard bits)
        furi_check(
            (FLASH->NSSR & (FLASH_NSSR_WRPERR | FLASH_NSSR_PGSERR | FLASH_NSSR_OPERR)) == 0);
        // Clear programming
        FLASH->NSCR &= ~FLASH_NSCR_PG;
    }
    // Lock flash
    furi_hal_flash_lock();
    FURI_CRITICAL_EXIT();
}

// RM0456, 7.3.6 "Flash main memory erase sequences"
void furi_hal_flash_erase(const uint8_t page) {
    // Abstract both banks as a single region: page index is global
    furi_check(
        page < FURI_HAL_FLASH_TOTAL_PAGES); // Ensure page number is valid (across both banks)

    FURI_CRITICAL_ENTER();
    // Unlock non-secure flash
    furi_hal_flash_unlock();

    // Wait for any previous operation to finish
    FuriHalCortexTimer timer = furi_hal_cortex_timer_get(FURI_HAL_FLASH_BUSY_WAIT_TIMEOUT_US);
    while(FLASH->NSSR & FLASH_NSSR_BSY) {
        if(furi_hal_cortex_timer_is_expired(timer)) {
            furi_crash("Flash busy timeout before erase");
        }
    }

    // Set page erase and page number
    // For STM32U5, PER bit initiates page erase. PNB sets the page number.
    // FLASH_NSCR_PNB is 7 bits wide, FLASH_NSCR_BKER selects the bank.
    uint8_t bank = (page < FLASH_PAGE_NB) ? 0 : 1;
    uint8_t page_in_bank = (page < FLASH_PAGE_NB) ? page : page - FLASH_PAGE_NB;

    FLASH->NSCR &= ~(FLASH_NSCR_PNB | FLASH_NSCR_BKER); // Clear previous page and bank selection
    if(bank == 0) {
        FLASH->NSCR &= ~FLASH_NSCR_BKER; // Select bank 1 (if BKER=0)
    } else {
        FLASH->NSCR |= FLASH_NSCR_BKER; // Select bank 2 (if BKER=1)
    }
    FLASH->NSCR |= (page_in_bank << FLASH_NSCR_PNB_Pos) & FLASH_NSCR_PNB;
    FLASH->NSCR |= FLASH_NSCR_PER; // Page erase operation
    FLASH->NSCR |= FLASH_NSCR_STRT; // Start erase operation

    // Wait for completion
    timer = furi_hal_cortex_timer_get(FURI_HAL_FLASH_ERASE_TIMEOUT_US);
    while(FLASH->NSSR & FLASH_NSSR_BSY) {
        if(furi_hal_cortex_timer_is_expired(timer)) {
            furi_crash("Flash erase timeout");
        }
    }

    // Check for errors (only standard bits)
    furi_check((FLASH->NSSR & (FLASH_NSSR_WRPERR | FLASH_NSSR_PGSERR | FLASH_NSSR_OPERR)) == 0);

    // Clear PER bit
    FLASH->NSCR &= ~FLASH_NSCR_PER;

    // Lock flash
    furi_hal_flash_lock();
    FURI_CRITICAL_EXIT();
}
