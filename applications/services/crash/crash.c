#include <furi.h>

#define CRASH_DELAY_MS (3000)

int32_t crash_srv(void*) {
    furi_delay_ms(CRASH_DELAY_MS);
    furi_crash("Crash service invoked");
}
