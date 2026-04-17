/*****************************************************************************/
/**
 * Copyright 2021 Silicon Laboratories, Inc.
 *
 *****************************************************************************/
//
// Originally generated using Studio and AFv6 apps, now hand mantained
//
// Description: Handlers for the EZSP frames that directly correspond to Ember
// API calls.

#include PLATFORM_HEADER
#include "stack/include/sl_zigbee_types.h"
#include "stack/internal/inc/internal-defs-patch.h"
#include "app/em260/command-context.h"
#include "stack/include/cbke-crypto-engine.h"
#include "stack/include/mfglib.h"
#include "stack/include/binding-table.h"
#include "app/util/ezsp/ezsp-frame-utilities.h"
#include "app/em260/command-handlers-cbke.h"
#include "app/em260/command-handlers-binding.h"
#include "app/em260/command-handlers-mfglib.h"
#include "app/em260/command-handlers-security.h"
#include "app/em260/command-handlers-zll.h"
#include "app/em260/command-handlers-zigbee-pro.h"
#include "child.h"
#include "message.h"
#include "zll-api.h"
#include "security.h"
#include "stack-info.h"
#include "network-formation.h"
#include "zigbee-device-stack.h"
#include "sl_zigbee_duty_cycle.h"
#include "multi-phy.h"

extern void halReboot(void);

#define SL_STATUS_SIZE sizeof(sl_status_t)
#define TOKEN_LENGTH_SIZE sizeof(tokenData.size)

// Extern for the auto generated handler as specified in the ezsp.yaml
extern bool sli_zigbee_af_process_ezsp_command_token_interface(uint16_t commandId);

// The handler that is not generated as specified in the ezsp.yaml
static void sl_zigbee_af_token_interface_set_or_get_ezsp_handler(bool set)
{
  uint32_t token = fetchInt32u();
  uint32_t index = fetchInt32u();
  if (!set) {
    sl_zigbee_token_data_t tokenData;
    tokenData.data = ezspWritePointer + SL_STATUS_SIZE + TOKEN_LENGTH_SIZE;
    sl_status_t status = sli_zigbee_stack_get_token_data(token,
                                                         index,
                                                         &tokenData);
    appendInt32u(status);
    appendInt32u(tokenData.size);
    ezspWritePointer = ezspWritePointer + tokenData.size;
  } else {
    sl_zigbee_token_data_t tokenData;
    tokenData.size = fetchInt32u();
    tokenData.data = ezspReadPointer;
    sl_status_t status = sli_zigbee_stack_set_token_data(token,
                                                         index,
                                                         &tokenData);
    appendInt32u(status);
  }
}

// Hook from command handler for the Token interface commands
bool sli_zigbee_af_process_ezsp_token_interface_commands(uint16_t commandId)
{
  switch (commandId) {
    case SL_ZIGBEE_EZSP_RESET_NODE:
      halReboot();
      break;
    case SL_ZIGBEE_EZSP_SET_TOKEN_DATA:
    case SL_ZIGBEE_EZSP_GET_TOKEN_DATA:
      sl_zigbee_af_token_interface_set_or_get_ezsp_handler(commandId
                                                           == SL_ZIGBEE_EZSP_SET_TOKEN_DATA);
      break;
    case SL_ZIGBEE_EZSP_TOKEN_FACTORY_RESET: {
      bool exclude_outgoing_fc;
      bool exclude_boot_counter;
      exclude_outgoing_fc = fetchInt8u();
      exclude_boot_counter = fetchInt8u();
      sli_zigbee_stack_token_factory_reset(exclude_outgoing_fc, exclude_boot_counter);
      break;
    }
    default:
      return sli_zigbee_af_process_ezsp_command_token_interface(commandId);
      break;
  }
  return true;
}
