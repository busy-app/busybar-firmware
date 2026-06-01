#include "ble_worker_i.h"
#include <furi.h>

#include "ble_worker_util.h"

#define TAG "BleWorker"

///TODO: Remove after all connection issues will be resolved
// Uncomment macro below in order to force ble advertising with public address only
// #define BLE_DEBUG_ADVERTISE_FORCE_PUBLIC

#define BLE_DEFAULT_LOCAL_NAME "BUSY Bar"

#define BLE_WORKER_TX_TIMEOUT_MS           (1000)
#define BLE_WORKER_INDICATE_RETRY_DELAY_MS (100)
#define BLE_WORKER_RETRY_PHY_TIMEOUT_MS    (500)

#define BLE_WORKER_LOCAL_DEV_ADDR_LEN 18 // Length of the local device address
#define BLE_WORKER_MAX_MTU_SIZE       240

#define UUID_SIZE 16

#define BLE_WORKER_BT_HCI_COMMAND_DISALLOWED 0x4E0C

#define BLE_CCCD_NOTIFICATION_ENABLED(cccd_value) ((cccd_value & 0x01) != 0)
#define BLE_CCCD_INDICATION_ENABLED(cccd_value)   ((cccd_value & 0x02) != 0)

//! Configuration bitmap for attributes
#define ATT_REC_MAINTAIN_IN_HOST BIT(0) ///< Attribute record maintained in Host
#define SEC_MODE_1_LEVEL_1       BIT(1) ///< NO Auth and No Enc
#define SEC_MODE_1_LEVEL_2       BIT(2) ///< UnAUTH with Enc
#define SEC_MODE_1_LEVEL_3       BIT(3) ///< AUTH with Enc
#define SEC_MODE_1_LEVEL_4       BIT(4) ///< AUTH LE_SC Pairing with Enc
#define ON_BR_EDR_LINK_ONLY      BIT(5) ///< BR/EDR link-only mode
#define ON_LE_LINK_ONLY          BIT(6) ///< LE link-only mode
#define VARIABLE_ATT_CHAR_VAL    BIT(7) ///< Variable characteristic value length

#ifdef BLE_DEBUG_ADVERTISE_FORCE_PUBLIC
#define BLE_SECURITY_MODE SEC_MODE_1_LEVEL_1
#else
#define BLE_SECURITY_MODE SEC_MODE_1_LEVEL_4
#endif

#define BLE_WORKER_MAINTAIN_CHARACTERISTICS

#ifdef BLE_WORKER_MAINTAIN_CHARACTERISTICS
#define RSI_BLE_ATT_CONFIG_BITMAP (BLE_SECURITY_MODE | ATT_REC_MAINTAIN_IN_HOST)
#else
#define RSI_BLE_ATT_CONFIG_BITMAP (BLE_SECURITY_MODE)
#endif

typedef struct {
    uint16_t handle;
    size_t data_size;
} BleDataHeader;

typedef struct {
    BleDataHeader header;
    uint8_t data[];
} BleDataItem;

typedef BleDataItem* BleDataItemPtr;

//===========================================================================================
///TODO:Remove this in future
static BleWorker* ble_worker_instance = NULL;
//===========================================================================================

int32_t ble_worker_write_response(uint8_t* dev_addr, uint8_t type) {
#ifdef BLE_WORKER_MAINTAIN_CHARACTERISTICS
    return rsi_ble_gatt_write_response(dev_addr, type);
#else
    UNUSED(dev_addr);
    UNUSED(type);
    return RSI_SUCCESS;
#endif
}
//===========================================================================================
static void retry_phy_timer_callback(void* ctx) {
    BleWorker* instance = ctx;
    ble_incoming_nwp_event_processor_spawn_event(
        instance->event_proc, BleIncomingNwpEventTypeDataLengthChange, 0, NULL);
}
//===========================================================================================
bool ble_worker_start_advertising(
    bool advertise_to_paired_only,
    const rsi_bt_event_le_security_keys_t* key,
    const BleAdvertiseContext* advertise) {
    rsi_ble_req_adv_t ble_adv = {0};

#ifdef BLE_DEBUG_ADVERTISE_FORCE_PUBLIC
    BLE_LOG_W("Public advertise forced!");
    advertise_to_paired_only = false;
#endif

    ble_adv.status = RSI_BLE_START_ADV;
    ///TODO: This is blocked because it doesn't work on IPhone. It just doesn't see
    ///BSB in case of direct advertise.
    // ble_adv.adv_type = advertise_to_paired_only ? DIR_CONN_LOW_DUTY_CYCLE : UNDIR_CONN;
    ble_adv.adv_type = UNDIR_CONN;

    ble_adv.adv_int_min = RSI_BLE_ADV_INT_MIN;
    ble_adv.adv_int_max = RSI_BLE_ADV_INT_MAX;
    ble_adv.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;

    rsi_ble_clear_acceptlist();
    if(advertise_to_paired_only) {
        rsi_ble_addto_acceptlist((int8_t*)key->Identity_addr, key->Identity_addr_type);
        ble_adv.filter_type = ALLOW_SCAN_REQ_ACCEPT_LIST_CONN_REQ_ACCEPT_LIST;
        ble_adv.own_addr_type = LE_RESOLVABLE_RANDOM_ADDRESS;
        memcpy(ble_adv.direct_addr, key->Identity_addr, 6);
        ble_adv.direct_addr_type = key->Identity_addr_type;
    } else {
        ble_adv.filter_type = RSI_BLE_ADV_FILTER_TYPE;
        ble_adv.own_addr_type = LE_PUBLIC_ADDRESS;
    }

    ble_advertise_refresh_data(advertise);

    sl_status_t status = rsi_ble_start_advertising_with_values(&ble_adv);

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to start advertising, error code : 0x%08lx", status);
    } else {
        BLE_LOG_I("Start advertising...");
    }

    return status == RSI_SUCCESS;
}

bool ble_worker_stop_advertising() {
    sl_status_t status;
    ///TODO: think of more reliable way of handling stop command when we are connected
    if(ble_worker_instance->connected) {
        status = rsi_ble_disconnect((int8_t*)ble_worker_instance->remote_dev_address);
        if(status != RSI_SUCCESS)
            BLE_LOG_W("Failed to disconnect, error code : 0x%08lx", status);
        else
            BLE_LOG_I("Disconnected");
    }

    status = rsi_ble_stop_advertising();

    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to stop advertising, error code : 0x%08lx", status);
        status = RSI_SUCCESS;
    } else
        BLE_LOG_I("Stop advertising...");

    return status == RSI_SUCCESS;
}

static int32_t ble_worker_thread_callback(void* context) {
    BleWorker* instance = context;
    BLE_LOG_I("Worker Thread Start");

    instance->event_loop = furi_event_loop_alloc();

    ble_transmitter_subscribe(instance->transport, instance->event_loop, context);
    ble_incoming_nwp_event_processor_subscribe(instance->event_proc, instance->event_loop);

    furi_event_loop_run(instance->event_loop);

    ble_incoming_nwp_event_processor_unsubscribe(instance->event_proc, instance->event_loop);
    ble_transmitter_unsubscribe(instance->transport, instance->event_loop);

    furi_event_loop_free(instance->event_loop);

    return 0;
}

/**
 * @fn         ble_worker_echo_app_prepare_128bit_uuid
 * @brief      this function is used to prepare the 128bit UUID
 * @param[in]  temp_service,received 128-bit service.
 * @param[out] temp_uuid,formed 128-bit service structure.
 * @return     none.
 * @section description
 * This function prepares the 128bit UUID
 */
static void
    ble_worker_prepare_128bit_uuid(const uint8_t temp_service[UUID_SIZE], uuid_t* temp_uuid) {
    temp_uuid->val.val128.data1 =
        ((temp_service[0] << 24) | (temp_service[1] << 16) | (temp_service[2] << 8) |
         (temp_service[3]));
    temp_uuid->val.val128.data2 = ((temp_service[5]) | (temp_service[4] << 8));
    temp_uuid->val.val128.data3 = ((temp_service[7]) | (temp_service[6] << 8));
    temp_uuid->val.val128.data4[0] = temp_service[9];
    temp_uuid->val.val128.data4[1] = temp_service[8];
    temp_uuid->val.val128.data4[2] = temp_service[11];
    temp_uuid->val.val128.data4[3] = temp_service[10];
    temp_uuid->val.val128.data4[4] = temp_service[15];
    temp_uuid->val.val128.data4[5] = temp_service[14];
    temp_uuid->val.val128.data4[6] = temp_service[13];
    temp_uuid->val.val128.data4[7] = temp_service[12];
}

/**
 * @fn         ble_worker_echo_app_add_char_serv_att
 * @brief      this function is used to add characteristic service attribute..
 * @param[in]  serv_handler, service handler.
 * @param[in]  handle, characteristic service attribute handle.
 * @param[in]  val_prop, characteristic value property.
 * @param[in]  att_val_handle, characteristic value handle
 * @param[in]  att_val_uuid, characteristic value uuid
 * @return     none.
 * @section description
 * This function is used at application to add characteristic attribute
 */
static uint16_t ble_worker_add_char_serv_att(
    void* serv_handler,
    uint16_t handle,
    uint8_t val_prop,
    uint16_t att_val_handle,
    uuid_t att_val_uuid) {
    rsi_ble_req_add_att_t new_att = {0};

    //! preparing the attribute service structure
    new_att.serv_handler = serv_handler;
    new_att.handle = handle;
    new_att.att_uuid.size = 2;
    new_att.att_uuid.val.val16 = RSI_BLE_CHAR_SERV_UUID;
    new_att.property = RSI_BLE_ATT_PROPERTY_READ;

    //! preparing the characteristic attribute value
    if(att_val_uuid.size == UUID_SIZE) {
        new_att.data_len = 4 + att_val_uuid.size;
        new_att.data[0] = val_prop;
        rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
        memcpy(&new_att.data[4], &att_val_uuid.val.val128, sizeof(att_val_uuid.val.val128));
    } else {
        new_att.data_len = 6;
        rsi_uint16_to_2bytes(&new_att.data[2], att_val_handle);
        new_att.data[0] = val_prop;
        rsi_uint16_to_2bytes(&new_att.data[4], att_val_uuid.val.val16);
    }

    //! Add attribute to the service
    sl_status_t status = rsi_ble_add_attribute(&new_att);
    if(status != SL_STATUS_OK) {
        BLE_LOG_W("Status: %04lX", status);
    }

    return handle;
}

static uint16_t ble_worker_add_char_val_att(
    void* serv_handler,
    uint16_t handle,
    uuid_t att_type_uuid,
    uint8_t val_prop,
    const uint8_t* data,
    uint8_t data_len,
    uint8_t auth_read) {
    rsi_ble_req_add_att_t new_att = {0};

    //! preparing the attributes
    new_att.serv_handler = serv_handler;
    new_att.handle = handle;
    new_att.config_bitmap = auth_read;
    memcpy(&new_att.att_uuid, &att_type_uuid, sizeof(uuid_t));
    new_att.property = val_prop;

    //! preparing the attribute value
    new_att.data_len = RSI_MIN(sizeof(new_att.data), data_len);

    if(data != NULL) memcpy(new_att.data, data, new_att.data_len);

    //! add attribute to the service
    sl_status_t status = rsi_ble_add_attribute(&new_att);

    if(status != SL_STATUS_OK) {
        BLE_LOG_W("Status: %04lX", status);
    }

    //! check the attribute property with notification/Indication
    if((val_prop & RSI_BLE_ATT_PROPERTY_NOTIFY) || (val_prop & RSI_BLE_ATT_PROPERTY_INDICATE)) {
        //! if notification/indication property supports then we need to add client characteristic service.
        handle += 1;
        //! preparing the client characteristic attribute & values
        memset(&new_att, 0, sizeof(rsi_ble_req_add_att_t));
        new_att.serv_handler = serv_handler;
        new_att.handle = handle;
        new_att.att_uuid.size = 2;
        new_att.att_uuid.val.val16 = RSI_BLE_CLIENT_CHAR_UUID;
        new_att.property = RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_WRITE;
        new_att.data_len = 2;
        new_att.config_bitmap = auth_read;

        //! add attribute to the service
        int32_t ret = rsi_ble_add_attribute(&new_att);
        BLE_LOG_D("Add CCCD handle: %04X, Ret: %lX", handle, ret);
        UNUSED(ret);
    }
    return handle;
}

static void ble_prepare_uuid(const Char_UUID_t* temp, const uint8_t size, uuid_t* uuid) {
    uuid->size = size;
    if(size == 2)
        uuid->val.val16 = temp->Char_UUID_16;
    else if(size == 16)
        ble_worker_prepare_128bit_uuid(temp->Char_UUID_128, uuid);
}

BleWorker* ble_worker_init(BleConnectionStateChanged connect_callback, void* ctx) {
    furi_assert(connect_callback);
    furi_assert(ctx);

    BleWorker* instance = malloc(sizeof(BleWorker));
    instance->state = BleWorkerStateIdle;
    instance->thread =
        furi_thread_alloc_ex("BleWorker", 3072U, ble_worker_thread_callback, instance);

    instance->on_connection_changed_cb = connect_callback;
    instance->on_connection_changed_ctx = ctx;
    instance->receive_sem = furi_semaphore_alloc(1, 1);
    instance->max_payload_size = BLE_WORKER_MAX_MTU_SIZE - BLE_WORKER_ATTR_HEADER_SIZE;
    instance->security_data = ble_security_alloc();
    instance->advertise = ble_advertise_alloc();
    ble_advertise_set_name(instance->advertise, BLE_DEFAULT_LOCAL_NAME);

    BleServiceEntryDict_init(instance->service_dict);

    instance->retry_phy_timer =
        furi_timer_alloc(retry_phy_timer_callback, FuriTimerTypeOnce, instance);

    //----------------------------------------------------------------------------------------------------------------
    static uint8_t rsi_app_resp_get_dev_addr[RSI_DEV_ADDR_LEN] = {0};
    uint8_t local_dev_addr[BLE_WORKER_LOCAL_DEV_ADDR_LEN] = {0};

    instance->pairing_info_available = ble_security_init(instance->security_data);

    sl_status_t status = rsi_bt_get_local_device_address(rsi_app_resp_get_dev_addr);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Get local device address failed = 0x%08lx", status);
        furi_crash();
    } else {
        rsi_6byte_dev_address_to_ascii(local_dev_addr, rsi_app_resp_get_dev_addr);
        BLE_LOG_I("Local device address %s", local_dev_addr);
    }

    instance->event_proc = ble_incoming_nwp_event_processor_alloc(instance);
    instance->transport = ble_transmitter_alloc();
    ble_nwp_core_config_callbacks(instance->event_proc, instance->transport);
    //----------------------------------------------------------------------------------------------------------------
    //! Set local name
    status = rsi_bt_set_local_name((const uint8_t*)BLE_DEFAULT_LOCAL_NAME);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set default local name, error code : 0x%08lx", status);
    }

    status = rsi_ble_set_random_address_with_value(rsi_app_resp_get_dev_addr);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set address: %08lX", status);
    }
    //----------------------------------------------------------------------------------------------------------------
    ble_advertise_print_data(instance->advertise);

    //Appearance adjustment
    uuid_t uuid = {0};
    uuid.size = 2;
    uuid.val.val16 = 0x2A01;
    uint16_t value_handle = 0;
    if(ble_find_characteristic_value_handle_by_uuid(&uuid, 0x001E, &value_handle)) {
        uint16_t data = 0x00C0;
        BLE_LOG_D("Handle found: %04X", value_handle);
        sl_status_t status = rsi_ble_set_local_att_value(value_handle, 2, (uint8_t*)&data);
        UNUSED(status);
        BLE_LOG_D("Status: %lX", status);
    }
    //---------------------------------------
    ///TODO:Remove this in future
    ble_worker_instance = instance;
    //---------------------------------------
    return instance;
}

bool ble_worker_register_service(BleServiceObject* service) {
    uuid_t rsi_uuid = {0};
    rsi_ble_resp_add_serv_t new_serv_resp = {0};

    ble_prepare_uuid(&service->config->uuid, service->config->uuid_size, &rsi_uuid);
    sl_status_t status = rsi_ble_add_service(rsi_uuid, &new_serv_resp);

    bool result = false;
    if(status == RSI_SUCCESS) {
        ///TODO: it's not very good that worker knows inner kitchen of ble_service object
        /// Possibly need to close all of this parts behind some methods
        service->service_handler = new_serv_resp.serv_handler;
        service->handle = new_serv_resp.start_handle;

        uint16_t handle = new_serv_resp.start_handle;
        BLE_LOG_D("Register service: 0x%04X", new_serv_resp.start_handle);
        for(uint8_t i = 0; i < service->config->char_count; i++) {
            BleCharacteristicObject* ch = service->chars[i];
            const BleCharacteristicDescriptor* ch_config = ble_characteristic_get_config(ch);

            memset(&rsi_uuid, 0, sizeof(uuid_t));
            ble_prepare_uuid(&ch_config->uuid, ch_config->uuid_size, &rsi_uuid);

            BLE_LOG_D("Add char %s att handle: %04X", ch_config->name, handle + 1);
            ble_worker_add_char_serv_att(
                service->service_handler,
                handle + 1,
                ch_config->char_properties,
                handle + 2,
                rsi_uuid);

            uint16_t value_handle = handle + 2;
            BLE_LOG_D("Add char %s val att handle: %04X", ch_config->name, value_handle);
            ble_characteristic_set_handle(ch, value_handle);
            handle = ble_worker_add_char_val_att(
                service->service_handler,
                value_handle,
                rsi_uuid,
                ch_config->char_properties,
                ble_characteristic_get_data(ch),
                ble_characteristic_get_data_size(ch),
                RSI_BLE_ATT_CONFIG_BITMAP);
            BleServiceEntry entry = {.service = service, .char_index = ch_config->intercom_index};
            BleServiceEntryDict_set_at(ble_worker_instance->service_dict, value_handle, entry);

            if((ch_config->char_properties & RSI_BLE_ATT_PROPERTY_NOTIFY) ||
               (ch_config->char_properties & RSI_BLE_ATT_PROPERTY_INDICATE)) {
                ble_characteristic_set_cccd_handle(ch, handle);
                BleServiceEntry entry = {
                    .service = service, .char_index = ch_config->intercom_index};
                BleServiceEntryDict_set_at(ble_worker_instance->service_dict, handle, entry);
            }
        }

        result = true;
    }

    return result;
}

void ble_worker_start() {
    do {
        if(ble_worker_instance->state != BleWorkerStateIdle) {
            BLE_LOG_W("BLE not in Idle state, skip advertise start");
            break;
        }

        const rsi_bt_event_le_security_keys_t* rpa =
            ble_security_get_rpa_data(ble_worker_instance->security_data);
        ble_worker_start_advertising(
            ble_worker_instance->pairing_info_available, rpa, ble_worker_instance->advertise);

        ble_worker_instance->state = BleWorkerStateAdvertising;
        furi_thread_start(ble_worker_instance->thread);
    } while(false);
}

void ble_worker_stop() {
    if(ble_worker_instance) {
        FuriThreadState state = furi_thread_get_state(ble_worker_instance->thread);
        if(state == FuriThreadStateRunning) {
            ble_incoming_nwp_event_processor_spawn_event(
                ble_worker_instance->event_proc, BleIncomingNwpEventTypeExit, 0, NULL);

            furi_thread_join(ble_worker_instance->thread);
            BLE_LOG_I("BLE Stopped");
        }
    }
}

///TODO: Part of device instance as ble_device_send
void ble_worker_send(uint16_t handle, uint16_t data_size, const uint8_t* data, uint16_t cccd_value) {
    size_t index = 0;
    size_t total_size = data_size;
    while(total_size) {
        size_t send_size = total_size > ble_worker_instance->max_payload_size ?
                               ble_worker_instance->max_payload_size :
                               total_size;

        bool send_result = ble_transmitter_send_chunk(
            ble_worker_instance->transport,
            ble_worker_instance->remote_dev_address,
            handle,
            send_size,
            &data[index],
            cccd_value);

        if(!send_result) {
            BLE_LOG_W("[%04X] - Tx terminated!", handle);
            break;
        }

        index += send_size;
        total_size -= send_size;
    }
}

void ble_worker_receive_confirm(uint16_t handle, uint8_t cccd_value) {
    UNUSED(handle);
    sl_status_t status;
    if(ble_worker_instance->connected && BLE_CCCD_INDICATION_ENABLED(cccd_value)) {
        status = rsi_ble_indicate_confirm(ble_worker_instance->remote_dev_address);
    } else {
        status = ble_worker_write_response(ble_worker_instance->remote_dev_address, 0);
    }

    furi_assert(handle == ble_worker_instance->rx_pending_handle);

    furi_semaphore_release(ble_worker_instance->receive_sem);
    ble_worker_instance->rx_pending_handle = 0;
    if(status != 0) BLE_LOG_W("Recv fail %08lX", status);
}

bool ble_worker_forget_pairing() {
    if(ble_worker_instance->state == BleWorkerStateAdvertising) {
        ble_worker_stop_advertising();
    }

    ble_security_rpa_disable();

    bool result = ble_security_delete_data(ble_worker_instance->security_data);

    ble_worker_instance->pairing_info_available = 0;

    if(ble_worker_instance->state == BleWorkerStateAdvertising) {
        ble_worker_start_advertising(false, NULL, ble_worker_instance->advertise);
    }

    if(result) BLE_LOG_I("Security data removed");
    return result;
}

bool ble_worker_pairing_exists() {
    return ble_security_pairing_present(ble_worker_instance->security_data);
}

void ble_worker_set_name(const char* new_name) {
    furi_assert(new_name);

    if(ble_worker_instance->state == BleWorkerStateAdvertising) {
        ble_worker_stop_advertising();
    }

    ble_advertise_set_name(ble_worker_instance->advertise, new_name);

    sl_status_t status = rsi_bt_set_local_name((const uint8_t*)new_name);
    if(status != RSI_SUCCESS) {
        BLE_LOG_W("Failed to set local name, error code : 0x%08lx", status);
    }

    if(ble_worker_instance->state == BleWorkerStateAdvertising) {
        ble_worker_start_advertising(false, NULL, ble_worker_instance->advertise);
    }
}
