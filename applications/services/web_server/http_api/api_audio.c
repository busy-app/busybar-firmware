#include "http_api.h"
#include <toolbox/path.h>
#include <audio/audio.h>

#define TAG "HttpAudio"

#define AUDIO_ASSETS_DIR  EXT_PATH("assets")
#define FILE_NAME_LEN_MAX 32

typedef struct {
    size_t len_remain;
    void* file;
} UploadClientCtx;

static bool api_audio_play_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    char temp_str[FILE_NAME_LEN_MAX];
    bool success = false;

    FuriString* file_path = furi_string_alloc();
    do {
        if(msg->query.len == 0) {
            break;
        }

        int var_len = mg_http_get_var(&msg->query, "app_id", temp_str, sizeof(temp_str));
        if(var_len <= 0) {
            return false;
        }
        furi_string_printf(file_path, "%s/%.*s", AUDIO_ASSETS_DIR, var_len, temp_str);

        var_len = mg_http_get_var(&msg->query, "path", temp_str, sizeof(temp_str));
        if(var_len <= 0) {
            break;
        }
        furi_string_cat_printf(file_path, "/%.*s", var_len, temp_str);

        Audio* audio = furi_record_open(RECORD_AUDIO);
        success = audio_play_file(audio, furi_string_get_cstr(file_path));
        furi_record_close(RECORD_AUDIO);

    } while(0);

    furi_string_free(file_path);
    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static bool api_audio_delete_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(conn);
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Audio* audio = furi_record_open(RECORD_AUDIO);
    audio_stop(audio);
    furi_record_close(RECORD_AUDIO);

    MG_REPLY_OK(conn);
    return true;
}

static bool api_audio_get_volume_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    Audio* audio = furi_record_open(RECORD_AUDIO);
    float volume = audio_get_volume(audio);
    furi_record_close(RECORD_AUDIO);

    FuriString* json_str = furi_string_alloc_printf("\"volume\":%lu", (uint32_t)(volume * 100.f));

    MG_REPLY_OK_BODY(conn, "{%s}\n", furi_string_get_cstr(json_str));
    furi_string_free(json_str);
    return true;
}

static bool api_audio_set_volume_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    UNUSED(msg);
    UNUSED(ctx);

    if(!IS_HTTP_ENDPOINT(path)) return false;

    bool success = false;
    do {
        if(msg->query.len == 0) break;

        char value_str[5];
        int volume = 0;

        int value_len = mg_http_get_var(&msg->query, "volume", value_str, sizeof(value_str));

        if(value_len <= 0) break;

        int value_num = sscanf(value_str, "%u", &volume);

        if(value_num == 1) {
            if((volume > 100) || (volume < 0)) break;
            Audio* audio = furi_record_open(RECORD_AUDIO);
            audio_set_volume(audio, (float)volume / 100.f);
            furi_record_close(RECORD_AUDIO);
            success = true;
        }
    } while(0);

    if(success) {
        MG_REPLY_OK(conn);
    } else {
        MG_REPLY_BAD_REQUEST(conn);
    }

    return true;
}

static const HttpHandler api_audio_handlers[] = {
    {
        .uri = "play",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_audio_play_callback,
    },
    {
        .uri = "play",
        .method = "DELETE",
        .type = HttpHandlerCustom,
        .on_request = api_audio_delete_callback,
    },
    {
        .uri = "volume",
        .method = "GET",
        .type = HttpHandlerCustom,
        .on_request = api_audio_get_volume_callback,
    },
    {
        .uri = "volume",
        .method = "POST",
        .type = HttpHandlerCustom,
        .on_request = api_audio_set_volume_callback,
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

bool http_api_audio_callback(
    FuriString* path,
    struct mg_connection* conn,
    struct mg_http_message* msg,
    void* ctx) {
    ApiAudioCtx* context = ctx;

    return http_handle_request(path, context->handlers, conn, msg);
}
