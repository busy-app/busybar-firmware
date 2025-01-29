#pragma once

#include <tusb.h>
// #include <device/dcd.h>
#include <device/usbd_pvt.h>

#define VERSION_BCD(maj, min, rev) (((maj & 0xFF) << 8) | ((min & 0x0F) << 4) | (rev & 0x0F))

void usb_net_init(void);

/* String descriptors */
enum UsbDevDescStr {
    UsbDevLang = 0,
    UsbDevManuf = 1,
    UsbDevProduct = 2,
    UsbDevSerial = 3,
};
