/** @file js_util.h
 *
 * @brief Utility functions for working with JS values
 */
#pragma once
#include "js_runner_types.h"
#include <furi/core/string.h>
#include <toolbox/sized_buffer.h>

#define JS_CHECK_ARGS_COUNT(n)                                                             \
    do {                                                                                   \
        if(args_count < n) {                                                               \
            return jerry_throw_sz(JERRY_ERROR_TYPE, "At least " #n " arguments required"); \
        }                                                                                  \
    } while(false)
#define JS_CHECK_INSTANCE()                                                    \
    do {                                                                       \
        if(!instance) {                                                        \
            return jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid \"this\" value"); \
        }                                                                      \
    } while(false)

/** @brief Check if value is not and exception and free it
 * @param value value to check and free
 */
void js_check_and_free(jerry_value_t value);

/** @brief Set a property in a JS object
 *
 * @param object Object (not freed)
 * @param name Property name
 * @param property Property value to set (freed)
 */
void js_set_property(jerry_value_t object, const char* name, jerry_value_t property);

/** @brief Set a method in a JS object
 *
 * @param object Object (not freed)
 * @param name Method name
 * @param handler handler Method handler
 */
void js_set_method(jerry_value_t object, const char* name, jerry_external_handler_t handler);

/** @brief Set a method in a JS object using a well-known symbol as a key
 *
 * @param object Object (not freed)
 * @param key Method name
 * @param handler handler Method handler
 */
void js_set_method_sym(
    jerry_value_t object,
    jerry_well_known_symbol_t key,
    jerry_external_handler_t handler);

/** @brief Set a property in a JS object using a well-known symbol as a key
 *
 * @param object Object (not freed)
 * @param key Property name
 * @param property Property value to set (freed)
 */
void js_set_property_sym(
    jerry_value_t object,
    jerry_well_known_symbol_t key,
    jerry_value_t property);

/** @brief Set a property defined by its getter and setter in a JS object
 *
 * @param object Object (not freed)
 * @param name Property name
 * @param getter getter callback (can be NULL)
 * @param setter getter callback (can be NULL)
 */
void js_set_property_getset(
    jerry_value_t object,
    const char* name,
    jerry_external_handler_t getter,
    jerry_external_handler_t setter);

/** @brief Test if an object has a property with given name */
bool js_object_has_property(jerry_value_t object, const char* key);

/** @brief Create a return value of an Iterator's next() method.
 *
 * The object has two properties: `done` and `value`.
 *
 * @param done value for the `done` property
 * @param value contents of the `value` property (freed).
 */
jerry_value_t js_iterator_result(bool done, jerry_value_t value);

/** @brief If value is a JS string, return its UTF8 representation.
 *
 * @return a heap-allocated string or NULL if value is not a JS string.
 */
char* js_string_to_c_string(jerry_value_t value);

/** @brief If value is a JS string, return its UTF8 representation.
 *
 * @return a new string or NULL if value is not a JS string.
 */
FuriString* js_string_to_furi_string(jerry_value_t value);

/** @brief Create a JS string by decoding a UTF-8 FuriString.
 *
 * @param s source string
 * @return JS string
 */
jerry_value_t js_utf8_string(const FuriString* s);

/** @brief Convert a JS value to an integer. Usual JS rules are applied.
 *
 * @param value The value.
 * @param[out] result Result placeholder.
 * @return true if conversion was successful, false if value could not be converted to an integer.
 */
bool js_value_to_integer(jerry_value_t value, int* result);

/** @brief Copy a property (if it exists) from one object to another. */
void js_copy_property(jerry_value_t dst, jerry_value_t src, const char* key);

/** @brief Check if an object is an instance of a given constructor (JS `instanceof` operation). */
bool js_is_instance_of(jerry_value_t obj, const char* constructor_name);

/** @brief Create and reject a Promise with a message */
jerry_value_t js_rejected_promise(const char* msg);

/** @brief Create and reject a Promise with a message extracted from an exception
 *
 *  @param exception exception (freed)
 */
jerry_value_t js_rejected_promise_from_exception(jerry_value_t exception);

/** @brief Reject a Promise with a message extracted from an exception
 *
 *  @param promise promise (freed)
 *  @param exception exception (freed)
 */
void js_reject_promise_with_exception(jerry_value_t promise, jerry_value_t exception);

/** @brief Create a string out of a JS exception.
 *
 * @param exception JS exception. This value is not freed.
 * @return exception string or NULL if conversion failed.
 */
FuriString* js_get_exception_string(jerry_value_t exception);

/**
 * @brief Log exception message with ERROR severity
 */
void js_log_exception(const char* tag, const char* msg, jerry_value_t exception);

/**
 * @brief Create an external ArrayBuffer referencing ByteArray's data.
 *
 * Ownership of the ByteArray is transferred to jerryscript.
 *
 * @param array pointer to ByteArray (heap-allocated).
 * @return a JS ArrayBuffer object.
 */
jerry_value_t js_arraybuffer_from_byte_array(ByteArray_t* array);

/**
 * @brief Create an external ArrayBuffer referencing SizedBuffer's data.
 *
 * Ownership of the buffer is transferred to jerryscript.
 *
 * @param buffer data to build an ArrayBuffer from.
 * @return a JS ArrayBuffer object.
 */
jerry_value_t js_arraybuffer_from_sized_buffer(SizedBuffer buffer);
