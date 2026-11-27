#include "js_app_installer_queue.h"

#include <stdarg.h>
#include <core/check.h>

JsAppInstallerQueueItem*
    js_app_installer_queue_item_alloc(const char* app_id, JsAppInstallerOperationType type, ...) {
    furi_assert(app_id);
    furi_assert(type < JsAppInstallerOperationTypeMax);

    JsAppInstallerQueueItem* instance = malloc(sizeof(JsAppInstallerQueueItem));
    JsAppInstallerOperationQueue_init_field(instance);

    instance->type = type;
    instance->app_id = furi_string_alloc_set(app_id);

    if(type == JsAppInstallerOperationTypeInstall) {
        va_list args;
        va_start(args, type);

        const JsAppInstallerSourceInfo* source_info = va_arg(args, JsAppInstallerSourceInfo*);
        furi_check(source_info->url);

        instance->url = furi_string_alloc_set(source_info->url);
        instance->app_version = source_info->version;

        va_end(args);
    }

    return instance;
}

void js_app_installer_queue_item_free(JsAppInstallerQueueItem* instance) {
    furi_assert(instance);

    furi_string_free(instance->app_id);

    if(instance->url != NULL) {
        furi_assert(instance->type == JsAppInstallerOperationTypeInstall);
        furi_string_free(instance->url);
    }

    free(instance);
}

void js_app_installer_queue_item_get_partial_info(
    const JsAppInstallerQueueItem* instance,
    JsAppInstallerOperationInfo* info) {
    furi_assert(instance);
    furi_assert(info);
    /* clang-format off */
    const JsAppInstallerOperationInfo operation_info = {
        .item_id = instance->id,
        .app_info = {
            .app_id = furi_string_get_cstr(instance->app_id),
            .version = instance->app_version,
        },
        .type = instance->type,
        .status = JsAppInstallerOperationStatusPending,
        .detail = {0},
    };
    /* clang-format on */
    *info = operation_info;
}
