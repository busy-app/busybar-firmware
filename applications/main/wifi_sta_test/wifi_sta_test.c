#include <furi.h>

#include "wifi_sta_test_i.h"

#include <gui/gui.h>
#include <gui/modules/var_item_list.h>
#include <gui/modules/label.h>
#include "helpers/wifi_sta_cli.h"

#define TAG "WifiStaTest"

typedef enum {
    WifiStaTestCustomEventExit = (1UL << 0),
    WifiStaTestCustomEventStartTest = (1UL << 1),
    WifiStaTestCustomEventStopTest = (1UL << 2),
} WifiStaTestCustomEvent;

typedef enum {
    WifiStaTestStateStop,
    WifiStaTestStateLoading,
    WifiStaTestStateRunning,
    WifiStaTestStateStoping,
} WifiStaTestState;

struct WifiStaTest {
    FuriEventLoop* event_loop;
    Gui* gui;
    Label* label_status;
    bool exit_on_back;
    Label* label;
    WifiStaCliSettings settings;
    WifiStaTestState test_state;
};

static bool wifi_sta_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    WifiStaTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(instance->event_loop, WifiStaTestCustomEventExit);
            instance->exit_on_back = true;
            consumed = true;
        } else if(event->key == InputKeyStart) {
            if(instance->test_state == WifiStaTestStateRunning) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, WifiStaTestCustomEventStopTest);
            } else if(instance->test_state == WifiStaTestStateStop) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, WifiStaTestCustomEventStartTest);
                instance->test_state = WifiStaTestStateLoading;
            }
            consumed = true;
        }
    }

    if(instance->test_state != WifiStaTestStateStop) {
        consumed = true;
    }
    return consumed;
}

static void wifi_sta_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    WifiStaTest* instance = context;

    if(events & WifiStaTestCustomEventExit) {
        if(instance->exit_on_back) {
            if(instance->test_state != WifiStaTestStateStop) {
                FURI_LOG_I(TAG, "Stop test");
                wifi_sta_cli_stop();
                instance->test_state = WifiStaTestStateStop;
            }
            furi_event_loop_stop(instance->event_loop);
        }
    }

    if(events & WifiStaTestCustomEventStartTest) {
        furi_check(instance->test_state == WifiStaTestStateLoading);
        if(wifi_sta_cli_start(instance, instance->settings)) {
            FURI_LOG_I(TAG, "Start test");
            instance->test_state = WifiStaTestStateRunning;
        } else {
            FURI_LOG_E(TAG, "Start test failed");
            wifi_sta_cli_stop();
            instance->test_state = WifiStaTestStateStop;
        }
    }

    if(events & WifiStaTestCustomEventStopTest) {
        furi_check(instance->test_state == WifiStaTestStateRunning);
        instance->test_state = WifiStaTestStateStoping;
        FURI_LOG_I(TAG, "Stop test");
        wifi_sta_cli_stop();
        instance->test_state = WifiStaTestStateStop;
    }
}

void wifi_sta_test_update(
    WifiStaTest* instance,
    WifiStaTestStatus sratus,
    FuriString* sta_ip_addr_str) {
    FuriString* str = furi_string_alloc();

    if(sratus == WifiStaTestStatusConnected) {
        furi_string_printf(str, "Connected to AP\nIP: %s", furi_string_get_cstr(sta_ip_addr_str));
    } else if(sratus == WifiStaTestStatusConnecting) {
        furi_string_printf(str, "Connecting to AP");
    } else if(sratus == WifiStaTestStatusDisconnected) {
        furi_string_printf(str, "Disconnected from AP");
    } else if(sratus == WifiStaTestStatusError) {
        furi_string_printf(str, "Error connecting to AP");
    } else {
        furi_crash();
    }

    with_gui(instance->gui, {
        label_set_text(instance->label_status, furi_string_get_cstr(str));
    });
    furi_string_free(str);
}

static WifiStaTest* wifi_sta_test_alloc(void) {
    WifiStaTest* instance = malloc(sizeof(WifiStaTest));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_sta_test_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, wifi_sta_test_input_callback, instance);

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdTop);
        Widget* top_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);

        instance->label_status = label_alloc(root);
        widget_set_pos_y(label_get_base(instance->label_status), 40);
        widget_set_height(label_get_base(instance->label_status), 30);
        //widget_set_visible(label_get_base(instance->label_status), false);

        instance->label = label_alloc(top_layer_root);
        label_set_text(
            instance->label, "WifiStaTest plz create ap\nSSID: Zyxel24\nPASS: 1qa2wszz");
        widget_set_pos(label_get_base(instance->label), 10, 0);
    });

    wifi_sta_test_update(instance, WifiStaTestStatusDisconnected, NULL);

    return instance;
}

static void wifi_sta_test_free(WifiStaTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, wifi_sta_test_input_callback);
        label_free(instance->label);
        label_free(instance->label_status);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t wifi_sta_test_app(void* arg) {
    UNUSED(arg);
    WifiStaTest* instance = wifi_sta_test_alloc();
    furi_event_loop_run(instance->event_loop);
    wifi_sta_test_free(instance);

    return 0;
}
