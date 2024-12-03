#pragma once

typedef struct Power Power;

Power* power_alloc(void);

void power_run(Power* instance);
