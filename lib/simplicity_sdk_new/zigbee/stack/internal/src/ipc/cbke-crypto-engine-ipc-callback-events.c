/***************************************************************************//**
 * @file cbke-crypto-engine-ipc-callback-events.c
 * @brief callback event handlers for cbke-crypto-engine
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
// automatically generated from cbke-crypto-engine.h.  Do not manually edit
#include "stack/internal/src/ipc/cbke-crypto-engine-ipc-callback-events.h"
#include "stack/internal/src/ipc/zigbee_ipc_callback_events.h"
#include "stack/include/multi-network.h"
extern void sl_zigbee_wakeup_common_task(void);
extern sl_status_t sl_zigbee_af_push_network_index(uint8_t networkIndex);
extern sl_status_t sl_zigbee_af_pop_network_index(void);

void sli_zigbee_stack_calculate_smacs_283k1_handler(sl_status_t status,
                                                    sl_zigbee_smac_data_t *initiatorSmac,
                                                    sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.calculate_smacs_283k1_handler.status = status;

  if (initiatorSmac != NULL) {
    cb_event->data.calculate_smacs_283k1_handler.initiatorSmac = *initiatorSmac;
  }

  if (responderSmac != NULL) {
    cb_event->data.calculate_smacs_283k1_handler.responderSmac = *responderSmac;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_CALCULATE_SMACS_283K1_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_calculate_smacs_handler(sl_status_t status,
                                              sl_zigbee_smac_data_t *initiatorSmac,
                                              sl_zigbee_smac_data_t *responderSmac)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.calculate_smacs_handler.status = status;

  if (initiatorSmac != NULL) {
    cb_event->data.calculate_smacs_handler.initiatorSmac = *initiatorSmac;
  }

  if (responderSmac != NULL) {
    cb_event->data.calculate_smacs_handler.responderSmac = *responderSmac;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_CALCULATE_SMACS_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_dsa_sign_handler(sl_status_t status,
                                       uint8_t messageLength,
                                       uint8_t *messageContents)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.dsa_sign_handler.status = status;
  cb_event->data.dsa_sign_handler.messageLength = messageLength;

  if (messageContents != NULL) {
    memmove(cb_event->data.dsa_sign_handler.messageContents, messageContents, sizeof(uint8_t) * messageLength);
  }

  cb_event->tag = SLI_ZIGBEE_STACK_DSA_SIGN_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_dsa_verify_handler(sl_status_t status)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.dsa_verify_handler.status = status;
  cb_event->tag = SLI_ZIGBEE_STACK_DSA_VERIFY_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_generate_cbke_keys_283k1_handler(sl_status_t status,
                                                       sl_zigbee_public_key_283k1_data_t *ephemeralPublicKey)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.generate_cbke_keys_283k1_handler.status = status;

  if (ephemeralPublicKey != NULL) {
    cb_event->data.generate_cbke_keys_283k1_handler.ephemeralPublicKey = *ephemeralPublicKey;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_GENERATE_CBKE_KEYS_283K1_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_stack_generate_cbke_keys_handler(sl_status_t status,
                                                 sl_zigbee_public_key_data_t *ephemeralPublicKey)
{
  sl_zigbee_stack_cb_event_t *cb_event = (sl_zigbee_stack_cb_event_t *) malloc(sizeof(sl_zigbee_stack_cb_event_t));
  cb_event->data.generate_cbke_keys_handler.status = status;

  if (ephemeralPublicKey != NULL) {
    cb_event->data.generate_cbke_keys_handler.ephemeralPublicKey = *ephemeralPublicKey;
  }

  cb_event->tag = SLI_ZIGBEE_STACK_GENERATE_CBKE_KEYS_HANDLER_IPC_EVENT_TYPE;
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  cb_event->network_idx = sl_zigbee_get_callback_network();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_event_publish(&sli_zigbee_ipc_publisher, SL_EVENT_CLASS_ZIGBEE, 1 /*priority*/, cb_event);
  sl_zigbee_wakeup_common_task();
}

void sli_zigbee_cbke_crypto_engine_process_ipc_event(sl_zigbee_stack_cb_event_t *cb_event)
{
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  (void)sl_zigbee_af_push_network_index(cb_event->network_idx);
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  switch (cb_event->tag) {
    case SLI_ZIGBEE_STACK_CALCULATE_SMACS_283K1_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_calculate_smacs_283k1_handler(cb_event->data.calculate_smacs_283k1_handler.status,
                                              &cb_event->data.calculate_smacs_283k1_handler.initiatorSmac,
                                              &cb_event->data.calculate_smacs_283k1_handler.responderSmac);
      break;

    case SLI_ZIGBEE_STACK_CALCULATE_SMACS_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_calculate_smacs_handler(cb_event->data.calculate_smacs_handler.status,
                                        &cb_event->data.calculate_smacs_handler.initiatorSmac,
                                        &cb_event->data.calculate_smacs_handler.responderSmac);
      break;

    case SLI_ZIGBEE_STACK_DSA_SIGN_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_dsa_sign_handler(cb_event->data.dsa_sign_handler.status,
                                 cb_event->data.dsa_sign_handler.messageLength,
                                 cb_event->data.dsa_sign_handler.messageContents);
      break;

    case SLI_ZIGBEE_STACK_DSA_VERIFY_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_dsa_verify_handler(cb_event->data.dsa_verify_handler.status);
      break;

    case SLI_ZIGBEE_STACK_GENERATE_CBKE_KEYS_283K1_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_generate_cbke_keys_283k1_handler(cb_event->data.generate_cbke_keys_283k1_handler.status,
                                                 &cb_event->data.generate_cbke_keys_283k1_handler.ephemeralPublicKey);
      break;

    case SLI_ZIGBEE_STACK_GENERATE_CBKE_KEYS_HANDLER_IPC_EVENT_TYPE:
      sl_zigbee_generate_cbke_keys_handler(cb_event->data.generate_cbke_keys_handler.status,
                                           &cb_event->data.generate_cbke_keys_handler.ephemeralPublicKey);
      break;

    default:
      /* do nothing */
      break;
  }
  #ifndef SL_ZIGBEE_MULTI_NETWORK_STRIPPED
  sl_zigbee_af_pop_network_index();
  #endif // !SL_ZIGBEE_MULTI_NETWORK_STRIPPED
}
