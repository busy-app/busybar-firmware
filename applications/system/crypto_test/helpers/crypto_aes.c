#include "crypto_aes.h"
#include "crypto_common.h"

#include <furi_hal_crypto.h>
#include <cli/cli_ansi.h>

#define TAG "CryptoAES"

#define BUFFER_SIZE 128

static const uint8_t message_const[] = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd',
                                        '!', '!', '!', '!', '!', 'H', 'e', 'l', 'l', 'o', ' ',
                                        'W', 'o', 'r', 'l', 'd', '!', '!', '!', '1', '1'};

static const uint8_t key_const[FURI_HAL_CRYPTO_AES_KEY_SIZE_256] = {
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!',
    'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '!', '!', '!', '!', '!'};

// IV used for SL_SI91X_AES_CBC and SL_SI91X_AES_CTR modes.
static const uint8_t iv_const[FURI_HAL_CRYPTO_AES_IV_SIZE] =
    {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void crypto_aes_check(char* tag, uint8_t* encrypted_buffer, uint8_t* decrypted_buffer) {
    crypto_common_print_buffer_char("msg =\t\t", (uint8_t*)message_const, sizeof(message_const));
    crypto_common_print_buffer_hex("msg =\t\t", (uint8_t*)message_const, sizeof(message_const));

    crypto_common_print_buffer_hex(
        "msg ecrypt =\t", encrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
    crypto_common_print_buffer_hex(
        "msg decrypt= \t", decrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
    if(memcmp(message_const, decrypted_buffer, sizeof(message_const)) != 0) {
        printf(ANSI_FG_RED "%s mode failed\r\n" ANSI_RESET, tag);
    } else {
        crypto_common_print_buffer_char(
            "msg decrypt= \t", decrypted_buffer, (BUFFER_SIZE < 64 ? BUFFER_SIZE : 64));
        printf(ANSI_FG_GREEN "%s mode success\r\n" ANSI_RESET, tag);
    }
}

void crypto_aes_test_custom_size_key(size_t key_size) {
    uint8_t encrypted_buffer[BUFFER_SIZE] = {0};
    uint8_t decrypted_buffer[BUFFER_SIZE] = {0};
    uint8_t message[sizeof(message_const)];
    memcpy(message, message_const, sizeof(message_const));
    uint8_t key[sizeof(key_const)];
    memcpy(key, key_const, sizeof(key_const));
    uint8_t iv[sizeof(iv_const)];
    memcpy(iv, iv_const, sizeof(iv_const));

    if(key_size == FURI_HAL_CRYPTO_AES_KEY_SIZE_128) {
        printf("\r\n\r\nCrypto AES 128 bit key\r\n");
    } else if(key_size == FURI_HAL_CRYPTO_AES_KEY_SIZE_192) {
        printf("\r\n\r\nCrypto AES 192 bit key\r\n");
    } else if(key_size == FURI_HAL_CRYPTO_AES_KEY_SIZE_256) {
        printf("\r\n\r\nCrypto AES 256 bit key\r\n");
    }

    //FuriHalCryptoAesModeECB
    printf("Crypto AES FuriHalCryptoAesModeECB\r\n");
    FuriHalCryptoAes* handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeECB, key, key_size, FuriHalCryptoWrappingModeOff);

    furi_hal_crypto_aes_encrypt(handle, NULL, message, sizeof(message), encrypted_buffer);
    furi_hal_crypto_aes_decrypt(handle, NULL, encrypted_buffer, sizeof(message), decrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    crypto_aes_check("FuriHalCryptoAesModeECB", encrypted_buffer, decrypted_buffer);

    //FuriHalCryptoAesModeCTR
    printf("Crypto AES FuriHalCryptoAesModeCTR\r\n");

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeCTR, key, key_size, FuriHalCryptoWrappingModeOff);

    furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
    furi_hal_crypto_aes_decrypt(handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    crypto_aes_check("FuriHalCryptoAesModeCTR", encrypted_buffer, decrypted_buffer);

    //FuriHalCryptoAesModeCTR
    printf("Crypto AES FuriHalCryptoAesModeCBC\r\n");

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeCBC, key, key_size, FuriHalCryptoWrappingModeOff);

    furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
    furi_hal_crypto_aes_decrypt(handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    crypto_aes_check("FuriHalCryptoAesModeCBC", encrypted_buffer, decrypted_buffer);

    //Wrap key
    printf("Crypto AES Wrap key\r\n");

    uint8_t wrapped_key[FURI_HAL_CRYPTO_AES_KEY_SIZE_256] = {0};
    furi_hal_crypto_wrap_key(key_size, key, wrapped_key);
    crypto_common_print_buffer_hex("Key =\t\t", key, key_size);
    crypto_common_print_buffer_hex("Wrapped key =\t", wrapped_key, key_size);

    //FuriHalCryptoAesModeECB, crypt key, decrypt wrapped key
    printf("Crypto AES FuriHalCryptoAesModeECB, Wrap key\r\n");

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeECB, key, key_size, FuriHalCryptoWrappingModeOff);
    furi_hal_crypto_aes_encrypt(handle, NULL, message, sizeof(message), encrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeECB, wrapped_key, key_size, FuriHalCryptoWrappingModeOn);
    furi_hal_crypto_aes_decrypt(handle, NULL, encrypted_buffer, sizeof(message), decrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    crypto_aes_check("FuriHalCryptoAesModeECB, Wrap key", encrypted_buffer, decrypted_buffer);

    //FuriHalCryptoAesModeCTR, crypt key, decrypt wrapped key
    printf("Crypto AES FuriHalCryptoAesModeCTR, Wrap key\r\n");

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeCTR, key, key_size, FuriHalCryptoWrappingModeOff);
    furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeCTR, wrapped_key, key_size, FuriHalCryptoWrappingModeOn);
    furi_hal_crypto_aes_decrypt(handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    crypto_aes_check("FuriHalCryptoAesModeCTR, Wrap key", encrypted_buffer, decrypted_buffer);

    //FuriHalCryptoAesModeCTR, crypt key, decrypt wrapped key
    printf("Crypto AES FuriHalCryptoAesModeCBC, Wrap key\r\n");

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeCBC, key, key_size, FuriHalCryptoWrappingModeOff);
    furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    handle = furi_hal_crypto_aes_init(
        FuriHalCryptoAesModeCBC, wrapped_key, key_size, FuriHalCryptoWrappingModeOn);
    furi_hal_crypto_aes_decrypt(handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
    furi_hal_crypto_aes_deinit(handle);

    crypto_aes_check("FuriHalCryptoAesModeCBC, Wrap key", encrypted_buffer, decrypted_buffer);
}

void crypto_aes_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    crypto_aes_test_custom_size_key(FURI_HAL_CRYPTO_AES_KEY_SIZE_128);
    crypto_aes_test_custom_size_key(FURI_HAL_CRYPTO_AES_KEY_SIZE_192);
    crypto_aes_test_custom_size_key(FURI_HAL_CRYPTO_AES_KEY_SIZE_256);

    printf("Crypto AES Encryption done\r\n");
}
