#pragma once

#include <stdint.h>
#include "furi_hal_usb.h"

#define CDC_MAX_PACKET_LEN (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define USB_ETH_MTU        1514

typedef struct FURI_PACKED {
    uint32_t bit_rate;
    uint8_t stop_bits; ///< 0: 1 stop bit - 1: 1.5 stop bits - 2: 2 stop bits
    uint8_t parity; ///< 0: None - 1: Odd - 2: Even - 3: Mark - 4: Space
    uint8_t data_bits; ///< can be 5, 6, 7, 8 or 16
} CdcLineCoding;

typedef struct {
    void (*tx_done_callback)(void* context);
    void (*rx_callback)(void* context);
    void (*ctrl_line_callback)(bool dtr, bool rts, void* context);
    void (*config_callback)(CdcLineCoding* config, void* context);
    void (*send_break_callback)(uint16_t duration_ms, void* context);
} CdcCallbacks;

typedef struct {
    const CdcCallbacks* callbacks;
    void* context;
} CdcContext;

extern FuriHalUsbInterface usb_default;

bool furi_hal_cdc_is_connected(void);

uint8_t furi_hal_cdc_get_line_state(void);

void furi_hal_cdc_get_line_coding(CdcLineCoding* coding);

uint32_t furi_hal_cdc_available(void);

uint32_t furi_hal_cdc_read(void* buffer, uint32_t bufsize);

bool furi_hal_cdc_peek(uint8_t* chr);

void furi_hal_cdc_read_flush(void);

uint32_t furi_hal_cdc_write(void const* buffer, uint32_t bufsize);

uint32_t furi_hal_cdc_write_flush(void);

uint32_t furi_hal_cdc_write_available(void);

bool furi_hal_cdc_write_clear(void);

bool furi_hal_hid_keyboard_report(uint8_t report_id, uint8_t modifier, uint8_t keycode[6]);

bool furi_hal_hid_mouse_report(
    uint8_t report_id,
    uint8_t buttons,
    int8_t x,
    int8_t y,
    int8_t vertical,
    int8_t horizontal);

void furi_hal_usb_eth_recv_renew(void);

bool furi_hal_usb_eth_can_xmit(uint16_t size);

void furi_hal_usb_eth_xmit(void* ref, uint16_t arg);
