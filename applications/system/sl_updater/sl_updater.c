#include "sl_updater.h"

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
    Si917BootloaderStateChangeBaudRateNewLeader,
    Si917BootloaderStateChangeBaudRateSuccess,
    Si917BootloaderStateSetImageType,
    Si917BootloaderStateSetImageSlot,
    Si917BootloaderStateKermitInit,
    Si917BootloaderStateKermitSend,
    Si917BootloaderStateWaitInstall,
    Si917BootloaderStateInstallSuccess,
    Si917BootloaderStateInstallFail,
} Si917BootloaderState;

struct SlUpdater {
    FuriEventLoop* event_loop;
    FuriStreamBuffer* rx_buffer;
    FuriString* rx_string;
    FuriHalSerialHandle* serial_handle;
    FuriEventLoopTimer* idle_timer;
#ifdef SRV_INTERCOM
    Intercom* intercom;
#endif
    uint8_t timeout_seconds;
    bool is_stack_image;
    Si917BootloaderState bootloader_state;
    Storage* storage;
    File* firmware_file;
    kermit_t* kermit;
};

//////////////////////////////////////////////////////////////////////////
// Kermit i/o functions

static int32_t kermit_src_file_read(void* context, uint8_t* buffer, size_t length) {
    SlUpdater* app = context;
    return storage_file_read(app->firmware_file, buffer, length);
}

static int32_t kermit_comms_send(void* context, const uint8_t* buffer, size_t length) {
    SlUpdater* app = context;
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

static void sl_updater_serial_irq_callback(
    FuriHalSerialHandle* handle,
    FuriHalSerialRxEvent events,
    void* context) {
    SlUpdater* instance = context;
    furi_check(handle == instance->serial_handle);

    if(events & FuriHalSerialRxEventData) {
        while(furi_hal_serial_rx_available(handle)) {
            const uint8_t c = furi_hal_serial_rx(handle);
            furi_check(
                furi_stream_buffer_send(instance->rx_buffer, &c, sizeof(c), 0) == sizeof(c));
        }
    }
}

static bool sl_updater_check_rx_for(SlUpdater* instance, const char* str) {
    if(furi_string_search_str(instance->rx_string, str) != FURI_STRING_FAILURE) {
        furi_string_reset(instance->rx_string);
        return true;
    }
    return false;
}

static void sl_updater_handle_rx(SlUpdater* instance) {
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
        if(sl_updater_check_rx_for(instance, "Enter 'U'")) {
            const uint8_t leader = 'U';
            furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));
            FURI_LOG_I(TAG, "Leader sent: %c", leader);
            instance->bootloader_state = Si917BootloaderStateBoot;
        }
        break;

    case Si917BootloaderStateBoot:
        if(sl_updater_check_rx_for(instance, "Change UART Baud Rate\r\n")) {
            const uint8_t choice = 'b';
            furi_hal_serial_tx(instance->serial_handle, &choice, sizeof(choice));
            FURI_LOG_I(TAG, "UART Baud Rate change request sent: %c", choice);
            instance->bootloader_state = Si917BootloaderStateChangeBaudRate;
        }
        break;

    case Si917BootloaderStateChangeBaudRate:
        if(sl_updater_check_rx_for(instance, "5 115200\r\n")) {
            const uint8_t choice = '4';
            furi_hal_serial_tx(instance->serial_handle, &choice, sizeof(choice));
            FURI_LOG_I(TAG, "UART Baud Rate speed request sent: %c", choice);

            furi_hal_serial_set_baud_rate(instance->serial_handle, 921600);
            instance->bootloader_state = Si917BootloaderStateChangeBaudRateNewLeader;
        }
        break;

    case Si917BootloaderStateChangeBaudRateNewLeader:
        const uint8_t leader = 'U';
        furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));
        FURI_LOG_I(TAG, "New Leader sent: %c", leader);
        // fall through

    case Si917BootloaderStateChangeBaudRateSuccess:
        if(sl_updater_check_rx_for(instance, "Baud Rate was updated successfully!")) {
            FURI_LOG_I(TAG, "Baud rate was set to 921600");
            instance->bootloader_state = Si917BootloaderStateSetImageType;
        }
        break;

    case Si917BootloaderStateSetImageType:
        if(sl_updater_check_rx_for(instance, "Enter Next Command")) {
            const uint8_t image_type = instance->is_stack_image ? 'B' : '4';
            furi_hal_serial_tx(instance->serial_handle, &image_type, sizeof(image_type));

            FURI_LOG_I(TAG, "Image type set to: %c", image_type);
            instance->bootloader_state = Si917BootloaderStateSetImageSlot;
        }
        break;

    case Si917BootloaderStateSetImageSlot:
        const char* needle = instance->is_stack_image ? "Enter Wireless Image No(0-f)" :
                                                        "Enter M4 Image No(1-f)";
        if(sl_updater_check_rx_for(instance, needle)) {
            const uint8_t image_slot = instance->is_stack_image ? '0' : '1';
            furi_hal_serial_tx(instance->serial_handle, &image_slot, sizeof(image_slot));

            FURI_LOG_I(TAG, "Image slot set to: %c", image_slot);
            instance->bootloader_state = Si917BootloaderStateKermitInit;
        }
        break;

    case Si917BootloaderStateKermitInit:
        const char* fw_needle = instance->is_stack_image ? "Send NWP firmware(*.rps) " :
                                                           "Send MCU firmware(*.rps)";
        if(sl_updater_check_rx_for(instance, fw_needle)) {
            FURI_LOG_W(TAG, "Kermit init");
            instance->bootloader_state = Si917BootloaderStateKermitSend;
            kermit_start(instance->kermit, 5);
        }
        break;

    case Si917BootloaderStateKermitSend:
        if(kermit_is_active(instance->kermit)) {
            if(kermit_feed_serial_data(
                   instance->kermit,
                   (const uint8_t*)furi_string_get_cstr(instance->rx_string),
                   furi_string_size(instance->rx_string))) {
                furi_string_reset(instance->rx_string);
            } else {
                FURI_LOG_E(TAG, "Kermit error");
                should_stop = true;
            }
        } else {
            FURI_LOG_I(TAG, "Kermit upload complete");
            instance->bootloader_state = Si917BootloaderStateWaitInstall;
        }
        break;

    case Si917BootloaderStateWaitInstall:
        if(sl_updater_check_rx_for(instance, "Upgradation Successful")) {
            FURI_LOG_W(TAG, "Install success");
            should_stop = true;
            instance->bootloader_state = Si917BootloaderStateInstallSuccess;
        } else if(sl_updater_check_rx_for(instance, "Upgradation Failed")) {
            FURI_LOG_E(TAG, "Install failed");
            should_stop = true;
            instance->bootloader_state = Si917BootloaderStateInstallFail;
        }
        break;

    case Si917BootloaderStateInstallSuccess:
    case Si917BootloaderStateInstallFail:
        // Do nothing
        should_stop = true;
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

static void sl_updater_rx_buffer_callback(FuriEventLoopObject* object, void* context) {
    SlUpdater* instance = context;
    furi_check(object == instance->rx_buffer);

    // Reset the idle timer
    furi_event_loop_timer_restart(instance->idle_timer);

    char c;
    while(furi_stream_buffer_bytes_available(instance->rx_buffer)) {
        furi_check(furi_stream_buffer_receive(instance->rx_buffer, &c, sizeof(c), 0) == sizeof(c));
        furi_string_push_back(instance->rx_string, c);
    }

    sl_updater_handle_rx(instance);
}

static void sl_updater_intercom_error_callback(IntercomError error, void* context) {
    UNUSED(error);
    UNUSED(context);
    // Empty callback
}

static void sl_update_idle_timer_callback(void* context) {
    SlUpdater* instance = context;

    FURI_LOG_W(TAG, "Watchdog expired");
    furi_event_loop_stop(instance->event_loop);
}

SlUpdater* sl_updater_alloc(void) {
    FURI_LOG_I(TAG, "Starting SL Update Test App");

    SlUpdater* instance = malloc(sizeof(SlUpdater));

    instance->event_loop = furi_event_loop_alloc();
    instance->rx_buffer = furi_stream_buffer_alloc(512, 1);
    instance->rx_string = furi_string_alloc();
    instance->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart2);

    instance->bootloader_state = Si917BootloaderStateInit;
    instance->timeout_seconds = 1;

    instance->kermit = kermit_alloc(&kermit_io, instance);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->firmware_file = storage_file_alloc(instance->storage);
    instance->idle_timer = furi_event_loop_timer_alloc(
        instance->event_loop,
        sl_update_idle_timer_callback,
        FuriEventLoopTimerTypePeriodic,
        instance);

    furi_event_loop_subscribe_stream_buffer(
        instance->event_loop,
        instance->rx_buffer,
        FuriEventLoopEventIn,
        sl_updater_rx_buffer_callback,
        instance);

    return instance;
}

void sl_updater_free(SlUpdater* instance) {
    furi_event_loop_timer_free(instance->idle_timer);
    furi_event_loop_unsubscribe(instance->event_loop, instance->rx_buffer);

    furi_string_free(instance->rx_string);
    furi_stream_buffer_free(instance->rx_buffer);
    furi_event_loop_free(instance->event_loop);

    kermit_free(instance->kermit);
    storage_file_free(instance->firmware_file);
    furi_record_close(RECORD_STORAGE);

    FURI_LOG_I(TAG, "SL Updater stopped");

    free(instance);
}

bool sl_updater_run(
    SlUpdater* instance,
    const char* firmware_path,
    bool is_stack_image,
    uint8_t timeout_seconds) {
    furi_check(instance);
    furi_check(firmware_path);

    instance->timeout_seconds = timeout_seconds;
    instance->is_stack_image = is_stack_image;

    instance->bootloader_state = Si917BootloaderStateInit;

    if(!storage_file_open(instance->firmware_file, firmware_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open firmware file: %s", firmware_path);
        return false;
    }

#ifdef SRV_INTERCOM
    // Prevent crashes
    instance->intercom = furi_record_open(RECORD_INTERCOM);
    intercom_set_error_callback(instance->intercom, sl_updater_intercom_error_callback, NULL);
#else
    UNUSED(sl_updater_intercom_error_callback);
#endif

    // Start idle timer
    furi_event_loop_timer_start(
        instance->idle_timer, furi_ms_to_ticks((instance->timeout_seconds + 2) * 1000));

    furi_hal_serial_init(instance->serial_handle, 115200);
    furi_hal_serial_set_callback(
        instance->serial_handle, NULL, sl_updater_serial_irq_callback, instance);
    furi_hal_serial_async_rx_start(instance->serial_handle, false);

    furi_hal_power_reset_917(true);

    const uint8_t leader = 0;
    furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader));

    furi_event_loop_run(instance->event_loop);

    bool success = instance->bootloader_state == Si917BootloaderStateInstallSuccess;

    furi_hal_power_reset_917(false);

#ifdef SRV_INTERCOM
    // TODO: The ability to reset intercom
    intercom_set_error_callback(instance->intercom, NULL, NULL);
    furi_record_close(RECORD_INTERCOM);
#endif

    furi_hal_serial_async_rx_stop(instance->serial_handle);
    furi_hal_serial_set_callback(instance->serial_handle, NULL, NULL, NULL);
    furi_hal_serial_control_release(instance->serial_handle);

    storage_file_close(instance->firmware_file);

    return success;
}
