#include <furi.h>

int32_t crash_srv(void*) {
    furi_crash("Crash service invoked");
}
