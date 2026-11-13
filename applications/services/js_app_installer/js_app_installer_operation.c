#include "js_app_installer_operation_i.h"

#include <core/check.h>
#include <core/kernel.h>
#include <core/log.h>

#include <js_app_launcher/js_app_launcher.h>

#include "operations/js_app_installer_operations.h"

#define TAG "JsAppInstallerOperation"

#define THREAD_STACK_SIZE (2 * 1024)

static void js_app_installer_operation_notify_step_started(
    JsAppInstallerOperation* instance,
    const char* step_name) {
    /* clang-format off */
    const JsAppInstallerOperationEvent event = {
        .type = JsAppInstallerOperationEventTypeProgress,
        .progress.detail = {
            .step = step_name,
            .has_progress = false,
        },
    };
    /* clang-format on */
    js_app_installer_operation_notify(instance, &event);
}

static void js_app_installer_operation_notify_finished(
    JsAppInstallerOperation* instance,
    JsAppInstallerError error) {
    const JsAppInstallerOperationEvent event = {
        .type = JsAppInstallerOperationEventTypeFinished,
        .finished.error = error,
    };

    js_app_installer_operation_notify(instance, &event);
}

static JsAppInstallerError
    js_app_installer_operation_can_proceed(const JsAppInstallerOperation* instance) {
    JsAppInstallerError error;
    FuriString* app_id = furi_string_alloc();

    do {
        const JsAppLauncherStatus status = js_app_launcher_get_running_app_id(app_id);
        if(status != JsAppLauncherStatusNotRunning) {
            if(status != JsAppLauncherStatusOk) {
                error = JsAppInstallerErrorInternal;
                break;
            }

            if(furi_string_equal(app_id, instance->app_id)) {
                error = JsAppInstallerErrorAppRunning;
                break;
            }
        }

        error = JsAppInstallerErrorNone;
    } while(false);

    furi_string_free(app_id);
    return error;
}

static JsAppInstallerError
    js_app_installer_operation_run_steps(JsAppInstallerOperation* instance) {
    JsAppInstallerError error = JsAppInstallerErrorNone;

    const JsAppInstallerOperationInterface* interface = instance->interface;
    furi_assert(interface);

    for(size_t i = 0; i < interface->step_count; ++i) {
        const JsAppInstallerOperationStep* step = &interface->steps[i];
        const char* step_name = step->name;
        furi_assert(step_name);

        if(instance->is_cancel_requested) {
            FURI_LOG_I(TAG, "Cancelled on step %zu", i);
            error = JsAppInstallerErrorCancelled;
            break;
        }

        js_app_installer_operation_notify_step_started(instance, step_name);

        furi_assert(step->handler);
        error = step->handler(instance);

        if(error != JsAppInstallerErrorNone) {
            FURI_LOG_E(TAG, "Step %s failed: 0x%X", step_name, error);
            break;
        }
    }

    furi_assert(interface->cleanup);
    interface->cleanup(instance);

    return error;
}

static int32_t js_app_installer_operation_thread_callback(void* arg) {
    furi_assert(arg);
    JsAppInstallerOperation* instance = arg;
    JsAppInstallerError error;

    do {
        error = js_app_installer_operation_can_proceed(instance);
        if(error != JsAppInstallerErrorNone) {
            break;
        }

        error = js_app_installer_operation_run_steps(instance);
    } while(false);

    js_app_installer_operation_notify_finished(instance, error);
    return 0;
}

JsAppInstallerOperation* js_app_installer_operation_alloc(const JsAppInstallerQueueItem* item) {
    const JsAppInstallerOperationType operation_type = item->type;
    furi_assert(operation_type < JsAppInstallerOperationTypeMax);

    const JsAppInstallerOperationInterface* interface =
        js_app_installer_operations[operation_type];
    furi_assert(interface);

    const size_t data_size = interface->data_size;
    furi_assert(data_size >= sizeof(JsAppInstallerOperation));

    JsAppInstallerOperation* instance = malloc(data_size);

    instance->interface = interface;
    instance->thread = furi_thread_alloc_ex(
        TAG, THREAD_STACK_SIZE, js_app_installer_operation_thread_callback, instance);
    instance->app_id = furi_string_alloc_set(item->app_id);

    furi_assert(interface->init);
    interface->init(instance, item);

    return instance;
}

void js_app_installer_operation_free(JsAppInstallerOperation* instance) {
    furi_check(instance);
    furi_thread_join(instance->thread);

    const JsAppInstallerOperationInterface* interface = instance->interface;
    furi_assert(interface);

    furi_assert(interface->deinit);
    interface->deinit(instance);

    furi_thread_free(instance->thread);
    furi_string_free(instance->app_id);
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

    const JsAppInstallerOperationInterface* interface = instance->interface;
    furi_assert(interface);
    furi_assert(interface->cancel);

    instance->is_cancel_requested = true;
    interface->cancel(instance);
}

void js_app_installer_operation_notify(
    const JsAppInstallerOperation* instance,
    const JsAppInstallerOperationEvent* event) {
    if(instance->callback) {
        instance->callback(event, instance->callback_context);
    }
}
