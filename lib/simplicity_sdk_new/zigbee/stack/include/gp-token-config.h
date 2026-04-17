/***************************************************************************//**
 * @file
 * @brief Zigbee GP token definitions used by the stack.
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

#define TOKEN_STACK_GP_DATA_DEFAULT { \
    0xFF,                             \
    0xFF,                             \
    0xFF,                             \
    0xFF,                             \
    0xFFFF,                           \
    0xFFFF,                           \
    0xFFFF,                           \
    { 0xFF, 0xFF, 0xFF, 0xFF,         \
      0xFF, 0xFF, 0xFF },             \
}
#define TOKEN_STACK_GP_PROXY_TABLE_DEFAULT { \
    0xFF,                                    \
    0xFFFFFFFFU,                             \
    { 0xFF, 0xFF, 0xFF, 0xFF,                \
      0xFF, 0xFF, 0xFF, 0xFF },              \
    0xFF,                                    \
    0xFF,                                    \
    { 0xFF, 0xFF, 0xFF, 0xFF,                \
      0xFF, 0xFF, 0xFF, 0xFF,                \
      0xFF, 0xFF, 0xFF, 0xFF,                \
      0xFF, 0xFF, 0xFF, 0xFF },              \
    { 0xFF, 0xFF },                          \
    {                                        \
      { 0xFF, 0xFF, 0xFF, 0xFF,              \
        0xFF, 0xFF, 0xFF, 0xFF },            \
      { 0xFF, 0xFF, 0xFF, 0xFF,              \
        0xFF, 0xFF, 0xFF, 0xFF },            \
    },                                       \
}
#define TOKEN_STACK_GP_INCOMING_FC_DEFAULT { 0x00000000U }
#define TOKEN_STACK_GP_SINK_TABLE_DEFAULT { \
    0xFF,                                   \
    0xFFFF,                                 \
    { 0xFF, 0xFF, 0xFF, 0xFF,               \
      0xFF, 0xFF, 0xFF, 0xFF },             \
    0xFF,                                   \
    0xFF,                                   \
    { 0xFF, 0xFF, 0xFF, 0xFF,               \
      0xFF, 0xFF, 0xFF, 0xFF,               \
      0xFF, 0xFF, 0xFF, 0xFF,               \
      0xFF, 0xFF, 0xFF, 0xFF },             \
    { 0xFF, 0xFF },                         \
    {                                       \
      { 0xFFFF, 0xFFFF },                   \
      { 0xFFFF, 0xFFFF },                   \
    },                                      \
    0xFFFF,                                 \
    0xFF,                                   \
    0xFF,                                   \
    0xFF                                    \
}
#define TOKEN_STACK_GP_INCOMING_FC_IN_SINK_DEFAULT { 0x00000000U }

#ifdef DEFINETYPES
typedef struct {
  uint8_t networkIndex;
  uint8_t nodeCapabilities;
  int8_t radioPower;
  uint8_t baseChannel;
  uint16_t localNodeId;
  uint16_t localPanId;
  uint16_t vendorId;
  uint8_t vendorString[7];
} tokTypeStackGpData;

typedef struct {
  uint8_t status;
  uint32_t options;
  //sl_zigbee_gp_address_t gpd;
  uint8_t gpAddress[8];
  uint8_t endpoint;
  //uint16_t assignedAlias;
  uint8_t securityOptions;
  uint8_t gpdKey[16]; // ignored if using Secure Key Storage (but moved to PSA and cleared if upgrade code is run)
  //sl_zigbee_gp_sink_list_entry_t sinkList[2];
  uint8_t sinkType[2];
  uint8_t sinkEUI[2][8];
  //uint16_t sinkNodeId[2];
} tokTypeStackGpProxyTableEntry;

typedef struct {
  uint8_t status;
  uint16_t options;
  //sl_zigbee_gp_address_t gpd;
  uint8_t gpAddress[8];
  uint8_t endpoint;
  uint8_t securityOptions;
  uint8_t gpdKey[16]; // ignored if using Secure Key Storage (but moved to PSA and cleared if upgrade code is run)
  uint8_t sinkType[2];
  uint16_t groupList[2][2];
  uint32_t securityFrameCounter; // This is no more used, Incoming FC for gpd in a separate Token to control its update.
  uint16_t assignedAlias;
  uint8_t deviceId;
  uint8_t groupcastRadius;
} tokTypeStackGpSinkTableEntry;

typedef uint32_t tokTypeGPDIncomingFC;
typedef uint32_t tokTypeGPDIncomingFCInSink;
#endif

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(STACK_GP_DATA,
                   tokTypeStackGpData,
                   TOKEN_STACK_GP_DATA_DEFAULT)
DEFINE_INDEXED_TOKEN(STACK_GP_PROXY_TABLE,
                     tokTypeStackGpProxyTableEntry,
                     SL_ZIGBEE_GP_PROXY_TABLE_SIZE,
                     TOKEN_STACK_GP_PROXY_TABLE_DEFAULT)

DEFINE_INDEXED_TOKEN(STACK_GP_INCOMING_FC,
                     tokTypeGPDIncomingFC,
                     SL_ZIGBEE_GP_INCOMING_FC_TOKEN_TABLE_SIZE,
                     TOKEN_STACK_GP_INCOMING_FC_DEFAULT)

// Sink Table and Sink table incoming FC tokens

DEFINE_INDEXED_TOKEN(STACK_GP_SINK_TABLE,
                     tokTypeStackGpSinkTableEntry,
                     SL_ZIGBEE_GP_SINK_TABLE_SIZE,
                     TOKEN_STACK_GP_SINK_TABLE_DEFAULT)

DEFINE_INDEXED_TOKEN(STACK_GP_INCOMING_FC_IN_SINK,
                     tokTypeGPDIncomingFCInSink,
                     SL_ZIGBEE_GP_SINK_TABLE_SIZE,
                     TOKEN_STACK_GP_INCOMING_FC_IN_SINK_DEFAULT)
#endif // DEFINETOKENS
