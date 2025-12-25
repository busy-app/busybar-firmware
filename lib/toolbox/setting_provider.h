/**
 * @file setting_provider.h
 * @brief Type-safe JSON settings persistence provider with validation and migration support
 *
 * The SettingProvider provides a structured way to persist application settings to JSON
 * files with automatic validation, default value handling, and schema migrations.
 *
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SettingProvider SettingProvider;
typedef struct SettingProviderSetting SettingProviderSetting;

/**
 * @brief Setting types
 */
typedef enum {
    SettingProviderSettingTypeBool, ///< Boolean value
    SettingProviderSettingTypeInt, ///< Integer value with validation
    SettingProviderSettingTypeFloat, ///< Floating-point value with validation
    SettingProviderSettingTypeString, ///< String value with validation
    SettingProviderSettingTypeCustom, ///< Custom type with serialize/deserialize
    /* TODO: SettingProviderSettingTypeArray */
    /* TODO: SettingProviderSettingTypeRaw */

    SettingProviderSettingTypeStructure, ///< Nested structure

    SettingProviderSettingTypesCount ///< Total number of setting types
} SettingProviderSettingType;

/**
 * @brief Interface for boolean settings
 */
typedef struct {
    bool default_value; ///< Default value to use when loading fails
} SettingProviderBoolInterface;

/**
 * @brief Interface for integer settings
 */
typedef struct {
    int default_value; ///< Default value to use when loading fails or validation fails
    /**
     * @brief Optional validation callback
     * @param setting The setting descriptor
     * @param value The value to validate
     * @return true if valid, false otherwise
     */
    bool (*is_valid_callback)(const SettingProviderSetting* setting, int value);
} SettingProviderIntInterface;

/**
 * @brief Interface for floating-point settings
 */
typedef struct {
    float default_value; ///< Default value to use when loading fails or validation fails
    /**
     * @brief Optional validation callback
     * @param setting The setting descriptor
     * @param value The value to validate
     * @return true if valid, false otherwise
     */
    bool (*is_valid_callback)(const SettingProviderSetting* setting, float value);
} SettingProviderFloatInterface;

/**
 * @brief Interface for string settings
 */
typedef struct {
    const char* default_value; ///< Default value to use when loading fails or validation fails
    /**
     * @brief Optional validation callback
     * @param setting The setting descriptor
     * @param value The value to validate
     * @return true if valid, false otherwise
     */
    bool (*is_valid_callback)(const SettingProviderSetting* setting, const FuriString* value);
} SettingProviderStringInterface;

/**
 * @brief Interface for custom type settings
 *
 * Custom types require serialize/deserialize callbacks to convert to/from string representation.
 */
typedef struct {
    const void* default_value; ///< Pointer to default value buffer
    /**
     * @brief Serialize callback - convert value to string
     * @param setting The setting descriptor
     * @param string Output string buffer
     * @param value Pointer to value to serialize
     * @return true if successful, false otherwise
     */
    bool (*serialize_callback)(
        const SettingProviderSetting* setting,
        FuriString* string,
        const void* value);
    /**
     * @brief Deserialize callback - convert string to value
     * @param setting The setting descriptor
     * @param value Output value buffer
     * @param string Input string to deserialize
     * @return true if successful, false otherwise
     */
    bool (*deserialize_callback)(
        const SettingProviderSetting* setting,
        void* value,
        const FuriString* string);
    size_t value_size; ///< Size of the value type in bytes
} SettingProviderCustomInterface;

/**
 * @brief Interface for nested structure settings
 *
 * Structures contain other settings, allowing hierarchical configuration.
 */
typedef struct {
    /**
     * @brief Array of inner setting descriptors
     * @note These descriptors should use NULL as the name to indicate they are relative to parent
     */
    const SettingProviderSetting* inner_settings;
    size_t inner_settings_count; ///< Number of inner settings
    /**
     * @brief Optional validation callback for the entire structure
     * @param setting The setting descriptor
     * @param value Pointer to structure value
     * @return true if valid, false otherwise
     */
    bool (*is_valid_callback)(const SettingProviderSetting* setting, const void* value);
} SettingProviderStructureInterface;

/**
 * @brief Descriptor for a single setting
 *
 * Defines the type, location, validation rules, and default values for a setting.
 */
struct SettingProviderSetting {
    const char* name; ///< JSON key name (NULL for root structure)
    const void* interface; ///< Type-specific interface (e.g., SettingProviderIntInterface*)
    const void* context; ///< Optional user context passed to callbacks
    size_t field_offset; ///< Offset in bytes from containing structure start
    SettingProviderSettingType type; ///< Setting type
};

/**
 * @brief Migration step descriptor
 *
 * Defines a migration from one schema version to another.
 */
typedef struct {
    int target_version; ///< Version this migration produces
    /**
     * @brief Migration callback
     * @param provider The setting provider
     * @return true if successful, false otherwise
     *
     * The callback should modify the JSON structure directly through
     * provider->json_root and provider->json_values.
     */
    bool (*callback)(SettingProvider* provider);
} SettingProviderMigration;

/**
 * @brief Allocate a new setting provider
 *
 * @param file_path Path to the JSON settings file (will be created if doesn't exist)
 * @param settings_version Current schema version for this application
 * @param migrations Array of migration steps (can be NULL if migrations_count is 0)
 * @param migrations_count Number of migration steps
 * @return Allocated provider instance
 *
 * @note The provider must be opened with setting_provider_open() before use
 * @note Free with setting_provider_free() when done
 */
SettingProvider* setting_provider_alloc(
    const char* file_path,
    int settings_version,
    const SettingProviderMigration* migrations,
    size_t migrations_count);

/**
 * @brief Free a setting provider
 *
 * @param provider Provider to free (can be closed or unclosed)
 */
void setting_provider_free(SettingProvider* provider);

/**
 * @brief Open and load settings from file
 *
 * Loads the JSON file, applies migrations if needed, and resets to defaults
 * if the file is corrupt or missing.
 *
 * @param provider Provider to open (must be allocated but not yet opened)
 *
 * @note Must be called before any load/save/reset operations
 */
void setting_provider_open(SettingProvider* provider);

/**
 * @brief Close and save settings to file
 *
 * Writes the current state to disk if there are pending changes.
 *
 * @param provider Provider to close (must be opened)
 * @return true if save was successful, false on error
 */
bool setting_provider_close(SettingProvider* provider);

/**
 * @brief Reset a setting to its default value
 *
 * Resets the setting to its default value and marks for writing.
 *
 * @param provider Open provider instance
 * @param setting Setting descriptor
 * @param value Pointer to value buffer, nullable
 */
void setting_provider_reset(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value);

/**
 * @brief Load a setting from the stored JSON
 *
 * Loads the value from JSON, applying validation and defaults as needed.
 * If the value is missing or invalid, the default will be used.
 *
 * @param provider Open provider instance
 * @param setting Setting descriptor
 * @param value Pointer to value buffer (will be updated with loaded value)
 */
void setting_provider_load(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    void* value);

/**
 * @brief Save a setting to the JSON structure
 *
 * Validates and writes the value to the JSON structure.
 * The file is not written until setting_provider_close() is called.
 *
 * @param provider Open provider instance
 * @param setting Setting descriptor
 * @param value Pointer to value to save
 * @return true if value was valid and saved, false if validation failed
 */
bool setting_provider_save(
    SettingProvider* provider,
    const SettingProviderSetting* setting,
    const void* value);

/**
 * @brief Delete a setting from the JSON structure
 *
 * @param provider Open provider instance
 * @param setting Setting descriptor to delete (NULL to delete all settings)
 *
 * @note When setting is NULL, clears the entire "values" object
 */
void setting_provider_drop(SettingProvider* provider, const SettingProviderSetting* setting);

#ifdef __cplusplus
}
#endif
