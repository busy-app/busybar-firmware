#include "compress.h"

#include <furi.h>
#include <heatshrink/heatshrink_encoder.h>
#include <heatshrink/heatshrink_decoder.h>
#include <stdint.h>

#include <zlib/zlib.h>

#define TAG "Compress"

/** Defines encoder and decoder window size */
#define COMPRESS_EXP_BUFF_SIZE_LOG (8u)

/** Defines encoder and decoder lookahead buffer size */
#define COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG (4u)

#define COMPRESS_ICON_ENCODED_BUFF_SIZE (256u)

const CompressConfigHeatshrink compress_config_heatshrink_default = {
    .window_sz2 = COMPRESS_EXP_BUFF_SIZE_LOG,
    .lookahead_sz2 = COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG,
    .input_buffer_sz = COMPRESS_ICON_ENCODED_BUFF_SIZE,
};


typedef void (*CompressStreamDecoderFreeFn)(CompressStreamDecoder* instance);
typedef bool (*CompressStreamDecoderReadFn)(CompressStreamDecoder* instance, uint8_t* data_out, size_t data_out_size);
typedef bool (*CompressStreamDecoderSeekFn)(CompressStreamDecoder* instance, size_t position);
typedef size_t (*CompressStreamDecoderTellFn)(CompressStreamDecoder* instance);
typedef bool (*CompressStreamDecoderResetFn)(CompressStreamDecoder* instance);

typedef struct CompressStreamDecoderOps {
    CompressStreamDecoderFreeFn free;
    CompressStreamDecoderReadFn read;
    CompressStreamDecoderSeekFn seek;
    CompressStreamDecoderTellFn tell;
    CompressStreamDecoderResetFn reset;
} CompressStreamDecoderOps;

static CompressStreamDecoder* hs_alloc(const CompressConfigHeatshrink* config, CompressIoCallback read_cb, void* read_context);
static void hs_free(CompressStreamDecoder* instance);
static bool hs_read(CompressStreamDecoder* instance, uint8_t* data_out, size_t data_out_size);
static bool hs_seek(CompressStreamDecoder* instance, size_t position);
static size_t hs_tell(CompressStreamDecoder* instance);
static bool hs_reset(CompressStreamDecoder* instance);

static CompressStreamDecoder* gz_alloc(CompressIoCallback read_cb, void* read_context);
static void gz_free(CompressStreamDecoder* instance);
static bool gz_read(CompressStreamDecoder* instance, uint8_t* data_out, size_t data_out_size);
static bool gz_seek(CompressStreamDecoder* instance, size_t position);
static size_t gz_tell(CompressStreamDecoder* instance);
static bool gz_reset(CompressStreamDecoder* instance);


static CompressStreamDecoderOps compress_ops[CompressTypeMax] = {
    [CompressTypeHeatshrink] = {
        .free = hs_free,
        .read = hs_read,
        .seek = hs_seek,
        .tell = hs_tell,
        .reset = hs_reset,
    },
    [CompressTypeGzip] = {
        .free = gz_free,
        .read = gz_read,
        .seek = gz_seek,
        .tell = gz_tell,
        .reset = gz_reset,
    },
};

struct CompressStreamDecoder {
    CompressStreamDecoderOps *ops;
    union {
        struct {
            heatshrink_decoder* decoder;
            size_t stream_position;
        };
        struct {
            z_stream zs;
            bool is_eof;
        };
    };
    size_t decode_buffer_size;
    size_t decode_buffer_position;
    uint8_t* decode_buffer;
    CompressIoCallback read_cb;
    void* read_context;
};

CompressStreamDecoder* compress_stream_decoder_alloc(
    CompressType type,
    const void* config,
    CompressIoCallback read_cb,
    void* read_context) {

    switch(type) {
    case CompressTypeHeatshrink:
        return hs_alloc(config, read_cb, read_context);
    case CompressTypeGzip:
        return gz_alloc(read_cb, read_context);
    default:
        furi_crash("Unimplemented");
        break;
    }
}

void compress_stream_decoder_free(CompressStreamDecoder* instance) {
    furi_check(instance);

    instance->ops->free(instance);
}

bool compress_stream_decoder_read(
    CompressStreamDecoder* instance,
    uint8_t* data_out,
    size_t data_out_size) {
    furi_check(instance);
    furi_check(data_out);

    return instance->ops->read(instance, data_out, data_out_size);
}

bool compress_stream_decoder_seek(CompressStreamDecoder* instance, size_t position) {
    furi_check(instance);

    return instance->ops->seek(instance, position);
}

size_t compress_stream_decoder_tell(CompressStreamDecoder* instance) {
    furi_check(instance);

    return instance->ops->tell(instance);
}

bool compress_stream_decoder_reset(CompressStreamDecoder* instance) {
    furi_check(instance);

    return instance->ops->reset(instance);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Heatshrink

static CompressStreamDecoder* hs_alloc(
    const CompressConfigHeatshrink* hs_config,
    CompressIoCallback read_cb, 
    void* read_context) {

    furi_check(hs_config);

    CompressStreamDecoder* instance = malloc(sizeof(CompressStreamDecoder));
    instance->ops = &compress_ops[CompressTypeHeatshrink];
    instance->decoder = heatshrink_decoder_alloc(
        hs_config->input_buffer_sz, hs_config->window_sz2, hs_config->lookahead_sz2);
    instance->stream_position = 0;
    instance->decode_buffer_size = hs_config->input_buffer_sz;
    instance->decode_buffer_position = 0;
    instance->decode_buffer = malloc(hs_config->input_buffer_sz);
    instance->read_cb = read_cb;
    instance->read_context = read_context;

    return instance;
}

static void hs_free(CompressStreamDecoder* instance) {
    heatshrink_decoder_free(instance->decoder);
    free(instance->decode_buffer);
    free(instance);
}

static bool hs_decode_stream_chunk(
    CompressStreamDecoder* sd,
    CompressIoCallback read_cb,
    void* read_context,
    uint8_t* decompressed_chunk,
    size_t decomp_chunk_size) {
    HSD_sink_res sink_res;
    HSD_poll_res poll_res;

    /* 
    First, try to output data from decoder to the output buffer. 
    If the we could fill the output buffer, return
    If the output buffer is not full, keep polling the decoder 
        until it has no more data to output.
    Then, read more data from the input and sink it to the decoder.
    Repeat until the input is exhausted or output buffer is full.
    */

    bool failed = false;
    bool can_sink_more = true;
    bool can_read_more = true;

    do {
        do {
            size_t poll_size = 0;
            poll_res = heatshrink_decoder_poll(
                sd->decoder, decompressed_chunk, decomp_chunk_size, &poll_size);
            if(poll_res < 0) {
                return false;
            }

            decomp_chunk_size -= poll_size;
            decompressed_chunk += poll_size;
        } while((poll_res == HSDR_POLL_MORE) && decomp_chunk_size);

        if(!decomp_chunk_size) {
            break;
        }

        if(can_read_more && (sd->decode_buffer_position < sd->decode_buffer_size)) {
            size_t read_size = read_cb(
                read_context,
                &sd->decode_buffer[sd->decode_buffer_position],
                sd->decode_buffer_size - sd->decode_buffer_position);
            sd->decode_buffer_position += read_size;
            can_read_more = read_size > 0;
        }

        while(sd->decode_buffer_position && can_sink_more) {
            size_t sink_size = 0;
            sink_res = heatshrink_decoder_sink(
                sd->decoder, sd->decode_buffer, sd->decode_buffer_position, &sink_size);
            can_sink_more = sink_res == HSDR_SINK_OK;
            if(sink_res < 0) {
                failed = true;
                break;
            }
            sd->decode_buffer_position -= sink_size;

            /* If some data was left in the buffer, move it to the beginning */
            if(sink_size && sd->decode_buffer_position) {
                memmove(
                    sd->decode_buffer, &sd->decode_buffer[sink_size], sd->decode_buffer_position);
            }
        }
    } while(!failed);

    return decomp_chunk_size == 0;
}


static bool hs_read(CompressStreamDecoder* instance, uint8_t* data_out, size_t data_out_size) {
    if(hs_decode_stream_chunk(
           instance, instance->read_cb, instance->read_context, data_out, data_out_size)) {
        instance->stream_position += data_out_size;
        return true;
    }
    return false;
}

static bool hs_seek(CompressStreamDecoder* instance, size_t position) {
    /* Check if requested position is ahead of current position 
       we can't rewind the input stream */
    furi_check(position >= instance->stream_position);

    /* Read and discard data up to requested position */
    uint8_t* dummy_buffer = malloc(instance->decode_buffer_size);
    bool success = true;

    while(instance->stream_position < position) {
        size_t bytes_to_read = position - instance->stream_position;
        if(bytes_to_read > instance->decode_buffer_size) {
            bytes_to_read = instance->decode_buffer_size;
        }
        if(!hs_read(instance, dummy_buffer, bytes_to_read)) {
            success = false;
            break;
        }
    }

    free(dummy_buffer);
    return success;
}

static size_t hs_tell(CompressStreamDecoder* instance) {
    return instance->stream_position;
}

static bool hs_reset(CompressStreamDecoder* instance) {
    /* Reset decoder and read buffer */
    heatshrink_decoder_reset(instance->decoder);
    instance->stream_position = 0;
    instance->decode_buffer_position = 0;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Gzip

#define GZ_DECODE_BUF_SIZE 8192

static CompressStreamDecoder* gz_alloc(CompressIoCallback read_cb, void* read_context) {
    furi_check(read_cb);

    CompressStreamDecoder *s = calloc(1, sizeof(CompressStreamDecoder));
    if(!s) {
        return NULL;
    }

    s->ops = &compress_ops[CompressTypeGzip];
    s->read_cb = read_cb;
    s->read_context = read_context;
    s->is_eof = false;
    s->decode_buffer_size = GZ_DECODE_BUF_SIZE;
    s->decode_buffer_position = 0;

    do {
        s->decode_buffer = malloc(GZ_DECODE_BUF_SIZE);
        if(!s->decode_buffer) {
            break;
        }

        do {
            memset(&s->zs, 0, sizeof(z_stream));
            int r = inflateInit2(&s->zs, 16 + MAX_WBITS); // magic parameter to allow gzip decoding
            if(r != Z_OK) {
                FURI_LOG_E(TAG, "inflateInit2 returned %d", r);
                break;
            }
            return s;
        } while(false);
        free(s->decode_buffer);
    } while(false);
    free(s);
    return NULL;
}

static void gz_free(CompressStreamDecoder* instance) {
    furi_check(instance);

    inflateEnd(&instance->zs);
    free(instance);
}

static bool gz_read(CompressStreamDecoder* instance, uint8_t* data_out, size_t data_out_size) {
    furi_check(instance);
    furi_check(data_out);

    z_stream *zs = &instance->zs;

    zs->next_out = data_out;
    zs->avail_out = data_out_size;

    while(zs->avail_out > 0) {
        if(zs->avail_in == 0 && !instance->is_eof) {
            // need more data
            int32_t r = instance->read_cb(instance->read_context, instance->decode_buffer, instance->decode_buffer_size);

            if(r < 0) {
                return false;
            } else if(r == 0) {
                instance->is_eof = true;
            } else {
                zs->next_in = instance->decode_buffer;
                zs->avail_in = (uInt)r;
            }
        }

        int r = inflate(zs, Z_NO_FLUSH);

        if(r == Z_STREAM_END) {
            // done
            break;
        } else if(r != Z_OK) {
            FURI_LOG_E(TAG, "gz_read: inflate returned %d (%s)", r, zs->msg);
            return false;
        } else if(instance->is_eof && zs->avail_in == 0) {
            // also done
            break;
        } else {
            // might need more input, continue
        }
    }

    size_t produced = data_out_size - zs->avail_out;

    return produced > 0;
}

static bool gz_seek(CompressStreamDecoder* instance, size_t position) {
    /* Check if requested position is ahead of current position 
       we can't rewind the input stream */
    furi_check(position >= instance->zs.total_out);

    // Read and discard data up to position
    const size_t buf_size = 1024;
    uint8_t *buf = malloc(buf_size);
    furi_check(buf);
    bool result = true;
    while(position > instance->zs.total_out) {
        size_t to_read = MIN(position - instance->zs.total_out, buf_size);
        if(!gz_read(instance, buf, to_read)) {
            FURI_LOG_E(TAG, "gz_read error");
            result = false;
            break;
        }
    }
    free(buf);
    return result;
}

static size_t gz_tell(CompressStreamDecoder* instance) {
    furi_check(instance);

    return instance->zs.total_out;
}

static bool gz_reset(CompressStreamDecoder* instance) {
    furi_check(instance);

    instance->is_eof = false;

    z_stream *zs = &instance->zs;
    inflateEnd(zs);
    memset(zs, 0, sizeof(z_stream));
    int r = inflateInit2(zs, 16 + MAX_WBITS);

    if(r != Z_OK) {
        FURI_LOG_E(TAG, "inflateInit2 returned %d", r);
    }
    return r == Z_OK;
}
