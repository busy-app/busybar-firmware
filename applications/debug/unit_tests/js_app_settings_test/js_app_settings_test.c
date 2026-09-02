#include "../unit_tests.h"
#include <js_app/js_app_settings.h>

#include <limits.h>
#include <math.h>
#include <string.h>

static JsAppSettings* js_app_settings_test_parse(const char* manifest) {
    return js_app_settings_parse(manifest, strlen(manifest));
}

MU_TEST(js_app_settings_test_root) {
    /* valid manifest */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":7,\"fields\":{"
            "\"flag\":{\"label\":\"Flag\",\"type\":\"boolean\",\"default\":true}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(7, settings->version);
        mu_assert_int_eq(1, settings->nodes_count);
        mu_assert_string_eq("flag", settings->nodes[0].id);

        js_app_settings_free(settings);
    }

    /* rejected manifests */
    static const char* const invalid[] = {
        "{\"format_version\":1,",
        "[1,2,3]",
        "{\"version\":1,\"fields\":{\"flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}}}",
        "{\"format_version\":2,\"version\":1,\"fields\":{\"flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}}}",
        "{\"format_version\":1,\"fields\":{\"flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}}}",
        "{\"format_version\":1,\"version\":1}",
        "{\"format_version\":1,\"version\":1,\"fields\":[{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}]}",
        "{\"format_version\":1,\"version\":1,\"fields\":{}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_node_id) {
    /* same id in different groups is allowed */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"grp_a\":{\"label\":\"A\",\"type\":\"group\",\"fields\":{"
            "\"flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}}},"
            "\"grp_b\":{\"label\":\"B\",\"type\":\"group\",\"fields\":{"
            "\"flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":false}}}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(2, settings->nodes_count);

        js_app_settings_free(settings);
    }

    /* duplicate ids on the same level */
    {
        mu_assert_null(js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"flag\":{\"label\":\"A\",\"type\":\"boolean\",\"default\":true},"
            "\"flag\":{\"label\":\"B\",\"type\":\"boolean\",\"default\":false}}}"));
    }

    /* rejected ids */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"Flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"1flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_node_common) {
    /* description is optional and owned by the tree */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"flag\":{\"label\":\"Flag\",\"description\":\"description value\","
            "\"type\":\"boolean\",\"default\":true},"
            "\"flag2\":{\"label\":\"Flag2\",\"type\":\"boolean\",\"default\":false}}}");

        mu_assert_not_null(settings);
        mu_assert_string_eq("description value", settings->nodes[0].description);
        mu_assert_null(settings->nodes[1].description);

        js_app_settings_free(settings);
    }

    /* rejected nodes */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"flag\":{\"type\":\"boolean\",\"default\":true}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"flag\":{\"label\":\"F\",\"type\":\"toggle\",\"default\":true}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_boolean) {
    /* default is captured */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"flag\":{\"label\":\"Flag\",\"type\":\"boolean\",\"default\":true}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(JsAppSettingsNodeTypeBool, settings->nodes[0].type);
        mu_check(((JsAppSettingsBoolData*)settings->nodes[0].data)->default_value);

        js_app_settings_free(settings);
    }

    /* rejected defaults */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"flag\":{\"label\":\"F\",\"type\":\"boolean\"}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_integer) {
    /* all keys captured */
    {
        JsAppSettings* settings =
            js_app_settings_test_parse("{\"format_version\":1,\"version\":1,\"fields\":{"
                                       "\"volume\":{\"label\":\"Volume\",\"type\":\"integer\","
                                       "\"default\":5,\"min\":0,\"max\":10,\"step\":2}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(JsAppSettingsNodeTypeInt, settings->nodes[0].type);

        JsAppSettingsIntData* data = settings->nodes[0].data;
        mu_assert_int_eq(5, data->default_value);
        mu_assert_int_eq(0, data->min_value);
        mu_assert_int_eq(10, data->max_value);
        mu_assert_int_eq(2, data->value_step);

        js_app_settings_free(settings);
    }

    /* optional keys default to an unbounded range and a unit step */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"any\":{\"label\":\"Any\",\"type\":\"integer\",\"default\":42}}}");

        mu_assert_not_null(settings);

        JsAppSettingsIntData* data = settings->nodes[0].data;
        mu_assert_int_eq(INT_MIN, data->min_value);
        mu_assert_int_eq(INT_MAX, data->max_value);
        mu_assert_int_eq(1, data->value_step);

        js_app_settings_free(settings);
    }

    /* rejected integers */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"any\":{\"label\":\"A\",\"type\":\"integer\",\"min\":0,\"max\":1}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"any\":{\"label\":\"A\",\"type\":\"integer\",\"default\":5,\"min\":10,\"max\":0}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"any\":{\"label\":\"A\",\"type\":\"integer\",\"default\":5,\"step\":0}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"any\":{\"label\":\"A\",\"type\":\"integer\",\"default\":5,\"min\":0,\"max\":10,\"step\":3}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"any\":{\"label\":\"A\",\"type\":\"integer\",\"default\":11,\"min\":0,\"max\":10}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_string) {
    /* all keys captured */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"name\":{\"label\":\"Name\",\"type\":\"string\","
            "\"default\":\"admin\",\"sensitive\":true,\"min_length\":2,\"max_length\":16}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(JsAppSettingsNodeTypeString, settings->nodes[0].type);

        JsAppSettingsStringData* data = settings->nodes[0].data;
        mu_assert_string_eq("admin", data->default_value);
        mu_check(data->is_sensitive);
        mu_assert_int_eq(2, data->min_length);
        mu_assert_int_eq(16, data->max_length);

        js_app_settings_free(settings);
    }

    /* optional keys default to insensitive and the standard length window */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"name\":{\"label\":\"Name\",\"type\":\"string\",\"default\":\"x\"}}}");

        mu_assert_not_null(settings);

        JsAppSettingsStringData* data = settings->nodes[0].data;
        mu_check(!data->is_sensitive);
        mu_assert_int_eq(0, data->min_length);
        mu_assert_int_eq(64, data->max_length);

        js_app_settings_free(settings);
    }

    /* rejected strings */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"name\":{\"label\":\"N\",\"type\":\"string\",\"default\":7}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"name\":{\"label\":\"N\",\"type\":\"string\",\"default\":\"abc\",\"min_length\":8,\"max_length\":4}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"name\":{\"label\":\"N\",\"type\":\"string\",\"default\":\"ab\",\"min_length\":3}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"name\":{\"label\":\"N\",\"type\":\"string\",\"default\":\"x\",\"max_length\":256}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_enum) {
    /* options, labels and the default index are captured */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"mode\":{\"label\":\"Mode\",\"type\":\"enum\",\"default\":\"fast\","
            "\"options\":[{\"value\":\"slow\",\"label\":\"Slow\"},"
            "{\"value\":\"fast\",\"label\":\"Fast\"},{\"value\":\"turbo\",\"label\":\"Turbo\"}]}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(JsAppSettingsNodeTypeEnum, settings->nodes[0].type);

        JsAppSettingsEnumData* data = settings->nodes[0].data;
        mu_assert_int_eq(3, data->options_count);
        mu_assert_int_eq(1, data->default_index);
        mu_assert_string_eq("slow", data->options[0].value);
        mu_assert_string_eq("Slow", data->options[0].label);
        mu_assert_string_eq("turbo", data->options[2].value);
        mu_assert_string_eq("Turbo", data->options[2].label);

        js_app_settings_free(settings);
    }

    /* rejected enums */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"mode\":{\"label\":\"M\",\"type\":\"enum\",\"options\":[{\"value\":\"a\",\"label\":\"A\"}]}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"mode\":{\"label\":\"M\",\"type\":\"enum\",\"default\":\"a\",\"options\":[]}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"mode\":{\"label\":\"M\",\"type\":\"enum\",\"default\":\"a\",\"options\":[{\"value\":\"a\"}]}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"mode\":{\"label\":\"M\",\"type\":\"enum\",\"default\":\"a\",\"options\":[{\"value\":\"a\",\"label\":\"A\"},{\"value\":\"a\",\"label\":\"Again\"}]}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"mode\":{\"label\":\"M\",\"type\":\"enum\",\"default\":\"missing\",\"options\":[{\"value\":\"a\",\"label\":\"A\"}]}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_color_codec) {
    /* both forms parse */
    {
        Color value = {0};

        mu_check(js_app_settings_color_parse("#FF8000", &value));
        mu_assert_int_eq(0xFF, value.r);
        mu_assert_int_eq(0x80, value.g);
        mu_assert_int_eq(0x00, value.b);
        mu_assert_int_eq(0xFF, value.a);

        mu_check(js_app_settings_color_parse("#FF800080", &value));
        mu_assert_int_eq(0xFF, value.r);
        mu_assert_int_eq(0x80, value.g);
        mu_assert_int_eq(0x00, value.b);
        mu_assert_int_eq(0x80, value.a);

        mu_check(js_app_settings_color_parse("#0B55AA", &value));
        mu_assert_int_eq(0x0B, value.r);
    }

    /* rejected colors */
    static const char* const invalid[] = {
        "FF8000",
        "#GG8000",
        "#0xF800",
        "#1234567",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        Color value = {0};
        mu_assert(!js_app_settings_color_parse(invalid[i], &value), invalid[i]);
    }

    /* formatting round-trips */
    {
        FuriString* string = furi_string_alloc();
        Color value = {0};

        value = (Color)COLOR_MAKE_RGBA(0x12, 0x34, 0x56, 0x78);
        js_app_settings_color_format(&value, string);
        mu_assert_string_eq("#12345678", furi_string_get_cstr(string));

        value = (Color)COLOR_MAKE_RGB(0xAB, 0xCD, 0xEF);
        js_app_settings_color_format(&value, string);
        mu_assert_string_eq("#ABCDEF", furi_string_get_cstr(string));
        mu_check(js_app_settings_color_parse(furi_string_get_cstr(string), &value));
        mu_assert_int_eq(0xAB, value.r);

        furi_string_free(string);
    }
}

MU_TEST(js_app_settings_test_time_codec) {
    /* both forms parse, has_seconds follows the form */
    {
        JsAppSettingsTimeValue value = {0};

        mu_check(js_app_settings_time_parse("09:30", &value));
        mu_assert_int_eq(9, value.time.hour);
        mu_assert_int_eq(30, value.time.minute);
        mu_assert_int_eq(0, value.time.second);
        mu_check(!value.has_seconds);

        mu_check(js_app_settings_time_parse("23:59:05", &value));
        mu_assert_int_eq(23, value.time.hour);
        mu_assert_int_eq(59, value.time.minute);
        mu_assert_int_eq(5, value.time.second);
        mu_check(value.has_seconds);
    }

    /* rejected times */
    static const char* const invalid[] = {
        "9:30",
        "1:2:3",
        "09:30x",
        "24:00",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        JsAppSettingsTimeValue value = {0};
        mu_assert(!js_app_settings_time_parse(invalid[i], &value), invalid[i]);
    }

    /* formatting round-trips */
    {
        FuriString* string = furi_string_alloc();
        JsAppSettingsTimeValue value = {0};

        mu_check(js_app_settings_time_parse("09:30", &value));
        js_app_settings_time_format(&value, string);
        mu_assert_string_eq("09:30", furi_string_get_cstr(string));

        mu_check(js_app_settings_time_parse("23:59:05", &value));
        js_app_settings_time_format(&value, string);
        mu_assert_string_eq("23:59:05", furi_string_get_cstr(string));

        furi_string_free(string);
    }
}

MU_TEST(js_app_settings_test_complex_field_defaults) {
    /* color and time defaults go through their codecs */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"accent\":{\"label\":\"A\",\"type\":\"color\",\"default\":\"red\"}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"alarm\":{\"label\":\"A\",\"type\":\"time\",\"default\":\"9:30\"}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"alarm\":{\"label\":\"A\",\"type\":\"time\"}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_geolocation) {
    /* auto mode has no coordinates */
    {
        JsAppSettings* settings =
            js_app_settings_test_parse("{\"format_version\":1,\"version\":1,\"fields\":{"
                                       "\"home\":{\"label\":\"Home\",\"type\":\"geolocation\","
                                       "\"default\":{\"mode\":\"auto\",\"name\":\"Home\"}}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(JsAppSettingsNodeTypeGeo, settings->nodes[0].type);

        JsAppSettingsGeoValue* value =
            &((JsAppSettingsGeoData*)settings->nodes[0].data)->default_value;
        mu_assert_int_eq(JsAppSettingsGeoModeAuto, value->mode);
        mu_assert_string_eq("Home", value->name);
        mu_check(isnan(value->latitude));
        mu_check(isnan(value->longitude));

        js_app_settings_free(settings);
    }

    /* fixed mode carries the coordinates */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"pin\":{\"label\":\"Pin\",\"type\":\"geolocation\","
            "\"default\":{\"mode\":\"fixed\",\"name\":\"Pin\",\"lat\":55.75,\"lon\":37.62}}}}");

        mu_assert_not_null(settings);

        JsAppSettingsGeoValue* value =
            &((JsAppSettingsGeoData*)settings->nodes[0].data)->default_value;
        mu_assert_int_eq(JsAppSettingsGeoModeFixed, value->mode);
        mu_assert_string_eq("Pin", value->name);
        mu_assert_double_eq(55.75, value->latitude);
        mu_assert_double_eq(37.62f, value->longitude);

        js_app_settings_free(settings);
    }

    /* rejected geolocations */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"pin\":{\"label\":\"P\",\"type\":\"geolocation\",\"default\":{\"mode\":\"fixed\",\"name\":\"Pin\"}}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"pin\":{\"label\":\"P\",\"type\":\"geolocation\",\"default\":{\"mode\":\"auto\",\"name\":\"Home\",\"lat\":1.0}}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"pin\":{\"label\":\"P\",\"type\":\"geolocation\",\"default\":{\"mode\":\"fixed\",\"name\":\"Pin\",\"lat\":91.0,\"lon\":0.0}}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"pin\":{\"label\":\"P\",\"type\":\"geolocation\",\"default\":{\"mode\":\"auto\",\"name\":\"\"}}}}",
        "{\"format_version\":1,\"version\":1,\"fields\":{\"pin\":{\"label\":\"P\",\"type\":\"geolocation\",\"default\":{\"mode\":\"manual\",\"name\":\"Pin\"}}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_groups) {
    /* nested groups keep their structure */
    {
        JsAppSettings* settings = js_app_settings_test_parse(
            "{\"format_version\":1,\"version\":1,\"fields\":{"
            "\"network\":{\"label\":\"Network\",\"type\":\"group\",\"fields\":{"
            "\"proxy\":{\"label\":\"Proxy\",\"type\":\"group\",\"fields\":{"
            "\"port\":{\"label\":\"Port\",\"type\":\"integer\",\"default\":8080,"
            "\"min\":1,\"max\":65535}}}}}}}");

        mu_assert_not_null(settings);
        mu_assert_int_eq(JsAppSettingsNodeTypeGroup, settings->nodes[0].type);

        JsAppSettingsGroupData* data = settings->nodes[0].data;
        mu_assert_int_eq(1, data->nodes_count);
        mu_assert_string_eq("proxy", data->nodes[0].id);

        JsAppSettingsGroupData* inner = data->nodes[0].data;
        mu_assert_int_eq(1, inner->nodes_count);
        mu_assert_string_eq("port", inner->nodes[0].id);
        mu_assert_int_eq(8080, ((JsAppSettingsIntData*)inner->nodes[0].data)->default_value);

        js_app_settings_free(settings);
    }

    /* rejected groups */
    static const char* const invalid[] = {
        "{\"format_version\":1,\"version\":1,\"fields\":{\"grp\":{\"label\":\"G\",\"type\":\"group\",\"fields\":{}}}}",
    };

    for(size_t i = 0; i < COUNT_OF(invalid); i++) {
        mu_assert(js_app_settings_test_parse(invalid[i]) == NULL, invalid[i]);
    }
}

MU_TEST(js_app_settings_test_validate_value) {
    JsAppSettings* settings = js_app_settings_test_parse(
        "{\"format_version\":1,\"version\":1,\"fields\":{"
        "\"flag\":{\"label\":\"F\",\"type\":\"boolean\",\"default\":true},"
        "\"num\":{\"label\":\"N\",\"type\":\"integer\",\"default\":5,\"min\":0,\"max\":10},"
        "\"name\":{\"label\":\"N\",\"type\":\"string\",\"default\":\"abc\",\"min_length\":2},"
        "\"mode\":{\"label\":\"M\",\"type\":\"enum\",\"default\":\"a\","
        "\"options\":[{\"value\":\"a\",\"label\":\"A\"},{\"value\":\"b\",\"label\":\"B\"}]},"
        "\"alarm\":{\"label\":\"A\",\"type\":\"time\",\"default\":\"09:30\"},"
        "\"pin\":{\"label\":\"P\",\"type\":\"geolocation\","
        "\"default\":{\"mode\":\"auto\",\"name\":\"Pin\"}}}}");

    mu_assert_not_null(settings);

    /* boolean accepts anything */
    {
        bool value = true;
        mu_check(js_app_settings_validate_value(&settings->nodes[0], &value));
        value = false;
        mu_check(js_app_settings_validate_value(&settings->nodes[0], &value));
    }

    /* integer is bounded by min and max */
    {
        int in_bounds = 7;
        int out_of_bounds = 10 + 1;
        mu_check(js_app_settings_validate_value(&settings->nodes[1], &in_bounds));
        mu_check(!js_app_settings_validate_value(&settings->nodes[1], &out_of_bounds));
    }

    /* string is bounded by its length window */
    {
        mu_check(js_app_settings_validate_value(&settings->nodes[2], "ab"));
        mu_check(!js_app_settings_validate_value(&settings->nodes[2], "a"));
    }

    /* enum is an index within the options */
    {
        int in_bounds = 1;
        int out_of_bounds = 2;
        mu_check(js_app_settings_validate_value(&settings->nodes[3], &in_bounds));
        mu_check(!js_app_settings_validate_value(&settings->nodes[3], &out_of_bounds));
    }

    /* time must be a real time of day */
    {
        JsAppSettingsTimeValue value = {0};
        mu_check(js_app_settings_time_parse("10:15", &value));
        mu_check(js_app_settings_validate_value(&settings->nodes[4], &value));

        value.time.hour = 24;
        mu_check(!js_app_settings_validate_value(&settings->nodes[4], &value));
    }

    /* geolocation must be well-formed */
    {
        JsAppSettingsGeoValue valid_fixed = {
            .mode = JsAppSettingsGeoModeFixed,
            .name = "Pin",
            .latitude = 1.0f,
            .longitude = 2.0f,
        };
        mu_check(js_app_settings_validate_value(&settings->nodes[5], &valid_fixed));

        JsAppSettingsGeoValue no_coordinates = valid_fixed;
        no_coordinates.latitude = NAN;
        no_coordinates.longitude = NAN;
        mu_check(!js_app_settings_validate_value(&settings->nodes[5], &no_coordinates));

        JsAppSettingsGeoValue out_of_range = valid_fixed;
        out_of_range.latitude = 91.0f;
        mu_check(!js_app_settings_validate_value(&settings->nodes[5], &out_of_range));

        JsAppSettingsGeoValue garbage_mode = valid_fixed;
        garbage_mode.mode = (JsAppSettingsGeoMode)42;
        garbage_mode.latitude = NAN;
        garbage_mode.longitude = NAN;
        mu_check(!js_app_settings_validate_value(&settings->nodes[5], &garbage_mode));
    }

    js_app_settings_free(settings);
}

MU_TEST(js_app_settings_test_integration) {
    JsAppSettings* settings = js_app_settings_test_parse(
        "{\"format_version\":1,\"version\":3,\"fields\":{"
        "\"flag\":{\"label\":\"Flag\",\"description\":\"description value\","
        "\"type\":\"boolean\",\"default\":false},"
        "\"volume\":{\"label\":\"Volume\",\"type\":\"integer\","
        "\"default\":7,\"min\":0,\"max\":11,\"step\":1},"
        "\"name\":{\"label\":\"Name\",\"type\":\"string\",\"default\":\"guest\"},"
        "\"mode\":{\"label\":\"Mode\",\"type\":\"enum\",\"default\":\"b\","
        "\"options\":[{\"value\":\"a\",\"label\":\"A\"},{\"value\":\"b\",\"label\":\"B\"}]},"
        "\"accent\":{\"label\":\"Accent\",\"type\":\"color\",\"default\":\"#00FF00\"},"
        "\"alarm\":{\"label\":\"Alarm\",\"type\":\"time\",\"default\":\"07:45:30\"},"
        "\"pin\":{\"label\":\"Pin\",\"type\":\"geolocation\","
        "\"default\":{\"mode\":\"fixed\",\"name\":\"Spot\",\"lat\":10.5,\"lon\":-20.25}},"
        "\"extra\":{\"label\":\"Extra\",\"type\":\"group\",\"fields\":{"
        "\"nested\":{\"label\":\"Nested\",\"type\":\"boolean\",\"default\":true}}}}}");

    mu_assert_not_null(settings);
    mu_assert_int_eq(3, settings->version);
    mu_assert_int_eq(8, settings->nodes_count);
    mu_assert_string_eq("description value", settings->nodes[0].description);
    mu_assert_int_eq(JsAppSettingsNodeTypeGroup, settings->nodes[7].type);

    js_app_settings_free(settings);
}

MU_TEST_SUITE(js_app_settings_test_suite) {
    MU_RUN_TEST(js_app_settings_test_root);
    MU_RUN_TEST(js_app_settings_test_node_id);
    MU_RUN_TEST(js_app_settings_test_node_common);
    MU_RUN_TEST(js_app_settings_test_boolean);
    MU_RUN_TEST(js_app_settings_test_integer);
    MU_RUN_TEST(js_app_settings_test_string);
    MU_RUN_TEST(js_app_settings_test_enum);
    MU_RUN_TEST(js_app_settings_test_color_codec);
    MU_RUN_TEST(js_app_settings_test_time_codec);
    MU_RUN_TEST(js_app_settings_test_complex_field_defaults);
    MU_RUN_TEST(js_app_settings_test_geolocation);
    MU_RUN_TEST(js_app_settings_test_groups);
    MU_RUN_TEST(js_app_settings_test_validate_value);
    MU_RUN_TEST(js_app_settings_test_integration);
}

int run_minunit_js_app_settings_test(void) {
    MU_RUN_SUITE(js_app_settings_test_suite);
    return MU_EXIT_CODE;
}
