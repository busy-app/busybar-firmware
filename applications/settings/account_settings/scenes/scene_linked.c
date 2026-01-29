#include "../account_settings.h"
#include <settings_helpers/gui_params.h>
#include <gui/modules/dialog.h>

typedef struct {
    Dialog* front_dialog;
    Dialog* back_dialog;
} SceneAccountInfo;

typedef enum {
    SceneEventUnlink = AppEventSceneEventsStart,
    SceneEventCancel,
} SceneEvent;

static void account_scene_linked_dialog_callback(uint8_t result, void* context) {
    AccountSettings* instance = context;
    if(result == 0) {
        account_settings_send_custom_event(instance, SceneEventUnlink);
    } else {
        account_settings_send_custom_event(instance, SceneEventCancel);
    }
}

static void get_short_email(AccountSettings* instance, FuriString* mail_str) {
    FuriString* temp_str = furi_string_alloc();
    account_model_get_email(instance->model, temp_str);
    do {
        if(furi_string_empty(temp_str)) break;
        char first_char = furi_string_get_char(temp_str, 0);
        size_t domain_start = furi_string_search_char(temp_str, '@', 0);
        if(domain_start == FURI_STRING_FAILURE) break;
        furi_string_right(temp_str, domain_start);

        furi_string_printf(mail_str, "%c...%s", first_char, furi_string_get_cstr(temp_str));
    } while(0);
    furi_string_free(temp_str);
}

static void account_scene_linked_on_enter(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneAccountInfo* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinked);

    FuriString* mail_short_str = furi_string_alloc();
    get_short_email(instance, mail_short_str);

    with_gui(instance->gui, {
        data->front_dialog = dialog_alloc(instance->front_scene_window);
        data->back_dialog = dialog_alloc(instance->back_scene_window);

        dialog_set_text(data->front_dialog, "Connected");
        dialog_set_text(data->back_dialog, "Connected");

        dialog_set_text_sub(data->front_dialog, furi_string_get_cstr(mail_short_str));
        dialog_set_text_sub(data->back_dialog, furi_string_get_cstr(mail_short_str));

        Color color_forget = COLOR_MAKE_RGB(0xED, 0x00, 0x18);
        Color color_cancel = COLOR_MAKE_RGB(0xFF, 0xFF, 0xFF);
        dialog_set_option_colors(data->front_dialog, color_forget, color_cancel);

        dialog_set_options(data->front_dialog, "Unlink", "Cancel");
        dialog_set_options(data->back_dialog, "Unlink", "Cancel");

        dialog_set_calback(data->front_dialog, account_scene_linked_dialog_callback, instance);
    });

    furi_string_free(mail_short_str);
}

static void account_scene_linked_on_exit(void* context) {
    furi_assert(context);

    AccountSettings* instance = context;
    SceneAccountInfo* data = scene_manager_get_scene_data(instance->scene_manager, SceneIdLinked);

    with_gui(instance->gui, {
        dialog_free(data->front_dialog);
        dialog_free(data->back_dialog);
    });
}

static bool account_scene_linked_on_event(const SceneManagerEvent* event, void* context) {
    furi_assert(context);

    AccountSettings* instance = context;

    bool consumed = false;
    if(event->type == SceneManagerEventTypeCustom) {
        switch(event->event) {
        case SceneEventUnlink:
            account_model_unlink(instance->model);
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
            consumed = true;
            break;
        case SceneEventCancel:
            desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
            consumed = true;
            break;
        case AppEventAccountUnlinked:
            scene_manager_replace_current_scene(instance->scene_manager, SceneIdConnecting);
            consumed = true;
        default:
            break;
        }
    } else if(event->type == SceneManagerEventTypeBack) {
        desktop_replace_current_app(instance->desktop, MAIN_SETTINGS_APP, THIS_SETTINGS_APP);
        consumed = true;
    }

    return consumed;
}

const Scene account_scene_linked = {
    .enter_callback = account_scene_linked_on_enter,
    .exit_callback = account_scene_linked_on_exit,
    .event_callback = account_scene_linked_on_event,
    .data_size = sizeof(SceneAccountInfo),
};
