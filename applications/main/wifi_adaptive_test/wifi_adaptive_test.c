#include <furi.h>
#include <lwip/sockets.h>

#include <gui/gui.h>
#include <gui/modules/label.h>
#include <gui/modules/image.h>
#include <storage/storage.h>
#include <gui/modules/flex_layout.h>
#include <gui/modules/var_item_list.h>

#include <wifi/wifi.h>
#include <usb_network/usb_network.h>

#define TAG "WifiAdaptiveTest"

#define IMAGE_FRONT_PATH EXT_PATH("apps_assets/debug/images/lab_test_front_display_72x16.bin")

#define SEND_ADDR_HOST_PART (50)
#define UDP_SEND_SIZE       (977U) // Limited by Intercom
#define UDP_SEND_PORT       (5000)

typedef enum {
    WifiAdaptiveTestCustomEventExit = (1UL << 0),
} WifiAdaptiveTestCustomEvent;

typedef struct {
    FuriThread* worker_thread;
    FuriEventLoop* event_loop;
    Gui* gui;
    Label* label;
    uint8_t send_buf[UDP_SEND_SIZE];
    WifiIpv4 local_addr;
    WifiIpv4 remote_addr;
    bool stop_worker;
    Image* image_front;
} WifiAdaptiveTest;

static bool wifi_adaptive_test_input_callback(const InputEvent* event, void* context) {
    furi_assert(event);
    furi_assert(context);
    WifiAdaptiveTest* instance = context;

    bool consumed = false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            furi_event_loop_set_custom_event(
                instance->event_loop, WifiAdaptiveTestCustomEventExit);
            consumed = true;
        }
    }

    return consumed;
}

static void wifi_adaptive_test_custom_event_callback(uint32_t events, void* context) {
    furi_assert(context);
    WifiAdaptiveTest* instance = context;

    if(events & WifiAdaptiveTestCustomEventExit) {
        instance->stop_worker = true;
        furi_event_loop_stop(instance->event_loop);
    }
}

static int32_t wifi_adaptive_test_worker(void* arg) {
    furi_assert(arg);
    WifiAdaptiveTest* instance = arg;

    FURI_LOG_I(TAG, "Worker started");

    UsbNetwork* usbnet = furi_record_open(RECORD_USB_NETWORK);
    usb_network_thread_init(usbnet);

    do {
        const int udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if(udp < 0) {
            FURI_LOG_E(TAG, "Failed to create socket: %s", strerror(errno));
            break;
        }

        const struct sockaddr_in to = {
            .sin_family = AF_INET,
            .sin_port = htons(UDP_SEND_PORT),
            .sin_addr.s_addr = instance->remote_addr.value,
        };

        while(!instance->stop_worker) {
            const ssize_t sent_size = sendto(
                udp, instance->send_buf, UDP_SEND_SIZE, 0, (struct sockaddr*)&to, sizeof(to));

            if(sent_size < 0) {
                FURI_LOG_E(TAG, "UDP tx error: %s", strerror(errno));
                break;
            }
            // Prevent deadlocks
            furi_thread_yield();
        }

        close(udp);

    } while(false);

    usb_network_thread_cleanup(usbnet);
    furi_record_close(RECORD_USB_NETWORK);

    FURI_LOG_I(TAG, "Worker stopped");

    return 0;
}

static WifiAdaptiveTest* wifi_adaptive_test_alloc(void) {
    WifiAdaptiveTest* instance = malloc(sizeof(WifiAdaptiveTest));

    instance->event_loop = furi_event_loop_alloc();
    instance->gui = furi_record_open(RECORD_GUI);
    instance->worker_thread =
        furi_thread_alloc_ex("WifiAdaptiveTestWorker", 4096, wifi_adaptive_test_worker, instance);

    for(uint32_t i = 0; i < UDP_SEND_SIZE; ++i) {
        instance->send_buf[i] = i % 0xFF;
    }

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_add_input_callback(main_layer, wifi_adaptive_test_input_callback, instance);

        Widget* back_screen = gui_layer_get_root_widget(main_layer, GuiDisplayIdBack);

        instance->label = label_alloc(back_screen);
        label_set_text(instance->label, "Waiting for Wifi ...");

        widget_set_align(label_get_base(instance->label), AlignCenter);

        // GuiDisplayIdFront
        Widget* root_front = gui_layer_get_root_widget(main_layer, GuiDisplayIdFront);
        instance->image_front = image_alloc(root_front);
        image_set_source(instance->image_front, IMAGE_FRONT_PATH);
        widget_set_align(image_get_base(instance->image_front), AlignCenter);
    });

    FuriString* message = furi_string_alloc();

    Wifi* wifi = furi_record_open(RECORD_WIFI);

    WifiInfo wifi_info;
    const WifiStatus status = wifi_get_info(wifi, &wifi_info);

    if(status != WifiStatusOk) {
        furi_string_printf(message, "Wifi error: %d", status);

    } else if(wifi_info.state != WifiStateUp) {
        furi_string_set(message, "Please connect to AP\nand restart the app");

    } else {
        WifiIpv4* local_addr = &instance->local_addr;
        WifiIpv4* remote_addr = &instance->remote_addr;

        *local_addr = wifi_info.ip_config.ip4.address;
        *remote_addr = instance->local_addr;

        remote_addr->bytes[3] = SEND_ADDR_HOST_PART;

        furi_string_printf(
            message,
            "Device address:\n%hhu.%hhu.%hhu.%hhu\n\n"
            "Target address:\n%hhu.%hhu.%hhu.%hhu",
            local_addr->bytes[0],
            local_addr->bytes[1],
            local_addr->bytes[2],
            local_addr->bytes[3],
            remote_addr->bytes[0],
            remote_addr->bytes[1],
            remote_addr->bytes[2],
            remote_addr->bytes[3]);

        furi_thread_start(instance->worker_thread);
    }

    with_gui(instance->gui, { label_set_text(instance->label, furi_string_get_cstr(message)); });

    furi_string_free(message);

    furi_event_loop_set_custom_event_callback(
        instance->event_loop, wifi_adaptive_test_custom_event_callback, instance);

    return instance;
}

static void wifi_adaptive_test_free(WifiAdaptiveTest* instance) {
    furi_thread_join(instance->worker_thread);
    furi_thread_free(instance->worker_thread);

    with_gui(instance->gui, {
        GuiLayer* main_layer = gui_get_layer(instance->gui, GuiLayerIdMain);
        gui_layer_remove_input_callback(main_layer, wifi_adaptive_test_input_callback);
        label_free(instance->label);
        image_free(instance->image_front);
    });

    furi_event_loop_free(instance->event_loop);
    free(instance);

    furi_record_close(RECORD_WIFI);
    furi_record_close(RECORD_GUI);
}

int32_t wifi_adaptive_test_app(void* arg) {
    UNUSED(arg);
    WifiAdaptiveTest* instance = wifi_adaptive_test_alloc();
    furi_event_loop_run(instance->event_loop);
    wifi_adaptive_test_free(instance);

    return 0;
}
