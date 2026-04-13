#pragma once

#include "busy_timer_saved_state_interface_v1.h"

typedef BusyTimerSavedStateV1 BusyTimerSavedState;

void busy_timer_saved_state_load(BusyTimerSavedState* saved_state);
void busy_timer_saved_state_save(const BusyTimerSavedState* saved_state);
