#include "lottie_service.h"

#include <furi.h>
#include <api_lock.h>
#include <thorvg_capi.h>

#include <lvgl.h>

#define TAG "LottieSrv"

#define LV_COLOR_FORMAT  (LV_COLOR_FORMAT_ARGB8888)
#define TVG_COLOR_FORMAT (TVG_COLORSPACE_ARGB8888)
#define BYTES_PER_PIXEL  (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT))

typedef enum {
    LottieServiceRequestTypeAlloc,
    LottieServiceRequestTypeFree,
    LottieServiceRequestTypeSetSource,
    LottieServiceRequestTypeOverrideSlot,
    LottieServiceRequestTypeGetInfo,
    LottieServiceRequestTypeStart,

    LottieServiceRequestTypeMax,
} LottieServiceRequestType;

typedef struct {
    LottieServiceTaskCallback callback;
    void* callback_context;
} LottieServiceAllocRequest;

typedef struct {
    LottieServiceTask* task;
} LottieServiceFreeRequest;

typedef struct {
    LottieServiceTask* task;
    const char* file_path;
} LottieServiceSetSourceRequest;

typedef struct {
    LottieServiceTask* task;
    const char* slot_str;
} LottieServiceOverrideSlotRequest;

typedef struct {
    const LottieServiceTask* task;
    LottieServiceTaskInfo* task_info;
} LottieServiceGetInfoRequest;

typedef struct {
    LottieServiceTask* task;
} LottieServiceStartRequest;

typedef union {
    LottieServiceAllocRequest alloc;
    LottieServiceFreeRequest free;
    LottieServiceSetSourceRequest set_source;
    LottieServiceOverrideSlotRequest override_slot;
    LottieServiceGetInfoRequest get_info;
    LottieServiceStartRequest start;
} LottieServiceRequest;

typedef union {
    bool* boolean;
    LottieServiceTask** task;
} LottieServiceResult;

typedef struct {
    const LottieServiceRequestType type;
    const LottieServiceRequest request;
    const LottieServiceResult result;
    FuriApiLock lock;
} LottieServiceMessage;

struct LottieServiceTask {
    LottieService* owner;
    FuriEventLoopTimer* timer;
    uint32_t* canvas_buf;
    Tvg_Paint* tvg_paint;
    Tvg_Canvas* tvg_canvas;
    Tvg_Animation* tvg_anim;
    LottieServiceTaskCallback callback;
    void* callback_context;
    LottieServiceTaskInfo info;
    uint32_t fps;
    uint32_t num_frames;
    uint32_t current_frame;
};

struct LottieService {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
};

typedef void (*const LottieServiceMessageHandler)(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result);

static const LottieServiceMessageHandler lottie_message_handlers[LottieServiceRequestTypeMax];

static void lottie_service_task_update(void* context) {
    furi_assert(context);
    LottieServiceTask* task = context;

    tvg_animation_set_frame(task->tvg_anim, task->current_frame);
    tvg_canvas_update(task->tvg_canvas);
    tvg_canvas_draw(task->tvg_canvas);
    tvg_canvas_sync(task->tvg_canvas);

    if(++task->current_frame >= task->num_frames) {
        task->current_frame = 0;
    }

    if(task->callback) {
        task->callback(task->canvas_buf, task->callback_context);
    }
}

static void lottie_service_task_alloc_handler(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result) {
    LottieServiceTask* task = malloc(sizeof(LottieServiceTask));

    task->owner = instance;
    task->timer = furi_event_loop_timer_alloc(
        instance->event_loop, lottie_service_task_update, FuriEventLoopTimerTypePeriodic, task);
    task->tvg_anim = tvg_animation_new();
    task->tvg_paint = tvg_animation_get_picture(task->tvg_anim);
    task->tvg_canvas = tvg_swcanvas_create();
    task->callback = request->alloc.callback;
    task->callback_context = request->alloc.callback_context;

    *result->task = task;
}

static void lottie_service_task_free_handler(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result) {
    UNUSED(instance);
    UNUSED(result);

    LottieServiceTask* task = request->free.task;

    furi_event_loop_timer_free(task->timer);

    tvg_animation_del(task->tvg_anim);
    tvg_canvas_destroy(task->tvg_canvas);

    if(task->canvas_buf) {
        free(task->canvas_buf);
    }

    free(task);
}

static void
    lottie_service_task_init_draw_buffer(LottieServiceTask* task, int32_t width, int32_t height) {
    task->info.canvas_width = width;
    task->info.canvas_height = height;
    task->info.canvas_buf_size = width * height * BYTES_PER_PIXEL;

    task->canvas_buf = realloc(task->canvas_buf, task->info.canvas_buf_size);

    const int32_t stride = lv_draw_buf_width_to_stride(width, LV_COLOR_FORMAT);

    tvg_swcanvas_set_target(
        task->tvg_canvas, task->canvas_buf, stride / 4, width, height, TVG_COLOR_FORMAT);
    tvg_canvas_push(task->tvg_canvas, task->tvg_paint);
}

static void lottie_service_task_set_source_handler(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result) {
    UNUSED(instance);

    bool success = false;

    LottieServiceTask* task = request->set_source.task;

    do {
        Tvg_Result res;

        res = tvg_picture_load(task->tvg_paint, request->set_source.file_path);
        if(res != TVG_RESULT_SUCCESS) break;

        float width, height;
        res = tvg_picture_get_size(task->tvg_paint, &width, &height);
        if(res != TVG_RESULT_SUCCESS) break;

        float num_frames;
        res = tvg_animation_get_total_frame(task->tvg_anim, &num_frames);
        if(res != TVG_RESULT_SUCCESS) break;

        task->num_frames = num_frames;

        float duration_s;
        res = tvg_animation_get_duration(task->tvg_anim, &duration_s);
        if(res != TVG_RESULT_SUCCESS) break;

        task->fps = num_frames / duration_s;

        lottie_service_task_init_draw_buffer(task, width, height);

        success = true;
    } while(false);

    *result->boolean = success;
}

static void lottie_service_task_override_slot_handler(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result) {
    UNUSED(instance);

    LottieServiceTask* task = request->override_slot.task;
    *result->boolean = tvg_lottie_animation_override(
                           task->tvg_anim, request->override_slot.slot_str) == TVG_RESULT_SUCCESS;
}

static void lottie_service_task_get_info_handler(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result) {
    UNUSED(instance);

    bool success = false;

    const LottieServiceTask* task = request->get_info.task;

    if(task->canvas_buf) {
        *request->get_info.task_info = task->info;
        success = true;
    }

    *result->boolean = success;
}

static void lottie_service_task_start_handler(
    LottieService* instance,
    const LottieServiceRequest* request,
    const LottieServiceResult* result) {
    UNUSED(instance);

    bool success = false;

    LottieServiceTask* task = request->start.task;

    if(task->canvas_buf) {
        if(task->num_frames > 1) {
            furi_event_loop_timer_start(task->timer, 1000.0F / task->fps);
        }
        furi_event_loop_pend_callback(instance->event_loop, lottie_service_task_update, task);
        success = true;
    }

    *result->boolean = success;
}

static void lottie_service_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    LottieService* instance = context;
    furi_assert(instance->message_queue == object);

    LottieServiceMessage message;
    furi_check(furi_message_queue_get(instance->message_queue, &message, 0) == FuriStatusOk);

    const LottieServiceRequestType request_type = message.type;
    furi_assert(request_type < LottieServiceRequestTypeMax);

    lottie_message_handlers[request_type](instance, &message.request, &message.result);
    api_lock_unlock(message.lock);
}

static LottieService* lottie_service_alloc(void) {
    LottieService* instance = malloc(sizeof(LottieService));

    instance->event_loop = furi_event_loop_alloc();
    instance->message_queue = furi_message_queue_alloc(1, sizeof(LottieServiceMessage));

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->message_queue,
        FuriEventLoopEventIn,
        lottie_service_message_queue_callback,
        instance);

    furi_record_create(RECORD_LOTTIE, instance);
    return instance;
}

static void lottie_service_send_message(LottieService* instance, LottieServiceMessage* message) {
    message->lock = api_lock_alloc_locked();

    furi_check(
        furi_message_queue_put(instance->message_queue, message, FuriWaitForever) == FuriStatusOk);

    api_lock_wait_unlock_and_free(message->lock);
}

// Public API

LottieServiceTask* lottie_service_task_alloc(
    LottieService* instance,
    LottieServiceTaskCallback callback,
    void* context) {
    furi_check(instance);

    LottieServiceTask* task;

    LottieServiceMessage message = {
        .type = LottieServiceRequestTypeAlloc,
        .request.alloc =
            {
                .callback = callback,
                .callback_context = context,
            },
        .result.task = &task,
    };

    lottie_service_send_message(instance, &message);
    return task;
}

void lottie_service_task_free(LottieServiceTask* task) {
    furi_check(task);
    furi_check(task->owner);

    LottieServiceMessage message = {
        .type = LottieServiceRequestTypeFree,
        .request.free =
            {
                .task = task,
            },
    };

    lottie_service_send_message(task->owner, &message);
}

bool lottie_service_task_set_source(LottieServiceTask* task, const char* file_path) {
    furi_check(task);
    furi_check(file_path);

    bool result;

    LottieServiceMessage message = {
        .type = LottieServiceRequestTypeSetSource,
        .request.set_source =
            {
                .task = task,
                .file_path = file_path,
            },
        .result.boolean = &result,
    };

    lottie_service_send_message(task->owner, &message);
    return result;
}

bool lottie_service_task_override_slot(LottieServiceTask* task, const char* slot_str) {
    furi_check(task);
    furi_check(slot_str);

    bool result;

    LottieServiceMessage message = {
        .type = LottieServiceRequestTypeOverrideSlot,
        .request.set_source =
            {
                .task = task,
                .file_path = slot_str,
            },
        .result.boolean = &result,
    };

    lottie_service_send_message(task->owner, &message);
    return result;
}

bool lottie_service_task_get_info(const LottieServiceTask* task, LottieServiceTaskInfo* info) {
    furi_check(task);
    furi_check(info);

    bool result;

    LottieServiceMessage message = {
        .type = LottieServiceRequestTypeGetInfo,
        .request.get_info =
            {
                .task = task,
                .task_info = info,
            },
        .result.boolean = &result,
    };

    lottie_service_send_message(task->owner, &message);
    return result;
}

bool lottie_service_task_start(LottieServiceTask* task) {
    furi_check(task);

    bool result;

    LottieServiceMessage message = {
        .type = LottieServiceRequestTypeStart,
        .request.start =
            {
                .task = task,
            },
        .result.boolean = &result,
    };

    lottie_service_send_message(task->owner, &message);
    return result;
}

// Service thread

int32_t lottie_srv(void* arg) {
    UNUSED(arg);

    LottieService* instance = lottie_service_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}

// Message handlers

static const LottieServiceMessageHandler lottie_message_handlers[LottieServiceRequestTypeMax] = {
    [LottieServiceRequestTypeAlloc] = lottie_service_task_alloc_handler,
    [LottieServiceRequestTypeFree] = lottie_service_task_free_handler,
    [LottieServiceRequestTypeSetSource] = lottie_service_task_set_source_handler,
    [LottieServiceRequestTypeOverrideSlot] = lottie_service_task_override_slot_handler,
    [LottieServiceRequestTypeGetInfo] = lottie_service_task_get_info_handler,
    [LottieServiceRequestTypeStart] = lottie_service_task_start_handler,
};
