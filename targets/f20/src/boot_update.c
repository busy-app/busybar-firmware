#include <furi_hal.h>
#include <furi.h>

#include <platform_startup.h>

#include <fatfs.h>
#include <toolbox/update_lib/update_util.h>
#include <toolbox/crc32_calc.h>

#define FS_MOUNT_POINT "/"

static FATFS* pfs = NULL;

static bool platform_boot_update_init(void) {
    bool success = false;

    // Init core HAL systems
    furi_hal_mpu_init();
    furi_hal_clock_init();
    furi_hal_interrupt_init();
    furi_hal_sdmmc_init(false);

    fatfs_init();

    // Init FS
    do {
        if(!furi_hal_sdmmc_is_sd_present()) {
            break;
        }

        if(!furi_hal_sdmmc_init_card()) {
            break;
        }

        pfs = malloc(sizeof(FATFS));
        if(!pfs) {
            break;
        }
        memset(pfs, 0, sizeof(FATFS));
        if(f_mount(pfs, FS_MOUNT_POINT, 1) != FR_OK) {
            free(pfs);
            pfs = NULL;
            break;
        }
        // Check if the filesystem is valid
        DWORD free_clst;
        if(f_getfree(FS_MOUNT_POINT, &free_clst, &pfs) != FR_OK) {
            f_mount(NULL, FS_MOUNT_POINT, 1); // Unmount the filesystem
            free(pfs);
            pfs = NULL;
            break;
        }

        success = true;
    } while(0);

    return success;
}

static void platform_boot_exec_update_deinit(void) {
    if(pfs) {
        f_mount(NULL, FS_MOUNT_POINT, 1); // Unmount the filesystem
        free(pfs);
        pfs = NULL;
    }
    furi_hal_sdmmc_deinit_card();
}

static UpdaterConfig* platform_boot_load_update_config(const char* config_path) {
    UpdaterConfig* config = NULL;
    FIL config_file = {0};
    FRESULT res = f_open(&config_file, config_path, FA_READ);
    if(res == FR_OK) {
        size_t file_size = f_size(&config_file);
        char* buffer = malloc(file_size + 1);
        UINT bytes_read = 0;
        if(buffer && (f_read(&config_file, buffer, file_size, &bytes_read) == FR_OK) &&
           (bytes_read == file_size)) {
            buffer[file_size] = '\0';
            config = updater_config_alloc();
            if(!updater_config_from_memory(config, buffer, file_size)) {
                updater_config_free(config);
                config = NULL;
            }
        }
        if(buffer) free(buffer);
        f_close(&config_file);
    }
    return config;
}

void platform_boot_exec_update(void) {
    if(!platform_boot_update_init()) {
        return;
    }

    const char* config_path = "/update/update.json";
    UpdaterConfig* config = platform_boot_load_update_config(config_path);
    if(!config) {
        platform_boot_exec_update_deinit();
        return;
    }

    updater_config_prefix_paths(config, "/update");

    // Validate stage file integrity and execute if valid
    FIL stage_file = {0};
    FRESULT res = f_open(&stage_file, furi_string_get_cstr(config->updater_stage), FA_READ);
    if(res == FR_OK) {
        size_t file_size = f_size(&stage_file);
        uint8_t* buffer = malloc(file_size);
        UINT bytes_read = 0;
        if(buffer && f_read(&stage_file, buffer, file_size, &bytes_read) == FR_OK &&
           bytes_read == file_size) {
            // Calculate CRC32
            uint32_t crc = crc32_calc_buffer(0, buffer, file_size);
            if(crc == config->updater_stage_crc32) {
                // Disable interrupts, move image to RAM start address and execute it
                __disable_irq();
                memmove((void*)SRAM1_BASE, buffer, file_size);
                furi_hal_cortex_jump(FuriHalCortexJumpSRAM);
            }
        }
        if(buffer) free(buffer);
        f_close(&stage_file);
    }

    updater_config_free(config);
    platform_boot_exec_update_deinit();
}
