#pragma once
#include <furi.h>
#include "../crypto_test.h"

void crypto_common_print_buffer_char(char* tag, uint8_t* buffer, uint16_t length);

void crypto_common_print_buffer_hex(char* tag, const uint8_t* buffer, uint16_t length);
