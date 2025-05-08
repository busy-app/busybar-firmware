#include <furi.h>
#include <furi_hal_nvm.h>

void furi_hal_rtc_reset_registers(void) {
}

void furi_hal_rtc_set_flag(FuriHalRtcFlag flag) {
    UNUSED(flag);
}

bool furi_hal_rtc_is_flag_set(FuriHalRtcFlag flag) {
    ///TODO: remove this when proper implementation will be done
    return flag == FuriHalRtcFlagDebug;
}

FuriHalRtcBootMode furi_hal_rtc_get_boot_mode(void) {
    return FuriHalRtcBootModeNormal;
}
