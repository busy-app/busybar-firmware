#include <furi_hal_version.h>
#include <furi_hal_rtc.h>

#include <furi.h>

#define TAG "FuriHalVersion"

const struct Version* furi_hal_version_get_firmware_version(void) {
    return version_get();
}
