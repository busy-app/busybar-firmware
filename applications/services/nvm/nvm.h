#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RECORD_NVM "nvm"

typedef enum {
    NvmKeyMin = 0xFFFFUL,
    NvmKeyUser1Min,
    /* Add more key ranges here */
    NvmKeyMatterMin = 0x87200UL,
    NvmKeyMatterMax = 0x87FFFUL,
    NvmKeyUser2Min,
    /* Add more key ranges here */
    NvmKeyMax = 0x1000000UL,
} NvmKey;

typedef struct Nvm Nvm;

bool nvm_exists(Nvm* instance, uint32_t key, size_t* len);

bool nvm_read(Nvm* instance, uint32_t key, void* data, size_t len);

bool nvm_read_partial(Nvm* instance, uint32_t key, void* data, size_t offset, size_t len);

bool nvm_write(Nvm* instance, uint32_t key, const void* data, size_t len);

bool nvm_read_counter(Nvm* instance, uint32_t key, uint32_t* value);

bool nvm_write_counter(Nvm* instance, uint32_t key, uint32_t value);

bool nvm_increment_counter(Nvm* instance, uint32_t key, uint32_t* value);

bool nvm_delete(Nvm* instance, uint32_t key);

bool nvm_repack(Nvm* instance);

bool nvm_erase_all(Nvm* instance);

#ifdef __cplusplus
}
#endif
