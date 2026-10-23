#include "slice.h"

#include <core/check.h>

void string_slice_reset(StringSlice* instance) {
    furi_check(instance);
    instance->first_char = "";
    instance->length = 0;
}

void string_slice_to_offset(
    const StringSlice* instance,
    StringSliceOffset* offset,
    const char* source) {
    furi_check(instance);
    furi_check(offset);
    furi_check(source);

    const char* first_char = instance->first_char;
    furi_check(first_char);
    furi_check(first_char >= source);

    const size_t start_offset = first_char - source;

    offset->start = start_offset;
    offset->end = start_offset + instance->length;
}

void string_slice_from_offset(
    StringSlice* instance,
    const StringSliceOffset* offset,
    const char* source) {
    furi_check(instance);
    furi_check(offset);
    furi_check(source);

    const size_t start_offset = offset->start;
    const size_t end_offset = offset->end;
    furi_check(end_offset >= start_offset);

    instance->first_char = source + start_offset;
    instance->length = end_offset - start_offset;
}

char* string_slice_to_owned_cstr(const StringSlice* instance) {
    furi_check(instance);
    furi_check(instance->first_char);

    char* cstr = malloc(instance->length + 1);
    memcpy(cstr, instance->first_char, instance->length);
    cstr[instance->length] = '\0';

    return cstr;
}
