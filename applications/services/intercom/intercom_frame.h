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

#define INTERCOM_FRAME_SIZE         (1024U)
#define INTERCOM_FRAME_SERVICE_SIZE (sizeof(IntercomFrameHeader) + sizeof(IntercomFrameTrailer))
#define INTERCOM_FRAME_PAYLOAD_SIZE (INTERCOM_FRAME_SIZE - INTERCOM_FRAME_SERVICE_SIZE)
#define INTERCOM_FRAME_DATA_SIZE    (INTERCOM_FRAME_PAYLOAD_SIZE - sizeof(uint16_t))

typedef enum {
    IntercomFrameErrorNone,
    IntercomFrameErrorChecksum,
} IntercomFrameError;

#pragma pack(push, 1)

typedef struct {
    uint8_t id;
    uint8_t error : 4;
    uint8_t port  : 4;
} IntercomFrameHeader;

typedef struct {
    uint16_t check;
} IntercomFrameTrailer;

typedef struct {
    uint16_t size;
    uint8_t data[INTERCOM_FRAME_DATA_SIZE];
} IntercomFramePayload;

typedef struct {
    IntercomFrameHeader header;
    IntercomFramePayload payload;
    IntercomFrameTrailer trailer;
} IntercomFrame;

#pragma pack(pop)

static_assert(sizeof(IntercomFrame) == INTERCOM_FRAME_SIZE);

/*
 * G.D. Nguyen, "Fast CRCs", IEEE Transactions on Computers, vol. 58, no. 10, pp. 1321-1331, Oct. 2009.
 */
static inline uint16_t intercom_frame_calculate_checksum(const IntercomFrame* frame) {
    uint16_t cksum = 0;

    const uint8_t* data = (const uint8_t*)frame;
    const size_t data_size = sizeof(IntercomFrameHeader) + sizeof(uint16_t) + frame->payload.size;

    for(uint32_t i = 0; i < data_size; ++i) {
        const uint16_t tmp = (cksum >> 8) ^ data[i];
        cksum = (tmp << 2) ^ (tmp << 1) ^ (tmp) ^ (cksum << 8);
    }

    return cksum;
}

static inline bool intercom_frame_is_valid(const IntercomFrame* frame) {
    bool is_valid = false;

    do {
        if(frame->payload.size > INTERCOM_FRAME_PAYLOAD_SIZE) break;
        if(intercom_frame_calculate_checksum(frame) != frame->trailer.check) break;

        is_valid = true;
    } while(false);

    return is_valid;
}

#ifdef __cplusplus
}
#endif
