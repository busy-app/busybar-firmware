#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WifiInfoView WifiInfoView;

typedef void (*WifiInfoViewCallback)(int32_t value, void* context);

WifiInfoView* wifi_info_view_front_alloc(Widget* parent);

WifiInfoView* wifi_info_view_back_alloc(Widget* parent);

void wifi_info_view_free(WifiInfoView* instance);

void wifi_info_view_set_address(WifiInfoView* instance, bool is_connected, const char* addr);

#ifdef __cplusplus
}
#endif
