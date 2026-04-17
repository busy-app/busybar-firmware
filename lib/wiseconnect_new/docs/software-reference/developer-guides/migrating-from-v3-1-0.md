# Migrating from WiSeConnect™ SDK v3.1.0 to v3.1.1

## Table of Contents

- [Overview](#overview)
- [Folder Changes](#folder-changes)
- [Migration Steps](#migration-steps)
  - [Update API Calls](#update-api-calls)
  - [Update Linker File](#update-linker-file)
  - [Update Types](#update-types)
  - [Update Constants](#update-constants)
  - [Update Component Usage](#update-component-usage)
  - [Update Component Names](#update-component-names)
  - [Update Macros](#update-macros)

## Overview

This is a guide for updating an existing application using the WiSeConnect™ SDK v3.1.0 to a v3.1.1 application.

This guide describes the folder changes made in the WiSeConnect SDK v3.1.0 to simplify it and make it easier to understand.

It also provides the steps for updating a v3.1.0 application to a v3.1.1 application.

There are several naming and interface changes in v3.1.1 as compared to v3.1.0,, mostly in order to standardize the names and improve the overall usage experience of the application programming interface (API). Migration requires the names everywhere to be updated in the existing code of an application.

## Folder Changes

| **Previous Location**  | **Current Location** | **Notes** |
|------------------------|----------------------|-----------|
| components/si91x/iot_socket | third_party/iot_socket | IoT Socket is a third-party library from ARM |
| components/si91x/bsd_socket | components/service/bsd_socket |  |
| components/si91x/platforms/efx32 | components/device/silabs/si91x/wireless/host_mcu/efx32 |  |
| components/si91x/platforms/siwx917_soc | components/device/silabs/si91x/wireless/host_mcu/si91x |  |
| components/si91x/services/sntp/src | components/service/sntp/si91x |  |
| components/si91x/protocols/firmware_upgrade | components/device/silabs/si91x/wireless/firmware_upgrade |  |
| components/si91x/protocols/icmp | components/device/silabs/si91x/wireless/icmp |  |
| components/si91x/socket | components/device/silabs/si91x/wireless/asynchronous_socket |  |
| components/si91x/socket_utility | components/device/silabs/si91x/wireless/socket |  |
| components/si91x_support/brd4180a | components/board/silabs/config/brd4180a |  |
| components/si91x_support/brd4180b | components/board/silabs/config/brd4180b |  |
| components/si91x_support/efm32gg11b | N/A | Removed. |
| components/si91x_support/efr32mg21 | N/A | Removed. |
| components/si91x_support/inc | components/common/inc |  |
| components/si91x_support/src | components/common/src |  |
| components/siwx917_soc/boards | components/board/silabs |  |
| components/siwx917_soc/core | components/device/silabs/si91x/mcu/core |  |
| components/siwx917_soc/drivers | components/device/silabs/si91x/mcu/drivers |  |
| components/siwx917_soc/hal | components/device/silabs/si91x/mcu/hal |  |
| components/siwx917_soc/platform_connectivity | components/device/silabs/si91x/wireless/ahb_interface |  |
| components/siwx917_soc/toolchain | components/device/silabs/si91x/mcu/toolchain |  |
| components/sl_tls | N/A | Removed. API renamed and recategorised. |
| components/service/http_client/si91x | components/service/http_client/si91x_socket |  |

## Migration Steps

In order to convert a WiSeConnect SDK v3.1.0 application to a v3.1.1 application,

1. Open your existing application project in Simplicity Studio.

2. In each source file of the project, replace the v3.1.0 names or interfaces with v3.1.1 names or interfaces. 

Refer to the tables in each of the sections that follow which map the v3.1.0 API elements to v3.1.1. In some instances, the differences between v3.1.0 and v3.1.1 are highlighted in **bold** text.
- [Update API Calls](#update-api-calls)
- [Update Linker File](#update-linker-file)
- [Update Types](#update-types)
- [Update Constants](#update-constants)
- [Update Component Usage](#update-component-usage)
- [Update Component Names](#update-component-names)
- [Update Macros](#update-macros)

### Update API Calls

| **Module**   | **v3.1.0** | **v3.1.1** |
|--------------|------------|------------|
| Wi-Fi | sl_status_t sl_wifi_init(const sl_wifi_device_configuration_t \*configuration, sl_wifi_event_handler_t event_handler) | sl_status_t sl_wifi_init(const sl_wifi_device_configuration_t \*configuration, **sl\_wifi\_device\_context\_t \*device\_context,** sl_wifi_event_handler_t event_handler) |
| Wi-Fi | sl_status_t **default\_wifi\_event\_handler**(sl_wifi_event_t event, sl_wifi_buffer_t \*buffer)  | sl_status_t **sl\_wifi\_default\_event\_handler**(sl_wifi_event_t event, sl_wifi_buffer_t \*buffer) |
| Wi-Fi | sl_status_t sl_wifi_get_firmware_version(**sl\_wifi\_version\_string\_t** \*version) | sl_status_t sl_wifi_get_firmware_version(**sl\_wifi\_firmware\_version\_t** \*version) |
| Wi-Fi | sl_status_t sl_wifi_get_max_tx_power(sl_wifi_interface_t interface, **uint8\_t** \*max_tx_power); | sl_status_t sl_wifi_get_max_tx_power(sl_wifi_interface_t interface, **sl\_wifi\_max\_tx\_power\_t** \*max_tx_power) |
| Wi-Fi | sl_status_t sl_wifi_get_ap_client_info(sl_wifi_interface_t interface, **sl\_wifi\_client\_info\_response** \*client_info); | sl_status_t sl_wifi_get_ap_client_info(sl_wifi_interface_t interface, **sl\_wifi\_client\_info\_response\_t** \*client_info) |
| BLE | rsi_bt_ber_enable_or_disable() | N/A (removed from SDK) |
| BLE | rsi_bt_per_cw_mode() | N/A (removed from SDK) |
| BLE | rsi_ble_update_directed_address() | N/A (removed from SDK) |
| BLE | rsi_bt_set_antenna_tx_power_level() | N/A (removed from SDK) |
| BLE | rsi_ble_vendor_rf_type() | N/A (removed from SDK) |
| BLE | rsi_ble_write_rf_path_compensation() | N/A (removed from SDK) |
| BLE | rsi_ble_read_rf_path_compensation() | N/A (removed from SDK) |
| SiWx91x Device Management | sl_status_t sl_net_init(sl_net_interface_t interface, const void \*configuration, void \*context, sl_net_event_handler_t event_handler) | sl_status_t sl_net_init(sl_net_interface_t interface, const void \*configuration, **void \*network\_context,** sl_net_event_handler_t event_handler) |
| SiWx91x Device Management | sl_status_t sl_net_deinit(sl_net_interface_t interface, **void \*context**) | sl_status_t sl_net_deinit(sl_net_interface_t interface) |
| SiWx91x Device Management | sl_status_t sl_net_delete_credential(sl_net_credential_id_t id) | sl_status_t sl_net_delete_credential(sl_net_credential_id_t id, **sl\_net\_credential\_type\_t type**) |
| SiWx91x Device Management | sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface, **void \*context**) | sl_status_t sl_net_wifi_client_deinit(sl_net_interface_t interface) |
| SiWx91x Device Management | sl_status_t sl_net_wifi_ap_deinit(sl_net_interface_t interface, **void \*context**) | sl_status_t sl_net_wifi_ap_deinit(sl_net_interface_t interface) |
| SiWx91x Device Management | sl_si91x_register_callback() | N/A (removed from SDK) |
| SiWx91x Device Management | sl_status_t **sl\_dns\_host\_get\_by\_name**(const char \*host_name, const uint32_t timeout, const sl_net_dns_resolution_ip_type_t dns_resolution_ip, sl_ip_address_t \*ip_address) | sl_status_t **sl\_net\_host\_get\_by\_name**(const char \*host_name, const uint32_t timeout, const sl_net_dns_resolution_ip_type_t dns_resolution_ip, sl_ip_address_t \*ip_address) |
| Crypto | psa_status_t sli_si91x_psa_ecdh_key_agreement(psa_algorithm_t alg, const psa_key_attributes_t \*attributes, uint8_t \*private_key, uint32_t private_key_len, uint8_t \*peer_key, uint32_t peer_key_length, uint8_t \*shared_secret, uint32_t shared_secret_size, **uint32\_t** shared_secret_length) | psa_status_t sli_si91x_psa_ecdh_key_agreement(psa_algorithm_t alg, const psa_key_attributes_t \*attributes, uint8_t \*private_key, uint32_t private_key_len, uint8_t \*peer_key, uint32_t peer_key_length, uint8_t \*shared_secret, uint32_t shared_secret_size, **uint32\_t \***shared_secret_length) |
| Crypto | sl_status_t **sl\_si91x\_psa\_trng\_init**() | psa_status_t **sli\_si91x\_crypto\_trng\_init**() |
| SNTP | sl_status_t sl_sntp_client_get_server_info(**uint8\_t** \*data, uint16_t data_length, uint32_t timeout) | sl_status_t sl_sntp_client_get_server_info(**sl\_sntp\_server\_info\_t** \*data, uint32_t timeout) |
| HTTP | sl_status_t sl_http_client_request_init(sl_http_client_request_t \*request, **sl\_http\_client\_event\_handler** event_handler, void \*request_context) | sl_status_t sl_http_client_request_init(sl_http_client_request_t \*request, **sl\_http\_client\_event\_handler\_t** event_handler, void \*request_context) |
| HTTP | sl_status_t sl_http_client_register_callback(sl_http_client_event_t event, sl_http_client_t client_handle, **sl\_http\_client\_event\_handler** function) | sl_status_t sl_http_client_register_callback(sl_http_client_event_t event, sl_http_client_t client_handle, **sl\_http\_client\_event\_handler\_t** function) |
| MQTT | sl_status_t sl_mqtt_client_init(sl_mqtt_client_t \*client, **sl\_mqtt\_client\_event\_handler** event_handler) | sl_status_t sl_mqtt_client_init(sl_mqtt_client_t \*client, **sl\_mqtt\_client\_event\_handler\_t** event_handler) |
| MQTT | sl_status_t sl_mqtt_client_subscribe(sl_mqtt_client_t \*client, const uint8_t \*topic, uint16_t topic_length, sl_mqtt_qos_t qos_level, uint32_t timeout, **sl\_mqtt\_client\_message\_received** message_handler, void \*context) | sl_status_t sl_mqtt_client_subscribe(sl_mqtt_client_t \*client, const uint8_t \*topic, uint16_t topic_length, sl_mqtt_qos_t qos_level, uint32_t timeout, **sl\_mqtt\_client\_message\_received\_t** message_handler, void \*context) |

### Update Linker File 
Support for new flash map changes with 'TA-1.8MB + MCU-2MB' configuration

| **Module** | **v3.1.0** | **v3.1.1** |
|------------|------------|------------|
| Flash start address in Linker file | 0x81c2000	| 0x8202000 |
| IPMU offset	| 0x81b0258	| 0x81f0258 |
| IVT offset	| 0x81c2000	| 0x8202000 |
| M4 flash size	| 2172KB	| 2044KB |


### Update Types

| **Module** | **v3.1.0** | **v3.1.1** |
|------------|------------|------------|
| Wi-Fi | sl_wifi_context_t | N/A (removed from SDK) |
| Wi-Fi | sl_wifi_version_string_t | N/A (removed from SDK) |
| Wi-Fi | sl_wifi_client_info | sl_wifi_client_info_t |
| Wi-Fi | sl_wifi_client_info_response | sl_wifi_client_info_response_t |
| Wi-Fi | sl_wifi_advanced_client_configuration_t<br> | Addition of **auth\_assoc\_timeout**, **keep\_alive\_timeout** member |
| Wi-Fi | sl_wifi_eap_credential_t<br> | Addition of **certificate\_key** member |
| BLE | typedef struct rsi_ble_mtu_exchange_s { uint8_t dev_addr[RSI_DEV_ADDR_LEN]; **uint8\_t** req_mtu_size; } rsi_ble_mtu_exchange_t; | typedef struct rsi_ble_mtu_exchange_s { uint8_t dev_addr[RSI_DEV_ADDR_LEN]; **uint16\_t** req_mtu_size; } rsi_ble_mtu_exchange_t; |
| BLE | typedef struct rsi_ble_mtu_exchange_resp_s { uint8_t dev_addr[RSI_DEV_ADDR_LEN]; **uint8\_t** req_mtu_size; } rsi_ble_mtu_exchange_resp_t; | typedef struct rsi_ble_mtu_exchange_resp_s { uint8_t dev_addr[RSI_DEV_ADDR_LEN]; **uint16\_t** req_mtu_size; } rsi_ble_mtu_exchange_resp_t; |
| BLE | typedef struct rsi_ble_prepare_write_resp_s { uint8_t dev_addr[RSI_DEV_ADDR_LEN]; uint8_t handle[2]; uint8_t offset[2]; **uint16\_t** length; uint8_t att_value[RSI_DEV_ATT_LEN]; } rsi_ble_prepare_write_resp_t; | typedef struct rsi_ble_prepare_write_resp_s { uint8_t dev_addr[RSI_DEV_ADDR_LEN];  uint8_t handle[2]; uint8_t offset[2]; **uint8\_t** length; uint8_t att_value[RSI_DEV_ATT_LEN]; } rsi_ble_prepare_write_resp_t; |
| BLE | rsi_ble_set_prop_protocol_ble_bandedge_tx_power_t<br><br> | N/A (removed from SDK) |
| SiWx91x Device Management | typedef struct {**ta\_m4\_commands\_t** sub_cmd;  uint8_t input_data_size;  uint8_t input_data[];<br>} **ta\_m4\_handshake\_parameters\_t**; | typedef struct { **sl\_si91x\_ta\_m4\_commands\_t** sub_cmd;  uint8_t input_data_size;  uint8_t input_data[];<br>} **sl\_si91x\_ta\_m4\_handshake\_parameters\_t**; |
| SiWx91x Device Management | sl_si91x_con_state_notificaton_t; | sl_si91x_module_state_stats_response_t |
| SiWx91x Device Management | sl_performance_profile_t | sl_si91x_performance_profile_t |
| SiWx91x Device Management | sl_wifi_device_configuration_t | Addition of **sl\_si91x\_dynamic\_pool**  member |
| SiWx91x Device Management | sl_wifi_performance_profile_t | Addition of **monitor\_interval** member |
| SiWx91x Device Management | typedef sl_status_t (\*sl_net_event_handler_t)(**uint32\_t** event, sl_status_t status, void \*data, uint32_t data_length); | typedef sl_status_t (\*sl_net_event_handler_t)(**sl\_net\_event\_t event**, sl_status_t status, void \*data, uint32_t data_length) |
| SiWx91x Device Management | ta_m4_commands_t | sl_si91x_ta_m4_commands_t |
| SiWx91x Device Management | N/A | sl_si91x_wifi_vap_id_t (new type) |
| SNTP | typedef struct { uint8_t \*server_host_name; uint8_t sntp_method; uint16_t sntp_timeout; uint8_t flags; sl_sntp_client_event_handler_t event_handler;  sl_sntp_set_time_sync_notification_handler_t<br>    time_sync_notifiication_handler; bool smooth_sync; bool server_from_dhcp; bool **renew\_servers\_after\_new\_IP**;  } sl_sntp_client_config_t; | typedef struct { uint8_t \*server_host_name; uint8_t sntp_method; uint16_t sntp_timeout; uint8_t flags; sl_sntp_client_event_handler_t event_handler; sl_sntp_set_time_sync_notification_handler_t<br>    time_sync_notifiication_handler; bool smooth_sync; bool server_from_dhcp; bool **renew\_servers\_after\_new\_ip**; } sl_sntp_client_config_t; |
| HTTP | sl_http_client_configuration_t | Addition of **https\_use\_sni** member |
| HTTP | sl_http_client_request_t<br> | Addition of  **si91x\_socket\_type\_length\_value\_t** member |
| HTTP | typedef struct sl_http_client_header_s { **struct sl\_http\_client\_header\_s** \*next;  char \*key; char \*value; } sl_http_client_header_t; | typedef struct { **sl\_http\_client\_header\_t** \*next; char \*key; char \*value; } sl_http_client_header_t; |
| MQTT | sl_mqtt_client_event_handler | sl_mqtt_client_event_handler_t |
| MQTT | sl_mqtt_client_message_received | sl_mqtt_client_message_received_t |
| MQTT | sl_mqtt_broker_t<br> | Addition of **keep\_alive\_retries** member |

### Update Constants

| **Module** | **v3.1.0** | **v3.1.1** |
|------------|------------|------------|
| Wi-Fi | CHECK_IF_EVENT_FAILED  | SL_WIFI_CHECK_IF_EVENT_FAILED |
| Wi-Fi | SL_MAX_SCANNED_AP  | SL_WIFI_MAX_SCANNED_AP |
| Wi-Fi | SL_MAX_CLIENT_COUNT  | SL_WIFI_MAX_CLIENT_COUNT |
| Wi-Fi | SL_WEP_KEY_LENGTH | SL_WIFI_WEP_KEY_LENGTH |
| Wi-Fi | SL_WEP_KEY_COUNT | SL_WIFI_WEP_KEY_COUNT |
| Wi-Fi | SL_EAP_USER_NAME_LENGTH | SL_WIFI_EAP_USER_NAME_LENGTH |
| Wi-Fi | SL_EAP_PASSWORD_LENGTH | SL_WIFI_EAP_PASSWORD_LENGTH |
| Wi-Fi | SL_EAP_CERTIFICATE_KEY_LENGTH | SL_WIFI_EAP_CERTIFICATE_KEY_LENGTH |
| Wi-Fi | SI91X_LOW_TRANSMIT_POWER_THRESHOLD | N/A (Moved to internal files for internal use ONLY) |
| Wi-Fi | SI91X_MEDIUM_TRANSMIT_POWER_THRESHOLD | N/A (Moved to internal files for internal use ONLY) |
| Wi-Fi | ARGS_CHECK_NULL_POINTER | SL_WIFI_ARGS_CHECK_NULL_POINTER |
| Wi-Fi | ARGS_CHECK_INVALID_INTERFACE | SL_WIFI_ARGS_CHECK_INVALID_INTERFACE |
| Wi-Fi | sl_wifi_operational_mode_t<br> | N/A (removed from SDK) |
| Wi-Fi | typedef enum { **SUSPEND\_INDEFINITELY**, **SUSPEND\_FOR\_DURATION**, **RESUME\_IMMEDIATELY** } sl_wifi_reschedule_twt_action_t; | typedef enum { **SL\_WIFI\_SUSPEND\_INDEFINITELY**, **SL\_WIFI\_SUSPEND\_FOR\_DURATION**, **SL\_WIFI\_RESUME\_IMMEDIATELY** } sl_wifi_reschedule_twt_action_t; |
| Wi-Fi | typedef enum { **SL\_WIFI\_CRED\_PSK** = 0, **SL\_WIFI\_CRED\_PMK**, **SL\_WIFI\_CRED\_WEP**, **SL\_WIFI\_CRED\_EAP** } sl_wifi_credential_type_t; | typedef enum { **SL\_WIFI\_PSK\_CREDENTIAL** = 0, **SL\_WIFI\_PMK\_CREDENTIAL**, **SL\_WIFI\_WEP\_CREDENTIAL**, **SL\_WIFI\_EAP\_CREDENTIAL** } sl_wifi_credential_type_t; |
| SiWx91x Device Management | RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM | SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM |
| SiWx91x Device Management | RAM_LEVEL_NWP_ADV_MCU_BASIC | SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC |
| SiWx91x Device Management | RAM_LEVEL_NWP_BASIC_MCU_ADV | SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV |
| SiWx91x Device Management | RAM_LEVEL_NWP_ALL_MCU_ZERO | SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO |
| SiWx91x Device Management | SI91X_JOIN_FEAT_STA_BG_ONLY_MODE_ENABLE | SL_SI91X_JOIN_FEAT_STA_BG_ONLY_MODE_ENABLE |
| SiWx91x Device Management | SI91X_JOIN_FEAT_LISTEN_INTERVAL_VALID | SL_SI91X_JOIN_FEAT_LISTEN_INTERVAL_VALID |
| SiWx91x Device Management | SI91X_JOIN_FEAT_QUICK_JOIN | SL_SI91X_JOIN_FEAT_QUICK_JOIN |
| SiWx91x Device Management | SI91X_JOIN_FEAT_CCXV2_FEATURE | SL_SI91X_JOIN_FEAT_CCXV2_FEATURE |
| SiWx91x Device Management | SI91X_JOIN_FEAT_BSSID_BASED | SL_SI91X_JOIN_FEAT_BSSID_BASED |
| SiWx91x Device Management | SI91X_JOIN_FEAT_MFP_CAPABLE_ONLY | SL_SI91X_JOIN_FEAT_MFP_CAPABLE_ONLY |
| SiWx91x Device Management | SI91X_JOIN_FEAT_MFP_CAPABLE_REQUIRED | SL_SI91X_JOIN_FEAT_MFP_CAPABLE_REQUIRED |
| SiWx91x Device Management | SI91X_JOIN_FEAT_PS_CMD_LISTEN_INTERVAL_VALID | SL_SI91X_JOIN_FEAT_PS_CMD_LISTEN_INTERVAL_VALID |
| SiWx91x Device Management | SSL_HIGH_STREAMING_BIT | SL_SI91X_FEAT_SSL_HIGH_STREAMING_BIT |
| SiWx91x Device Management | FEAT_SECURE_ATTESTATION | SL_SI91X_FEAT_SECURE_ATTESTATION |
| SiWx91x Device Management | SL_SI91X_FEAT_CUSTOM_FEAT_EXTENTION_VALID | SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID |
| SiWx91x Device Management | CHIP_917 | SLI_SI917 |
| SiWx91x Device Management | SI91X_TAKE_M4_64K | SL_SI91X_TAKE_M4_64K |
| SiWx91x Device Management | SI91X_GET_IPMU_PROGRAMMING_VALUES | SL_SI91X_GET_IPMU_PROGRAMMING_VALUES |
| SiWx91x Device Management | SI91X_READ_TA_REGISTER | SL_SI91X_READ_TA_REGISTER |
| SiWx91x Device Management | SI91X_WRITE_TA_REGISTER | SL_SI91X_WRITE_TA_REGISTER |
| SiWx91x Device Management | SI91X_ENABLE_XTAL | SL_SI91X_ENABLE_XTAL |
| SiWx91x Device Management | SI91X_WRITE_TO_COMMON_FLASH | SL_SI91X_WRITE_TO_COMMON_FLASH |
| SiWx91x Device Management | typedef enum { **SI91X\_AP\_KEEP\_ALIVE\_DISABLE** = 0, **SI91X\_AP\_DEAUTH\_BASED\_KEEP\_ALIVE** = 1,  **SI91X\_AP\_NULL\_BASED\_KEEP\_ALIVE** = 3 } **si91x\_ap\_keepalive\_type\_t**; | typedef enum { **SL\_SI91X\_AP\_KEEP\_ALIVE\_DISABLE** = 0, **SL\_SI91X\_AP\_DEAUTH\_BASED\_KEEP\_ALIVE** = 1, **SL\_SI91X\_AP\_NULL\_BASED\_KEEP\_ALIVE** = 3  } **sl\_si91x\_ap\_keepalive\_type\_t**; |
| SiWx91x Device Management | typedef enum {  SL_NET_INVALID_CREDENTIAL_TYPE, SL_NET_WIFI_PSK, SL_NET_WIFI_PMK,  SL_NET_WIFI_WEP, **SL\_NET\_WIFI\_EAP**, SL_NET_WIFI_PEAP, **SL\_NET\_MQTT\_CLIENT\_CREDENTIALS, SL\_NET\_HTTP\_CLIENT\_CREDENTIALS, SL\_NET\_TLS\_SERVER\_CERTIFICATE, SL\_NET\_TLS\_SERVER\_KEY,   SL\_NET\_TLS\_CLIENT\_CERTIFICATE,  SL\_NET\_TLS\_CLIENT\_KEY, SL\_NET\_TLS\_ROOT\_CERTIFICATE**,  } sl_net_credential_type_t; | typedef enum { SL_NET_INVALID_CREDENTIAL_TYPE, SL_NET_WIFI_PSK, SL_NET_WIFI_PMK,  SL_NET_WIFI_WEP,  **SL\_NET\_CERTIFICATE, SL\_NET\_PUBLIC\_KEY, SL\_NET\_PRIVATE\_KEY, SL\_NET\_SIGNING\_CERTIFICATE, SL\_NET\_HTTP\_CLIENT\_CREDENTIAL, SL\_NET\_EAP\_CLIENT\_CREDENTIAL, SL\_NET\_MQTT\_CLIENT\_CREDENTIAL** } sl_net_credential_type_t;  (SL_NET_XXX credentials are TLS credentials) |
| SiWx91x Device Management | typedef enum { SL_NET_INVALID_CREDENTIAL_ID = 0,  SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID, SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID, **SL\_NET\_MQTT\_SERVER\_CERTIFICATE\_ID**, **SL\_NET\_MQTT\_SERVER\_KEY\_ID**, **SL\_NET\_MQTT\_CLIENT\_CREDENTIALS\_ID**, **SL\_NET\_HTTP\_CLIENT\_CREDENTIAL\_ID**, **SL\_NET\_DEFAULT\_PMK\_CREDENTIAL\_ID**, **SL\_NET\_USER\_CREDENTIAL\_ID** = (int)(1u \<\< 7), } sl_net_credential_id_t; | typedef enum { SL_NET_INVALID_CREDENTIAL_ID  = 0, SL_NET_DEFAULT_WIFI_CLIENT_CREDENTIAL_ID = 1, SL_NET_DEFAULT_WIFI_AP_CREDENTIAL_ID     = 2, **SL\_NET\_WIFI\_EAP\_CLIENT\_CREDENTIAL\_ID** = 3, **SL\_NET\_WIFI\_EAP\_SERVER\_CREDENTIAL\_ID** = 4, **SL\_NET\_USER\_CREDENTIAL\_ID**                = 5, **SL\_NET\_TLS\_CLIENT\_CREDENTIAL\_START** = (1 \<\< 8), **SL\_NET\_TLS\_SERVER\_CREDENTIAL\_START** = (2 \<\< 8), **SL\_NET\_MQTT\_SERVER\_CREDENTIAL\_START** = (3 \<\< 8), **SL\_NET\_MQTT\_CLIENT\_CREDENTIAL\_START** = (4 \<\< 8), **SL\_NET\_HTTP\_SERVER\_CREDENTIAL\_START** = (5 \<\< 8), **SL\_NET\_HTTP\_CLIENT\_CREDENTIAL\_START** = (6 \<\< 8) } sl_net_credential_id_t; |
| SiWx91x Device Management | typedef enum {  SL_NET_INTERFACE_UP_EVENT, SL_NET_INTERFACE_DOWN_EVENT, **SL\_NET\_IP\_UP\_EVENT**, **SL\_NET\_IP\_DOWN\_EVENT**, **SL\_NET\_IP\_EXPIRED\_EVENT**, **SL\_NET\_PING\_RESPONSE\_EVENT**, **SL\_NET\_WIFI\_CLIENT\_CONNECTED\_EVENT**, **SL\_NET\_WIFI\_CLIENT\_DISCONNECTED\_EVENT**, SL_NET_DNS_RESOLVE_EVENT, SL_NET_OTA_FW_UPDATE_EVENT, **SL\_NET\_ASYNC\_MODULE\_STATUS\_EVENT**, SL_NET_EVENT_COUNT } sl_net_event_t; | typedef enum {<br>  **SL\_NET\_PING\_RESPONSE\_EVENT**, ///\< Ping Response Event<br>  SL_NET_DNS_RESOLVE_EVENT,   ///\< DNS Address resolution Event<br>  SL_NET_OTA_FW_UPDATE_EVENT, ///\< OTA Firmware Update Event<br>  SL_NET_EVENT_COUNT          ///\< Maximum event count<br>} sl_net_event_t; |
| HTTP | SL_MAX_HTTP_CLIENT_WRITE_BUFFER_LENGTH   | SL_HTTP_CLIENT_MAX_WRITE_BUFFER_LENGTH |
| Status Codes | SL_STATUS_SI91X_SSL_TLS_MAX_ SOCKETS_REACHED_OR_FTP_CLIENT_NOT_CONNECTED | This has been split into two status codes: **SL\_STATUS\_SI91X\_SSL\_TLS\_MAX\_SOCKETS\_REACHED** and **SL\_STATUS\_SI91X\_FTP\_CLIENT\_NOT\_CONNECTED** |

### Update Component Usage

For the following components, usage in the following form (where `\<component\>' is as mentioned in the table below):

```shell
requires:
- name: "<component>"
```

has been replaced with usage in the form:

```shell
component:
- id: "<component>"
```

| **Module** | **\<component\>** 
|------------|-----------------------
| SiWx91x MCU | sleeptimer
| SiWx91x MCU | nvm3_lib

### Update Component Names

| **v3.1.0** | **v3.1.1** | **Notes** |
|------------|------------|-----------|
| si91x | sl_si91x_wireless | Renamed to accurately reflect the component's usage. |
| si91x_basic_buffers | sl_si91x_basic_buffers |  |
| si91x_core | N/A | Removed |
| si91x_internal_stack | sl_si91x_internal_stack |  |
| si91x_lwip_stack | sl_si91x_lwip_stack |  |
| socket_utility | sl_s91x_socket |  |
| socket | sl_si91x_asynchronous_socket |  |
| N/A | bsd_socket_api | Split BSD socket headers from implementation |
| basic_network_manager | basic_network_config_manager |  |
| nvm3_network_manager | nvm3_network_config_manager |  |
| siwx917_soc | sl_si91x_mcu | Renamed to accurately reflect the component's usage. |
| N/A | wiseconnect3_common | New component for WiSeConnect 3 SDK common libraries |
| N/A | ulp_platform | New component for ultra low power examples |

### Update Macros

| **v3.1.0** | **v3.1.1** |
|----------------|----------------|
| ALARM_TIMER_BASED_WAKEUP=1 |  SL_SI91X_MCU_ALARM_BASED_WAKEUP |
| BUTTON_BASED_WAKEUP=1 |  SL_SI91X_MCU_BUTTON_BASED_WAKEUP |
| WIRELESS_BASED_WAKEUP_TO_USE=1 |  SL_SI91X_MCU_WIRELESS_BASED_WAKEUP |
| INTERRUPT_BASED_DATA_RX=1 |  SLI_SI91X_MCU_INTR_BASED_RX_ON_UART |
| HIGH_EFFICIENCY_PARAMS_SUPPORT=1 |  SLI_SI91X_CONFIG_WIFI6_PARAMS |
| M4_OTAF_DF=1 | SLI_SI91X_MCU_FW_UPGRADE_OTA_DUAL_FLASH |
| RSI_DEBUG_PRINTS=1 | SL_SI91X_PRINT_DBG_LOG |
| BLE_ENABLE=1 | SLI_SI91X_ENABLE_BLE |
| RSI_LITTLE_ENDIAN=1 | SL_SI91X_ENABLE_LITTLE_ENDIAN |
| COMMON_FLASH_EN=1 | SLI_SI91X_MCU_COMMON_FLASH_MODE |
| CHIP_917=1 |  SLI_SI917 |
| CHIP_917B0=1 |  SLI_SI917B0 |
| SI917_RADIO_BOARD=1 | SLI_SI91X_MCU_CONFIG_RADIO_BOARD_BASE_VER |
| SI917_RADIO_BOARD_V2=1 | SLI_SI91X_MCU_CONFIG_RADIO_BOARD_VER2 |
| RAM_EXECUTION=1 | SLI_SI91X_MCU_ENABLE_RAM_BASED_EXECUTION |
| PSRAM_IN_USE=1 | SLI_SI91X_MCU_ENABLE_PSRAM_FEATURE |
| EXTERNAL_LDO_FOR_PSRAM=1 | SLI_SI91X_MCU_EXTERNAL_LDO_FOR_PSRAM |
| ENABLE_IPMU_APIS=1 |  SLI_SI91X_MCU_ENABLE_IPMU_APIS |
| MEM_CONFIG_917=1 |  SL_SI91X_SI917_RAM_MEM_CONFIG |
| RSI_M4_INTERFACE=1 |  SLI_SI91X_MCU_INTERFACE |
| MQTT_CLIENT_FEATURE=1 |  SLI_SI91X_EMBEDDED_MQTT_CLIENT |
| ipv6_FEATURE_REQUIRED=1 |  SLI_SI91X_ENABLE_IPV6 |
| SI91X_SOCKET_FEATURE=1 |  SLI_SI91X_SOCKETS |
| si91x_internal_stack_FEATURE_REQUIRED=1 |  SLI_SI91X_OFFLOAD_NETWORK_STACK |
| SI91X_NETWORK_OFFLOAD_ENABLED=1 | SLI_SI91X_OFFLOAD_NETWORK_STACK |
| wifi_FEATURE_REQUIRED=1 | SL_WIFI_COMPONENT_INCLUDED |
| SL_WIFI_COMPONENT_INCLUDED=1 |  SL_WIFI_COMPONENT_INCLUDED |
| RSI_WITH_OS=1 | SLI_SI91X_ENABLE_OS |
| lwip_FEATURE_REQUIRED |  SLI_SI91X_LWIP_HOSTED_NETWORK_STACK |
| si91x_lwip_stack_FEATURE_REQUIRED | SLI_SI91X_LWIP_HOSTED_NETWORK_STACK |
| ethernet_FEATURE_REQUIRED |  SLI_SI91X_LWIP_NETWORK_INTERFACE |
| SNTP_CLIENT_FEATURE | SLI_SI91X_INTERNAL_SNTP_CLIENT |
| HTTP_CLIENT_FEATURE | SLI_SI91X_INTERNAL_HTTP_CLIENT |
| SL_SI91X_ENABLE_LOWPWR_RET_LDO(New) | SL_SI91X_ENABLE_LOWPWR_RET_LDO  |