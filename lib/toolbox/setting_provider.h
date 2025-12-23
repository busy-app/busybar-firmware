/**
 * @file setting_provider.h
 * @brief JSON-based settings management with version migration support.
 *
 * This library provides a flexible framework for managing application settings
 * stored in JSON format. Features include:
 * - Type-safe setting load/save with validation
 * - Automatic default value handling
 * - Version migration support for backwards compatibility
 * - Support for bool, int, float, string, and custom types
 * - Write buffering (changes flushed on close)
 *
 * Typical usage:
 * 1. setting_provider_alloc() - Create provider with file path and migrations
 * 2. setting_provider_open() - Load settings from file
 * 3. setting_provider_load()/save() - Read and write individual settings
 * 4. setting_provider_close() - Flush changes to disk
 * 5. setting_provider_free() - Free memory
 */
#pragma once

#include <furi.h>

#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque settings provider structure.
 *
 * Manages persistent JSON-based settings with version migration support.
 */
typedef struct SettingProvider SettingProvider;

/**
 * @brief Setting type enumeration.
 *
 * Defines the supported data types for settings.
 */
typedef enum {
    SettingProviderSettingTypeBool, /**< Boolean setting */
    SettingProviderSettingTypeInt, /**< Integer setting */
    SettingProviderSettingTypeFloat, /**< Float setting */
    SettingProviderSettingTypeString, /**< String setting */
    SettingProviderSettingTypeCustom, /**< Custom type with user-defined serialization */

    SettingProviderSettingTypesCount
} SettingProviderSettingType;

/**
 * @brief Boolean setting interface.
 */
typedef struct {
    bool default_value; /**< Default value used if setting is missing or invalid */
} SettingProviderBoolInterface;

/**
 * @brief Integer setting interface.
 */
typedef struct {
    int default_value; /**< Default value used if setting is missing or invalid */
    bool (*is_valid_callback)(
        int value); /**< Optional validation callback. Returns true if value is valid. */
} SettingProviderIntInterface;

/**
 * @brief Float setting interface.
 */
typedef struct {
    float default_value; /**< Default value used if setting is missing or invalid */
    bool (*is_valid_callback)(
        float value); /**< Optional validation callback. Returns true if value is valid. */
} SettingProviderFloatInterface;

/**
 * @brief String setting interface.
 */
typedef struct {
    const char* default_value; /**< Default value used if setting is missing or invalid */
    bool (*is_valid_callback)(
        const FuriString*
            value); /**< Optional validation callback. Returns true if value is valid. */
} SettingProviderStringInterface;

/**
 * @brief Custom setting interface.
 *
 * Allows implementing settings with custom types by providing serialization/deserialization callbacks.
 */
typedef struct {
    const void* default_value; /**< Default value used if setting is missing or invalid */
    bool (*is_valid_callback)(
        const void* value); /**< Optional validation callback. Returns true if value is valid. */
    void (*deep_copy_callback)(
        void* target,
        const void* source); /**< Required. Copies value from source to target. */
    void (*serialize_callback)(
        cJSON* json,
        const void* value); /**< Required. Serializes value to cJSON object. */
    void (*deserialize_callback)(
        void* value,
        const cJSON* json); /**< Required. Deserializes value from cJSON object. */
} SettingProviderCustomInterface;

/**
 * @brief Setting descriptor.
 *
 * Describes a single setting with its name, type, and type-specific interface.
 */
typedef struct {
    const char* name; /**< Setting key name in JSON */
    const void*
        interface; /**< Pointer to type-specific interface (e.g., SettingProviderIntInterface*) */
    const void* context; /**< Optional user context pointer */
    SettingProviderSettingType type; /**< Setting data type */
} SettingProviderSetting;

/**
 * @brief Migration callback function.
 *
 * @param provider Settings provider instance
 * @return true if migration succeeded, false otherwise
 */
typedef bool (*SettingProviderMigrationCallback)(SettingProvider* provider);

/**
 * @brief Settings migration descriptor.
 *
 * Describes a migration step from one version to the next.
 * Migrations are applied sequentially when opening a settings file with older version.
 */
typedef struct {
    int target_version; /**< Target version (must be source_version + 1) */
    SettingProviderMigrationCallback callback; /**< Migration function */
} SettingProviderMigration;

/**
 * @brief Allocate a settings provider instance.
 *
 * @param file_path Path to the settings JSON file
 * @param target_version Current/target settings version
 * @param migrations Array of migration descriptors (can be NULL if migrations_count is 0)
 * @param migrations_count Number of migrations in the array
 * @return Allocated provider instance (must be freed with setting_provider_free)
 */
SettingProvider* setting_provider_alloc(
    const char* file_path,
    int settings_version,
    const SettingProviderMigration* migrations,
    size_t migrations_count);

/**
 * @brief Free a settings provider instance.
 *
 * IMPORTANT: Provider must be closed before freeing (see setting_provider_close).
 * This function only deallocates memory and does not perform any I/O.
 *
 * @param provider Settings provider instance
 */
void setting_provider_free(SettingProvider* provider);

/**
 * @brief Open settings file and load configuration.
 *
 * Opens the settings file, parses JSON, and applies any necessary migrations.
 * If file doesn't exist or is empty, creates a new empty settings object.
 * If file is corrupted, creates a new empty settings object and marks it for writing.
 *
 * @param provider Settings provider instance
 * @return true if file was opened successfully, false on error
 */
void setting_provider_open(SettingProvider* provider);

/**
 * @brief Close settings file and flush pending changes.
 *
 * Writes any pending changes to disk and cleans up JSON objects.
 * After closing, the provider can be reopened with setting_provider_open or freed.
 *
 * @param provider Settings provider instance
 * @return true if changes were written successfully (or no changes pending), false on I/O error
 */
bool setting_provider_close(SettingProvider* provider);

/**
 * @brief Load a setting value.
 *
 * Reads setting from JSON. If setting is missing or invalid, uses default value
 * from the interface and writes it back to JSON.
 *
 * @param provider Settings provider instance (must be open)
 * @param setting Setting descriptor
 * @param value Pointer to store the loaded value (type must match setting->type)
 */
void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value);

/**
 * @brief Save a setting value.
 *
 * Validates (if validation callback provided) and writes setting to JSON.
 * Changes are not written to disk until setting_provider_close is called.
 *
 * @param provider Settings provider instance (must be open)
 * @param setting Setting descriptor
 * @param value Pointer to the value to save (type must match setting->type)
 */
bool setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value);

/**
 * @brief Delete a setting from the configuration.
 *
 * Removes the setting from JSON. Changes are not written to disk until
 * setting_provider_close is called.
 *
 * @param provider Settings provider instance (must be open)
 * @param setting Setting descriptor
 */
void setting_provider_drop(SettingProvider* provider, const SettingProviderSetting* setting);

#ifdef __cplusplus
}
#endif
