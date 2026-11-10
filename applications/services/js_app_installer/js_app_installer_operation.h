#pragma once

#include "js_app_installer_common.h"
#include "js_app_installer_queue.h"

typedef struct JsAppInstallerOperation JsAppInstallerOperation;

typedef enum {
    JsAppInstallerOperationEventTypeProgress,
    JsAppInstallerOperationEventTypeFinished,
} JsAppInstallerOperationEventType;

typedef struct {
    JsAppInstallerOperationDetail detail;
} JsAppInstallerOperationEventProgress;

typedef struct {
    JsAppInstallerOperationEventType type;
    union {
        JsAppInstallerOperationEventProgress progress;
    };
} JsAppInstallerOperationEvent;

typedef void (
    *JsAppInstallerOperationCallback)(const JsAppInstallerOperationEvent* event, void* context);

JsAppInstallerOperation* js_app_installer_operation_alloc(const JsAppInstallerQueueItem* item);

void js_app_installer_operation_free(JsAppInstallerOperation* instance);

void js_app_installer_operation_set_callback(
    JsAppInstallerOperation* instance,
    JsAppInstallerOperationCallback callback,
    void* context);

void js_app_installer_operation_start(JsAppInstallerOperation* instance);

void js_app_installer_operation_cancel(JsAppInstallerOperation* instance);
