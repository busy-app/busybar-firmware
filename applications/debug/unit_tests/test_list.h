/**
 * @file test_list.h
 * @brief Lists the tests
 */

#pragma once

#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TEST_FUNCTION_DECLS
#include "test_test/test_test.h"
#include "pipe_test/pipe_test.h"
#include "json_helper_test/json_helper_test.h"
#include "setting_provider_test/setting_provider_test.h"
#include "state_test/state_test.h"
#include "tar_test/tar_test.h"
#include "device_name_test/device_name_test.h"
#include "storage_test/storage_test.h"
#undef TEST_FUNCTION_DECLS

typedef int (*TestCallback)(void);

static TestCallback unit_test_callbacks[] = {
#define TEST_FUNCTION_REFS
#include "test_test/test_test.h"
#include "pipe_test/pipe_test.h"
#include "json_helper_test/json_helper_test.h"
#include "setting_provider_test/setting_provider_test.h"
#include "state_test/state_test.h"
#include "tar_test/tar_test.h"
#include "device_name_test/device_name_test.h"
#include "storage_test/storage_test.h"
#undef TEST_FUNCTION_REFS
};

#define UNIT_TEST_COUNT COUNT_OF(unit_test_callbacks)

#ifdef __cplusplus
}
#endif
