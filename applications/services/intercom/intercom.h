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
    IntercomChannelInput, /**< Input handling */
    IntercomChannelWifi, /**< Wireless network handling */
    IntercomChannelWifiData, /**< Wireless network data handling */
    IntercomChannelStatusLights, /**< Status lights handling */
    IntercomChannelCli, /**< Command line interface handling */
    IntercomChannelBle, /**< BLE handling */
    IntercomChannelCryptoBackup, /**< Crypto backup handling */
    IntercomChannelMatter, /**< Matter smart home protocol */
    /* Add more channels here as needed */
    IntercomChannelDebug = 15, /**< Testing, debugging, etc */
    IntercomChannelMax, /**< Special value for internal use */
} IntercomChannel;

/**
 * @brief Opaque channel handle
 */
typedef struct IntercomChHandle IntercomChHandle;

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
 * This function blocks until code on the other chip opens a channel with the
 * same ID. Once it does, this function returns on both chips.
 * 
 * @warning You must not call this function from multiple threads on the
 *          same chip.
 * 
 * @param[in] instance Intercom instance
 * @param[in] ch_id Channel ID
 * @param[in] timeout Timeout for waiting for the other chip, in ticks. If this
 *                    timeout is exceeded, the operation will still succeed, but
 *                    there will be no guarantee that the other chip has opened
 *                    the channel and is ready to accept frames.
 * @param[in] rx_callback Data reception callback. May be NULL if reception is
 *                        not required.
 * @param[in] context Context for provided callback. May be NULL if rx_callback
 *                    is NULL.
 * 
 * @returns Channel handle
 */
IntercomChHandle* intercom_channel_open(
    Intercom* instance,
    IntercomChannel ch_id,
    FuriWait timeout,
    IntercomRxCallback rx_callback,
    void* context);

/**
 * @brief Closes an Intercom channel
 * 
 * Nothing is signaled to the other chip. However, with this you can open the
 * channel on the same chip again.
 * 
 * Additional calls to `intercom_channel_open` won't notify the other chip
 * either. Frames received in between this `close` and the next `open` will be
 * silently discarded.
 * 
 * @param[in] handle Intercom channel handle
 */
void intercom_channel_close(IntercomChHandle* handle);

/**
 * @brief Transmit data through Intercom.
 *
 * The incoming data will be automatically split into frames and sent asynchronously.
 *
 * @param[in] handle Pointer to the acquired Intercom handle
 * @param[in] data Pointer to the data to send
 * @param[in] data_size Number of bytes to send
 * @param[in] timeout Time to wait for the transmission to complete, in milliseconds
 * @returns number of bytes that were actually transmitted before timeout
 */
size_t intercom_tx(IntercomChHandle* handle, const void* data, size_t data_size, uint32_t timeout);

#ifdef __cplusplus
}
#endif
