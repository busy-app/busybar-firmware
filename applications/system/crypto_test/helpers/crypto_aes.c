#include "crypto_aes.h"
#include "crypto_common.h"

#include <furi_hal_aes.h>
#include <furi_hal_wrap_key.h>

#define TAG "Crypto_AES"

#define BUFFER_SIZE 128

static const uint8_t message_const[] =
    {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!'};

static const uint8_t key_const[FURI_HAL_AES_KEY_SIZE_256] = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!'};

// IV used for SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes.
static const uint8_t iv_const[FURI_HAL_AES_IV_SIZE] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void crypto_aes_chek(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    uint8_t* encrypted_buffer,
    uint8_t* decrypted_buffer) {
    crypto_common_print_buffer_char(
        app, msg, "msg =\t\t", (uint8_t*)message_const, sizeof(message_const));
    crypto_common_print_buffer_hex(
        app, msg, "msg =\t\t", (uint8_t*)message_const, sizeof(message_const));

    crypto_common_print_buffer_hex(
        app, msg, "msg ecrypt =\t", encrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
    crypto_common_print_buffer_hex(
        app, msg, "msg decrypt= \t", decrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
    if(memcmp(message_const, decrypted_buffer, sizeof(message_const)) != 0) {
        furi_string_printf(msg, "\033[0;31m %s mode failed\033[0m\r\n", tag);
        crypto_test_app_send_text(app, msg);
    } else {
        crypto_common_print_buffer_char(
            app, msg, "msg decrypt= \t", decrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
        furi_string_printf(msg, "\033[0;32m %s mode success\033[0m\r\n", tag);
        crypto_test_app_send_text(app, msg);
    }
}

void crypto_aes_test_custom_size_key(CryptoTestApp* app, FuriString* msg, size_t key_size) {
    uint8_t encrypted_buffer[BUFFER_SIZE] = {0};
    uint8_t decrypted_buffer[BUFFER_SIZE] = {0};
    uint8_t message[sizeof(message_const)];
    memcpy(message, message_const, sizeof(message_const));
    uint8_t key[sizeof(key_const)];
    memcpy(key, key_const, sizeof(key_const));
    uint8_t iv[sizeof(iv_const)];
    memcpy(iv, iv_const, sizeof(iv_const));

    if(key_size == FURI_HAL_AES_KEY_SIZE_128) {
        furi_string_printf(msg, "\r\n\r\n\033[0;33m Crypto AES 128 bit key\033[0m\r\n");
    } else if(key_size == FURI_HAL_AES_KEY_SIZE_192) {
        furi_string_printf(msg, "\r\n\r\n\033[0;33m Crypto AES 192 bit key\033[0m\r\n");
    } else if(key_size == FURI_HAL_AES_KEY_SIZE_256) {
        furi_string_printf(msg, "\r\n\r\n\033[0;33m Crypto AES 256 bit key\033[0m\r\n");
    }
    crypto_test_app_send_text(app, msg);

    //FuriHalAesModeECB
    furi_string_printf(msg, "Crypto AES FuriHalAesModeECB\r\n");
    crypto_test_app_send_text(app, msg);
    FuriHalAes* handle =
        furi_hal_aes_init(FuriHalAesModeECB, key, key_size, NULL, FuriHalAesWrappingModeOff);

    furi_hal_aes_encrypt(handle, message, sizeof(message), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(message), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_chek(app, msg, "FuriHalAesModeECB", encrypted_buffer, decrypted_buffer);

    //FuriHalAesModeCTR
    furi_string_printf(msg, "Crypto AES FuriHalAesModeCTR\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(FuriHalAesModeCTR, key, key_size, iv, FuriHalAesWrappingModeOff);

    furi_hal_aes_encrypt(handle, message, sizeof(message), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(message), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_chek(app, msg, "FuriHalAesModeCTR", encrypted_buffer, decrypted_buffer);

    //FuriHalAesModeCTR
    furi_string_printf(msg, "Crypto AES FuriHalAesModeCBC\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(FuriHalAesModeCBC, key, key_size, iv, FuriHalAesWrappingModeOff);

    furi_hal_aes_encrypt(handle, message, sizeof(message), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(message), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_chek(app, msg, "FuriHalAesModeCBC", encrypted_buffer, decrypted_buffer);

    //Wrap key
    furi_string_printf(msg, "Crypto AES Wrap key\r\n");
    crypto_test_app_send_text(app, msg);

    uint8_t wrapped_key[FURI_HAL_AES_KEY_SIZE_256] = {0};
    furi_hal_wrap_key(key_size, key, wrapped_key);
    crypto_common_print_buffer_hex(app, msg, "Key =\t\t", key, key_size);
    crypto_common_print_buffer_hex(app, msg, "Wrapped key =\t", wrapped_key, key_size);

    //FuriHalAesModeECB, crypt key, decrypt wrapped key
    furi_string_printf(msg, "Crypto AES FuriHalAesModeECB, Wrap key\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(FuriHalAesModeECB, key, key_size, NULL, FuriHalAesWrappingModeOff);
    furi_hal_aes_encrypt(handle, message, sizeof(message), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_deinit(handle);

    handle = furi_hal_aes_init(
        FuriHalAesModeECB, wrapped_key, key_size, NULL, FuriHalAesWrappingModeOn);
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(message), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_chek(app, msg, "FuriHalAesModeECB, Wrap key", encrypted_buffer, decrypted_buffer);

    //FuriHalAesModeCTR, crypt key, decrypt wrapped key
    furi_string_printf(msg, "Crypto AES FuriHalAesModeCTR, Wrap key\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(FuriHalAesModeCTR, key, key_size, iv, FuriHalAesWrappingModeOff);
    furi_hal_aes_encrypt(handle, message, sizeof(message), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_deinit(handle);

    handle =
        furi_hal_aes_init(FuriHalAesModeCTR, wrapped_key, key_size, iv, FuriHalAesWrappingModeOn);
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(message), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_chek(app, msg, "FuriHalAesModeCTR, Wrap key", encrypted_buffer, decrypted_buffer);

    //FuriHalAesModeCTR, crypt key, decrypt wrapped key
    furi_string_printf(msg, "Crypto AES FuriHalAesModeCBC, Wrap key\r\n");
    crypto_test_app_send_text(app, msg);

    handle = furi_hal_aes_init(FuriHalAesModeCBC, key, key_size, iv, FuriHalAesWrappingModeOff);
    furi_hal_aes_encrypt(handle, message, sizeof(message), encrypted_buffer);
    printf("\r\nAES encryption success\r\n");
    furi_hal_aes_deinit(handle);

    handle =
        furi_hal_aes_init(FuriHalAesModeCBC, wrapped_key, key_size, iv, FuriHalAesWrappingModeOn);
    furi_hal_aes_decrypt(handle, encrypted_buffer, sizeof(message), decrypted_buffer);
    printf("\r\nAES decryption success\r\n");
    furi_hal_aes_deinit(handle);

    crypto_aes_chek(app, msg, "FuriHalAesModeCBC, Wrap key", encrypted_buffer, decrypted_buffer);
}

void crypto_aes_test(CryptoTestApp* app, FuriString* msg) {
    crypto_aes_test_custom_size_key(app, msg, FURI_HAL_AES_KEY_SIZE_128);
    crypto_aes_test_custom_size_key(app, msg, FURI_HAL_AES_KEY_SIZE_192);
    crypto_aes_test_custom_size_key(app, msg, FURI_HAL_AES_KEY_SIZE_256);

    furi_string_printf(msg, "Crypto AES Encryption done\r\n");
    crypto_test_app_send_text(app, msg);
}
