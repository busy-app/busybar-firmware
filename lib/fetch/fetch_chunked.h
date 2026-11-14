/**
 * @file fetch_chunked.h
 * @brief Handle chunked HTTP responses
 */

#pragma once

#include <stdbool.h>

#include <toolbox/sized_buffer.h>

typedef struct FetchChunked FetchChunked;

typedef void (*FetchChunkedDataCallback)(SizedBufferConst data, void* context);
typedef void (*FetchChunkedErrorCallback)(void* context);

FetchChunked* fetch_chunked_alloc(
    FetchChunkedDataCallback data_cb,
    FetchChunkedErrorCallback error_cb,
    void* callback_ctx);

void fetch_chunked_free(FetchChunked* instance);

bool fetch_chunked_process(FetchChunked* instance, SizedBufferConst packet);

bool fetch_chunked_is_complete(FetchChunked* instance);
