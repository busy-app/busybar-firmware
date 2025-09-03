#include "nvm.h"

#include <nvm3.h>
#include <nvm3_default_config.h>

#include <furi.h>

#include <wifi/wifi_common.h>

#define TAG "Nvm"

struct Nvm {
    FuriMutex* lock;
};

static Nvm nvm_instance;

bool nvm_exists(Nvm* instance, uint32_t key, size_t* len) {
    furi_check(instance);

    uint32_t dummy_type;
    size_t dummy_len;

    return nvm3_getObjectInfo(nvm3_defaultHandle, key, &dummy_type, len ? len : &dummy_len) ==
           SL_STATUS_OK;
}

bool nvm_read(Nvm* instance, uint32_t key, void* data, size_t len) {
    furi_check(instance);
    furi_check(data);

    return nvm3_readData(nvm3_defaultHandle, key, data, len) == SL_STATUS_OK;
}

bool nvm_read_partial(Nvm* instance, uint32_t key, void* data, size_t offset, size_t len) {
    furi_check(instance);
    furi_check(data);

    return nvm3_readPartialData(nvm3_defaultHandle, key, data, offset, len) == SL_STATUS_OK;
}

bool nvm_write(Nvm* instance, uint32_t key, const void* data, size_t len) {
    furi_check(instance);
    furi_check(data);

    return nvm3_writeData(nvm3_defaultHandle, key, data, len) == SL_STATUS_OK;
}

bool nvm_read_counter(Nvm* instance, uint32_t key, uint32_t* value) {
    furi_check(instance);
    furi_check(value);

    return nvm3_readCounter(nvm3_defaultHandle, key, value) == SL_STATUS_OK;
}

bool nvm_write_counter(Nvm* instance, uint32_t key, uint32_t value) {
    furi_check(instance);
    return nvm3_writeCounter(nvm3_defaultHandle, key, value) == SL_STATUS_OK;
}

bool nvm_increment_counter(Nvm* instance, uint32_t key, uint32_t* value) {
    furi_check(instance);
    return nvm3_incrementCounter(nvm3_defaultHandle, key, value) == SL_STATUS_OK;
}

bool nvm_delete(Nvm* instance, uint32_t key) {
    furi_check(instance);
    return nvm3_deleteObject(nvm3_defaultHandle, key) == SL_STATUS_OK;
}

bool nvm_repack(Nvm* instance) {
    furi_check(instance);
    return nvm3_repack(nvm3_defaultHandle) == SL_STATUS_OK;
}

bool nvm_erase_all(Nvm* instance) {
    furi_check(instance);
    return nvm3_eraseAll(nvm3_defaultHandle) == SL_STATUS_OK;
}

// Startup hook

void nvm_on_system_start(void) {
    // Wifi is needed to access all NWP functions
    furi_record_open(RECORD_WIFI);

    furi_check(nvm_instance.lock == NULL);
    nvm_instance.lock = furi_mutex_alloc(FuriMutexTypeNormal);

    furi_check(nvm3_initDefault() == SL_STATUS_OK);
    furi_record_create(RECORD_NVM, &nvm_instance);

    FURI_LOG_I(TAG, "Init OK");
}

// Redefinitions of weak SDK functions

void nvm3_lockBegin(void) {
    furi_check(furi_mutex_acquire(nvm_instance.lock, FuriWaitForever) == FuriStatusOk);
}

void nvm3_lockEnd(void) {
    furi_check(furi_mutex_release(nvm_instance.lock) == FuriStatusOk);
}
