/**
 * @file intercom.h
 * @brief Chip-to-chip intercommunication via U(S)ART service
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <furi/core/pubsub.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Intercom FURI record identifier.
 */
#define RECORD_INTERCOM "intercom"

/**
 * @brief Opaque Intercom type declaration.
 */
typedef struct Intercom Intercom;

/**
 * @brief Enumeration of available channel identifiers.
 */
typedef enum {
    IntercomChannelInput, /**< Input handling */
    IntercomChannelWifi, /**< Wireless network handling */
    IntercomChannelSockets, /**< Network sockets handling */
    IntercomChannelStatusLights, /**< Status lights handling */
    IntercomChannelCli, /**< Command line interface handling */
    IntercomChannelCryptoBackup, /**< Crypto backup handling */
    /* Add more channels here as needed */
    IntercomChannelDebug = 15, /**< Testing, debugging, etc */
    IntercomChannelMax, /**< Special value for internal use */
} IntercomChannel;

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
} IntercomEventType;

typedef struct {
    IntercomEventType type; /**< Type of the event */
    union {
        const char* message; /**< Optional message, if applicable */
    };
} IntercomEvent;

/**
 * @brief Receive callback function type.
 *
 * @param[in] data Pointer to the received data
 * @param[in] data_size Number of bytes received
 * @param[in,out] context Pointer to a user-specified context object
 */
typedef void (*IntercomRxCallback)(const void* data, size_t data_size, void* context);

/**
 * @brief Set a callback function for received data.
 *
 * @warning The user code MUST copy all of the data provided in the callback.
 *
 * @param[in,out] instance Pointer to the Intercom instance
 * @param[in] channel Channel identifier from the IntercomChannel enumeration
 * @param[in] callback Pointer to the function to be called upon reception of data
 * @param[in,out] context Pointer to a user-specified object (will be passed to the callback)
 */
void intercom_set_rx_callback(
    Intercom* instance,
    IntercomChannel channel,
    IntercomRxCallback callback,
    void* context);

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
 * @brief Transmit data through Intercom.
 *
 * The incoming data will be automatically split into frames and sent asynchronously.
 *
 * @param[in,out] instance Pointer to the Intercom instance
 * @param[in] channel Channel identifier from the IntercomChannel enumeration
 * @param[in] data Pointer to the data to send
 * @param[in] data_size Number of bytes to send
 * @param[in] timeout Time to wait for the transmission to complete, in milliseconds
 * @returns number of bytes that were actually transmitted before timeout
 */
size_t intercom_tx(
    Intercom* instance,
    IntercomChannel channel,
    const void* data,
    size_t data_size,
    uint32_t timeout);

#ifdef __cplusplus
}
#endif
