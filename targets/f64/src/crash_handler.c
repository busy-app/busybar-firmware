#include <furi/core/check.h>
#include <furi_hal_nvm.h>

bool furi_crash_handler(bool dap_debug) {
    // Never reboot
    UNUSED(dap_debug);
    return false;
}
