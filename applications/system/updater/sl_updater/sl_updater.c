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
#ifdef SRV_LOG_STORAGE
#include <log_storage/log_storage.h>
#endif

#define TAG "SlUpdater"

#include "../helpers/fkermit.h"

typedef enum {
    Si917BootloaderModeDefault,
    Si917BootloaderModeProbe,
} Si917BootloaderMode;

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
    FuriString* bootloader_version;
    FuriHalSerialHandle* serial_handle;
    FuriEventLoopTimer* idle_timer;
#ifdef SRV_INTERCOM
    Intercom* intercom;
#endif
    uint8_t install_timeout_seconds;
    bool is_stack_image;
    uint8_t baud_throttle;
    Si917BootloaderState bootloader_state;
    Si917BootloaderMode bootloader_mode;
    Storage* storage;
    File* firmware_file;
    Kermit* kermit;
    SlUpdaterProgressCallback progress_callback;
    void* progress_callback_context;
};

#define KERMIT_TIMEOUT_SECONDS               (2)
#define SL_UPDATER_AUTO_BAUD_RATE_TIMEOUT_MS (13000U) // 13s
#define EVENT_LOOP_TICK_PERIOD_MS            (100)

typedef struct {
    const char* choice;
    uint32_t baudrate;
} SlUpdaterBaudeRate;

static SlUpdaterBaudeRate const sl_updater_baudrate[] = {
    {"4", 921600},
    {"3", 460800},
    {"2", 230400},
    {"5", 115200},
    {"1", 38400},
    {"0", 9600},
};

//////////////////////////////////////////////////////////////////////////
// Kermit i/o functions

static int32_t kermit_src_file_read(void* context, uint8_t* buffer, size_t length) {
    SlUpdater* app = context;
    int32_t bytes_read = storage_file_read(app->firmware_file, buffer, length);

    if(app->progress_callback && bytes_read > 0) {
        uint32_t file_size = storage_file_size(app->firmware_file) + 1;
        uint32_t file_pos = storage_file_tell(app->firmware_file);
        if(file_size > 0) {
            uint8_t percentage = (uint8_t)((file_pos * 100) / file_size);
            app->progress_callback(
                SlUpdaterProgressPhaseUploading, percentage, app->progress_callback_context);
        }
    }
    return bytes_read;
}

static int32_t kermit_comms_send(void* context, const uint8_t* buffer, size_t length) {
    SlUpdater* app = context;
    FURI_LOG_T(TAG, "Sending %d bytes", length);

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

    furi_hal_serial_tx(app->serial_handle, buffer, length, FuriWaitForever);
    return length;
}

static const KermitIo kermit_io = {
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

static bool sl_update_get_version(SlUpdater* instance) {
    size_t pos = furi_string_search_str(instance->rx_string, "BootLoader Version");
    if(pos == FURI_STRING_FAILURE) return false;

    furi_string_right(instance->rx_string, pos);
    pos = furi_string_search_rchar(instance->rx_string, '\r');

    if(pos == FURI_STRING_FAILURE) return false;

    furi_string_left(instance->rx_string, pos);
    furi_string_set(instance->bootloader_version, instance->rx_string);
    return true;
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
            furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader), FuriWaitForever);
            FURI_LOG_I(TAG, "Leader sent: %c", leader);
            instance->bootloader_state = Si917BootloaderStateBoot;
        }
        break;

    case Si917BootloaderStateBoot:
        if((instance->bootloader_mode == Si917BootloaderModeProbe) &&
           (sl_update_get_version(instance))) {
            FURI_LOG_I(TAG, "Probe success");
            instance->bootloader_state = Si917BootloaderStateInstallSuccess;
        } else {
            if(sl_updater_check_rx_for(instance, "Change UART Baud Rate\r\n")) {
                const uint8_t choice = 'b';
                furi_hal_serial_tx(
                    instance->serial_handle, &choice, sizeof(choice), FuriWaitForever);
                FURI_LOG_I(TAG, "UART Baud Rate change request sent: %c", choice);
                instance->bootloader_state = Si917BootloaderStateChangeBaudRate;
            }
        }
        break;

    case Si917BootloaderStateChangeBaudRate:
        if(sl_updater_check_rx_for(instance, "5 115200\r\n")) {
            furi_hal_serial_tx(
                instance->serial_handle,
                (uint8_t*)sl_updater_baudrate[instance->baud_throttle].choice,
                1,
                FuriWaitForever);
            FURI_LOG_I(
                TAG,
                "UART Baud Rate speed request sent: %s",
                sl_updater_baudrate[instance->baud_throttle].choice);

            furi_hal_serial_set_baud_rate(
                instance->serial_handle, sl_updater_baudrate[instance->baud_throttle].baudrate);
            FURI_LOG_I(
                TAG,
                "Reference baud set %ld",
                furi_hal_serial_get_baud_rate(instance->serial_handle));
            const uint8_t leader = 'U';
            furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader), FuriWaitForever);
            if(furi_hal_serial_set_auto_baud_rate(
                   instance->serial_handle,
                   FuriHalSerialAutoBaudRateMode0x55Frame,
                   SL_UPDATER_AUTO_BAUD_RATE_TIMEOUT_MS)) {
                FURI_LOG_I(
                    TAG,
                    "Baud rate auto set %ld",
                    furi_hal_serial_get_baud_rate(instance->serial_handle));
            } else {
                FURI_LOG_E(TAG, "Baud rate auto set failed");
            }
            instance->bootloader_state = Si917BootloaderStateChangeBaudRateNewLeader;
        }
        break;

    case Si917BootloaderStateChangeBaudRateNewLeader:
        const uint8_t leader = 'U';
        furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader), FuriWaitForever);
        FURI_LOG_I(TAG, "New Leader sent: %c", leader);
        // fall through

    case Si917BootloaderStateChangeBaudRateSuccess:
        if(sl_updater_check_rx_for(instance, "Baud Rate was updated successfully!")) {
            FURI_LOG_I(
                TAG,
                "BL baud rate set to %ld",
                sl_updater_baudrate[instance->baud_throttle].baudrate);
            instance->bootloader_state = Si917BootloaderStateSetImageType;
        }
        break;

    case Si917BootloaderStateSetImageType:
        if(sl_updater_check_rx_for(instance, "Enter Next Command")) {
            furi_delay_ms(10);
            const uint8_t image_type = instance->is_stack_image ? 'B' : '4';
            furi_hal_serial_tx(
                instance->serial_handle, &image_type, sizeof(image_type), FuriWaitForever);

            FURI_LOG_I(TAG, "Image type set to: %c", image_type);
            instance->bootloader_state = Si917BootloaderStateSetImageSlot;
        }
        break;

    case Si917BootloaderStateSetImageSlot:
        const char* needle = instance->is_stack_image ? "Enter Wireless Image No(0-f)" :
                                                        "Enter M4 Image No(1-f)";
        if(sl_updater_check_rx_for(instance, needle)) {
            const uint8_t image_slot = instance->is_stack_image ? '0' : '1';
            furi_hal_serial_tx(
                instance->serial_handle, &image_slot, sizeof(image_slot), FuriWaitForever);

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
            kermit_start(instance->kermit, KERMIT_TIMEOUT_SECONDS);
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

            if(instance->progress_callback) {
                instance->progress_callback(
                    SlUpdaterProgressPhaseAwaitingInstall, 0, instance->progress_callback_context);
            }
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

static void sl_update_idle_timer_callback(void* context) {
    SlUpdater* instance = context;
    if(instance->bootloader_state == Si917BootloaderStateWaitInstall) {
        // First time we timeout, we update the timer to an actual install timeout
        uint32_t desired_timeout = furi_ms_to_ticks(instance->install_timeout_seconds * 1000);
        if(furi_event_loop_timer_get_interval(instance->idle_timer) != desired_timeout) {
            furi_event_loop_timer_start(instance->idle_timer, desired_timeout);
            FURI_LOG_I(
                TAG, "Setting install timeout to %d seconds", instance->install_timeout_seconds);
            return;
        }
    }

    FURI_LOG_W(TAG, "Watchdog expired");
    furi_event_loop_stop(instance->event_loop);
}

static void sl_updater_tick(void* context) {
    SlUpdater* instance = context;

    if(instance->bootloader_state == Si917BootloaderStateWaitInstall) {
        size_t expected_interval = furi_ms_to_ticks(instance->install_timeout_seconds * 1000);
        size_t interval = furi_event_loop_timer_get_interval(instance->idle_timer);
        if(interval != expected_interval) return;

        size_t remaining = furi_event_loop_timer_get_remaining_time(instance->idle_timer);
        size_t passed = interval - remaining;

        instance->progress_callback(
            SlUpdaterProgressPhaseAwaitingInstall,
            (passed * 100) / interval,
            instance->progress_callback_context);
    }
}

SlUpdater* sl_updater_alloc(void) {
    FURI_LOG_I(TAG, "Starting");

    SlUpdater* instance = malloc(sizeof(SlUpdater));

    instance->event_loop = furi_event_loop_alloc();
    instance->rx_buffer = furi_stream_buffer_alloc(512, 1);
    instance->rx_string = furi_string_alloc();
    instance->serial_handle = NULL;
    instance->baud_throttle = 0;

    instance->bootloader_state = Si917BootloaderStateInit;
    instance->install_timeout_seconds = 1;

    instance->kermit = kermit_alloc(&kermit_io, instance);
    instance->storage = furi_record_open(RECORD_STORAGE);
    instance->firmware_file = storage_file_alloc(instance->storage);
    instance->progress_callback = NULL;
    instance->progress_callback_context = NULL;
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

    furi_event_loop_tick_set(
        instance->event_loop,
        furi_ms_to_ticks(EVENT_LOOP_TICK_PERIOD_MS),
        sl_updater_tick,
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

    free(instance);
    FURI_LOG_I(TAG, "Stopped");
}

void sl_updater_set_progress_callback(
    SlUpdater* instance,
    SlUpdaterProgressCallback callback,
    void* context) {
    furi_check(instance);
    instance->progress_callback = callback;
    instance->progress_callback_context = context;
}

static SlUpdaterStatus
    sl_update_inner_run(SlUpdater* instance, Si917BootloaderMode mode, uint8_t baud_throttle) {
    if(baud_throttle < COUNT_OF(sl_updater_baudrate)) {
        instance->baud_throttle = baud_throttle;
    } else {
        instance->baud_throttle = COUNT_OF(sl_updater_baudrate);
    }

    kermit_reset_state(instance->kermit);
    // We start with the default timeout since it's enough for non-install operations
    furi_event_loop_timer_start(
        instance->idle_timer, furi_ms_to_ticks((KERMIT_TIMEOUT_SECONDS + 1) * 1000));

    instance->bootloader_mode = mode;
    instance->bootloader_state = Si917BootloaderStateInit;

#ifdef SRV_LOG_STORAGE
    LogStorage* log_storage = furi_record_open(RECORD_LOG_STORAGE);
    log_storage_suspend_remote(log_storage);
#endif

    instance->serial_handle = furi_hal_serial_control_acquire(FuriHalSerialIdUsart2);

    furi_hal_serial_init(instance->serial_handle, 115200);
    furi_hal_serial_set_rx_callback(
        instance->serial_handle, sl_updater_serial_irq_callback, instance);
    furi_hal_serial_async_rx_start(instance->serial_handle, false);

    furi_hal_power_reset_917(true);

    const uint8_t leader = 0;
    furi_hal_serial_tx(instance->serial_handle, &leader, sizeof(leader), FuriWaitForever);

    furi_event_loop_run(instance->event_loop);

    SlUpdaterStatus status = (instance->bootloader_state == Si917BootloaderStateInstallSuccess) ?
                                 SlUpdaterStatusOk :
                                 SlUpdaterStatusFailure;

    furi_hal_power_reset_917(false);

    furi_hal_serial_async_rx_stop(instance->serial_handle);
    furi_hal_serial_set_rx_callback(instance->serial_handle, NULL, NULL);
    furi_hal_serial_control_release(instance->serial_handle);
    instance->serial_handle = NULL;

#ifdef SRV_LOG_STORAGE
    log_storage_resume_remote(log_storage);
    furi_record_close(RECORD_LOG_STORAGE);
#endif

    return status;
}

SlUpdaterStatus sl_update_probe(SlUpdater* instance, uint8_t baud_throttle, FuriString* version) {
    furi_check(instance);
    furi_assert(version);
    furi_assert(baud_throttle <= COUNT_OF(sl_updater_baudrate));

    instance->bootloader_version = version;
    return sl_update_inner_run(instance, Si917BootloaderModeProbe, baud_throttle);
}

SlUpdaterStatus sl_updater_run(
    SlUpdater* instance,
    const char* firmware_path,
    bool is_stack_image,
    uint8_t install_timeout_seconds,
    uint8_t baud_throttle) {
    furi_check(instance);
    furi_check(firmware_path);
    furi_check(instance->serial_handle == NULL);
    furi_assert(baud_throttle <= COUNT_OF(sl_updater_baudrate));

    instance->is_stack_image = is_stack_image;
    instance->install_timeout_seconds = install_timeout_seconds;

    if(!storage_file_open(instance->firmware_file, firmware_path, FSAM_READ, FSOM_OPEN_EXISTING)) {
        FURI_LOG_E(TAG, "Failed to open firmware file");
        return SlUpdaterStatusFileNotFound;
    }

    SlUpdaterStatus status =
        sl_update_inner_run(instance, Si917BootloaderModeDefault, baud_throttle);
    storage_file_close(instance->firmware_file);
    return status;
}
