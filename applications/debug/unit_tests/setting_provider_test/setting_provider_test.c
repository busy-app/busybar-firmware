/**
 * @file setting_provider_test.c
 * @brief Unit tests for setting_provider
 */

#include "../unit_tests.h"

#include <toolbox/setting_provider.h>

#define SETTING_PROVIDER_TEST_PATH UNIT_TESTS_PATH("setting_provider_test.json")

static SettingProvider* provider;

/* custom type for testing - RGB color */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} TestColor;

/* test structure with nested settings */
typedef struct {
    int width;
    int height;
    TestColor background;
} TestRectangle;

/* validation callbacks */
static bool test_int_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);
    return (value >= 0 && value <= 1000);
}

static bool test_float_is_valid(const SettingProviderSetting* setting, float value) {
    UNUSED(setting);
    return (value >= 0.0f && value <= 100.0f);
}

static bool test_string_is_valid(const SettingProviderSetting* setting, const char* value) {
    UNUSED(setting);
    return (strlen(value) <= 20);
}

static bool
    test_furi_string_is_valid(const SettingProviderSetting* setting, const FuriString* value) {
    UNUSED(setting);
    return (furi_string_size(value) <= 20);
}

/* custom type callbacks for color */
static bool test_color_serialize(
    const SettingProviderSetting* setting,
    FuriString* string,
    const void* value) {
    UNUSED(setting);
    const TestColor* color = value;
    furi_string_printf(string, "#%02x%02x%02x", color->r, color->g, color->b);
    return true;
}

static bool test_color_deserialize(
    const SettingProviderSetting* setting,
    void* value,
    const FuriString* string) {
    UNUSED(setting);
    TestColor* color = value;
    const char* str = furi_string_get_cstr(string);

    if(str[0] != '#') return false;

    unsigned int r, g, b;
    if(sscanf(str, "#%02x%02x%02x", &r, &g, &b) == 3) {
        color->r = (uint8_t)r;
        color->g = (uint8_t)g;
        color->b = (uint8_t)b;
        return true;
    }
    return false;
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

/* static default values for nested settings */
static const uint8_t test_default_r = 255;
static const uint8_t test_default_g = 255;
static const uint8_t test_default_b = 255;

/* nested settings for rectangle */
static const SettingProviderSetting rectangle_color_settings[] = {
    {
        .name = "r",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &test_default_r,
                .serialize_callback = test_color_serialize,
                .deserialize_callback = test_color_deserialize,
                .default_value_size = sizeof(uint8_t),
            },
        .context = NULL,
        .field_offset = offsetof(TestColor, r),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "g",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &test_default_g,
                .serialize_callback = test_color_serialize,
                .deserialize_callback = test_color_deserialize,
                .default_value_size = sizeof(uint8_t),
            },
        .context = NULL,
        .field_offset = offsetof(TestColor, g),
        .type = SettingProviderSettingTypeCustom,
    },
    {
        .name = "b",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &test_default_b,
                .serialize_callback = test_color_serialize,
                .deserialize_callback = test_color_deserialize,
                .default_value_size = sizeof(uint8_t),
            },
        .context = NULL,
        .field_offset = offsetof(TestColor, b),
        .type = SettingProviderSettingTypeCustom,
    },
};

static const SettingProviderSetting rectangle_settings[] = {
    {
        .name = "width",
        .interface =
            &(const SettingProviderIntInterface){
                .default_value = 100,
                .is_valid_callback = NULL,
            },
        .context = NULL,
        .field_offset = offsetof(TestRectangle, width),
        .type = SettingProviderSettingTypeInt,
    },
    {
        .name = "height",
        .interface =
            &(const SettingProviderIntInterface){
                .default_value = 100,
                .is_valid_callback = NULL,
            },
        .context = NULL,
        .field_offset = offsetof(TestRectangle, height),
        .type = SettingProviderSettingTypeInt,
    },
    {
        .name = "background",
        .interface =
            &(const SettingProviderStructureInterface){
                .is_valid_callback = NULL,
                .inner_settings = rectangle_color_settings,
                .inner_settings_count = COUNT_OF(rectangle_color_settings),
            },
        .context = NULL,
        .field_offset = offsetof(TestRectangle, background),
        .type = SettingProviderSettingTypeStructure,
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

static void setting_provider_reopen(void) {
    setting_provider_teardown();
    setting_provider_setup();
    setting_provider_open(provider);
}

/* test basic bool operations */
MU_TEST(setting_provider_test_bool) {
    setting_provider_open(provider);

    {
        const SettingProviderSetting setting = {
            .name = "test_bool_false",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeBool,
        };
        bool value = true;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(false, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_bool_true",
            .interface = &(const SettingProviderBoolInterface){.default_value = true},
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeBool,
        };
        bool value = false;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(true, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_bool_saved",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeBool,
        };
        bool value = true;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save bool");

        value = false;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(true, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_bool_reset",
            .interface = &(const SettingProviderBoolInterface){.default_value = false},
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeBool,
        };
        bool value = true;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save bool");

        setting_provider_reset(provider, &setting, &value);
        mu_assert_int_eq(false, value);
    }
}

/* test basic int operations */
MU_TEST(setting_provider_test_int) {
    setting_provider_open(provider);

    {
        const SettingProviderSetting setting = {
            .name = "test_int_zero",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = 0,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeInt,
        };
        int value = 999;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(0, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_int_default",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = 42,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeInt,
        };
        int value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(42, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_int_saved",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = 0,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeInt,
        };
        int value = 123;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save int");

        value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(123, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_int_reset",
            .interface =
                &(const SettingProviderIntInterface){
                    .default_value = 100, .is_valid_callback = NULL},
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeInt,
        };
        int value = 200;
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save int");

        setting_provider_reset(provider, &setting, &value);
        mu_assert_int_eq(100, value);
    }
}

/* test int validation */
MU_TEST(setting_provider_test_int_validation) {
    setting_provider_open(provider);

    const SettingProviderSetting setting = {
        .name = "test_int_validated",
        .interface =
            &(const SettingProviderIntInterface){
                .default_value = 50,
                .is_valid_callback = test_int_is_valid,
            },
        .context = NULL,
        .field_offset = 0,
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

    {
        int invalid_value = -10;
        mu_assert(
            !setting_provider_save(provider, &setting, &invalid_value),
            "Should reject negative int");

        int value = 0;
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(100, value);
    }
}

/* test basic float operations */
MU_TEST(setting_provider_test_float) {
    setting_provider_open(provider);

    {
        const SettingProviderSetting setting = {
            .name = "test_float_zero",
            .interface =
                &(const SettingProviderFloatInterface){
                    .default_value = 0.0f,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeFloat,
        };
        float value = 999.9f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(0.0f, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_float_default",
            .interface =
                &(const SettingProviderFloatInterface){
                    .default_value = 3.14f,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeFloat,
        };
        float value = 0.0f;
        setting_provider_load(provider, &setting, &value);
        mu_assert_double_eq(3.14f, value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_float_saved",
            .interface =
                &(const SettingProviderFloatInterface){
                    .default_value = 0.0f,
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
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

    const SettingProviderSetting setting = {
        .name = "test_float_validated",
        .interface =
            &(const SettingProviderFloatInterface){
                .default_value = 50.0f,
                .is_valid_callback = test_float_is_valid,
            },
        .context = NULL,
        .field_offset = 0,
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

/* test C-string operations */
MU_TEST(setting_provider_test_string) {
    setting_provider_open(provider);

    {
        const SettingProviderSetting setting = {
            .name = "test_string_default",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "default_string",
                    .is_valid_callback = NULL,
                    .max_length = 32,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeString,
        };
        char value[32];
        setting_provider_load(provider, &setting, value);
        mu_assert_string_eq("default_string", value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_string_saved",
            .interface =
                &(const SettingProviderStringInterface){
                    .default_value = "default",
                    .is_valid_callback = NULL,
                    .max_length = 32,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeString,
        };
        const char* value = "test_value";
        mu_assert(setting_provider_save(provider, &setting, value), "Failed to save string");

        char loaded[32];
        setting_provider_load(provider, &setting, loaded);
        mu_assert_string_eq("test_value", loaded);
    }
}

/* test C-string validation */
MU_TEST(setting_provider_test_string_validation) {
    setting_provider_open(provider);

    const SettingProviderSetting setting = {
        .name = "test_string_validated",
        .interface =
            &(const SettingProviderStringInterface){
                .default_value = "default",
                .is_valid_callback = test_string_is_valid,
                .max_length = 32,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeString,
    };

    {
        const char* value = "valid";
        mu_assert(setting_provider_save(provider, &setting, value), "Failed to save valid string");

        char loaded[32];
        setting_provider_load(provider, &setting, loaded);
        mu_assert_string_eq("valid", loaded);
    }

    {
        const char* invalid_value = "this_string_is_way_too_long";
        mu_assert(
            !setting_provider_save(provider, &setting, invalid_value),
            "Should reject invalid string");

        char loaded[32];
        setting_provider_load(provider, &setting, loaded);
        mu_assert_string_eq("valid", loaded);
    }
}

/* test FuriString operations */
MU_TEST(setting_provider_test_furi_string) {
    setting_provider_open(provider);

    {
        const SettingProviderSetting setting = {
            .name = "test_furi_string_default",
            .interface =
                &(const SettingProviderFuriStringInterface){
                    .default_value = "default_string",
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeFuriString,
        };
        FuriString* value = furi_string_alloc();
        setting_provider_load(provider, &setting, &value);
        mu_assert_string_eq("default_string", furi_string_get_cstr(value));
        furi_string_free(value);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_furi_string_saved",
            .interface =
                &(const SettingProviderFuriStringInterface){
                    .default_value = "default",
                    .is_valid_callback = NULL,
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeFuriString,
        };
        FuriString* value = furi_string_alloc_set("test_value");
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save FuriString");

        furi_string_reset(value);
        setting_provider_load(provider, &setting, &value);
        mu_assert_string_eq("test_value", furi_string_get_cstr(value));
        furi_string_free(value);
    }
}

/* test FuriString validation */
MU_TEST(setting_provider_test_furi_string_validation) {
    setting_provider_open(provider);

    const SettingProviderSetting setting = {
        .name = "test_furi_string_validated",
        .interface =
            &(const SettingProviderFuriStringInterface){
                .default_value = "default",
                .is_valid_callback = test_furi_string_is_valid,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeFuriString,
    };

    {
        FuriString* value = furi_string_alloc_set("valid");
        mu_assert(
            setting_provider_save(provider, &setting, &value), "Failed to save valid FuriString");
        furi_string_free(value);

        value = furi_string_alloc();
        setting_provider_load(provider, &setting, &value);
        mu_assert_string_eq("valid", furi_string_get_cstr(value));
        furi_string_free(value);
    }

    {
        FuriString* invalid_value = furi_string_alloc_set("this_string_is_way_too_long");
        mu_assert(
            !setting_provider_save(provider, &setting, &invalid_value),
            "Should reject invalid FuriString");
        furi_string_free(invalid_value);

        FuriString* value = furi_string_alloc();
        setting_provider_load(provider, &setting, &value);
        mu_assert_string_eq("valid", furi_string_get_cstr(value));
        furi_string_free(value);
    }
}

/* test custom type (RGB color) */
MU_TEST(setting_provider_test_custom) {
    setting_provider_open(provider);

    static const TestColor default_red = {.r = 255, .g = 0, .b = 0};

    {
        const SettingProviderSetting setting = {
            .name = "test_color_red",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &default_red,
                    .serialize_callback = test_color_serialize,
                    .deserialize_callback = test_color_deserialize,
                    .default_value_size = sizeof(TestColor),
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeCustom,
        };

        TestColor value = {0};
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(255, value.r);
        mu_assert_int_eq(0, value.g);
        mu_assert_int_eq(0, value.b);
    }

    static const TestColor default_color = {.r = 128, .g = 64, .b = 32};

    {
        const SettingProviderSetting setting = {
            .name = "test_color_custom",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &default_color,
                    .serialize_callback = test_color_serialize,
                    .deserialize_callback = test_color_deserialize,
                    .default_value_size = sizeof(TestColor),
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeCustom,
        };

        TestColor value = {.r = 30, .g = 40, .b = 50};
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save custom");

        value = (TestColor){0};
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(30, value.r);
        mu_assert_int_eq(40, value.g);
        mu_assert_int_eq(50, value.b);
    }

    static const TestColor default_validate_color = {.r = 100, .g = 100, .b = 100};

    {
        const SettingProviderSetting setting = {
            .name = "test_color_validated",
            .interface =
                &(const SettingProviderCustomInterface){
                    .default_value = &default_validate_color,
                    .serialize_callback = test_color_serialize,
                    .deserialize_callback = test_color_deserialize,
                    .default_value_size = sizeof(TestColor),
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeCustom,
        };

        TestColor value = {.r = 30, .g = 40, .b = 50};
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save custom");

        TestColor invalid_value = {.r = 30, .g = 40, .b = 50};
        mu_assert(
            setting_provider_save(provider, &setting, &invalid_value),
            "Should accept valid color");
    }
}

/* test nested structure settings */
MU_TEST(setting_provider_test_structure) {
    setting_provider_open(provider);

    {
        const SettingProviderSetting setting = {
            .name = "test_rectangle",
            .interface =
                &(const SettingProviderStructureInterface){
                    .is_valid_callback = NULL,
                    .inner_settings = rectangle_settings,
                    .inner_settings_count = COUNT_OF(rectangle_settings),
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeStructure,
        };

        TestRectangle value = {0};
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(100, value.width);
        mu_assert_int_eq(100, value.height);
        mu_assert_int_eq(255, value.background.r);
        mu_assert_int_eq(255, value.background.g);
        mu_assert_int_eq(255, value.background.b);
    }

    {
        const SettingProviderSetting setting = {
            .name = "test_rectangle_saved",
            .interface =
                &(const SettingProviderStructureInterface){
                    .is_valid_callback = NULL,
                    .inner_settings = rectangle_settings,
                    .inner_settings_count = COUNT_OF(rectangle_settings),
                },
            .context = NULL,
            .field_offset = 0,
            .type = SettingProviderSettingTypeStructure,
        };

        TestRectangle value = {
            .width = 640,
            .height = 480,
            .background =
                {
                    .r = 10,
                    .g = 20,
                    .b = 30,
                },
        };
        mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save structure");

        value = (TestRectangle){0};
        setting_provider_load(provider, &setting, &value);
        mu_assert_int_eq(640, value.width);
        mu_assert_int_eq(480, value.height);
        mu_assert_int_eq(10, value.background.r);
        mu_assert_int_eq(20, value.background.g);
        mu_assert_int_eq(30, value.background.b);
    }
}

/* test drop functionality */
MU_TEST(setting_provider_test_drop) {
    setting_provider_open(provider);

    const SettingProviderSetting setting = {
        .name = "test_int_to_drop",
        .interface =
            &(const SettingProviderIntInterface){
                .default_value = 100,
                .is_valid_callback = NULL,
            },
        .context = NULL,
        .field_offset = 0,
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

/* test drop all functionality */
MU_TEST(setting_provider_test_drop_all) {
    setting_provider_open(provider);

    const SettingProviderSetting setting1 = {
        .name = "test_drop_all_1",
        .interface =
            &(const SettingProviderIntInterface){
                .default_value = 100,
                .is_valid_callback = NULL,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeInt,
    };

    const SettingProviderSetting setting2 = {
        .name = "test_drop_all_2",
        .interface =
            &(const SettingProviderIntInterface){
                .default_value = 200,
                .is_valid_callback = NULL,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeInt,
    };

    int value1 = 150;
    int value2 = 250;
    mu_assert(setting_provider_save(provider, &setting1, &value1), "Failed to save int");
    mu_assert(setting_provider_save(provider, &setting2, &value2), "Failed to save int");

    value1 = 0;
    value2 = 0;
    setting_provider_load(provider, &setting1, &value1);
    setting_provider_load(provider, &setting2, &value2);
    mu_assert_int_eq(150, value1);
    mu_assert_int_eq(250, value2);

    setting_provider_drop(provider, NULL);

    value1 = 0;
    value2 = 0;
    setting_provider_load(provider, &setting1, &value1);
    setting_provider_load(provider, &setting2, &value2);
    mu_assert_int_eq(100, value1);
    mu_assert_int_eq(200, value2);

    const int rewrite_value1 = 333;
    const int rewrite_value2 = 444;
    mu_assert(
        setting_provider_save(provider, &setting1, &rewrite_value1),
        "Failed to save int after drop");
    mu_assert(
        setting_provider_save(provider, &setting2, &rewrite_value2),
        "Failed to save int after drop");

    setting_provider_reopen();

    value1 = 0;
    value2 = 0;
    setting_provider_load(provider, &setting1, &value1);
    setting_provider_load(provider, &setting2, &value2);
    mu_assert_int_eq(rewrite_value1, value1);
    mu_assert_int_eq(rewrite_value2, value2);
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
    MU_RUN_TEST(setting_provider_test_furi_string);
    MU_RUN_TEST(setting_provider_test_furi_string_validation);
    MU_RUN_TEST(setting_provider_test_custom);
    MU_RUN_TEST(setting_provider_test_structure);
    MU_RUN_TEST(setting_provider_test_drop);
    MU_RUN_TEST(setting_provider_test_drop_all);
}

int run_minunit_setting_provider_test(void) {
    MU_RUN_SUITE(setting_provider_test_suite);
    return MU_EXIT_CODE;
}
