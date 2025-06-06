#include "updater_core.h"
#include <toolbox/update_lib/dfu_file.h>
#include <toolbox/update_lib/update_util.h>
#include <furi_hal_flash.h>
#include <furi_hal_power.h>

// Make UpdaterState struct visible for this file
struct UpdaterState {
    Storage* storage;
    File* file;
    FuriString* update_folder;
    UpdaterConfig* config;
};

#include <furi.h>
#include <furi_hal_nvm.h>

#define TAG "UpdaterApp"

#define STM_DFU_VENDOR_ID   0x0483
#define STM_DFU_PRODUCT_ID  0xDF11
#define BSB_DFU_DEVICE_CODE 0xFFFF

static const DfuValidationParams bsb_dfu_params = {
    .device = BSB_DFU_DEVICE_CODE,
    .product = STM_DFU_PRODUCT_ID,
    .vendor = STM_DFU_VENDOR_ID,
};

static bool update_task_flash_program_page(
    const uint8_t i_page,
    const uint8_t* update_block,
    uint16_t update_block_len) {
    furi_hal_flash_program_page(i_page, update_block, update_block_len);
    return true;
}

static bool page_task_compare_flash(
    const uint8_t i_page,
    const uint8_t* update_block,
    uint16_t update_block_len) {
    const size_t page_addr = furi_hal_flash_get_base() + furi_hal_flash_get_page_size() * i_page;
    return memcmp(update_block, (void*)page_addr, update_block_len) == 0;
}

static bool check_address_boundaries(const size_t address) {
    const size_t min_allowed_address = furi_hal_flash_get_base();
    const size_t max_allowed_address = (size_t)furi_hal_flash_get_free_end_address();
    return (address >= min_allowed_address) && (address < max_allowed_address);
}

static void update_task_file_progress(const uint8_t progress, void* context) {
    UNUSED(context);
    UNUSED(progress);
}

static void updater_execute(const char* update_path) {
    static DfuUpdateTask page_task = {
        .address_cb = &check_address_boundaries,
        .progress_cb = &update_task_file_progress,
        .task_cb = &update_task_flash_program_page,
        .context = NULL,
    };

    UpdaterState* state = updater_state_alloc(update_path);

    bool config_ok = updater_load_configuration(state);
    FURI_LOG_I(TAG, "Updater config loaded: %s", config_ok ? "OK" : "FAIL");
    config_ok = updater_validate_config(state);
    FURI_LOG_I(TAG, "Updater config validated: %s", config_ok ? "OK" : "FAIL");

    // Flash DFU file if present in config
    bool dfu_flashed = false;
    if(state && state->config && state->config->updater_dfu &&
       !furi_string_empty(state->config->updater_dfu)) {
        const char* dfu_path = furi_string_get_cstr(state->config->updater_dfu);
        FURI_LOG_I(TAG, "Flashing DFU file: %s", dfu_path);
        File* dfu_file = storage_file_alloc(state->storage);
        if(storage_file_open(dfu_file, dfu_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            if(!dfu_file_validate_crc(dfu_file, NULL, NULL)) {
                FURI_LOG_E(TAG, "DFU CRC validation failed: %s", dfu_path);
            } else {
                uint8_t n_targets = dfu_file_validate_headers(dfu_file, &bsb_dfu_params);
                if(n_targets > 0) {
                    if(dfu_file_process_targets(&page_task, dfu_file, n_targets)) {
                        FURI_LOG_I(TAG, "DFU flashing succeeded: %s", dfu_path);
                        dfu_flashed = true;
                    } else {
                        FURI_LOG_E(TAG, "DFU flashing failed: %s", dfu_path);
                    }

                    page_task.task_cb = &page_task_compare_flash;
                    if(dfu_file_process_targets(&page_task, dfu_file, n_targets)) {
                        FURI_LOG_I(TAG, "DFU flash verification succeeded: %s", dfu_path);
                    } else {
                        FURI_LOG_E(TAG, "DFU flash verification failed: %s", dfu_path);
                        dfu_flashed = false;
                    }
                } else {
                    FURI_LOG_E(TAG, "DFU header validation failed: %s", dfu_path);
                }
            }
            storage_file_close(dfu_file);
        } else {
            FURI_LOG_E(TAG, "Failed to open DFU file: %s", dfu_path);
        }
        storage_file_free(dfu_file);
    }

    if(dfu_flashed) {
        // Restart system after successful update and verification
        furi_hal_power_reset();
    }

    updater_state_free(state);
}

int32_t updater_srv(void* arg) {
    UNUSED(arg);

    FURI_LOG_I(TAG, "Updater service started");

    // if(furi_hal_nvm_get_boot_mode() == FuriHalNvmBootModeNormal) {
    //     FURI_LOG_E(TAG, "Updater service in normal boot mode");
    //     furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeNormal);
    //     furi_thread_suspend(furi_thread_get_current_id());
    // }

    updater_execute("/ext/update");
    furi_thread_suspend(furi_thread_get_current_id());

    return 0;
}
