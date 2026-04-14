#include <tusb.h>

#include "usb_network_i.h"

#define TAG "UsbGlue"

void tud_mount_cb(void) {
    FURI_LOG_I(TAG, "Mounted");
    usb_network_up();
}

void tud_umount_cb(void) {
    FURI_LOG_I(TAG, "Unmounted");
    usb_network_down();
}

void tud_suspend_cb(bool remote_wakeup_en) {
    UNUSED(remote_wakeup_en);
    FURI_LOG_I(TAG, "Suspended");
    usb_network_down();
}

void tud_resume_cb(void) {
    FURI_LOG_I(TAG, "Resumed");
    usb_network_up();
}

bool tud_network_recv_cb(const uint8_t* src, uint16_t size) {
    return usb_network_rx(src, size);
}

uint16_t tud_network_xmit_cb(uint8_t* dst, void* ref, uint16_t arg) {
    UNUSED(arg);
    return usb_network_tx(dst, ref);
}
