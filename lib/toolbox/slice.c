#include "slice.h"

#include <core/check.h>

void string_slice_reset(StringSlice* instance) {
    furi_check(instance);
    instance->first_char = "";
    instance->length = 0;
}
