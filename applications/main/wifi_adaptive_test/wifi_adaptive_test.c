#include <furi.h>

#include "wifi_adaptive_test_i.h"

#include <gui/gui.h>
#include <gui/modules/var_item_list.h>
#include <gui/modules/label.h>
#include "helpers/wifi_adaptive_cli.h"

#define TAG "WifiAdaptiveTest"

#define UDP_SERVER_IP "192.168.1.2"

typedef enum {
    WifiAdaptiveTestCustomEventExit = (1UL << 0),
    WifiAdaptiveTestCustomEventStartTest = (1UL << 1),
    WifiAdaptiveTestCustomEventStopTest = (1UL << 2),
} WifiAdaptiveTestCustomEvent;

typedef enum {
    WifiAdaptiveTestStateStop,
    WifiAdaptiveTestStateLoading,
    WifiAdaptiveTestStateRunning,
    WifiAdaptiveTestStateStoping,
} WifiAdaptiveTestState;

struct WifiAdaptiveTest {
    FuriEventLoop* event_loop;
    Gui* gui;
    Label* label_status;
    bool exit_on_back;
    Label* label;
    WifiAdaptiveCliSettings settings;
    WifiAdaptiveTestState test_state;
};

static bool wifi_adaptive_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    WifiAdaptiveTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(
                instance->event_loop, WifiAdaptiveTestCustomEventExit);
            instance->exit_on_back = true;
            consumed = true;
        }

    } else if(event->type == InputTypeLong) {
        if(event->key == InputKeyStart) {
            if(instance->test_state == WifiAdaptiveTestStateRunning) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, WifiAdaptiveTestCustomEventStopTest);
            } else if(instance->test_state == WifiAdaptiveTestStateStop) {
                furi_event_loop_set_custom_event(
                    instance->event_loop, WifiAdaptiveTestCustomEventStartTest);
                instance->test_state = WifiAdaptiveTestStateLoading;
            }
            consumed = true;
        }
    }

    if(instance->test_state != WifiAdaptiveTestStateStop) {
        consumed = true;
    }
    return consumed;
}

static void wifi_adaptive_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    WifiAdaptiveTest* instance = context;

    if(events & WifiAdaptiveTestCustomEventExit) {
        if(instance->exit_on_back) {
            if(instance->test_state != WifiAdaptiveTestStateStop) {
                FURI_LOG_I(TAG, "Stop test");
                wifi_adaptive_cli_stop();
                instance->test_state = WifiAdaptiveTestStateStop;
            }
            furi_event_loop_stop(instance->event_loop);
        }
    }

    if(events & WifiAdaptiveTestCustomEventStartTest) {
        furi_check(instance->test_state == WifiAdaptiveTestStateLoading);
        if(wifi_adaptive_cli_start(instance, instance->settings)) {
            FURI_LOG_I(TAG, "Start test");
            instance->test_state = WifiAdaptiveTestStateRunning;
        } else {
            FURI_LOG_E(TAG, "Start test failed");
            wifi_adaptive_cli_stop();
            instance->test_state = WifiAdaptiveTestStateStop;
        }
    }

    if(events & WifiAdaptiveTestCustomEventStopTest) {
        furi_check(instance->test_state == WifiAdaptiveTestStateRunning);
        instance->test_state = WifiAdaptiveTestStateStoping;
        FURI_LOG_I(TAG, "Stop test");
        wifi_adaptive_cli_stop();
        instance->test_state = WifiAdaptiveTestStateStop;
    }
}

void wifi_adaptive_test_update(
    WifiAdaptiveTest* instance,
    WifiAdaptiveTestStatus sratus,
    FuriString* sta_ip_addr_str) {
    FuriString* str = furi_string_alloc();

    if(sratus == WifiAdaptiveTestStatusConnected) {
        furi_string_printf(str, "Connected to AP\nIP: %s", furi_string_get_cstr(sta_ip_addr_str));
    } else if(sratus == WifiAdaptiveTestStatusConnecting) {
        furi_string_printf(str, "Connecting to AP");
    } else if(sratus == WifiAdaptiveTestStatusDisconnected) {
        furi_string_printf(str, "Disconnected from AP");
    } else if(sratus == WifiAdaptiveTestStatusError) {
        furi_string_printf(str, "Error connecting to AP");
    } else {
        furi_crash();
    }

    with_gui(instance->gui, {
        label_set_text(instance->label_status, furi_string_get_cstr(str));
    });
    furi_string_free(str);
}

void wifi_adaptive_test_set_default_settings(WifiAdaptiveTest* instance) {
    instance->settings.ip = UDP_SERVER_IP;
}

static WifiAdaptiveTest* wifi_adaptive_test_alloc(void) {
    WifiAdaptiveTest* instance = malloc(sizeof(WifiAdaptiveTest));

    wifi_adaptive_test_set_default_settings(instance);

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_adaptive_test_custom_event_callback, instance);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, wifi_adaptive_test_input_callback, instance);

        GuiLayer* top_layer = gui_get_layer(instance->gui, GuiLayerIdTop);
        Widget* top_layer_root = gui_layer_get_root_widget(top_layer, GuiDisplayIdBack);

        Widget* root = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);

        instance->label_status = label_alloc(root);
        widget_set_pos_y(label_get_base(instance->label_status), 60);
        widget_set_height(label_get_base(instance->label_status), 30);

        instance->label = label_alloc(top_layer_root);
        label_set_text(
            instance->label,
            "WifiAdaptiveTest plz create ap\n"
            "SSID: Zyxel24\n"
            "PASS: 1qa2wszz\n"
            "set static ip " UDP_SERVER_IP " on your PC\n"
            "start \"iperf.exe -s -u -p 5001 -i 1\"");
        widget_set_pos(label_get_base(instance->label), 5, 0);
    });

    wifi_adaptive_test_update(instance, WifiAdaptiveTestStatusDisconnected, NULL);

    return instance;
}

static void wifi_adaptive_test_free(WifiAdaptiveTest* instance) {
    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, wifi_adaptive_test_input_callback);
        label_free(instance->label);
        label_free(instance->label_status);
    });

    furi_record_close(RECORD_GUI);

    furi_event_loop_free(instance->event_loop);
    free(instance);
}

int32_t wifi_adaptive_test_app(void* arg) {
    UNUSED(arg);
    WifiAdaptiveTest* instance = wifi_adaptive_test_alloc();
    furi_event_loop_run(instance->event_loop);
    wifi_adaptive_test_free(instance);

    return 0;
}
