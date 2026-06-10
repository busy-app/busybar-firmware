#include "device_info.h"
#include <m-array.h>
#include <furi_hal_info.h>

// =====
// Types
// =====

typedef struct {
    DeviceInfoCallback callback;
    void* context;
} DeviceInfoSegment;

ARRAY_DEF(DeviceInfoSegments, DeviceInfoSegment, M_POD_OPLIST)

struct DeviceInfo {
    FuriMutex* mutex;
    DeviceInfoSegments_t segments;
};

typedef struct {
    PropertyValueCallback print_callback;
    void* print_context;
    bool is_last_segment;
} DeviceInfoFilterContext;

// ===========
// Private API
// ===========

static DeviceInfo* device_info_alloc(void) {
    DeviceInfo* dev_info = malloc(sizeof(DeviceInfo));

    dev_info->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    DeviceInfoSegments_init(dev_info->segments);

    return dev_info;
}

static void
    device_info_filter(const char* key, const char* value, bool last_in_segment, void* context) {
    furi_check(context);
    DeviceInfoFilterContext* filter_context = context;

    bool report_as_last = filter_context->is_last_segment && last_in_segment;

    filter_context->print_callback(key, value, report_as_last, filter_context->print_context);
}

// ==========
// Public API
// ==========

void device_info_register_segment(DeviceInfo* dev_info, DeviceInfoCallback callback, void* context) {
    furi_check(dev_info);
    furi_check(callback);

    furi_check(furi_mutex_acquire(dev_info->mutex, FuriWaitForever) == FuriStatusOk);

    DeviceInfoSegment* segment = DeviceInfoSegments_push_new(dev_info->segments);
    segment->callback = callback;
    segment->context = context;

    furi_check(furi_mutex_release(dev_info->mutex) == FuriStatusOk);
}

void device_info_unregister_segment(DeviceInfo* dev_info, DeviceInfoCallback callback) {
    furi_check(dev_info);
    furi_check(callback);

    furi_check(furi_mutex_acquire(dev_info->mutex, FuriWaitForever) == FuriStatusOk);

    DeviceInfoSegments_it_ct it;
    for(DeviceInfoSegments_it(it, dev_info->segments); !DeviceInfoSegments_end_p(it);
        DeviceInfoSegments_next(it)) {
        const DeviceInfoSegment* segment = DeviceInfoSegments_cref(it);
        if(segment->callback == callback) DeviceInfoSegments_remove(dev_info->segments, it);
    }

    furi_check(furi_mutex_release(dev_info->mutex) == FuriStatusOk);
}

void device_info_query(
    DeviceInfo* dev_info,
    PropertyValueCallback print_callback,
    char separator,
    void* print_context) {
    furi_check(dev_info);
    furi_check(print_callback);

    furi_check(furi_mutex_acquire(dev_info->mutex, FuriWaitForever) == FuriStatusOk);

    DeviceInfoSegments_it_ct it;
    for(DeviceInfoSegments_it(it, dev_info->segments); !DeviceInfoSegments_end_p(it);
        DeviceInfoSegments_next(it)) {
        const DeviceInfoSegment* segment = DeviceInfoSegments_cref(it);

        DeviceInfoFilterContext filter_ctx = {
            .print_callback = print_callback,
            .print_context = print_context,
            .is_last_segment = DeviceInfoSegments_last_p(it),
        };

        segment->callback(device_info_filter, separator, segment->context, &filter_ctx);
    }

    furi_check(furi_mutex_release(dev_info->mutex) == FuriStatusOk);
}

// ============
// Startup hook
// ============

static void furi_hal_adapter_for_device_info(
    PropertyValueCallback print_callback,
    char separator,
    void* info_context,
    void* print_context) {
    UNUSED(info_context);
    furi_hal_info_get(print_callback, separator, print_context);
}

void device_info_on_system_start(void) {
    DeviceInfo* dev_info = device_info_alloc();

    device_info_register_segment(dev_info, furi_hal_adapter_for_device_info, NULL);

    furi_record_create(RECORD_DEVICE_INFO, dev_info);
}
