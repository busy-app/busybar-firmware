#include "furi_hal_flash.h"

#include <system_si91x.h>
#include <si917_linker.h>

size_t furi_hal_flash_get_base(void) {
    return M4_FLASH_BASE;
}

const void* furi_hal_flash_get_free_end_address(void) {
    return &__free_flash_start__;
}
