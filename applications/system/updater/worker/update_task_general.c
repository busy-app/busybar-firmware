#include "update_task_i.h"

#include <toolbox/update_lib/dfu_file.h>

#define TAG "UpdateTask"

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
    UpdateTask* update_task = context;
    update_task_set_progress(update_task, UpdateTaskStageProgress, progress);
}

static bool update_task_write_dfu(UpdateTask* update_task) {
    DfuUpdateTask page_task = {
        .address_cb = &check_address_boundaries,
        .progress_cb = &update_task_file_progress,
        .task_cb = &update_task_flash_program_page,
        .context = update_task,
    };

    bool success = false;
    do {
        CHECK_RESULT(update_task_open_file(
            update_task,
            updater_manifest_get_path(
                update_config_get_manifest(update_task->config), UpdateManifestPathDfu)));

        File* dfu_file = update_task->file;
        CHECK_RESULT(dfu_file_validate_crc(dfu_file, NULL, NULL));

        uint8_t n_targets = dfu_file_validate_headers(dfu_file, &bsb_dfu_params);
        FURI_LOG_I(TAG, "DFU targets: %u", n_targets);
        if(n_targets == 0) {
            break;
        }

        // Program DFU
        update_task_set_progress(update_task, UpdateTaskStageFlashWrite, 0);
        page_task.task_cb = &update_task_flash_program_page;
        CHECK_RESULT(dfu_file_process_targets(&page_task, dfu_file, n_targets));

        // Verify DFU
        update_task_set_progress(update_task, UpdateTaskStageFlashValidate, 0);
        page_task.task_cb = &pdate_task_compare_flash;
        CHECK_RESULT(dfu_file_process_targets(&page_task, dfu_file, n_targets));

        success = true;
    } while(false);

    return success;
}

int32_t update_task_worker_general(void* context) {
    furi_assert(context);
    UpdateTask* update_task = context;
    bool success = false;

    do {
        CHECK_RESULT(update_task_parse_manifest(update_task));

        if(update_task->state.groups & UpdateTaskStageGroupFirmware) {
            CHECK_RESULT(update_task_write_dfu(update_task));
        }

        update_task_set_progress(update_task, UpdateTaskStageCompleted, 100);
        success = true;
    } while(false);

    if(!success) {
        update_task_set_progress(update_task, UpdateTaskStageError, 0);
        return UPDATE_TASK_FAILED;
    }

    return UPDATE_TASK_NOERR;
}
