#include "crypto_aes.h"
#include <sl_si91x_aes.h>

#include <furi_hal_aes.h>

#define TAG "Crypto_AES"

#define BUFFER_SIZE 256

// uint8_t msg1[] =
//     {0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a};
uint8_t msg1[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!'};

// uint8_t key[SL_SI91X_AES_KEY_SIZE_256] = {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe,
//                                           0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
//                                           0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7,
//                                           0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4};
uint8_t key[SL_SI91X_AES_KEY_SIZE_256] = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!', 'H',
    'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!'}; // 256 bits key

uint8_t encrypted_buffer[BUFFER_SIZE];
uint8_t decrypted_buffer[BUFFER_SIZE];

// IV used for SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes.
uint8_t iv[SL_SI91X_IV_SIZE] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void crypto_aes_print_buffer_char(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    uint8_t* buffer,
    uint16_t length) {
    furi_string_printf(msg, "%s", tag);
    for(uint16_t i = 0; i < length; i++) {
        if(buffer[i] != 0x00) {
            furi_string_cat_printf(msg, "%c", buffer[i]);
        } else {
            furi_string_cat_printf(msg, "[00]");
        }
    }
    furi_string_cat_printf(msg, "\r\n");
    crypto_test_app_send_text(app, msg);
}

void crypto_aes_print_buffer_hex(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    uint8_t* buffer,
    uint16_t length) {
    furi_string_printf(msg, "%s", tag);

    for(uint16_t i = 0; i < length; i++) {
        furi_string_cat_printf(msg, "%02X", buffer[i]);
    }
    furi_string_cat_printf(msg, "\r\n");
    crypto_test_app_send_text(app, msg);
}

void crypto_aes_encryption(CryptoTestApp* app, FuriString* msg) {
    //FuriHalAesModeECB

    furi_string_printf(msg, "Crypto AES FuriHalAesModeECB\r\n");
    crypto_test_app_send_text(app, msg);
    FuriHalAes* handle = furi_hal_aes_init(
        FuriHalAesKeySize256, FuriHalAesModeECB, key, NULL, FuriHalAesWrappingModeOff);

    furi_hal_aes_encrypt(handle, msg1, sizeof(msg1), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(msg1), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_print_buffer_char(app, msg, "msg =\t\t", msg1, sizeof(msg1));
    crypto_aes_print_buffer_hex(app, msg, "msg =\t\t", msg1, sizeof(msg1));

    crypto_aes_print_buffer_hex(app, msg, "msg ecrypt =\t", encrypted_buffer, 64);
    crypto_aes_print_buffer_hex(app, msg, "msg decrypt= \t", decrypted_buffer, 64);
    crypto_aes_print_buffer_char(app, msg, "msg decrypt= \t", decrypted_buffer, 64);

    //FuriHalAesModeCTR
    furi_string_printf(msg, "Crypto AES FuriHalAesModeCTR\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(
        FuriHalAesKeySize256, FuriHalAesModeCTR, key, iv, FuriHalAesWrappingModeOff);

    furi_hal_aes_encrypt(handle, msg1, sizeof(msg1), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(msg1), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_print_buffer_char(app, msg, "msg =\t\t", msg1, sizeof(msg1));
    crypto_aes_print_buffer_hex(app, msg, "msg =\t\t", msg1, sizeof(msg1));

    crypto_aes_print_buffer_hex(app, msg, "msg ecrypt =\t", encrypted_buffer, 64);
    crypto_aes_print_buffer_hex(app, msg, "msg decrypt= \t", decrypted_buffer, 64);
    crypto_aes_print_buffer_char(app, msg, "msg decrypt= \t", decrypted_buffer, 64);

    //FuriHalAesModeCTR
    furi_string_printf(msg, "Crypto AES FuriHalAesModeCBC\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(
        FuriHalAesKeySize256, FuriHalAesModeCBC, key, iv, FuriHalAesWrappingModeOff);

    furi_hal_aes_encrypt(handle, msg1, sizeof(msg1), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(msg1), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_print_buffer_char(app, msg, "msg =\t\t", msg1, sizeof(msg1));
    crypto_aes_print_buffer_hex(app, msg, "msg =\t\t", msg1, sizeof(msg1));

    crypto_aes_print_buffer_hex(app, msg, "msg ecrypt =\t", encrypted_buffer, 64);
    crypto_aes_print_buffer_hex(app, msg, "msg decrypt= \t", decrypted_buffer, 64);
    crypto_aes_print_buffer_char(app, msg, "msg decrypt= \t", decrypted_buffer, 64);

    furi_string_printf(msg, "Crypto AES Encryption done\r\n");
    crypto_test_app_send_text(app, msg);
}
