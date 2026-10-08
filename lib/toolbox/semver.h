/**
 * @file semver.h
 * @brief
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
} SemVer;

bool semver_parse(SemVer* instance, const char* str);
