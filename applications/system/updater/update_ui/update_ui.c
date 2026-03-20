#include "update_ui_i.h"
#include "scenes/scenes.h"

#define THREAD_NAME            "UpdateUi"
#define THREAD_STACK_SIZE      (2 * 1024)
#define THREAD_EXIT_TIMEOUT_MS 3000

#define INPUT_QUEUE_CAPACITY   8
#define INPUT_QUEUE_TIMEOUT_MS 3000

#define EVENT_QUEUE_CAPACITY   8
#define EVENT_QUEUE_TIMEOUT_MS 3000

#define SCENE_WINDOW_BACKGROUND_COLOR ((Color)COLOR_MAKE_RGB(0x00, 0x00, 0x00))

typedef struct {
    FuriSemaphore* exit_thread_semaphore;
    FuriSemaphore* join_thread_semaphore;

    UpdateUiSceneIdx startup_scene_idx;

    bool is_updater_session_active;
} UpdateUiStartup;

/* thread implementation */

static void update_ui_exit_thread_semaphore_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    UpdateUi* instance = context;

    furi_event_loop_stop(instance->event_loop);
}

static bool update_ui_input_callback(const InputEvent* event, void* context) {
    UpdateUi* instance = context;

    FuriStatus input_queue_status = furi_message_queue_put(
        instance->input_queue, event, furi_ms_to_ticks(INPUT_QUEUE_TIMEOUT_MS));

    if(input_queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to put an item into input queue.");
    }

    return true;
}

static void update_ui_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    UpdateUi* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort && event.key == InputKeyBack) {
            scene_manager_handle_back_event(instance->scene_manager);
        }
    }
}

static void update_ui_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    UpdateUi* instance = context;

    uint32_t event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        scene_manager_handle_custom_event(instance->scene_manager, event);
    }
}

static void update_ui_setup(UpdateUi* instance, const UpdateUiStartup* startup_instance) {
    instance->gui = furi_record_open(RECORD_GUI);
    instance->updater = furi_record_open(RECORD_UPDATER);

    instance->scene_manager =
        scene_manager_alloc(update_ui_internal_scenes, UpdateUiSceneIdxsCount, instance);

    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(INPUT_QUEUE_CAPACITY, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(EVENT_QUEUE_CAPACITY, sizeof(uint32_t));

    instance->failure_preset.front_text = furi_string_alloc();
    instance->failure_preset.back_primary_text = furi_string_alloc();
    instance->failure_preset.back_detail_text = furi_string_alloc();

    with_gui(instance->gui, {
        GuiLayer* system_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        gui_layer_add_input_callback(system_layer, update_ui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(system_layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);
        widget_set_background_color(instance->front_scene_window, SCENE_WINDOW_BACKGROUND_COLOR);

        Widget* back_root = gui_layer_get_root_widget(system_layer, GuiDisplayIdBack);
        instance->back_scene_window = widget_alloc(back_root);
        widget_set_background_color(instance->back_scene_window, SCENE_WINDOW_BACKGROUND_COLOR);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        update_ui_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        update_ui_event_queue_callback,
        instance);

    furi_event_loop_subscribe_semaphore(
        instance->event_loop,
        startup_instance->exit_thread_semaphore,
        FuriEventLoopEventIn,
        update_ui_exit_thread_semaphore_callback,
        instance);

    /* reset exit thread semaphore's state in case it was released */
    furi_semaphore_acquire(startup_instance->exit_thread_semaphore, 0);

    scene_manager_next_scene(instance->scene_manager, startup_instance->startup_scene_idx);
}

static void update_ui_cleanup(UpdateUi* instance, const UpdateUiStartup* startup_instance) {
    furi_event_loop_unsubscribe(instance->event_loop, startup_instance->exit_thread_semaphore);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);

    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* system_layer = gui_get_layer(instance->gui, GuiLayerIdSystem);
        gui_layer_remove_input_callback(system_layer, update_ui_input_callback);

        widget_free(instance->front_scene_window);
        widget_free(instance->back_scene_window);
    });

    furi_string_free(instance->failure_preset.front_text);
    furi_string_free(instance->failure_preset.back_primary_text);
    furi_string_free(instance->failure_preset.back_detail_text);

    furi_message_queue_free(instance->event_queue);
    furi_message_queue_free(instance->input_queue);

    furi_event_loop_free(instance->event_loop);

    furi_record_close(RECORD_UPDATER);
    furi_record_close(RECORD_GUI);
}

static int32_t update_ui_thread_callback(void* context) {
    FURI_LOG_T(TAG, "UI thread is running...");

    UpdateUi* instance = malloc(sizeof(*instance));
    update_ui_setup(instance, context);

    furi_event_loop_run(instance->event_loop);

    update_ui_cleanup(instance, context);
    free(instance);

    FURI_LOG_T(TAG, "UI thread is exiting...");

    return 0;
}

void update_ui_internal_fire_event(UpdateUi* instance, uint32_t event) {
    FuriStatus event_queue_status = furi_message_queue_put(
        instance->event_queue, &event, furi_ms_to_ticks(EVENT_QUEUE_TIMEOUT_MS));

    if(event_queue_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to put an item into event queue.");
    }
}

/* startup implementation */

static void
    update_ui_thread_state_callback(FuriThread* thread, FuriThreadState state, void* context) {
    UpdateUiStartup* startup_instance = context;

    if(state != FuriThreadStateStopped) return;

    furi_thread_free(thread);
    furi_semaphore_release(startup_instance->join_thread_semaphore);
}

static void update_ui_startup_state_callback(const void* item, void* context) {
    const UpdaterUpdateState* state = item;
    UpdateUiStartup* startup_instance = context;

    switch(state->event) {
    case UpdaterUpdateEventSessionStop:
        startup_instance->is_updater_session_active = false;
        return;

    case UpdaterUpdateEventActionBegin:
        break;

    default:
        return;
    }

    if(startup_instance->is_updater_session_active) return;

    UpdateUiSceneIdx startup_scene_idx;
    switch(state->action) {
    case UpdaterUpdateActionDownload:
        startup_scene_idx = UpdateUiSceneIdxDownload;
        break;

    case UpdaterUpdateActionShaVerification:
    /* fall-through */
    case UpdaterUpdateActionUnpack:
        startup_scene_idx = UpdateUiSceneIdxPrepare;
        break;

    default:
        return;
    }

    FURI_LOG_T(TAG, "Preparing to spawn UI thread...");

    furi_semaphore_release(startup_instance->exit_thread_semaphore);
    FuriStatus join_thread_semaphore_status = furi_semaphore_acquire(
        startup_instance->join_thread_semaphore, furi_ms_to_ticks(THREAD_EXIT_TIMEOUT_MS));

    if(join_thread_semaphore_status != FuriStatusOk) {
        FURI_LOG_E(TAG, "Failed to join running UI thread.");
        return;
    }

    startup_instance->is_updater_session_active = true;
    startup_instance->startup_scene_idx = startup_scene_idx;

    FURI_LOG_T(TAG, "Spawning UI thread...");

    FuriThread* thread = furi_thread_alloc_ex(
        THREAD_NAME, THREAD_STACK_SIZE, update_ui_thread_callback, startup_instance);

    furi_thread_set_state_callback(thread, update_ui_thread_state_callback);
    furi_thread_set_state_context(thread, startup_instance);

    furi_thread_start(thread);
}

void update_ui_startup(void) {
    UpdateUiStartup* startup_instance = malloc(sizeof(*startup_instance));
    startup_instance->exit_thread_semaphore = furi_semaphore_alloc(1, 0);
    startup_instance->join_thread_semaphore = furi_semaphore_alloc(1, 1);
    startup_instance->is_updater_session_active = false;

    /* persistent state subscription - don't close the record */
    Updater* updater = furi_record_open(RECORD_UPDATER);
    furi_state_subscribe(
        updater_get_update_state(updater), update_ui_startup_state_callback, startup_instance);
}
