#pragma once

#include <tusb.h>

typedef struct FuriHalUsbInterface FuriHalUsbInterface;

struct FuriHalUsbInterface {
    void* (*init)(void* settings);
    void (*deinit)(void* intf_inst);

    void (*reset)(uint8_t rhport);
    uint16_t (*open)(uint8_t rhport, tusb_desc_interface_t const* desc_intf, uint16_t max_len);
    bool (*control_xfer_cb)(uint8_t rhport, uint8_t stage, tusb_control_request_t const* request);
    bool (*xfer_cb)(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);
    void (*sof)(uint8_t rhport, uint32_t frame_count);
    char* (*str_desc_custom)(uint8_t rhport, uint8_t index);

    void (*connect_state)(bool state);

    tusb_desc_device_t* dev_descr;

    void* str_manuf_descr;
    void* str_prod_descr;
    void* str_serial_descr;

    uint8_t* cfg_fs_descr;
    uint8_t* cfg_hs_descr;
    uint8_t* bos_descr;
};

void furi_hal_usb_init(void);

bool furi_hal_usb_set_config(FuriHalUsbInterface* new_if, void* ctx);

void furi_hal_usb_enable(bool state);
