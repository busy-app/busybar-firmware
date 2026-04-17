/***************************************************************************/ /**
 * @file app.c
 * @brief Top level application functions.
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "app.h"

#include <string.h>
#include "cmsis_os2.h"
#include "sl_net.h"
#include "sl_wifi.h"
#include "sl_status.h"

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
//! Memory length for driver
#define GLOBAL_BUFF_LEN            15000
#define RSI_DRIVER_TASK_STACK_SIZE 500
#define RSI_DRIVER_TASK_PRIORITY   2

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
static void application_start(void *argument);
static void psa_crypto_deinitialize(void);
// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
//! Memory to initialize driver
uint8_t global_buf[GLOBAL_BUFF_LEN];
// -----------------------------------------------------------------------------
//                                Variable Definitions
// -----------------------------------------------------------------------------
const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

static const sl_wifi_device_configuration_t client_configuration = {
  .boot_option = LOAD_NWP_FW,
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode       = SL_SI91X_CLIENT_MODE,
                   .coex_mode       = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION
#ifdef SLI_SI91X_MCU_INTERFACE
                                       | SL_WIFI_FEAT_WPS_DISABLE
#endif
                                       ),
                   .tcp_ip_feature_bit_map     = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
                   .custom_feature_bit_map     = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map = (
#ifdef SLI_SI91X_MCU_INTERFACE
                     SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC
#else
                     SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE
#endif
#if defined(SLI_SI917)
                     | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                     ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = 0,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

/// Asymmetric key storage selection
static uint8_t asymmetric_key_storage_select;

static const char *asymmetric_key_storage_string[] = { "VOLATILE PLAIN", "NO KEY TYPE" };

/// Asymmetric key ECC curve selection
static uint8_t asymmetric_key_curve_select;

static const char *asymmetric_key_curve_string[] = { "SECPxxxR1", "MONTGOMERY" };

static const psa_ecc_family_t asymmetric_key_curve[] = { PSA_ECC_FAMILY_SECP_R1, PSA_ECC_FAMILY_MONTGOMERY };

/// ECC SECPR1 key size selection
static uint8_t secpr1_key_size_select;

static const char *secpr1_key_size_string[] = {
  "SECP192R1",
  "SECP256R1",
  "SECP384R1",
  "SECP521R1",
};

static const size_t secpr1_key_size[] = { 192, 256, 384, 521 };

/// ECC Montgomery key size selection
static uint8_t montgomery_key_size_select;

static const char *montgomery_key_size_string[] = { "CURVE25519 (X25519)", "CURVE448 (X448)" };

static const size_t montgomery_key_size[] = { 255, 448 };

/// Selected key size
static size_t selected_key_size;

/// Selected key string
static const char *selected_key_string;

/// Client key identifier
static psa_key_id_t client_key_id;

/// Server key identifier
static psa_key_id_t server_key_id;

/// Shared secret copy
static uint8_t shared_secret_copy[SHARED_SECRET_SIZE];

/// Number of iterations application ran
static uint8_t app_count = 0;
// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
/*****************************************************************************
 * Initialize application.
 ******************************************************************************/
void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);

  sl_status_t status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &client_configuration, NULL, NULL);
  if (status != SL_STATUS_OK) {
    printf("Failed to start Wi-Fi client interface: 0x%lx\r\n", status);
    return;
  }
  printf("\r\nWi-Fi Init Success\r\n");

  app_process_action();
}

static void psa_crypto_deinitialize(void)
{
  printf("\n\r  . PSA Crypto deinitialization\n");
  // Destroy the keys regardless they are valid or not
  set_key_id(client_key_id);
  printf("\n\r  + Destroying a client key... ");
  destroy_key();
  set_key_id(server_key_id);
  printf("\n\r  + Destroying a server key... ");
  destroy_key();
  mbedtls_psa_crypto_free();
}

/***************************************************************************/ /**
 * Application state machine, called infinitely.
 ******************************************************************************/
void app_process_action(void)
{
  psa_status_t ret;
  ret = psa_crypto_init();
  if (ret != PSA_SUCCESS) {
    printf("PSA Crypto Init failed with status : %ld\n", ret);
  } else {
    printf("PSA Crypto Init Success\n");
  }

  asymmetric_key_storage_select = VOLATILE_PLAIN_KEY;
  printf("\n\r Current asymmetric key is a %s key.\n", asymmetric_key_storage_string[asymmetric_key_storage_select]);

  asymmetric_key_curve_select = 0;
  printf("\n\rCurrent ECC curve is %s.\n", asymmetric_key_curve_string[asymmetric_key_curve_select]);

  if (asymmetric_key_curve_select == 0) {
    secpr1_key_size_select = 1;
    selected_key_size      = secpr1_key_size[secpr1_key_size_select];
    selected_key_string    = secpr1_key_size_string[secpr1_key_size_select];
    printf("  + Current %s key length is %d-bit (%s).\n",
           asymmetric_key_curve_string[asymmetric_key_curve_select],
           secpr1_key_size[secpr1_key_size_select],
           secpr1_key_size_string[secpr1_key_size_select]);
  } else if (asymmetric_key_curve_select == 1) {
    montgomery_key_size_select = 0;
    printf("  + Current %s key length is %d-bit (%s).\n",
           asymmetric_key_curve_string[asymmetric_key_curve_select],
           montgomery_key_size[montgomery_key_size_select],
           montgomery_key_size_string[montgomery_key_size_select]);

    selected_key_size   = montgomery_key_size[montgomery_key_size_select];
    selected_key_string = montgomery_key_size_string[montgomery_key_size_select];
  }

  // Create client key
  printf("\n\r  . ECDH Client\n");
  printf("\n\r  + Creating a %s (%d-bit) %s client key... ",
         selected_key_string,
         selected_key_size,
         asymmetric_key_storage_string[asymmetric_key_storage_select]);
  if (create_import_key(CREATE_KEY_OFFSET + asymmetric_key_storage_select,
                        PSA_KEY_TYPE_ECC_KEY_PAIR(asymmetric_key_curve[asymmetric_key_curve_select]),
                        selected_key_size,
                        CLIENT_KEY_ID,
                        DEFAULT_KEY_USAGE,
                        PSA_ALG_ECDH)
      == PSA_SUCCESS) {
    // Free resources
    reset_key_attr();
    // Save client key identifier
    client_key_id = get_key_id();
  } else {
    psa_crypto_deinitialize();
  }

  // Create server key
  printf("\n\r  + Creating a %s (%d-bit) %s server key... ",
         selected_key_string,
         selected_key_size,
         asymmetric_key_storage_string[asymmetric_key_storage_select]);

  if (create_import_key(CREATE_KEY_OFFSET + asymmetric_key_storage_select,
                        PSA_KEY_TYPE_ECC_KEY_PAIR(asymmetric_key_curve[asymmetric_key_curve_select]),
                        selected_key_size,
                        SERVER_KEY_ID,
                        DEFAULT_KEY_USAGE,
                        PSA_ALG_ECDH)
      == PSA_SUCCESS) {
    // Free resources
    reset_key_attr();
    // Save server key identifier
    server_key_id = get_key_id();
  } else {
    psa_crypto_deinitialize();
  }

  printf("\n\r  + Exporting a public key of a %s (%d-bit) %s server key... ",
         selected_key_string,
         selected_key_size,
         asymmetric_key_storage_string[asymmetric_key_storage_select]);
  // Export server public key
  set_key_id(server_key_id);
  if (export_public_key() == PSA_SUCCESS) {
    printf("\n\rServer public key export success.");
  } else {
    psa_crypto_deinitialize();
  }

  printf("\n\r  + Computing client shared secret with a %s (%d-bit) server "
         "public key... ",
         selected_key_string,
         selected_key_size);
  // Use server public key to compute shared secret
  memset(get_shared_secret_buf_ptr(), 0, SHARED_SECRET_SIZE);
  set_key_id(client_key_id);
  set_peer_public_key_buf_ptr(get_key_buf_ptr());
  set_peer_public_key_len(get_key_len());
  if (compute_ecdh_shared_secret() == PSA_SUCCESS) {
    // Save client shared secret
    memcpy(shared_secret_copy, get_shared_secret_buf_ptr(), get_shared_secret_length());
  } else {
    psa_crypto_deinitialize();
  }

  printf("\n\r  . ECDH Server\n\r");
  printf("\n\r  + Exporting a public key of a %s (%d-bit) %s client key... ",
         selected_key_string,
         selected_key_size,
         asymmetric_key_storage_string[asymmetric_key_storage_select]);
  // Export client public key
  set_key_id(client_key_id);
  if (export_public_key() == PSA_SUCCESS) {
    printf("\n\rClient public key export success.");
  } else {
    psa_crypto_deinitialize();
  }

  printf("\n\r  + Computing server shared secret with a %s (%d-bit) client "
         "public key... ",
         selected_key_string,
         selected_key_size);
  // Use client public key to compute shared secret
  memset(get_shared_secret_buf_ptr(), 0, SHARED_SECRET_SIZE);
  set_key_id(server_key_id);
  set_peer_public_key_buf_ptr(get_key_buf_ptr());
  set_peer_public_key_len(get_key_len());
  app_count++;
  printf("\n\r APPLICATION RUN COUNT: %d", app_count);
  if (compute_ecdh_shared_secret() == PSA_SUCCESS) {
    printf("\n\r  + Comparing shared secret from client and server... ");
    if (memcmp(shared_secret_copy, get_shared_secret_buf_ptr(), get_shared_secret_length()) == 0) {
      printf("OK\n");
    } else {
      printf("Failed\n");
    }
  } else {
    psa_crypto_deinitialize();
  }

  printf("\n\r  + Destroying a %s (%d-bit) %s client key... ",
         selected_key_string,
         selected_key_size,
         asymmetric_key_storage_string[asymmetric_key_storage_select]);
  set_key_id(client_key_id);
  if (destroy_key() == PSA_SUCCESS) {
    client_key_id = 0; // Reset client key identifier
  }

  printf("\n\r  + Destroying a %s (%d-bit) %s server key... ",
         selected_key_string,
         selected_key_size,
         asymmetric_key_storage_string[asymmetric_key_storage_select]);
  set_key_id(server_key_id);
  if (destroy_key() == PSA_SUCCESS) {
    server_key_id = 0; // Reset server key identifier
  }
}
