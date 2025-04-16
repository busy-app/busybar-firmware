#pragma once

#include <furi.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Power Power;

#define RECORD_POWER "power"

#define CHARGE_CURRENT_MAX 3300

typedef enum {
    PowerRebootHardware, // Hardware power reboot using charger
    PowerRebootNormal, // Reboot U5 + 917
    PowerRebootNormalU5, // Reboot U5 only
    PowerRebootNormal917, // Reboot 917 only
    PowerRebootDfuU5, // Reboot U5 to DFU
    PowerRebootDfu917, // Reboot 917 to DFU
} PowerRebootMode;

typedef enum {
    PowerEventReady,
    PowerEventBatteryLow,
    PowerEventBatteryHot,
    PowerEventChargerConnect,
    PowerEventChargerDisconnect,
} PowerEventType;

typedef struct {
    PowerEventType type;
} PowerEvent;

typedef struct {
    bool is_charging;
    bool is_full_charged;
    bool charge_enabled;
    uint8_t charge;

    int32_t current_battery;
    uint32_t current_usb;

    uint32_t voltage_battery;
    uint32_t voltage_usb;

    float temperature_charger;
    float temperature_battery;

    uint32_t charge_ilim_usb;
    uint32_t charge_ilim_battery;
} PowerInfo;

typedef struct {
    uint8_t cap_number;
    uint8_t cc_line;
    uint8_t cap_id;
    uint32_t voltage_set;
    uint32_t current_max;
    uint32_t passive_mode_current;
    struct {
        uint32_t voltage_min;
        uint32_t voltage_max;
        uint32_t current_max;
        uint8_t pdo_id;
        bool is_fixed;
    } cap[7];
} PowerPdInfo;

FuriPubSub* power_get_pubsub(Power* power);
bool power_off(Power* power);
void power_reboot(Power* power, PowerRebootMode mode);
bool power_is_usb_connected(Power* power);
bool power_is_battery_ready(Power* power);
void power_get_info(Power* power, PowerInfo* info);
void power_charge_enable(Power* power, bool enable);
void power_set_charge_current(Power* power, uint32_t current_ma);
void power_get_pd_info(Power* power, PowerPdInfo* info);
void power_set_pd_mode(Power* power, uint32_t voltage_mv);

// TODO: internal API
void power_on_usb_pd_update(Power* power, uint32_t voltage, uint32_t current);
