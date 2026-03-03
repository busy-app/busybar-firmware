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

/**
 * @brief Intercom FURI record identifier.
 */
#define RECORD_INTERCOM "intercom"

/**
 * @brief Opaque Intercom type declaration.
 */
typedef struct Intercom Intercom;

typedef enum {
    IntercomStatusOk,
    IntercomStatusUnknown,
    IntercomStatusErrorSync,
    IntercomStatusErrorFraming,
    IntercomStatusErrorTimeout,
} IntercomStatus;

/**
 * @brief Enumeration of available channel identifiers.
 */
typedef enum {
    IntercomChannelIdInput, /**< Input handling */
    IntercomChannelIdWifiControl, /**< Wireless network control handling */
    IntercomChannelIdWifiData, /**< Wireless network data handling */
    IntercomChannelIdStatusLights, /**< Status lights handling */
    IntercomChannelIdCli, /**< Command line interface handling */
    IntercomChannelIdBle, /**< BLE handling */
    IntercomChannelIdCryptoBackup, /**< Crypto backup handling */
    IntercomChannelIdTlsCrypto, /**< TLS Crypto handling */
    IntercomChannelIdMatter, /**< Matter smart home protocol */
    IntercomChannelIdSlInfo, /**< Wireless co-processor info channel */
    /* Add more channels here as needed */
    IntercomChannelIdDebug, /**< Testing, debugging, etc */
    IntercomChannelIdMeta, /**< Special channel for internal Intercom use. Do not use. */
    IntercomChannelIdMax, /**< Special value for internal use */
} IntercomChannelId;

FuriState* intercom_get_state(const Intercom* instance);

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
