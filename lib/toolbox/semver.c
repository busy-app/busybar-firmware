#include "semver.h"

#include <core/check.h>

#include "slice.h"

#define SEMVER_PART_LEN_MAX (10) // Enough to fit UINT32_MAX decimal representation

typedef enum {
    SemVerPartIdxMajor,
    SemVerPartIdxMinor,
    SemVerPartIdxPatch,
    SemVerPartIdxMax,
} SemVerPartIdx;

static void semver_set_part_value(SemVer* instance, SemVerPartIdx part_idx, uint32_t value) {
    if(part_idx == SemVerPartIdxMajor) {
        instance->major = value;
    } else if(part_idx == SemVerPartIdxMinor) {
        instance->minor = value;
    } else if(part_idx == SemVerPartIdxPatch) {
        instance->patch = value;
    } else {
        furi_crash("Invalid SemVerPartIdx value");
    }
}

static bool semver_split_parts(
    const char* source,
    size_t source_len,
    StringSlice* parts,
    size_t parts_count) {
    StringSlice* part = parts;
    part->first_char = source;
    part->length = 0;

    size_t i, part_idx;
    for(i = 0, part_idx = 0; i < source_len; ++i) {
        if(source[i] == '.') {
            ++part_idx;
            if(part_idx >= parts_count) {
                break;
            }

            part = &parts[part_idx];
            part->first_char = source + i + 1;
            part->length = 0;

        } else {
            ++part->length;
        }
    }

    bool can_split = false;

    if((i = source_len) && (part_idx == (parts_count - 1))) {
        can_split = true;
    }

    return can_split;
}

static bool semver_is_part_valid(const StringSlice* part) {
    const size_t part_len = part->length;

    if((part_len == 0) || (part_len > SEMVER_PART_LEN_MAX)) {
        return false;
    }

    bool is_valid = true;

    for(size_t i = 0; i < part_len; ++i) {
        int c = part->first_char[i];
        if(c > 0x7f || !isdigit(c)) {
            is_valid = false;
            break;
        }
    }

    return is_valid;
}

static bool semver_validate_parts(const StringSlice* parts, size_t parts_count) {
    bool is_valid = true;

    for(size_t i = 0; i < parts_count; ++i) {
        if(!semver_is_part_valid(&parts[i])) {
            is_valid = false;
            break;
        }
    }

    return is_valid;
}

static bool semver_parse_part(const StringSlice* part, uint32_t* value) {
    bool can_parse = false;

    char tmp[SEMVER_PART_LEN_MAX + 1];
    strncpy(tmp, part->first_char, part->length);
    tmp[part->length] = '\0';

    const long long parsed_val = atoll(tmp);

    if(parsed_val <= UINT32_MAX) {
        *value = parsed_val;
        can_parse = true;
    }

    return can_parse;
}

static bool semver_parse_parts(SemVer* instance, const StringSlice* parts, size_t parts_count) {
    bool can_parse = true;

    for(size_t i = 0; i < parts_count; ++i) {
        uint32_t value;
        if(!semver_parse_part(&parts[i], &value)) {
            can_parse = false;
            break;
        }

        semver_set_part_value(instance, i, value);
    }

    return can_parse;
}

static bool semver_parse_with_length(SemVer* instance, const char* source, size_t source_len) {
    bool can_parse = false;

    do {
        if(source_len == 0) {
            break;
        }

        StringSlice parts[SemVerPartIdxMax];

        if(!semver_split_parts(source, source_len, parts, COUNT_OF(parts))) {
            break;
        }

        if(!semver_validate_parts(parts, COUNT_OF(parts))) {
            break;
        }

        if(!semver_parse_parts(instance, parts, COUNT_OF(parts))) {
            break;
        }

        can_parse = true;
    } while(false);

    return can_parse;
}

bool semver_parse(SemVer* instance, const char* source) {
    furi_check(instance);
    furi_check(source);

    const size_t str_len = strlen(source);
    return semver_parse_with_length(instance, source, str_len);
}
