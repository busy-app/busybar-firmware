#include "../unit_tests.h"
#include <device_name/device_name_i.h>

typedef struct {
    const char* name;
    DeviceNameError status;
} DeviceNameTestDisallowedName;

MU_TEST(device_name_test_validation_basic) {
    static const char* const allowed_names[] = {
        DEVICE_NAME_DEFAULT,
        "Anna's BUSY Bar",
        ":) :( ;_; ._. -_=",
        "20chr, just undr lim",
        "BUSY Bar nr. 42",
    };

    static DeviceNameTestDisallowedName disallowed_names[] = {
        {
            .name = "21chr, just ovr limit",
            .status = DeviceNameErrorTooLong,
        },
        {
            .name = "Very very very very very very very very very long name",
            .status = DeviceNameErrorTooLong,
        },
        {
            .name = "            ",
            .status = DeviceNameErrorOnlySpaces,
        },
        {
            .name = " ",
            .status = DeviceNameErrorOnlySpaces,
        },
        {
            .name = "",
            .status = DeviceNameErrorEmpty,
        },
        {
            .name = "БИЗИ Бар",
            .status = DeviceNameErrorIllegalChar,
        },

    };

    for(size_t i = 0; i < COUNT_OF(allowed_names); i++) {
        mu_assert_int_eq(DeviceNameErrorNone, device_name_validate(allowed_names[i]));
    }

    for(size_t i = 0; i < COUNT_OF(disallowed_names); i++) {
        mu_assert_int_eq(
            disallowed_names[i].status, device_name_validate(disallowed_names[i].name));
    }
}

MU_TEST_SUITE(device_name_validation_test_suite) {
    MU_RUN_TEST(device_name_test_validation_basic);
}

int run_minunit_device_name_test(void) {
    MU_RUN_SUITE(device_name_validation_test_suite);
    return MU_EXIT_CODE;
}
