#include <furi.h>

#include <furi_hal_resources.h>
#include <furi_hal_serial_control.h>
#include <furi_hal_serial.h>

#include <furi_hal_power.h>

#ifdef SRV_INTERCOM
#include <intercom/intercom.h>
#endif

#include <storage/storage.h>

#define TAG "SlUpdateTest"

#include "fkermit.h"

typedef enum {
    Si917BootloaderStateInit,
    Si917BootloaderStateBoot,
    Si917BootloaderStateChangeBaudRate,
    Si917BootloaderStateChangeBaudRateSpeed,
    Si917BootloaderStateChangeBaudRateSpeedSuccess,
    Si917BootloaderStateSetImageType,
    Si917BootloaderStateSetImageSlot,
    Si917BootloaderStateKermitInit,
    Si917BootloaderStateKermitSend,
    Si917BootloaderStateWaitInstall,
} Si917BootloaderState;

typedef struct {
    FuriEventLoop* event_loop;
    FuriStreamBuffer* rx_buffer;
    FuriString* rx_string;
    FuriHalSerialHandle* serial_handle;
    FuriEventLoopTimer* idle_timer;
#ifdef SRV_INTERCOM
    Intercom* intercom;
#endif
    Si917BootloaderState bootloader_state;
    Storage* storage;
    File* firmware_file;
    kermit_t* kermit;
} SlUpdateTestApp;

//////////////////////////////////////////////////////////////////////////
// Kermit i/o functions

static int32_t kermit_src_file_read(void* context, uint8_t* buffer, size_t length) {
    SlUpdateTestApp* app = context;
    return storage_file_read(app->firmware_file, buffer, length);
}

static int32_t kermit_comms_send(void* context, const uint8_t* buffer, size_t length) {
    SlUpdateTestApp* app = context;
    FURI_LOG_D(TAG, "Sending %d bytes", length);

#ifdef KERMIT_DEBUG
    FuriString* str = furi_string_alloc();
    furi_string_cat_printf(str, "Sending %d bytes: ", length);
    for(size_t i = 0; i < length; i++) {
        furi_string_cat_printf(str, "%02x ", buffer[i]);
    }
    furi_string_cat_printf(str, "\n%s", buffer);
    FURI_LOG_I(TAG, "%s", furi_string_get_cstr(str));
    furi_string_free(str);
#endif

    furi_hal_serial_tx(app->serial_handle, buffer, length);
    return length;
}

static const kermit_io_t kermit_io = {
    .src_file_read = kermit_src_file_read,
    .comms_send = kermit_comms_send,
};

//////////////////////////////////////////////////////////////////////////

static void sl_update_test_app_serial_irq_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent events,
    void* context) {
    SlUpdateTestApp* instance = context;
    furi_check(handle == instance->serial_handle);

    if(events & FuriHalSerialRxEventData) {
        while(furi_hal_serial_rx_available(handle)) {
            const uint8_t c = furi_hal_serial_rx(handle);
            furi_check(
                furi_stream_buffer_send(instance->rx_buffer, &c, sizeof(c), 0) == sizeof(c));
        }
    }
}

static bool sl_update_test_app_check_rx_for(SlUpdateTestApp* instance, const char* str) {
    if(furi_string_search_str(instance->rx_string, str) != FURI_STRING_FAILURE) {
        furi_string_reset(instance->rx_string);
        return true;
    }
    return false;
}

static void sl_update_test_app_handle_rx(SlUpdateTestApp* instance) {
#ifdef KERMIT_DEBUG
    FURI_LOG_D(
        TAG,
        "State: %d, Received data: %s",
        instance->bootloader_state,
        furi_string_get_cstr(instance->rx_string));
#endif

    bool should_stop = false;

    switch(instance->bootloader_state) {
    case Si917BootloaderStateInit:
        if(sl_update_test_app_check_rx_for(instance, "Enter 'U'")) {
            const uint8_t leader = 'U';
            furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));
            FURI_LOG_I(TAG, "Leader sent: %c", leader);
            instance->bootloader_state = Si917BootloaderStateBoot;
        }
        break;

    case Si917BootloaderStateBoot:
        if(sl_update_test_app_check_rx_for(instance, "Change UART Baud Rate\r\n")) {
            const uint8_t choice = 'b';
            furi_hal_serial_tx(instance->serial_handle, &choice, sizeof(choice));
            FURI_LOG_I(TAG, "UART Baud Rate change request sent: %c", choice);
            instance->bootloader_state = Si917BootloaderStateChangeBaudRate;
        }
        break;

    case Si917BootloaderStateChangeBaudRate:
        if(sl_update_test_app_check_rx_for(instance, "5 115200\r\n")) {
            const uint8_t choice = '4';
            furi_hal_serial_tx(instance->serial_handle, &choice, sizeof(choice));
            FURI_LOG_I(TAG, "UART Baud Rate speed request sent: %c", choice);

            furi_hal_serial_set_baud_rate(instance->serial_handle, 921600);

            // const uint8_t leader = 'U';
            // furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));
            // FURI_LOG_I(TAG, "New Leader sent: %c", leader);

            instance->bootloader_state = Si917BootloaderStateChangeBaudRateSpeed;
        }
        break;

    case Si917BootloaderStateChangeBaudRateSpeed:
        const uint8_t leader = 'U';
        furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));
        FURI_LOG_I(TAG, "New Leader sent: %c", leader);
        // fall through

    case Si917BootloaderStateChangeBaudRateSpeedSuccess:
        if(sl_update_test_app_check_rx_for(instance, "Baud Rate was updated successfully!")) {
            FURI_LOG_I(TAG, "Baud rate was set to 921600");
            instance->bootloader_state = Si917BootloaderStateSetImageType;
        }
        break;

    case Si917BootloaderStateSetImageType:
        if(sl_update_test_app_check_rx_for(instance, "Enter Next Command")) {
            const uint8_t image_type = '4';
            furi_hal_serial_tx(instance->serial_handle, &image_type, sizeof(image_type));

            FURI_LOG_I(TAG, "Image type set to: %c", image_type);
            instance->bootloader_state = Si917BootloaderStateSetImageSlot;
        }
        break;

    case Si917BootloaderStateSetImageSlot:
        if(sl_update_test_app_check_rx_for(instance, "Enter M4 Image No(1-f)")) {
            const uint8_t image_slot = '1';
            furi_hal_serial_tx(instance->serial_handle, &image_slot, sizeof(image_slot));

            FURI_LOG_I(TAG, "Image slot set to: %c", image_slot);
            instance->bootloader_state = Si917BootloaderStateKermitInit;
        }
        break;

    case Si917BootloaderStateKermitInit:
        if(sl_update_test_app_check_rx_for(instance, "Send MCU firmware(*.rps)")) {
            FURI_LOG_W(TAG, "Kermit init");
            instance->bootloader_state = Si917BootloaderStateKermitSend;
            storage_file_open(
                instance->firmware_file, "/ext/firmware.rps", FSAM_READ, FSOM_OPEN_EXISTING);
            kermit_run(instance->kermit);
        }
        break;

    case Si917BootloaderStateKermitSend:
        if(kermit_is_active(instance->kermit)) {
            int32_t data_size = furi_string_size(instance->rx_string);
            int32_t data_fed = kermit_feed_serial_data(
                instance->kermit,
                (const uint8_t*)furi_string_get_cstr(instance->rx_string),
                data_size);
            furi_string_reset(instance->rx_string);
            if(data_fed != data_size) {
                FURI_LOG_E(TAG, "Error feeding data to kermit");
                should_stop = true;
            }
        } else {
            FURI_LOG_I(TAG, "Kermit upload complete");
            instance->bootloader_state = Si917BootloaderStateWaitInstall;
        }
        break;

    case Si917BootloaderStateWaitInstall:
        if(sl_update_test_app_check_rx_for(instance, "Upgradation Successful")) {
            FURI_LOG_W(TAG, "Install success");
            should_stop = true;
        } else if(sl_update_test_app_check_rx_for(instance, "Upgradation Failed")) {
            FURI_LOG_E(TAG, "Install failed");
            should_stop = true;
        }
        break;

    default:
        FURI_LOG_E(TAG, "Invalid state: %d", instance->bootloader_state);
        should_stop = true;
    }

    if(should_stop) {
        FURI_LOG_I(TAG, "Stopping event loop, state: %d", instance->bootloader_state);
        furi_event_loop_stop(instance->event_loop);
    }
}

static void sl_update_test_app_rx_buffer_callback(FuriEventLoopObject* object, void* context) {
    SlUpdateTestApp* instance = context;
    furi_check(object == instance->rx_buffer);

    // Reset the idle timer
    furi_event_loop_timer_restart(instance->idle_timer);

    char c;
    while(furi_stream_buffer_bytes_available(instance->rx_buffer)) {
        furi_check(furi_stream_buffer_receive(instance->rx_buffer, &c, sizeof(c), 0) == sizeof(c));
        furi_string_push_back(instance->rx_string, c);
    }

    sl_update_test_app_handle_rx(instance);
}

static void sl_update_app_intercom_error_callback(IntercomError error, void* context) {
    UNUSED(error);
    UNUSED(context);
    // Empty callback
}

static void sl_update_test_idle_timer_callback(void* context) {
    SlUpdateTestApp* instance = context;

    FURI_LOG_W(TAG, "Watchdog expired");
    furi_event_loop_stop(instance->event_loop);
}

static SlUpdateTestApp* sl_update_test_app_alloc(void) {
    FURI_LOG_I(TAG, "Starting SL Update Test App");

    SlUpdateTestApp* instance = malloc(sizeof(SlUpdateTestApp));

    instance->event_loop = furi_event_loop_alloc();
    instance->rx_buffer = furi_stream_buffer_alloc(512, 1);
    instance->rx_string = furi_string_alloc();
    instance->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart2);

    instance->bootloader_state = Si917BootloaderStateInit;

    instance->kermit = kermit_alloc(&kermit_io, instance);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->firmware_file = storage_file_alloc(instance->storage);
    instance->idle_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        sl_update_test_idle_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    furi_event_loop_subscribe_stream_buffer(
        instance->event_loop,
        instance->rx_buffer,
        FuriEventLoopEventIn,
        sl_update_test_app_rx_buffer_callback,
        instance);

#ifdef SRV_INTERCOM
    // Prevent crashes
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_error_callback(instance->intercom, sl_update_app_intercom_error_callback, NULL);
#else
    UNUSED(sl_update_app_intercom_error_callback);
#endif

    // Start idle timer
    furi_event_loop_timer_start(instance->idle_timer, 5000); // FIXME

    furi_hal_serial_init(instance->serial_handle, 115200);
    furi_hal_serial_set_callback(
        instance->serial_handle, NULL, sl_update_test_app_serial_irq_callback, instance);
    furi_hal_serial_async_rx_start(instance->serial_handle, false);

    furi_hal_power_reset_917(true);

    const uint8_t leader = 0;
    furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));

    return instance;
}

static void sl_update_test_app_free(SlUpdateTestApp* instance) {
#ifdef SRV_INTERCOM
    // TODO: The ability to reset intercom
    intercom_set_error_callback(instance->intercom, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);
#endif

    furi_hal_serial_async_rx_stop(instance->serial_handle);
    furi_hal_serial_set_callback(instance->serial_handle, NULL, NULL, NULL);
    furi_hal_serial_control_release(instance->serial_handle);

    furi_event_loop_timer_free(instance->idle_timer);
    furi_event_loop_unsubscribe(instance->event_loop, instance->rx_buffer);

    furi_string_free(instance->rx_string);
    furi_stream_buffer_free(instance->rx_buffer);
    furi_event_loop_free(instance->event_loop);

    kermit_free(instance->kermit);
    storage_file_free(instance->firmware_file);
    furi_record_close(RECORD_STORAGE);

    furi_hal_power_reset_917(false);

    FURI_LOG_I(TAG, "SL Update Test App stopped");

    free(instance);
}

int32_t sl_update_test_app(void* arg) {
    UNUSED(arg);

    SlUpdateTestApp* instance = sl_update_test_app_alloc();
    furi_event_loop_run(instance->event_loop);
    sl_update_test_app_free(instance);

    return 0;
}
