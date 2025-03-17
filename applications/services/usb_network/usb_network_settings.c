#include <stdint.h>
#include <furi.h>
#include "usb_network_settings.h"
#include <storage/storage.h>
#include <cjson/cJSON.h>

// TODO: furi_hal_version
#define DEFAULT_MAC         {0x0C, 0xFA, 0x22, 0x01, 0x23, 0x45}
#define DEFAULT_HOSTNAME    "busybar"
#define DEFAULT_WEBUSB_ZONE ".local"

#define SETTINGS_FILE "/ext/data/settings/usb_network_settings.json"

#define TAG "USB NET"

static FuriString* hostname = NULL;
static FuriString* webusb_url = NULL;

static UsbNetworkAddress address = {
    .ip = {10, 12, 34, 1},
    .netmask = {255, 255, 255, 0},
    .gateway = {0, 0, 0, 0},
};

static uint8_t mac_address[6] = DEFAULT_MAC;

UsbNetworkAddress usb_network_settings_get_address(void) {
    return address;
}

const uint8_t* usb_network_settings_get_mac_address(void) {
    return mac_address;
}

const char* usb_network_settings_get_hostname(void) {
    furi_check(hostname);
    return furi_string_get_cstr(hostname);
}

const char* usb_network_settings_get_webusb_url(void) {
    furi_check(webusb_url);
    return furi_string_get_cstr(webusb_url);
}

static bool furi_string_from_cjson(FuriString* str, cJSON* root, const char* key) {
    cJSON* item = cJSON_GetObjectItem(root, key);

    if(item) {
        furi_string_set(str, item->valuestring);
        return true;
    } else {
        return false;
    }
}

static bool usb_network_ip_from_cjson(UsbNetworkIp* ip, cJSON* root, const char* key) {
    cJSON* item = cJSON_GetObjectItem(root, key);

    if(item) {
        unsigned int ip_arr[4];
        size_t items = sscanf(
            item->valuestring, "%u.%u.%u.%u", &ip_arr[0], &ip_arr[1], &ip_arr[2], &ip_arr[3]);
        if(items == 4 && ip_arr[0] < 256 && ip_arr[1] < 256 && ip_arr[2] < 256 &&
           ip_arr[3] < 256) {
            *ip = (UsbNetworkIp){ip_arr[0], ip_arr[1], ip_arr[2], ip_arr[3]};
            return true;
        }
    }

    return false;
}

static bool usb_settings_load(Storage* storage) {
    bool result = false;
    File* file = storage_file_alloc(storage);

    do {
        if(!storage_file_open(file, SETTINGS_FILE, FSAM_READ, FSOM_OPEN_EXISTING)) {
            break;
        }

        size_t file_size = storage_file_size(file);
        if(file_size == 0) {
            break;
        }

        char* buffer = malloc(file_size + 1);
        if(!buffer) {
            break;
        }

        if(storage_file_read(file, buffer, file_size) != file_size) {
            break;
        }

        buffer[file_size] = '\0';

        result = true;

        cJSON* root = cJSON_Parse(buffer);
        if(root) {
            furi_string_from_cjson(hostname, root, "hostname");
            furi_string_from_cjson(webusb_url, root, "webusb_url");
            usb_network_ip_from_cjson(&address.ip, root, "ip");
            usb_network_ip_from_cjson(&address.netmask, root, "netmask");
        }
        cJSON_Delete(root);

    } while(false);

    storage_file_free(file);
    return result;
}

void usb_network_settings_init(void) {
    hostname = furi_string_alloc_set(DEFAULT_HOSTNAME);
    webusb_url = furi_string_alloc_set(DEFAULT_HOSTNAME DEFAULT_WEBUSB_ZONE);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    usb_settings_load(storage);
    furi_record_close(RECORD_STORAGE);

    FURI_LOG_D(TAG, "Hostname: %s", furi_string_get_cstr(hostname));
    FURI_LOG_D(TAG, "WebUSB URL: %s", furi_string_get_cstr(webusb_url));
    FURI_LOG_D(TAG, "IP: %d.%d.%d.%d", address.ip.a, address.ip.b, address.ip.c, address.ip.d);
    FURI_LOG_D(
        TAG,
        "Netmask: %d.%d.%d.%d",
        address.netmask.a,
        address.netmask.b,
        address.netmask.c,
        address.netmask.d);
}
