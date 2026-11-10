#include "js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>

#include "operations/js_app_installer_operations.h"

#define TAG "JsAppInstallerOperation"

#define THREAD_STACK_SIZE (2 * 1024)

static int32_t js_app_operation_thread_callback(void* arg) {
    furi_assert(arg);
    JsAppInstallerOperation* instance = arg;

    const JsAppInstallerOperationInterface* interface = instance->interface;
    furi_assert(interface);
    furi_assert(interface->run);

    const JsAppInstallerError error = interface->run(instance);
    /* clang-format off */
    const JsAppInstallerOperationEvent event = {
        .type = JsAppInstallerOperationEventTypeFinished,
        .finished = {
            .error = error,
        },
    };
    /* clang-format on */
    js_app_installer_operation_emit(instance, &event);
    return 0;
}

JsAppInstallerOperation* js_app_installer_operation_alloc(const JsAppInstallerQueueItem* item) {
    JsAppInstallerOperation* instance = malloc(sizeof(JsAppInstallerOperation));

    const JsAppInstallerOperationType operation_type = item->type;
    furi_assert(operation_type < JsAppInstallerOperationTypeMax);

    const JsAppInstallerOperationInterface* interface =
        js_app_installer_operations[operation_type];
    furi_assert(interface);
    furi_assert(interface->alloc);

    instance->data = interface->alloc(item);
    instance->interface = interface;

    instance->thread =
        furi_thread_alloc_ex(TAG, THREAD_STACK_SIZE, js_app_operation_thread_callback, instance);
    instance->app_id = furi_string_alloc_set(item->app_id);

    return instance;
}

void js_app_installer_operation_free(JsAppInstallerOperation* instance) {
    furi_check(instance);

    furi_thread_join(instance->thread);
    furi_thread_free(instance->thread);
    furi_string_free(instance->app_id);

    const JsAppInstallerOperationInterface* interface = instance->interface;
    furi_assert(interface);
    furi_assert(interface->free);

    interface->free(instance->data);
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

void js_app_installer_operation_emit(
    const JsAppInstallerOperation* instance,
    const JsAppInstallerOperationEvent* event) {
    if(instance->callback) {
        instance->callback(event, instance->callback_context);
    }
}
