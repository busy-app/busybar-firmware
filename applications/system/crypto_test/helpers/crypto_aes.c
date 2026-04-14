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

static void crypto_aes_check(char* tag, uint8_t* encrypted_buffer, uint8_t* decrypted_buffer) {
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

static bool check_status(FuriHalCryptoStatus status) {
    if(status != FuriHalCryptoStatusOk) {
        printf(ANSI_FG_RED "fail (status = %d)\r\n" ANSI_RESET, status);
        return false;
    } else {
        return true;
    }
}

static void crypto_aes_test_custom_size_key(FuriHalCryptoKeyType type) {
    uint8_t encrypted_buffer[BUFFER_SIZE] = {0};
    uint8_t decrypted_buffer[BUFFER_SIZE] = {0};

    uint8_t message[sizeof(message_const)];
    memcpy(message, message_const, sizeof(message_const));

    FuriHalCryptoKey* keyy = furi_hal_crypto_key_alloc();
    memcpy(keyy->data, key_const, sizeof(key_const));
    keyy->type = type;
    keyy->flags = 0;

    switch(type) {
    case FuriHalCryptoKeyTypeAes128:
        printf("\r\n\r\nCrypto AES 128 bit key\r\n");
        keyy->length = FURI_HAL_CRYPTO_AES_KEY_SIZE_128;
        break;
    case FuriHalCryptoKeyTypeAes192:
        printf("\r\n\r\nCrypto AES 192 bit key\r\n");
        keyy->length = FURI_HAL_CRYPTO_AES_KEY_SIZE_192;
        break;
    case FuriHalCryptoKeyTypeAes256:
        printf("\r\n\r\nCrypto AES 256 bit key\r\n");
        keyy->length = FURI_HAL_CRYPTO_AES_KEY_SIZE_256;
        break;
    default:
        furi_assert(false);
        return;
    }

    uint8_t iv[sizeof(iv_const)];
    memcpy(iv, iv_const, sizeof(iv_const));

    //FuriHalCryptoAesModeECB
    printf("Crypto AES FuriHalCryptoAesModeECB\r\n");
    FuriHalCryptoAes* handle = NULL;
    FuriHalCryptoStatus status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeECB, keyy);
    if(check_status(status)) {
        furi_hal_crypto_aes_encrypt(handle, NULL, message, sizeof(message), encrypted_buffer);
        furi_hal_crypto_aes_decrypt(
            handle, NULL, encrypted_buffer, sizeof(message), decrypted_buffer);
        furi_hal_crypto_aes_deinit(handle);

        crypto_aes_check("FuriHalCryptoAesModeECB", encrypted_buffer, decrypted_buffer);
    }

    //FuriHalCryptoAesModeCTR
    printf("Crypto AES FuriHalCryptoAesModeCTR\r\n");
    status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCTR, keyy);

    if(check_status(status)) {
        furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
        furi_hal_crypto_aes_decrypt(
            handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
        furi_hal_crypto_aes_deinit(handle);

        crypto_aes_check("FuriHalCryptoAesModeCTR", encrypted_buffer, decrypted_buffer);
    }

    //FuriHalCryptoAesModeCTR
    printf("Crypto AES FuriHalCryptoAesModeCBC\r\n");

    status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCBC, keyy);

    if(check_status(status)) {
        furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
        furi_hal_crypto_aes_decrypt(
            handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
        furi_hal_crypto_aes_deinit(handle);

        crypto_aes_check("FuriHalCryptoAesModeCBC", encrypted_buffer, decrypted_buffer);
    }

    //Wrap key
    printf("Crypto AES Wrap key\r\n");

    FuriHalCryptoKey* wrapped_keyy = furi_hal_crypto_key_alloc();
    status = furi_hal_crypto_wrap_key(keyy, wrapped_keyy);
    if(check_status(status)) {
        crypto_common_print_key("Key =\t\t", keyy);
        crypto_common_print_key("Wrapped key =\t", wrapped_keyy);

        //FuriHalCryptoAesModeECB, crypt key, decrypt wrapped key
        printf("Crypto AES FuriHalCryptoAesModeECB, Wrap key\r\n");

        status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeECB, keyy);
        if(check_status(status)) {
            furi_hal_crypto_aes_encrypt(handle, NULL, message, sizeof(message), encrypted_buffer);
            furi_hal_crypto_aes_deinit(handle);
        }

        status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeECB, wrapped_keyy);
        if(check_status(status)) {
            furi_hal_crypto_aes_decrypt(
                handle, NULL, encrypted_buffer, sizeof(message), decrypted_buffer);
            furi_hal_crypto_aes_deinit(handle);
        }

        crypto_aes_check("FuriHalCryptoAesModeECB, Wrap key", encrypted_buffer, decrypted_buffer);

        //FuriHalCryptoAesModeCTR, crypt key, decrypt wrapped key
        printf("Crypto AES FuriHalCryptoAesModeCTR, Wrap key\r\n");

        status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCTR, keyy);
        if(check_status(status)) {
            furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
            furi_hal_crypto_aes_deinit(handle);
        }

        status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCTR, wrapped_keyy);
        if(check_status(status)) {
            furi_hal_crypto_aes_decrypt(
                handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
            furi_hal_crypto_aes_deinit(handle);
        }

        crypto_aes_check("FuriHalCryptoAesModeCTR, Wrap key", encrypted_buffer, decrypted_buffer);

        //FuriHalCryptoAesModeCTR, crypt key, decrypt wrapped key
        printf("Crypto AES FuriHalCryptoAesModeCBC, Wrap key\r\n");

        status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCBC, keyy);
        if(check_status(status)) {
            furi_hal_crypto_aes_encrypt(handle, iv, message, sizeof(message), encrypted_buffer);
            furi_hal_crypto_aes_deinit(handle);
        }

        status = furi_hal_crypto_aes_init(&handle, FuriHalCryptoAesModeCBC, wrapped_keyy);
        if(check_status(status)) {
            furi_hal_crypto_aes_decrypt(
                handle, iv, encrypted_buffer, sizeof(message), decrypted_buffer);
            furi_hal_crypto_aes_deinit(handle);
        }
        crypto_aes_check("FuriHalCryptoAesModeCBC, Wrap key", encrypted_buffer, decrypted_buffer);
    }
    furi_hal_crypto_key_free(wrapped_keyy);
    furi_hal_crypto_key_free(keyy);
}

void crypto_aes_command(PipeSide* pipe, FuriString* args, void* context) {
    UNUSED(pipe);
    UNUSED(args);
    UNUSED(context);

    crypto_aes_test_custom_size_key(FuriHalCryptoKeyTypeAes128);
    crypto_aes_test_custom_size_key(FuriHalCryptoKeyTypeAes192);
    crypto_aes_test_custom_size_key(FuriHalCryptoKeyTypeAes256);

    printf("Crypto AES Encryption done\r\n");
}
