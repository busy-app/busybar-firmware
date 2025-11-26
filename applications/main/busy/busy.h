#pragma once

#define RECORD_BUSY_APP "busy_app"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BusyApp BusyApp;

void busy_show_timer(BusyApp* instance);

#ifdef __cplusplus
}
#endif
