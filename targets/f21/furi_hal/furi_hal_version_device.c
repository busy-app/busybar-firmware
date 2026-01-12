#include <furi_hal_version.h>

bool furi_hal_version_check_target_match(void) {
    return furi_hal_version_get_hw_target() == 21;
}
