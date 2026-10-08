#include "semver.h"

#include <core/check.h>

#include "slice.h"

#define SEMVER_PLACE_LEN_MAX (10) // Enough to fit UINT32_MAX decimal representation

typedef enum {
    SemVerPlaceIdxMajor,
    SemVerPlaceIdxMinor,
    SemVerPlaceIdxPatch,
    SemVerPlaceIdxMax,
} SemVerPlaceIdx;

static void semver_set_place_value(SemVer* instance, SemVerPlaceIdx place_idx, uint32_t value) {
    if(place_idx == SemVerPlaceIdxMajor) {
        instance->major = value;
    } else if(place_idx == SemVerPlaceIdxMinor) {
        instance->minor = value;
    } else if(place_idx == SemVerPlaceIdxPatch) {
        instance->patch = value;
    } else {
        furi_crash("Invalid SemVerPlaceIdx value");
    }
}

static bool semver_split_places(const char* str, size_t str_len, StringSlice* places) {
    StringSlice* place = places;
    place->first_char = str;
    place->length = 0;

    size_t i, place_idx;
    for(i = 0, place_idx = 0; i < str_len; ++i) {
        if(str[i] == '.') {
            ++place_idx;
            if(place_idx >= SemVerPlaceIdxMax) {
                break;
            }

            place = &places[place_idx];
            place->first_char = str + i + 1;
            place->length = 0;

        } else {
            ++place->length;
        }
    }

    bool can_split = false;

    if((i = str_len) && (place_idx == (SemVerPlaceIdxMax - 1))) {
        can_split = true;
    }

    return can_split;
}

static bool semver_is_place_valid(const StringSlice* place) {
    const size_t place_len = place->length;

    if((place_len == 0) || (place_len > SEMVER_PLACE_LEN_MAX)) {
        return false;
    }

    bool is_valid = true;

    for(size_t i = 0; i < place_len; ++i) {
        int c = place->first_char[i];
        if(c > 0x7f || !isdigit(c)) {
            is_valid = false;
            break;
        }
    }

    return is_valid;
}

static bool semver_validate_places(const StringSlice* places) {
    bool is_valid = true;

    for(size_t i = 0; i < SemVerPlaceIdxMax; ++i) {
        if(!semver_is_place_valid(&places[i])) {
            is_valid = false;
            break;
        }
    }

    return is_valid;
}

static bool semver_parse_place(const StringSlice* place, uint32_t* value) {
    bool can_parse = false;

    char tmp[SEMVER_PLACE_LEN_MAX + 1];
    strncpy(tmp, place->first_char, place->length);
    tmp[place->length] = '\0';

    const long long parsed_val = atoll(tmp);

    if(parsed_val <= UINT32_MAX) {
        *value = parsed_val;
        can_parse = true;
    }

    return can_parse;
}

static bool semver_parse_places(SemVer* instance, const StringSlice* places) {
    bool can_parse = true;

    for(size_t i = 0; i < SemVerPlaceIdxMax; ++i) {
        uint32_t value;
        if(!semver_parse_place(&places[i], &value)) {
            can_parse = false;
            break;
        }

        semver_set_place_value(instance, i, value);
    }

    return can_parse;
}

static bool semver_parse_with_length(SemVer* instance, const char* str, size_t str_len) {
    bool can_parse = false;

    do {
        if(str_len == 0) {
            break;
        }

        StringSlice places[SemVerPlaceIdxMax];

        if(!semver_split_places(str, str_len, places)) {
            break;
        }

        if(!semver_validate_places(places)) {
            break;
        }

        if(!semver_parse_places(instance, places)) {
            break;
        }

        can_parse = true;
    } while(false);

    return can_parse;
}

bool semver_parse(SemVer* instance, const char* str) {
    furi_check(instance);
    furi_check(str);

    const size_t str_len = strlen(str);
    return semver_parse_with_length(instance, str, str_len);
}
