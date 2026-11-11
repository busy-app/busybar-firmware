#pragma once

#include "js_app_installer_operation.h"

#include <core/thread.h>

typedef void JsAppInstallerOperationData;

typedef void (*JsAppInstallerOperationInit)(
    JsAppInstallerOperation* instance,
    const JsAppInstallerQueueItem* item);

typedef void (*JsAppInstallerOperationClear)(JsAppInstallerOperation* instance);

typedef JsAppInstallerError (*JsAppInstallerOperationRun)(JsAppInstallerOperation* instance);

typedef struct {
    JsAppInstallerOperationInit init;
    JsAppInstallerOperationClear clear;
    JsAppInstallerOperationRun run;
    size_t data_size;
} JsAppInstallerOperationInterface;

struct JsAppInstallerOperation {
    const JsAppInstallerOperationInterface* interface;

    FuriThread* thread;
    FuriString* app_id;
    JsAppInstallerOperationCallback callback;
    void* callback_context;
};

void js_app_installer_operation_emit(
    const JsAppInstallerOperation* instance,
    const JsAppInstallerOperationEvent* event);
