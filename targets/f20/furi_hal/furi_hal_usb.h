#pragma once

void furi_hal_usb_init(void);

void furi_hal_usb_set_irq(FuriHalInterruptISR usb_isr, void* isr_ctx);
