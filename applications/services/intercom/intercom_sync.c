/**
 * @file intercom_sync.c
 */
#include "intercom_i.h"

#ifdef INTERCOM_DISABLE_VERSION_CHECK
#define INTERCOM_CONTROL_STRING_DEFAULT "intercom"
#else // INTERCOM_DISABLE_VERSION_CHECK
#include <version.h>
#endif // INTERCOM_DISABLE_VERSION_CHECK

#define INTERCOM_SYNC_TIMEOUT_MS (2000)

#define INTERCOM_SYNC_DEBOUNCE_WINDOW_MS          (10)
#define INTERCOM_SYNC_DEBOUNCE_SAMPLE_INTERVAL_MS (1)
#define INTERCOM_SYNC_DEBOUNCE_CONFIDENCE_THRESHOLD \
    (INTERCOM_SYNC_DEBOUNCE_WINDOW_MS / INTERCOM_SYNC_DEBOUNCE_SAMPLE_INTERVAL_MS)

#define TAG "IntercomSync"

static const char* intercom_get_control_string(void) {
    const char* str;
#ifdef INTERCOM_DISABLE_VERSION_CHECK
    str = INTERCOM_CONTROL_STRING_DEFAULT;
#else // INTERCOM_DISABLE_VERSION_CHECK
    const Version* version = version_get();
    str = version_get_githash(version);
#endif // INTERCOM_DISABLE_VERSION_CHECK
    return str;
}

static uint32_t intercom_sync_get_timeout_left(uint32_t start_time) {
    const uint32_t dt = furi_get_tick() - start_time;
    return dt < INTERCOM_SYNC_TIMEOUT_MS ? INTERCOM_SYNC_TIMEOUT_MS - dt : 0;
}

static bool intercom_sync_wait_for_other_side(FuriHalSerialHandle* serial, uint32_t start_time) {
    bool success = false;

    int32_t confidence = 0;

    while(intercom_sync_get_timeout_left(start_time)) {
        const int32_t delta = furi_hal_serial_get_pin_state(serial, FuriHalSerialPinCts) ? -1 : 1;

        confidence = MAX(confidence + delta, 0);

        if(confidence >= INTERCOM_SYNC_DEBOUNCE_CONFIDENCE_THRESHOLD) {
            success = true;
            break;
        }

        furi_delay_ms(INTERCOM_SYNC_DEBOUNCE_SAMPLE_INTERVAL_MS);
    }

    return success;
}

static bool intercom_sync_send_char(FuriHalSerialHandle* serial, uint8_t ch, uint32_t start_time) {
    bool success = false;

    do {
        const size_t tx_size = furi_hal_serial_tx(
            serial, &ch, sizeof(ch), intercom_sync_get_timeout_left(start_time));

        if(tx_size != sizeof(ch)) {
            break;
        }

        if(!furi_hal_serial_tx_wait_complete(serial, intercom_sync_get_timeout_left(start_time))) {
            break;
        }

        success = true;
    } while(false);

    return success;
}

static bool
    intercom_sync_wait_for_char(FuriHalSerialHandle* serial, uint8_t ch, uint32_t start_time) {
    bool success = false;

    while(intercom_sync_get_timeout_left(start_time)) {
        if(furi_hal_serial_rx_available(serial) && furi_hal_serial_rx(serial) == ch) {
            success = true;
            break;
        }
    }

    return success;
}

static bool intercom_sync_do_handshake(FuriHalSerialHandle* serial, uint32_t start_time) {
    const char* control_str = intercom_get_control_string();
    const size_t control_str_len = strlen(control_str);

    uint32_t i;
    for(i = 0; i < control_str_len; ++i) {
        if(!intercom_sync_send_char(serial, control_str[i], start_time)) {
            break;
        }
        if(!intercom_sync_wait_for_char(serial, control_str[i], start_time)) {
            break;
        }
    }

    return (i == control_str_len);
}

bool intercom_sync_serial(FuriHalSerialHandle* serial) {
    bool success = false;

    do {
        const uint32_t start_time = furi_get_tick();

        if(!intercom_sync_wait_for_other_side(serial, start_time)) {
            FURI_LOG_E(TAG, "No presence signal from the other side");
            break;
        }
        if(!intercom_sync_do_handshake(serial, start_time)) {
            FURI_LOG_E(TAG, "Handshake failure, possible version mismatch");
            break;
        }

        success = true;
    } while(false);

    return success;
}
