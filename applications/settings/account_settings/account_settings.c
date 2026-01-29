#include "account_settings.h"
#include <settings_helpers/app_desc.h>
#include <settings_helpers/gui_params.h>
#include <wifi/wifi.h>

typedef struct {
    AppEvent event;
    union {
        char link_pin[ACCOUNT_MODEL_LINK_PIN_LEN + 1];
    };
} AccountSettingsEvent;

static bool account_settings_thread_signal_callback(uint32_t signal, void* arg, void* context) {
    UNUSED(arg);

    AccountSettings* instance = context;

    switch(signal) {
    case FuriSignalExit:
        furi_event_loop_stop(instance->event_loop);
        return true;

    case FuriSignalAboutToExit:
        account_settings_send_custom_event(instance, AppEventAboutToExit);
        return true;

    default:
        return false;
    }
}

static void account_settings_input_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    AccountSettings* instance = context;

    InputEvent event;
    while(furi_message_queue_get(instance->input_queue, &event, 0) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                scene_manager_handle_back_event(instance->scene_manager);
            }
        }
    }
}

static void account_settings_event_queue_callback(FuriEventLoopObject* object, void* context) {
    UNUSED(object);

    furi_assert(context);

    AccountSettings* instance = context;

    AccountSettingsEvent event;
    while(furi_message_queue_get(instance->event_queue, &event, 0) == FuriStatusOk) {
        if(event.event == AppEventAccountLinkPin) {
            strncpy(instance->link_pin, event.link_pin, ACCOUNT_MODEL_LINK_PIN_LEN);
        }
        scene_manager_handle_custom_event(instance->scene_manager, event.event);
    }
}

static bool account_settings_gui_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            consumed = true;
        }
    }

    if(consumed) {
        furi_check(
            furi_message_queue_put(instance->input_queue, event, FuriWaitForever) == FuriStatusOk);
    }

    return consumed;
}

static void account_settings_account_event_callback(
    AccountModelEvent event,
    const char* pin,
    void* context) {
    AccountSettings* instance = context;

    AccountSettingsEvent evt;
    switch(event) {
    case AccountModelEventStateChange:
        evt.event = AppEventAccountStateChange;
        break;
    case AccountModelEventPinGot:
        evt.event = AppEventAccountLinkPin;
        furi_assert(pin);
        strncpy(evt.link_pin, pin, ACCOUNT_MODEL_LINK_PIN_LEN);
        break;
    case AccountModelEventPinTimeout:
        evt.event = AppEventAccountLinkPinTimeout;
        break;
    case AccountModelEventLinkDone:
        evt.event = AppEventAccountLinkDone;
        break;
    case AccountModelEventUnlinked:
        evt.event = AppEventAccountUnlinked;
        break;
    }

    furi_check(
        furi_message_queue_put(instance->event_queue, &evt, FuriWaitForever) == FuriStatusOk);
}

static AccountSettings* account_settings_alloc() {
    AccountSettings* instance = malloc(sizeof(AccountSettings));
    instance->event_loop = furi_event_loop_alloc();
    instance->input_queue = furi_message_queue_alloc(4, sizeof(InputEvent));
    instance->event_queue = furi_message_queue_alloc(4, sizeof(AccountSettingsEvent));
    instance->scene_manager =
        scene_manager_alloc(account_settings_scenes, COUNT_OF(account_settings_scenes), instance);

    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);
    instance->front_display = furi_record_open(RECORD_FRONT_DISPLAY);
    instance->back_display = furi_record_open(RECORD_BACK_DISPLAY);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(layer, account_settings_gui_input_callback, instance);

        Widget* front_root = gui_layer_get_root_widget(layer, GuiDisplayIdFront);
        instance->front_scene_window = widget_alloc(front_root);

        Widget* back_root = gui_layer_get_root_widget(layer, GuiDisplayIdBack);
        instance->back_container = flex_layout_alloc(back_root, FlexLayoutTypeColumn);

        instance->back_nav_bar = nav_bar_alloc(flex_layout_get_base(instance->back_container));
        nav_bar_set_header_image(instance->back_nav_bar, SETTINGS_ICON_BACK);
        nav_bar_push_location(instance->back_nav_bar, "ACCOUNT");
        widget_set_height(nav_bar_get_base(instance->back_nav_bar), SETTINGS_NAV_BAR_HEIGHT);
        widget_set_padding(nav_bar_get_base(instance->back_nav_bar), 2, 2, 0, 0);

        instance->back_scene_window = widget_alloc(flex_layout_get_base(instance->back_container));
        flex_layout_set_child_widget_grow(
            instance->back_container, instance->back_scene_window, 1);
    });

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->input_queue,
        FuriEventLoopEventIn,
        account_settings_input_queue_callback,
        instance);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->event_queue,
        FuriEventLoopEventIn,
        account_settings_event_queue_callback,
        instance);

    instance->model = account_model_alloc();
    account_model_set_event_callback(
        instance->model, account_settings_account_event_callback, instance);

    if(account_model_is_linked(instance->model)) {
        scene_manager_next_scene(instance->scene_manager, SceneIdLinked);
    } else {
        // TODO: Use wifi model?
        WifiInfo wifi_info;
        wifi_get_info(furi_record_open(RECORD_WIFI), &wifi_info);
        furi_record_close(RECORD_WIFI);
        if(wifi_info.state == WifiStateDisconnected) {
            scene_manager_next_scene(instance->scene_manager, SceneIdNoWifi);
        } else {
            AccountModelState state = account_model_get_state(instance->model);
            if(state == AccountModelStateConnectedNotLinked) {
                scene_manager_next_scene(instance->scene_manager, SceneIdNotLinked);
            } else {
                scene_manager_next_scene(instance->scene_manager, SceneIdConnecting);
            }
        }
    }

    return instance;
}

static void account_settings_free(AccountSettings* instance) {
    furi_assert(instance);
    scene_manager_free(instance->scene_manager);

    with_gui(instance->gui, {
        GuiLayer* layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(layer, account_settings_gui_input_callback);

        widget_free(instance->front_scene_window);
        widget_free(instance->back_scene_window);
        flex_layout_free(instance->back_container);
    });

    account_model_free(instance->model);

    furi_record_close(RECORD_DESKTOP);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_FRONT_DISPLAY);
    furi_record_close(RECORD_BACK_DISPLAY);

    furi_event_loop_unsubscribe(instance->event_loop, instance->input_queue);
    furi_event_loop_unsubscribe(instance->event_loop, instance->event_queue);
    furi_message_queue_free(instance->input_queue);
    furi_message_queue_free(instance->event_queue);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t account_settings_entry(void* arg) {
    if(arg) {
        AccountModel* model = account_model_alloc();
        bool is_linked = account_model_is_linked(model);

        SettingsAppDescriptor* descriptor = arg;
        furi_string_set_str(descriptor->front_title, "Account");
        furi_string_set_str(descriptor->back_title, "Account");
        if(is_linked) {
            furi_string_set_str(descriptor->front_icon, IMG_PATH("account_front_ok_8x8.bin"));
            furi_string_set_str(descriptor->back_icon, IMG_PATH("account_back_11x11.bin"));
        } else {
            furi_string_set_str(descriptor->front_icon, IMG_PATH("account_front_gray_8x8.bin"));
            furi_string_set_str(descriptor->back_icon, IMG_PATH("account_back_11x11.bin"));
        }

        account_model_free(model);
        return 0;
    }

    AccountSettings* instance = account_settings_alloc();
    FuriThread* thread = furi_thread_get_current();
    furi_thread_set_signal_callback(thread, account_settings_thread_signal_callback, instance);
    furi_event_loop_run(instance->event_loop);
    furi_thread_set_signal_callback(thread, NULL, NULL);
    account_settings_free(instance);

    return 0;
}

void account_settings_send_custom_event(AccountSettings* instance, uint32_t event) {
    furi_assert(instance);

    AccountSettingsEvent evt = {.event = event};

    furi_check(
        furi_message_queue_put(instance->event_queue, &evt, FuriWaitForever) == FuriStatusOk);
}
