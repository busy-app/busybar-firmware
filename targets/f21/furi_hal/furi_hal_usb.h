#pragma once

#include <furi_hal_interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

void furi_hal_usb_init(void);

void furi_hal_usb_set_irq(FuriHalInterruptISR usb_isr, void* isr_ctx);

#ifdef __cplusplus
}
#endif
