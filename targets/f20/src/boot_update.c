#include <furi_hal.h>
#include <furi.h>

#include <platform_startup.h>

#include <fatfs.h>

#define FS_MOUNT_POINT "/"

static FATFS* pfs = NULL;

static bool platform_boot_update_init(void) {
    // Init core HAL systems
    bool success = false;

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

#define UPDATE_STAGE_FILE FS_MOUNT_POINT "/updater.bin"

void platform_boot_exec_update(void) {
    if(!platform_boot_update_init()) {
        return;
    }

    FIL stage_file = {0};
    FILINFO stat = {0};
    uint8_t* buffer = NULL;

    do {
        if(f_stat(UPDATE_STAGE_FILE, &stat) != FR_OK) {
            break;
        }

        if(stat.fsize == 0) {
            break;
        }

        if(f_open(&stage_file, UPDATE_STAGE_FILE, FA_READ) != FR_OK) {
            break;
        }

        buffer = malloc(stat.fsize);

        UINT bytes_read;
        if(f_read(&stage_file, buffer, stat.fsize, &bytes_read) != FR_OK ||
           bytes_read != stat.fsize) {
            f_close(&stage_file);
            break;
        }

        f_close(&stage_file);

        // Disable interrupts, move image to RAM start address and execute it
        __disable_irq();
        memmove((void*)0x20000000, buffer, stat.fsize);
        furi_hal_cortex_jump(FuriHalCortexJumpSRAM);

    } while(0);

    if(buffer) {
        free(buffer);
    }

    platform_boot_exec_update_deinit();
}
