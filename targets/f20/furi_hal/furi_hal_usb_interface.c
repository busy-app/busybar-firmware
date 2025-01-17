#include <furi_hal.h>
#include <tusb.h>
#include "furi_hal_usb_i.h"
#include "furi_hal_usb_interface_i.h"
#include "class/cdc/cdc.h"
#include "class/hid/hid_device.h"
#include "class/net/ncm.h"

#define TAG "USB IF"

#define WEBUSB_URL "lwip.local"

#define CDC_EP_BUF_SIZE 1024
#define CDC_TX_BUF_SIZE 1024
#define CDC_RX_BUF_SIZE 2048

#define STR_INDEX_ETH_MAC  4
#define STR_INDEX_ETH_NAME 5

enum {
    INTERFACE_ID_ETH = 0,
    INTERFACE_ID_ETH_DATA,
    INTERFACE_ID_VCP,
    INTERFACE_ID_VCP_DATA,
    INTERFACE_ID_HID,
    INTERFACE_COUNT,
};

enum {
    VENDOR_REQUEST_WEBUSB = 1,
    VENDOR_REQUEST_MSOS20 = 2
};

static tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = VERSION_BCD(2, 1, 0),

    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0x37C1,
    .idProduct = 0x0001,
    .bcdDevice = VERSION_BCD(1, 0, 1),

    .iManufacturer = UsbDevManuf,
    .iProduct = UsbDevProduct,
    .iSerialNumber = UsbDevSerial,

    .bNumConfigurations = 0x01,
};

#define CONFIG_TOTAL_LEN \
    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN + TUD_CDC_NCM_DESC_LEN)
#define BOS_TOTAL_LEN (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

#define MS_OS_20_DESC_LEN 0xB2

#define EPNUM_ETH_NOTIF 0x81
#define EPNUM_ETH_OUT   0x02
#define EPNUM_ETH_IN    0x82
#define EPNUM_CDC_NOTIF 0x83
#define EPNUM_CDC_OUT   0x04
#define EPNUM_CDC_IN    0x84
#define EPNUM_HID       0x85

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

    // Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
    TUD_CDC_NCM_DESCRIPTOR(
        INTERFACE_ID_ETH,
        STR_INDEX_ETH_NAME,
        STR_INDEX_ETH_MAC,
        EPNUM_ETH_NOTIF,
        64,
        EPNUM_ETH_OUT,
        EPNUM_ETH_IN,
        64,
        USB_ETH_MTU),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
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
};

static uint8_t const desc_hs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, INTERFACE_COUNT, 0, CONFIG_TOTAL_LEN, 0x00, 500),

    // Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
    TUD_CDC_NCM_DESCRIPTOR(
        INTERFACE_ID_ETH,
        STR_INDEX_ETH_NAME,
        STR_INDEX_ETH_MAC,
        EPNUM_ETH_NOTIF,
        64,
        EPNUM_ETH_OUT,
        EPNUM_ETH_IN,
        512,
        USB_ETH_MTU),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
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
};

static uint8_t const desc_bos[] = {
    // total length, number of device caps
    TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 2),

    // Vendor Code, iLandingPage
    TUD_BOS_WEBUSB_DESCRIPTOR(VENDOR_REQUEST_WEBUSB, 1),

    // Microsoft OS 2.0 descriptor
    TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, VENDOR_REQUEST_MSOS20),
};

static uint8_t const desc_ms_os_20[] = {
    /* clang-format off */
    // Set header: length, type, windows version, total length
    U16_TO_U8S_LE(0x000A),
    U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000),
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

    // Configuration subset header: length, type, configuration index, reserved, configuration total length
    U16_TO_U8S_LE(0x0008),
    U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A),

    // Function Subset header: length, type, first interface, reserved, subset length
    U16_TO_U8S_LE(0x0008),
    U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), INTERFACE_ID_ETH, 0,
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08),

    // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
    U16_TO_U8S_LE(0x0014),
    U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID),
    'W', 'I', 'N', 'N', 'C', 'M', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    // MS OS 2.0 Registry property descriptor: length, type
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08 - 0x08 - 0x14),
    U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),

    // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
    U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), 
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 
    't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 
    'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,

    U16_TO_U8S_LE(0x0050), // wPropertyDataLength
    // bPropertyData: {12345678-0D08-43FD-8B3E-127CA8AFFF9D} 
    '{', 0x00, '1', 0x00, '2', 0x00, '3', 0x00, '4', 0x00, '5', 0x00, '6', 0x00, '7', 0x00, 
    '8', 0x00, '-', 0x00, '0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00, '-', 0x00, '4', 0x00, 
    '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00, '8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, 
    '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00, 'C', 0x00, 'A', 0x00, '8', 0x00, 'A', 0x00, 
    'F', 0x00, 'F', 0x00, 'F', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00,
    /* clang-format on */
};

static const tusb_desc_webusb_url_t desc_webusb_url = {
    .bLength = 3 + sizeof(WEBUSB_URL) - 1,
    .bDescriptorType = 3, // WEBUSB URL type
    .bScheme = 0, // 0: http, 1: https
    .url = WEBUSB_URL,
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

static bool usbd_interface_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const* request) {
    if(request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR) {
        if(stage == CONTROL_STAGE_SETUP) {
            switch(request->bRequest) {
            case VENDOR_REQUEST_WEBUSB:
                // Get landing page url
                return tud_control_xfer(
                    rhport, request, (void*)(uintptr_t)&desc_webusb_url, desc_webusb_url.bLength);

            case VENDOR_REQUEST_MSOS20:
                if(request->wIndex == 7) {
                    // Get Microsoft OS 2.0 compatible descriptor
                    uint16_t total_len = 0;
                    memcpy(&total_len, desc_ms_os_20 + 8, 2);

                    return tud_control_xfer(
                        rhport, request, (void*)(uintptr_t)desc_ms_os_20, total_len);
                } else {
                    return false;
                }

            default:
                break;
            }
        }

    } else {
        if(request->wIndex == INTERFACE_ID_VCP) {
            return usbd_cdc_control_xfer_cb(rhport, stage, request);
        } else if(request->wIndex == INTERFACE_ID_HID) {
            return usbd_hid_control_xfer_cb(rhport, stage, request);
        } else if(
            (request->wIndex == INTERFACE_ID_ETH) || (request->wIndex == INTERFACE_ID_ETH_DATA)) {
            return usbd_eth_control_xfer_cb(rhport, stage, request);
        } else if(
            (request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS) &&
            (request->bRequest == NCM_GET_NTB_PARAMETERS)) {
            return usbd_eth_control_xfer_cb(rhport, stage, request);
        }
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
    .bos_descr = (uint8_t*)desc_bos,
    .str_manuf_descr = "Flipper Devices",
    .str_prod_descr = "BusyStatusBar",
    .str_serial_descr = "0", // TODO: furi_hal_version
    .cfg_fs_descr = (uint8_t*)desc_fs_configuration,
    .cfg_hs_descr = (uint8_t*)desc_hs_configuration,
};
