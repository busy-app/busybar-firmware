#pragma once

#include <furi.h>

#define PDO_NUMBER_MAX 7

typedef struct PowerUsbPd PowerUsbPd;

typedef struct {
    uint32_t passive_mode_current;
    size_t cap_number;
    struct {
        uint32_t voltage_min;
        uint32_t voltage_max;
        uint32_t current_max;
        uint8_t pdo_id;
        bool is_fixed;
    } cap[PDO_NUMBER_MAX];
} PowerUsbPdCapability;

PowerUsbPd* power_usb_pd_alloc(FuriMessageQueue** pd_queue);

void power_usb_pd_msg_handler(FuriEventLoopObject* object, void* context);

void power_usb_pd_start(PowerUsbPd* pd);

void power_usb_pd_get_capabilities(PowerUsbPd* pd, PowerUsbPdCapability* caps);

void power_usb_pd_request_power(PowerUsbPd* pd, uint32_t voltage_mv, uint32_t current_ma);
