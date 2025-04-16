#include "crypto_common.h"

void crypto_common_print_buffer_char(
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

void crypto_common_print_buffer_hex(
    CryptoTestApp* app,
    FuriString* msg,
    char* tag,
    const uint8_t* buffer,
    uint16_t length) {
    furi_string_printf(msg, "%s", tag);

    for(uint16_t i = 0; i < length; i++) {
        furi_string_cat_printf(msg, "%02X", buffer[i]);
    }
    furi_string_cat_printf(msg, "\r\n");
    crypto_test_app_send_text(app, msg);
}
