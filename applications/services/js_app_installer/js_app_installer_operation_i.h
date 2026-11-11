#pragma once

#include "js_app_installer_operation.h"

#include <core/thread.h>

typedef void (*JsAppInstallerOperationInit)(
    JsAppInstallerOperation* instance,
    const JsAppInstallerQueueItem* item);

typedef void (*JsAppInstallerOperationDeinit)(JsAppInstallerOperation* instance);

typedef void (*JsAppInstallerOperationCancel)(JsAppInstallerOperation* instance);

typedef void (*JsAppInstallerOperationCleanup)(JsAppInstallerOperation* instance);

typedef JsAppInstallerError (*JsAppInstallerOperationStepHandler)(
    JsAppInstallerOperation* instance);

typedef struct {
    const char* name;
    JsAppInstallerOperationStepHandler handler;
} JsAppInstallerOperationStep;

typedef struct {
    JsAppInstallerOperationInit init;
    JsAppInstallerOperationDeinit deinit;
    JsAppInstallerOperationCancel cancel;
    JsAppInstallerOperationCleanup cleanup;
    size_t data_size;
    size_t step_count;
    JsAppInstallerOperationStep steps[];
} JsAppInstallerOperationInterface;

struct JsAppInstallerOperation {
    const JsAppInstallerOperationInterface* interface;

    FuriThread* thread;
    FuriString* app_id;
    JsAppInstallerOperationCallback callback;
    void* callback_context;
    _Atomic bool is_cancel_requested;
};

void js_app_installer_operation_notify(
    const JsAppInstallerOperation* instance,
    const JsAppInstallerOperationEvent* event);
