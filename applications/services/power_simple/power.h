#pragma once

#include <stdint.h>

typedef struct Power Power;

#define RECORD_POWER "power"

Power* power_alloc(void);

void power_run(Power* instance);

void power_off(Power* power);
void power_shutdown(Power* power);
void power_reboot(Power* power);

// TODO: internal API
void power_on_usb_pd_update(Power* power, uint32_t voltage, uint32_t current);
