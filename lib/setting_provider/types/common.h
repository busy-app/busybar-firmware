#pragma once

#include <furi.h>
#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

FURI_ALWAYS_INLINE
static bool json_read_bool(cJSON* json_node, const char* key, bool* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsBool(item) ? *value = cJSON_IsTrue(item), true : false;
}

FURI_ALWAYS_INLINE
static void json_write_bool(cJSON* json_node, const char* key, bool value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsBool(item)) {
        cJSON_SetBoolValue(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddBoolToObject(json_node, key, value);
    }
}

FURI_ALWAYS_INLINE
static bool json_read_int(cJSON* json_node, const char* key, int* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsNumber(item) ? *value = item->valueint, true : false;
}

FURI_ALWAYS_INLINE
static void json_write_int(cJSON* json_node, const char* key, int value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsNumber(item)) {
        cJSON_SetIntValue(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddNumberToObject(json_node, key, value);
    }
}

FURI_ALWAYS_INLINE
static bool json_read_float(cJSON* json_node, const char* key, float* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsNumber(item) ? *value = item->valuedouble, true : false;
}

FURI_ALWAYS_INLINE
static void json_write_float(cJSON* json_node, const char* key, float value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsNumber(item)) {
        cJSON_SetNumberValue(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddNumberToObject(json_node, key, value);
    }
}

FURI_ALWAYS_INLINE
static bool json_read_string(cJSON* json_node, const char* key, const char** value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsString(item) ? *value = item->valuestring, true : false;
}

FURI_ALWAYS_INLINE
static void json_write_string(cJSON* json_node, const char* key, const char* value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);

    if(cJSON_IsString(item)) {
        cJSON_SetValuestring(item, value);
    } else {
        cJSON_DeleteItemFromObject(json_node, key);
        cJSON_AddStringToObject(json_node, key, value);
    }
}

FURI_ALWAYS_INLINE
static bool json_read_object(cJSON* json_node, const char* key, cJSON** value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return cJSON_IsObject(item) ? *value = item, true : false;
}

FURI_ALWAYS_INLINE
static void json_write_object(cJSON* json_node, const char* key, cJSON* value) {
    cJSON_DeleteItemFromObject(json_node, key);
    cJSON_AddItemToObject(json_node, key, value);
}

FURI_ALWAYS_INLINE
static bool json_read_any(cJSON* json_node, const char* key, cJSON** value) {
    cJSON* item = cJSON_GetObjectItem(json_node, key);
    return item ? *value = item, true : false;
}

FURI_ALWAYS_INLINE
static void json_write_any(cJSON* json_node, const char* key, cJSON* value) {
    cJSON_DeleteItemFromObject(json_node, key);
    cJSON_AddItemToObject(json_node, key, value);
}

#ifdef __cplusplus
}
#endif
