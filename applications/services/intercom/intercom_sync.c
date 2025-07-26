#include "intercom_i.h"

#define INTERCOM_SYNC_LEADER_1 (0x55)
#define INTERCOM_SYNC_LEADER_2 (0xAA)

#define INTERCOM_SYNC_INTERVAL_1_MS (5UL)
#define INTERCOM_SYNC_INTERVAL_2_MS (1UL)

#define INTERCOM_SYNC_CHAR_TIMEOUT_MS (1000UL)
#define INTERCOM_SYNC_TIMEOUT_MS      (1000UL)

typedef struct {
    uint8_t leader;
    uint32_t interval;
    bool repeat_leader;
} IntercomSyncSequence;

static const IntercomSyncSequence intercom_sync_sequences[] = {
    // Sequence 1 - ensure that the target is responding
    {
        .leader = INTERCOM_SYNC_LEADER_1,
        .interval = INTERCOM_SYNC_INTERVAL_1_MS,
        .repeat_leader = true,
    },
    // Sequence 2 - ensure that data streams are in sync
    {
        .leader = INTERCOM_SYNC_LEADER_2,
        .interval = INTERCOM_SYNC_INTERVAL_2_MS,
        .repeat_leader = false,
    },
};

/**
 * Basic principle of operation:
 *
 * 1. Send a leader character,
 * 2. Wait for received data,
 * 3. If the received character is a leader character, report success.
 * 4. If no characters have been received, retry until a leader character is detected or the timeout expires.
 * 5. Depending on the repeat_leader flag, send the leader repeatedly when retrying, or do it only in the beginning.
 */
static bool
    intercom_sync_sequence(FuriHalSerialHandle* serial, const IntercomSyncSequence* sequence) {
    bool success = false;
    bool send_leader = true;

    const uint32_t start_time = furi_get_tick();

    while(furi_get_tick() - start_time < furi_ms_to_ticks(INTERCOM_SYNC_TIMEOUT_MS)) {
        if(send_leader) {
            if(furi_hal_serial_tx(serial, &sequence->leader, 1, INTERCOM_SYNC_CHAR_TIMEOUT_MS) !=
               1) {
                break;
            }
            if(!furi_hal_serial_tx_wait_complete(serial, INTERCOM_SYNC_CHAR_TIMEOUT_MS)) {
                break;
            }
            // If repeat_leader is false, then the leader is sent only once
            if(!sequence->repeat_leader) {
                send_leader = false;
            }
        }

        if(furi_hal_serial_rx_available(serial)) {
            if(furi_hal_serial_rx(serial) == sequence->leader) {
                success = true;
                break;
            }
        }

        furi_delay_ms(sequence->interval);
    }

    return success;
}

/**
 * The synchronisation procedure lets 2 devices agree on where the beginning of the data stream is.
 *
 * This method has the following benefits:
 * - Rejection of any erroneous data that might have been accidentally sent during configuration.
 * - Independence of the startup timing. Both sides only must be brought up within the timeout period.
 *
 * Basic operating principle:
 *
 * 1. Send the 1st sequence leader character REPEATEDLY until the other side responds with it.
 * 2. Send the 2nd sequence leader character ONCE until the other side responds with it.
 * 3. Once the 2nd sequence leader character has been received, the data streams are synchronised.
 * 4. If any of the above procedures could not be completed within a timeout, report an error.
 *
 * Example time diagram:
 *
 * 1st sequence leader char = 'A'
 * 2nd sequence leader char = 'B'
 *
 * SIDE A | TX | (startup) AAAAAAAAAAAAAAAB
 *        | RX | (garbage or silence)    AB(sync!)
 * -------+----+----------------------------------
 * SIDE B | TX | (startup)               AB
 *        | RX | (garbage or silence)    AB(sync!)
 * -------+----+----------------------------------
 *               TIME ->
 */
bool intercom_sync_serial(FuriHalSerialHandle* serial) {
    bool success = true;

    for(uint32_t i = 0; i < COUNT_OF(intercom_sync_sequences); ++i) {
        if(!intercom_sync_sequence(serial, &intercom_sync_sequences[i])) {
            success = false;
            break;
        }
    }

    return success;
}

void intercom_sync_request(const GpioPin* gpio) {
    furi_hal_gpio_init(gpio, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write_open_drain(gpio, false);
    furi_delay_us(10);
    furi_hal_gpio_write_open_drain(gpio, true);
    furi_hal_gpio_init(gpio, GpioModeInput, GpioPullNo, GpioSpeedLow);
}
