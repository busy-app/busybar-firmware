#include "lottie_service.h"

#include <furi.h>
#include <api_lock.h>
#include <thorvg_capi.h>

#include <lvgl.h>

#define TAG "LottieSrv"

#define COLOR_FORMAT    (LV_COLOR_FORMAT_ARGB8888)
#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(COLOR_FORMAT))

typedef enum {
    LottieServiceRequestTypeAlloc,
    LottieServiceRequestTypeFree,
    LottieServiceRequestTypeSetSource,
    LottieServiceRequestTypeOverrideSlot,
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
    const LottieServiceRequestType type;
    union {
        const LottieServiceAllocRequest alloc;
        const LottieServiceFreeRequest free;
        const LottieServiceSetSourceRequest set_source;
        const LottieServiceOverrideSlotRequest override_slot;
    } request;
    union {
        bool* boolean;
        LottieServiceTask** task;
    } result;
    FuriApiLock lock;
} LottieServiceMessage;

struct LottieServiceTask {
    LottieService* owner;
    FuriEventLoopTimer* timer;
    Tvg_Paint* tvg_paint;
    Tvg_Canvas* tvg_canvas;
    Tvg_Animation* tvg_anim;
    LottieServiceTaskCallback callback;
    void* callback_context;
    LottieServiceTaskInfo info;
    uint32_t num_frames;
    uint32_t current_frame;
};

struct LottieService {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
};

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
        task->callback(task->callback_context);
    }
}

static LottieServiceTask* lottie_service_task_alloc_internal(
    LottieService* instance,
    LottieServiceTaskCallback callback,
    void* context) {
    LottieServiceTask* task = malloc(sizeof(LottieServiceTask));

    task->owner = instance;
    task->timer = furi_event_loop_timer_alloc(
        instance->event_loop, lottie_service_task_update, FuriEventLoopTimerTypePeriodic, task);
    task->tvg_anim = tvg_animation_new();
    task->tvg_paint = tvg_animation_get_picture(task->tvg_anim);
    task->tvg_canvas = tvg_swcanvas_create();
    task->callback = callback;
    task->callback_context = context;

    return task;
}

static void lottie_service_task_free_internal(LottieServiceTask* task) {
    furi_event_loop_timer_free(task->timer);

    tvg_animation_del(task->tvg_anim);
    tvg_canvas_destroy(task->tvg_canvas);

    if(task->info.canvas_buf) {
        free(task->info.canvas_buf);
    }

    free(task);
}

static void
    lottie_service_task_init_draw_buffer(LottieServiceTask* task, int32_t width, int32_t height) {
    task->info.canvas_buf = realloc(task->info.canvas_buf, width * height * BYTES_PER_PIXEL);
    task->info.canvas_width = width;
    task->info.canvas_height = height;

    const int32_t stride = lv_draw_buf_width_to_stride(width, COLOR_FORMAT);

    tvg_swcanvas_set_target(
        task->tvg_canvas,
        task->info.canvas_buf,
        stride / 4,
        width,
        height,
        TVG_COLORSPACE_ARGB8888);
    tvg_canvas_push(task->tvg_canvas, task->tvg_paint);
}

static bool
    lottie_service_task_set_source_internal(LottieServiceTask* task, const char* file_path) {
    bool success = false;

    do {
        Tvg_Result res;

        res = tvg_picture_load(task->tvg_paint, file_path);
        if(res != TVG_RESULT_SUCCESS) {
            break;
        }

        float width, height;
        res = tvg_picture_get_size(task->tvg_paint, &width, &height);

        if(res != TVG_RESULT_SUCCESS) {
            break;
        }

        float num_frames;
        res = tvg_animation_get_total_frame(task->tvg_anim, &num_frames);

        if(res != TVG_RESULT_SUCCESS) {
            break;
        }

        task->num_frames = num_frames;

        float duration_s;
        res = tvg_animation_get_duration(task->tvg_anim, &duration_s);

        if(res != TVG_RESULT_SUCCESS) {
            break;
        }

        const float fps = num_frames / duration_s;

        lottie_service_task_init_draw_buffer(task, width, height);
        furi_event_loop_timer_start(task->timer, 1000.0F / fps);

        success = true;
    } while(false);

    return success;
}

static bool
    lottie_service_task_override_slot_internal(LottieServiceTask* task, const char* slot_str) {
    return tvg_lottie_animation_override(task->tvg_anim, slot_str) == TVG_RESULT_SUCCESS;
}

static void lottie_service_message_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);

    LottieService* instance = context;
    furi_assert(instance->message_queue == object);

    LottieServiceMessage msg;
    furi_check(furi_message_queue_get(instance->message_queue, &msg, 0) == FuriStatusOk);

    if(msg.type == LottieServiceRequestTypeAlloc) {
        const LottieServiceAllocRequest* req = &msg.request.alloc;
        *msg.result.task =
            lottie_service_task_alloc_internal(instance, req->callback, req->callback_context);
    } else if(msg.type == LottieServiceRequestTypeFree) {
        const LottieServiceFreeRequest* req = &msg.request.free;
        lottie_service_task_free_internal(req->task);
    } else if(msg.type == LottieServiceRequestTypeSetSource) {
        const LottieServiceSetSourceRequest* req = &msg.request.set_source;
        *msg.result.boolean = lottie_service_task_set_source_internal(req->task, req->file_path);
    } else if(msg.type == LottieServiceRequestTypeOverrideSlot) {
        const LottieServiceOverrideSlotRequest* req = &msg.request.override_slot;
        *msg.result.boolean = lottie_service_task_override_slot_internal(req->task, req->slot_str);
    } else {
        furi_crash();
    }

    api_lock_unlock(msg.lock);
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

    // TODO: Make it safer?
    *info = task->info;
    return true;
}

// Service thread

int32_t lottie_srv(void* arg) {
    UNUSED(arg);

    LottieService* instance = lottie_service_alloc();
    furi_event_loop_run(instance->event_loop);

    furi_crash();
}
