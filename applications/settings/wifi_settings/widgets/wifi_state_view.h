#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WifiStateView WifiStateView;

WifiStateView* wifi_state_view_front_alloc(Widget* parent);

WifiStateView* wifi_state_view_back_alloc(Widget* parent);

void wifi_state_view_free(WifiStateView* instance);

void wifi_state_view_set_state(WifiStateView* instance, bool connected, const char* ssid);

#ifdef __cplusplus
}
#endif
