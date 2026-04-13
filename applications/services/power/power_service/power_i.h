#pragma once

#include "power.h"
#include <furi_hal.h>
#include <toolbox/api_lock.h>

// =========================================
// Battery state of charge (power_battery.c)
// =========================================

typedef struct {
    uint8_t* curves;
    uint16_t percent_points;
    uint16_t current_points;
    uint16_t current_range;
    uint16_t tolerance;
    bool is_in_flash;
} PowerBatCalibration;

PowerBatCalibration* power_get_crude_calibration(void);

PowerBatCalibration* power_load_bat_calibration(const char* path);

void power_unload_bat_calibration(PowerBatCalibration* cal);

uint8_t power_get_battery_charge(
    const PowerBatCalibration* cal,
    int32_t voltage_mv,
    int32_t current_ma);

// ===================================
// USB Power Delivery (power_usb_pd.c)
// ===================================

#define PDO_NUMBER_MAX 7

typedef struct {
    uint32_t passive_mode_current;
    size_t cap_number;
    uint8_t cc_line;
    uint8_t cap_id_current;
    struct {
        uint32_t voltage_min;
        uint32_t voltage_max;
        uint32_t current_max;
        uint8_t pdo_id;
        bool is_fixed;
    } cap[PDO_NUMBER_MAX];
} PowerUsbPdCapability;

typedef struct PowerUsbPd PowerUsbPd;

PowerUsbPd* power_usb_pd_alloc(FuriMessageQueue** pd_queue);

void power_usb_pd_msg_handler(FuriEventLoopObject* object, void* context);

void power_usb_pd_start(PowerUsbPd* pd);

void power_usb_pd_get_capabilities(PowerUsbPd* pd, PowerUsbPdCapability* caps);

void power_usb_pd_request_power(PowerUsbPd* pd, uint32_t voltage_mv, uint32_t current_ma);

// ==========
// Public API
// ==========

typedef enum {
    PowerMessageTypeOff,
    PowerMessageTypeReboot,
    PowerMessageTypeGetInfo,
    PowerMessageTypeIsUsbConnected,
    PowerMessageTypeIsBatteryReady,
    PowerMessageTypeChargeEnable,
    PowerMessageTypeSetChargeCurrent,
    PowerMessageTypePdGetInfo,
    PowerMessageTypePdRequest,
    PowerMessageTypeLoadBatCal,

    // TODO: separate queue for internal messages?
    PowerMessageTypeUsbPdUpdate,
} PowerMessageType;

typedef struct {
    PowerMessageType type;
    FuriApiLock lock;
    union {
        PowerRebootMode reboot_mode;
        struct {
            uint32_t voltage;
            uint32_t current;
        } pd_mode;
        PowerInfo* power_info;
        PowerPdInfo* pd_info;
        bool* param_bool;
        int32_t* param_int;
        char* param_str_owned;
    };
} PowerMessage;

// ==============
// Service struct
// ==============

typedef enum {
    PowerBatteryStateNormal,
    PowerBatteryStateLow,
    PowerBatteryStateCritical,
} PowerBatteryState;

struct Power {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriSemaphore* gpio_semaphore;
    PowerUsbPd* usb_pd;
    FuriPubSub* event_pubsub;
    struct {
        bool charger_alive;
        bool battery_ready;
        bool usb_connected;
        bool pd_initialized;
    } state;
    PowerInfo info;
    PowerPdInfo pd_info;
    uint32_t input_current_limit;
    uint32_t charger_current_limit;
    bool charger_enabled;
    PowerBatteryState battery_state;
    PowerBatCalibration* bat_cal;
    bool tried_to_load_storage_cal;

#ifndef FURI_RAM_EXEC
    bool shipping_mode_wait;
#endif
};
