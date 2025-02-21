#include <furi.h>
#include <furi_hal.h>
#include <tusb.h>
#include "usb_i.h"

#define TAG "USB"

typedef struct {
    FuriEventLoop* event_loop;
    FuriSemaphore* usb_semaphore;
} UsbService;

static UsbService* usb_service = NULL;

// void tud_event_hook_cb(uint8_t rhport, uint32_t eventid, bool in_isr) {
//     UNUSED(rhport);
//     UNUSED(eventid);
//     UNUSED(in_isr);

//     furi_semaphore_release(usb_service->usb_semaphore);
// }

static void usb_core_irq(void* context) {
    UNUSED(context);
    tusb_int_handler(BOARD_TUD_RHPORT, true);
}

// static void usb_core_handler(FuriEventLoopObject* object, void* context) {
//     furi_assert(context);
//     UsbService* usb = context;

//     furi_assert(usb->usb_semaphore == object);
//     furi_check(furi_semaphore_acquire(object, 0) == FuriStatusOk);

//     do {
//         tud_task_ext(0, false);
//     } while(tud_task_event_ready());
// }

int32_t usb_srv(void* p) {
    UNUSED(p);

    usb_service = malloc(sizeof(UsbService));

    usb_network_init();

    furi_hal_usb_set_irq(usb_core_irq, usb_service);
    tusb_rhport_init_t dev_init = {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUSB_SPEED_AUTO,
    };
    tusb_init(BOARD_TUD_RHPORT, &dev_init);

    while(1) {
        tud_task_ext(FuriWaitForever, false);
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

    FURI_LOG_D("tUSB", "%s", furi_string_get_cstr(string));
    furi_string_free(string);
    return 0;
}
