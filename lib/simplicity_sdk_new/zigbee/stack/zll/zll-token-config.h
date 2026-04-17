/***************************************************************************//**
 * @file
 * @brief ZigBee Light Link token definitions used by the stack.
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

#define TOKEN_STACK_ZLL_DATA_DEFAULT { \
    SL_ZIGBEE_ZLL_STATE_FACTORY_NEW,   \
    0x0000,                            \
    0x0000,                            \
    0x0000,                            \
    0x0000,                            \
    0x0000,                            \
    0                                  \
}

#define TOKEN_STACK_ZLL_SECURITY_DEFAULT {              \
    0x00000000,                                         \
    0x00,                                               \
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, \
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, \
}

#ifdef DEFINETYPES
typedef struct {
  uint32_t bitmask;
  uint16_t freeNodeIdMin;
  uint16_t freeNodeIdMax;
  uint16_t myGroupIdMin;
  uint16_t freeGroupIdMin;
  uint16_t freeGroupIdMax;
  uint8_t rssiCorrection;
} tokTypeStackZllData;

typedef struct {
  uint32_t bitmask;
  uint8_t keyIndex;
  uint8_t encryptionKey[16]; // ignored if using Secure Key Storage
  uint8_t preconfiguredKey[16]; // ignored if using Secure Key Storage
} tokTypeStackZllSecurity;
#endif

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(STACK_ZLL_DATA,
                   tokTypeStackZllData,
                   TOKEN_STACK_ZLL_DATA_DEFAULT)

DEFINE_BASIC_TOKEN(STACK_ZLL_SECURITY,
                   tokTypeStackZllSecurity,
                   TOKEN_STACK_ZLL_SECURITY_DEFAULT)
#endif // DEFINETOKENS
