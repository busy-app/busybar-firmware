#include "http_api.h"
#include <toolbox/path.h>
#include <audio/audio.h>

#define TAG "HttpAudio"

#define AUDIO_ASSETS_DIR EXT_PATH("assets")

typedef struct {
    size_t len_remain;
    void* file;
} UploadClientCtx;

static bool
    api_audio_play_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(ctx);
    char file_path[32];
    bool success = false;

    do {
        if(msg->query.len == 0) {
            break;
        }

        int var_len = mg_http_get_var(&msg->query, "path", file_path, sizeof(file_path));
        if(var_len <= 0) {
            break;
        }

        FuriString* path =
            furi_string_alloc_printf("%s/%.*s", AUDIO_ASSETS_DIR, var_len, file_path);
        Audio* audio = furi_record_open(RECORD_AUDIO);
        success = audio_play_file(audio, furi_string_get_cstr(path));
        furi_record_close(RECORD_AUDIO);
        furi_string_free(path);

    } while(0);

    if(!success) {
        mg_http_reply(conn, 400, "", "Bad Request");
        return true;
    }

    if(success) {
        mg_http_reply(conn, 200, "Content-Type: application/json\r\n", "{\"result\":\"OK\"}\n");
    }

    return true;
}

static bool
    api_audio_delete_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    UNUSED(conn);
    UNUSED(msg);
    UNUSED(ctx);
    FURI_LOG_I(TAG, "DELETE");

    Audio* audio = furi_record_open(RECORD_AUDIO);
    audio_stop(audio);
    furi_record_close(RECORD_AUDIO);

    mg_http_reply(conn, 200, "Content-Type: application/json\r\n", "{\"result\":\"OK\"}\n");
    return true;
}

static const HttpHandler api_audio_handlers[] = {
    {
        .uri = "#/play",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_audio_play_callback,
    },
    {
        .uri = "#/play",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_audio_delete_callback,
    },
};

typedef struct {
    HttpHandlersList_t handlers;
} ApiAudioCtx;

void* http_api_audio_alloc(void) {
    ApiAudioCtx* context = malloc(sizeof(ApiAudioCtx));
    HttpHandlersList_init(context->handlers);

    for(size_t i = COUNT_OF(api_audio_handlers); i > 0; i--) {
        http_handler_add(context->handlers, &api_audio_handlers[i - 1]);
    }
    return context;
}

void http_api_audio_free(void* ctx) {
    furi_assert(ctx);
    ApiAudioCtx* context = ctx;
    HttpHandlersList_clear(context->handlers);
    free(context);
}

bool http_api_audio_callback(struct mg_connection* conn, struct mg_http_message* msg, void* ctx) {
    ApiAudioCtx* context = ctx;

    return http_handle_request(context->handlers, conn, msg);
}
