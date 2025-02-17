#pragma once

#include <sl_si91x_aes.h>

typedef struct FuriHalAes FuriHalAes;
typedef enum {
    FuriHalAesKeySize128 = SL_SI91X_AES_KEY_SIZE_128,
    FuriHalAesKeySize192 = SL_SI91X_AES_KEY_SIZE_192,
    FuriHalAesKeySize256 = SL_SI91X_AES_KEY_SIZE_256,
} FuriHalAesKeySize;

typedef enum {
    FuriHalAesModeCBC = SL_SI91X_AES_CBC,
    FuriHalAesModeECB = SL_SI91X_AES_ECB,
    FuriHalAesModeCTR = SL_SI91X_AES_CTR,
} FuriHalAesMode;

typedef enum {
    FuriHalAesWrappingModeOff,
    FuriHalAesWrappingModeOn,
} FuriHalAesWrappingMode;

#ifdef __cplusplus
extern "C" {
#endif
FuriHalAes* furi_hal_aes_init(
    FuriHalAesKeySize key_size,
    FuriHalAesMode mode,
    uint8_t* key,
    uint8_t* iv,
    FuriHalAesWrappingMode wrapping_mode);
void furi_hal_aes_deinit(FuriHalAes* handle);
bool furi_hal_aes_encrypt(
    FuriHalAes* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output);
bool furi_hal_aes_decrypt(
    FuriHalAes* handle,
    uint8_t* input,
    uint16_t input_length,
    uint8_t* output);

#ifdef __cplusplus
}
#endif
