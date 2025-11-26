#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_dac_init(void);

void furi_hal_dac_pa_enable(void);

void furi_hal_dac_pa_disable(void);

typedef size_t (*FuriHalDacCallback)(uint8_t* buffer, size_t buffer_size, void* context);

void furi_hal_dac_start(FuriHalDacCallback callback, void* callback_context, uint32_t samplerate);

void furi_hal_dac_stop(void);

#ifdef __cplusplus
}
#endif
