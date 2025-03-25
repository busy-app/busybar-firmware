#include "sl_update.h"

#include <furi.h>

int32_t sl_update_app(void* arg) {
    UNUSED(arg);

    SlUpdater* instance = sl_updater_alloc();
    sl_updater_run(instance, "/ext/firmware.rps", 6);
    sl_updater_free(instance);

    return 0;
}
