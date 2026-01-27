#pragma once

#include <stdbool.h>

#include <updater/updater.h>

#ifdef __cplusplus
extern "C" {
#endif

void factory_reset_perform(Updater* updater, bool shipping_mode);

#ifdef __cplusplus
}
#endif
