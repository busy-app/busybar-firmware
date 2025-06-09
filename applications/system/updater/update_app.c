#include "update_config.h"

#include <toolbox/update_lib/dfu_file.h>
#include <toolbox/update_lib/update_manifest.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/path.h>

#include <furi_hal_flash.h>
#include <furi_hal_power.h>
#include <furi.h>
#include <furi_hal_nvm.h>
#include <storage/storage.h>

#define TAG          "UpdaterApp"
#define MAX_PATH_LEN 256

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
    FURI_LOG_D(
        TAG,
        "Programming flash page %u at address 0x%08X with update block of length %u",
        i_page,
        (unsigned int)(furi_hal_flash_get_base() + furi_hal_flash_get_page_size() * i_page),
        update_block_len);
    furi_hal_flash_program_page(i_page, update_block, update_block_len);
    return true;
}

static bool pdate_task_compare_flash(
    const uint8_t i_page,
    const uint8_t* update_block,
    uint16_t update_block_len) {
    const size_t page_addr = furi_hal_flash_get_base() + furi_hal_flash_get_page_size() * i_page;
    FURI_LOG_D(
        TAG,
        "Comparing flash page %u at address 0x%08X with update block of length %u",
        i_page,
        (unsigned int)page_addr,
        update_block_len);
    return memcmp(update_block, (void*)page_addr, update_block_len) == 0;
}

static bool check_address_boundaries(const size_t address) {
    const size_t min_allowed_address = furi_hal_flash_get_base();
    const size_t max_allowed_address = (size_t)furi_hal_flash_get_free_end_address();
    FURI_LOG_D(
        TAG,
        "Checking address 0x%08X against boundaries: 0x%08X - 0x%08X",
        (unsigned int)address,
        (unsigned int)min_allowed_address,
        (unsigned int)max_allowed_address);
    return (address >= min_allowed_address) && (address < max_allowed_address);
}

static void update_task_file_progress(const uint8_t progress, void* context) {
    UNUSED(context);
    UNUSED(progress);
}

static bool updater_execute(const char* update_path) {
    FURI_LOG_I(TAG, "Update from: %s", update_path);

    bool overall_success = false;
    Storage* storage = NULL;
    UpdateConfig* state = NULL;
    File* dfu_file = NULL;

    DfuUpdateTask page_task = {
        .address_cb = &check_address_boundaries,
        .progress_cb = &update_task_file_progress,
        .task_cb = &update_task_flash_program_page,
        .context = NULL,
    };

    storage = furi_record_open(RECORD_STORAGE);
    do {
        state = update_config_alloc();

        UpdateConfigValidation validation_result = update_config_load(state, update_path);
        if(validation_result != UpdateConfigValidationOK) {
            FURI_LOG_E(
                TAG,
                "Config load failed: %s, Error: %s (%d)",
                update_path,
                update_config_validation_get_error_str(validation_result),
                validation_result);
            break;
        }

        const UpdateManifest* config = update_config_get_manifest(state);
        const FuriString* dfu_furi_string =
            updater_manifest_get_path(config, UpdateManifestPathDfu);

        if(dfu_furi_string && !furi_string_empty(dfu_furi_string)) {
            const char* dfu_path = furi_string_get_cstr(dfu_furi_string);
            FURI_LOG_I(TAG, "DFU path: '%s'", dfu_path);

            dfu_file = storage_file_alloc(storage);
            if(!dfu_file) {
                FURI_LOG_E(TAG, "DFU file alloc failed");
                break;
            }

            if(!storage_file_open(dfu_file, dfu_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
                FURI_LOG_E(TAG, "DFU open failed");
                break;
            }

            if(!dfu_file_validate_crc(dfu_file, NULL, NULL)) {
                FURI_LOG_E(TAG, "DFU CRC failed");
                break;
            }

            uint8_t n_targets = dfu_file_validate_headers(dfu_file, &bsb_dfu_params);
            FURI_LOG_I(TAG, "DFU targets: %u", n_targets);
            if(n_targets == 0) {
                FURI_LOG_E(TAG, "DFU header invalid");
                break;
            }

            // Program DFU
            page_task.task_cb = &update_task_flash_program_page;
            if(!dfu_file_process_targets(&page_task, dfu_file, n_targets)) {
                FURI_LOG_E(TAG, "DFU flash failed");
                break;
            }
            FURI_LOG_I(TAG, "DFU flash OK");

            // Verify DFU
            page_task.task_cb = &pdate_task_compare_flash;
            if(!dfu_file_process_targets(&page_task, dfu_file, n_targets)) {
                FURI_LOG_E(TAG, "DFU verify failed");
                break;
            }
            FURI_LOG_I(TAG, "DFU verify OK");
            overall_success = true;

        } else {
            FURI_LOG_I(TAG, "No DFU in manifest.");
            overall_success = true;
        }

    } while(false);

    if(dfu_file) {
        storage_file_free(dfu_file);
    }
    if(state) {
        update_config_free(state);
    }
    furi_record_close(RECORD_STORAGE);

    if(overall_success) {
        FURI_LOG_I(TAG, "Update exec OK.");
    } else {
        FURI_LOG_E(TAG, "Update exec FAILED.");
    }

    return overall_success;
}

int32_t updater_srv(void* arg) {
    UNUSED(arg);
    FURI_LOG_I(TAG, "Updater service started.");

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* update_manifest_path_fstr = furi_string_alloc();
    bool execution_succeeded = false;

    if(update_config_read_pointer_file(storage, update_manifest_path_fstr) &&
       !furi_string_empty(update_manifest_path_fstr)) {
        execution_succeeded = updater_execute(furi_string_get_cstr(update_manifest_path_fstr));
    } else {
        FURI_LOG_E(TAG, "Failed to get update path from pointer. Rebooting.");
    }

    FURI_LOG_I(TAG, execution_succeeded ? "Update OK" : "Update FAILED.");

    if(update_manifest_path_fstr) {
        furi_string_free(update_manifest_path_fstr);
    }

    furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeNormal); // Ensure NVM is set before reboot
    furi_record_close(RECORD_STORAGE);
    furi_delay_ms(50);
    furi_hal_power_reset();

    // This part should not be reached
    furi_thread_suspend(furi_thread_get_current_id());
    return execution_succeeded ? 0 : 1;
}
