#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool furi_hal_sai_init(void);

typedef int16_t (*FuriHalSaiCallback)(void* context);

void furi_hal_sai_start(FuriHalSaiCallback callback, void* callback_context);

void furi_hal_sai_stop(void);

#ifdef __cplusplus
}
#endif
