#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <m-array.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PDO_NUMBER_MAX 10

typedef struct {
    size_t cap_number;
    struct {
        uint32_t voltage_min;
        uint32_t voltage_max;
        uint32_t current_max;
        bool is_fixed;
    } cap[PDO_NUMBER_MAX];
} UsbPdCapability;

void furi_hal_usb_pd_init(void);

void furi_hal_usb_pd_request_power(
    uint32_t voltage_mv,
    uint32_t current_ma,
    void* callback,
    void* ctx);

FuriPubSub* furi_hal_usb_pd_get_pubsub(void);

#ifdef __cplusplus
}
#endif
