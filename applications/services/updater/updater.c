#include <furi.h>

int32_t updater_srv(void* arg) {
    UNUSED(arg);

    furi_thread_suspend(furi_thread_get_current_id());

    return 0;
}
