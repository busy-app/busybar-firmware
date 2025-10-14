/**
 * @file intercom.h
 * @brief Chip-to-chip intercommunication via U(S)ART service
 */
#pragma once

#include <furi.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =======================
// Channel-nonspecific API
// =======================

/**
 * @brief Intercom FURI record identifier.
 */
#define RECORD_INTERCOM "intercom"

/**
 * @brief Opaque Intercom type declaration.
 */
typedef struct Intercom Intercom;

/**
 * @brief Enumeration of possible errors.
 */
typedef enum {
    IntercomErrorSync, /**< Other side has requested synchronization, which failed */
    IntercomErrorFraming, /**< Invalid frame (incorrect structure or checksum) */
    IntercomErrorTransmit, /**< Transmission has been inhibited for too long by HW */
} IntercomError;

typedef enum {
    IntercomEventTypeError, /**< Error event */
    IntercomEventTypeSyncStateChanged, /**< Sync state changed event */
} IntercomEventType;

typedef struct {
    IntercomEventType type; /**< Type of the event */
    union {
        const char* message; /**< Optional message, if applicable */
        bool is_in_sync; /**< New sync state */
    };
} IntercomEvent;

/**
 * @brief Enable error handling. 
 *        If enabled, the Intercom service will call the error callback and can crash on errors.
 *
 * @param[in,out] instance Pointer to the Intercom instance
 */
void intercom_error_handling_enable(Intercom* instance);

/**
 * @brief Disable error handling.
 *        If disabled, the Intercom service will not call the error callback and will not crash on errors.
 *
 * @param[in,out] instance Pointer to the Intercom instance
 */
void intercom_error_handling_disable(Intercom* instance);

/**
 * @brief Get the Intercom PubSub instance.
 *
 * @param[in,out] instance Pointer to the Intercom instance
 * @returns Pointer to the FuriPubSub instance
 */
FuriPubSub* intercom_get_pubsub(Intercom* instance);

/**
 * @brief Check if the intercom is synced and ready for communication.
 *
 * @param[in] instance Pointer to the Intercom instance
 * @returns true if synced and ready, false otherwise
 */
bool intercom_is_in_sync(Intercom* instance);

// ===========
// Channel API
// ===========

/**
 * @brief Enumeration of available channel identifiers.
 */
typedef enum {
    IntercomChannelIdInput, /**< Input handling */
    IntercomChannelIdWifi, /**< Wireless network handling */
    IntercomChannelIdWifiData, /**< Wireless network data handling */
    IntercomChannelIdStatusLights, /**< Status lights handling */
    IntercomChannelIdCli, /**< Command line interface handling */
    IntercomChannelIdBle, /**< BLE handling */
    IntercomChannelIdCryptoBackup, /**< Crypto backup handling */
    IntercomChannelIdMatter, /**< Matter smart home protocol */
    /* Add more channels here as needed */
    IntercomChannelIdDebug = 15, /**< Testing, debugging, etc */
    IntercomChannelIdMax, /**< Special value for internal use */
} IntercomChannelId;

/**
 * @brief Opaque channel handle
 */
typedef struct IntercomChannel IntercomChannel;

/**
 * @brief Receive callback function type.
 *
 * @param[in] data Pointer to the received data
 * @param[in] data_size Number of bytes received
 * @param[in,out] context Pointer to a user-specified context object
 */
typedef void (*IntercomRxCallback)(const void* data, size_t data_size, void* context);

/**
 * @brief Open an Intercom channel
 * 
 * Signals to the other chip that you are ready to accept messages on the
 * specified channel.
 * 
 * You may pass `NULL` to `rx_callback`, meaning you don't expect any data to
 * arrive from the other side. If the other side does end up sending data to
 * you, the system will crash with a clear message.
 * 
 * @warning You must not call this function from multiple threads on the
 *          same chip.
 * 
 * @param[in] instance Intercom instance
 * @param[in] ch_id Channel ID
 * @param[in] rx_callback Data reception callback. May be NULL.
 * @param[in] context Context for provided callback. May be NULL if rx_callback
 *                    is NULL.
 * 
 * @returns Channel handle
 */
IntercomChannel* intercom_channel_open(
    Intercom* instance,
    IntercomChannelId channel_id,
    IntercomRxCallback rx_callback,
    void* context);

/**
 * @brief Transmit data through Intercom.
 * 
 * The data will be automatically split into frames and sent asynchronously.
 * 
 * Blocks until the other chip signals that it's ready to accept messages on
 * this channel.
 * 
 * @param[in] handle Pointer to the acquired Intercom channel handle
 * @param[in] data Pointer to the data to send
 * @param[in] data_size Number of bytes to send
 * @param[in] timeout Time to wait for the transmission to complete, in milliseconds
 * @returns number of bytes that were actually transmitted before timeout
 */
size_t intercom_tx(IntercomChannel* channel, const void* data, size_t data_size, uint32_t timeout);

#ifdef __cplusplus
}
#endif
