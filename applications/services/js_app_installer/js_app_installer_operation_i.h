#pragma once

#include "js_app_installer_operation.h"

#include <core/thread.h>

typedef void JsAppInstallerOperationData;

typedef JsAppInstallerOperationData* (*JsAppInstallerOperationInterfaceAlloc)(
    const JsAppInstallerQueueItem* item);
typedef void (*JsAppInstallerOperationInterfaceFree)(JsAppInstallerOperationData* data);
typedef JsAppInstallerError (*JsAppInstallerOperationInterfaceRun)(
    JsAppInstallerOperation* instance);

typedef struct {
    JsAppInstallerOperationInterfaceAlloc alloc;
    JsAppInstallerOperationInterfaceFree free;
    JsAppInstallerOperationInterfaceRun run;
} JsAppInstallerOperationInterface;

struct JsAppInstallerOperation {
    FuriThread* thread;
    FuriString* app_id;

    JsAppInstallerOperationCallback callback;
    void* callback_context;

    const JsAppInstallerOperationInterface* interface;
    JsAppInstallerOperationData* data;
};

void js_app_installer_operation_emit(
    const JsAppInstallerOperation* instance,
    const JsAppInstallerOperationEvent* event);
