#include <furi/furi.h>
#include <time/time.h>
#include <js_runner/js_runner_i.h>
#include <storage/storage.h>
#include <path.h>

#include <utz/utz.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#include <jerryscript.h>
#pragma GCC diagnostic pop
#include <string.h>

#define TAG "JSGlue"

size_t jerry_port_context_alloc(size_t context_size) {
    size_t result = js_runner_thread_context_alloc(context_size);
    return result;
}

struct jerry_context_t* jerry_port_context_get(void) {
    void* result = js_runner_thread_context_get();
    return result;
}

void jerry_port_context_free(void) {
    js_runner_thread_context_free();
}

void jerry_port_init(void) {
}

void jerry_port_fatal(jerry_fatal_code_t code) {
    FURI_LOG_E(TAG, "jerryscript fatal error %d", code);
    furi_crash(code);
}

double jerry_port_current_time(void) {
    return (double)time_get_timestamp_ms();
}

int32_t jerry_port_local_tza(double unix_ms) {
    Time* time = furi_record_open(RECORD_TIME);
    TimeSettings settings;
    time_get_settings(time, &settings);
    furi_record_close(RECORD_TIME);
    DateTimeMs dt = datetime_timestamp_ms_to_datetime((time_t)unix_ms);
    utz_offset_t offset;
    utz_get_current_offset(&settings.timezone, &dt.dt, &offset);
    return (offset.hours * 60 + offset.minutes) * 60 * 1000;
}

jerry_char_t* jerry_port_path_normalize(const jerry_char_t* path_p, jerry_size_t path_size) {
    UNUSED(path_size);
    JS_TRACE("Normalize path: %s", path_p);
    FuriString* path_norm = furi_string_alloc();
    path_normalize((const char*)path_p, path_norm, true);
    jerry_char_t* result = malloc(furi_string_size(path_norm) + 1);
    strcpy((char*)result, furi_string_get_cstr(path_norm));
    furi_string_free(path_norm);
    JS_TRACE("Normalized: %s", result);
    return result;
}

void jerry_port_path_free(jerry_char_t* path_p) {
    free(path_p);
}

jerry_size_t jerry_port_path_base(const jerry_char_t* path_p) {
    JS_TRACE("Path base: %s", path_p);
    const char* last_slash = strrchr((const char*)path_p, '/');
    if(last_slash) {
        return last_slash - (const char*)path_p + 1;
    } else {
        return 0;
    }
}

static FuriString* get_normalized_root_path(const char* file_name_p) {
    FuriString* abs_path = furi_string_alloc();
    if(js_runner_get_root_path(abs_path)) {
        path_append(abs_path, file_name_p);
        FuriString* abs_path_norm = furi_string_alloc();
        path_normalize(furi_string_get_cstr(abs_path), abs_path_norm, false);
        furi_string_free(abs_path);
        return abs_path_norm;
    } else {
        furi_string_free(abs_path);
        return NULL;
    }
}

jerry_char_t* jerry_port_source_read(const char* file_name_p, jerry_size_t* out_size_p) {
    JS_TRACE("Source read: %s", file_name_p);
    FuriString* abs_path_norm = get_normalized_root_path(file_name_p);
    if(!abs_path_norm) {
        return NULL;
    }
    const char* abs_path_cstr = furi_string_get_cstr(abs_path_norm);
    JS_TRACE("Abs path: %s", abs_path_cstr);
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* f = storage_file_alloc(storage);
    jerry_char_t* result = NULL;
    do {
        if(!storage_file_open(f, abs_path_cstr, FSAM_READ, FSOM_OPEN_EXISTING)) {
            FURI_LOG_E(TAG, "Cannot open file %s", abs_path_cstr);
            break;
        }
        uint64_t file_size = storage_file_size(f);
        if(file_size == 0) {
            FURI_LOG_E(TAG, "File is empty %s", abs_path_cstr);
            break;
        }
        if(file_size > JS_RUNNER_MAX_SCRIPT_SIZE) {
            FURI_LOG_E(TAG, "File is too large %s", abs_path_cstr);
            break;
        }
        result = malloc(file_size);
        size_t read_bytes = storage_file_read(f, result, file_size);
        if(read_bytes != file_size) {
            FURI_LOG_E(TAG, "Error reading file %s", abs_path_cstr);
            free(result);
            break;
        } else {
            *out_size_p = file_size;
        }
    } while(false);
    storage_file_free(f);
    furi_record_close(RECORD_STORAGE);
    furi_string_free(abs_path_norm);
    return result;
}

void jerry_port_source_free(jerry_char_t* buffer_p) {
    free(buffer_p);
}
