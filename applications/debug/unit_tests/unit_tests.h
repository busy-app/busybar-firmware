#pragma once
#include <storage/storage.h>
#include "minunit.h"

#define UNIT_TESTS_FOLDER EXT_PATH("unit_tests")

// This macro is used to create a path for unit tests files
// We guarantee that the unit_tests folder is always exists
#define UNIT_TESTS_PATH(path) UNIT_TESTS_FOLDER "/" path
