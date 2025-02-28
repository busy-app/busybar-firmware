#include "nvm3.h"

#include <nvm3.h>
#include <nvm3_default_config.h>

#include <args.h>
#include <strint.h>

#define TAG "NVM3"

#define NVM3_DEFAULT_HANDLE   nvm3_defaultHandle
#define NVM3_MAX_OBJECT_COUNT 100
#define NVM3_MIN_DATA_KEY     NVM3_KEY_MIN
#define NVM3_MAX_DATA_KEY     NVM3_KEY_MAX

bool nvm3_test_init(void) {
    sl_status_t err = nvm3_initDefault();
    if(err != ECODE_NVM3_OK) {
        return false;
    }
    return true;
}

void nvm3_test_deinit(void) {
    nvm3_deinitDefault();
}

bool nvm3_test_write(uint32_t key, uint8_t* data, uint32_t len) {
    if(ECODE_NVM3_OK == nvm3_writeData(NVM3_DEFAULT_HANDLE, key, (uint8_t*)data, len)) {
        return true;
    }
    return false;
}

bool nvm3_test_read(uint32_t key, uint8_t* data, uint32_t len) {
    bool ret = false;
    sl_status_t err;
    uint32_t obj_type;
    size_t obj_len;

    err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &obj_type, &obj_len);
    if(err != ECODE_NVM3_OK || obj_type != NVM3_OBJECTTYPE_DATA || len != obj_len) {
        // Key does not contain data object
        ret = false;
    } else if(ECODE_NVM3_OK == nvm3_readData(NVM3_DEFAULT_HANDLE, key, (uint8_t*)data, len)) {
        ret = true;
    }
    return ret;
}

bool nvm3_test_delete(uint32_t key) {
    if(ECODE_NVM3_OK == nvm3_deleteObject(NVM3_DEFAULT_HANDLE, key)) {
        return true;
    }
    return false;
}

bool nvm3_test_increment_counter(uint32_t key, uint32_t* value) {
    bool ret = false;
    sl_status_t err;
    uint32_t obj_type;
    size_t obj_len;

    err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &obj_type, &obj_len);
    if(err != ECODE_NVM3_OK || obj_type != NVM3_OBJECTTYPE_COUNTER) {
        // Key does not contain counter object
        ret = false;
    } else if(ECODE_NVM3_OK == nvm3_incrementCounter(NVM3_DEFAULT_HANDLE, key, value)) {
        ret = true;
    }
    return ret;
}

bool nvm3_test_read_counter(uint32_t key, uint32_t* value) {
    bool ret = false;
    sl_status_t err;
    uint32_t obj_type;
    size_t obj_len;

    err = nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, key, &obj_type, &obj_len);
    if(err != ECODE_NVM3_OK || obj_type != NVM3_OBJECTTYPE_COUNTER) {
        // Key does not contain counter object
        ret = false;
    } else if(ECODE_NVM3_OK == nvm3_readCounter(NVM3_DEFAULT_HANDLE, key, value)) {
        ret = true;
    }
    return ret;
}

bool nvm3_test_write_counter(uint32_t key, uint32_t value) {
    if(ECODE_NVM3_OK == nvm3_writeCounter(NVM3_DEFAULT_HANDLE, key, value)) {
        return true;
    }
    return false;
}

bool nvm3_test_erase_all(void) {
    // Delete all data in NVM3.
    // not used  user don't need to call this function to get NVM3 into an initial valid state
    if(ECODE_NVM3_OK == nvm3_eraseAll(NVM3_DEFAULT_HANDLE)) {
        return true;
    }
    return false;
}

bool nvm3_test_repack_if_need(void) {
    if(ECODE_NVM3_OK == nvm3_repackNeeded(NVM3_DEFAULT_HANDLE)) {
        if(ECODE_NVM3_OK == nvm3_repack(NVM3_DEFAULT_HANDLE)) {
            return true;
        }
    }
    return false;
}

void nvm3_test_print_objects(FuriString* msg) {
    nvm3_ObjectKey_t keys[NVM3_MAX_OBJECT_COUNT];
    size_t len, objects_count;
    uint32_t type;
    Ecode_t err;
    uint32_t counter = 0;
    size_t i;
    uint8_t buffer[NVM3_DEFAULT_MAX_OBJECT_SIZE];

    furi_string_reset(msg);

    objects_count = nvm3_enumDeletedObjects(
        NVM3_DEFAULT_HANDLE,
        (uint32_t*)keys,
        sizeof(keys) / sizeof(keys[0]),
        NVM3_MIN_DATA_KEY,
        NVM3_MAX_DATA_KEY);

    // check for NVM3 stored object count
    if(objects_count == 0) {
        furi_string_cat(msg, "No deleted objects found\r\n");
    } else {
        furi_string_cat(msg, "Keys of objects deleted from NVM3:\r\n");
        for(i = 0; i < objects_count; i++) {
            furi_string_cat_printf(msg, "> %lu\r\n", keys[i]);
        }
    }

    // Retrieve the keys of stored data
    objects_count = nvm3_enumObjects(
        NVM3_DEFAULT_HANDLE,
        (uint32_t*)keys,
        sizeof(keys) / sizeof(keys[0]),
        NVM3_MIN_DATA_KEY,
        NVM3_MAX_DATA_KEY);
    if(objects_count == 0) {
        furi_string_printf(msg, "No stored objects found\r\n");
    } else {
        furi_string_printf(msg, "Keys and contents of objects stored in NVM3:\r\n");
        for(i = 0; i < objects_count; i++) {
            nvm3_getObjectInfo(NVM3_DEFAULT_HANDLE, keys[i], &type, &len);
            if(type == NVM3_OBJECTTYPE_DATA) {
                err = nvm3_readData(NVM3_DEFAULT_HANDLE, keys[i], buffer, len);
                EFM_ASSERT(ECODE_NVM3_OK == err);
                buffer[len] = '\0';
                furi_string_cat_printf(msg, "> %lu: %s\r\n", keys[i], buffer);
            } else if(type == NVM3_OBJECTTYPE_COUNTER) {
                err = nvm3_readCounter(NVM3_DEFAULT_HANDLE, keys[i], &counter);
                EFM_ASSERT(ECODE_NVM3_OK == err);
                furi_string_cat_printf(msg, "> %lu: %lu\r\n", keys[i], counter);
            }
        }
    }
}
