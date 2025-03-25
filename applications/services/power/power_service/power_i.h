#pragma once

#include "power.h"
#include <furi_hal.h>
#include <toolbox/api_lock.h>

typedef enum {
    PowerMessageTypeOff,
    PowerMessageTypeReboot,
    PowerMessageTypeGetInfo,
    PowerMessageTypeIsUsbConnected,
    PowerMessageTypeChargeEnable,
    PowerMessageTypeSetChargeCurrent,
    PowerMessageTypePdGetInfo,
    PowerMessageTypePdRequest,

    // TODO: separate queue for internal messages?
    PowerMessageTypeUsbPdUpdate,
} PowerMessageType;

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

typedef struct {
    bool charger_alive;
    bool usb_connected;
} PowerState;

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
    };
} PowerMessage;

struct Power {
    FuriEventLoop* event_loop;
    FuriMessageQueue* message_queue;
    FuriSemaphore* gpio_semaphore;
    PowerUsbPd* usb_pd;
    FuriPubSub* event_pubsub;
    PowerState state;
    PowerInfo info;
    PowerPdInfo pd_info;
    uint32_t input_current_limit;
    uint32_t charger_current_limit;
    bool charger_enabled;
};

PowerUsbPd* power_usb_pd_alloc(FuriMessageQueue** pd_queue);

void power_usb_pd_msg_handler(FuriEventLoopObject* object, void* context);

void power_usb_pd_start(PowerUsbPd* pd);

void power_usb_pd_get_capabilities(PowerUsbPd* pd, PowerUsbPdCapability* caps);

void power_usb_pd_request_power(PowerUsbPd* pd, uint32_t voltage_mv, uint32_t current_ma);

uint8_t power_get_battery_charge(uint32_t voltage_mv, int32_t current_ma, bool is_charging);
