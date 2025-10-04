#include "l10n.h"
#include "l10n_table.h"

#define TAG                    "L10nSrv"
#define FALLBACK_LOCALE        L10nLocaleEnUs
#define MAX_CANDIDATE_LOCALES  2
#define BUFFER_SIZE            128
#define Q_SIZE                 1
#define L10N_CONFIG_FILE       APP_DATA_PATH("config.json")
#define L10N_LOCALE_CONFIG_KEY "locale"

#include <stdarg.h>
#include <storage/storage.h>
#include <toolbox/api_lock.h>
#include <json_helper.h>
#include <furi_hal_nvm.h>

struct L10nSrv {
    FuriEventLoop* event_loop;
    FuriMessageQueue* request_queue;

    // safe to read concurrently, initialized once at startup:
    L10nLocale locale;
    Storage* storage;
};

struct L10nContext {
    L10nSrv* service;
    const L10nTable* tables[MAX_CANDIDATE_LOCALES];
    char buffer[BUFFER_SIZE];
};

// ===============
// Service helpers
// ===============

static void l10n_set_locale_in_storage(L10nLocale id) {
    furi_check(
        json_config_write_single_int(L10N_CONFIG_FILE, L10N_LOCALE_CONFIG_KEY, id) ==
        JsonConfigStatusOk);
    furi_hal_nvm_set_locale(id);
}

static L10nLocale l10n_get_locale_from_storage() {
    int json_value;
    if(json_config_read_single_int(L10N_CONFIG_FILE, L10N_LOCALE_CONFIG_KEY, &json_value, NULL) ==
       JsonConfigStatusOk) {
        if((json_value >= 0) && (json_value < L10nLocaleCOUNT)) return json_value;
    }

    FURI_LOG_W(TAG, "Failed to read locale from storage, using backup RAM");
    L10nLocale nvm_value = furi_hal_nvm_get_locale();
    if(nvm_value < L10nLocaleCOUNT) return nvm_value;

    FURI_LOG_W(TAG, "Failed to read locale backup RAM, using default");
    return FALLBACK_LOCALE;
}

// ===========
// Service API
// ===========

typedef enum {
    L10nSrvRequestTypeSetLocale,
} L10nSrvRequestType;

typedef struct {
    FuriApiLock lock;
    L10nSrvRequestType type;
    union {
        L10nLocale locale;
    };
} L10nSrvRequest;

const L10nLocaleInfo L10N_LOCALE_INFO[L10nLocaleCOUNT] = {
    [L10nLocaleEnUs] = {.self_name = "English (US)", .iso_name = "en-US"},
    [L10nLocaleRuRu] = {.self_name = "Русский", .iso_name = "ru-RU"},
};

static void l10n_process_request(FuriEventLoopObject* object, void* context) {
    furi_assert(object);
    furi_assert(context);
    FuriMessageQueue* request_queue = object;
    L10nSrv* service = context;
    furi_assert(service->request_queue == request_queue);
    L10nSrvRequest* request;
    furi_check(furi_message_queue_get(request_queue, &request, 0) == FuriStatusOk);

    switch(request->type) {
    case L10nSrvRequestTypeSetLocale:
        l10n_set_locale_in_storage(request->locale);
        // the setting will be applied after a restart
        break;
    }

    api_lock_unlock(request->lock);
}

static void l10n_synchronous_request(L10nSrv* service, L10nSrvRequest* request) {
    request->lock = api_lock_alloc_locked();
    furi_check(
        furi_message_queue_put(service->request_queue, &request, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(request->lock);
}

const L10nLocaleInfo* l10n_locale_info(L10nLocale id) {
    furi_check(id < L10nLocaleCOUNT);
    return &L10N_LOCALE_INFO[id];
}

void l10n_set_global_locale(L10nSrv* service, L10nLocale id) {
    L10nSrvRequest request = {
        .type = L10nSrvRequestTypeSetLocale,
        .locale = id,
    };
    l10n_synchronous_request(service, &request);
}

L10nLocale l10n_get_global_locale(L10nSrv* service) {
    return service->locale;
}

// ===========
// Context API
// ===========

static const L10nTable* l10n_table_try_get_from_storage_by_id(
    Storage* storage,
    const char* asset_path,
    const char* locale_name) {
    const L10nTable* table = NULL;
    FuriString* path = furi_string_alloc_printf("%s/%s", asset_path, locale_name);
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, furi_string_get_cstr(path), FSAM_READ, FSOM_OPEN_EXISTING))
            break;
        table = l10n_table_alloc_from_storage(file);
        furi_check(table);
        furi_check(storage_file_close(file));
    } while(0);

    storage_file_free(file);
    furi_string_free(path);
    return table;
}

static const L10nTable* l10n_table_try_get(
    Storage* storage,
    L10nSource source,
    const char* app_id_or_path,
    L10nLocale locale) {
    if(source == L10nSourceFlash) {
        return l10n_table_alloc_builtin(app_id_or_path, locale);
    } else if(source == L10nSourceStorage) {
        const char* name = l10n_locale_info(locale)->iso_name;
        return l10n_table_try_get_from_storage_by_id(storage, app_id_or_path, name);
    } else {
        furi_crash();
    }
}

L10nContext* l10n_context_open(L10nSrv* service, const char* app_id_or_path, L10nSource source) {
    L10nLocale locale = service->locale;
    Storage* storage = service->storage;

    L10nContext* context = malloc(sizeof(L10nContext));
    context->service = service;

    L10nLocale locales[MAX_CANDIDATE_LOCALES] = {
        locale,
        L10nLocaleEnUs,
    };

    bool has_at_least_one_locale = false;
    for(size_t i = 0; i < COUNT_OF(locales); i++) {
        context->tables[i] = l10n_table_try_get(storage, source, app_id_or_path, locales[i]);
        if(context->tables[i]) has_at_least_one_locale = true;
    }

    furi_check(has_at_least_one_locale);

    return context;
}

void l10n_context_close(L10nContext* context) {
    furi_check(context);

    for(size_t i = 0; i < MAX_CANDIDATE_LOCALES; i++) {
        const L10nTable* table = context->tables[i];
        if(table) l10n_table_free(table);
    }

    free(context);
}

static const char* l10n_get_template(L10nContext* context, L10nKey key) {
    furi_assert(context);

    const char* template;
    for(size_t i = 0; i < MAX_CANDIDATE_LOCALES; i++) {
        const L10nTable* table = context->tables[i];
        if(!table) continue;
        template = l10n_table_get(table, key);
        if(template) break;
    }

    furi_check(template);
    return template;
}

const char* l10n_get(L10nContext* context, L10nKey key, ...) {
    furi_check(context);

    va_list args;
    va_start(args, 0);
    const char* template = l10n_get_template(context, key);
    vsnprintf(context->buffer, sizeof(context->buffer), template, args);
    va_end(args);

    return context->buffer;
}

void l10n_get_into(L10nContext* context, char* buf, size_t buf_size, L10nKey key, ...) {
    furi_check(context);

    va_list args;
    va_start(args, 0);
    const char* template = l10n_get_template(context, key);
    vsnprintf(buf, buf_size, template, args);
    va_end(args);
}

void l10n_get_furi_str(L10nContext* context, FuriString* string, L10nKey key, ...) {
    furi_check(context);

    va_list args;
    va_start(args, 0);
    const char* template = l10n_get_template(context, key);
    furi_string_vprintf(string, template, args);
    va_end(args);
}

const char* l10n_get_resource(L10nContext* context, const char* template) {
    furi_check(context);
    furi_check(template);

    char* first_format = strchr(template, '%');
    char* last_format = strrchr(template, '%');
    furi_check(first_format);
    furi_check(first_format == last_format);
    furi_check(first_format[1] == 's');

    const char* candidates[] = {
        l10n_locale_info(context->service->locale)->iso_name,
        l10n_locale_info(FALLBACK_LOCALE)->iso_name,
        "",
    };
    const size_t candidate_count = COUNT_OF(candidates);

    for(size_t i = 0; i < candidate_count; i++) {
        const char* candidate = candidates[i];
        bool add_underscore = i != (candidate_count - 1);

        char locale[16];
        snprintf(locale, sizeof(locale), "%s%s", add_underscore ? "_" : "", candidate);
        snprintf(context->buffer, sizeof(context->buffer), template, locale);

        if(storage_file_exists(context->service->storage, context->buffer)) break;
    }

    return context->buffer;
}

// =============
// Service setup
// =============

L10nSrv* l10n_srv_alloc() {
    L10nSrv* l10n = malloc(sizeof(L10nSrv));

    l10n->storage = furi_record_open(RECORD_STORAGE);
    l10n->locale = l10n_get_locale_from_storage();
    l10n_set_locale_in_storage(l10n->locale); // synchronize both storages

    l10n->event_loop = furi_event_loop_alloc();
    l10n->request_queue = furi_message_queue_alloc(Q_SIZE, sizeof(L10nSrvRequest*));
    furi_event_loop_subscribe_message_queue(
        l10n->event_loop, l10n->request_queue, FuriEventLoopEventIn, l10n_process_request, l10n);

    return l10n;
}

int32_t l10n_srv(void* arg) {
    UNUSED(arg);

    L10nSrv* l10n = l10n_srv_alloc();
    furi_record_create(RECORD_L10N, l10n);
    furi_event_loop_run(l10n->event_loop);

    return 0;
}
