#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_nvm.h>
#include <furi_hal_cortex.h>

#include <platform_startup.h>

#include <storage/storage.h>

#include <fatfs.h>
#include <toolbox/update_lib/update_manifest.h>
#include <toolbox/update_lib/common_vals.h>
#include <toolbox/crc32_calc.h>

#define MAX_PATH_LEN         256
#define MAX_CONFIG_FILE_SIZE (2 * 1024)
#define MAX_STAGE_FILE_SIZE  (2 * 1024 * 1024)

#define FS_MOUNT_POINT    "/"
#define POINTER_FILE_PATH FS_MOUNT_POINT UPDATE_POINTER_FILE_NAME

static FATFS* pfs = NULL;

static void platform_boot_update_sys_init(void) {
    furi_hal_mpu_init();
    furi_hal_clock_init();
    furi_hal_interrupt_init();
    furi_hal_sdmmc_init(false);

    fatfs_init();
}

static void platform_boot_update_sys_deinit(void) {
    if(pfs) {
        f_mount(NULL, FS_MOUNT_POINT, 1);
        free(pfs);
        pfs = NULL;
    }
}

static bool platform_boot_update_mount_fs(void) {
    bool success = false;
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
    } while(false);

    return success;
}

static FuriString* platform_boot_read_manifest_path(const char* pointer_file_path) {
    FIL f_pointer_file;

    char relative_path_buffer[MAX_PATH_LEN] = {0};
    FuriString* manifest_path = NULL;

    do {
        FRESULT fres = f_open(&f_pointer_file, pointer_file_path, FA_READ);
        if(fres != FR_OK) {
            break;
        }

        UINT bytes_read_fatfs;
        fres = f_read(&f_pointer_file, relative_path_buffer, MAX_PATH_LEN, &bytes_read_fatfs);
        if(fres != FR_OK || bytes_read_fatfs == 0 || bytes_read_fatfs >= MAX_PATH_LEN) {
            break;
        }

        relative_path_buffer[bytes_read_fatfs] = '\0'; // Null-terminate the string
        manifest_path = furi_string_alloc_set(relative_path_buffer);
        if(furi_string_start_with(manifest_path, STORAGE_EXT_PATH_PREFIX)) {
            furi_string_right(manifest_path, strlen(STORAGE_EXT_PATH_PREFIX));
        }
    } while(false);
    f_close(&f_pointer_file);

    return manifest_path;
}

static bool platform_boot_load_and_verify_stage(
    const UpdateManifest* config,
    uint8_t** stage_buffer,
    size_t* stage_size) {
    *stage_buffer = NULL;
    *stage_size = 0;
    bool success = false;
    FIL stage_file_fatfs = {0};
    FRESULT fres;
    bool file_opened = false;

    do {
        const FuriString* updater_stage_path =
            updater_manifest_get_path(config, UpdateManifestPathStage);
        if(furi_string_empty(updater_stage_path)) {
            break;
        }

        fres = f_open(&stage_file_fatfs, furi_string_get_cstr(updater_stage_path), FA_READ);
        if(fres != FR_OK) {
            break;
        }
        file_opened = true;

        *stage_size = f_size(&stage_file_fatfs);
        if(*stage_size == 0 || *stage_size > MAX_STAGE_FILE_SIZE) {
            break;
        }

        *stage_buffer = malloc(*stage_size);
        if(!*stage_buffer) {
            break;
        }

        UINT bytes_read_fatfs_stage = 0;
        fres = f_read(&stage_file_fatfs, *stage_buffer, *stage_size, &bytes_read_fatfs_stage);
        if(fres != FR_OK || bytes_read_fatfs_stage != *stage_size) {
            break;
        }

        uint32_t crc = crc32_calc_buffer(0, *stage_buffer, *stage_size);
        uint32_t expected_crc = updater_manifest_get_updater_stage_crc32(config);
        if(crc == expected_crc) {
            success = true;
        }
    } while(false);

    if(file_opened) {
        f_close(&stage_file_fatfs);
    }

    if(!success) {
        if(*stage_buffer) {
            free(*stage_buffer);
            *stage_buffer = NULL;
        }
        *stage_size = 0;
    }

    return success;
}

static inline void platform_boot_jump_to_stage(uint8_t* stage_buffer, size_t stage_size) {
    __disable_irq();
    memmove((void*)SRAM1_BASE, stage_buffer, stage_size);
    furi_hal_cortex_jump(FuriHalCortexJumpSRAM);
}

static UpdateManifest* platform_boot_load_update_config(const char* config_path) {
    UpdateManifest* config = NULL;
    FIL config_file = {0};
    FRESULT res = f_open(&config_file, config_path, FA_READ);
    if(res == FR_OK) {
        size_t file_size = f_size(&config_file);
        if(file_size > 0 && file_size < MAX_CONFIG_FILE_SIZE) {
            char* buffer = malloc(file_size + 1);
            UINT bytes_read = 0;
            if((f_read(&config_file, buffer, file_size, &bytes_read) == FR_OK) &&
               (bytes_read == file_size)) {
                buffer[file_size] = '\0';
                config = updater_manifest_alloc();
                if(!updater_manifest_init_from_memory(config, buffer, file_size)) {
                    updater_manifest_free(config);
                    config = NULL;
                }
            }
            free(buffer);
        }
        f_close(&config_file);
    }
    return config;
}

static void platform_boot_exec_update(FuriString** out_update_config_path, bool* out_fs_mounted) {
    *out_fs_mounted = false;
    FuriString* local_update_config_path = NULL;
    UpdateManifest* config = NULL;
    uint8_t* stage_load_buffer = NULL;

    if(!platform_boot_update_mount_fs()) {
        return;
    }
    *out_fs_mounted = true;

    local_update_config_path = platform_boot_read_manifest_path(POINTER_FILE_PATH);
    *out_update_config_path = local_update_config_path;

    if(!local_update_config_path) {
        return;
    }
    do {
        config = platform_boot_load_update_config(furi_string_get_cstr(local_update_config_path));
        if(!config) {
            break;
        }

        FuriString* base_path_str = furi_string_alloc_set(local_update_config_path);
        furi_string_left(base_path_str, furi_string_search_rchar(base_path_str, '/'));
        updater_manifest_prefix_paths(config, furi_string_get_cstr(base_path_str));
        furi_string_free(base_path_str);

        uint8_t* stage_execution_buffer_ptr = NULL;
        size_t stage_execution_size = 0;

        if(platform_boot_load_and_verify_stage(
               config, &stage_execution_buffer_ptr, &stage_execution_size)) {
            stage_load_buffer =
                stage_execution_buffer_ptr; // stage_load_buffer now owns the memory
            platform_boot_jump_to_stage(stage_load_buffer, stage_execution_size);
            // If platform_boot_jump_to_stage is successful, execution does not continue here.
            // If it returns, it's an error; stage_load_buffer needs to be freed.
        }
    } while(false);

    if(stage_load_buffer) free(stage_load_buffer);
    if(config) updater_manifest_free(config);
}

void platform_boot_to_update(void) {
    platform_boot_update_sys_init();

    FuriString* update_config_path = NULL;
    bool fs_mounted_by_exec = false;

    platform_boot_exec_update(&update_config_path, &fs_mounted_by_exec);

    // should not reach here if jump was successful - do cleanup otherwise
    if(fs_mounted_by_exec) {
        platform_boot_update_sys_deinit();
    }

    if(update_config_path) {
        furi_string_free(update_config_path);
    }
}
