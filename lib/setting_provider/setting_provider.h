/**
 * @file setting_provider.h
 *
 * @brief Type-safe settings storage with JSON persistence and versioned migrations.
 */

#pragma once

#include <furi.h>
#include <cjson/cJSON.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque settings provider handle. */
typedef struct SettingProvider SettingProvider;

/** Supported setting data types. */
typedef enum {
    SettingProviderSettingTypeBool, /**< Boolean value (JSON true/false) */
    SettingProviderSettingTypeInt, /**< Signed integer value */
    SettingProviderSettingTypeFloat, /**< Floating-point value */
    SettingProviderSettingTypeString, /**< Null-terminated C string */
    SettingProviderSettingTypeCustom, /**< User-defined with serialize/deserialize callbacks */
    SettingProviderSettingTypeEnum, /**< Enum mapped to string names */
    SettingProviderSettingTypeUnion, /**< Tagged union with discriminator */
    SettingProviderSettingTypeStruct, /**< Nested structure with inner settings */
    SettingProviderSettingTypeRaw, /**< Raw data with direct cJSON serialization */

    SettingProviderSettingTypesCount /**< Sentinel value, do not use */
} SettingProviderSettingType;

/**
 * @brief Setting descriptor.
 *
 * Defines a single setting's metadata. The interface pointer must match the type
 * (e.g., SettingProviderIntInterface for SettingProviderSettingTypeInt).
 */
typedef struct {
    const char* name; /**< Unique key in JSON storage, or NULL for flat struct/union type */
    const void* interface; /**< Type-specific interface (cast to appropriate XxInterface) */
    const void* context; /**< User context, or NULL */
    size_t field_offset; /**< Offset in parent struct (use offsetof), 0 for top-level */
    SettingProviderSettingType type; /**< Data type selector */
} SettingProviderSetting;

/** @name Type Interfaces
 *
 * Each interface defines type-specific behavior for a setting type.
 * Set unused callbacks to NULL where applicable.
 *
 * @{
 */

/** Boolean interface - stores as JSON true/false. */
typedef struct {
    bool default_value; /**< Default value when setting is not present in storage */
} SettingProviderBoolInterface;

/** Integer interface with optional validation. */
typedef struct {
    bool (*is_valid_callback)(
        const SettingProviderSetting* setting,
        int value); /**< Returns true if value is valid, NULL skips validation */
    int default_value; /**< Default value when setting is not present in storage */
} SettingProviderIntInterface;

/** Float interface with optional validation. */
typedef struct {
    bool (*is_valid_callback)(
        const SettingProviderSetting* setting,
        float value); /**< Returns true if value is valid, NULL skips validation */
    float default_value; /**< Default value when setting is not present in storage */
} SettingProviderFloatInterface;

/** String interface with optional validation. */
typedef struct {
    bool (*is_valid_callback)(
        const SettingProviderSetting* setting,
        const char* value); /**< Returns true if value is valid, NULL skips validation */
    const char* default_value; /**< Default string value when setting is not present in storage */
    size_t max_size; /**< Buffer size for loading (including null terminator) */
} SettingProviderStringInterface;

/**
 * @brief Custom type interface with string serialization.
 *
 * Allows defining arbitrary types with custom serialize/deserialize logic
 * using FuriString as the intermediate format.
 */
typedef struct {
    bool (*is_valid_callback)(
        const SettingProviderSetting* setting,
        const void* value); /**< Returns true if value is valid, NULL skips validation */
    bool (*serialize_callback)(
        const SettingProviderSetting* setting,
        const void* value,
        FuriString* string); /**< Convert value to string representation */
    bool (*deserialize_callback)(
        const SettingProviderSetting* setting,
        const char* string,
        void* value); /**< Parse string to value */
    const void* default_value; /**< Pointer to default value */
    size_t default_value_size; /**< Size of default value in bytes */
} SettingProviderCustomInterface;

/**
 * @brief Enum interface - stores integer as string from map.
 *
 * Serializes enum values as human-readable strings instead of integers.
 */
typedef struct {
    const char* const* string_map; /**< Array of string names, indexed by enum value */
    int string_map_length; /**< Number of entries in string_map */
    size_t type_size; /**< Enum's type size in bytes (use sizeof(EnumType)) */
    const void* default_value; /**< Pointer to default enum value */
} SettingProviderEnumInterface;

/**
 * @brief Union interface - stores tagged union with discriminator.
 *
 * @warning The tag_setting must use SettingProviderSettingTypeEnum type.
 */
typedef struct {
    const SettingProviderSetting* tag_setting; /**< Discriminator setting (enum type only) */
    const SettingProviderSetting* inner_settings; /**< Array of possible variant settings */
} SettingProviderUnionInterface;

/** Struct interface - stores nested object with multiple fields. */
typedef struct {
    bool (*is_valid_callback)(
        const SettingProviderSetting* setting,
        const void* value); /**< Returns true if struct is valid, NULL skips validation */
    const SettingProviderSetting* inner_settings; /**< Array of member settings */
    size_t inner_settings_count; /**< Number of member settings in inner_settings */
} SettingProviderStructInterface;

/** Raw interface with direct cJSON serialization. */
typedef struct {
    bool (*is_valid_callback)(
        const SettingProviderSetting* setting,
        const void* value); /**< Returns true if value is valid, NULL skips validation */
    bool (*serialize_callback)(
        const SettingProviderSetting* setting,
        const void* value,
        cJSON* json_node); /**< Convert value directly to cJSON node */
    bool (*deserialize_callback)(
        const SettingProviderSetting* setting,
        const cJSON* json_node,
        void* value); /**< Parse cJSON node to value */
    const void* default_value; /**< Pointer to default value */
    size_t default_value_size; /**< Size of default value in bytes */
} SettingProviderRawInterface;

/** @} */

/**
 * @brief Migration structure for version upgrades.
 */
typedef struct {
    int target_version; /**< Version this migration upgrades to */
    bool (*migrate_callback)(SettingProvider* instance); /**< Modifies in-memory JSON to upgrade */
} SettingProviderMigration;

/**
 * @brief Allocate a settings provider.
 *
 * @param[in] file_path         Path to JSON settings file
 * @param[in] settings_version  Current version (must be > 0, used for migrations)
 * @param[in] migrations        Array of migration callbacks, or NULL if no migrations needed
 * @param[in] migrations_count  Number of entries in migrations array
 *
 * @return    Allocated setting provider instance
 */
SettingProvider* setting_provider_alloc(
    const char* file_path,
    int settings_version,
    const SettingProviderMigration* migrations,
    size_t migrations_count);

/**
 * @brief Free settings setting provider instance.
 *
 * @param[in] instance Instance to be freed
 */
void setting_provider_free(SettingProvider* instance);

/**
 * @brief Save a setting value.
 *
 * Validates the value before saving.
 *
 * @param[in] instance  Setting provider instance
 * @param[in] setting   Setting descriptor
 * @param[in] value     Pointer to value (type must match setting->type)
 *
 * @return    true on success, false if validation failed or write error
 */
bool setting_provider_save(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    const void* value);

/**
 * @brief Load a setting value.
 *
 * Loads the value from JSON storage. If the setting is not present in storage,
 * the default value from the interface is used and saved.
 *
 * @param[in]  instance  Setting provider instance
 * @param[in]  setting   Setting descriptor
 * @param[out] value     Output buffer (must match expected type/size)
 *
 * @return    true on success, false on parse error
 */
bool setting_provider_load(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    void* value);

/**
 * @brief Reset setting to default value.
 *
 * Writes the default value to storage and optionally returns it in the
 * output buffer.
 *
 * @param[in]  instance  Setting provider instance
 * @param[in]  setting   Setting descriptor
 * @param[out] value     Output buffer for default value, or NULL if not needed
 *
 * @return    true on success, false on write error
 */
bool setting_provider_reset(
    SettingProvider* instance,
    const SettingProviderSetting* setting,
    void* value);

/**
 * @brief Validate a value against a setting's constraints.
 *
 * @param[in] setting  Setting descriptor
 * @param[in] value    Pointer to value to validate
 *
 * @return    true if value is valid or no validation is defined
 */
bool setting_provider_validate(const SettingProviderSetting* setting, const void* value);

#ifdef __cplusplus
}
#endif
