#include <furi.h>
#include <core/thread.h>

int32_t recovery_srv(void* context) {
    UNUSED(context);
    FURI_LOG_I("Recovery", "Recovery service started");

    while(1) {
        furi_delay_ms(100);
    }

    return 0;
}
