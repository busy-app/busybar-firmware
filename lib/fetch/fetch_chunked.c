#include "fetch_chunked.h"
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <furi/core/core_defines.h>
#include <furi/core/check.h>
#include <furi/core/log.h>

#define TAG "chunked"

typedef enum State {
    StateSizeHead,
    StateSizeTail,
    StateExt,
    StateSizeLf,
    StateChunk,
    StateChunkCr,
    StateChunkLf,
    StateTrailer,
    StateError,

    StateMax,
} State;

typedef struct FetchChunked {
    FetchChunkedDataCallback data_cb;
    FetchChunkedErrorCallback error_cb;
    void* callback_ctx;

    State state;
    uint64_t chunk_size;
} FetchChunked;

FetchChunked* fetch_chunked_alloc(
    FetchChunkedDataCallback data_cb,
    FetchChunkedErrorCallback error_cb,
    void* callback_ctx) {
    FetchChunked* instance = malloc(sizeof(FetchChunked));
    instance->data_cb = data_cb;
    instance->error_cb = error_cb;
    instance->callback_ctx = callback_ctx;
    instance->state = StateSizeHead;
    return instance;
}

void fetch_chunked_free(FetchChunked* instance) {
    free(instance);
}

static bool fetch_chunked_is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static int fetch_chunked_hex_to_int(char c) {
    if(c >= '0' && c <= '9') {
        return c - '0';
    } else if(c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if(c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else {
        furi_assert(!"not a hex digit");
        return 0;
    }
}

static void fetch_chunked_raise_error(FetchChunked* instance) {
    instance->state = StateError;
    instance->error_cb(instance->callback_ctx);
}

typedef size_t (*FetchChunkedParseStateHandler)(FetchChunked* instance, SizedBufferConst packet);

static size_t
    fetch_chunked_handle_state_size_head(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(fetch_chunked_is_hex_digit(c)) {
        instance->chunk_size = fetch_chunked_hex_to_int(c);
        instance->state = StateSizeTail;
        return 1;
    } else {
        fetch_chunked_raise_error(instance);
        return packet.size;
    }
}

static size_t
    fetch_chunked_handle_state_size_tail(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(fetch_chunked_is_hex_digit(c)) {
        int digit = fetch_chunked_hex_to_int(c);
        if(instance->chunk_size > UINT64_MAX / 16) {
            fetch_chunked_raise_error(instance);
            return packet.size;
        } else {
            instance->chunk_size = instance->chunk_size * 16 + digit;
        }
    } else if(c == '\r') {
        instance->state = StateSizeLf;
    } else {
        instance->state = StateExt;
    }
    return 1;
}

static size_t fetch_chunked_handle_state_ext(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\r') {
        instance->state = StateSizeLf;
    }
    return 1;
}

static size_t fetch_chunked_handle_state_size_lf(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\n') {
        if(instance->chunk_size == 0) {
            instance->state = StateTrailer;
        } else {
            instance->state = StateChunk;
        }
        return 1;
    } else {
        fetch_chunked_raise_error(instance);
        return packet.size;
    }
}

static size_t fetch_chunked_handle_state_chunk(FetchChunked* instance, SizedBufferConst packet) {
    size_t processed = MIN(instance->chunk_size, packet.size);
    instance->data_cb(
        (SizedBufferConst){.buffer = packet.buffer, .size = processed}, instance->callback_ctx);
    instance->chunk_size -= processed;
    if(instance->chunk_size == 0) {
        instance->state = StateChunkCr;
    }
    return processed;
}

static size_t
    fetch_chunked_handle_state_chunk_cr(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\r') {
        instance->state = StateChunkLf;
        return 1;
    } else {
        fetch_chunked_raise_error(instance);
        return packet.size;
    }
}

static size_t
    fetch_chunked_handle_state_chunk_lf(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\n') {
        instance->state = StateSizeHead;
        return 1;
    } else {
        fetch_chunked_raise_error(instance);
        return packet.size;
    }
}

static size_t fetch_chunked_handle_state_trailer(FetchChunked* instance, SizedBufferConst packet) {
    UNUSED(instance);
    return packet.size;
}

static size_t fetch_chunked_handle_state_error(FetchChunked* instance, SizedBufferConst packet) {
    UNUSED(instance);
    return packet.size;
}

static const FetchChunkedParseStateHandler fetch_chunked_parse_state_handlers[] = {
    [StateSizeHead] = fetch_chunked_handle_state_size_head,
    [StateSizeTail] = fetch_chunked_handle_state_size_tail,
    [StateExt] = fetch_chunked_handle_state_ext,
    [StateSizeLf] = fetch_chunked_handle_state_size_lf,
    [StateChunk] = fetch_chunked_handle_state_chunk,
    [StateChunkCr] = fetch_chunked_handle_state_chunk_cr,
    [StateChunkLf] = fetch_chunked_handle_state_chunk_lf,
    [StateTrailer] = fetch_chunked_handle_state_trailer,
    [StateError] = fetch_chunked_handle_state_error,
};

static_assert(COUNT_OF(fetch_chunked_parse_state_handlers) == StateMax);

bool fetch_chunked_process(FetchChunked* instance, SizedBufferConst packet) {
    size_t remaining = packet.size;
    const uint8_t* buffer = packet.buffer;

    while(remaining > 0) {
        size_t processed = fetch_chunked_parse_state_handlers[instance->state](
            instance, (SizedBufferConst){.buffer = buffer, .size = remaining});
        remaining -= processed;
        buffer += processed;
    }

    return instance->state >= StateTrailer;
}
