#include "js_url.h"

#include <toolbox/url.h>

static void url_free_cb(void* native_p, jerry_object_native_info_t* info_p) {
    UNUSED(info_p);

    Url* url_native = native_p;
    url_free(url_native);
}

static const jerry_object_native_info_t url_native_info = {.free_cb = url_free_cb};

static jerry_value_t js_url_get(const jerry_call_info_t* call_info, UrlPartId part_id) {
    Url* url_native = jerry_object_get_native_ptr(call_info->this_value, &url_native_info);
    furi_assert(url_native);
    const StringSlice* part = url_get_part(url_native, part_id);
    return jerry_string((const jerry_char_t*)part->first_char, part->length, JERRY_ENCODING_CESU8);
}

static jerry_value_t js_url_href_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdHref);
}

static jerry_value_t js_url_origin_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdOrigin);
}

static jerry_value_t js_url_protocol_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdProtocol);
}

static jerry_value_t js_url_host_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdHost);
}

static jerry_value_t js_url_hostname_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdHostname);
}

static jerry_value_t js_url_port_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdPort);
}

static jerry_value_t js_url_pathname_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdPathname);
}

static jerry_value_t js_url_search_get(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    UNUSED(args);
    UNUSED(args_count);
    return js_url_get(call_info, UrlPartIdSearch);
}

static jerry_value_t js_url_init(jerry_value_t this_value, const char* url_str) {
    Url* url_native = url_alloc();

    if(!url_parse(url_native, url_str)) {
        url_free(url_native);
        return jerry_throw_sz(JERRY_ERROR_TYPE, "Invalid URL");
    }

    jerry_object_set_native_ptr(this_value, &url_native_info, url_native);

    return jerry_undefined();
}

static jerry_value_t url_constructor(
    const jerry_call_info_t* call_info,
    const jerry_value_t args[],
    const jerry_length_t args_count) {
    JS_CHECK_CONSTRUCTOR();
    JS_CHECK_ARGS_COUNT(1);

    jerry_value_t arg_url = jerry_value_to_string(JS_ARG(0));
    if(jerry_value_is_exception(arg_url)) {
        return arg_url;
    }

    char* url_str = js_string_to_c_string(arg_url);
    furi_check(url_str);

    const jerry_value_t result = js_url_init(call_info->this_value, url_str);

    jerry_value_free(arg_url);
    free(url_str);

    return result;
}

void js_setup_url(void) {
    jerry_value_t global_obj = jerry_current_realm();

    jerry_value_t constructor = jerry_function_external(url_constructor);
    jerry_value_free(jerry_object_set_sz(global_obj, "URL", constructor));

    jerry_value_t prototype = jerry_object();
    js_set_property_getset(prototype, "href", js_url_href_get, NULL);
    js_set_property_getset(prototype, "origin", js_url_origin_get, NULL);
    js_set_property_getset(prototype, "protocol", js_url_protocol_get, NULL);
    js_set_property_getset(prototype, "host", js_url_host_get, NULL);
    js_set_property_getset(prototype, "hostname", js_url_hostname_get, NULL);
    js_set_property_getset(prototype, "port", js_url_port_get, NULL);
    js_set_property_getset(prototype, "pathname", js_url_pathname_get, NULL);
    js_set_property_getset(prototype, "search", js_url_search_get, NULL);
    js_set_method(prototype, "toString", js_url_href_get);

    js_check_and_free(jerry_object_set_sz(constructor, "prototype", prototype));

    jerry_value_free(prototype);
    jerry_value_free(constructor);
    jerry_value_free(global_obj);
}
