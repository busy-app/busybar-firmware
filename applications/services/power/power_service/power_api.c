#include "power_i.h"

bool power_off(Power* instance) {
    furi_check(instance);

    bool usb_connected = power_is_usb_connected(instance);
    if(usb_connected) {
        return false;
    }

    PowerMessage msg = {
        .type = PowerMessageTypeOff,
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    return true;
}

void power_reboot(Power* instance, PowerRebootMode mode) {
    PowerMessage msg = {
        .type = PowerMessageTypeReboot,
        .reboot_mode = mode,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(instance->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

bool power_is_usb_connected(Power* power) {
    furi_check(power);

    bool ret = false;
    PowerMessage msg = {
        .type = PowerMessageTypeIsUsbConnected,
        .param_bool = &ret,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
    return ret;
}

bool power_is_battery_ready(Power* power) {
    furi_check(power);

    bool ret = false;
    PowerMessage msg = {
        .type = PowerMessageTypeIsBatteryReady,
        .param_bool = &ret,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
    return ret;
}

void power_charge_enable(Power* power, bool enable) {
    furi_check(power);

    bool param = enable;

    PowerMessage msg = {
        .type = PowerMessageTypeChargeEnable,
        .param_bool = &param,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

void power_set_charge_current(Power* power, uint32_t current_ma) {
    furi_check(power);
    furi_check(current_ma <= CHARGE_CURRENT_MAX);

    int32_t param = current_ma;

    PowerMessage msg = {
        .type = PowerMessageTypeSetChargeCurrent,
        .param_int = &param,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

void power_get_info(Power* power, PowerInfo* info) {
    furi_check(power);
    furi_check(info);

    PowerMessage msg = {
        .type = PowerMessageTypeGetInfo,
        .power_info = info,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

void power_get_pd_info(Power* power, PowerPdInfo* info) {
    furi_check(power);
    furi_check(info);

    PowerMessage msg = {
        .type = PowerMessageTypePdGetInfo,
        .pd_info = info,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}

void power_set_pd_mode(Power* power, uint32_t voltage_mv) {
    furi_check(power);
    int32_t param = voltage_mv;

    PowerMessage msg = {
        .type = PowerMessageTypePdRequest,
        .param_int = &param,
        .lock = api_lock_alloc_locked(),
    };

    furi_check(
        furi_message_queue_put(power->message_queue, &msg, FuriWaitForever) == FuriStatusOk);
    api_lock_wait_unlock_and_free(msg.lock);
}
