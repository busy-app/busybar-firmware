#pragma once
#include "loader.h"

#include <toolbox/api_lock.h>

typedef struct {
    char* args;
    FuriThread* thread;
    bool insomniac;
} LoaderAppData;

struct Loader {
    FuriEventLoop* event_loop;
    FuriMessageQueue* queue;
    FuriPubSub* pubsub;
    LoaderAppData app;
};

typedef enum {
    LoaderMessageTypeStart,
    LoaderMessageTypeStop,
    LoaderMessageTypeAppClosed,
    LoaderMessageTypeLock,
    LoaderMessageTypeUnlock,
    LoaderMessageTypeIsLocked,
    LoaderMessageTypeGetApplicationName,
    LoaderMessageTypeSendCustomSignal,
    LoaderMessageTypeMax,
} LoaderMessageType;

typedef struct {
    const char* name;
    const char* args;
    FuriString* error_message;
} LoaderMessageStartByName;

typedef struct {
    uint32_t signal;
    void* arg;
} LoaderMessageLoaderSendCustomSignal;

typedef struct {
    LoaderStatus value;
} LoaderMessageLoaderStatusResult;

typedef struct {
    bool value;
} LoaderMessageBoolResult;

typedef struct {
    FuriApiLock api_lock;
    LoaderMessageType type;

    union {
        LoaderMessageStartByName start;
        LoaderMessageLoaderSendCustomSignal custom_signal;
        FuriString* application_name;
    };

    union {
        LoaderMessageLoaderStatusResult* status_value;
        LoaderMessageBoolResult* bool_value;
    };
} LoaderMessage;

typedef void (*LoaderMessageHandler)(Loader* loader, const LoaderMessage* message);
