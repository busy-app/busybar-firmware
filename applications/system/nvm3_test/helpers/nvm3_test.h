#pragma once
#include <furi.h>

bool nvm3_test_init(void);
void nvm3_test_deinit(void);
bool nvm3_test_write(uint32_t key, uint8_t* data, uint32_t len);
bool nvm3_test_read(uint32_t key, uint8_t* data, uint32_t len);
bool nvm3_test_delete(uint32_t key);
bool nvm3_test_increment_counter(uint32_t key, uint32_t* value);
bool nvm3_test_read_counter(uint32_t key, uint32_t* value);
bool nvm3_test_write_counter(uint32_t key, uint32_t value);
bool nvm3_test_erase_all(void);
bool nvm3_test_repack_if_need(void);
void nvm3_test_print_objects(FuriString* msg);
