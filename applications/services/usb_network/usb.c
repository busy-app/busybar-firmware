#include "usb_i.h"

#include <furi.h>
#include <furi_hal.h>
#include <tusb.h>

#define TAG "Usb"

static void usb_core_irq(void* context) {
    UNUSED(context);
    tusb_int_handler(BOARD_TUD_RHPORT, true);
}

int32_t usb_srv(void* p) {
    UNUSED(p);

    usb_network_init();

    furi_hal_usb_set_irq(usb_core_irq, NULL);

    furi_thread_set_current_priority(FuriThreadPriorityHigh);

    const tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO,
    };

    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    while(1) {
        tud_task();
    }

    return 0;
}

int usb_srv_log(const char* fmt, ...) {
    FuriString* string = furi_string_alloc();

    va_list args;
    va_start(args, fmt);
    furi_string_vprintf(string, fmt, args);
    va_end(args);

    furi_string_trim(string, "\r\n");

    if(!furi_string_empty(string)) {
        FURI_LOG_D(TAG, "%s", furi_string_get_cstr(string));
    }

    furi_string_free(string);
    return 0;
}
