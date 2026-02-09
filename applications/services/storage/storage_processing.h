#pragma once
#include <furi.h>
#include "storage.h"
#include "storage_i.h"
#include "storage_message.h"
#include "storage_glue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Process storage API message.
 *
 * @return true if the shutdown semaphore should be released.
 */
bool storage_process_message(Storage* app, StorageMessage* message);

#ifdef __cplusplus
}
#endif
