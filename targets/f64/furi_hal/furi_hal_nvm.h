#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalRtcBootModeDummy, //Just to get it to compile
} FuriHalRtcBootMode;

/** Get RTC boot mode
 *
 * @return     The RTC boot mode.
 */
FuriHalRtcBootMode furi_hal_nvm_get_boot_mode(void);

#ifdef __cplusplus
}
#endif
