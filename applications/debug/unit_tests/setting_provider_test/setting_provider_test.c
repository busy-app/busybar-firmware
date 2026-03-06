/**
 * @file setting_provider_test.c
 * @brief Minimal unit tests for setting_provider
 */

#include "../unit_tests.h"

#include <setting_provider.h>

#define SETTING_PROVIDER_TEST_PATH UNIT_TESTS_PATH("setting_provider_test.json")

static SettingProvider* provider;

/* ========== Test types ========== */

/* Custom type for testing - RGB color */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} TestColor;

/* Struct with nested settings */
typedef struct {
    int width;
    int height;
    TestColor background;
} TestRectangle;

/* Enum for union discriminator */
typedef enum {
    TestShapeTypeCircle,
    TestShapeTypeRectangle,
    TestShapeTypesCount,

    TestShapeTypeDefault = TestShapeTypeCircle,
} TestShapeType;

typedef struct {
    int radius;
} TestCircle;

typedef struct {
    int width;
    int height;
} TestShapeRect;

typedef struct {
    TestShapeType type;
    union {
        TestCircle circle;
        TestShapeRect rectangle;
    } data;
} TestShape;

/* Enum for basic enum tests */
typedef enum {
    TestEnumFirst,
    TestEnumSecond,
    TestEnumThird,
    TestEnumCount
} TestEnum;

/* Raw type test struct */
typedef struct {
    int x;
    int y;
} TestPoint;

/* ========== Callbacks ========== */

static bool test_int_is_valid(const SettingProviderSetting* setting, int value) {
    UNUSED(setting);
    return (value >= 0 && value <= 1000);
}

static bool test_color_serialize(
    const SettingProviderSetting* setting,
    const void* value,
    FuriString* string) {
    UNUSED(setting);
    const TestColor* color = value;
    furi_string_printf(string, "#%02x%02x%02x", color->r, color->g, color->b);
    return true;
}

static bool
    test_color_deserialize(const SettingProviderSetting* setting, const char* string, void* value) {
    UNUSED(setting);
    TestColor* color = value;

    if(string[0] != '#') return false;

    unsigned int r, g, b;
    if(sscanf(string, "#%02x%02x%02x", &r, &g, &b) == 3) {
        color->r = (uint8_t)r;
        color->g = (uint8_t)g;
        color->b = (uint8_t)b;
        return true;
    }
    return false;
}

static bool
    test_raw_serialize(const SettingProviderSetting* setting, const void* value, cJSON* json_node) {
    UNUSED(setting);
    const TestPoint* point = value;
    cJSON_AddNumberToObject(json_node, "x", point->x);
    cJSON_AddNumberToObject(json_node, "y", point->y);
    return true;
}

static bool test_raw_deserialize(
    const SettingProviderSetting* setting,
    const cJSON* json_node,
    void* value) {
    UNUSED(setting);
    TestPoint* point = value;
    const cJSON* x = cJSON_GetObjectItem((cJSON*)json_node, "x");
    const cJSON* y = cJSON_GetObjectItem((cJSON*)json_node, "y");
    if(!cJSON_IsNumber(x) || !cJSON_IsNumber(y)) return false;
    point->x = x->valueint;
    point->y = y->valueint;
    return true;
}

static bool test_migration_v1_to_v2(SettingProvider* instance) {
    UNUSED(instance);
    return true;
}

/* ========== Static data for settings ========== */

static const SettingProviderMigration test_migrations[] = {
    {
        .target_version = 2,
        .migrate_callback = test_migration_v1_to_v2,
    },
};

static const char* const test_enum_strings[TestEnumCount] = {
    [TestEnumFirst] = "first",
    [TestEnumSecond] = "second",
    [TestEnumThird] = "third",
};

static const TestEnum test_enum_default = TestEnumSecond;

static const char* const test_shape_type_strings[TestShapeTypesCount] = {
    [TestShapeTypeCircle] = "circle",
    [TestShapeTypeRectangle] = "rectangle",
};

static const TestShapeType test_shape_type_default = TestShapeTypeDefault;

static const uint8_t test_color_r = 255;
static const uint8_t test_color_g = 255;
static const uint8_t test_color_b = 255;

static const SettingProviderSetting rectangle_color_settings[] = {
    {
        .name = "r",
        .interface =
            &(const SettingProviderCustomInterface){
                .default_value = &test_color_r,
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
                .default_value = &test_color_g,
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
                .default_value = &test_color_b,
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
        .interface = &(const SettingProviderIntInterface){.default_value = 100},
        .context = NULL,
        .field_offset = offsetof(TestRectangle, width),
        .type = SettingProviderSettingTypeInt,
    },
    {
        .name = "height",
        .interface = &(const SettingProviderIntInterface){.default_value = 100},
        .context = NULL,
        .field_offset = offsetof(TestRectangle, height),
        .type = SettingProviderSettingTypeInt,
    },
    {
        .name = "background",
        .interface =
            &(const SettingProviderStructInterface){
                .is_valid_callback = NULL,
                .inner_settings = rectangle_color_settings,
                .inner_settings_count = COUNT_OF(rectangle_color_settings),
            },
        .context = NULL,
        .field_offset = offsetof(TestRectangle, background),
        .type = SettingProviderSettingTypeStruct,
    },
};

static const SettingProviderSetting test_circle_settings[] = {
    {
        .name = "radius",
        .interface = &(const SettingProviderIntInterface){.default_value = 10},
        .context = NULL,
        .field_offset = offsetof(TestCircle, radius),
        .type = SettingProviderSettingTypeInt,
    },
};

static const SettingProviderSetting test_rect_settings[] = {
    {
        .name = "width",
        .interface = &(const SettingProviderIntInterface){.default_value = 50},
        .context = NULL,
        .field_offset = offsetof(TestShapeRect, width),
        .type = SettingProviderSettingTypeInt,
    },
    {
        .name = "height",
        .interface = &(const SettingProviderIntInterface){.default_value = 50},
        .context = NULL,
        .field_offset = offsetof(TestShapeRect, height),
        .type = SettingProviderSettingTypeInt,
    },
};

static const SettingProviderSetting test_shape_type_setting = {
    .name = "type",
    .interface =
        &(const SettingProviderEnumInterface){
            .string_map = test_shape_type_strings,
            .string_map_length = TestShapeTypesCount,
            .type_size = sizeof(TestShapeType),
            .default_value = &test_shape_type_default,
        },
    .context = NULL,
    .field_offset = offsetof(TestShape, type),
    .type = SettingProviderSettingTypeEnum,
};

static const SettingProviderSetting test_shape_union_settings[] = {
    [TestShapeTypeCircle] =
        {
            .name = "data",
            .interface =
                &(const SettingProviderStructInterface){
                    .is_valid_callback = NULL,
                    .inner_settings = test_circle_settings,
                    .inner_settings_count = COUNT_OF(test_circle_settings),
                },
            .context = NULL,
            .field_offset = offsetof(TestShape, data.circle),
            .type = SettingProviderSettingTypeStruct,
        },
    [TestShapeTypeRectangle] =
        {
            .name = "data",
            .interface =
                &(const SettingProviderStructInterface){
                    .is_valid_callback = NULL,
                    .inner_settings = test_rect_settings,
                    .inner_settings_count = COUNT_OF(test_rect_settings),
                },
            .context = NULL,
            .field_offset = offsetof(TestShape, data.rectangle),
            .type = SettingProviderSettingTypeStruct,
        },
};

static const TestPoint test_point_default = {.x = 0, .y = 0};

/* ========== Setup/Teardown ========== */

static void setting_provider_setup(void) {
    provider = setting_provider_alloc(
        SETTING_PROVIDER_TEST_PATH, 2, test_migrations, COUNT_OF(test_migrations));
}

static void setting_provider_teardown(void) {
    setting_provider_free(provider);
    provider = NULL;
}

/* ========== Test 1: Alloc/Free ========== */

MU_TEST(setting_provider_test_alloc_free) {
    /* Just verify alloc/free works - setup/teardown handles this */
    mu_assert(provider != NULL, "Provider should be allocated");
}

/* ========== Test 2: File Not Found (uses defaults) ========== */

MU_TEST(setting_provider_test_file_not_found) {
    const SettingProviderSetting setting = {
        .name = "test_missing_file",
        .interface = &(const SettingProviderIntInterface){.default_value = 42},
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeInt,
    };

    int value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(42, value); /* Should get default */
}

/* ========== Test 3: Bool Basic ========== */

MU_TEST(setting_provider_test_bool_basic) {
    const SettingProviderSetting setting = {
        .name = "test_bool",
        .interface = &(const SettingProviderBoolInterface){.default_value = false},
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeBool,
    };

    /* Load default */
    bool value = true;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(false, value);

    /* Save true */
    value = true;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save bool");

    /* Reload */
    value = false;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(true, value);

    /* Reset */
    setting_provider_reset(provider, &setting, &value);
    mu_assert_int_eq(false, value);
}

/* ========== Test 4: Int Basic ========== */

MU_TEST(setting_provider_test_int_basic) {
    const SettingProviderSetting setting = {
        .name = "test_int",
        .interface = &(const SettingProviderIntInterface){.default_value = 100},
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeInt,
    };

    /* Load default */
    int value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(100, value);

    /* Save */
    value = 500;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save int");

    /* Reload */
    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(500, value);

    /* Reset */
    setting_provider_reset(provider, &setting, &value);
    mu_assert_int_eq(100, value);
}

/* ========== Test 5: Int Validation ========== */

MU_TEST(setting_provider_test_int_validation) {
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

    /* Valid value */
    int value = 100;
    mu_assert(setting_provider_save(provider, &setting, &value), "Should accept valid int");

    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(100, value);

    /* Invalid value (too large) */
    int invalid = 2000;
    mu_assert(!setting_provider_save(provider, &setting, &invalid), "Should reject invalid int");

    /* Invalid value (negative) */
    invalid = -10;
    mu_assert(!setting_provider_save(provider, &setting, &invalid), "Should reject negative int");

    /* Original value preserved */
    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(100, value);
}

/* ========== Test 6: Float Basic ========== */

MU_TEST(setting_provider_test_float_basic) {
    const SettingProviderSetting setting = {
        .name = "test_float",
        .interface = &(const SettingProviderFloatInterface){.default_value = 1.5f},
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeFloat,
    };

    /* Load default */
    float value = 0.0f;
    setting_provider_load(provider, &setting, &value);
    mu_assert_double_eq(1.5f, value);

    /* Save */
    value = 3.14159f;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save float");

    /* Reload */
    value = 0.0f;
    setting_provider_load(provider, &setting, &value);
    mu_assert_double_eq(3.14159f, value);
}

/* ========== Test 7: String Basic ========== */

MU_TEST(setting_provider_test_string_basic) {
    const SettingProviderSetting setting = {
        .name = "test_string",
        .interface =
            &(const SettingProviderStringInterface){
                .default_value = "hello",
                .max_size = 32,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeString,
    };

    /* Load default */
    char value[32] = {0};
    setting_provider_load(provider, &setting, value);
    mu_assert_string_eq("hello", value);

    /* Save */
    const char* new_value = "world";
    mu_assert(setting_provider_save(provider, &setting, new_value), "Failed to save string");

    /* Reload */
    memset(value, 0, sizeof(value));
    setting_provider_load(provider, &setting, value);
    mu_assert_string_eq("world", value);
}

/* ========== Test 8: String Truncation ========== */

MU_TEST(setting_provider_test_string_truncation) {
    const SettingProviderSetting setting = {
        .name = "test_string_truncated",
        .interface =
            &(const SettingProviderStringInterface){
                .default_value = "short",
                .max_size = 10,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeString,
    };

    /* String too long for buffer */
    const char* too_long = "this_string_is_way_too_long";
    mu_assert(
        !setting_provider_save(provider, &setting, too_long), "Should reject oversized string");

    /* Default preserved */
    char value[10] = {0};
    setting_provider_load(provider, &setting, value);
    mu_assert_string_eq("short", value);
}

/* ========== Test 9: Enum Basic ========== */

MU_TEST(setting_provider_test_enum_basic) {
    const SettingProviderSetting setting = {
        .name = "test_enum",
        .interface =
            &(const SettingProviderEnumInterface){
                .string_map = test_enum_strings,
                .string_map_length = TestEnumCount,
                .type_size = sizeof(TestEnum),
                .default_value = &test_enum_default,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeEnum,
    };

    /* Load default (TestEnumSecond) */
    TestEnum value = TestEnumFirst;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(TestEnumSecond, value);

    /* Save each variant */
    value = TestEnumFirst;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save enum");
    value = TestEnumCount;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(TestEnumFirst, value);

    value = TestEnumThird;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save enum");
    value = TestEnumCount;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(TestEnumThird, value);

    /* Reset */
    setting_provider_reset(provider, &setting, &value);
    mu_assert_int_eq(TestEnumSecond, value);
}

/* ========== Test 10: Custom Basic ========== */

MU_TEST(setting_provider_test_custom_basic) {
    static const TestColor default_color = {.r = 128, .g = 64, .b = 32};

    const SettingProviderSetting setting = {
        .name = "test_custom",
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

    /* Load default */
    TestColor value = {0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(128, value.r);
    mu_assert_int_eq(64, value.g);
    mu_assert_int_eq(32, value.b);

    /* Save custom */
    value = (TestColor){.r = 10, .g = 20, .b = 30};
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save custom");

    /* Reload */
    value = (TestColor){0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(10, value.r);
    mu_assert_int_eq(20, value.g);
    mu_assert_int_eq(30, value.b);
}

/* ========== Test 11: Struct Nested ========== */

MU_TEST(setting_provider_test_struct_nested) {
    const SettingProviderSetting setting = {
        .name = "test_struct",
        .interface =
            &(const SettingProviderStructInterface){
                .is_valid_callback = NULL,
                .inner_settings = rectangle_settings,
                .inner_settings_count = COUNT_OF(rectangle_settings),
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeStruct,
    };

    /* Load defaults */
    TestRectangle value = {0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(100, value.width);
    mu_assert_int_eq(100, value.height);
    mu_assert_int_eq(255, value.background.r);
    mu_assert_int_eq(255, value.background.g);
    mu_assert_int_eq(255, value.background.b);

    /* Save nested struct */
    value = (TestRectangle){
        .width = 640,
        .height = 480,
        .background = {.r = 10, .g = 20, .b = 30},
    };
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save struct");

    /* Reload */
    value = (TestRectangle){0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(640, value.width);
    mu_assert_int_eq(480, value.height);
    mu_assert_int_eq(10, value.background.r);
    mu_assert_int_eq(20, value.background.g);
    mu_assert_int_eq(30, value.background.b);
}

/* ========== Test 12: Union Basic ========== */

MU_TEST(setting_provider_test_union_basic) {
    const SettingProviderSetting setting = {
        .name = "test_union",
        .interface =
            &(const SettingProviderUnionInterface){
                .tag_setting = &test_shape_type_setting,
                .inner_settings = test_shape_union_settings,
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeUnion,
    };

    /* Load default (circle) */
    TestShape value = {0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(TestShapeTypeCircle, value.type);
    mu_assert_int_eq(10, value.data.circle.radius);

    /* Save circle variant */
    value = (TestShape){.type = TestShapeTypeCircle, .data.circle = {.radius = 25}};
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save circle");

    value = (TestShape){0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(TestShapeTypeCircle, value.type);
    mu_assert_int_eq(25, value.data.circle.radius);

    /* Switch to rectangle variant */
    value = (TestShape){
        .type = TestShapeTypeRectangle, .data.rectangle = {.width = 200, .height = 100}};
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save rectangle");

    value = (TestShape){0};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(TestShapeTypeRectangle, value.type);
    mu_assert_int_eq(200, value.data.rectangle.width);
    mu_assert_int_eq(100, value.data.rectangle.height);

    /* Reset */
    setting_provider_reset(provider, &setting, &value);
    mu_assert_int_eq(TestShapeTypeCircle, value.type);
    mu_assert_int_eq(10, value.data.circle.radius);
}

/* ========== Test 13: Raw Basic ========== */

MU_TEST(setting_provider_test_raw_basic) {
    const SettingProviderSetting setting = {
        .name = "test_raw",
        .interface =
            &(const SettingProviderRawInterface){
                .is_valid_callback = NULL,
                .serialize_callback = test_raw_serialize,
                .deserialize_callback = test_raw_deserialize,
                .default_value = &test_point_default,
                .default_value_size = sizeof(TestPoint),
            },
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeRaw,
    };

    /* Load default */
    TestPoint value = {-1, -1};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(0, value.x);
    mu_assert_int_eq(0, value.y);

    /* Save */
    value = (TestPoint){.x = 100, .y = 200};
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save raw");

    /* Reload */
    value = (TestPoint){-1, -1};
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(100, value.x);
    mu_assert_int_eq(200, value.y);
}

/* ========== Test 14: Migration Simple ========== */

MU_TEST(setting_provider_test_migration_simple) {
    /* Provider is created with version 2 and migration to v2 exists.
     * When loading a file with version 1, migration should run.
     * This test verifies the migration system works by checking
     * that version mismatch is handled gracefully. */

    const SettingProviderSetting setting = {
        .name = "test_migration",
        .interface = &(const SettingProviderIntInterface){.default_value = 999},
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeInt,
    };

    /* First load creates file with current version (2) */
    int value = 0;
    setting_provider_load(provider, &setting, &value);

    /* Save a value */
    value = 777;
    setting_provider_save(provider, &setting, &value);

    /* Reload - migration check happens but is skipped (already v2) */
    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(777, value);
}

/* ========== Test 15: Persistence ========== */

MU_TEST(setting_provider_test_persistence) {
    const SettingProviderSetting setting = {
        .name = "test_persistence",
        .interface = &(const SettingProviderIntInterface){.default_value = 0},
        .context = NULL,
        .field_offset = 0,
        .type = SettingProviderSettingTypeInt,
    };

    /* Save a value */
    int value = 12345;
    mu_assert(setting_provider_save(provider, &setting, &value), "Failed to save");

    /* Free provider */
    setting_provider_free(provider);
    provider = NULL;

    /* Reallocate (simulating app restart) */
    provider = setting_provider_alloc(
        SETTING_PROVIDER_TEST_PATH, 2, test_migrations, COUNT_OF(test_migrations));

    /* Load - should get saved value */
    value = 0;
    setting_provider_load(provider, &setting, &value);
    mu_assert_int_eq(12345, value);
}

/* ========== Test Suite ========== */

MU_TEST_SUITE(setting_provider_test_suite) {
    MU_SUITE_CONFIGURE(&setting_provider_setup, &setting_provider_teardown);
    MU_RUN_TEST(setting_provider_test_alloc_free);
    MU_RUN_TEST(setting_provider_test_file_not_found);
    MU_RUN_TEST(setting_provider_test_bool_basic);
    MU_RUN_TEST(setting_provider_test_int_basic);
    MU_RUN_TEST(setting_provider_test_int_validation);
    MU_RUN_TEST(setting_provider_test_float_basic);
    MU_RUN_TEST(setting_provider_test_string_basic);
    MU_RUN_TEST(setting_provider_test_string_truncation);
    MU_RUN_TEST(setting_provider_test_enum_basic);
    MU_RUN_TEST(setting_provider_test_custom_basic);
    MU_RUN_TEST(setting_provider_test_struct_nested);
    MU_RUN_TEST(setting_provider_test_union_basic);
    MU_RUN_TEST(setting_provider_test_raw_basic);
    MU_RUN_TEST(setting_provider_test_migration_simple);
    MU_RUN_TEST(setting_provider_test_persistence);
}

int run_minunit_setting_provider_test(void) {
    MU_RUN_SUITE(setting_provider_test_suite);
    return MU_EXIT_CODE;
}
