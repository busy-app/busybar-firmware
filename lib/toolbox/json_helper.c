#include "json_helper.h"
#include <storage/storage.h>
#include <cjson/cJSON.h>

struct JsonConfig {
    char* file_path;
    cJSON* root;
    bool write_pending;
};

JsonConfig* json_config_alloc(void) {
    JsonConfig* inst = malloc(sizeof(JsonConfig));
    return inst;
}

JsonConfigStatus json_config_free(JsonConfig* inst) {
    JsonConfigStatus status = JsonConfigStatusOk;

    if(inst->write_pending) {
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);

        if(storage_file_open(file, inst->file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
            char* buffer = cJSON_Print(inst->root);
            if(buffer) {
                size_t buffer_len = strlen(buffer);
                size_t bytes_written = storage_file_write(file, buffer, buffer_len);
                if(buffer_len != bytes_written) {
                    status = JsonConfigStatusError;
                }
            }
            free(buffer);
        } else {
            status = JsonConfigStatusError;
        }

        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }

    if(inst->file_path) {
        free(inst->file_path);
    }
    if(inst->root) {
        cJSON_Delete(inst->root);
    }
    free(inst);

    return status;
}

JsonConfigStatus json_config_open(JsonConfig* inst, const char* file_path) {
    furi_assert(inst);
    furi_assert(file_path);
    furi_assert(inst->file_path == NULL);

    inst->file_path = strdup(file_path);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    JsonConfigStatus status = JsonConfigStatusOk;
    do {
        if(!storage_file_open(file, file_path, FSAM_READ_WRITE, FSOM_OPEN_ALWAYS)) {
            status = JsonConfigStatusError;
            break;
        }

        const size_t file_size = storage_file_size(file);
        if(file_size == 0) {
            inst->root = cJSON_CreateObject();
            inst->write_pending = true;
            status = JsonConfigStatusMissing;
            break;
        }

        char* buffer = malloc(file_size + 1);
        if(storage_file_read(file, buffer, file_size) != file_size) {
            status = JsonConfigStatusError;
            free(buffer);
            break;
        }

        inst->root = cJSON_Parse(buffer);
        if(!inst->root) {
            inst->root = cJSON_CreateObject();
            inst->write_pending = true;
            status = JsonConfigStatusMissing;
        }
        free(buffer);

    } while(false);

    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);

    return status;
}

JsonConfigStatus
    json_config_read_int(JsonConfig* inst, const char* key, int* val, int* val_default) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    furi_assert(val);

    JsonConfigStatus status = JsonConfigStatusOk;
    cJSON* item = cJSON_GetObjectItem(inst->root, key);

    if(item) {
        if(cJSON_IsNumber(item)) {
            *val = item->valueint;
        } else {
            status = JsonConfigStatusMissing;
        }
    } else {
        status = JsonConfigStatusMissing;
    }

    if((status == JsonConfigStatusMissing) && (val_default)) {
        json_config_write_int(inst, key, *val_default);
        *val = *val_default;
    }

    return status;
}

JsonConfigStatus
    json_config_read_number(JsonConfig* inst, const char* key, float* val, float* val_default) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    furi_assert(val);

    JsonConfigStatus status = JsonConfigStatusOk;
    cJSON* item = cJSON_GetObjectItem(inst->root, key);

    if(item) {
        if(cJSON_IsNumber(item)) {
            *val = (float)item->valuedouble;
        } else {
            status = JsonConfigStatusMissing;
        }
    } else {
        status = JsonConfigStatusMissing;
    }

    if((status == JsonConfigStatusMissing) && (val_default)) {
        json_config_write_number(inst, key, *val_default);
        *val = *val_default;
    }

    return status;
}

JsonConfigStatus
    json_config_read_bool(JsonConfig* inst, const char* key, bool* val, bool* val_default) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    furi_assert(val);

    JsonConfigStatus status = JsonConfigStatusOk;
    cJSON* item = cJSON_GetObjectItem(inst->root, key);

    if(item) {
        if(cJSON_IsBool(item)) {
            *val = cJSON_IsTrue(item);
        } else {
            status = JsonConfigStatusMissing;
        }
    } else {
        status = JsonConfigStatusMissing;
    }

    if((status == JsonConfigStatusMissing) && (val_default)) {
        json_config_write_bool(inst, key, *val_default);
        *val = *val_default;
    }

    return status;
}

JsonConfigStatus json_config_read_str(
    JsonConfig* inst,
    const char* key,
    FuriString* val,
    const char* val_default) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    furi_assert(val);

    JsonConfigStatus status = JsonConfigStatusOk;
    cJSON* item = cJSON_GetObjectItem(inst->root, key);

    if(item) {
        if(cJSON_IsString(item)) {
            furi_string_set(val, item->valuestring);
        } else {
            status = JsonConfigStatusMissing;
        }
    } else {
        status = JsonConfigStatusMissing;
    }

    if((status == JsonConfigStatusMissing) && (val_default)) {
        json_config_write_str(inst, key, val_default);
        furi_string_set(val, val_default);
    }

    return status;
}

// TODO: unit tests
JsonConfigStatus json_config_delete(JsonConfig* inst, const char* key) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    JsonConfigStatus status = JsonConfigStatusOk;

    cJSON_DeleteItemFromObject(inst->root, key);
    inst->write_pending = true;

    return status;
}

JsonConfigStatus json_config_write_int(JsonConfig* inst, const char* key, int val) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    JsonConfigStatus status = JsonConfigStatusOk;

    cJSON_DeleteItemFromObject(inst->root, key);
    cJSON_AddNumberToObject(inst->root, key, val);
    inst->write_pending = true;

    return status;
}

JsonConfigStatus json_config_write_number(JsonConfig* inst, const char* key, float val) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    JsonConfigStatus status = JsonConfigStatusOk;

    cJSON_DeleteItemFromObject(inst->root, key);
    cJSON_AddNumberToObject(inst->root, key, (double)val);
    inst->write_pending = true;

    return status;
}

JsonConfigStatus json_config_write_bool(JsonConfig* inst, const char* key, bool val) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    JsonConfigStatus status = JsonConfigStatusOk;

    cJSON_DeleteItemFromObject(inst->root, key);
    cJSON_AddBoolToObject(inst->root, key, val);
    inst->write_pending = true;

    return status;
}

JsonConfigStatus json_config_write_str(JsonConfig* inst, const char* key, const char* val) {
    furi_assert(inst);
    furi_assert(inst->root);
    furi_assert(key);
    JsonConfigStatus status = JsonConfigStatusOk;

    cJSON_DeleteItemFromObject(inst->root, key);
    cJSON_AddStringToObject(inst->root, key, val);
    inst->write_pending = true;

    return status;
}

JsonConfigStatus
    json_config_read_single_int(char* file_path, char* key, int* val, int* val_default) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        status = json_config_read_int(cfg, key, val, val_default);
    } else {
        if(val_default) {
            *val = *val_default;
        }
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus json_config_read_single_number(
    char* file_path,
    const char* key,
    float* val,
    float* val_default) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        status = json_config_read_number(cfg, key, val, val_default);
    } else {
        if(val_default) {
            *val = *val_default;
        }
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus
    json_config_read_single_bool(char* file_path, const char* key, bool* val, bool* val_default) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        status = json_config_read_bool(cfg, key, val, val_default);
    } else {
        if(val_default) {
            *val = *val_default;
        }
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus json_config_read_single_str(
    char* file_path,
    const char* key,
    FuriString* val,
    const char* val_default) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        status = json_config_read_str(cfg, key, val, val_default);
    } else {
        if(val_default) {
            furi_string_set(val, val_default);
        }
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus json_config_write_single_int(char* file_path, char* key, int val) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        json_config_write_int(cfg, key, val);
        return json_config_free(cfg);
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus json_config_write_single_number(char* file_path, const char* key, float val) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        json_config_write_number(cfg, key, val);
        return json_config_free(cfg);
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus json_config_write_single_bool(char* file_path, const char* key, bool val) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        json_config_write_bool(cfg, key, val);
        return json_config_free(cfg);
    }
    json_config_free(cfg);

    return status;
}

JsonConfigStatus json_config_write_single_str(char* file_path, const char* key, const char* val) {
    JsonConfigStatus status = JsonConfigStatusOk;

    JsonConfig* cfg = json_config_alloc();
    status = json_config_open(cfg, file_path);
    if(status != JsonConfigStatusError) {
        json_config_write_str(cfg, key, val);
        return json_config_free(cfg);
    }
    json_config_free(cfg);

    return status;
}
