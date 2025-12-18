#include "fkermit.h"

#include <furi.h>

#define TAG "Kermit"

// Controls debug output and features
#ifndef KERMIT_DEBUG
#define KERMIT_DEBUG 0
#endif

#if KERMIT_DEBUG
#define KERMIT_LOG(...) FURI_LOG_D(TAG, __VA_ARGS__)
#else
#define KERMIT_LOG(...)
#endif

// Kermit protocol constants
#define KERMIT_PACKET_MAX_LENGTH     94
#define KERMIT_PACKET_EXT_MAX_LENGTH 9024

#define KERMIT_PACKET_MARK (0x01)
#define KERMIT_PACKET_END  ('\r')

// Kermit transfer configuration
#define KERMIT_NPAD         (0)
#define KERMIT_PADC         (0)
#define KERMIT_CONTROL_CHAR ('#')
#define KERMIT_EBQ_MODE     ('N')
#define KERMIT_BCT_MODE     ('1') // 1 byte 6-bit checksum
#define KERMIT_RPT          (' ')
#define KERMIT_CAPAS_MASK   (0x02)
#define KERMIT_WINDOW_SZ    (0)

#define KERMIT_EXT_PACKET_SIZE_MOD (95)
#define KERMIT_SEQ_MODULO          (64)

#define KERMIT_DEFAULT_FILE_NAME "FIRMWA.RPS"

// Kermit packet container
typedef struct {
    uint8_t* data;
    int32_t sz;
} kermit_packet_t;

typedef enum {
    KERMIT_PACKET_TYPE_INIT = 'S',
    KERMIT_PACKET_TYPE_DATA = 'D',
    KERMIT_PACKET_TYPE_EOF = 'Z',
    KERMIT_PACKET_TYPE_ACK = 'Y',
    KERMIT_PACKET_TYPE_NAK = 'N',
    KERMIT_PACKET_TYPE_BREAK = 'B',
    KERMIT_PACKET_TYPE_FILE = 'F',
} kermit_packet_type_t;

typedef enum {
    KERMIT_PACKET_STATE_ERROR,
    KERMIT_PACKET_STATE_WAIT_MARK,
    KERMIT_PACKET_STATE_WAIT_LEN,
    KERMIT_PACKET_STATE_WAIT_SEQ,
    KERMIT_PACKET_STATE_WAIT_TYPE,
    KERMIT_PACKET_STATE_WAIT_CONTENTS,
    KERMIT_PACKET_STATE_WAIT_CHECKSUM,
    KERMIT_PACKET_STATE_WAIT_END,
} kermit_packet_state_t;

// Packet reassembly state
typedef struct {
    kermit_packet_t* contents;
    kermit_packet_type_t type;
    kermit_packet_state_t state;
    uint8_t seq;
    uint8_t len;
    uint16_t checksum;
} kermit_rx_t;

typedef enum {
    KERMIT_FILE_TRANSFER_STATE_IDLE,
    KERMIT_FILE_TRANSFER_STATE_SYNC_PARAMS,
    KERMIT_FILE_TRANSFER_STATE_SEND_FILE_NAME,
    KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA,
    KERMIT_FILE_TRANSFER_STATE_SEND_FILE_EOF,
    KERMIT_FILE_TRANSFER_STATE_SEND_BREAK,
    KERMIT_FILE_TRANSFER_STATE_DONE,
} kermit_file_transfer_state_t;

struct kermit_t {
    uint32_t max_packet_length, max_ext_packet_length;
    uint8_t seq_counter;
    const kermit_io_t* io;
    void* io_context;
    kermit_rx_t rx;
    kermit_file_transfer_state_t file_transfer_state;
};

// Kermit packet header and footer structures

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

_Static_assert(sizeof(kermit_init_packet_t) == 13, "Invalid size of kermit_init_packet_t");

// Kermit packet allocation and deallocation

static kermit_packet_t* kermit_packet_alloc(size_t sz) {
    kermit_packet_t* packet = malloc(sizeof(kermit_packet_t));
    packet->data = sz ? malloc(sz) : NULL;
    packet->sz = sz;
    return packet;
}

void kermit_packet_free(kermit_packet_t* packet) {
    if(!packet) {
        return;
    }
    free(packet->data);
    free(packet);
}

// Kermit protocol conversion functions

FURI_ALWAYS_INLINE uint8_t kermit_tochar(uint8_t value) {
#if KERMIT_DEBUG
    furi_check(value <= 94);
#endif
    return value + 32;
}

FURI_ALWAYS_INLINE uint8_t kermit_fromchar(uint8_t value) {
#if KERMIT_DEBUG
    furi_check(value >= 32);
#endif
    return value - 32;
}

FURI_ALWAYS_INLINE uint8_t kermit_ctl(uint8_t value) {
    return value ^ 64;
}

uint8_t kermit_checksum(const uint8_t* data, size_t length) {
    uint32_t sum = 0;
    for(size_t i = 0; i < length; i++) {
        sum += data[i];
    }

    return kermit_tochar((sum + ((sum & 0xC0) >> 6)) & 0x3F);
}

static FURI_ALWAYS_INLINE uint8_t kermit_next_seq(uint8_t seq) {
    return (seq + 1) % KERMIT_SEQ_MODULO;
}

// Kermit packet creation functions

kermit_packet_t* kermit_create_packet(
    kermit_t* kermit,
    const kermit_packet_type_t packet_type,
    const uint8_t* data,
    size_t length) {
    furi_check((length == 0) || (length && (data != NULL)));
    furi_check(length <= kermit->max_packet_length);

    kermit_packet_t* packet = kermit_packet_alloc(
        sizeof(kermit_packet_header_t) + length + sizeof(kermit_packet_footer_t));

    kermit_packet_header_t header = {
        .mark = KERMIT_PACKET_MARK,
        .length = kermit_tochar(length + 3), // seq + type + check
        .seq = kermit_tochar(kermit->seq_counter),
        .type = packet_type,
    };
    memcpy(packet->data, &header, sizeof(header));
    memcpy(packet->data + sizeof(header), data, length);

    kermit_packet_footer_t footer = {
        // Skip the marks
        .checksum = kermit_checksum(packet->data + 1, packet->sz - 2),
        .end = KERMIT_PACKET_END,
    };
    memcpy(packet->data + sizeof(header) + length, &footer, sizeof(footer));

    kermit->seq_counter = kermit_next_seq(kermit->seq_counter);
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
        .blank = kermit_tochar(0),
        .seq = kermit_tochar(kermit->seq_counter),
        .type = packet_type,
        .length1 = kermit_tochar((length + 1) / KERMIT_EXT_PACKET_SIZE_MOD),
        .length2 = kermit_tochar((length + 1) % KERMIT_EXT_PACKET_SIZE_MOD),
        .header_checksum = 0,
    };
    header.header_checksum = kermit_checksum((uint8_t*)(&header) + 1, sizeof(header) - 2);
    memcpy(packet->data, &header, sizeof(header));
    memcpy(packet->data + sizeof(header), data, length);

    kermit_packet_footer_t footer = {
        // Skip the marks
        .checksum = kermit_checksum(packet->data + 1, packet->sz - 2),
        .end = KERMIT_PACKET_END,
    };
    memcpy(packet->data + sizeof(header) + length, &footer, sizeof(footer));

    kermit->seq_counter = kermit_next_seq(kermit->seq_counter);
    return packet;
}

void kermit_reset_state(kermit_t* kermit) {
    kermit->seq_counter = 0;

    kermit->max_packet_length = KERMIT_PACKET_MAX_LENGTH;
    kermit->max_ext_packet_length = KERMIT_PACKET_EXT_MAX_LENGTH;

    if(kermit->rx.contents) {
        kermit_packet_free(kermit->rx.contents);
    }

    memset(&kermit->rx, 0, sizeof(kermit_rx_t));
    kermit->rx.state = KERMIT_PACKET_STATE_WAIT_MARK;

    kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_IDLE;
}

kermit_t* kermit_alloc(const kermit_io_t* io, void* context) {
    furi_check(io != NULL);

    kermit_t* kermit = malloc(sizeof(kermit_t));
    kermit->io = io;
    kermit->io_context = context;

    kermit->rx.contents = NULL;
    kermit_reset_state(kermit);

    return kermit;
}

void kermit_free(kermit_t* kermit) {
    kermit_reset_state(kermit);
    free(kermit);
}

bool kermit_is_active(kermit_t* kermit) {
    return kermit->file_transfer_state != KERMIT_FILE_TRANSFER_STATE_DONE;
}

static bool kermit_process_packet(kermit_t* kermit);

static bool kermit_feed_byte(kermit_t* kermit, uint8_t c) {
    furi_check(kermit != NULL);

    kermit_rx_t* rx = &kermit->rx;

    KERMIT_LOG("Received: %02x, state: %d", c, rx->state);

    bool result = true;
    switch(rx->state) {
    case KERMIT_PACKET_STATE_WAIT_MARK:
        if(c == KERMIT_PACKET_MARK) {
            rx->state = KERMIT_PACKET_STATE_WAIT_LEN;
        } else {
            KERMIT_LOG("Garbage data: %02x", c);
        }
        break;

    case KERMIT_PACKET_STATE_WAIT_LEN:
        rx->len = kermit_fromchar(c);
        if(rx->len > kermit->max_packet_length) {
            FURI_LOG_E(TAG, "Invalid packet length: %u > %lu", rx->len, kermit->max_packet_length);
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        if(rx->contents) {
            FURI_LOG_E(TAG, "Packet already allocated but not processed");
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        if(rx->len < 3) {
            FURI_LOG_E(TAG, "Invalid packet length: %u < 3", rx->len);
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        // seq + type + check go to separate fields in reassembly
        rx->len -= 3;
        rx->state = KERMIT_PACKET_STATE_WAIT_SEQ;
        rx->contents = kermit_packet_alloc(rx->len);
        break;

    case KERMIT_PACKET_STATE_WAIT_SEQ:
        rx->seq = kermit_fromchar(c);
        KERMIT_LOG("Packet seq: %d", rx->seq);
        rx->state = KERMIT_PACKET_STATE_WAIT_TYPE;
        break;

    case KERMIT_PACKET_STATE_WAIT_TYPE:
        rx->type = c;
        KERMIT_LOG("Packet type: %c", c);
        rx->state = rx->len ? KERMIT_PACKET_STATE_WAIT_CONTENTS :
                              KERMIT_PACKET_STATE_WAIT_CHECKSUM;
        break;

    case KERMIT_PACKET_STATE_WAIT_CONTENTS:
        KERMIT_LOG("Packet sz: %ld, rem rxlen: %d", rx->contents->sz, rx->len);
        furi_check(rx->len >= 0);
        furi_check(rx->contents->sz - rx->len < rx->contents->sz);

        if(c == KERMIT_PACKET_END) {
            KERMIT_LOG("End of packet in contents");
            rx->state = KERMIT_PACKET_STATE_ERROR;
        }

        rx->contents->data[rx->contents->sz - rx->len] = c;
        rx->len--;
        if(rx->len == 0) {
            KERMIT_LOG("Received all expected data");
            rx->state = KERMIT_PACKET_STATE_WAIT_CHECKSUM;
        }
        break;

    case KERMIT_PACKET_STATE_WAIT_CHECKSUM:
        rx->checksum = kermit_fromchar(c);
        // todo: validate?
        // challenging, because we do not store header fields right next to the data
        UNUSED(rx->checksum);
        rx->state = KERMIT_PACKET_STATE_WAIT_END;
        break;

    case KERMIT_PACKET_STATE_WAIT_END:
        if(c != KERMIT_PACKET_END) {
            FURI_LOG_E(TAG, "Invalid end of packet");
            rx->state = KERMIT_PACKET_STATE_ERROR;
            return false;
        }
        result = kermit_process_packet(kermit);

        kermit_packet_free(rx->contents);
        rx->contents = NULL;

        rx->state = KERMIT_PACKET_STATE_WAIT_MARK;
        break;

    case KERMIT_PACKET_STATE_ERROR:
        FURI_LOG_E(TAG, "Error state");
        // fallthrough
    default:
        FURI_LOG_E(TAG, "Invalid state: %d", rx->state);
        result = false;
        break;
    }

    return result;
}

bool kermit_feed_serial_data(kermit_t* kermit, const uint8_t* data, size_t length) {
    furi_check(kermit != NULL);
    furi_check(data != NULL);
    furi_check(length > 0);

    while(length--) {
        if(!kermit_feed_byte(kermit, *data++)) {
            return false;
        }
    }
    return true;
}

static bool kermit_tx_and_release_packet(kermit_t* kermit, kermit_packet_t* packet) {
    furi_check(packet != NULL);

    bool success =
        (kermit->io->comms_send(kermit->io_context, packet->data, packet->sz) == packet->sz);

    kermit_packet_free(packet);
    return success;
}

bool kermit_start(kermit_t* kermit, const uint8_t timeout_seconds) {
    furi_check(kermit != NULL);
    furi_check(kermit->file_transfer_state == KERMIT_FILE_TRANSFER_STATE_IDLE);

    // Start kermit session

    kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_SYNC_PARAMS;

    kermit_init_packet_t init_packet_data = {
        .maxl = kermit_tochar(kermit->max_packet_length),
        .timo = kermit_tochar(timeout_seconds),
        .npad = kermit_tochar(KERMIT_NPAD),
        .padc = kermit_ctl(KERMIT_PADC),
        .eol = kermit_tochar(KERMIT_PACKET_END), // should be `kermit_ctl` to spec
        .qctl = KERMIT_CONTROL_CHAR,
        .ebq = KERMIT_EBQ_MODE,
        .bct = KERMIT_BCT_MODE,
        .rpt = KERMIT_RPT,
        .capas = kermit_tochar(KERMIT_CAPAS_MASK),
        .wslots = kermit_tochar(KERMIT_WINDOW_SZ),
        .maxlx1 = kermit_tochar(kermit->max_ext_packet_length / KERMIT_EXT_PACKET_SIZE_MOD),
        .maxlx2 = kermit_tochar(kermit->max_ext_packet_length % KERMIT_EXT_PACKET_SIZE_MOD),
    };

    kermit_packet_t* init_packet = kermit_create_packet(
        kermit, KERMIT_PACKET_TYPE_INIT, (uint8_t*)&init_packet_data, sizeof(init_packet_data));

    return kermit_tx_and_release_packet(kermit, init_packet);
}

static bool kermit_parse_session_params(kermit_t* kermit) {
    furi_check(kermit->rx.contents != NULL);
    furi_check(kermit->rx.type == KERMIT_PACKET_TYPE_ACK);

    if(kermit->rx.contents->sz != sizeof(kermit_init_packet_t)) {
        FURI_LOG_E(TAG, "Invalid ACK packet size: %ld", kermit->rx.contents->sz);
        return false;
    }

    kermit_init_packet_t* init_packet = (kermit_init_packet_t*)kermit->rx.contents->data;

    KERMIT_LOG("maxl: %d", kermit_fromchar(init_packet->maxl));
    KERMIT_LOG("timo: %d", kermit_fromchar(init_packet->timo));
    KERMIT_LOG("npad: %d", kermit_fromchar(init_packet->npad));
    KERMIT_LOG("padc: %d", kermit_ctl(init_packet->padc));
    KERMIT_LOG("eol: %d", kermit_fromchar(init_packet->eol));
    KERMIT_LOG("qctl: %d", init_packet->qctl);
    KERMIT_LOG("ebq: %d", init_packet->ebq);
    KERMIT_LOG("bct: %d", init_packet->bct);
    KERMIT_LOG("rpt: %d", init_packet->rpt);
    KERMIT_LOG("capas: %d", kermit_fromchar(init_packet->capas));
    KERMIT_LOG("wslots: %d", kermit_fromchar(init_packet->wslots));
    KERMIT_LOG("maxlx1: %d", kermit_fromchar(init_packet->maxlx1));
    KERMIT_LOG("maxlx2: %d", kermit_fromchar(init_packet->maxlx2));

    kermit->max_packet_length = kermit_fromchar(init_packet->maxl);
    kermit->max_ext_packet_length =
        kermit_fromchar(init_packet->maxlx1) * KERMIT_EXT_PACKET_SIZE_MOD +
        kermit_fromchar(init_packet->maxlx2);
    FURI_LOG_I(
        TAG,
        "Established max len: %ld, max ext len: %ld",
        kermit->max_packet_length,
        kermit->max_ext_packet_length);

    return true;
}

static kermit_packet_t* kermit_encode_file_data_packet(kermit_t* kermit) {
    furi_check(kermit->file_transfer_state == KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA);
    furi_check(kermit->max_ext_packet_length >= 2);

    uint8_t* encoded_data = malloc(kermit->max_ext_packet_length);
    uint8_t* encoded_data_write = encoded_data;

    uint8_t* read_buffer = malloc(kermit->max_ext_packet_length / 2);
    const size_t read_length = kermit->io->src_file_read(
        kermit->io_context, read_buffer, kermit->max_ext_packet_length / 2);

    for(size_t i = 0; i < read_length; i++) {
        const uint8_t file_char = read_buffer[i];

        if(((file_char & 0x7F) < 0x20) || (file_char == 0x7F)) {
            *encoded_data_write++ = KERMIT_CONTROL_CHAR;
            *encoded_data_write++ = kermit_ctl(file_char);
        } else if(file_char == KERMIT_CONTROL_CHAR) {
            *encoded_data_write++ = KERMIT_CONTROL_CHAR;
            *encoded_data_write++ = KERMIT_CONTROL_CHAR;
        } else {
            *encoded_data_write++ = file_char;
        }
    }
    free(read_buffer);
    const size_t encoded_length = encoded_data_write - encoded_data;

    kermit_packet_t* packet = NULL;
    if(encoded_length == 0) {
        FURI_LOG_I(TAG, "EOF packet");
        packet = kermit_create_packet(kermit, KERMIT_PACKET_TYPE_EOF, NULL, 0);
        kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_SEND_FILE_EOF;
    } else {
        KERMIT_LOG("Packeted %d bytes from %d file bytes", encoded_length, read_length);
        packet = kermit_create_ext_packet(
            kermit, KERMIT_PACKET_TYPE_DATA, encoded_data, encoded_length);
    }

    free(encoded_data);
    return packet;
}

static kermit_packet_t* kermit_encode_file_header_packet(kermit_t* kermit, const char* filename) {
    return kermit_create_packet(
        kermit, KERMIT_PACKET_TYPE_FILE, (uint8_t*)filename, strlen(filename));
}

static bool kermit_process_packet(kermit_t* kermit) {
    furi_check(kermit->rx.contents != NULL);

    KERMIT_LOG(
        "Received packet type: %c, seq: %i, data: %s",
        kermit->rx.type,
        kermit->rx.seq,
        kermit->rx.contents->sz ? (const char*)kermit->rx.contents->data : "NULL");

    if(kermit_next_seq(kermit->rx.seq) != kermit->seq_counter) {
        FURI_LOG_E(TAG, "Broken sequencing");
        return false;
    }

    kermit_packet_t* response_packet = NULL;
    bool rx_packet_is_sane = true;

    switch(kermit->rx.type) {
    case KERMIT_PACKET_TYPE_ACK:
        KERMIT_LOG("ACK received, state: %d", kermit->file_transfer_state);
        switch(kermit->file_transfer_state) {
        case KERMIT_FILE_TRANSFER_STATE_SYNC_PARAMS:
            kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA;
            rx_packet_is_sane = kermit_parse_session_params(kermit);
            response_packet = kermit_encode_file_header_packet(kermit, KERMIT_DEFAULT_FILE_NAME);
            break;

        case KERMIT_FILE_TRANSFER_STATE_SEND_FILE_DATA:
            // may change state to EOF
            response_packet = kermit_encode_file_data_packet(kermit);
            break;

        case KERMIT_FILE_TRANSFER_STATE_SEND_FILE_EOF:
            kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_SEND_BREAK;
            response_packet = kermit_create_packet(kermit, KERMIT_PACKET_TYPE_BREAK, NULL, 0);
            break;

        case KERMIT_FILE_TRANSFER_STATE_SEND_BREAK:
            kermit->file_transfer_state = KERMIT_FILE_TRANSFER_STATE_DONE;
            FURI_LOG_I(TAG, "File transfer done");
            kermit_packet_free(kermit->rx.contents);
            kermit->rx.contents = NULL;
            return true;

        default:
            FURI_LOG_E(TAG, "Invalid state for ACK packet: %d", kermit->file_transfer_state);
            rx_packet_is_sane = false;
            break;
        }
        break;

    case KERMIT_PACKET_TYPE_NAK:
        FURI_LOG_E(TAG, "NAK received");
        break;

    default:
        FURI_LOG_E(TAG, "Unsupported packet type: '%c' (%02xh)", kermit->rx.type, kermit->rx.type);
        break;
    }

    bool tx_failed = false;
    if(response_packet) {
        tx_failed = !kermit_tx_and_release_packet(kermit, response_packet);
    }

    kermit_packet_free(kermit->rx.contents);
    kermit->rx.contents = NULL;

    return rx_packet_is_sane && (response_packet != NULL) && !tx_failed;
}
