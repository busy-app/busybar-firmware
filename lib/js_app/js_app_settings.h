/**
 * @file js_app_settings.h
 * @brief Settings manifest parser for JavaScript applications.
 *
 * Parses & validates application's settings manifest (settings.json) 
 * into a tree of typed nodes.
 */

#pragma once

#include "setting_types/js_app_settings_color.h"
#include "setting_types/js_app_settings_geo.h"
#include "setting_types/js_app_settings_time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsAppSettingsNode JsAppSettingsNode;

/**
 * @brief Settings node type.
 *
 * Each value corresponds to a "type" key of a manifest entry.
 */
typedef enum {
    JsAppSettingsNodeTypeBool, ///< "boolean" toggle.
    JsAppSettingsNodeTypeInt, ///< "integer" input.
    JsAppSettingsNodeTypeString, ///< "string" input.
    JsAppSettingsNodeTypeEnum, ///< "enum" selector.
    JsAppSettingsNodeTypeColor, ///< "color" picker.
    JsAppSettingsNodeTypeTime, ///< "time" value.
    JsAppSettingsNodeTypeGeo, ///< "geolocation" value.

    JsAppSettingsNodeTypeGroup, ///< "group" container.

    JsAppSettingsNodeTypesCount,
} JsAppSettingsNodeType;

/**
 * @brief Boolean node data.
 */
typedef struct {
    bool default_value; ///< Default state.
} JsAppSettingsBoolData;

/**
 * @brief Integer node data.
 */
typedef struct {
    int default_value; ///< Default value, always within [min_value, max_value].
    int min_value; ///< Lower bound, INT_MIN when unspecified.
    int max_value; ///< Upper bound, INT_MAX when unspecified.
    unsigned int value_step; ///< Adjustment step, 1 when unspecified.
} JsAppSettingsIntData;

/**
 * @brief String node data.
 */
typedef struct {
    char* default_value; ///< Default text, owned by the tree.
    unsigned int min_length; ///< Minimum length in bytes, 0 when unspecified.
    unsigned int max_length; ///< Maximum length in bytes, 64 when unspecified.
    bool is_sensitive; ///< Hide the text while editing.
} JsAppSettingsStringData;

/**
 * @brief Enumeration option.
 */
typedef struct {
    char* value; ///< Machine-readable value, owned by the tree.
    char* label; ///< User-visible label, owned by the tree.
} JsAppSettingsEnumOption;

/**
 * @brief Enumeration node data.
 */
typedef struct {
    int default_index; ///< Index of the default option within options.
    JsAppSettingsEnumOption* options; ///< Option array, owned by the tree.
    size_t options_count; ///< Number of options, between 1 and 32.
} JsAppSettingsEnumData;

/**
 * @brief Color node data.
 */
typedef struct {
    Color default_value; ///< Default color.
} JsAppSettingsColorData;

/**
 * @brief Time node data.
 */
typedef struct {
    JsAppSettingsTimeValue default_value; ///< Default time.
} JsAppSettingsTimeData;

/**
 * @brief Geolocation node data.
 */
typedef struct {
    JsAppSettingsGeoValue default_value; ///< Default position.
} JsAppSettingsGeoData;

/**
 * @brief Group node data.
 */
typedef struct {
    JsAppSettingsNode* nodes; ///< Child nodes, owned by the tree.
    size_t nodes_count; ///< Number of child nodes.
} JsAppSettingsGroupData;

/**
 * @brief Settings node.
 *
 * A single field or a recursive group of fields. The type member selects the
 * type of the data payload; all strings and the payload itself are owned by
 * the tree and released by js_app_settings_free().
 */
struct JsAppSettingsNode {
    char* id; ///< Identifier, unique among siblings.
    char* label; ///< User-visible name.
    char* description; ///< Optional details, NULL when absent.

    JsAppSettingsNodeType type; ///< Node kind, selects the data payload.
    void* data; ///< Per-type payload.
};

/**
 * @brief Parsed settings tree.
 */
typedef struct {
    int version; ///< Application-defined manifest version.

    JsAppSettingsNode* nodes; ///< Root-level nodes.
    size_t nodes_count; ///< Number of root-level nodes.
} JsAppSettings;

/**
 * @brief Validate a value against a node's constraints.
 *
 * The value pointer is type-specific:
 * bool* for boolean, int* for integer, char* for string,
 * int* for enum (option index), Color* for color,
 * const JsAppSettingsTimeValue* for time,
 * const JsAppSettingsGeoValue* for geolocation.
 * Boolean and color nodes accept any value.
 *
 * @param[in] node Settings node.
 * @param[in] value Value matching the node's type.
 * @return true if the value satisfies the node's bounds, false otherwise.
 */
bool js_app_settings_validate_value(const JsAppSettingsNode* node, const void* value);

/**
 * @brief Parse a settings manifest.
 *
 * @param[in] data Manifest buffer, need not be NUL-terminated.
 * @param[in] length Manifest length in bytes.
 * @return Allocated settings tree, or NULL if the manifest is invalid;
 *         failures are logged with the offending field's path.
 */
JsAppSettings* js_app_settings_parse(const char* data, uint32_t length);

/**
 * @brief Free a settings tree.
 *
 * Releases the tree and every string and node array it owns.
 *
 * @param[in] settings Tree returned by js_app_settings_parse().
 */
void js_app_settings_free(JsAppSettings* settings);

#ifdef __cplusplus
}
#endif
