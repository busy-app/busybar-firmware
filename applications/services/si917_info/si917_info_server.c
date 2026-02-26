#include <furi.h>
#include <intercom/intercom.h>
#include <wifi/wifi.h>
#include "si917_info_common.h"

#include <formatters/sl_rps/sl_rps.h>
#include <sl_wifi.h>
#include <rsi_bt_common_apis.h>

#define TAG "Si917InfoServer"

typedef enum {
    Si917InfoEventRequest,
} Si917InfoServerEvent;

typedef struct {
    Intercom* intercom;
    IntercomChannel* intercom_ch;
    FuriEventLoop* event_loop;
    FuriMessageQueue* command_queue;
} Si917InfoServer;

static void si917_info_send_response(Si917InfoServer* instance) {
    Si917InfoResponseMessage response = {.type = Si917InfoMessageResponse};

    furi_record_open(RECORD_WIFI);
    sl_status_t status = SL_STATUS_FAIL;
    do {
        sl_wifi_firmware_version_t fw_version;
        status = sl_wifi_get_firmware_version(&fw_version);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to get firmware version: 0x%08lX", status);
            break;
        } else {
            FuriString* version_str = furi_string_alloc();
            sl_rps_format_nwp_version(
                version_str,
                &(SlRpsNwpVersion){
                    .major = fw_version.major,
                    .minor = fw_version.minor,
                    .patch = fw_version.patch_num,
                    .build = fw_version.build_num,
                    .security = fw_version.security_version,
                    .rom_id = fw_version.rom_id,
                    .chip_id = fw_version.chip_id,
                    .customer_id = fw_version.customer_id,
                });
            furi_assert(furi_string_size(version_str) <= SI917_NWP_VERSION_STR_LEN_MAX);
            strncpy(
                response.data.nwp_version,
                furi_string_get_cstr(version_str),
                SI917_NWP_VERSION_STR_LEN_MAX);
            furi_string_free(version_str);
        }

        sl_mac_address_t mac_addr = {0};
        status = rsi_bt_get_local_device_address((uint8_t*)&mac_addr);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to get local device address: 0x%08lX", status);
            break;
        } else {
            // RSI provides BLE MAC address with reverse byte order, we need to change the order back
            for(uint8_t i = 0; i < 6; i++) {
                response.data.ble_mac[i] = mac_addr.octet[5 - i];
            }
        }
        status = sl_wifi_get_mac_address(SL_WIFI_CLIENT_INTERFACE, &mac_addr);
        if(status != SL_STATUS_OK) {
            FURI_LOG_E(TAG, "Failed to get WiFi MAC address: 0x%08lX", status);
            break;
        } else {
            for(uint8_t i = 0; i < 6; i++) {
                response.data.wifi_mac[i] = mac_addr.octet[i];
            }
        }
    } while(0);
    furi_record_close(RECORD_WIFI);

    if(status == SL_STATUS_OK) {
        size_t tx_size = intercom_tx(
            instance->intercom_ch, &response, sizeof(Si917InfoResponseMessage), FuriWaitForever);
        furi_check(tx_size == sizeof(Si917InfoResponseMessage), "Failed to send data");
    }
}

static void si917_info_server_message_callback(FuriEventLoopObject* object, void* context) {
    Si917InfoServer* instance = context;
    furi_check(instance);
    furi_check(object == instance->command_queue);

    Si917InfoServerEvent event;
    furi_check(furi_message_queue_get(instance->command_queue, &event, 0) == FuriStatusOk);

    if(event == Si917InfoEventRequest) {
        si917_info_send_response(instance);
    }
}

static void si917_info_server_rx_callback(const void* data, size_t data_size, void* context) {
    furi_assert(data);

    furi_assert(data_size == sizeof(Si917InfoRequestMessage));
    const Si917InfoRequestMessage* msg = data;
    furi_assert(msg->type == Si917InfoMessageRequest);

    furi_assert(context);
    Si917InfoServer* instance = context;

    Si917InfoServerEvent event = Si917InfoEventRequest;

    furi_check(
        furi_message_queue_put(instance->command_queue, &event, FuriWaitForever) == FuriStatusOk);
}

int32_t si917_info_server_init(void* arg) {
    UNUSED(arg);
    Si917InfoServer* instance = malloc(sizeof(Si917InfoServer));
    instance->event_loop = furi_event_loop_alloc();
    instance->command_queue = furi_message_queue_alloc(8, sizeof(Si917InfoServerEvent));
    furi_event_loop_subscribe_message_queue(
        instance->event_loop,
        instance->command_queue,
        FuriEventLoopEventIn,
        si917_info_server_message_callback,
        instance);

    instance->intercom = furi_record_open(RECORD_INTERCOM);
    instance->intercom_ch = intercom_channel_open(
        instance->intercom, IntercomChannelIdSi917Info, si917_info_server_rx_callback, instance);

    FURI_LOG_I(TAG, "Start");

    furi_event_loop_run(instance->event_loop);

    return 0;
}
