#include "crypto_common.h"

void crypto_common_print_buffer_char(const char* tag, const uint8_t* buffer, uint16_t length) {
    printf("%s", tag);
    for(uint16_t i = 0; i < length; i++) {
        if(buffer[i] != 0x00) {
            printf("%c", buffer[i]);
        } else {
            printf("[00]");
        }
    }
    printf("\r\n");
}

void crypto_common_print_buffer_hex(const char* tag, const uint8_t* buffer, uint16_t length) {
    printf("%s", tag);

    for(uint16_t i = 0; i < length; i++) {
        printf("%02X", buffer[i]);
    }
    printf("\r\n");
}

void crypto_common_print_key(const char* tag, const FuriHalCryptoKey* key) {
    printf("%s", tag);
    printf("%s ", furi_hal_crypto_get_key_type_name(key->type));
    if(furi_hal_crypto_key_is_wrapped(key)) {
        printf("wrapped ");
    }
    crypto_common_print_buffer_hex("", key->data, key->length);
}

CryptoCommonTestResult
    crypto_common_test_result_compose(CryptoCommonTestResult a, CryptoCommonTestResult b) {
    return MAX(a, b);
}
