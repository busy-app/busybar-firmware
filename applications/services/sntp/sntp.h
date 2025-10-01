#pragma once

#include "sntp_settings.h"

#include <furi.h>

#define RECORD_SNTP "sntp"

typedef struct Sntp Sntp;

#ifdef __cplusplus
extern "C" {
#endif

void sntp_status_update(Sntp* instance, bool success);

const SntpSettings* sntp_get_settings(const Sntp* instance);

bool sntp_set_settings(Sntp* instance, const SntpSettings* settings);

#ifdef __cplusplus
}
#endif
