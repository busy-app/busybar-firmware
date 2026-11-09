#pragma once

#include "js_app_installer.h"

#include <m-i-list.h>

typedef struct {
    JsAppInstallerOperationType type;
    uint32_t id;
    FuriString* app_id;
    FuriString* url;
    JsAppInstallerAppVersion app_version;
    ILIST_INTERFACE(JsAppInstallerOperationQueue, JsAppInstallerQueueItem);
} JsAppInstallerQueueItem;

JsAppInstallerQueueItem*
    js_app_installer_queue_item_alloc(const char* app_id, JsAppInstallerOperationType type, ...);
void js_app_installer_queue_item_free(JsAppInstallerQueueItem* instance);

#define JS_APP_INSTALLER_QUEUE_ITEM_DEL(a) js_app_installer_queue_item_free(a)
#define JS_APP_INSTALLER_QUEUE_ITEM_OPLIST \
    M_OPEXTEND(M_POD_OPLIST, DEL(JS_APP_INSTALLER_QUEUE_ITEM_DEL))

M_ILIST_DEF(
    JsAppInstallerOperationQueue,
    JsAppInstallerQueueItem,
    JS_APP_INSTALLER_QUEUE_ITEM_OPLIST)

typedef void (*JsAppInstallerOperationStepHandler)(void);

typedef struct {
    JsAppInstallerOperationStepHandler handler;
} JsAppInstallerOperationStep;

typedef struct {
    const JsAppInstallerOperationStep* steps;
} JsAppInstallerOperation;
