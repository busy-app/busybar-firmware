#include <furi.h>
#include <applications.h>

#include <desktop/desktop.h>

#include <gui/gui.h>
#include <gui/modules/submenu.h>

#define TAG "AppsMenu"

#ifndef APPS_MENU_ERROR_TEST
#define APPS_MENU_ERROR_TEST 0
#endif

typedef struct {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    Desktop* desktop;
    Gui* gui;
    Submenu* submenu;
} AppsMenu;

static void apps_menu_submenu_item_callback(uint32_t index, void* context) {
    furi_assert(context);
    AppsMenu* instance = context;

    furi_check(furi_message_queue_put(instance->queue, &index, FuriWaitForever) == FuriStatusOk);
}

static void apps_menu_queue_callback(FuriEventLoopObject* object, void* context) {
    furi_assert(context);
    AppsMenu* instance = context;

    furi_assert(object == instance->queue);

    uint32_t index;
    furi_check(furi_message_queue_get(instance->queue, &index, 0) == FuriStatusOk);

    const char* app_name = "";

    if(index < FLIPPER_APPS_COUNT) {
        app_name = FLIPPER_APPS[index].name;
    }

    if(desktop_replace_current_app(instance->desktop, app_name, NULL)) {
        FURI_LOG_D(TAG, "Selected app %s", app_name);
    } else {
        FURI_LOG_E(TAG, "Failed to select app %s", app_name);
    }
}

static AppsMenu* apps_menu_alloc(void) {
    AppsMenu* instance = malloc(sizeof(AppsMenu));

    instance->event_loop = furi_event_loop_alloc();
    instance->queue = furi_message_queue_alloc(1, sizeof(uint32_t));
    instance->desktop = furi_record_open(RECORD_DESKTOP);
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->queue,
        FuriEventLoopEventIn,
        apps_menu_queue_callback,
        instance);

    with_gui(instance->gui, {
        Widget* root = gui_get_root_widget(instance->gui, GuiDisplayIdFront, GuiLayerIdMain);
        instance->submenu = submenu_alloc(root);

        for(uint32_t i = 0; i < FLIPPER_APPS_COUNT; ++i) {
            const FlipperInternalApplication* app = &FLIPPER_APPS[i];
            submenu_add_item(
                instance->submenu, app->name, i, apps_menu_submenu_item_callback, instance);
        }
        UNUSED(apps_menu_submenu_item_callback);

#if APPS_MENU_ERROR_TEST
        submenu_add_item(
            instance->submenu, "Error Test", UINT32_MAX, apps_menu_submenu_item_callback, instance);
#endif
        gui_add_active_widget(instance->gui, (Widget*)instance->submenu);
    });

    return instance;
}

static void apps_menu_free(AppsMenu* instance) {
    with_gui(instance->gui, { submenu_free(instance->submenu); });

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_DESKTOP);

    furi_event_loop_unsubscribe(instance->event_loop, instance->queue);
    furi_message_queue_free(instance->queue);
    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t apps_menu_app(void* arg) {
    UNUSED(arg);

    AppsMenu* instance = apps_menu_alloc();
    furi_event_loop_run(instance->event_loop);
    apps_menu_free(instance);

    return 0;
}
