#include "fkermit.h"

#include <furi.h>

#define TAG "Kermit"

#define KERMIT_PACKET_MAX_LENGTH     94
#define KERMIT_PACKET_EXT_MAX_LENGTH 9024

#define KERMIT_PACKET_MARK         (0x01)
#define KERMIT_PACKET_END          ('\r')
#define KERMIT_EXT_PACKET_SIZE_MOD (95)

#define KERMIT_CONTROL_CHAR ('#')

typedef struct {
    uint8_t* data;
    int32_t sz;
} kermit_packet_t;

typedef enum {
    KERMIT_PACKET_STATE_ERROR,
    KERMIT_PACKET_STATE_WAIT_MARK,
    KERMIT_PACKET_STATE_WAIT_LEN,
    KERMIT_PACKET_STATE_WAIT_CONTENTS,
    KERMIT_PACKET_STATE_WAIT_CHECKSUM,
    KERMIT_PACKET_STATE_WAIT_END,
} kermit_packet_state_t;

typedef struct {
    kermit_packet_state_t state;
    uint8_t seq;
    uint8_t len;
    uint16_t checksum;
    kermit_packet_t* packet;
} kermit_rx_t;

typedef enum {
    KERMIT_FILE_TRANSFER_STATE_IDLE,
    KERMIT_FILE_TRANSFER_STATE_SYNC_PARAMS,
    KERMIT_FILE_TRANSFER_STATE_SEND_FILE_NAME,
    KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA,
    KERMIT_FILE_TRANSFER_STATE_DONE,
} kermit_file_transfer_state_t;

struct kermit_t {
    uint8_t seq_counter;
    uint32_t max_packet_length, max_ext_packet_length;
    const kermit_io_t* io;
    void* io_context;
    kermit_rx_t rx; // Packet reassembly state
    kermit_file_transfer_state_t file_transfer_state;
};

typedef enum {
    KERMIT_PACKET_TYPE_INIT = 'S',
    KERMIT_PACKET_TYPE_DATA = 'D',
    KERMIT_PACKET_TYPE_EOF = 'Z',
    KERMIT_PACKET_TYPE_ACK = 'Y',
    KERMIT_PACKET_TYPE_NAK = 'N',
    KERMIT_PACKET_TYPE_BREAK = 'B',
    KERMIT_PACKET_TYPE_FILE = 'F',
} kermit_packet_type_t;

//////////////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t mark;
    uint8_t length;
    uint8_t seq;
    uint8_t type;
} FURI_PACKED kermit_packet_header_t;

_Static_assert(sizeof(kermit_packet_header_t) == 4, "Invalid size of kermit_packet_header_t");

typedef struct {
    uint8_t mark;
    uint8_t blank;
    uint8_t seq;
    uint8_t type;
    uint8_t length1;
    uint8_t length2;
    uint8_t header_checksum;
} FURI_PACKED kermit_packet_ext_header_t;

_Static_assert(
    sizeof(kermit_packet_ext_header_t) == 7,
    "Invalid size of kermit_packet_ext_header_t");

typedef struct {
    uint8_t checksum;
    uint8_t end;
} FURI_PACKED kermit_packet_footer_t;

_Static_assert(sizeof(kermit_packet_footer_t) == 2, "Invalid size of kermit_packet_footer_t");

//////////////////////////////////////////////////////////////////////////

static kermit_packet_t* kermit_packet_alloc(size_t sz) {
    kermit_packet_t* packet = malloc(sizeof(kermit_packet_t));
    packet->data = malloc(sz);
    packet->sz = sz;
    return packet;
}

inline void kermit_packet_free(kermit_packet_t* packet) {
    if(!packet) {
        return;
    }
    free(packet->data);
    free(packet);
}

//////////////////////////////////////////////////////////////////////////

inline uint8_t kermit_tochar(uint8_t value) {
    return value + 32;
}

inline uint8_t kermit_fromchar(uint8_t value) {
    return value - 32;
}

inline uint8_t kermit_ctl(uint8_t value) {
    return value ^ 64;
}

inline uint8_t kermit_checksum(const uint8_t* data, size_t length) {
    uint32_t sum = 0;
    for(size_t i = 0; i < length; i++) {
        sum += data[i];
    }

    return kermit_tochar((sum + ((sum & 0xC0) >> 6)) & 0x3F);
}

//////////////////////////////////////////////////////////////////////////

kermit_packet_t* kermit_create_packet(
    kermit_t* kermit,
    const kermit_packet_type_t packet_type,
    const uint8_t* data,
    size_t length) {
    furi_check(data != NULL);
    furi_check(length <= kermit->max_packet_length);

    kermit_packet_t* packet = kermit_packet_alloc(
        sizeof(kermit_packet_header_t) + length + sizeof(kermit_packet_footer_t));

    kermit_packet_header_t header = {
        .mark = KERMIT_PACKET_MARK,
        .length = kermit_tochar(length),
        .seq = kermit_tochar(0),
        .type = packet_type,
    };

    memcpy(packet->data, &header, sizeof(header));
    memcpy(packet->data + sizeof(header), data, length);

    kermit_packet_footer_t footer = {
        .checksum = kermit_checksum(packet->data + 1, packet->sz - 2), // Skip the marks
        .end = KERMIT_PACKET_END,
    };

    memcpy(packet->data + sizeof(header) + length, &footer, sizeof(footer));

    return packet;
}

kermit_packet_t* kermit_create_ext_packet(
    kermit_t* kermit,
    const kermit_packet_type_t packet_type,
    const uint8_t* data,
    size_t length) {
    furi_check(data != NULL);
    furi_check(length <= kermit->max_ext_packet_length);

    kermit_packet_t* packet = kermit_packet_alloc(
        sizeof(kermit_packet_ext_header_t) + length + sizeof(kermit_packet_footer_t));

    kermit_packet_ext_header_t header = {
        .mark = KERMIT_PACKET_MARK,
        .blank = 0,
        .seq = kermit_tochar(0),
        .type = packet_type,
        .length1 = kermit_tochar(length / KERMIT_EXT_PACKET_SIZE_MOD),
        .length2 = kermit_tochar(length % KERMIT_EXT_PACKET_SIZE_MOD),
        .header_checksum = 0,
    };
    header.header_checksum = kermit_checksum((uint8_t*)(&header) + 1, sizeof(header) - 2);

    memcpy(packet->data, &header, sizeof(header));
    memcpy(packet->data + sizeof(header), data, length);

    kermit_packet_footer_t footer = {
        .checksum = kermit_checksum(packet->data + 1, packet->sz - 2), // Skip the marks
        .end = KERMIT_PACKET_END,
    };

    memcpy(packet->data + sizeof(header) + length, &footer, sizeof(footer));

    return packet;
}

static void kermit_reset_state(kermit_t* kermit) {
    kermit->seq_counter = 0;

    kermit->max_packet_length = KERMIT_PACKET_MAX_LENGTH;
    kermit->max_ext_packet_length = KERMIT_PACKET_EXT_MAX_LENGTH;

    memset(&kermit->rx, 0, sizeof(kermit_rx_t));
    kermit->rx.state = KERMIT_PACKET_STATE_WAIT_MARK;

    kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_IDLE;
}

kermit_t* kermit_alloc(void* context, const kermit_io_t* io) {
    furi_check(io != NULL);

    kermit_t* kermit = malloc(sizeof(kermit_t));

    kermit->io = io;
    kermit->io_context = context;

    kermit_reset_state(kermit);

    return kermit;
}

void kermit_free(kermit_t* kermit) {
    free(kermit);
}

static bool kermit_process_packet(kermit_t* kermit);

typedef struct {
    uint8_t maxl;
    uint8_t timo;
    uint8_t npad;
    uint8_t padc;
    uint8_t eol;
    uint8_t qctl;
    uint8_t ebq;
    uint8_t bct;
    uint8_t rpt;
    uint8_t capas;
    uint8_t wslots;
    uint8_t maxlx1, maxlx2;
} FURI_PACKED kermit_init_packet_t;

static bool kermit_feed_byte(kermit_t* kermit, uint8_t c) {
    furi_check(kermit != NULL);

    kermit_rx_t* rx = &kermit->rx;

    FURI_LOG_I(TAG, "Received: %c, state: %d", c, rx->state);

    bool result = true;
    switch(rx->state) {
    case KERMIT_PACKET_STATE_WAIT_MARK:
        if(c == KERMIT_PACKET_MARK) {
            rx->state = KERMIT_PACKET_STATE_WAIT_LEN;
        }
        break;

    case KERMIT_PACKET_STATE_WAIT_LEN:
        rx->len = kermit_fromchar(c);
        if(rx->len > kermit->max_packet_length) {
            FURI_LOG_E(TAG, "Invalid packet length");
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        if(rx->packet) {
            FURI_LOG_E(TAG, "Packet already allocated");
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }

        rx->state = KERMIT_PACKET_STATE_WAIT_CONTENTS;
        rx->packet = kermit_packet_alloc(rx->len + 4);
        break;

    case KERMIT_PACKET_STATE_WAIT_CONTENTS:
        rx->len--;
        rx->packet->data[rx->packet->sz - rx->len - 1] = c;
        if(c == KERMIT_PACKET_END) {
            furi_check(rx->len == 0);
            rx->state = KERMIT_PACKET_STATE_WAIT_END;
        } else if(rx->len == 0) {
            FURI_LOG_E(TAG, "Invalid packet length");
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        break;

    case KERMIT_PACKET_STATE_WAIT_CHECKSUM:
        rx->checksum = kermit_fromchar(c);
        UNUSED(rx->checksum); // FIXME: validate
        rx->state = KERMIT_PACKET_STATE_WAIT_END;
        break;

    case KERMIT_PACKET_STATE_WAIT_END:
        if(c != KERMIT_PACKET_END) {
            FURI_LOG_E(TAG, "Invalid end of packet");
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        rx->state = KERMIT_PACKET_STATE_WAIT_MARK;

        kermit_process_packet(kermit);

        kermit_packet_free(rx->packet);
        rx->packet = NULL;

        break;

    case KERMIT_PACKET_STATE_ERROR:
        FURI_LOG_E(TAG, "Error state");
        result = false;
        break;

    default:
        FURI_LOG_E(TAG, "Invalid state: %d", rx->state);
        result = false;
        break;
    }

    return result;
}

int32_t kermit_feed_serial_data(kermit_t* kermit, const uint8_t* data, size_t length) {
    furi_check(kermit != NULL);
    furi_check(data != NULL);

    while(length > 0) {
        uint8_t c = *data;
        data++;
        length--;

        if(!kermit_feed_byte(kermit, c)) {
            return -1;
        }
    }

    return 0;
}

bool kermit_run(kermit_t* kermit) {
    furi_check(kermit != NULL);
    furi_check(kermit->file_transfer_state == KERMIT_FILE_TRANSFER_STATE_IDLE);

    // Start kermit session

    bool result = false;
    kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_SYNC_PARAMS;

    kermit_init_packet_t init_packet_data = {
        .maxl = kermit_tochar(kermit->max_packet_length),
        .timo = kermit_tochar(10), // FIXME
        .npad = kermit_tochar(0),
        .padc = kermit_ctl(0),
        .eol = kermit_ctl(KERMIT_PACKET_END),
        .qctl = (KERMIT_CONTROL_CHAR), // ??
        .ebq = 'N',
        .bct = '1',
        .rpt = ' ',
        .capas = kermit_tochar(0x02),
        .wslots = kermit_tochar(0),
        .maxlx1 = kermit_tochar(kermit->max_ext_packet_length / KERMIT_EXT_PACKET_SIZE_MOD),
        .maxlx2 = kermit_tochar(kermit->max_ext_packet_length % KERMIT_EXT_PACKET_SIZE_MOD),
    };

    kermit_packet_t* init_packet = kermit_create_packet(
        kermit, KERMIT_PACKET_TYPE_INIT, (uint8_t*)&init_packet_data, sizeof(init_packet_data));

    do {
        if(kermit->io->comms_send(kermit->io_context, init_packet->data, init_packet->sz) !=
           init_packet->sz) {
            break;
        }
        result = true;
    } while(false);

    kermit_packet_free(init_packet);

    // Further exchange is done in kermit_feed_serial_data
    return result;
}

static kermit_packet_t* kermit_encode_data_packet(kermit_t* kermit) {
    furi_check(kermit->file_transfer_state == KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA);
    furi_check(kermit->max_ext_packet_length >= 2);

    uint8_t* tmp_buffer = malloc(kermit->max_ext_packet_length);
    uint8_t* buffer_ptr = tmp_buffer;
    do {
        uint8_t file_char;
        if(kermit->io->src_file_read(kermit->io_context, &file_char, 1) != 1) {
            break; // eof or error
        }

        if(((file_char & 0x7F) < 0x20) || (file_char == 0x7F)) {
            *buffer_ptr = KERMIT_CONTROL_CHAR;
            buffer_ptr++;
            *buffer_ptr = kermit_ctl(file_char);
        } else if(file_char == KERMIT_CONTROL_CHAR) {
            *buffer_ptr = KERMIT_CONTROL_CHAR;
            buffer_ptr++;
            *buffer_ptr = KERMIT_CONTROL_CHAR;
        } else {
            *buffer_ptr = file_char;
        }
        buffer_ptr++;
        // leave 1 byte in case last char will be escaped
        // so we don't need to rewind the file pointer if it doesn't fit
    } while((uint32_t)(buffer_ptr - tmp_buffer) < (kermit->max_ext_packet_length + 1));

    size_t length = buffer_ptr - tmp_buffer;
    kermit_packet_t* packet =
        kermit_create_ext_packet(kermit, KERMIT_PACKET_TYPE_DATA, tmp_buffer, length);
    return packet;
}

static kermit_packet_t* kermit_encode_file_header_packet(kermit_t* kermit) {
    const char filename[] = "FIRMWA.RPS";

    kermit_packet_t* packet = kermit_create_packet(
        kermit, KERMIT_PACKET_TYPE_FILE, (uint8_t*)filename, sizeof(filename));
    return packet;
}

static bool kermit_process_packet(kermit_t* kermit) {
    furi_check(kermit->rx.packet != NULL);

    FURI_LOG_I(TAG, "Received packet: %s", kermit->rx.packet->data);

    // kermit_packet_header_t* header = (kermit_packet_header_t*)packet->data;
    // furi_check(header->seq == kermit_tochar(seq));
    kermit_packet_header_t* header = (kermit_packet_header_t*)kermit->rx.packet->data;
    furi_check(header->mark == KERMIT_PACKET_MARK);

    kermit_packet_t* response_packet = NULL;

    switch(header->type) {
    case KERMIT_PACKET_TYPE_ACK:
        if(kermit->file_transfer_state == KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA) {
            response_packet = kermit_encode_data_packet(kermit);
        } else if(kermit->file_transfer_state == KERMIT_FILE_TRANSFER_STATE_SYNC_PARAMS) {
            kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA;
            // TODO: Parse params from ACK packet
            response_packet = kermit_encode_file_header_packet(kermit);
        }
        break;

    case KERMIT_PACKET_TYPE_NAK:
        FURI_LOG_E(TAG, "NAK received");
        return false;

    default:
        FURI_LOG_E(TAG, "Invalid packet type: %c", header->type);
        break;
    }

    if(response_packet) {
        kermit->io->comms_send(kermit->io_context, response_packet->data, response_packet->sz);
        kermit_packet_free(response_packet);
    }

    kermit_packet_free(kermit->rx.packet);
    kermit->rx.packet = NULL;

    return false;
}
