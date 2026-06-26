#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file fkermit.h
 * @brief Minimal send-only Kermit file-transfer sender.
 *
 * Implements the sender side of the Kermit protocol used to push firmware
 * images to a device bootloader over a byte-oriented link (UART). File data is
 * transmitted and the receiver's ACK/NAK packets are consumed; no file data is
 * ever received. The protocol logic is fully decoupled from hardware through
 * the KermitIo interface.
 *
 * Kermit protocol reference: https://www.kermitproject.org/kpackets.html
 */

/**
 * @brief I/O interface used by the Kermit sender to read the source file and
 *        transmit data over the communication channel.
 *
 * Every callback receives the user-provided context unchanged. The pointed-to
 * @ref KermitIo instance must outlive any session created from it.
 */
typedef struct {
    /**
     * @brief Read up to @p length bytes of source data into @p buffer.
     * @param[in] context User-provided context.
     * @param[out] buffer Destination buffer.
     * @param[in] length Number of bytes to read.
     * @return Number of bytes read; 0 indicates end of file.
     */
    int32_t (*src_file_read)(void* context, uint8_t* buffer, size_t length);
    /**
     * @brief Send @p length bytes over the communication channel.
     * @param[in] context User-provided context.
     * @param[in] buffer Data to send.
     * @param[in] length Number of bytes to send.
     * @return Number of bytes actually sent.
     */
    int32_t (*comms_send)(void* context, const uint8_t* buffer, size_t length);
} KermitIo;

/**
 * @brief Opaque Kermit session handle.
 */
typedef struct Kermit Kermit;

/**
 * @brief Allocate a new Kermit session in the idle state.
 * @param[in] io I/O interface used by the sender. Must not be NULL.
 * @param[in] context User-provided context passed back to every I/O callback.
 * @return Pointer to the newly allocated session.
 */
Kermit* kermit_alloc(const KermitIo* io, void* context);

/**
 * @brief Free a Kermit session and release any pending receive buffer.
 * @param[in] kermit Session handle. Must not be NULL.
 */
void kermit_free(Kermit* kermit);

/**
 * @brief Start a file transfer by transmitting the Send-Init (S) packet.
 *
 * Must be called while the session is idle. The transfer is then driven by
 * feeding the receiver's responses with kermit_feed_serial_data().
 *
 * @param[in] kermit Session handle.
 * @param[in] timeout_seconds Per-packet timeout advertised to the receiver
 *                            (Kermit TIMO field), in seconds.
 * @return true if the Send-Init packet was transmitted, false otherwise.
 */
bool kermit_start(Kermit* kermit, const uint8_t timeout_seconds);

/**
 * @brief Check whether the transfer has completed.
 * @param[in] kermit Session handle.
 * @return true if the transfer is still in progress or has not started yet;
 *         false once it has completed.
 */
bool kermit_is_active(Kermit* kermit);

/**
 * @brief Feed received serial data into the session state machine.
 *
 * Typically carries the receiver's ACK/NAK packets. Non-Kermit bytes are
 * skipped until a packet mark is seen, so interleaved bootloader text is
 * tolerated.
 *
 * @param[in] kermit Session handle.
 * @param[in] data Received bytes. Must not be NULL.
 * @param[in] length Number of bytes. Must be greater than 0.
 * @return true if the data was processed successfully. If false is returned,
 *         the session state is invalid and the transfer must be aborted.
 */
bool kermit_feed_serial_data(Kermit* kermit, const uint8_t* data, size_t length);

/**
 * @brief Reset the session back to the idle state.
 *
 * Resets the sequence counter, frees any in-progress receive buffer, and moves
 * the file-transfer state machine to idle. Safe to call at any time.
 * @param[in] kermit Session handle.
 */
void kermit_reset_state(Kermit* kermit);

#ifdef __cplusplus
}
#endif
