#include <furi/core/check.h>
#include <furi_hal_nvm.h>

bool furi_crash_handler(bool dap_debug) {
    // 1. If debug adapter is connected, halt
    // 2. If debug build and debug flag is set, halt
    // 3. Otherwise reboot
    if(dap_debug) {
        return false;
    } else {
#if defined(FURI_DEBUG)
        bool sysctl_debug = furi_hal_nvm_is_flag_set(FuriHalNvmFlagDebug);
        return !sysctl_debug;
#else
        return true;
#endif
    }
}
