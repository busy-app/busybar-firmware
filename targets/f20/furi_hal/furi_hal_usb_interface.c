#include <furi_hal.h>
#include <tusb.h>
#include "furi_hal_usb_i.h"
#include "furi_hal_usb_interface_i.h"
#include "class/cdc/cdc.h"
#include "class/hid/hid_device.h"

#define TAG "USB IF"

#define CDC_EP_BUF_SIZE 1024
#define CDC_TX_BUF_SIZE 1024
#define CDC_RX_BUF_SIZE 2048

#define STR_INDEX_ETH_MAC  4
#define STR_INDEX_ETH_NAME 5

enum {
    INTERFACE_ID_VCP = 0,
    INTERFACE_ID_VCP_DATA = 1,
    INTERFACE_ID_HID = 2,
    INTERFACE_ID_ETH = 3,
    INTERFACE_ID_ETH_DATA = 4,
    INTERFACE_COUNT,
};

static tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),

    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x4567,
    .idProduct = 0x89ab,
    .bcdDevice = VERSION_BCD(1, 0, 1),

    .iManufacturer = UsbDevManuf,
    .iProduct = UsbDevProduct,
    .iSerialNumber = UsbDevSerial,

    .bNumConfigurations = 0x01,
};

#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_ECM_DESC_LEN)

#define EPNUM_CDC_NOTIF 0x81
#define EPNUM_CDC_OUT   0x02
#define EPNUM_CDC_IN    0x82
#define EPNUM_HID       0x83
#define EPNUM_ETH_NOTIF 0x84
#define EPNUM_ETH_OUT   0x05
#define EPNUM_ETH_IN    0x85

enum {
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_MOUSE,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_COUNT,
};

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(REPORT_ID_MOUSE)),
    TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL)),
};

static uint8_t const desc_fs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, INTERFACE_COUNT, 0, CONFIG_TOTAL_LEN, 0x00, 500),

    // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(INTERFACE_ID_VCP, 0, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(
        INTERFACE_ID_HID,
        0,
        HID_ITF_PROTOCOL_NONE,
        sizeof(desc_hid_report),
        EPNUM_HID,
        8,
        5),

    TUD_CDC_ECM_DESCRIPTOR(
        INTERFACE_ID_ETH,
        STR_INDEX_ETH_NAME,
        STR_INDEX_ETH_MAC,
        EPNUM_ETH_NOTIF,
        64, //TODO:
        EPNUM_ETH_OUT,
        EPNUM_ETH_IN,
        64,
        USB_ETH_MTU),
};

static uint8_t const desc_hs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, INTERFACE_COUNT, 0, CONFIG_TOTAL_LEN, 0x00, 500),

    // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(INTERFACE_ID_VCP, 0, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),

    // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
    TUD_HID_DESCRIPTOR(
        INTERFACE_ID_HID,
        0,
        HID_ITF_PROTOCOL_NONE,
        sizeof(desc_hid_report),
        EPNUM_HID,
        8,
        5),

    TUD_CDC_ECM_DESCRIPTOR(
        INTERFACE_ID_ETH,
        STR_INDEX_ETH_NAME,
        STR_INDEX_ETH_MAC,
        EPNUM_ETH_NOTIF,
        64, //TODO:
        EPNUM_ETH_OUT,
        EPNUM_ETH_IN,
        512,
        USB_ETH_MTU),
};

uint8_t const* usbd_get_report_desc(void) {
    return desc_hid_report;
}

static void* usbd_interface_init(void* settings) {
    usbd_cdc_init(settings);
    usbd_hid_init(NULL);
    usbd_eth_init(NULL);

    return NULL;
}

static void usbd_interface_deinit(void* intf_inst) {
    UNUSED(intf_inst);
    usbd_cdc_deinit();
    usbd_hid_deinit();
    usbd_eth_deinit();
    dcd_edpt_close_all(BOARD_TUD_RHPORT); // TODO: usbd_edpt_close_all
}

static void usbd_interface_reset(uint8_t rhport) {
    usbd_cdc_reset(rhport);
    usbd_hid_reset(rhport);
    usbd_eth_reset(rhport);
}

static uint16_t
    usbd_interface_open(uint8_t rhport, tusb_desc_interface_t const* itf_desc, uint16_t max_len) {
    if(itf_desc->bInterfaceNumber == INTERFACE_ID_VCP) {
        return usbd_cdc_open(rhport, itf_desc, max_len);
    } else if(itf_desc->bInterfaceNumber == INTERFACE_ID_HID) {
        return usbd_hid_open(rhport, itf_desc, max_len);
    } else if(itf_desc->bInterfaceNumber == INTERFACE_ID_ETH) {
        return usbd_eth_open(rhport, itf_desc, max_len);
    }
    return sizeof(tusb_desc_interface_t);
}

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
static bool usbd_interface_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const* request) {
    if(request->wIndex == INTERFACE_ID_VCP) {
        return usbd_cdc_control_xfer_cb(rhport, stage, request);
    } else if(request->wIndex == INTERFACE_ID_HID) {
        return usbd_hid_control_xfer_cb(rhport, stage, request);
    } else if((request->wIndex == INTERFACE_ID_ETH) || (request->wIndex == INTERFACE_ID_ETH_DATA)) {
        return usbd_eth_control_xfer_cb(rhport, stage, request);
    }

    return true;
}

static bool usbd_interface_xfer_cb(
    uint8_t rhport,
    uint8_t ep_addr,
    xfer_result_t result,
    uint32_t xferred_bytes) {
    usbd_cdc_xfer_cb(rhport, ep_addr, result, xferred_bytes);
    usbd_hid_xfer_cb(rhport, ep_addr, result, xferred_bytes);
    usbd_eth_xfer_cb(rhport, ep_addr, result, xferred_bytes);

    return true;
}

char* usbd_interface_str_cb(uint8_t rhport, uint8_t index) {
    UNUSED(rhport);
    if(index == STR_INDEX_ETH_MAC) {
        return usbd_eth_get_mac_str();
    } else if(index == STR_INDEX_ETH_NAME) {
        return "USB ETH";
    }
    return NULL;
}

FuriHalUsbInterface usb_default = {
    .init = usbd_interface_init,
    .deinit = usbd_interface_deinit,

    .reset = usbd_interface_reset,
    .open = usbd_interface_open,
    .control_xfer_cb = usbd_interface_control_xfer_cb,
    .xfer_cb = usbd_interface_xfer_cb,
    .str_desc_custom = usbd_interface_str_cb,
    .sof = NULL,

    .connect_state = NULL,

    .dev_descr = (tusb_desc_device_t*)&desc_device,
    .str_manuf_descr = "TinyUSB",
    .str_prod_descr = "TinyUSB Device",
    .str_serial_descr = "0",
    .cfg_fs_descr = (uint8_t*)desc_fs_configuration,
    .cfg_hs_descr = (uint8_t*)desc_hs_configuration,
};
