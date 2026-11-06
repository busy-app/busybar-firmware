#pragma once

#include "js_app_installer.h"

#include <m-i-list.h>

typedef struct {
    JsAppInstallerOperationType type;
    uint32_t operation_id;
    FuriString* app_id;
    FuriString* url;
    JsAppInstallerAppVersion app_version;
    ILIST_INTERFACE(JsAppInstallerOperationQueue, JsAppInstallerQueueItem);
} JsAppInstallerQueueItem;

M_ILIST_DEF(JsAppInstallerOperationQueue, JsAppInstallerQueueItem, M_POD_OPLIST)

typedef void (*JsAppInstallerOperationStepHandler)(void);

typedef struct {
    JsAppInstallerOperationStepHandler handler;
} JsAppInstallerOperationStep;

typedef struct {
    const JsAppInstallerOperationStep* steps;
} JsAppInstallerOperation;

JsAppInstallerQueueItem*
    js_app_installer_queue_item_alloc(const char* app_id, JsAppInstallerOperationType type, ...);
void js_app_installer_queue_item_free(JsAppInstallerQueueItem* instance);
