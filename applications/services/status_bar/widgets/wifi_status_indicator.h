#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WifiStatusIndicator WifiStatusIndicator;

WifiStatusIndicator* wifi_status_indicator_alloc(Widget* parent);

void wifi_status_indicator_free(WifiStatusIndicator* instance);

Widget* wifi_status_indicator_get_base(WifiStatusIndicator* instance);

#ifdef __cplusplus
}
#endif
