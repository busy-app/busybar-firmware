#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct UsbStatusIndicator UsbStatusIndicator;

UsbStatusIndicator* usb_status_indicator_alloc(Widget* parent);

void usb_status_indicator_free(UsbStatusIndicator* instance);

Widget* usb_status_indicator_get_base(UsbStatusIndicator* instance);

void usb_status_indicator_set_connection_state(UsbStatusIndicator* instance, bool is_connected);

#ifdef __cplusplus
}
#endif
