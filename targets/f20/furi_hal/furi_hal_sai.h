#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FuriHalSaiEventHalfTransfer,
    FuriHalSaiEventTransferComplete,
} FuriHalSaiEvent;

typedef void (*FuriHalSaiCallback)(FuriHalSaiEvent event, void* context);

bool furi_hal_sai_init(void);

void furi_hal_sai_set_data(const int16_t* data, uint32_t data_count);
void furi_hal_sai_set_callback(FuriHalSaiCallback callback, void* context);

void furi_hal_sai_start(void);
void furi_hal_sai_stop(void);

#ifdef __cplusplus
}
#endif
