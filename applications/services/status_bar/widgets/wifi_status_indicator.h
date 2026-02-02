#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WifiStatusIndicatorStateUnknown,
    WifiStatusIndicatorStateDisconnected,
    WifiStatusIndicatorStateConnecting,
    WifiStatusIndicatorStateConnected,
    WifiStatusIndicatorStateMax,
} WifiStatusIndicatorState;

typedef struct WifiStatusIndicator WifiStatusIndicator;

WifiStatusIndicator* wifi_status_indicator_alloc(Widget* parent);

void wifi_status_indicator_free(WifiStatusIndicator* instance);

Widget* wifi_status_indicator_get_base(WifiStatusIndicator* instance);

void wifi_status_indicator_set_state(WifiStatusIndicator* instance, WifiStatusIndicatorState state);

#ifdef __cplusplus
}
#endif
