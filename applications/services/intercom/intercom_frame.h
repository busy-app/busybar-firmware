/**
 * @file intercom_protocol.h
 * @brief Frame definitions and parsing for the Intercom service
 */
#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Total frame size */
#define INTERCOM_FRAME_SIZE         (1024U)
/** Maximum data (payload) size */
#define INTERCOM_FRAME_DATA_SIZE    (INTERCOM_FRAME_SIZE - 5U)
/** Command field size (bits) in status byte */
#define INTERCOM_FRAME_COMMAND_BITS (4U)
/** Channel number field size (bits) in status byte */
#define INTERCOM_FRAME_CHAN_BITS    (4U)

typedef enum {
    IntercomFrameCommandData, /** Data on already opened channel */
    IntercomFrameCommandOpen, /**< Indicating readiness to open channel */
    IntercomFrameCommandMax,
} IntercomFrameCommand;

// Ensure there's enough bits to represent every command and channel
static_assert(IntercomFrameCommandMax <= (1 << INTERCOM_FRAME_COMMAND_BITS));
static_assert(IntercomChannelIdMax <= (1 << INTERCOM_FRAME_CHAN_BITS));
static_assert(INTERCOM_FRAME_COMMAND_BITS + INTERCOM_FRAME_CHAN_BITS == 8);

/**
 * @brief Intercom frame structure.
 *
 * All Intercom frames have a fixed size of 1024 bytes.
 */
typedef struct FURI_PACKED {
    uint8_t status_byte; /**< Command and channel identifier */
    uint16_t data_size; /**< Size of the data (payload) contained in this frame */
    uint8_t data[INTERCOM_FRAME_DATA_SIZE]; /**< Data (payload) to transmit with the frame */
    uint16_t check; /**< 16-bit checksum for transmission error detection */
} IntercomFrame;

static_assert(sizeof(IntercomFrame) == INTERCOM_FRAME_SIZE);

/**
 * @brief Parses the command field out of a frame
 */
static inline IntercomFrameCommand intercom_frame_get_command(const IntercomFrame* frame) {
    uint8_t mask = (1 << INTERCOM_FRAME_COMMAND_BITS) - 1;
    return (frame->status_byte >> INTERCOM_FRAME_CHAN_BITS) & mask;
}

/**
 * @brief Parses the channel field out of a frame
 */
static inline IntercomChannelId intercom_frame_get_channel(const IntercomFrame* frame) {
    uint8_t mask = (1 << INTERCOM_FRAME_CHAN_BITS) - 1;
    return frame->status_byte & mask;
}

/**
 * @brief Makes a status byte
 */
static inline uint8_t
    intercom_frame_make_status(IntercomFrameCommand command, IntercomChannelId channel) {
    uint8_t status = 0;
    status |= channel;
    status |= (command << INTERCOM_FRAME_CHAN_BITS);
    return status;
}

/**
 * @brief Calculate the checksum of a given frame.
 *
 * @param[in] frame Pointer to a frame to calculate the checksum for.
 * @returns 16-bit checksum value
 *
 * @warning The data_size field MUST be between 0 and INTERCOM_FRAME_DATA_SIZE.
 *
 * Source: G.D. Nguyen, "Fast CRCs", IEEE Transactions on Computers, vol. 58, no. 10, pp. 1321-1331, Oct. 2009.
 */
static inline uint16_t intercom_frame_get_checksum(const IntercomFrame* frame) {
    uint16_t cksum = 0;

    const uint8_t* p = (const uint8_t*)frame;
    const size_t p_len = offsetof(IntercomFrame, data) + frame->data_size;

    for(uint32_t i = 0; i < p_len; ++i) {
        const uint16_t tmp = (cksum >> 8) ^ p[i];
        cksum = (tmp << 2) ^ (tmp << 1) ^ (tmp) ^ (cksum << 8);
    }

    return cksum;
}

/**
 * @brief Check if the received frame is valid.
 *
 * @param[in] frame Pointer to a frame to check for validity.
 * @returns true if the frame is valid, false otherwise
 */
static inline bool intercom_frame_is_valid(const IntercomFrame* frame) {
    bool is_valid = false;

    do {
        if(intercom_frame_get_command(frame) >= IntercomFrameCommandMax) break;
        if(intercom_frame_get_channel(frame) >= IntercomChannelIdMax) break;
        if(frame->data_size > INTERCOM_FRAME_DATA_SIZE) break;
        if(intercom_frame_get_checksum(frame) != frame->check) break;

        is_valid = true;
    } while(false);

    return is_valid;
}

#ifdef __cplusplus
}
#endif
