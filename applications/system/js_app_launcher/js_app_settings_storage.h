/**
 * @file js_app_settings_storage.h
 * @brief Settings storage bridge for JavaScript applications.
 *
 * Composes a parsed settings schema into a setting provider descriptor tree
 * with a single POD value block, backed by an application data file.
 */

#pragma once

#include <js_app/js_app_settings.h>
#include <setting_provider.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JsAppSettingsStorage JsAppSettingsStorage;

/**
 * @brief Build a settings storage for an application.
 *
 * Parses the application's settings schema, composes the provider descriptor
 * tree and allocates the value block. Does not load any values; call
 * js_app_settings_storage_load() before reading any.
 *
 * @param[in] app_id Application identifier.
 * @return Allocated storage, or NULL when the application has no settings
 *         schema; failures are logged.
 */
JsAppSettingsStorage* js_app_settings_storage_alloc(const char* app_id);

/**
 * @brief Release a settings storage.
 *
 * Releases the storage with the schema tree, the descriptor tree and the
 * value block.
 *
 * @param[in] instance Storage returned by js_app_settings_storage_alloc().
 */
void js_app_settings_storage_free(JsAppSettingsStorage* instance);

/**
 * @brief Load values from the application's data file.
 *
 * Missing and invalid fields fall back to the schema defaults; an absent or
 * unparsable file still loads every default.
 *
 * @param[in] instance Storage.
 * @return true when the values are usable, false only on storage errors.
 */
bool js_app_settings_storage_load(JsAppSettingsStorage* instance);

/**
 * @brief Write every value to the application's data file.
 *
 * The file is rebuilt as a whole; a single invalid field discards the save.
 *
 * @param[in] instance Storage.
 * @return true on success, false on a write or validation failure.
 */
bool js_app_settings_storage_save(JsAppSettingsStorage* instance);

/**
 * @brief Get the root setting of the descriptor tree.
 *
 * @param[in] instance Storage.
 * @return Anonymous struct setting covering the whole value block.
 */
const SettingProviderSetting* js_app_settings_storage_get_root(JsAppSettingsStorage* instance);

/**
 * @brief Get the schema node a setting was built from.
 *
 * @param[in] instance Setting from the descriptor tree, not the root.
 * @return Node that produced the setting.
 */
const JsAppSettingsNode* js_app_settings_storage_get_node(const SettingProviderSetting* instance);

/**
 * @brief Get the value behind a setting.
 *
 * @param[in] instance Setting from the descriptor tree, not the root.
 * @return Pointer into the storage's value block, valid until free.
 */
void* js_app_settings_storage_get_value(const SettingProviderSetting* instance);

#ifdef __cplusplus
}
#endif
