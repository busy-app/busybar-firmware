#include "furi_hal_nvm.h"

#include <furi.h>
#include <furi_hal_debug.h>
#include <furi_hal_bus.h>

#include <stm32u5xx_ll_pwr.h>
#include <stm32u5_linker.h>

#define TAG "FuriHalNvm"

#define NVM_MAGIC   0xB00B0005
#define NVM_VERSION 0x00000001

typedef struct {
    uint32_t magic;
    uint32_t version;
} NvmHeader;

typedef struct {
    NvmHeader header;
    uint32_t flags;
    FuriHalNvmBootMode boot_mode;
    uint32_t fault_data;
    uint32_t switch_pos;
} NvmData;

_Static_assert(FuriHalNvmFlagCount <= 32, "Too many NVM flags defined!");

static volatile NvmData* nvm_storage = (NvmData*)(&__bkp_start__);

#pragma GCC push_options
#pragma GCC optimize("O0")
void furi_hal_nvm_reset(void) {
    memset((void*)nvm_storage, 0, sizeof(NvmData));

    // Set magic and version
    nvm_storage->header.magic = NVM_MAGIC;
    nvm_storage->header.version = NVM_VERSION;

    // Clear flags and boot mode
    nvm_storage->flags = 0;
    nvm_storage->boot_mode = FuriHalNvmBootModeNormal;
}
#pragma GCC pop_options

void furi_hal_nvm_set_flag(FuriHalNvmFlag flag) {
    nvm_storage->flags |= (1 << flag);
    furi_assert(furi_hal_nvm_is_flag_set(flag));
}

void furi_hal_nvm_reset_flag(FuriHalNvmFlag flag) {
    nvm_storage->flags &= ~(1 << flag);
    furi_assert(!furi_hal_nvm_is_flag_set(flag));
}

bool furi_hal_nvm_is_flag_set(FuriHalNvmFlag flag) {
    return (nvm_storage->flags & (1 << flag)) != 0;
}

FuriHalNvmBootMode furi_hal_nvm_get_boot_mode(void) {
    return nvm_storage->boot_mode;
}

void furi_hal_nvm_set_boot_mode(FuriHalNvmBootMode mode) {
    nvm_storage->boot_mode = mode;
}

void furi_hal_nvm_init_early(void) {
    furi_hal_bus_enable(FuriHalBusPWR);
    // Start 32KHz xtal oscillator
    LL_PWR_EnableBkUpAccess();
    while(LL_PWR_IsEnabledBkUpAccess() == 0U) {
    }

    if(nvm_storage->header.magic != NVM_MAGIC || nvm_storage->header.version != NVM_VERSION) {
        furi_hal_nvm_reset();
    }

    if(furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug)) {
        furi_hal_debug_enable();
        furi_log_set_level(FuriLogLevelDebug);
    } else {
        furi_hal_debug_disable();
        furi_log_set_level(FuriLogLevelDefault);
    }
}

void furi_hal_nvm_init(void) {
    FURI_LOG_I(TAG, "Init OK");
}

void furi_hal_nvm_set_fault_data(uint32_t value) {
    nvm_storage->fault_data = value;
}

void furi_hal_nvm_store_switch_pos(uint32_t pos) {
    nvm_storage->switch_pos = pos;
}

uint32_t furi_hal_nvm_get_switch_pos(void) {
    return nvm_storage->switch_pos;
}
