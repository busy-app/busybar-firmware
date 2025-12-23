/**
 * @file setting_provider_test.c
 * @brief Unit tests for setting_provider
 */

#include "../unit_tests.h"

#include <toolbox/setting_provider.h>

#define SETTING_PROVIDER_TEST_PATH UNIT_TESTS_PATH("setting_provider_test.json")

static SettingProvider* provider;

/* custom type for testing */
typedef struct {
    int x;
    int y;
} TestPoint;

/* custom type callbacks */
static void test_point_deep_copy(void* target, const void* source) {
    TestPoint* dst = target;
    const TestPoint* src = source;

    dst->x = src->x;
    dst->y = src->y;
}

static void test_point_serialize(cJSON* json, const void* value) {
    const TestPoint* point = (const TestPoint*)value;

    cJSON_AddNumberToObject(json, "x", point->x);
    cJSON_AddNumberToObject(json, "y", point->y);
}

static void test_point_deserialize(void* value, const cJSON* json) {
    TestPoint* point = value;

    cJSON* x_item = cJSON_GetObjectItem(json, "x");
    cJSON* y_item = cJSON_GetObjectItem(json, "y");

    point->x = x_item ? x_item->valueint : 0;
    point->y = y_item ? y_item->valueint : 0;
}

static bool test_point_is_valid(const void* value) {
    const TestPoint* point = value;

    return (point->x >= 0 && point->x <= 100 && point->y >= 0 && point->y <= 100);
}

/* validation callbacks */
static bool test_int_is_valid(int value) {
    return (value >= 0 && value <= 1000);
}

static bool test_float_is_valid(float value) {
    return (value >= 0.0f && value <= 100.0f);
}

static bool test_string_is_valid(const FuriString* value) {
    return (furi_string_size(value) <= 20);
}

/* migration callback */
static bool test_migration_v1_to_v2(SettingProvider* prov) {
    UNUSED(prov);
    /* simple migration - would normally modify settings */
    return true;
}

static const SettingProviderMigration test_migrations[] = {
    {
        .target_version = 2,
        .callback = test_migration_v1_to_v2,
    },
};

static void setting_provider_setup(void) {
    provider = setting_provider_alloc(
        SETTING_PROVIDER_TEST_PATH, 2, test_migrations, COUNT_OF(test_migrations));
}

static void setting_provider_teardown(void) {
    mu_assert(setting_provider_close(provider), "Failed to close provider");

    setting_provider_free(provider);
    provider = NULL;
}

/* test basic bool operations */
MU_TEST(setting_provider_test_bool) {
    setting_provider_open(provider);

    {
        SettingProviderBoolInterface interface = {.default_value = false};
        SettingProviderSetting setting = {
            .name = "test_bool_false",
            .interface = &interface,
            .type = SettingProviderSettingTypeBool,
        };

        bool value = true;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(false, value);
    }

    {
        SettingProviderBoolInterface interface = {.default_value = true};
        SettingProviderSetting setting = {
            .name = "test_bool_true",
            .interface = &interface,
            .type = SettingProviderSettingTypeBool,
        };

        bool value = false;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(true, value);
    }

    {
        SettingProviderBoolInterface interface = {.default_value = false};
        SettingProviderSetting setting = {
            .name = "test_bool_saved",
            .interface = &interface,
            .type = SettingProviderSettingTypeBool,
        };

        bool value = true;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save bool");

        value = false;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(true, value);
    }
}

/* test basic int operations */
MU_TEST(setting_provider_test_int) {
    setting_provider_open(provider);

    {
        SettingProviderIntInterface interface = {
            .default_value = 0,
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_int_zero",
            .interface = &interface,
            .type = SettingProviderSettingTypeInt,
        };

        int value = 999;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(0, value);
    }

    {
        SettingProviderIntInterface interface = {
            .default_value = 42,
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_int_default",
            .interface = &interface,
            .type = SettingProviderSettingTypeInt,
        };

        int value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(42, value);
    }

    {
        SettingProviderIntInterface interface = {
            .default_value = 0,
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_int_saved",
            .interface = &interface,
            .type = SettingProviderSettingTypeInt,
        };

        int value = 123;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save int");

        value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(123, value);
    }
}

/* test int validation */
MU_TEST(setting_provider_test_int_validation) {
    setting_provider_open(provider);

    SettingProviderIntInterface interface = {
        .default_value = 50,
        .is_valid_callback = test_int_is_valid,
    };
    SettingProviderSetting setting = {
        .name = "test_int_validated",
        .interface = &interface,
        .type = SettingProviderSettingTypeInt,
    };

    {
        int value = 100;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save valid int");

        value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(100, value);
    }

    {
        int invalid_value = 2000;
        mu_assert(
            !setting_provider_save(provider, &setting, &invalid_value),
            "Should reject invalid int");

        int value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(100, value);
    }
}

/* test basic float operations */
MU_TEST(setting_provider_test_float) {
    setting_provider_open(provider);

    {
        SettingProviderFloatInterface interface = {
            .default_value = 0.0f,
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_float_zero",
            .interface = &interface,
            .type = SettingProviderSettingTypeFloat,
        };

        float value = 999.9f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(0.0f, value);
    }

    {
        SettingProviderFloatInterface interface = {
            .default_value = 3.14f,
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_float_default",
            .interface = &interface,
            .type = SettingProviderSettingTypeFloat,
        };

        float value = 0.0f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(3.14f, value);
    }

    {
        SettingProviderFloatInterface interface = {
            .default_value = 0.0f,
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_float_saved",
            .interface = &interface,
            .type = SettingProviderSettingTypeFloat,
        };

        float value = 12.34f;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save float");

        value = 0.0f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(12.34f, value);
    }
}

/* test float validation */
MU_TEST(setting_provider_test_float_validation) {
    setting_provider_open(provider);

    SettingProviderFloatInterface interface = {
        .default_value = 50.0f,
        .is_valid_callback = test_float_is_valid,
    };
    SettingProviderSetting setting = {
        .name = "test_float_validated",
        .interface = &interface,
        .type = SettingProviderSettingTypeFloat,
    };

    {
        float value = 25.5f;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save valid float");

        value = 0.0f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(25.5f, value);
    }

    {
        float invalid_value = 200.0f;
        mu_assert(
            !setting_provider_save(provider, &setting, &invalid_value),
            "Should reject invalid float");

        float value = 0.0f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(25.5f, value);
    }
}

/* test string operations */
MU_TEST(setting_provider_test_string) {
    setting_provider_open(provider);

    {
        SettingProviderStringInterface interface = {
            .default_value = "default_string",
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_string_default",
            .interface = &interface,
            .type = SettingProviderSettingTypeString,
        };

        FuriString* value = furi_string_alloc();
        setting_provider_load(provider, &setting, value);
        mu_assert_string_eq("default_string", furi_string_get_cstr(value));
        furi_string_free(value);
    }

    {
        SettingProviderStringInterface interface = {
            .default_value = "default",
            .is_valid_callback = NULL,
        };
        SettingProviderSetting setting = {
            .name = "test_string_saved",
            .interface = &interface,
            .type = SettingProviderSettingTypeString,
        };

        FuriString* value = furi_string_alloc_set("test_value");
        mu_assert(setting_provider_save(provider, &setting, value), "Failed to save string");

        furi_string_reset(value);
        setting_provider_load(provider, &setting, value);
        mu_assert_string_eq("test_value", furi_string_get_cstr(value));
        furi_string_free(value);
    }
}

/* test string validation */
MU_TEST(setting_provider_test_string_validation) {
    setting_provider_open(provider);

    SettingProviderStringInterface interface = {
        .default_value = "default",
        .is_valid_callback = test_string_is_valid,
    };
    SettingProviderSetting setting = {
        .name = "test_string_validated",
        .interface = &interface,
        .type = SettingProviderSettingTypeString,
    };

    {
        FuriString* value = furi_string_alloc_set("valid");
        mu_assert(setting_provider_save(provider, &setting, value), "Failed to save valid string");
        furi_string_free(value);

        value = furi_string_alloc();
        setting_provider_load(provider, &setting, value);
        mu_assert_string_eq("valid", furi_string_get_cstr(value));
        furi_string_free(value);
    }

    {
        FuriString* invalid_value = furi_string_alloc_set("this_string_is_way_too_long");
        mu_assert(
            !setting_provider_save(provider, &setting, invalid_value),
            "Should reject invalid string");
        furi_string_free(invalid_value);

        FuriString* value = furi_string_alloc();
        setting_provider_load(provider, &setting, value);
        mu_assert_string_eq("valid", furi_string_get_cstr(value));
        furi_string_free(value);
    }
}

/* test custom type */
MU_TEST(setting_provider_test_custom) {
    setting_provider_open(provider);

    TestPoint default_point = {.x = 10, .y = 20};

    SettingProviderCustomInterface interface = {
        .default_value = &default_point,
        .is_valid_callback = test_point_is_valid,
        .deep_copy_callback = test_point_deep_copy,
        .serialize_callback = test_point_serialize,
        .deserialize_callback = test_point_deserialize,
    };
    SettingProviderSetting setting = {
        .name = "test_custom_point",
        .interface = &interface,
        .type = SettingProviderSettingTypeCustom,
    };

    {
        TestPoint value = {0};
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(10, value.x);
        mu_assert_int_eq(20, value.y);
    }

    {
        TestPoint value = {.x = 30, .y = 40};
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save custom");

        value.x = 0;
        value.y = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(30, value.x);
        mu_assert_int_eq(40, value.y);
    }

    {
        TestPoint invalid_value = {.x = 200, .y = 200};
        mu_assert(
            !setting_provider_save(provider, &setting, &invalid_value),
            "Should reject invalid custom");

        TestPoint value = {0};
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(30, value.x);
        mu_assert_int_eq(40, value.y);
    }
}

/* test drop functionality */
MU_TEST(setting_provider_test_drop) {
    setting_provider_open(provider);

    SettingProviderIntInterface interface = {
        .default_value = 100,
        .is_valid_callback = NULL,
    };
    SettingProviderSetting setting = {
        .name = "test_int_to_drop",
        .interface = &interface,
        .type = SettingProviderSettingTypeInt,
    };

    int value = 200;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save int");

    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(200, value);

    setting_provider_drop(provider, &setting);

    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(100, value);
}

/* test suite */
MU_TEST_SUITE(setting_provider_test_suite) {
    MU_SUITE_CONFIGURE(&setting_provider_setup, &setting_provider_teardown);
    MU_RUN_TEST(setting_provider_test_bool);
    MU_RUN_TEST(setting_provider_test_int);
    MU_RUN_TEST(setting_provider_test_int_validation);
    MU_RUN_TEST(setting_provider_test_float);
    MU_RUN_TEST(setting_provider_test_float_validation);
    MU_RUN_TEST(setting_provider_test_string);
    MU_RUN_TEST(setting_provider_test_string_validation);
    MU_RUN_TEST(setting_provider_test_custom);
    MU_RUN_TEST(setting_provider_test_drop);
}

int run_minunit_setting_provider_test(void) {
    MU_RUN_SUITE(setting_provider_test_suite);
    return MU_EXIT_CODE;
}
