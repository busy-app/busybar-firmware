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
#define INTERCOM_FRAME_SIZE      (1024U)
/** Maximum data (payload) size */
#define INTERCOM_FRAME_DATA_SIZE (INTERCOM_FRAME_SIZE - 5U)

/**
 * @brief Intercom frame structure.
 *
 * All Intercom frames have a fixed size of 1024 bytes.
 */
typedef struct FURI_PACKED {
    uint8_t channel_id; /**< Channel identitier */
    uint16_t data_size; /**< Size of the data (payload) contained in this frame */
    uint8_t data[INTERCOM_FRAME_DATA_SIZE]; /**< Data (payload) to transmit with the frame */
    uint16_t check; /**< 16-bit checksum for transmission error detection */
} IntercomFrame;

static_assert(sizeof(IntercomFrame) == INTERCOM_FRAME_SIZE);

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
        if(frame->channel_id >= IntercomChannelIdMax) break;
        if(frame->data_size > INTERCOM_FRAME_DATA_SIZE) break;
        if(intercom_frame_get_checksum(frame) != frame->check) break;

        is_valid = true;
    } while(false);

    return is_valid;
}

#ifdef __cplusplus
}
#endif
