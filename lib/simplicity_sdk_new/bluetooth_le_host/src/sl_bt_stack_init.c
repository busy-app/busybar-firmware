/***************************************************************************//**
 * @brief Initialization APIs for Bluetooth stack
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// Include the catalog first to make sure everything below can see it
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#else // SL_COMPONENT_CATALOG_PRESENT
#error The SL component catalog required by Bluetooth stack is not present in the build
#endif // SL_COMPONENT_CATALOG_PRESENT

// Define the initilization mode for the configuration definition headers. The
// initialization mode for the Bluetooth stack depends on the on-demand start
// component. When on-demand start is used, features support de-initialization.
// When on-demand start is not used, the de-init functions are omitted to
// eliminate code that's not needed in the build.
#if defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)
#define SLI_BT_INIT_MODE on_demand
#else
#define SLI_BT_INIT_MODE init_always
#endif

#include "sl_status.h"
#include "sl_assert.h"
#include "sl_bt_api.h"
#include "sl_bt_stack_config.h"
#include "sl_bt_stack_init.h"
#include "sl_bluetooth_config.h"
#include "sli_bgapi.h"
#include "sli_bt_api.h"
#include "sli_bt_gattdb_def.h"
#include "sli_bt_config_defs.h"

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_bt_rtos_adaptation.h"
#endif

#if defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
#include "sli_bt_event_system.h"
#endif

#if defined(SL_CATALOG_BGAPI_SERVICE_PRESENT)
#include "sli_bgapi_service_api.h"
#endif // defined(SL_CATALOG_BGAPI_SERVICE_PRESENT)

#ifdef SL_CATALOG_GATT_CONFIGURATION_PRESENT
extern const sli_bt_gattdb_t gattdb;
#else
const sli_bt_gattdb_t gattdb = { 0 };
#endif // SL_CATALOG_GATT_CONFIGURATION_PRESENT

// -----------------------------------------------------------------------------
// Initialization of BGAPI device structures for the Bluetooth host stack

// Forward declaration of BGAPI classes
SLI_BT_DECLARE_BGAPI_CLASS(bt, system);
SLI_BT_DECLARE_BGAPI_CLASS(bt, nvm);
SLI_BT_DECLARE_BGAPI_CLASS(bt, gap);
SLI_BT_DECLARE_BGAPI_CLASS(bt, sm);
SLI_BT_DECLARE_BGAPI_CLASS(bt, external_bondingdb);
SLI_BT_DECLARE_BGAPI_CLASS(bt, accept_list);
SLI_BT_DECLARE_BGAPI_CLASS(bt, resolving_list);
SLI_BT_DECLARE_BGAPI_CLASS(bt, advertiser);
SLI_BT_DECLARE_BGAPI_CLASS(bt, legacy_advertiser);
SLI_BT_DECLARE_BGAPI_CLASS(bt, extended_advertiser);
SLI_BT_DECLARE_BGAPI_CLASS(bt, periodic_advertiser);
SLI_BT_DECLARE_BGAPI_CLASS(bt, scanner);
SLI_BT_DECLARE_BGAPI_CLASS(bt, sync);
SLI_BT_DECLARE_BGAPI_CLASS(bt, pawr_advertiser);
SLI_BT_DECLARE_BGAPI_CLASS(bt, sync_scanner);
SLI_BT_DECLARE_BGAPI_CLASS(bt, periodic_sync);
SLI_BT_DECLARE_BGAPI_CLASS(bt, pawr_sync);
SLI_BT_DECLARE_BGAPI_CLASS(bt, past_receiver);
SLI_BT_DECLARE_BGAPI_CLASS(bt, advertiser_past);
SLI_BT_DECLARE_BGAPI_CLASS(bt, sync_past);
SLI_BT_DECLARE_BGAPI_CLASS(bt, cs);
SLI_BT_DECLARE_BGAPI_CLASS(bt, cs_test);
SLI_BT_DECLARE_BGAPI_CLASS(bt, l2cap);
SLI_BT_DECLARE_BGAPI_CLASS(bt, connection);
SLI_BT_DECLARE_BGAPI_CLASS(bt, gatt);
SLI_BT_DECLARE_BGAPI_CLASS(bt, gattdb);
SLI_BT_DECLARE_BGAPI_CLASS(bt, gatt_server);
SLI_BT_DECLARE_BGAPI_CLASS(bt, cte_receiver);
SLI_BT_DECLARE_BGAPI_CLASS(bt, cte_transmitter);
SLI_BT_DECLARE_BGAPI_CLASS(bt, test);
SLI_BT_DECLARE_BGAPI_CLASS(bt, coex);
SLI_BT_DECLARE_BGAPI_CLASS(bt, resource);
SLI_BT_DECLARE_BGAPI_CLASS(bt, connection_analyzer);
SLI_BT_DECLARE_BGAPI_CLASS(bt, linklayer);

// Some features do not correspond directly to a particular component but are
// needed depending on a specific combination of components. Decide the derived
// feature selections here to simplify the feature inclusion rules below.

// CTE receiver is present if either AoA or AoD receiver is present
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_AOA_RECEIVER_PRESENT) \
  || defined(SL_CATALOG_BLUETOOTH_FEATURE_AOD_RECEIVER_PRESENT)
#define SLI_BT_CTE_RECEIVER_PRESENT
#endif

// Advertiser requires selection of legacy and/or extended advertiser
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT) \
  && !defined(SL_CATALOG_BLUETOOTH_FEATURE_LEGACY_ADVERTISER_PRESENT) \
  && !defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_ADVERTISER_PRESENT)
#error Incomplete Bluetooth advertiser feature detected. Add component \
       bluetooth_feature_legacy_advertiser for advertising with legacy \
       advertising PDUs. Add component bluetooth_feature_extended_advertiser \
       for advertising with extended advertising PDUs.
#endif

// Scanner requires selection of legacy and/or extended scanner
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT) \
  && !defined(SL_CATALOG_BLUETOOTH_FEATURE_LEGACY_SCANNER_PRESENT) \
  && !defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_SCANNER_PRESENT)
#error Incomplete Bluetooth scanner feature detected. Add component \
       bluetooth_feature_legacy_scanner for scanning advertisements in legacy \
       advertising PDUs. Add component bluetooth_feature_extended_scanner for \
       scanning advertisements in legacy or extended advertising PDUs.
#endif

// Extern declaration of component configuration structures
extern const struct sli_bt_component_config sli_bt_external_bondingdb_config;
extern const struct sli_bt_component_config sli_bt_accept_list_config;
extern const struct sli_bt_component_config sli_bt_sync_config;
extern const struct sli_bt_component_config sli_bt_advertiser_config;
extern const struct sli_bt_component_config sli_bt_periodic_advertiser_config;
extern const struct sli_bt_component_config sli_bt_l2cap_config;
extern const struct sli_bt_component_config sli_bt_connection_config;
extern const struct sli_bt_component_config sli_bt_dynamic_gattdb_config;

/** @brief Structure that specifies the Bluetooth configuration */
static const sl_bt_configuration_t bt_config = SL_BT_CONFIG_DEFAULT;

// Extern declaration of component init functions
extern sli_bgapi_component_init_func_t sli_bt_rtos_adaptation_init;
extern sli_bgapi_component_deinit_func_t sli_bt_rtos_adaptation_deinit;
extern sli_bgapi_component_init_func_t sli_bt_core_init;
extern sli_bgapi_component_start_func_t sli_bt_core_start;
extern sli_bgapi_component_deinit_func_t sli_bt_core_deinit;
extern sli_bgapi_component_init_func_t sli_bt_system_on_demand_start_init;
extern sli_bgapi_component_init_func_t sli_bt_system_init;
extern sli_bgapi_component_deinit_func_t sli_bt_system_deinit;
extern sli_bgapi_component_start_func_t sli_bt_builtin_bonding_database_start;
extern sli_bgapi_component_deinit_func_t sli_bt_builtin_bonding_database_deinit;
extern sli_bgapi_component_init_func_t sli_bt_external_bondingdb_init;
extern sli_bgapi_component_init_func_t sli_bt_sm_init;
extern sli_bgapi_component_start_func_t sli_bt_sm_start;
extern sli_bgapi_component_deinit_func_t sli_bt_sm_deinit;
extern sli_bgapi_component_init_func_t sli_bt_accept_list_init;
extern sli_bgapi_component_start_func_t sli_bt_resolving_list_start;
extern sli_bgapi_component_init_func_t sli_bt_scanner_init;
extern sli_bgapi_component_init_func_t sli_bt_scanner_base_init;
extern sli_bgapi_component_deinit_func_t sli_bt_scanner_base_deinit;
extern sli_bgapi_component_init_func_t sli_bt_extended_scanner_init;
extern sli_bgapi_component_init_func_t sli_bt_sync_init;
extern sli_bgapi_component_deinit_func_t sli_bt_sync_deinit;
extern sli_bgapi_component_init_func_t sli_bt_sync_scanner_init;
extern sli_bgapi_component_deinit_func_t sli_bt_sync_scanner_deinit;
extern sli_bgapi_component_init_func_t sli_bt_periodic_sync_init;
extern sli_bgapi_component_init_func_t sli_bt_pawr_sync_init;
extern sli_bgapi_component_init_func_t sli_bt_advertiser_init;
extern sli_bgapi_component_stop_func_t sli_bt_advertiser_stop;
extern sli_bgapi_component_deinit_func_t sli_bt_advertiser_deinit;
extern sli_bgapi_component_init_func_t sli_bt_extended_advertiser_init;
extern sli_bgapi_component_init_func_t sli_bt_periodic_advertiser_init;
extern sli_bgapi_component_deinit_func_t sli_bt_periodic_advertiser_deinit;
extern sli_bgapi_component_init_func_t sli_bt_pawr_advertiser_init;
extern sli_bgapi_component_start_func_t sli_bt_channel_sounding_start;
extern sli_bgapi_component_init_func_t sli_bt_channel_sounding_test_init;
extern sli_bgapi_component_init_func_t sli_bt_l2cap_init;
extern sli_bgapi_component_deinit_func_t sli_bt_l2cap_deinit;
extern sli_bgapi_component_init_func_t sli_bt_connection_init;
extern sli_bgapi_component_start_func_t sli_bt_connection_start;
extern sli_bgapi_component_stop_func_t sli_bt_connection_stop;
extern sli_bgapi_component_deinit_func_t sli_bt_connection_deinit;
extern sli_bgapi_component_init_func_t sli_bt_connection_role_central_init;
extern sli_bgapi_component_init_func_t sli_bt_connection_role_peripheral_init;
extern sli_bgapi_component_init_func_t sli_bt_connection_statistics_init;
extern sli_bgapi_component_start_func_t sli_bt_connection_subrating_start;
extern sli_bgapi_component_start_func_t sli_bt_dynamic_gattdb_start;
extern sli_bgapi_component_deinit_func_t sli_bt_dynamic_gattdb_deinit;
extern sli_bgapi_component_init_func_t sli_bt_cte_receiver_init;
extern sli_bgapi_component_deinit_func_t sli_bt_cte_receiver_deinit;
extern sli_bgapi_component_init_func_t sli_bt_test_init;
extern sli_bgapi_component_deinit_func_t sli_bt_test_deinit;
extern sli_bgapi_component_init_func_t sli_bt_power_control_init;
extern sli_bgapi_component_deinit_func_t sli_bt_power_control_deinit;
extern sli_bgapi_component_init_func_t sli_bt_connection_user_power_control_init;
extern sli_bgapi_component_init_func_t sli_bt_gatt_client_init;
extern sli_bgapi_component_deinit_func_t sli_bt_gatt_client_deinit;
extern sli_bgapi_component_init_func_t sli_bt_gatt_server_init;
extern sli_bgapi_component_deinit_func_t sli_bt_gatt_server_deinit;
extern sli_bgapi_component_init_func_t sli_bt_gatt_client_att_mtu_request_only_init;
extern sli_bgapi_component_init_func_t sli_bt_accurate_api_address_types_init;
extern sli_bgapi_component_start_func_t sli_bt_resource_start;
extern sli_bgapi_component_deinit_func_t sli_bt_resource_deinit;

// NULL-terminated array of component init structures
static const sli_bgapi_component_init_info_t bt_component_init_info[] = {
#if defined(SL_CATALOG_KERNEL_PRESENT)
  { sli_bt_rtos_adaptation_init, NULL },
#endif
  { sli_bt_core_init, &bt_config },
#if defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)
  { sli_bt_system_on_demand_start_init, NULL },
#endif
  { sli_bt_system_init, &bt_config },
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTERNAL_BONDING_DATABASE_PRESENT)
  { sli_bt_external_bondingdb_init, &sli_bt_external_bondingdb_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  { sli_bt_sm_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ACCEPT_LIST_PRESENT)
  { sli_bt_accept_list_init, &sli_bt_accept_list_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  { sli_bt_scanner_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  { sli_bt_scanner_base_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_SCANNER_PRESENT)
  { sli_bt_extended_scanner_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  { sli_bt_sync_init, &sli_bt_sync_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
  { sli_bt_sync_scanner_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_SYNC_PRESENT)
  { sli_bt_periodic_sync_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_SYNC_PRESENT)
  { sli_bt_pawr_sync_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  { sli_bt_advertiser_init, &sli_bt_advertiser_config },
#endif
#if defined(SLI_BT_ENABLE_EXTENDED_ADVERTISER_FEATURE)
  { sli_bt_extended_advertiser_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  { sli_bt_periodic_advertiser_init, &sli_bt_periodic_advertiser_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
  { sli_bt_pawr_advertiser_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
  { sli_bt_channel_sounding_test_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_L2CAP_PRESENT)
  { sli_bt_l2cap_init, &sli_bt_l2cap_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  { sli_bt_connection_init, &sli_bt_connection_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ROLE_CENTRAL_PRESENT)
  { sli_bt_connection_role_central_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ROLE_PERIPHERAL_PRESENT)
  { sli_bt_connection_role_peripheral_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_STATISTICS_PRESENT)
  { sli_bt_connection_statistics_init, NULL },
#endif
#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
  { sli_bt_cte_receiver_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_TEST_PRESENT)
  { sli_bt_test_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT)
  { sli_bt_power_control_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_USER_POWER_CONTROL_PRESENT)
  { sli_bt_connection_user_power_control_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_PRESENT)
  { sli_bt_gatt_client_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_SERVER_PRESENT)
  { sli_bt_gatt_server_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_CLIENT_ATT_MTU_REQUEST_ONLY_PRESENT)
  { sli_bt_gatt_client_att_mtu_request_only_init, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_USE_ACCURATE_API_ADDRESS_TYPES_PRESENT)
  { sli_bt_accurate_api_address_types_init, NULL },
#endif
  { NULL, NULL }
};

// NULL-terminated array of component start structures
static const sli_bgapi_component_start_info_t bt_component_start_info[] = {
  { sli_bt_core_start, &bt_config },
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_BUILTIN_BONDING_DATABASE_PRESENT)
  { sli_bt_builtin_bonding_database_start, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  { sli_bt_sm_start, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
  { sli_bt_resolving_list_start, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT)
  { sli_bt_channel_sounding_start, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  { sli_bt_connection_start, &sli_bt_connection_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_SUBRATING_PRESENT)
  { sli_bt_connection_subrating_start, NULL },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_DYNAMIC_GATTDB_PRESENT)
  { sli_bt_dynamic_gattdb_start, &sli_bt_dynamic_gattdb_config },
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
  { sli_bt_resource_start, NULL },
#endif
  { NULL, NULL }
};

// Stop and deinit functions are only needed when the On-demand Start component
// is present in the build
#if defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)

// NULL-terminated array of component stop functions.
static sli_bgapi_component_stop_func_t * const bt_component_stop_functions[] = {
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_stop,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  sli_bt_advertiser_stop,
#endif
  NULL
};

// NULL-terminated array of component deinit functions.
static sli_bgapi_component_deinit_func_t * const bt_component_deinit_functions[] = {
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
  sli_bt_resource_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_SERVER_PRESENT)
  sli_bt_gatt_server_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_PRESENT)
  sli_bt_gatt_client_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_POWER_CONTROL_PRESENT)
  sli_bt_power_control_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_TEST_PRESENT)
  sli_bt_test_deinit,
#endif
#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
  sli_bt_cte_receiver_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_DYNAMIC_GATTDB_PRESENT)
  sli_bt_dynamic_gattdb_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
  sli_bt_connection_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_L2CAP_PRESENT)
  sli_bt_l2cap_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
  sli_bt_periodic_advertiser_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
  sli_bt_advertiser_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
  sli_bt_sync_scanner_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
  sli_bt_sync_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
  sli_bt_scanner_base_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
  sli_bt_sm_deinit,
#endif
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_BUILTIN_BONDING_DATABASE_PRESENT)
  sli_bt_builtin_bonding_database_deinit,
#endif
  sli_bt_system_deinit,
  sli_bt_core_deinit,
#if defined(SL_CATALOG_KERNEL_PRESENT)
  sli_bt_rtos_adaptation_deinit,
#endif
  NULL
};

#endif // defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)

// If the build configuration needs a specific BGAPI class, we pick it for
// inclusion in the BGAPI list.

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYSTEM_PRESENT)
#define SLI_BT_BGAPI_SYSTEM SLI_BT_USE_BGAPI_CLASS(bt, system),
#else
#define SLI_BT_BGAPI_SYSTEM
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_NVM_PRESENT)
#define SLI_BT_BGAPI_NVM SLI_BT_USE_BGAPI_CLASS(bt, nvm),
#else
#define SLI_BT_BGAPI_NVM
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GAP_PRESENT)
#define SLI_BT_BGAPI_GAP SLI_BT_USE_BGAPI_CLASS(bt, gap),
#else
#define SLI_BT_BGAPI_GAP
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SM_PRESENT)
#define SLI_BT_BGAPI_SM SLI_BT_USE_BGAPI_CLASS(bt, sm),
#else
#define SLI_BT_BGAPI_SM
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTERNAL_BONDING_DATABASE_PRESENT)
#define SLI_BT_BGAPI_EXTERNAL_BONDINGDB SLI_BT_USE_BGAPI_CLASS(bt, external_bondingdb),
#else
#define SLI_BT_BGAPI_EXTERNAL_BONDINGDB
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ACCEPT_LIST_PRESENT)
#define SLI_BT_BGAPI_ACCEPT_LIST SLI_BT_USE_BGAPI_CLASS(bt, accept_list),
#else
#define SLI_BT_BGAPI_ACCEPT_LIST
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
#define SLI_BT_BGAPI_RESOLVING_LIST SLI_BT_USE_BGAPI_CLASS(bt, resolving_list),
#else
#define SLI_BT_BGAPI_RESOLVING_LIST
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PRESENT)
#define SLI_BT_BGAPI_ADVERTISER   SLI_BT_USE_BGAPI_CLASS(bt, advertiser),
#else
#define SLI_BT_BGAPI_ADVERTISER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_LEGACY_ADVERTISER_PRESENT)
#define SLI_BT_BGAPI_LEGACY_ADVERTISER SLI_BT_USE_BGAPI_CLASS(bt, legacy_advertiser),
#else
#define SLI_BT_BGAPI_LEGACY_ADVERTISER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_EXTENDED_ADVERTISER_PRESENT)
#define SLI_BT_BGAPI_EXTENDED_ADVERTISER SLI_BT_USE_BGAPI_CLASS(bt, extended_advertiser),
#else
#define SLI_BT_BGAPI_EXTENDED_ADVERTISER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_ADVERTISER_PRESENT)
#define SLI_BT_BGAPI_PERIODIC_ADVERTISER SLI_BT_USE_BGAPI_CLASS(bt, periodic_advertiser),
#else
#define SLI_BT_BGAPI_PERIODIC_ADVERTISER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_ADVERTISER_PRESENT)
#define SLI_BT_BGAPI_PAWR_ADVERTISER SLI_BT_USE_BGAPI_CLASS(bt, pawr_advertiser),
#else
#define SLI_BT_BGAPI_PAWR_ADVERTISER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SCANNER_PRESENT)
#define SLI_BT_BGAPI_SCANNER SLI_BT_USE_BGAPI_CLASS(bt, scanner),
#else
#define SLI_BT_BGAPI_SCANNER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PRESENT)
#define SLI_BT_BGAPI_SYNC SLI_BT_USE_BGAPI_CLASS(bt, sync),
#else
#define SLI_BT_BGAPI_SYNC
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_SCANNER_PRESENT)
#define SLI_BT_BGAPI_SYNC_SCANNER SLI_BT_USE_BGAPI_CLASS(bt, sync_scanner),
#else
#define SLI_BT_BGAPI_SYNC_SCANNER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PERIODIC_SYNC_PRESENT)
#define SLI_BT_BGAPI_PERIODIC_SYNC SLI_BT_USE_BGAPI_CLASS(bt, periodic_sync),
#else
#define SLI_BT_BGAPI_PERIODIC_SYNC
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAWR_SYNC_PRESENT)
#define SLI_BT_BGAPI_PAWR_SYNC SLI_BT_USE_BGAPI_CLASS(bt, pawr_sync),
#else
#define SLI_BT_BGAPI_PAWR_SYNC
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_PAST_RECEIVER_PRESENT)
#define SLI_BT_BGAPI_PAST_RECEIVER SLI_BT_USE_BGAPI_CLASS(bt, past_receiver),
#else
#define SLI_BT_BGAPI_PAST_RECEIVER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_ADVERTISER_PAST_PRESENT)
#define SLI_BT_BGAPI_ADVERTISER_PAST SLI_BT_USE_BGAPI_CLASS(bt, advertiser_past),
#else
#define SLI_BT_BGAPI_ADVERTISER_PAST
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_SYNC_PAST_PRESENT)
#define SLI_BT_BGAPI_SYNC_PAST SLI_BT_USE_BGAPI_CLASS(bt, sync_past),
#else
#define SLI_BT_BGAPI_SYNC_PAST
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_PRESENT)
#define SLI_BT_BGAPI_CS SLI_BT_USE_BGAPI_CLASS(bt, cs),
#else
#define SLI_BT_BGAPI_CS
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CS_TEST_PRESENT)
#define SLI_BT_BGAPI_CS_TEST SLI_BT_USE_BGAPI_CLASS(bt, cs_test),
#else
#define SLI_BT_BGAPI_CS_TEST
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_L2CAP_PRESENT)
#define SLI_BT_BGAPI_L2CAP SLI_BT_USE_BGAPI_CLASS(bt, l2cap),
#else
#define SLI_BT_BGAPI_L2CAP
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_PRESENT)
#define SLI_BT_BGAPI_CONNECTION SLI_BT_USE_BGAPI_CLASS(bt, connection),
#else
#define SLI_BT_BGAPI_CONNECTION
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_PRESENT)
#define SLI_BT_BGAPI_GATT SLI_BT_USE_BGAPI_CLASS(bt, gatt),
#else
#define SLI_BT_BGAPI_GATT
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_DYNAMIC_GATTDB_PRESENT)
#define SLI_BT_BGAPI_DYNAMIC_GATTDB SLI_BT_USE_BGAPI_CLASS(bt, gattdb),
#else
#define SLI_BT_BGAPI_DYNAMIC_GATTDB
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_GATT_SERVER_PRESENT)
#define SLI_BT_BGAPI_GATT_SERVER SLI_BT_USE_BGAPI_CLASS(bt, gatt_server),
#else
#define SLI_BT_BGAPI_GATT_SERVER
#endif

#if defined(SLI_BT_CTE_RECEIVER_PRESENT)
#define SLI_BT_BGAPI_CTE_RECEIVER SLI_BT_USE_BGAPI_CLASS(bt, cte_receiver),
#else
#define SLI_BT_BGAPI_CTE_RECEIVER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CTE_TRANSMITTER_PRESENT)
#define SLI_BT_BGAPI_CTE_TRANSMITTER SLI_BT_USE_BGAPI_CLASS(bt, cte_transmitter),
#else
#define SLI_BT_BGAPI_CTE_TRANSMITTER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_TEST_PRESENT)
#define SLI_BT_BGAPI_TEST SLI_BT_USE_BGAPI_CLASS(bt, test),
#else
#define SLI_BT_BGAPI_TEST
#endif

#if defined(SL_CATALOG_RAIL_UTIL_COEX_PRESENT)
#define SLI_BT_BGAPI_COEX SLI_BT_USE_BGAPI_CLASS(bt, coex),
#else
#define SLI_BT_BGAPI_COEX
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOURCE_REPORT_PRESENT)
#define SLI_BT_BGAPI_RESOURCE SLI_BT_USE_BGAPI_CLASS(bt, resource),
#else
#define SLI_BT_BGAPI_RESOURCE
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_CONNECTION_ANALYZER_PRESENT)
#define SLI_BT_BGAPI_CONNECTION_ANALYZER SLI_BT_USE_BGAPI_CLASS(bt, connection_analyzer),
#else
#define SLI_BT_BGAPI_CONNECTION_ANALYZER
#endif

#if defined(SL_CATALOG_BLUETOOTH_FEATURE_LINKLAYER_INTERFACE_PRESENT)
#define SLI_BT_BGAPI_LINKLAYER SLI_BT_USE_BGAPI_CLASS(bt, linklayer),
#else
#define SLI_BT_BGAPI_LINKLAYER
#endif

/** @brief Table of BGAPI classes available when Bluetooth is started */
static const struct sli_bgapi_class * const bt_bgapi_classes_when_started[] =
{
  // Invoke the BGAPI class inclusion macro for each feature that provides a
  // BGAPI class. Depending on the build configuration, the feature inclusion
  // rules above have defined the macro to either empty or the relevant BGAPI
  // class declaration.
  SLI_BT_BGAPI_SYSTEM
  SLI_BT_BGAPI_NVM
  SLI_BT_BGAPI_GAP
  SLI_BT_BGAPI_SM
  SLI_BT_BGAPI_EXTERNAL_BONDINGDB
  SLI_BT_BGAPI_ACCEPT_LIST
  SLI_BT_BGAPI_RESOLVING_LIST
  SLI_BT_BGAPI_ADVERTISER
  SLI_BT_BGAPI_LEGACY_ADVERTISER
  SLI_BT_BGAPI_EXTENDED_ADVERTISER
  SLI_BT_BGAPI_PERIODIC_ADVERTISER
  SLI_BT_BGAPI_PAWR_ADVERTISER
  SLI_BT_BGAPI_SCANNER
  SLI_BT_BGAPI_SYNC
  SLI_BT_BGAPI_SYNC_SCANNER
  SLI_BT_BGAPI_PERIODIC_SYNC
  SLI_BT_BGAPI_PAWR_SYNC
  SLI_BT_BGAPI_PAST_RECEIVER
  SLI_BT_BGAPI_ADVERTISER_PAST
  SLI_BT_BGAPI_SYNC_PAST
  SLI_BT_BGAPI_CS
  SLI_BT_BGAPI_CS_TEST
  SLI_BT_BGAPI_L2CAP
  SLI_BT_BGAPI_CONNECTION
  SLI_BT_BGAPI_GATT
  SLI_BT_BGAPI_DYNAMIC_GATTDB
  SLI_BT_BGAPI_GATT_SERVER
  SLI_BT_BGAPI_CTE_RECEIVER
  SLI_BT_BGAPI_CTE_TRANSMITTER
  SLI_BT_BGAPI_TEST
  SLI_BT_BGAPI_COEX
  SLI_BT_BGAPI_RESOURCE
  SLI_BT_BGAPI_CONNECTION_ANALYZER
  SLI_BT_BGAPI_LINKLAYER
  NULL
};

/**
 * @brief Table of BGAPI classes available when Bluetooth is stopped.
 *
 * These are only needed when the On-demand Start component is present in the
 * build.
 */
#if defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)
static const struct sli_bgapi_class * const bt_bgapi_classes_when_stopped[] =
{
  SLI_BT_BGAPI_SYSTEM
  NULL
};
#endif

// Structure that collects the read-only info of the Bluetooth host BGAPI device
static const sli_bgapi_device_info_t bt_device_info = {
  .component_init_info = bt_component_init_info,
  .component_start_info = bt_component_start_info,
#if defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)
  .component_stop_functions = bt_component_stop_functions,
  .component_deinit_functions = bt_component_deinit_functions,
  .bgapi_classes_when_started = bt_bgapi_classes_when_started,
  .bgapi_classes_when_stopped = bt_bgapi_classes_when_stopped,
#else
  .component_stop_functions = NULL,
  .component_deinit_functions = NULL,
  .bgapi_classes_when_started = bt_bgapi_classes_when_started,
  .bgapi_classes_when_stopped = NULL,
#endif
};

// BGAPI device structures for BGAPI Service are only needed when it's present
#if defined(SL_CATALOG_BGAPI_SERVICE_PRESENT)

// -----------------------------------------------------------------------------
// Initialization of BGAPI device structures for the BGAPI Service

// Forward declaration of BGAPI classes
SLI_BT_DECLARE_BGAPI_CLASS(bgapi, system);

/** @brief Table of BGAPI classes of BGAPI Service */
static const struct sli_bgapi_class * const bgapi_service_classes[] =
{
#if defined(SL_CATALOG_BGAPI_SERVICE_FEATURE_SYSTEM_PRESENT)
  SLI_BT_USE_BGAPI_CLASS(bgapi, system),
#endif // defined(SL_CATALOG_BLUETOOTH_FEATURE_SYSTEM_PRESENT)
  NULL
};

// Structure that collects the read-only info of the BGAPI Service device
static const sli_bgapi_device_info_t bgapi_service_device_info = {
  .component_init_info = NULL,
  .component_start_info = NULL,
  .component_stop_functions = NULL,
  .component_deinit_functions = NULL,
  .bgapi_classes_when_started = bgapi_service_classes,
  .bgapi_classes_when_stopped = bgapi_service_classes,
};
#endif // defined(SL_CATALOG_BGAPI_SERVICE_PRESENT)

// -----------------------------------------------------------------------------
// Initialization entry points used with `sl_system`

// Initialize the Bluetooth stack.
sl_status_t sl_bt_stack_init()
{
  // This initialization entry point is used in the single-stage `sl_system`
  // initialization. We implement the single-stage initialization by performing
  // the two stages of `sl_main` initialization in one go. Since `sl_main`
  // initialization functions do not return a value, they will assert on errors.
  // If the return here, initialization was successful.
  sli_bt_stack_permanent_allocation();
  sli_bt_stack_functional_init();

  return SL_STATUS_OK;
}

// -----------------------------------------------------------------------------
// Initialization entry points used with `sl_main`

// Make permanent memory allocations for the Bluetooth stack.
void sli_bt_stack_permanent_allocation(void)
{
  sl_status_t status = SL_STATUS_FAIL;

  // When Event System IPC is in use, let it make its permanent allocations
#if defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
  status =  sli_bt_event_system_permanent_allocations();
  EFM_ASSERT(status == SL_STATUS_OK);
#endif

  // When a kernel is present, let RTOS adaptation make its permanent
  // allocations
#if defined(SL_CATALOG_KERNEL_PRESENT)
  status = sli_bt_rtos_adaptation_permanent_allocation();
  EFM_ASSERT(status == SL_STATUS_OK);
#endif // defined(SL_CATALOG_KERNEL_PRESENT)

  // Register the Bluetooth host stack BGAPI device
  status = sli_bt_register_bgapi_device(&bt_config, &bt_device_info);
  EFM_ASSERT(status == SL_STATUS_OK);

  // If BGAPI Service is present, register its BGAPI device
#if defined(SL_CATALOG_BGAPI_SERVICE_PRESENT)
  status = sli_bgapi_service_register_bgapi_device(&bgapi_service_device_info);
  EFM_ASSERT(status == SL_STATUS_OK);
#endif // defined(SL_CATALOG_BGAPI_SERVICE_PRESENT)

  // When the On-demand Start feature is present, the application is in full
  // control of when to start the Bluetooth stack, and the stack initialization
  // occurs when the application starts the stack. When not present, the stack
  // initialization occurs in the permanent allocation stage.
#if !defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)
  status = sli_bt_init_bgapi_device();
  EFM_ASSERT(status == SL_STATUS_OK);
#endif // !defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)

  // Suppress warning about potentially unused variable
  (void) status;
}

// Perform functional initialization of the Bluetooth stack.
void sli_bt_stack_functional_init(void)
{
  sl_status_t status = SL_STATUS_FAIL;

  // When Event System IPC is in use, let it perform its init
#if defined(SL_CATALOG_BLUETOOTH_EVENT_SYSTEM_IPC_PRESENT)
  status =  sli_bt_event_system_functional_init();
  if (status != SL_STATUS_OK) {
    return;
  }
#endif

  // When the On-demand Start feature is present, the application is in full
  // control of when to start the Bluetooth stack. When not present, the stack
  // start is triggered here in the functional initialization stage.
#if !defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)

  // When an RTOS is present, the starting of the Bluetooth host stack is driven
  // by the RTOS adaptation from within the Bluetooth host stack task. When not
  // present (in a baremetal build), the starting happens here directly.
#if defined(SL_CATALOG_KERNEL_PRESENT)
  status = sli_bt_rtos_adaptation_start();
  EFM_ASSERT(status == SL_STATUS_OK);
#else
  status = sli_bt_start_bgapi_device();
  EFM_ASSERT(status == SL_STATUS_OK);
#endif

#endif // !defined(SL_CATALOG_BLUETOOTH_ON_DEMAND_START_PRESENT)

  // Suppress warning about potentially unused variable
  (void) status;
}