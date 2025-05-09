#include <furi_hal.h>
#include <furi.h>

#include <platform_startup.h>

#include <fatfs.h>

#define FS_MOUNT_POINT "/boot"

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

void platform_boot_exec_update(void) {
    if(!platform_boot_update_init()) {
        return;
    }
}
