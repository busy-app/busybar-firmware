#include "api_tokens.h"

#include <storage/storage.h>
#include <cjson/cJSON.h>
#include <toolbox/sha256_calc.h>
#include <mbedtls/base64.h>
#include <furi_hal_rtc.h>
#include <furi_hal_random.h>

// ===========
// Definitions
// ===========

#define TAG "Tokens"

#define TOKEN_LIST_DIR      EXT_PATH("apps_data/tokens")
#define TOKEN_LIST_PATH     EXT_PATH("apps_data/tokens/access.json")
#define TOKEN_LIST_MAX_SIZE (32 * 1024)

#define TOKEN_FLUSH_PERIOD (furi_ms_to_ticks(30 * 1000))

#define NEW_TOKEN_LENGTH               (API_TOKENS_LENGTH)
#define BASE64_ENCODING_EFFICIENCY     (0.73f)
#define TOKEN_DISPLAY_ID_JOINER        "…"
#define TOKEN_DISPLAY_ID_RIGHT_PORTION (6)
#define TOKEN_DISPLAY_ID_LENGTH \
    (API_TOKENS_SHORT_ID_LENGTH + strlen(TOKEN_DISPLAY_ID_JOINER) + TOKEN_DISPLAY_ID_RIGHT_PORTION)

struct ApiTokens {
    Storage* storage;
    FuriMutex* mutex;

    // all following fields are protected by `mutex`:

    ApiTokensEntry* entries;
    size_t entry_count;

    FuriTimer* flush_timer;
    bool is_dirty;
};

// ============================
// Stateless internal functions
// ============================

static void* api_tokens_realloc(void* previous, size_t size) {
    if(size) {
        return realloc(previous, size);
    } else {
        free(previous);
        return NULL;
    }
}

static void api_tokens_generate(char* out, size_t out_size) {
    furi_assert(out);

    uint8_t randomness[(int)ceilf(NEW_TOKEN_LENGTH * BASE64_ENCODING_EFFICIENCY)];
    furi_hal_random_fill_buf(randomness, sizeof(randomness));

    size_t token_length;
    uint8_t token[NEW_TOKEN_LENGTH + 8];
    mbedtls_base64_encode(token, sizeof(token), &token_length, randomness, sizeof(randomness));
    furi_assert(token_length >= NEW_TOKEN_LENGTH);
    token[NEW_TOKEN_LENGTH] = '\0';

    const char* blacklisted_chars = "+/";
    for(size_t i = 0; i < NEW_TOKEN_LENGTH; i++) {
        const char* blacklisted_char = strchr(blacklisted_chars, token[i]);
        if(blacklisted_char) {
            size_t blacklist_idx = blacklisted_char - blacklisted_chars;
            token[i] = 'A' + blacklist_idx;
        }
    }

    memset(out, 0, out_size);
    strncpy(out, (const char*)token, out_size - 1);
}

static void api_tokens_infer_ids(const char* token, char* short_id, char* display_id) {
    furi_assert(token);
    furi_assert(short_id);
    furi_assert(display_id);

    memcpy(short_id, token, API_TOKENS_SHORT_ID_LENGTH);
    short_id[API_TOKENS_SHORT_ID_LENGTH] = '\0';

    memcpy(display_id, short_id, API_TOKENS_SHORT_ID_LENGTH);
    display_id[API_TOKENS_SHORT_ID_LENGTH] = '\0';
    strcat(display_id, TOKEN_DISPLAY_ID_JOINER);
    memcpy(
        display_id + strlen(display_id),
        token + NEW_TOKEN_LENGTH - TOKEN_DISPLAY_ID_RIGHT_PORTION,
        TOKEN_DISPLAY_ID_RIGHT_PORTION);
    display_id[TOKEN_DISPLAY_ID_LENGTH] = '\0';
}

// ===========================
// Stateful internal functions
// ===========================

static void api_tokens_lock(ApiTokens* tokens) {
    furi_assert(tokens);
    furi_check(furi_mutex_acquire(tokens->mutex, FuriWaitForever) == FuriStatusOk);
}

static void api_tokens_unlock(ApiTokens* tokens) {
    furi_assert(tokens);
    furi_check(furi_mutex_release(tokens->mutex) == FuriStatusOk);
}

/**
 * Context: locked
 */
static bool api_tokens_load(ApiTokens* tokens, const char* path) {
    furi_assert(tokens);

    FuriString* json_serialized =
        storage_simply_read_entire_file_to_string(tokens->storage, path, TOKEN_LIST_MAX_SIZE);

    if(!json_serialized) {
        FURI_LOG_E(TAG, "Failed to load from '%s'", path);
        return false;
    }

    ApiTokensEntry* new_entries = NULL;
    size_t new_entry_count = 0;
    cJSON* json = cJSON_Parse(furi_string_get_cstr(json_serialized));
    bool success = true;

    do {
        cJSON* json_tokens = cJSON_GetObjectItem(json, "tokens");
        if(!cJSON_IsArray(json_tokens)) {
            FURI_LOG_E(TAG, "Incorrect data in '%s': 'tokens' must be an array", path);
            break;
        }

        new_entry_count = cJSON_GetArraySize(json_tokens);
        new_entries = api_tokens_realloc(NULL, sizeof(ApiTokensEntry) * new_entry_count);
        ApiTokensEntry* c_entry = &new_entries[0];

        cJSON* json_entry;
        cJSON_ArrayForEach(json_entry, json_tokens) {
            char* string = cJSON_GetStringValue(cJSON_GetObjectItem(json_entry, "short_id"));
            if(!string) {
                FURI_LOG_E(
                    TAG, "Incorrect data in '%s': 'tokens[i].short_id' must be a string", path);
                success = false;
                break;
            }
            c_entry->short_id = strdup(string);

            string = cJSON_GetStringValue(cJSON_GetObjectItem(json_entry, "display_id"));
            if(!string) {
                FURI_LOG_E(
                    TAG, "Incorrect data in '%s': 'tokens[i].display_id' must be a string", path);
                success = false;
                break;
            }
            c_entry->display_id = strdup(string);

            string = cJSON_GetStringValue(cJSON_GetObjectItem(json_entry, "owner"));
            if(!string) {
                FURI_LOG_E(
                    TAG, "Incorrect data in '%s': 'tokens[i].owner' must be a string", path);
                success = false;
                break;
            }
            c_entry->owner = strdup(string);

            string = cJSON_GetStringValue(cJSON_GetObjectItem(json_entry, "token_hash"));
            if(!string) {
                FURI_LOG_E(
                    TAG, "Incorrect data in '%s': 'tokens[i].token_hash' must be a string", path);
                success = false;
                break;
            }
            c_entry->token_hash = strdup(string);

            string = cJSON_GetStringValue(cJSON_GetObjectItem(json_entry, "created_at"));
            if(!string) {
                FURI_LOG_E(
                    TAG,
                    "Incorrect data in '%s': 'tokens[i].created_at' must be a string with a number",
                    path);
                success = false;
                break;
            }
            c_entry->created_at = atoll(string);

            string = cJSON_GetStringValue(cJSON_GetObjectItem(json_entry, "last_used_at"));
            if(!string) {
                FURI_LOG_E(
                    TAG,
                    "Incorrect data in '%s': 'tokens[i].last_used_at' must be a string with a number",
                    path);
                success = false;
                break;
            }
            c_entry->last_used_at = atoll(string);

            c_entry->type = ApiApiTokensEntryTypeHashed;

            c_entry++;
        }

        if(!success) break;
    } while(0);

    cJSON_Delete(json);
    furi_string_free(json_serialized);

    if(success) {
        free(tokens->entries);
        tokens->entries = new_entries;
        tokens->entry_count = new_entry_count;
    }

    return success;
}

/**
 * Context: locked
 */
static bool api_tokens_dump(ApiTokens* tokens, const char* path) {
    furi_assert(tokens);

    cJSON* json = cJSON_CreateObject();
    cJSON* json_tokens = cJSON_AddArrayToObject(json, "tokens");

    for(size_t i = 0; i < tokens->entry_count; i++) {
        const ApiTokensEntry* c_entry = &tokens->entries[i];
        cJSON* json_entry = cJSON_CreateObject();
        furi_check(cJSON_AddItemToArray(json_tokens, json_entry));

        cJSON_AddStringToObject(json_entry, "short_id", c_entry->short_id);
        cJSON_AddStringToObject(json_entry, "display_id", c_entry->display_id);
        cJSON_AddStringToObject(json_entry, "owner", c_entry->owner);
        cJSON_AddStringToObject(json_entry, "token_hash", c_entry->token_hash);

        char buffer[64];
        snprintf(buffer, sizeof(buffer) - 1, "%lld", c_entry->created_at);
        cJSON_AddStringToObject(json_entry, "created_at", buffer);
        snprintf(buffer, sizeof(buffer) - 1, "%lld", c_entry->last_used_at);
        cJSON_AddStringToObject(json_entry, "last_used_at", buffer);
    }

    char* json_serialized = cJSON_Print(json);
    cJSON_Delete(json);

    bool success = storage_simply_write_entire_file(
        tokens->storage, path, json_serialized, strlen(json_serialized));

    if(!success) {
        FURI_LOG_E(TAG, "Failed to dump to '%s'", path);
    }

    free(json_serialized);
    return success;
}

/**
 * Context: unlocked
 */
static void api_tokens_flush_timer_callback(void* context) {
    furi_assert(context);
    ApiTokens* tokens = context;

    api_tokens_lock(tokens);

    if(tokens->is_dirty) {
        if(api_tokens_dump(tokens, TOKEN_LIST_PATH)) {
            tokens->is_dirty = false;
        }
    }

    api_tokens_unlock(tokens);
}

static void api_tokens_entry_free(ApiTokensEntry* entry) {
    furi_assert(entry);
    free(entry->short_id);
    free(entry->display_id);
    free(entry->owner);
    furi_assert(entry->type == ApiApiTokensEntryTypeHashed);
    free(entry->token_hash);
}

static ApiTokens* api_tokens_alloc(void) {
    ApiTokens* tokens = malloc(sizeof(ApiTokens));

    tokens->mutex = furi_mutex_alloc(FuriMutexTypeNormal);

    tokens->storage = furi_record_open(RECORD_STORAGE);
    storage_simply_mkdir(tokens->storage, TOKEN_LIST_DIR);

    api_tokens_load(tokens, TOKEN_LIST_PATH);

    tokens->flush_timer =
        furi_timer_alloc(api_tokens_flush_timer_callback, FuriTimerTypePeriodic, tokens);
    furi_check(furi_timer_start(tokens->flush_timer, TOKEN_FLUSH_PERIOD) == FuriStatusOk);

    return tokens;
}

void api_tokens_start(void) {
    ApiTokens* tokens = api_tokens_alloc();
    furi_record_create(RECORD_API_TOKENS, tokens);
}

// ==========
// Public API
// ==========

void api_tokens_mint(
    ApiTokens* tokens,
    const char* owner,
    ApiTokenInfoCallback callback,
    void* context) {
    furi_assert(tokens);
    furi_assert(owner);
    furi_assert(callback);

    char token[NEW_TOKEN_LENGTH + 1];
    api_tokens_generate(token, sizeof(token));

    api_tokens_lock(tokens);

    tokens->entry_count++;
    tokens->entries =
        api_tokens_realloc(tokens->entries, sizeof(ApiTokensEntry) * tokens->entry_count);
    ApiTokensEntry* entry = &tokens->entries[tokens->entry_count - 1];

    entry->type = ApiApiTokensEntryTypeFull;
    entry->short_id = malloc(API_TOKENS_SHORT_ID_LENGTH + 1);
    entry->display_id = malloc(TOKEN_DISPLAY_ID_LENGTH + 1);
    api_tokens_infer_ids(token, entry->short_id, entry->display_id);
    entry->created_at = furi_hal_rtc_get_timestamp_ms();
    entry->last_used_at = 0;
    entry->owner = strdup(owner);
    entry->full_token = token;

    callback(entry, context);

    FuriString* token_hash = furi_string_alloc();
    sha256_string_calc_buffer((const uint8_t*)token, strlen(token), token_hash);
    const char* token_hash_cstr = furi_string_get_cstr(token_hash);
    entry->token_hash = strdup(token_hash_cstr);
    furi_string_free(token_hash);
    entry->type = ApiApiTokensEntryTypeHashed;

    tokens->is_dirty = !api_tokens_dump(tokens, TOKEN_LIST_PATH);

    api_tokens_unlock(tokens);
}

void api_tokens_list(ApiTokens* tokens, ApiTokenInfoCallback callback, void* context) {
    furi_assert(tokens);
    furi_assert(callback);

    api_tokens_lock(tokens);

    for(size_t i = 0; i < tokens->entry_count; i++) {
        const ApiTokensEntry* entry = &tokens->entries[i];
        callback(entry, context);
    }

    api_tokens_unlock(tokens);
}

bool api_tokens_revoke(ApiTokens* tokens, const char* short_id) {
    furi_assert(tokens);
    furi_assert(short_id);

    api_tokens_lock(tokens);
    bool success = false;

    for(size_t i = 0; i < tokens->entry_count; i++) {
        ApiTokensEntry* entry = &tokens->entries[i];
        if(strcmp(entry->short_id, short_id) != 0) continue;

        api_tokens_entry_free(entry);

        size_t entries_before_matching = i;
        size_t entries_after_matching = tokens->entry_count - entries_before_matching - 1;
        ApiTokensEntry* move_dst = tokens->entries + entries_before_matching;
        ApiTokensEntry* move_src = tokens->entries + entries_before_matching + 1;
        memmove(move_dst, move_src, entries_after_matching * sizeof(ApiTokensEntry));

        tokens->is_dirty = true;
        tokens->entry_count--;
        tokens->entries =
            api_tokens_realloc(tokens->entries, tokens->entry_count * sizeof(ApiTokensEntry));

        success = true;
        break;
    }

    api_tokens_unlock(tokens);
    return success;
}

void api_tokens_reset_all(ApiTokens* tokens) {
    furi_assert(tokens);

    api_tokens_lock(tokens);

    if(tokens->entries) {
        for(size_t i = 0; i < tokens->entry_count; i++) {
            api_tokens_entry_free(&tokens->entries[i]);
        }
        free(tokens->entries);
        tokens->entries = NULL;
        tokens->entry_count = 0;
        tokens->is_dirty = true;
    }

    api_tokens_unlock(tokens);
}

bool api_tokens_validate_and_record_usage(ApiTokens* tokens, const char* full_token) {
    furi_assert(tokens);

    FuriString* token_hash = furi_string_alloc();
    sha256_string_calc_buffer((const uint8_t*)full_token, strlen(full_token), token_hash);
    const char* token_hash_cstr = furi_string_get_cstr(token_hash);

    bool success = false;
    api_tokens_lock(tokens);

    for(size_t i = 0; i < tokens->entry_count; i++) {
        ApiTokensEntry* entry = &tokens->entries[i];
        if(strcmp(entry->token_hash, token_hash_cstr) != 0) continue;

        success = true;
        entry->last_used_at = furi_hal_rtc_get_timestamp_ms();
        tokens->is_dirty = true;
        break;
    }

    api_tokens_unlock(tokens);

    furi_string_free(token_hash);
    return success;
}
