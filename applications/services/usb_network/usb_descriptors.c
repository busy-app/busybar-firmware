#include "usb_network_settings.h"

#include <furi.h>
#include <furi_hal_version.h>
#include <toolbox/hex.h>

#include <tusb.h>
#include <class/net/net_device.h>
#include <class/net/ncm.h>

#define VERSION_BCD(maj, min, rev) (((maj & 0xFF) << 8) | ((min & 0x0F) << 4) | (rev & 0x0F))

// Length of template descriptor
#define USB_NET_NCM_DESC_LEN (8 + 9 + 5 + 5 + 13 + 6 + 7 + 9 + 9 + 7 + 7)

// CDC-ECM Descriptor Template
// Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
/* clang-format off */
#define USB_NET_NCM_DESCRIPTOR(_itfnum, _desc_stridx, _mac_stridx, _ep_notif, _ep_notif_size, _epout, _epin, _epsize, _maxsegmentsize) \
  /* Interface Association */\
  8, TUSB_DESC_INTERFACE_ASSOCIATION, _itfnum, 2, TUSB_CLASS_CDC, CDC_COMM_SUBCLASS_NETWORK_CONTROL_MODEL, 0, 0,\
  /* CDC Control Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_CDC, CDC_COMM_SUBCLASS_NETWORK_CONTROL_MODEL, 0, _desc_stridx,\
  /* CDC-NCM Header */\
  5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_HEADER, U16_TO_U8S_LE(0x0110),\
  /* CDC-NCM Union */\
  5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_UNION, _itfnum, (uint8_t)((_itfnum) + 1),\
  /* CDC-NCM Functional Descriptor */\
  13, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_ETHERNET_NETWORKING, _mac_stridx, 0, 0, 0, 0, U16_TO_U8S_LE(_maxsegmentsize), U16_TO_U8S_LE(0), 0, \
  /* CDC-NCM Functional Descriptor */\
  6, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_NCM, U16_TO_U8S_LE(0x0100), 0, \
  /* Endpoint Notification */\
  7, TUSB_DESC_ENDPOINT, _ep_notif, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_ep_notif_size), 10,\
  /* CDC Data Interface (default inactive) */\
  9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum)+1), 0, 0, TUSB_CLASS_CDC_DATA, 0, NCM_DATA_PROTOCOL_NETWORK_TRANSFER_BLOCK, 0,\
  /* CDC Data Interface (alternative active) */\
  9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum)+1), 1, 2, TUSB_CLASS_CDC_DATA, 0, NCM_DATA_PROTOCOL_NETWORK_TRANSFER_BLOCK, 0,\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0,\
  /* Endpoint Out */\
  7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0
/* clang-format on */

#define NCM_CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + USB_NET_NCM_DESC_LEN)

#define EPNUM_NCM_NOTIF    0x81
#define EPNUM_NCM_DATA_OUT 0x02
#define EPNUM_NCM_DATA_IN  0x82

enum UsbStrDesc {
    UsbStrLang = 0,
    UsbStrManufacturer,
    UsbStrProduct,
    UsbStrSerial,
    UsbStrNcmInterface,
    UsbStrNcmMac,
};

enum {
    UsbItfNcm = 0,
    UsbItfNcmData,
    UsbItfNumTotal,
};

enum {
    UsbVendorReqWebUsb = 1,
    UsbVendorReqMsos20 = 2
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
    .idProduct = 0x6213,
    .bcdDevice = VERSION_BCD(1, 0, 1),

    .iManufacturer = UsbStrManufacturer,
    .iProduct = UsbStrProduct,
    .iSerialNumber = UsbStrSerial,

    .bNumConfigurations = 1,
};

static tusb_desc_device_qualifier_t const desc_qualifier = {
    .bLength = sizeof(tusb_desc_device_qualifier_t),
    .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
    .bcdUSB = VERSION_BCD(2, 1, 0),

    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .bNumConfigurations = 1,
    .bReserved = 0x00,
};

static uint8_t const desc_cfg_hs[] = {
    TUD_CONFIG_DESCRIPTOR(1, UsbItfNumTotal, 0, NCM_CONFIG_TOTAL_LEN, 0x00, 500),
    USB_NET_NCM_DESCRIPTOR(
        UsbItfNcm,
        UsbStrNcmInterface,
        UsbStrNcmMac,
        EPNUM_NCM_NOTIF,
        64,
        EPNUM_NCM_DATA_OUT,
        EPNUM_NCM_DATA_IN,
        512,
        CFG_TUD_NET_MTU),
};

static uint8_t const desc_cfg_fs[] = {
    TUD_CONFIG_DESCRIPTOR(1, UsbItfNumTotal, 0, NCM_CONFIG_TOTAL_LEN, 0x00, 500),
    USB_NET_NCM_DESCRIPTOR(
        UsbItfNcm,
        UsbStrNcmInterface,
        UsbStrNcmMac,
        EPNUM_NCM_NOTIF,
        64,
        EPNUM_NCM_DATA_OUT,
        EPNUM_NCM_DATA_IN,
        64,
        CFG_TUD_NET_MTU),
};

static uint8_t desc_cfg_other_speed[NCM_CONFIG_TOTAL_LEN];

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device;
}

uint8_t const* tud_descriptor_device_qualifier_cb(void) {
    return (uint8_t const*)&desc_qualifier;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    if(index != 0) {
        return NULL;
    }
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_cfg_hs : desc_cfg_fs;
}

uint8_t const* tud_descriptor_other_speed_configuration_cb(uint8_t index) {
    if(index != 0) {
        return NULL;
    }
    const void* cfg = (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_cfg_fs : desc_cfg_hs;
    memcpy(desc_cfg_other_speed, cfg, NCM_CONFIG_TOTAL_LEN);

    desc_cfg_other_speed[1] = TUSB_DESC_OTHER_SPEED_CONFIG;

    return desc_cfg_other_speed;
}

static char const* desc_string_arr[] = {
    [UsbStrLang] = (const char[]){0x09, 0x04},
    [UsbStrManufacturer] = "Flipper Devices Inc.",
    [UsbStrProduct] = "BUSY Bar USB Ethernet",
    [UsbStrSerial] = NULL,
    [UsbStrNcmInterface] = "Network Interface",
    [UsbStrNcmMac] = NULL,
};

static uint16_t desc_string_temp[32 + 1];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    UNUSED(langid);
    size_t chr_count = 0;

    switch(index) {
    case UsbStrLang:
        memcpy(&desc_string_temp[1], desc_string_arr[0], 2);
        chr_count = 1;
        break;

    case UsbStrSerial:
        FuriString* uid = furi_string_alloc();
        hex_bytes_to_string(furi_hal_version_uid(), furi_hal_version_uid_size(), uid);

        size_t uid_len = furi_string_size(uid);
        furi_assert(uid_len < COUNT_OF(desc_string_temp));
        for(uint8_t i = 0; i < uid_len; i++) {
            desc_string_temp[i] = furi_string_get_char(uid, i);
        }
        chr_count = uid_len;
        furi_string_free(uid);
        break;

    case UsbStrNcmMac:
        const uint8_t* ncm_mac = usb_network_settings_get_mac_address();
        for(uint8_t i = 0; i < 6; i++) {
            desc_string_temp[i * 2 + 1] = "0123456789ABCDEF"[(ncm_mac[i] >> 4) & 0xf];
            desc_string_temp[i * 2 + 2] = "0123456789ABCDEF"[(ncm_mac[i] >> 0) & 0xf];
        }
        chr_count = 6 * 2;
        break;

    default:
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if(index >= COUNT_OF(desc_string_arr)) {
            return NULL;
        }

        const char* str = desc_string_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        size_t const max_len = COUNT_OF(desc_string_temp) - 1;
        if(chr_count > max_len) {
            chr_count = max_len;
        }

        // Convert ASCII string into UTF-16
        for(size_t i = 0; i < chr_count; i++) {
            desc_string_temp[i + 1] = str[i];
        }
        break;
    }

    // first byte is length (including header), second byte is string type
    desc_string_temp[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

    return desc_string_temp;
}

#define BOS_TOTAL_LEN                                                             \
    (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN + \
     USB_20_EXT_DESC_LEN)
#define MS_OS_20_DESC_LEN   0xB2
#define USB_20_EXT_DESC_LEN 7

static uint8_t const desc_bos[] = {
    // total length, number of device caps
    TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 3),

    // Vendor Code, iLandingPage
    TUD_BOS_WEBUSB_DESCRIPTOR(UsbVendorReqWebUsb, 1),

    // Microsoft OS 2.0 descriptor
    TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, UsbVendorReqMsos20),

    // USB 2.0 Extension Descriptor
    0x07,
    TUSB_DESC_DEVICE_CAPABILITY,
    DEVICE_CAPABILITY_USB20_EXTENSION,
    0x00,
    0x00,
    0x00,
    0x00};

uint8_t const* tud_descriptor_bos_cb(void) {
    return (uint8_t const*)(desc_bos);
}

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
    U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), UsbItfNcm, 0,
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

bool tud_vendor_control_xfer_cb(
    uint8_t rhport,
    uint8_t stage,
    tusb_control_request_t const* request) {
    // nothing to with DATA & ACK stage
    if(stage != CONTROL_STAGE_SETUP) {
        return true;
    }

    switch(request->bmRequestType_bit.type) {
    case TUSB_REQ_TYPE_VENDOR:
        switch(request->bRequest) {
        case UsbVendorReqWebUsb:

            // Get landing page url
            const char* hostname = usb_network_settings_get_hostname();
            const char* zone = ".local";

            size_t webusb_url_desc_size =
                sizeof(tusb_desc_webusb_url_t) + strlen(hostname) + strlen(zone) + 1;
            tusb_desc_webusb_url_t* webusb_url = alloca(webusb_url_desc_size);
            memset(webusb_url, 0, webusb_url_desc_size);

            webusb_url->bLength = 3 + strlen(hostname) + strlen(zone);
            webusb_url->bDescriptorType = 3; // WEBUSB URL type
            webusb_url->bScheme = 0; // 0: http, 1: https
            strcpy(webusb_url->url, hostname);
            strcat(webusb_url->url, zone);

            return tud_control_xfer(
                rhport, request, (void*)(uintptr_t)webusb_url, webusb_url->bLength);

        case UsbVendorReqMsos20:
            if(request->wIndex == 7) {
                // Get Microsoft OS 2.0 compatible descriptor
                uint16_t total_len = 0;
                furi_assert(sizeof(desc_ms_os_20) > 10);
                memcpy(&total_len, desc_ms_os_20 + 8, sizeof(total_len));

                return tud_control_xfer(
                    rhport, request, (void*)(uintptr_t)desc_ms_os_20, total_len);
            } else {
                return false;
            }

        default:
            break;
        }
        break;

    default:
        break;
    }

    // stall unknown request
    return false;
}
