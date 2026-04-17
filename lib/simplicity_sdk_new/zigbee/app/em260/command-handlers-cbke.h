/***************************************************************************//**
 * @file
 * @brief Declarations for CBKE command handlers.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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
// Ezsp Command Handlers

sl_status_t sl_zigbee_af_ezsp_dsa_sign_command_cb(uint8_t messageLength,
                                                  uint8_t* message);

sl_status_t sl_zigbee_af_ezsp_save_preinstalled_cbke_data_283k1_command_cb(void);

// The CBKE Data associated with ECC 283k1 is sent in three separate EZSP
// commands because the total length of the combined data is greater than the
// max EZSP message length.  The first EZSP command will contain the certificate
// followed next by the public key and finally the private key. Once
// all the data has been received by the NCP a 4th command is sent by the host
// indicating to push the data into permanent storage.  We use Ember message
// buffers as temporary storage for the 3 components of the CBKE 283k1 data.
#define CBKE_283K1_DATA_CERTIFICATE_OFFSET 0
#define CBKE_283K1_DATA_PUBLIC_KEY_OFFSET \
  (CBKE_283K1_DATA_CERTIFICATE_OFFSET + SL_ZIGBEE_CERTIFICATE_283K1_SIZE)
#define CBKE_283K1_DATA_PRIVATE_KEY_OFFSET \
  (CBKE_283K1_DATA_PUBLIC_KEY_OFFSET + SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE)
#define CBKE_283K1_DATA_LENGTH \
  (SL_ZIGBEE_CERTIFICATE_283K1_SIZE + SL_ZIGBEE_PUBLIC_KEY_283K1_SIZE + SL_ZIGBEE_PRIVATE_KEY_283K1_SIZE)

void sl_zigbee_xncp_cbke_set_value_command_cb(sl_zigbee_af_plugin_ezsp_value_command_context_t* context);
void sl_zigbee_xncp_cbke_configuration_value_command_cb(sl_zigbee_af_plugin_ezsp_configuration_value_command_context_t* context);

// -----------------------------------------------------------------------------
// Framework callbacks

void sl_zigbee_xncp_cbke_permit_host_to_ncp_frame_cb(bool *permit);
