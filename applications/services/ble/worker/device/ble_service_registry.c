#include "ble_service_registry.h"

// #include "../../service/ble_service_i.h"
#include "../../service/target/ble_service_target.h"
#include "../../ble_log.h"
#include "../_nwp_callbacks/ble_nwp_headers.h"

#include <m-dict.h>

#define TAG "BleRegistry"

#define UUID_SIZE 16

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

DICT_DEF2(BleServiceEntryDict, uint16_t, M_DEFAULT_OPLIST, BleServiceRegistryEntry, M_POD_OPLIST)

struct BleServiceRegistry {
    BleServiceEntryDict_t service_dict;
};

static uint16_t ble_service_registry_add_char_serv_att(
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

static uint16_t ble_service_registry_add_char_val_att(
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

static void ble_service_registry_prepare_128bit_uuid(
    const uint8_t temp_service[UUID_SIZE],
    uuid_t* temp_uuid) {
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

static void
    ble_service_registry_prepare_uuid(const Char_UUID_t* temp, const uint8_t size, uuid_t* uuid) {
    uuid->size = size;
    if(size == 2)
        uuid->val.val16 = temp->Char_UUID_16;
    else if(size == 16)
        ble_service_registry_prepare_128bit_uuid(temp->Char_UUID_128, uuid);
}

static void ble_service_registry_set_nwp_registration_info(
    BleServiceObject* instance,
    const uint16_t handle,
    void* service_handler) {
    furi_assert(instance);
    furi_assert(service_handler);
    instance->service_handler = service_handler;
    instance->handle = handle;
}

bool ble_service_registry_add_service_entry(
    BleServiceRegistry* instance,
    BleServiceObject* service) {
    uuid_t rsi_uuid = {0};
    rsi_ble_resp_add_serv_t new_serv_resp = {0};

    ble_service_registry_prepare_uuid(
        &service->config->uuid, service->config->uuid_size, &rsi_uuid);
    sl_status_t status = rsi_ble_add_service(rsi_uuid, &new_serv_resp);

    bool result = false;
    if(status == RSI_SUCCESS) {
        ble_service_registry_set_nwp_registration_info(
            service, new_serv_resp.start_handle, new_serv_resp.serv_handler);

        uint16_t handle = new_serv_resp.start_handle;
        BLE_LOG_D("Register service: 0x%04X", new_serv_resp.start_handle);
        for(uint8_t i = 0; i < service->config->char_count; i++) {
            BleCharacteristicObject* ch = service->chars[i];

            const BleCharacteristicConfig* ch_config = ble_characteristic_get_config(ch);

            memset(&rsi_uuid, 0, sizeof(uuid_t));
            ble_service_registry_prepare_uuid(&ch_config->uuid, ch_config->uuid_size, &rsi_uuid);

            BLE_LOG_D("Add char %s att handle: %04X", ch_config->name, handle + 1);
            ble_service_registry_add_char_serv_att(
                service->service_handler,
                handle + 1,
                ch_config->char_properties,
                handle + 2,
                rsi_uuid);

            uint16_t value_handle = handle + 2;
            BLE_LOG_D("Add char %s val att handle: %04X", ch_config->name, value_handle);
            ble_characteristic_set_handle(ch, value_handle);
            handle = ble_service_registry_add_char_val_att(
                service->service_handler,
                value_handle,
                rsi_uuid,
                ch_config->char_properties,
                ble_characteristic_get_data(ch),
                ble_characteristic_get_data_size(ch),
                RSI_BLE_ATT_CONFIG_BITMAP);
            BleServiceRegistryEntry entry = {
                .service = service, .char_index = ch_config->intercom_index};
            BleServiceEntryDict_set_at(instance->service_dict, value_handle, entry);

            if((ch_config->char_properties & RSI_BLE_ATT_PROPERTY_NOTIFY) ||
               (ch_config->char_properties & RSI_BLE_ATT_PROPERTY_INDICATE)) {
                ble_characteristic_set_cccd_handle(ch, handle);
                BleServiceRegistryEntry entry = {
                    .service = service, .char_index = ch_config->intercom_index};
                BleServiceEntryDict_set_at(instance->service_dict, handle, entry);
            }
        }

        result = true;
    }

    return result;
}

const BleServiceRegistryEntry*
    ble_service_registry_get_service_entry(BleServiceRegistry* instance, const uint16_t handle) {
    furi_assert(instance);

    BleServiceRegistryEntry* entry = BleServiceEntryDict_get(instance->service_dict, handle);
    return entry;
}

void ble_service_registry_reset_cccds(BleServiceRegistry* instance) {
    furi_assert(instance);

    BleServiceEntryDict_it_t entry_iter;
    for(BleServiceEntryDict_it(entry_iter, instance->service_dict);
        !BleServiceEntryDict_end_p(entry_iter);
        BleServiceEntryDict_next(entry_iter)) {
        BleServiceEntryDict_itref_t* entry_ref = BleServiceEntryDict_ref(entry_iter);

        BleServiceRegistryEntry* entry = &entry_ref->value;
        BleServiceObject* service = entry->service;
        if(ble_service_lock(service)) {
            BleCharacteristicObject* ch = service->chars[entry->char_index];
            ble_characteristic_set_cccd_value(ch, 0);
            ble_service_unlock(service);
        }
    }
}

BleServiceRegistry* ble_service_registry_alloc() {
    BleServiceRegistry* instance = malloc(sizeof(BleServiceRegistry));
    BleServiceEntryDict_init(instance->service_dict);
    return instance;
}

void ble_service_registry_free(BleServiceRegistry* instance) {
    furi_assert(instance);
    BleServiceEntryDict_clear(instance->service_dict);
    free(instance);
}
