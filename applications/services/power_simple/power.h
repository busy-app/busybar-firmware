#pragma once

typedef struct Power Power;

#define RECORD_POWER "power"

Power* power_alloc(void);

void power_run(Power* instance);

void power_off(Power* power);
void power_shutdown(Power* power);
void power_reboot(Power* power);
