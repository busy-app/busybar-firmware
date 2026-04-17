/***************************************************************************//**
 * @file
 * @brief
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

// This API mimics the halInternalGetTokenData(), halInternalSetTokenData()
// interface with multi-network support.
void sli_zigbee_stack_token_primitive(bool tokenRead,
                                      void* tokenStruct,
                                      uint32_t tokenAddress,
                                      uint8_t length);

#define sli_zigbee_get_stack_token(tokenStruct, tokenAddress, tokenSize) \
  sli_zigbee_stack_token_primitive(true, tokenStruct, tokenAddress, tokenSize)

#define sli_zigbee_set_stack_token(tokenAddress, tokenStruct, tokenSize) \
  sli_zigbee_stack_token_primitive(false, tokenStruct, tokenAddress, tokenSize)
