#include "updater_core.h"

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
#define MAX_PATH_LEN 256 // Using a fixed size as FURI_STRING_UTF8_MAX_LENGTH was problematic

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

static bool page_task_compare_flash(
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

static void updater_execute(const char* update_path) {
    static DfuUpdateTask page_task = {
        .address_cb = &check_address_boundaries,
        .progress_cb = &update_task_file_progress,
        .task_cb = &update_task_flash_program_page,
        .context = NULL,
    };

    FURI_LOG_I(TAG, "Executing update from path: %s", update_path);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    UpdaterState* state = updater_state_alloc();

    bool config_ok = updater_state_init_config(state, update_path);
    if(!config_ok) {
        FURI_LOG_E(TAG, "Failed to initialize updater config from path: %s", update_path);
        updater_state_free(state);
        furi_record_close(RECORD_STORAGE);
        return;
    }

    config_ok = updater_state_validate_config(state);
    FURI_LOG_I(TAG, "Updater config validated: %s", config_ok ? "OK" : "FAIL");
    if(!config_ok) {
        FURI_LOG_E(TAG, "Updater config validation failed for path: %s", update_path);
        updater_state_free(state);
        furi_record_close(RECORD_STORAGE);
        return; // Early exit if config is invalid
    }

    // Flash DFU file if present in config
    bool dfu_flashed = false;
    const UpdateManifest* config = updater_state_get_config(state);
    // Corrected: use updater_manifest_get_path and UpdateManifestPathDfu from update_manifest.h
    const FuriString* dfu_furi_string = updater_manifest_get_path(config, UpdateManifestPathDfu);
    if(state && config && dfu_furi_string && !furi_string_empty(dfu_furi_string)) {
        const char* dfu_path = furi_string_get_cstr(dfu_furi_string);
        FURI_LOG_I(TAG, "Flashing DFU file: %s", dfu_path);

        File* dfu_file = storage_file_alloc(storage);
        if(storage_file_open(dfu_file, dfu_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
            if(!dfu_file_validate_crc(dfu_file, NULL, NULL)) {
                FURI_LOG_E(TAG, "DFU CRC validation failed: %s", dfu_path);
            } else {
                uint8_t n_targets = dfu_file_validate_headers(dfu_file, &bsb_dfu_params);
                FURI_LOG_I(TAG, "DFU file has %u targets", n_targets);
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
        FURI_LOG_I(TAG, "DFU flash successful. System will restart.");
        furi_delay_ms(100); // Brief delay before reset
        furi_hal_power_reset();
    } else {
        FURI_LOG_W(TAG, "DFU flashing was not performed or failed. System will restart.");
        // Even if DFU flashing didn't happen or failed, we've finished this stage.
        // Boot mode and pointer file are handled by the caller (updater_srv)
    }

    furi_record_close(RECORD_STORAGE);
    updater_state_free(state);
}

int32_t updater_srv(void* arg) {
    UNUSED(arg);
    FURI_LOG_I(TAG, "Updater service (stage) started.");

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* pointer_file = storage_file_alloc(storage);
    char update_folder_relative_path[MAX_PATH_LEN]; // Use MAX_PATH_LEN
    bool path_read_success = false;

    FURI_LOG_I(TAG, "Attempting to read update path from: %s", EXT_PATH(UPDATE_POINTER_FILE_NAME));
    if(storage_file_open(
           pointer_file, EXT_PATH(UPDATE_POINTER_FILE_NAME), FSAM_READ, FSOM_OPEN_EXISTING)) {
        uint16_t bytes_read = storage_file_read(
            pointer_file, update_folder_relative_path, sizeof(update_folder_relative_path) - 1);
        if(storage_file_get_error(pointer_file) == FSE_OK && bytes_read > 0 &&
           bytes_read < sizeof(update_folder_relative_path)) {
            update_folder_relative_path[bytes_read] = '\0';
            path_read_success = true;
            FURI_LOG_I(TAG, "Read relative update path: '%s'", update_folder_relative_path);
        } else {
            FURI_LOG_E(
                TAG,
                "Failed to read from pointer file or path is invalid/empty. Error: %s, Bytes: %d",
                storage_error_get_desc(storage_file_get_error(pointer_file)),
                bytes_read);
        }
        storage_file_close(pointer_file);
    } else {
        FURI_LOG_E(
            TAG,
            "Pointer file %s not found or cannot be opened. Error: %s",
            EXT_PATH(UPDATE_POINTER_FILE_NAME),
            storage_error_get_desc(storage_file_get_error(pointer_file)));
    }
    storage_file_free(pointer_file);

    // Always try to delete the pointer file after attempting to read it.
    // If reading failed, we still want to remove it to prevent issues on next boot.
    // If reading succeeded, it has served its purpose for this stage.
    FURI_LOG_I(TAG, "Deleting pointer file: %s", EXT_PATH(UPDATE_POINTER_FILE_NAME));
    FS_Error fs_err = storage_common_remove(storage, EXT_PATH(UPDATE_POINTER_FILE_NAME));
    if(fs_err != FSE_OK && fs_err != FSE_NOT_EXIST) {
        FURI_LOG_W(
            TAG,
            "Failed to delete pointer file %s. Error: %s",
            EXT_PATH(UPDATE_POINTER_FILE_NAME),
            storage_error_get_desc(fs_err));
    } else {
        FURI_LOG_I(TAG, "Pointer file deleted or was not present.");
    }

    if(!path_read_success) {
        FURI_LOG_E(
            TAG, "Failed to obtain update path. Setting boot mode to normal and rebooting.");
        furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeNormal);
        furi_record_close(RECORD_STORAGE);
        furi_delay_ms(100);
        furi_hal_power_reset(); // Reboot to normal mode
        furi_thread_suspend(furi_thread_get_current_id()); // Should not be reached
        return 1;
    }

    updater_execute(update_folder_relative_path);

    FURI_LOG_I(
        TAG, "Update process operations finished. Setting boot mode to normal and rebooting.");
    furi_hal_nvm_set_boot_mode(FuriHalNvmBootModeNormal);

    furi_record_close(RECORD_STORAGE);
    furi_delay_ms(100); // Brief delay before reset
    furi_hal_power_reset();

    // This part should not be reached as power_reset() will restart the device.
    furi_thread_suspend(furi_thread_get_current_id());
    return 0; // Should not be reached
}
