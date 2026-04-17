/***************************************************************************//**
 * @file
 * @brief Definitions for stack tokens for Common Token Manager.
 * See @ref token_stack for documentation.
 *
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

#include "stack/include/sl_zigbee_types.h"
#include "sl_token_manager_defines.h"

/**
 * @name Common Token Object Key IDs
 * @brief The Common Token object key is used as a distinct identifier tag for a
 * token stored in Common Token Manager.
 *
 * Every token must have a defined Dynamic Token object key ID and the object key
 * ID must be unique. The object key ID defined must be in the following
 * format:
 * COMMON_TOKEN_tokenname
 *
 * where tokenname is the name of the token without COMMON_TOKEN_ prefix.

 *@{
 */

// STACK KEYS
#define COMMON_TOKEN_STACK_NVDATA_VERSION                         SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xFF01), 0)
#define COMMON_TOKEN_STACK_BOOT_COUNTER                           SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE263), 1)
#define COMMON_TOKEN_STACK_NONCE_COUNTER                          SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE563), 1)
#define COMMON_TOKEN_STACK_ANALYSIS_REBOOT                        SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE162), 0)
#define COMMON_TOKEN_STACK_KEYS                                   SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xEB79), 0)
#define COMMON_TOKEN_STACK_NODE_DATA                              SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xEE64), 0)
#define COMMON_TOKEN_STACK_CLASSIC_DATA                           SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE364), 0)
#define COMMON_TOKEN_STACK_ALTERNATE_KEY                          SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE475), 0)
#define COMMON_TOKEN_STACK_APS_FRAME_COUNTER                      SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE123), 1)
#define COMMON_TOKEN_STACK_TRUST_CENTER                           SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE124), 0)
#define COMMON_TOKEN_STACK_NETWORK_MANAGEMENT                     SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE125), 0)
#define COMMON_TOKEN_STACK_PARENT_INFO                            SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE126), 0)
#define COMMON_TOKEN_STACK_PARENT_ADDITIONAL_INFO                 SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE127), 0)
#define COMMON_TOKEN_STACK_MULTI_PHY_NWK_INFO                     SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE128), 0)
#define COMMON_TOKEN_STACK_MIN_RECEIVED_RSSI                      SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE129), 0)
// Restored EUI64
#define COMMON_TOKEN_STACK_RESTORED_EUI64                         SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE12A), 0)
// Stack context (ZCP or BDB)
#define COMMON_TOKEN_STACK_CONTEXT                                SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE12B), 0)
// This token is used to track some additional information of the TCLK on the joiner
#define COMMON_TOKEN_STACK_TRUST_CENTER_ADDITIONAL_INFO           SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE12C), 0)
// MULTI-NETWORK STACK KEYS
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_KEYS                     SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0000), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_NODE_DATA                SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0080), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_ALTERNATE_KEY            SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0100), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_TRUST_CENTER             SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0180), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_NETWORK_MANAGEMENT       SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0200), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_PARENT_INFO              SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0280), 0)

// Temporary solution for multi-network nwk counters:
// This counter will be used on the network with index 1.
#define COMMON_TOKEN_MULTI_NETWORK_STACK_NONCE_COUNTER            SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE220), 1)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define COMMON_TOKEN_MULTI_NETWORK_STACK_PARENT_ADDITIONAL_INFO   SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0300), 0)

// GP stack tokens.
#define COMMON_TOKEN_STACK_GP_DATA                                SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE258), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_GP_PROXY_TABLE                         SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0380), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_GP_SINK_TABLE                          SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0400), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_GP_INCOMING_FC                         SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0480), 0)

// APP KEYS
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_BINDING_TABLE                          SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0500), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_CHILD_TABLE                            SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0580), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_KEY_TABLE                              SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0600), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_CERTIFICATE_TABLE                      SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0680), 0)
#define COMMON_TOKEN_STACK_ZLL_DATA                               SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE501), 0)
#define COMMON_TOKEN_STACK_ZLL_SECURITY                           SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0xE502), 0)
// This key is used for an indexed token and the subsequent 0x7F keys are also reserved.
#define COMMON_TOKEN_STACK_ADDITIONAL_CHILD_DATA                  SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0700), 0)

// This key is used for an indexed token and the subsequent 0x7F keys are also reserved
#define COMMON_TOKEN_STACK_GP_INCOMING_FC_IN_SINK                 SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x0780), 0)
// This key is used for an indexed token that tracks additional information of the key table.
#define COMMON_TOKEN_STACK_KEY_TABLE_ADDITIONAL_INFO              SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x1000), 0)
// This key is used to store additional link keys beyond the limitations of the token system
#define COMMON_TOKEN_STACK_KEY_TABLE_EXTENDED                     SL_TOKEN_GET_DYNAMIC_TOKEN((SL_TOKEN_NVM3_REGION_ZIGBEE | 0x892C), 0)

/** @} END Common Token Object Key IDs  */

#define TOKEN_STACK_NVDATA_VERSION_DEFAULT           CURRENT_STACK_TOKEN_VERSION
#define TOKEN_STACK_APS_FRAME_COUNTER_DEFAULT        0x00000000
#define TOKEN_STACK_ALTERNATE_KEY_DEFAULT            STACK_ALT_NWK_KEY_DEFAULT_VALUE
#define TOKEN_STACK_BOOT_COUNTER_DEFAULT             0x0000
#define TOKEN_STACK_NONCE_COUNTER_DEFAULT            0x00000000
#define TOKEN_STACK_ANALYSIS_REBOOT_DEFAULT          0x0000
#define TOKEN_STACK_KEYS_DEFAULT                     STACK_KEYS_DEFAULT_VALUE
#define TOKEN_STACK_NODE_DATA_DEFAULT                { 0xFFFF, -1, 0, 0x00, 0x00, 0x0000, { 0 } }
#define TOKEN_STACK_TRUST_CENTER_DEFAULT             { 0, }
#define TOKEN_STACK_NETWORK_MANAGEMENT_DEFAULT       { 0, 0xFFFF, 0 }
#define TOKEN_STACK_PARENT_INFO_DEFAULT              { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 0xFFFF }
#define TOKEN_STACK_PARENT_ADDITIONAL_INFO_DEFAULT   0x0000
#define TOKEN_STACK_MULTI_PHY_NWK_INFO_DEFAULT       { -1, 0, 0, 0, 0, 0, 0x80 }
#define TOKEN_STACK_MIN_RECEIVED_RSSI_DEFAULT        { -100, }
#define TOKEN_STACK_RESTORED_EUI64_DEFAULT           { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF } }

#define TOKEN_MULTI_NETWORK_STACK_KEYS_DEFAULT                     STACK_KEYS_DEFAULT_VALUE
#define TOKEN_MULTI_NETWORK_STACK_NODE_DATA_DEFAULT                { 0, }
#define TOKEN_MULTI_NETWORK_STACK_ALTERNATE_KEY_DEFAULT            STACK_ALT_NWK_KEY_DEFAULT_VALUE
#define TOKEN_MULTI_NETWORK_STACK_TRUST_CENTER_DEFAULT             { 0, }
#define TOKEN_MULTI_NETWORK_STACK_NETWORK_MANAGEMENT_DEFAULT       { 0, }
#define TOKEN_MULTI_NETWORK_STACK_PARENT_INFO_DEFAULT              { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, 0xFFFF }

#define TOKEN_MULTI_NETWORK_STACK_NONCE_COUNTER_DEFAULT            0x00000000
#define TOKEN_MULTI_NETWORK_STACK_PARENT_ADDITIONAL_INFO_DEFAULT   { 0, }

#define TOKEN_STACK_BINDING_TABLE_DEFAULT   { 0, }
#define TOKEN_STACK_CHILD_TABLE_DEFAULT     { 0, }

#define TOKEN_STACK_KEY_TABLE_DEFAULT            { 0, }
#define TOKEN_STACK_KEY_TABLE_EXTENDED_DEFAULT   { 0, }

#define TOKEN_STACK_CERTIFICATE_TABLE_DEFAULT       { 0, }
#define TOKEN_STACK_ADDITIONAL_CHILD_DATA_DEFAULT   { 0x0F, }
#define TOKEN_STACK_CONTEXT_DEFAULT             { 0, }
#define TOKEN_STACK_TRUST_CENTER_ADDITIONAL_INFO_DEFAULT  { 0, }
#define TOKEN_STACK_KEY_TABLE_ADDITIONAL_INFO_DEFAULT  { 0, }
//////////////////////////////////////////////////////////////////////////////
// MANUFACTURING DATA
#if !defined(EZSP_HOST) && !defined(ZIGBEE_STACK_ON_HOST) && !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#include "sl_token_manager_manufacturing.h"
#endif // !EZSP_HOST

//////////////////////////////////////////////////////////////////////////////
// STACK DATA

/**
 * @brief The current version number of the stack tokens.
 * MSB is the version, LSB is a complement.
 *
 * See hal/micro/token.h for a more complete explanation.
 */
#define CURRENT_STACK_TOKEN_VERSION 0x03FC //MSB is version, LSB is complement.

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
typedef uint16_t tokTypeStackNvdataVersion;
typedef uint32_t tokTypeStackBootCounter;
//This was introduced to save the type of keep alives supported by the parent.
//Bits 0-2 are currently in use, bit 2 covers power negotiation.
//Bits 3-14 are reserved. Bit 15 is used in alt-mac
typedef uint16_t tokTypeStackParentAdditionalInfo;
typedef uint16_t tokTypeStackAnalysisReboot;
typedef uint32_t tokTypeStackNonceCounter;
typedef int8_t tokTypeRSSI;
typedef int8_t tokStackContext;
typedef uint32_t tokStackTrustCenterAdditionalInfo;
typedef uint32_t tokStackKeyTableAdditionalInfo;
// While capable series 2 parts save keys in Secure Vault, the token layout is
// left untouched. If keys have been moved to PSA, the key values in the below
// tokens are zeroed out and unused. Some tokens have bits to represent that the
// token holds an ID that points to PSA.

typedef struct {
  uint8_t networkKey[16]; // ignored if using Secure Key Storage (but moved to PSA and cleared if upgrade code is run)
  uint8_t activeKeySeqNum;
} tokTypeStackKeys;

typedef struct {
  uint16_t panId;
  int8_t radioTxPower;
  uint8_t radioFreqChannel;
  uint8_t stackProfile;
  uint8_t nodeType;
  uint16_t zigbeeNodeId;
  uint8_t extendedPanId[8];
} tokTypeStackNodeData;
typedef struct {
  uint16_t mode;
  uint8_t eui64[8];
  uint8_t key[16];  // ignored if using Secure Key Storage
} tokTypeStackTrustCenter;
typedef struct {
  uint32_t activeChannels;
  uint16_t managerNodeId;
  uint8_t updateId;
} tokTypeStackNetworkManagement;
typedef struct {
  uint8_t parentEui[8];
  uint16_t parentNodeId;
} tokTypeStackParentInfo;
typedef struct {
  int8_t radioTxPower;
  uint8_t radioFreqChannel;
  uint32_t pg28ChannelMask;
  uint32_t pg29ChannelMask;
  uint32_t pg30ChannelMask;
  uint32_t pg31ChannelMask;
  uint8_t optionsMask;
} tokTypeStackMultiPhyNwkInfo;
typedef struct {
  uint8_t restoredEui[8];
} tokTypeStackRestoredEui64;
#endif

#define STACK_KEYS_DEFAULT_VALUE        { { 0, }, 0 }
#define STACK_ALT_NWK_KEY_DEFAULT_VALUE { { 0, }, 0 }

//////////////////////////////////////////////////////////////////////////////
// MULTI-NETWORK STACK TOKENS: Tokens for the networks with index > 0.
// The 0-index network info is stored in the usual tokens.

#define EXTRA_NETWORKS_NUMBER (SL_ZIGBEE_SUPPORTED_NETWORKS - 1)

//////////////////////////////////////////////////////////////////////////////
// APPLICATION DATA
// *If a fixed application token is desired, its address must be above 384.*

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
typedef uint8_t tokTypeStackBindingTable[13];
typedef uint8_t tokTypeStackChildTable[11];
typedef uint8_t tokTypeStackKeyTable[25];
#endif
#define KEY_TABLE_ENTRY_SIZE       25 // 8-byte IEEE + 16-byte Key + 1-byte info
#define KEY_ENTRY_IEEE_OFFSET       0
#define KEY_ENTRY_KEY_DATA_OFFSET   8 // first 4 bytes may point to PSA ID if data[KEY_ENTRY_INFO_OFFSET] & KEY_TABLE_ENTRY_HAS_PSA_ID
#define KEY_ENTRY_INFO_OFFSET      24

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
typedef uint8_t tokTypeStackAdditionalChildData;
#endif
// Certificate Table Entry
//   Certificate:    48-bytes
//   CA Public Key:  22-bytes
//   Private Key:    21-bytes
//   Flags:          1-byte
#define TOKEN_CERTIFICATE_TABLE_ENTRY_SIZE (48 + 22 + 21 + 1)
#define TOKEN_CERTIFICATE_TABLE_ENTRY_FLAGS_INDEX (TOKEN_CERTIFICATE_TABLE_ENTRY_SIZE - 1)
#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
typedef uint8_t tokTypeStackCertificateTable[TOKEN_CERTIFICATE_TABLE_ENTRY_SIZE];
#endif

// These must appear before the application header so that the token
// numbering is consistent regardless of whether application tokens are
// defined.
#include "stack/zll/zll-token-config.h"
#define ZLL_TOKENS_PRESENT
#include "stack/core/multi-pan-token-config.h"

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
// NVM3KEY domain base keys
#define NVM3KEY_DOMAIN_USER    0x00000U
#define NVM3KEY_DOMAIN_ZIGBEE  0x10000U
#define NVM3KEY_DOMAIN_COMMON  0x80000U

#define DEFINE_BASIC_TOKEN(name, type, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

#define DEFINE_COUNTER_TOKEN(name, type, ...) \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1, __VA_ARGS__)

#define DEFINE_INDEXED_TOKEN(name, type, arraysize, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize), __VA_ARGS__)

#define DEFINE_FIXED_BASIC_TOKEN(name, type, address, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

#define DEFINE_FIXED_COUNTER_TOKEN(name, type, address, ...) \
  TOKEN_DEF(name, CREATOR_##name, 1, 0, type, 1, __VA_ARGS__)

#define DEFINE_FIXED_INDEXED_TOKEN(name, type, arraysize, address, ...) \
  TOKEN_DEF(name, CREATOR_##name, 0, 1, type, (arraysize), __VA_ARGS__)

#define DEFINE_MFG_TOKEN(name, type, address, ...) \
  TOKEN_MFG(name, CREATOR_##name, 0, 0, type, 1, __VA_ARGS__)

#endif

#if !defined(EZSP_HOST)
#if defined(SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(JAM_BUILT_GP_LIB) || defined(SL_ZIGBEE_TEST)
  #include "stack/include/gp-token-config.h"
  #define GP_TOKENS_PRESENT
#endif // (SL_CATALOG_ZIGBEE_GREEN_POWER_PRESENT) || defined(JAM_BUILT_GP_LIB) || (SL_ZIGBEE_TEST)
#endif //!EZSP_HOST

#if !defined(SL_CATALOG_TOKEN_MANAGER_PRESENT)
#if !defined(SL_ZIGBEE_SCRIPTED_TEST)
 #if (defined(SL_CATALOG_ZIGBEE_ZCL_FRAMEWORK_CORE_PRESENT) \
  || (defined(SL_CATALOG_ZIGBEE_GREEN_POWER_ADAPTER_PRESENT)))
   #define APP_FRAMEWORK_TOKEN_HEADER "app/framework/util/af-token.h"
 # elif defined(SL_CATALOG_ZIGBEE_NCP_FRAMEWORK_PRESENT)
   #define APP_FRAMEWORK_TOKEN_HEADER "app/framework/util/af-ncp-token.h"
 #endif

#if defined(APP_FRAMEWORK_TOKEN_HEADER)

#define DEFINETYPES
#include APP_FRAMEWORK_TOKEN_HEADER
#undef DEFINETYPES

#define DEFINETOKENS

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  TOKEN_##name,
enum {
    #include APP_FRAMEWORK_TOKEN_HEADER
  APP_FRAMEWORK_TOKEN_COUNT
};
#undef TOKEN_DEF

#if APP_FRAMEWORK_TOKEN_COUNT
//Define the size (in bytes) of all tokens/
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  TOKEN_##name##_SIZE = sizeof(type),
enum {
  #include APP_FRAMEWORK_TOKEN_HEADER
};
#undef TOKEN_DEF
#endif

#undef DEFINETOKENS

#endif // APP_FRAMEWORK_TOKEN_HEADER

#else // SL_ZIGBEE_SCRIPTED_TEST

#if defined(APPLICATION_TOKEN_HEADER)

#define DEFINETYPES
#include APPLICATION_TOKEN_HEADER
#undef DEFINETYPES

#define DEFINETOKENS

#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  TOKEN_##name,
enum {
    #include APPLICATION_TOKEN_HEADER
  APPLICATION_TOKEN_COUNT
};
#undef TOKEN_DEF

#if APPLICATION_TOKEN_COUNT
//Define the size (in bytes) of all tokens/
#define TOKEN_DEF(name, creator, iscnt, isidx, type, arraysize, ...) \
  TOKEN_##name##_SIZE = sizeof(type),
enum {
  #include APPLICATION_TOKEN_HEADER
};
#undef TOKEN_DEF
#endif

#undef DEFINETOKENS

#endif // APPLICATION_TOKEN_HEADER

#endif // !SL_ZIGBEE_SCRIPTED_TEST

#endif
