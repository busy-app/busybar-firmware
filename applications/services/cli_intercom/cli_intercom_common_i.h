/**
 * @file cli_intercom_common_i.h
 * CliIntercom data serialization format
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ===================
// Protocol definition
// ===================

typedef enum {
    // Status messages (length == 0, no payload):
    CliIntercomMessageTypeSpawn,
    CliIntercomMessageTypeDisconnect,

    // Payload messages (length >= 0, has payload):
    CliIntercomMessageTypeData,

    // Don't use:
    CliIntercomMessageTypeMAX,
} CliIntercomMessageType;

#define CliIntercomMessageTypeStatusMAX CliIntercomMessageTypeData

#define CLI_INTERCOM_MESSAGE_TYPE_BITS   (2)
#define CLI_INTERCOM_PAYLOAD_LENGTH_BITS (6)

// =====================================
// Serialization/deserialization helpers
// =====================================

#define CLI_INTERCOM_MAX_PAYLOAD_LEN ((size_t)((1 << CLI_INTERCOM_PAYLOAD_LENGTH_BITS) - 1))

static_assert(CLI_INTERCOM_MESSAGE_TYPE_BITS + CLI_INTERCOM_PAYLOAD_LENGTH_BITS == 8);
static_assert(CliIntercomMessageTypeMAX <= (1 << CLI_INTERCOM_MESSAGE_TYPE_BITS));

static inline uint8_t
    cli_intercom_construct_status(CliIntercomMessageType msg_type, uint8_t length) {
    furi_assert(msg_type < CliIntercomMessageTypeMAX);
    furi_assert(length <= CLI_INTERCOM_MAX_PAYLOAD_LEN);
#ifdef BSB_MCU_SI917
    furi_assert(msg_type != CliIntercomMessageTypeSpawn); // can't spawn a shell on u5
#endif
    return (msg_type << CLI_INTERCOM_PAYLOAD_LENGTH_BITS) | length;
}

static inline void
    cli_intercom_parse_status(uint8_t status, CliIntercomMessageType* msg_type, uint8_t* length) {
    *msg_type = status >> CLI_INTERCOM_PAYLOAD_LENGTH_BITS;
    *length = status & CLI_INTERCOM_MAX_PAYLOAD_LEN;
    if(*msg_type < CliIntercomMessageTypeStatusMAX) furi_assert(!*length);

#ifdef BSB_MCU_SI917
    furi_assert(*msg_type != CliIntercomMessageTypeSpawn); // can't spawn a shell on u5
#endif
}

#ifdef __cplusplus
}
#endif
