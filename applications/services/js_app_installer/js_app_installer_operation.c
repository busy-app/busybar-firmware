#include "js_app_installer_operation.h"

#include <core/check.h>
#include <core/thread.h>
#include <core/kernel.h>

#define TAG "JsAppInstallerOperation"

#define THREAD_STACK_SIZE (2 * 1024)

struct JsAppInstallerOperation {
    FuriThread* thread;
    JsAppInstallerOperationCallback callback;
    void* callback_context;
};

static void js_app_installer_operation_emit(
    const JsAppInstallerOperation* instance,
    const JsAppInstallerOperationEvent* event) {
    if(instance->callback) {
        instance->callback(event, instance->callback_context);
    }
}

static int32_t js_app_operation_thread_callback(void* arg) {
    furi_assert(arg);
    JsAppInstallerOperation* instance = arg;

    // TODO: Real implementation
    furi_delay_ms(1000);

    const JsAppInstallerOperationEvent event = {
        .type = JsAppInstallerOperationEventTypeFinished,
    };

    js_app_installer_operation_emit(instance, &event);

    return 0;
}

JsAppInstallerOperation* js_app_installer_operation_alloc(void) {
    JsAppInstallerOperation* instance = malloc(sizeof(JsAppInstallerOperation));

    instance->thread =
        furi_thread_alloc_ex(TAG, THREAD_STACK_SIZE, js_app_operation_thread_callback, instance);

    return instance;
}

void js_app_installer_operation_free(JsAppInstallerOperation* instance) {
    furi_check(instance);

    furi_thread_join(instance->thread);
    furi_thread_free(instance->thread);
    free(instance);
}

void js_app_installer_operation_set_callback(
    JsAppInstallerOperation* instance,
    JsAppInstallerOperationCallback callback,
    void* context) {
    furi_check(instance);
    furi_check(callback);

    instance->callback = callback;
    instance->callback_context = context;
}

void js_app_installer_operation_start(JsAppInstallerOperation* instance) {
    furi_check(instance);
    furi_thread_start(instance->thread);
}

void js_app_installer_operation_cancel(JsAppInstallerOperation* instance) {
    furi_check(instance);
    // TODO: Implementation
}
