#include "fetch_chunked.h"
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <furi/core/core_defines.h>
#include <furi/core/check.h>

typedef enum {
    FetchChunkedStateSizeHead,
    FetchChunkedStateSizeTail,
    FetchChunkedStateExt,
    FetchChunkedStateSizeLf,
    FetchChunkedStateChunk,
    FetchChunkedStateChunkCr,
    FetchChunkedStateChunkLf,
    FetchChunkedStateTrailer,
    FetchChunkedStateError,
    FetchChunkedStateMax,
} FetchChunkedState;

struct FetchChunked {
    FetchChunkedDataCallback data_callback;
    FetchChunkedErrorCallback error_callback;
    void* callback_context;

    FetchChunkedState state;
    uint64_t chunk_size;
};

FetchChunked* fetch_chunked_alloc(
    FetchChunkedDataCallback data_callback,
    FetchChunkedErrorCallback error_callback,
    void* callback_context) {
    FetchChunked* instance = malloc(sizeof(FetchChunked));
    instance->data_callback = data_callback;
    instance->error_callback = error_callback;
    instance->callback_context = callback_context;
    instance->state = FetchChunkedStateSizeHead;
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
        furi_crash("not a hex digit");
        return 0;
    }
}

static void fetch_chunked_raise_error(FetchChunked* instance) {
    instance->state = FetchChunkedStateError;
    instance->error_callback(instance->callback_context);
}

typedef size_t (*FetchChunkedParseStateHandler)(FetchChunked* instance, SizedBufferConst packet);

static size_t
    fetch_chunked_handle_state_size_head(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(fetch_chunked_is_hex_digit(c)) {
        instance->chunk_size = fetch_chunked_hex_to_int(c);
        instance->state = FetchChunkedStateSizeTail;
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
        instance->state = FetchChunkedStateSizeLf;
    } else {
        // Relaxed condition for extension, not checking for a semicolon
        instance->state = FetchChunkedStateExt;
    }
    return 1;
}

static size_t fetch_chunked_handle_state_ext(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\r') {
        instance->state = FetchChunkedStateSizeLf;
    }
    return 1;
}

static size_t fetch_chunked_handle_state_size_lf(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\n') {
        if(instance->chunk_size == 0) {
            instance->state = FetchChunkedStateTrailer;
        } else {
            instance->state = FetchChunkedStateChunk;
        }
        return 1;
    } else {
        fetch_chunked_raise_error(instance);
        return packet.size;
    }
}

static size_t fetch_chunked_handle_state_chunk(FetchChunked* instance, SizedBufferConst packet) {
    size_t processed = MIN(instance->chunk_size, packet.size);
    instance->data_callback(
        (SizedBufferConst){.buffer = packet.buffer, .size = processed},
        instance->callback_context);
    instance->chunk_size -= processed;
    if(instance->chunk_size == 0) {
        instance->state = FetchChunkedStateChunkCr;
    }
    return processed;
}

static size_t
    fetch_chunked_handle_state_chunk_cr(FetchChunked* instance, SizedBufferConst packet) {
    char c = *(char*)packet.buffer;
    if(c == '\r') {
        instance->state = FetchChunkedStateChunkLf;
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
        instance->state = FetchChunkedStateSizeHead;
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
    [FetchChunkedStateSizeHead] = fetch_chunked_handle_state_size_head,
    [FetchChunkedStateSizeTail] = fetch_chunked_handle_state_size_tail,
    [FetchChunkedStateExt] = fetch_chunked_handle_state_ext,
    [FetchChunkedStateSizeLf] = fetch_chunked_handle_state_size_lf,
    [FetchChunkedStateChunk] = fetch_chunked_handle_state_chunk,
    [FetchChunkedStateChunkCr] = fetch_chunked_handle_state_chunk_cr,
    [FetchChunkedStateChunkLf] = fetch_chunked_handle_state_chunk_lf,
    [FetchChunkedStateTrailer] = fetch_chunked_handle_state_trailer,
    [FetchChunkedStateError] = fetch_chunked_handle_state_error,
};

static_assert(COUNT_OF(fetch_chunked_parse_state_handlers) == FetchChunkedStateMax);

bool fetch_chunked_process(FetchChunked* instance, SizedBufferConst packet) {
    size_t remaining = packet.size;
    const uint8_t* buffer = packet.buffer;

    while(remaining > 0) {
        size_t processed = fetch_chunked_parse_state_handlers[instance->state](
            instance, (SizedBufferConst){.buffer = buffer, .size = remaining});
        remaining -= processed;
        buffer += processed;
    }

    return instance->state >= FetchChunkedStateTrailer;
}

bool fetch_chunked_is_complete(FetchChunked* instance) {
    return instance->state == FetchChunkedStateTrailer;
}
