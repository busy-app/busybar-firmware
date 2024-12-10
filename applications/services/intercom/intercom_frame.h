/**
 * @file intercom_protocol.h
 * @brief Frame definitions and parsing for the Intercom service
 */
#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INTERCOM_FRAME_SIZE      (1024U)
#define INTERCOM_FRAME_DATA_SIZE (INTERCOM_FRAME_SIZE - 5U)

#pragma pack(push, 1)

typedef struct {
    uint8_t channel;
    uint16_t data_size;
    uint8_t data[INTERCOM_FRAME_DATA_SIZE];
    uint16_t check;
} IntercomFrame;

#pragma pack(pop)

static_assert(sizeof(IntercomFrame) == INTERCOM_FRAME_SIZE);

/*
 * G.D. Nguyen, "Fast CRCs", IEEE Transactions on Computers, vol. 58, no. 10, pp. 1321-1331, Oct. 2009.
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

static inline bool intercom_frame_is_valid(const IntercomFrame* frame) {
    bool is_valid = false;

    do {
        if(frame->data_size > INTERCOM_FRAME_DATA_SIZE) break;
        if(intercom_frame_get_checksum(frame) != frame->check) break;

        is_valid = true;
    } while(false);

    return is_valid;
}

#ifdef __cplusplus
}
#endif
