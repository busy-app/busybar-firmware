#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JsonConfigStatusOk,
    JsonConfigStatusMissing, /**< File or JSON object is missing, was created/set to default (if enabled) */
    JsonConfigStatusError, /**< FS error */
} JsonConfigStatus;

typedef struct JsonConfig JsonConfig;

/**
 * @brief Allocate and initialize a config instance.
 *
 * @return pointer to the created instance.
 */
JsonConfig* json_config_alloc(void);

/**
 * @brief Free the config instance.
 *
 * If some changes were made using json_config_write_x, this function will write changes to the file.
 * @param a pointer to the instance to be freed.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_free(JsonConfig* inst);

/**
 * @brief Load config from a file.
 *
 * @param a pointer to the config instance.
 * @param a file path string.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_open(JsonConfig* inst, const char* file_path);

/**
 * @brief Read an integer value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a pointer to the config instance.
 * @param object name.
 * @param a pointer to store the value.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_int(JsonConfig* inst, const char* key, int* val, int* val_default);

/**
 * @brief Read a number (float) value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a pointer to the config instance.
 * @param object name.
 * @param a pointer to store the value.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_number(JsonConfig* inst, const char* key, float* val, float* val_default);

/**
 * @brief Read a boolean value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a pointer to the config instance.
 * @param object name.
 * @param a pointer to store the value.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_bool(JsonConfig* inst, const char* key, bool* val, bool* val_default);

/**
 * @brief Read a string value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a pointer to the config instance.
 * @param object name.
 * @param a pointer to store the string, valid untill the json_config_free or the next object modification
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_str(JsonConfig* inst, const char* key, char** val, char* val_default);

/**
 * @brief Write an integer value.
 * 
 * @param a pointer to the config instance.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_int(JsonConfig* inst, const char* key, int val);

/**
 * @brief Write a number (float) value.
 * 
 * @param a pointer to the config instance.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_number(JsonConfig* inst, const char* key, float val);

/**
 * @brief Write a boolean value.
 * 
 * @param a pointer to the config instance.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_bool(JsonConfig* inst, const char* key, bool val);

/**
 * @brief Write a string.
 * 
 * @param a pointer to the config instance.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_str(JsonConfig* inst, const char* key, const char* val);

/**
 * @brief Read an integer value - simpler version for a single value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a file path string.
 * @param object name.
 * @param a pointer to store the value.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_single_int(char* file_path, char* key, int* val, int* val_default);

/**
 * @brief Read a number (float) value - simpler version for a single value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a file path string.
 * @param object name.
 * @param a pointer to store the value.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_read_single_number(
    char* file_path,
    const char* key,
    float* val,
    float* val_default);

/**
 * @brief Read a boolean value - simpler version for a single value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a file path string.
 * @param object name.
 * @param a pointer to store the value.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_single_bool(char* file_path, const char* key, bool* val, bool* val_default);

/**
 * @brief Read a string value - simpler version for a single value.
 * 
 * If value is missing, it will be written to default value (if val_default != NULL)
 * @param a file path string.
 * @param object name.
 * @param a pointer to store the string, you have to free it after use.
 * @param a pointer to default value (set to NULL to bypass default value write).
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus
    json_config_read_single_str(char* file_path, const char* key, char** val, char* val_default);

/**
 * @brief Write an integer value - simpler version for a single value.
 * 
 * @param a file path string.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_single_int(char* file_path, char* key, int val);
/**
 * @brief Write a number (float) value - simpler version for a single value.
 * 
 * @param a file path string.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_single_number(char* file_path, const char* key, float val);

/**
 * @brief Write a boolean value - simpler version for a single value.
 * 
 * @param a file path string.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_single_bool(char* file_path, const char* key, bool val);

/**
 * @brief Write a string - simpler version for a single value.
 * 
 * @param a file path string.
 * @param object name.
 * @param value to write.
 * @return Operation status (JsonConfigStatus).
 */
JsonConfigStatus json_config_write_single_str(char* file_path, const char* key, const char* val);

#ifdef __cplusplus
}
#endif
