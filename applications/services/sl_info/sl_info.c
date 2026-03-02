#include "sl_info.h"

#include <m-array.h>

#include <intercom/intercom.h>

#include "sl_info_common_i.h"

#define TAG "SlInfo"

typedef struct {
    const char* key;
    const char* value;
} SlInfoItem;

// NOTE: Not using a standard dict because we want to preserve the order of items.
ARRAY_DEF(SlInfoItemVec, SlInfoItem, M_POD_OPLIST)

struct SlInfo {
    SlInfoItemVec_t items;
    _Atomic bool is_ready;
};

// Implementation

static void sl_info_add_item(SlInfo* instance, const SlInfoIntercomFrame* frame) {
    SlInfoItem* item = SlInfoItemVec_push_new(instance->items);
    item->key = strdup(frame->key);
    item->value = strdup(frame->value);
}

static const char* sl_info_find_value(const SlInfo* instance, const char* key) {
    const char* value = NULL;

    SlInfoItemVec_it_ct it;
    for(SlInfoItemVec_it(it, instance->items); !SlInfoItemVec_end_p(it); SlInfoItemVec_next(it)) {
        const SlInfoItem* item = SlInfoItemVec_cref(it);
        if(strcmp(item->key, key) == 0) {
            value = item->value;
            break;
        }
    }

    return value;
}

static void sl_info_output_values(
    const SlInfo* instance,
    PropertyValueCallback value_callback,
    void* context) {
    SlInfoItemVec_it_ct it;
    for(SlInfoItemVec_it(it, instance->items); !SlInfoItemVec_end_p(it); SlInfoItemVec_next(it)) {
        const SlInfoItem* item = SlInfoItemVec_cref(it);
        const bool is_last = SlInfoItemVec_last_p(it);
        value_callback(item->key, item->value, is_last, context);
    }
}

static void sl_info_intercom_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(context);
    SlInfo* instance = context;

    furi_check(!instance->is_ready);
    furi_check(data_size == sizeof(SlInfoIntercomFrame));

    const SlInfoIntercomFrame* frame = data;
    sl_info_add_item(instance, frame);

    if(frame->is_last) {
        FURI_LOG_I(TAG, "Data ready");
        instance->is_ready = true;
    }
}

static void sl_info_wait_for_data(SlInfo* instance) {
    FURI_LOG_D(TAG, "Waiting for data...");

    Intercom* intercom = furi_record_open(RECORD_INTERCOM);
    IntercomChannel* info_channel = intercom_channel_open(
        intercom, IntercomChannelIdSlInfo, sl_info_intercom_rx_callback, instance);
    // Not sending anything to the intercom channel
    UNUSED(info_channel);
}

static SlInfo* sl_info_alloc(void) {
    SlInfo* instance = malloc(sizeof(SlInfo));
    SlInfoItemVec_init(instance->items);

    furi_record_create(RECORD_SL_INFO, instance);
    return instance;
}

// Public API
SlInfoStatus sl_info_get_value(const SlInfo* instance, const char* key, const char** value) {
    furi_check(instance);
    furi_check(key);
    furi_check(value);

    SlInfoStatus status;

    do {
        if(!instance->is_ready) {
            status = SlInfoStatusNotReady;
            break;
        }

        const char* found_value = sl_info_find_value(instance, key);

        if(found_value == NULL) {
            status = SlInfoStatusNotFound;
            break;
        }

        *value = found_value;
        status = SlInfoStatusOk;

    } while(false);

    return status;
}

SlInfoStatus sl_info_get_values(
    const SlInfo* instance,
    PropertyValueCallback value_callback,
    void* context) {
    furi_check(instance);
    furi_check(value_callback);

    SlInfoStatus status;

    do {
        if(!instance->is_ready) {
            status = SlInfoStatusNotReady;
            break;
        }

        sl_info_output_values(instance, value_callback, context);
        status = SlInfoStatusOk;

    } while(false);

    return status;
}

// Startup hook thread
void sl_info_on_system_start(void) {
    SlInfo* instance = sl_info_alloc();
    sl_info_wait_for_data(instance);
}
