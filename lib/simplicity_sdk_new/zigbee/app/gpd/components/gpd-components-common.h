/***************************************************************************//**
 * @file
 * @brief Routines for timers used by the GPD.
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
#ifndef __SILABS_GPD_COMPONENTS_COMMON_H__
#define __SILABS_GPD_COMPONENTS_COMMON_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gpd-apps-config.h"
#include "sl_rail.h"
#include "rail_types.h"
#include "sl_clock_manager.h"
#include "sl_gpio.h"
#include "em_emu.h"
//rail ieee 802.15.4
#include  "sl_rail_ieee802154.h"

//mbedtls
#include "mbedtls/ccm.h"

// Common defines
#define FAILED  (-1)
#define SUCCESS (0)

#define HI_UINT16(a) ((uint8_t)(((uint16_t)(a) >> 8) & 0xFF))
#define LO_UINT16(a) ((uint8_t)((uint16_t)(a) & 0xFF))

#define GP_FIFO_SIZE 128

#define SL_ZIGBEE_GPD_TX_MPDU_SIZE 128
#define SL_ZIGBEE_GPD_RX_MPDU_SIZE 128

#define SL_ZIGBEE_GPD_EUI64_SIZE 8

#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_INDICATION_MASK 0x04
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_INDICATION_MASK_OFFSET 2
// GPD max payload sizes
#define SL_ZIGBEE_GPD_SRC_ID_MAX_PAYLOAD_SIZE   59
#define SL_ZIGBEE_GPD_IEEE_ID_MAX_PAYLOAD_SIZE  54

// GPD Application Type
#define SL_ZIGBEE_GPD_APP_ID_SRC_ID    0
#define SL_ZIGBEE_GPD_APP_ID_IEEE_ID   2

//GPD MAC Sequence Capability Type
#define SL_ZIGBEE_GPD_MAC_SEQ_RANDOM     0
#define SL_ZIGBEE_GPD_MAC_SEQ_INCREMENTAL 1

// Offsets in incoming frame
#define SL_ZIGBEE_GPD_MAC_LONG_ADDRESS_OFFSET 6
#define SL_ZIGBEE_GPD_NWK_FC_OFFSET_FOR_LONG_ADDRESS 14
#define SL_ZIGBEE_GPD_NWK_FC_OFFSET_FOR_SHORT_ADDRESS 8

// GPD Security Level Type
#define SL_ZIGBEE_GPD_SECURITY_LEVEL_NONE           0x00 // No security
#define SL_ZIGBEE_GPD_SECURITY_LEVEL_RESERVED       0x01 // Reserved
#define SL_ZIGBEE_GPD_SECURITY_LEVEL_LONG_AUTH_ONLY 0x02 // long 4 bytes Frame counter, 4 bytes MIC, only authentication no encryption
#define SL_ZIGBEE_GPD_SECURITY_LEVEL_LONG_FULL      0x03 // long 4 bytes Frame counter, 4 bytes MIC, authentication and encryption
#define TAG_SIZE                                4
//GPD Security Key Type
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_NOKEY         0x00   // No key
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_NWK           0x01   // NWK Key
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_GROUP         0x02   // ZGP group key
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_GROUP_DERIVED 0x03   // ZGPD Derived group key
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_OOB           0x04   // (individual) OOB ZGPD Key
#define SL_ZIGBEE_GPD_SECURITY_KEY_TYPE_DERIVED       0x07   // derived individual ZGPD key

//GPD Security Key Indication Flag
#define SL_ZIGBEE_GPD_SECURITY_KEY_INDICATION_SHARED     0x00
#define SL_ZIGBEE_GPD_SECURITY_KEY_INDICATION_INDIVIDUAL 0x01

// NV Storage macros
#define SL_ZIGBEE_GPD_NV_DATA_GPD_FRAME_COUNTER_INDEX  0
#define SL_ZIGBEE_GPD_NV_DATA_GPD_KEY_INDEX            4
#define SL_ZIGBEE_GPD_NV_DATA_GPD_SEC_LEVEL_INDEX      20
#define SL_ZIGBEE_GPD_NV_DATA_GPD_SEC_KEY_TYPE_INDEX   21
#define SL_ZIGBEE_GPD_NV_DATA_GPD_CHANNEL_INDEX        22
#define SL_ZIGBEE_GPD_NV_DATA_GPD_STATE_INDEX          23

#define SL_ZIGBEE_GPD_NV_DATA_SIZE                     32

////////////////////////////
// GPD Commands
#define GP_CMD_COMMISSIONING            0xE0
#define GP_CMD_DECOMMISSIONING          0xE1
#define GP_CMD_COMMISSIONING_SUCCESS    0xE2
#define GP_CMD_CHANNEL_RQST             0xE3
#define GP_CMD_COMMISSIONING_APP_DESCR  0xE4

#define GP_CMD_COMMISSIONING_REPLY      0xF0
#define GP_CMD_CHANNEL_CONFIG           0xF3

#define GP_CMD_TOGGLE                   0x22

#define GP_CMD_ATTRIBUTE_REPORTING      0xA0

// MAC Frame control field
// MAC FC bits
//-----------------------------------------------------------------------------
//0-2  | 3       | 4      | 5 | 6          | 7-9| 10-11      | 12-13  | 14-15
//-----+---------+--------+---+------------+----+------------+--------+--------
//Frame| Security| Frame  | AR| PAN ID     | Res| Destination| Frame  | Source
//Type | Enabled | Pending|   | Compression|    | Addressing | Version| Addressing
//     |         |        |   |            |    | Mode       |        | Mode
//-----------------------------------------------------------------------------
#define SL_ZIGBEE_GPD_MAC_FC_WORD                     0x0801
#define SL_ZIGBEE_GPD_MAC_FC_IEEE_WORD                0xC841
#define SL_ZIGBEE_GPD_MAC_FC_IEEE_WORD_INCOMING       0x0C01
#define SL_ZIGBEE_GPD_MAC_FOOTER_SIZE                 2
#define SL_ZIGBEE_GPD_MAC_HEADER_SIZE                 8
#define SL_ZIGBEE_GPD_MAC_HEADER_SIZE_IEEE_RX         14
#define SL_ZIGBEE_GPD_MAC_HEADER_SIZE_IEEE_TX         16

// Authentication Macro
#define SL_ZIGBEE_GPD_AUTH_FIXED_LEN_SRC_ID    10
#define SL_ZIGBEE_GPD_AUTH_START_SRC_ID        8
#define SL_ZIGBEE_GPD_AUTH_START_IEEE          16
#define SL_ZIGBEE_GPD_AUTH_FIXED_LEN_IEEE      7

// Network Frame Control Fields
// Frame Type sub-field
#define SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA     0x00
#define SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_MAINT    0x01
#define SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_MASK     0x03

// Zigbee Protocol Version sub-field
#define SL_ZIGBEE_GPD_NWK_FC_ZIGBEE_VER          0x03
#define SL_ZIGBEE_GPD_NWK_FC_ZIGBEE_VER_MASK     0x1C
#define SL_ZIGBEE_GPD_NWK_FC_ZIGBEE_VER_OFFSET   2

// Autocommissioning sub-field
#define SL_ZIGBEE_GPD_NWK_FC_AUTOCOM_MASK   0x40
// Extended Network Frame Control sub-field
#define SL_ZIGBEE_GPD_NWK_FC_EXT_FC_MASK       0x80

//Extended Network Frame Control Fields
// Application Id sub-field
#define SL_ZIGBEE_GPD_EXT_NWK_FC_APP_ID_MASK         0x07
// Security Level sub-field
#define SL_ZIGBEE_GPD_EXT_NWK_FC_SECURITY_LEVEL_MASK 0x18
#define SL_ZIGBEE_GPD_EXT_NWK_FC_SECURITY_LEVEL_OFFSET 3
// Security Key Type sub-field
#define SL_ZIGBEE_GPD_EXT_NWK_FC_SEC_KEY_TYPE_MASK   0x20
// Rx After Tx sub-field
#define SL_ZIGBEE_GPD_EXT_NWK_FC_RX_AFTER_TX_MASK    0x40

// Direction sub-field
#define SL_ZIGBEE_GPD_EXT_NWK_FC_DIR_ZGPD_TO_ZGPP 0
#define SL_ZIGBEE_GPD_EXT_NWK_FC_DIR_ZGPP_TO_ZGPD 1
#define SL_ZIGBEE_GPD_EXT_NWK_FC_DIRECTION_MASK   0x80

/*******************************************************************************
* Commissioning Option Fields
*******************************************************************************/
// MAC Sequence Number Capability sub-field
#define SL_ZIGBEE_GPD_COM_OPT_MAC_SEQ_NBR_CAP_MASK    0x01
// Rx Capability sub-field
#define SL_ZIGBEE_GPD_COM_OPT_RX_CAP_MASK    0x02
// Application Information sub-field
#define SL_ZIGBEE_GPD_COM_OPT_APP_INFO_MASK   0x04
// PanID Request sub-field
#define SL_ZIGBEE_GPD_COM_OPT_PANID_REQ_MASK    0x10
// Security Key Request sub-field
#define SL_ZIGBEE_GPD_COM_OPT_KEY_REQ_MASK    0x20
// Fixed Location sub-field
#define SL_ZIGBEE_GPD_COM_OPT_FIXED_LOC_MASK    0x40
// Extended Option sub-field
#define SL_ZIGBEE_GPD_COM_OPT_EXT_OPT_MASK    0x80

/*******************************************************************************
* Extended Commissioning Option Fields
*******************************************************************************/
// Security Level Capability sub-field
#define SL_ZIGBEE_GPD_EXT_COM_OPT_SECURITY_LEVEL_CAP_MASK    0x03
// Key type sub-field
#define SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_TYPE_MASK    0x1C
#define SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_TYPE_OFFSET  2
// Key Present sub-field
#define SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_PRESENT_MASK    0x20
// Key Encrypted sub-field
#define SL_ZIGBEE_GPD_EXT_COM_OPT_KEY_ENCRYPT_MASK    0x40
// Outgoing Counter Present sub-field
#define SL_ZIGBEE_GPD_EXT_COM_OPT_OUT_COUNTER_PRESENT_MASK    0x80

// Application Information Commissioning Fields
//
// MS ID sub field
#define SL_ZIGBEE_GPD_APP_INFO_COM_MS_ID_MASK         0x01
// Model ID sub field
#define SL_ZIGBEE_GPD_APP_INFO_COM_MODEL_ID_MASK      0x02
// CMD list sub field
#define SL_ZIGBEE_GPD_APP_INFO_COM_CMDLIST_MASK       0x04
// CLuster list sub field
#define SL_ZIGBEE_GPD_APP_INFO_COM_CLUSTERLIST_MASK   0x08
// Switch information mask
#define SL_ZIGBEE_GPD_APP_INFO_COM_SWITCH_INFORMATION_PRESENT_MASK 0x10
// Application Description Follows mask
#define SL_ZIGBEE_GPD_APP_INFO_COM_APP_DESCRIPTION_FOLLOWS_MASK 0x20
//

//Commissioning Reply Option Flag
//
// PanId Present sub-field
#define SL_ZIGBEE_GPD_COM_REP_OPT_PANID_PRESENT_MASK   0x01
// Key Present sub-field
#define SL_ZIGBEE_GPD_COM_REP_OPT_KEY_PRESENT_MASK     0x02
// Security Key Encrypted
#define SL_ZIGBEE_GPD_COM_REP_OPT_KEY_ENCRYPT_MASK     0x04
// Security Level sub-field
#define SL_ZIGBEE_GPD_COM_REP_OPT_SECURITY_LEVEL_MASK  0x18
#define SL_ZIGBEE_GPD_COM_REP_OPT_SECURITY_LEVEL_OFFSET 0x03
// Haskey sub-field
#define SL_ZIGBEE_GPD_COM_REP_OPT_KEY_TYPE_MASK        0xE0
#define SL_ZIGBEE_GPD_COM_REP_OPT_KEY_TYPE_OFFSET      0x05

#define SL_ZIGBEE_GPD_MIC_LENGTH            4

#define SL_ZIGBEE_GPD_CMD_MAX_PAYLOAD_SIZE  40

// SecurityLevel enumeration for type sl_zigbee_gpd_security_level_t_t
typedef uint8_t sl_zigbee_gpd_security_level_t_t;
// Security Key enumeration for type sl_zigbee_gpd_security_key_type_t_t
typedef uint8_t sl_zigbee_gpd_security_key_type_t_t;
// Security Key Indication values
typedef uint8_t sl_zigbee_gpd_security_key_indication_type_t_t;
// Application Radio State
typedef uint8_t sl_zigbee_gpd_radio_state_t_t;
enum {
  GPD_RADIO_STATE_IDLE         = 0x00,
  GPD_RADIO_STATE_TRANSMITTING = 0x01,
  GPD_RADIO_STATE_RECEIVING    = 0x02
};

typedef uint8_t sl_zigbee_gpd_application_state_t_t;
enum {
  SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED,
  SL_ZIGBEE_GPD_APP_STATE_CHANNEL_REQUEST,
  SL_ZIGBEE_GPD_APP_STATE_CHANNEL_RECEIVED,
  SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_REQUEST,
  SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_REPLY_RECEIVED,
  SL_ZIGBEE_GPD_APP_STATE_COMMISSIONING_SUCCESS_REQUEST,
  SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL,
  SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL_COMMAND_REQUEST,
  SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL_COMMAND_RECEIVED,
  SL_ZIGBEE_GPD_APP_STATE_INVALID
};

// Callback type defines
typedef uint8_t EmebrGpdNvLoadStore_t;
enum {
  EMEBER_GPD_AF_CALLBACK_LOAD_GPD_FROM_NVM,
  EMEBER_GPD_AF_CALLBACK_STORE_GPD_TO_NVM,
};

#define SL_ZIGBEE_AP_PLUGIN_APPS_APP_NEXT_RX_CHANNEL 11
#define SL_ZIGBEE_AP_PLUGIN_APPS_APP_SECOND_NEXT_RX_CHANNEL 11
#define SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER 1

// GPD address structure
typedef struct {
  union {
    uint32_t srcId;
    uint8_t ieee[8];
  }id;
  uint8_t gpdEndpoint;
  uint8_t appId;
}sl_zigbee_gpd_addr_t_t;

// GPD device
typedef struct {
  sl_zigbee_gpd_addr_t_t addr;
  bool autoCommissioning;
  uint8_t rxAfterTx;
  uint8_t rxOffset;
  uint8_t minRxWindow;
  sl_zigbee_gpd_security_level_t_t securityLevel;
  sl_zigbee_gpd_security_key_type_t_t securityKeyType;
  uint8_t securityKey[16];
  uint32_t securityFrameCounter;
  bool skipCca;
  uint8_t channel;
  sl_zigbee_gpd_radio_state_t_t radioState;
  sl_zigbee_gpd_application_state_t_t gpdState;
}sl_zigbee_gpd_t_t;

// Function Prototypes
void sl_zigbee_gpd_radio_init();
int8_t sl_zigbee_af_gpdf_send(uint8_t frameType,
                              sl_zigbee_gpd_t_t * gpd,
                              uint8_t* payload,
                              uint8_t payloadLength,
                              uint8_t repetition_nb);

uint8_t sl_zigbee_build_maint_mpdu(uint8_t * mpdu,
                                   sl_zigbee_gpd_t_t * gpd,
                                   uint8_t * pPayload,
                                   uint8_t payloadLength);

int8_t sl_zigbee_gpd_process_incomming_command(uint8_t * gpdCommandBuffer,
                                               uint8_t length);

int8_t sl_zigbee_gpd_parse_and_process_rx_gpdf(uint8_t * buffer,
                                               uint16_t length);

// Radio Function Prototypes
void sl_zigbee_gpd_rail_start_tx_wrapper(bool skipCca,
                                         uint16_t channel);
void sl_zigbee_gpd_rail_write_tx_fifo_wrapper(const uint8_t *dataPtr,
                                              uint16_t writeLength);
void sl_zigbee_gpd_rail_start_rx_wrapper(uint8_t channel);
void sl_zigbee_gpd_rail_idle_wrapper(void);
uint16_t sl_zigbee_gpd_rail_get_radio_entropy_wrapper(uint8_t *dataPtr, uint16_t dataLength);
void sl_zigbee_gpd_rail_provide_rail_handle(sl_rail_handle_t handle);

// Security Function Prototypes
int8_t sl_zigbee_gpd_security_encrypt_key(sl_zigbee_gpd_addr_t_t * addr,
                                          const uint8_t* oobKey,
                                          uint8_t * oobScured,
                                          uint8_t * mic);
void sl_zigbee_gpd_security_init(sl_zigbee_gpd_addr_t_t * addr,
                                 uint8_t* securityKey,
                                 uint32_t frameCounter);
void sl_zigbee_gpd_set_fc_into_tx_nonce(uint32_t value);
void sl_zigbee_gpd_set_fc_into_rx_nonce(uint32_t value);

int8_t sl_zigbee_tag_tx_buff(uint8_t * data,
                             uint8_t length,
                             uint8_t authStart,
                             uint8_t authLength);

int8_t sl_zigbee_encrypt_and_tag_tx_buff(uint8_t* data,
                                         uint8_t length,
                                         uint8_t payloadLength,
                                         uint8_t authStart,
                                         uint8_t authLength);

int8_t sl_zigbee_gpd_mbedtls_ccm_setkey_wraper(const uint8_t * key);

int8_t sl_zigbee_gpd_security_decrypt_key(sl_zigbee_gpd_addr_t_t * addr,
                                          uint8_t * pKeyEncrypted,
                                          uint8_t * pKeyMic,
                                          uint8_t * pSecCounter,
                                          uint8_t * decryptedKey);

int8_t sl_zigbee_decrypt_rx_buffer(uint8_t * rxMpdu,
                                   uint8_t authStart,
                                   uint8_t encrLength,
                                   uint8_t authLength);
// Timer Functions
void sl_zigbee_gpd_le_timer_init(void);
void sl_zigbee_gpd_load_le_timer(uint32_t timeInUs);
bool sl_zigbee_gpd_le_timer_running(void);

// Utility Functions
void sl_zigbee_gpd_utility_copy_4_bytes(uint8_t * dst, uint32_t value);
bool sl_zigbee_gpd_utility_addr_match(sl_zigbee_gpd_addr_t_t * a1, sl_zigbee_gpd_addr_t_t * a2);

// AF Send Receive functions
uint8_t * sl_zigbee_gpd_get_rx_mpdu(void);

// Nwk Functions
uint8_t sl_zigbee_gpd_build_mpdu(uint8_t * mpdu,
                                 uint8_t * payload,
                                 uint8_t payloadLength,
                                 sl_zigbee_gpd_t_t * gpd);
uint8_t sl_zigbee_gpd_secure_outgoing_m_pdu(uint8_t * mpdu,
                                            uint8_t mpduLength,
                                            uint8_t payloadLength,
                                            sl_zigbee_gpd_security_level_t_t securityLevel);
uint8_t sl_zigbee_build_out_going_pdu(uint8_t frameType,
                                      uint8_t * mpdu,
                                      uint8_t * payload,
                                      uint8_t payloadLength,
                                      sl_zigbee_gpd_t_t * gpd);
// Commissioning
int8_t sl_zigbee_gpd_process_commissioning_reply(sl_zigbee_gpd_t_t * gpd,
                                                 uint8_t payload_size,
                                                 uint8_t * pRxbuffer);
// Gpd Node
sl_zigbee_gpd_t_t * sl_zigbee_gpd_init(void);
sl_zigbee_gpd_t_t * sl_zigbee_gpd_get_gpd(void);
void sl_zigbee_gpd_set_channel(uint8_t channel);
void sl_zigbee_gpd_set_state(uint8_t gpdState);
sl_zigbee_gpd_addr_t_t * sl_zigbee_gpd_get_addr(void);
void sl_zigbee_gpd_load_gpd_default_configuration(sl_zigbee_gpd_t_t * gpd);

// NV functions
void sl_zigbee_gpd_nv_init(void);
void sl_zigbee_gpd_save_gpd_frame_counter_to_shadow(uint32_t frameCounter);
void sl_zigbee_gpd_copy_to_shadow(sl_zigbee_gpd_t_t * gpd);
void sl_zigbee_gpd_copy_from_shadow(sl_zigbee_gpd_t_t * gpd);
void sl_zigbee_gpd_store_sec_data_to_nv(sl_zigbee_gpd_t_t * gpd);
void sl_zigbee_gpd_store_sec_data_from_nv(sl_zigbee_gpd_t_t * gpd);
void sl_zigbee_gpd_load_sec_data_from_nv(sl_zigbee_gpd_t_t * gpd);

//Incomming Command and Message Handler
void sl_zigbee_gpd_incoming_command_handler(uint8_t *gpdCommandBuffer,
                                            uint8_t length);
void sl_zigbee_gpd_incoming_message_handler(uint8_t * buffer,
                                            uint16_t length);

void sl_zigbee_gpd_af_plugin_commission(sl_zigbee_gpd_t_t * gpd);
void sl_zigbee_gpd_af_plugin_de_commission(sl_zigbee_gpd_t_t * gpd);
#endif
