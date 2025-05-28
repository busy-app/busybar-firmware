#include "crypto_common.h"

void crypto_common_print_buffer_char(char* tag, uint8_t* buffer, uint16_t length) {
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

void crypto_common_print_buffer_hex(char* tag, const uint8_t* buffer, uint16_t length) {
    printf("%s", tag);

    for(uint16_t i = 0; i < length; i++) {
        printf("%02X", buffer[i]);
    }
    printf("\r\n");
}
