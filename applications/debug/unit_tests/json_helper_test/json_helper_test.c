/**
 * @file test_test.c
 * @brief Tests the tests. These tests tests should pass the tests.
 */

#include "../unit_tests.h"
#include <toolbox/json_helper.h>

#define JSON_UNIT_TESTS_NORMAL_PATH UNIT_TESTS_PATH("unit_test.json")
#define JSON_UNIT_TESTS_SINGLE_PATH UNIT_TESTS_PATH("unit_test_single.json")

static JsonConfig* json_config;

static void json_config_setup(void) {
    json_config = json_config_alloc();
}

static void json_config_teardown(void) {
    furi_check(json_config_free(json_config) == JsonConfigStatusOk);
    json_config = NULL;
}

MU_TEST(json_helper_test_read_default) {
    mu_assert_int_eq(
        JsonConfigStatusMissing, json_config_open(json_config, JSON_UNIT_TESTS_NORMAL_PATH));

    {
        int val = 0;
        int default_val = 42;
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_int(json_config, "test_default_int_key", &val, &default_val));
        mu_assert_int_eq(default_val, val);
    }

    {
        float val = 0.0f;
        float default_val = 3.14f;
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_number(json_config, "test_default_float_key", &val, &default_val));
        mu_assert_double_eq(default_val, val);
    }

    {
        bool val = false;
        bool default_val = true;
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_bool(json_config, "test_default_bool_key", &val, &default_val));
        mu_assert_int_eq(default_val, val);
    }

    {
        FuriString* val = furi_string_alloc();
        const char* default_val = "test_string_default_value";
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_str(json_config, "test_default_str_key", val, default_val));
        mu_assert_string_eq(default_val, furi_string_get_cstr(val));
        furi_string_free(val);
    }
}

MU_TEST(json_helper_test_write) {
    mu_assert_int_eq(
        JsonConfigStatusOk, json_config_open(json_config, JSON_UNIT_TESTS_NORMAL_PATH));
    mu_assert_int_eq(JsonConfigStatusOk, json_config_write_int(json_config, "test_int_key", 69));
    mu_assert_int_eq(
        JsonConfigStatusOk, json_config_write_number(json_config, "test_float_key", 22.8f));
    mu_assert_int_eq(
        JsonConfigStatusOk, json_config_write_bool(json_config, "test_bool_key", true));
    mu_assert_int_eq(
        JsonConfigStatusOk,
        json_config_write_str(json_config, "test_str_key", "test_string_value"));
}

MU_TEST(json_helper_test_read) {
    mu_assert_int_eq(
        JsonConfigStatusOk, json_config_open(json_config, JSON_UNIT_TESTS_NORMAL_PATH));

    // check that default values are written
    {
        int val = 0;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_int(json_config, "test_default_int_key", &val, NULL));
        mu_assert_int_eq(42, val);
    }

    {
        float val = 0.0f;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_number(json_config, "test_default_float_key", &val, NULL));
        mu_assert_double_eq(3.14f, val);
    }

    {
        bool val = false;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_bool(json_config, "test_default_bool_key", &val, NULL));
        mu_assert_int_eq(true, val);
    }

    {
        FuriString* val = furi_string_alloc();
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_str(json_config, "test_default_str_key", val, NULL));
        mu_assert_string_eq("test_string_default_value", furi_string_get_cstr(val));
        furi_string_free(val);
    }

    // explicitly written values
    {
        int val = 0;
        mu_assert_int_eq(
            JsonConfigStatusOk, json_config_read_int(json_config, "test_int_key", &val, NULL));
        mu_assert_int_eq(69, val);
    }

    {
        float val = 0.0f;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_number(json_config, "test_float_key", &val, NULL));
        mu_assert_double_eq(22.8f, val);
    }

    {
        bool val = false;
        mu_assert_int_eq(
            JsonConfigStatusOk, json_config_read_bool(json_config, "test_bool_key", &val, NULL));
        mu_assert_int_eq(true, val);
    }

    {
        FuriString* val = furi_string_alloc();
        mu_assert_int_eq(
            JsonConfigStatusOk, json_config_read_str(json_config, "test_str_key", val, NULL));
        mu_assert_string_eq("test_string_value", furi_string_get_cstr(val));
        furi_string_free(val);
    }
}

MU_TEST_SUITE(json_helper_test_suite_normal) {
    MU_SUITE_CONFIGURE(&json_config_setup, &json_config_teardown);
    MU_RUN_TEST(json_helper_test_read_default);
    MU_RUN_TEST(json_helper_test_write);
    MU_RUN_TEST(json_helper_test_read);
}

MU_TEST(json_helper_test_read_default_single) {
    {
        int val = 0;
        int default_val = 42;
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_single_int(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_int_key", &val, &default_val));
        mu_assert_int_eq(default_val, val);
    }

    {
        float val = 0.0f;
        float default_val = 3.14f;
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_single_number(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_float_key", &val, &default_val));
        mu_assert_double_eq(default_val, val);
    }

    {
        bool val = false;
        bool default_val = true;
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_single_bool(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_bool_key", &val, &default_val));
        mu_assert_int_eq(default_val, val);
    }

    {
        FuriString* val = furi_string_alloc();
        const char* default_val = "test_string_default_value";
        mu_assert_int_eq(
            JsonConfigStatusMissing,
            json_config_read_single_str(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_str_key", val, default_val));
        mu_assert_string_eq(default_val, furi_string_get_cstr(val));
        furi_string_free(val);
    }
}

MU_TEST(json_helper_test_write_single) {
    mu_assert_int_eq(
        JsonConfigStatusOk,
        json_config_write_single_int(JSON_UNIT_TESTS_SINGLE_PATH, "test_int_key", 69));
    mu_assert_int_eq(
        JsonConfigStatusOk,
        json_config_write_single_number(JSON_UNIT_TESTS_SINGLE_PATH, "test_float_key", 22.8f));
    mu_assert_int_eq(
        JsonConfigStatusOk,
        json_config_write_single_bool(JSON_UNIT_TESTS_SINGLE_PATH, "test_bool_key", true));
    mu_assert_int_eq(
        JsonConfigStatusOk,
        json_config_write_single_str(
            JSON_UNIT_TESTS_SINGLE_PATH, "test_str_key", "test_string_value"));
}

MU_TEST(json_helper_test_read_single) {
    // check that default values are written
    {
        int val = 0;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_int(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_int_key", &val, NULL));
        mu_assert_int_eq(42, val);
    }

    {
        float val = 0.0f;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_number(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_float_key", &val, NULL));
        mu_assert_double_eq(3.14f, val);
    }

    {
        bool val = false;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_bool(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_bool_key", &val, NULL));
        mu_assert_int_eq(true, val);
    }

    {
        FuriString* val = furi_string_alloc();
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_str(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_default_str_key", val, NULL));
        mu_assert_string_eq("test_string_default_value", furi_string_get_cstr(val));
        furi_string_free(val);
    }

    // explicitly written values
    {
        int val = 0;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_int(JSON_UNIT_TESTS_SINGLE_PATH, "test_int_key", &val, NULL));
        mu_assert_int_eq(69, val);
    }

    {
        float val = 0.0f;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_number(
                JSON_UNIT_TESTS_SINGLE_PATH, "test_float_key", &val, NULL));
        mu_assert_double_eq(22.8f, val);
    }

    {
        bool val = false;
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_bool(JSON_UNIT_TESTS_SINGLE_PATH, "test_bool_key", &val, NULL));
        mu_assert_int_eq(true, val);
    }

    {
        FuriString* val = furi_string_alloc();
        mu_assert_int_eq(
            JsonConfigStatusOk,
            json_config_read_single_str(JSON_UNIT_TESTS_SINGLE_PATH, "test_str_key", val, NULL));
        mu_assert_string_eq("test_string_value", furi_string_get_cstr(val));
        furi_string_free(val);
    }
}

MU_TEST_SUITE(json_helper_test_suite_single) {
    MU_RUN_TEST(json_helper_test_read_default_single);
    MU_RUN_TEST(json_helper_test_write_single);
    MU_RUN_TEST(json_helper_test_read_single);
}

int run_minunit_json_helper_test(void) {
    MU_RUN_SUITE(json_helper_test_suite_normal);
    MU_RUN_SUITE(json_helper_test_suite_single);
    return MU_EXIT_CODE;
}
