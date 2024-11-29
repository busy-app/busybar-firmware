#include <furi.h>
#include "power.h"

int32_t power_srv_app(void* p) {
    UNUSED(p);

    Power* instance = power_alloc();
    power_run(instance);
    power_free(instance);

    return 0;
}
