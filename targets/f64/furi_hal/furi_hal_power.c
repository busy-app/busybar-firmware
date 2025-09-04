#include "furi_hal_power.h"

#include <furi_hal_cortex.h>

void furi_hal_power_reset(void) {
    furi_hal_cortex_system_reset();
}
