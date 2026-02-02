#include "../unit_tests.h"
#include <device_name/device_name_i.h>

MU_TEST(device_name_test_validation_basic) {
    static const char* const allowed_names[] = {
        DEVICE_NAME_DEFAULT,
        "Anna's BUSY Bar",
        ":) :( ;_; ._. -_=",
        "20chr, just undr lim",
        "BUSY Bar nr. 42",
    };

    static const char* const disallowed_names[] = {
        "21chr, just ovr limit",
        "Very very very very very very very very very long name",
        "<!-- hi -->",
        "<script>",
        "            ",
        " ",
        "",
        "&#104;&#105;", // hi
        "БИЗИ Бар",
    };

    FuriString* name = furi_string_alloc();
    FuriString* error = furi_string_alloc();

    for(size_t i = 0; i < COUNT_OF(allowed_names); i++) {
        furi_string_set_str(name, allowed_names[i]);
        mu_assert_int_eq(true, device_name_validate(name, error));
        mu_assert_int_eq(true, device_name_validate(name, NULL));
    }

    for(size_t i = 0; i < COUNT_OF(disallowed_names); i++) {
        furi_string_set_str(name, disallowed_names[i]);
        furi_string_reset(error);
        mu_assert_int_eq(false, device_name_validate(name, error));
        mu_assert_int_greater_than(0, furi_string_size(error));
        mu_assert_int_eq(false, device_name_validate(name, NULL));
    }

    furi_string_free(name);
    furi_string_free(error);
}

MU_TEST_SUITE(device_name_validation_test_suite) {
    MU_RUN_TEST(device_name_test_validation_basic);
}

int run_minunit_device_name_test(void) {
    MU_RUN_SUITE(device_name_validation_test_suite);
    return MU_EXIT_CODE;
}
