#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WifiNotConnectedView WifiNotConnectedView;

WifiNotConnectedView* wifi_not_connected_view_front_alloc(Widget* parent);

WifiNotConnectedView* wifi_not_connected_view_back_alloc(Widget* parent);

void wifi_not_connected_view_free(WifiNotConnectedView* instance);

#ifdef __cplusplus
}
#endif
