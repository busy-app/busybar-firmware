#pragma once

#define RECORD_INTERCOM_WATCHDOG "intercom_watchdog"

typedef struct IntercomWatchdog IntercomWatchdog;

void intercom_watchdog_arm(IntercomWatchdog* instance);

void intercom_watchdog_disarm(IntercomWatchdog* instance);
