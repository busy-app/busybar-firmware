/**
 * @file js_fetch.h
 *
 * @brief fetch() JS API implementation
 */
#pragma once
#include "js_runner_i.h"
#include <m-deque.h>
#include <sized_buffer.h>

typedef struct JsFetch JsFetch;

typedef enum JsFetchEventType {
    JsFetchEventTypeInvalid,
    JsFetchEventTypeHeaders,
    JsFetchEventTypeRxData,
    JsFetchEventTypeError,
    JsFetchEventTypeDone,
    JsFetchEventTypeThreadExit,
} JsFetchEventType;

typedef struct JsFetchEvent {
    JsFetchEventType type;
    JsFetch* instance;
    union {
        SizedBuffer data;
        FuriString* error;
    };
} JsFetchEvent;

typedef enum JsFetchDataEventType {
    JsFetchDataEventTypeInvalid,
    JsFetchDataEventTypeData,
    JsFetchDataEventTypeDone,
    JsFetchDataEventTypeError,
} JsFetchDataEventType;

typedef struct JsFetchDataEvent {
    JsFetchDataEventType type;
    union {
        SizedBuffer data;
        FuriString* error;
    };
} JsFetchDataEvent;

M_DEQUE_DEF(DataEventQueue, JsFetchDataEvent, M_POD_OPLIST);

/**
 * @brief Data sink callback - called when there is new event.
 *
 * @return true if event is consumed
 */
typedef bool (
    *JsFetchDataSinkCallback)(JsFetch* instance, JsFetchDataEvent* event, void* callback_context);

typedef struct JsFetchFetch {
    ChildStatus status;
    FuriThread* thread;
    Fetch* fetch;
} JsFetchFetch;

typedef struct JsFetchSink {
    ChildStatus status;
    JsFetchDataSinkCallback on_event;
    void* context;
    bool feeding;
} JsFetchSink;

typedef struct JsFetchPromise {
    ChildStatus status;
    jerry_value_t promise;
} JsFetchPromise;

typedef struct JsFetchResponse {
    ChildStatus status;
} JsFetchResponse;

typedef struct JsFetch {
    JsRunnerApp* app;
    FuriMessageQueue* event_queue;
    FetchRequest request;

    DataEventQueue_t chunk_queue;

    JsFetchFetch fetch;
    JsFetchSink sink;
    JsFetchPromise promise;
    JsFetchResponse response;
} JsFetch;

extern const jerry_object_native_info_t js_fetch_response_native_info;

void js_setup_fetch(void);

void js_fetch_process_event(const JsFetchEvent* event);

/**
 * @brief Set a data sink which receives incoming data chunks.
 *
 * Data sink, when set, has an exclusive access to data chunks through its callback.
 * It is not possible to set another data sink.
 *
 * @param instance JsFetch instance
 * @param callback_context user pointer to be passed to the callback
 * @param callback if NULL, data sink is no more
 * @return true if data sink has been set, false if another data sink is or has already been active.
 */
bool js_fetch_set_data_sink(
    JsFetch* instance,
    JsFetchDataSinkCallback callback,
    void* callback_context);

/**
 * @brief Indicate that a data sink is ready to receive new data.
 * @param instance JsFetch instance
 */
void js_fetch_data_sink_ready(JsFetch* instance);

/**
 * Cancel ongoing fetch activity
 *
 * @param instance JsFetch instance
 */
bool js_fetch_cancel(JsFetch* instance);

/**
 * @brief If fetch thread is running, indicate it to stop.
 *
 * @param instance JsFetch instance
 */
void js_fetch_abort(JsFetch* instance);
