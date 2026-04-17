/***************************************************************************//**
 * @file app_process.c
 * @brief Implements application specific functions.
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
//                                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "app_init.h"
#include "gpd-components-common.h"
#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif
#include "sl_sleeptimer.h"
#include "nvm3.h"
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#include "sl_zigbee_debug_print.h"
#endif // SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT

#ifdef SL_CATALOG_SIMPLE_BUTTON_PRESENT
#include "sl_simple_button_instances.h"
#ifdef SL_CATALOG_SIMPLE_BUTTON_BTN0_PRESENT
#include "sl_simple_button_btn0_config.h"
#define BUTTON_INSTANCE_0 sl_button_btn0
#endif //SL_CATALOG_SIMPLE_BUTTON_BTN0_PRESENT
#ifdef SL_CATALOG_SIMPLE_BUTTON_BTN1_PRESENT
#include "sl_simple_button_btn1_config.h"
#define BUTTON_INSTANCE_1 sl_button_btn1
#endif //SL_CATALOG_SIMPLE_BUTTON_BTN1_PRESENT
#endif //SL_CATALOG_SIMPLE_BUTTON_PRESENT

#ifdef SL_CATALOG_SIMPLE_LED_PRESENT
#include "sl_simple_led_instances.h"
// LED Indication
#ifdef SL_CATALOG_SIMPLE_LED_LED0_PRESENT
#define ACTIVITY_LED sl_led_led0 //BOARDLED0
#endif //SL_CATALOG_SIMPLE_LED_LED0_PRESENT
#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
#define COMMISSIONING_STATE_LED sl_led_led1 //BOARDLED1
#else
#define COMMISSIONING_STATE_LED sl_led_led0 //BOARDLED0
#endif

#define BOARD_LED_ON(led) sl_led_turn_on(&led)
#define BOARD_LED_OFF(led) sl_led_turn_off(&led)

#else
#define BOARD_LED_ON(led)
#define BOARD_LED_OFF(led)

#endif //SL_CATALOG_SIMPLE_LED_PRESENT

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------

#define SL_ZIGBEE_GPD_NV_DATA_TAG 0xA9A1

#define GPD_BUTTON_COUNT SL_SIMPLE_BUTTON_COUNT

// App button press event types
enum {
  APP_EVENT_ACTION_IDLE = 0,
  APP_EVENT_ACTION_SEND_COMMISSION = 0x01,
  APP_EVENT_ACTION_SEND_DECOMMISSION = 0x02,
  APP_EVENT_ACTION_SEND_TOGGLE = 0x03,
  APP_EVENT_ACTION_SEND_REPORT = 0x04,
};
typedef uint8_t GpdAppEventActionType;

#ifndef APP_SINGLE_EVENT_COMMISSIONING_LOOP_TIME_MS
#define APP_SINGLE_EVENT_COMMISSIONING_LOOP_TIME_MS 1000
#endif

#ifndef APP_GPD_REPORTING_TIME_MS
#define APP_GPD_REPORTING_TIME_MS 30000
#endif

// If debug print is enabled
#ifdef SL_CATALOG_ZIGBEE_DEBUG_PRINT_PRESENT
#define gpdDebugPrintf sl_zigbee_core_debug_print
#else
#define gpdDebugPrintf(...)
#endif

// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------

static void processAppEvent(sl_zigbee_gpd_t_t * gpd, GpdAppEventActionType *appAction);
static void leTimeCallback(sl_sleeptimer_timer_handle_t *handle, void *contextData);
static void appSingleEventCommissionTimer(sl_sleeptimer_timer_handle_t *handle, void *contextData);
static void reportTimeCallback(sl_sleeptimer_timer_handle_t *handle, void *contextData);
static void reportData(uint32_t reportInterval_ms);
static void sendReport(sl_zigbee_gpd_t_t * gpd);

// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------

static GpdAppEventActionType appAction;
static sl_sleeptimer_timer_handle_t le_timer_handle;
static sl_sleeptimer_timer_handle_t app_single_event_commission;
static sl_sleeptimer_timer_handle_t report_timer_handle;

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * CLI command to send a report
 *****************************************************************************/
void sl_zigbee_gpd_sensor_report_event(void)
{
  appAction = APP_EVENT_ACTION_SEND_REPORT;
}

/******************************************************************************
 * Sets the sleepy flag that will allow the module sleep untill a button
 * is pressed.
 *****************************************************************************/
void sl_zigbee_gpd_sensor_set_sleep(void)
{
  // enable sleep
}

/******************************************************************************
 * Application state machine, called infinitely
 *****************************************************************************/
void app_process_action(void)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  // GPD state LED indication if enabled.
  if (gpd->gpdState < SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL) {
    BOARD_LED_ON(COMMISSIONING_STATE_LED);
  } else {
    BOARD_LED_OFF(COMMISSIONING_STATE_LED);
  }
  // Periodic reporting in the operational state
  if (gpd->gpdState >= SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL) {
    reportData(APP_GPD_REPORTING_TIME_MS);
  }

  // Process application actions
  if (appAction != APP_EVENT_ACTION_IDLE) {
    processAppEvent(gpd, &appAction);
  }
}

void sl_zigbee_gpd_load_le_timer(uint32_t timeInUs)
{
  // in Hz => Time period in micro sec T = 1000000/f
  uint32_t f = sl_sleeptimer_get_timer_frequency();
  // ticks needed = (timeout needed in mico sec)/(T in micro sec) =  ((timeout needed in micro sec) * f)/1000000
  uint32_t tick = (timeInUs * f) / 1000000;
  sl_sleeptimer_restart_timer(&le_timer_handle,
                              tick,
                              leTimeCallback,
                              NULL,
                              0,
                              0);
}

bool sl_zigbee_gpd_le_timer_running(void)
{
  bool running = false;
  sl_sleeptimer_is_timer_running(&le_timer_handle, &running);
  return running;
}

// Application Commissioning that completes all the statest of the commissioning
void sl_zigbee_gpd_app_single_event_commission(void)
{
  sl_sleeptimer_restart_periodic_timer_ms(&app_single_event_commission,
                                          APP_SINGLE_EVENT_COMMISSIONING_LOOP_TIME_MS,
                                          appSingleEventCommissionTimer,
                                          NULL,
                                          0,
                                          0);
}

/** @brief Called from the imcomming command handler context for all the incoming
 *         command before the command handler handles it. based on the return code
 *         of this callback the internal command handler skips the processing.
 *
 * @param gpdCommand CommandId.
 * @param length length of the command
 * @param commandPayload The pointer to the commissioning reply payload.
 * Ver.: always
 *
 * @return true if application handled it.
 */
bool sl_zigbee_gpd_af_plugin_incoming_command_cb(uint8_t gpdCommand,
                                                 uint8_t length,
                                                 uint8_t * commandPayload)
{
  gpdDebugPrintf("RX: cmd=%02X len=%02X payload[", gpdCommand, length);
  if (commandPayload != NULL
      && length > 0
      && length < 0xFE) {
    for (int i = 0; i < length; i++) {
      gpdDebugPrintf("%02X", commandPayload[i]);
    }
  }
  gpdDebugPrintf("]\n");
  // Unused variable warning suppression when print is not available.
  (void)gpdCommand;
  (void)length;
  (void)commandPayload;
  return false;
}

// NVM Initialisation
void sl_zigbee_gpd_af_plugin_nv_init_cb(void)
{
  // Initialise the NV
  // use default NVM space and handle
  (void)nvm3_open(nvm3_defaultHandle, nvm3_defaultInit);

  // Do re-packing if needed
  if (nvm3_repackNeeded(nvm3_defaultHandle)) {
    (void)nvm3_repack(nvm3_defaultHandle);
  }
}

/** @brief Called to the application to give a chance to load or store the GPD Context
 *.        in a non volatile context. Thsi can help the application to use any other
 *         non volatile storage.
 *
 * @param nvmData The pointer to the data that needs saving or retrieving to or from
 *                the non-volatile memory.
 * @param sizeOfNvmData The size of the data non-volatile data.
 * @param loadStore indication wether to load or store the context.
 * Ver.: always
 *
 * @return true if application handled it.
 */
bool sl_zigbee_gpd_af_plugin_nv_save_and_load_cb(sl_zigbee_gpd_t_t * gpd,
                                                 uint8_t * nvmData,
                                                 uint8_t sizeOfNvmData,
                                                 EmebrGpdNvLoadStore_t loadStore)
{
  if (loadStore == EMEBER_GPD_AF_CALLBACK_LOAD_GPD_FROM_NVM) {
    uint32_t objectType;
    size_t dataLen;
    if (0 == nvm3_countObjects(nvm3_defaultHandle)
        || (nvm3_getObjectInfo(nvm3_defaultHandle,
                               SL_ZIGBEE_GPD_NV_DATA_TAG,
                               &objectType,
                               &dataLen) == SL_STATUS_OK
            && objectType != NVM3_OBJECTTYPE_DATA)) {
      // Fresh chip , erase, create a storage with default setting.
      // Erase all objects and write initial data to NVM3
      (void)nvm3_eraseAll(nvm3_defaultHandle);
      // First write to the NVM shadow so that it updated with default ones
      sl_zigbee_gpd_copy_to_shadow(gpd);
      // Write the data to NVM
      (void)nvm3_writeData(nvm3_defaultHandle,
                           SL_ZIGBEE_GPD_NV_DATA_TAG,
                           nvmData,
                           sizeOfNvmData);
    } else {
      (void)nvm3_readData(nvm3_defaultHandle,
                          SL_ZIGBEE_GPD_NV_DATA_TAG,
                          nvmData,
                          sizeOfNvmData);
    }
  } else if (loadStore == EMEBER_GPD_AF_CALLBACK_STORE_GPD_TO_NVM) {
    (void)nvm3_writeData(nvm3_defaultHandle,
                         SL_ZIGBEE_GPD_NV_DATA_TAG,
                         nvmData,
                         sizeOfNvmData);
  } else {
    // bad command
  }
  return false;
}

// -----------------------------------------------------------------------------
//                          Static Function Definitions
// -----------------------------------------------------------------------------

/******************************************************************************
 * Application event processing
 *****************************************************************************/
static void processAppEvent(sl_zigbee_gpd_t_t * gpd, GpdAppEventActionType *appAction)
{
  if (*appAction != APP_EVENT_ACTION_IDLE) {
    BOARD_LED_ON(ACTIVITY_LED);
    if (*appAction == APP_EVENT_ACTION_SEND_DECOMMISSION) {
      sl_zigbee_gpd_af_plugin_de_commission(gpd);
      sl_zigbee_gpd_store_sec_data_to_nv(gpd);
      gpdDebugPrintf("Decomm Cmd : ");
    } else if (*appAction == APP_EVENT_ACTION_SEND_COMMISSION) {
      sl_zigbee_gpd_af_plugin_commission(gpd);
      if (gpd->gpdState > SL_ZIGBEE_GPD_APP_STATE_NOT_COMMISSIONED) {
        sl_zigbee_gpd_store_sec_data_to_nv(gpd);
      }
      gpdDebugPrintf("Comm. Cmd : ");
    } else if (*appAction == APP_EVENT_ACTION_SEND_REPORT) {
      sendReport(gpd);
      sl_zigbee_gpd_store_sec_data_to_nv(gpd);
      gpdDebugPrintf("Reporting : ");
    }
    gpdDebugPrintf("Comm. State :%d\n", gpd->gpdState);
    *appAction = APP_EVENT_ACTION_IDLE;
    BOARD_LED_OFF(ACTIVITY_LED);
  }
}

static void leTimeCallback(sl_sleeptimer_timer_handle_t *handle, void *contextData)
{
  (void)contextData;
  sl_sleeptimer_stop_timer(handle);
}

static void appSingleEventCommissionTimer(sl_sleeptimer_timer_handle_t *handle, void *contextData)
{
  (void)contextData;
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  if (gpd->gpdState < SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL) {
    appAction = APP_EVENT_ACTION_SEND_COMMISSION;
  } else {
    sl_sleeptimer_stop_timer(handle);
  }
}

static void sendReport(sl_zigbee_gpd_t_t * gpd)
{
  uint8_t command[] = {
    GP_CMD_ATTRIBUTE_REPORTING,
    0x06, 0x04, // Cluster Id
    0x00, 0x00, // Attribute Id
    0x10,       // Attribute Type
    0x01        // Attribute Value
  };
  gpd->rxAfterTx = false;
  sl_zigbee_af_gpdf_send(SL_ZIGBEE_GPD_NWK_FC_FRAME_TYPE_DATA,
                         gpd,
                         command,
                         sizeof(command),
                         SL_ZIGBEE_AF_PLUGIN_APPS_CMD_RESEND_NUMBER);
}

static void reportData(uint32_t reportInterval_ms)
{
  bool running = false;
  sl_sleeptimer_is_timer_running(&report_timer_handle, &running);
  if (running == false) {
    sl_sleeptimer_restart_periodic_timer_ms(&report_timer_handle,
                                            reportInterval_ms,
                                            reportTimeCallback,
                                            NULL,
                                            0,
                                            0);
  }
}

static void reportTimeCallback(sl_sleeptimer_timer_handle_t *handle, void *contextData)
{
  (void)handle;
  (void)contextData;
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  if (gpd->gpdState >= SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL) {
    appAction = APP_EVENT_ACTION_SEND_REPORT;
  } else {
    sl_sleeptimer_stop_timer(handle);
  }
}

// Button Events
#if GPD_BUTTON_COUNT == 2

void sl_button_on_change(const sl_button_t *handle)
{
  if ((&BUTTON_INSTANCE_0 == handle)
      || (&BUTTON_INSTANCE_1 == handle)) {
    sl_button_state_t botton0State = sl_button_get_state(&BUTTON_INSTANCE_0);
    sl_button_state_t botton1State = sl_button_get_state(&BUTTON_INSTANCE_1);
    if (botton0State == SL_SIMPLE_BUTTON_PRESSED && botton1State == SL_SIMPLE_BUTTON_PRESSED) {
      appAction = APP_EVENT_ACTION_SEND_DECOMMISSION;
    } else if (botton0State == SL_SIMPLE_BUTTON_PRESSED) {
      appAction = APP_EVENT_ACTION_SEND_COMMISSION;
    } else if (botton1State == SL_SIMPLE_BUTTON_PRESSED) {
      appAction = APP_EVENT_ACTION_SEND_REPORT;
    }
  }
}

#elif GPD_BUTTON_COUNT == 1

static sl_sleeptimer_timer_handle_t button_release_timer_handle;
static void buttonReleaseTimeout(sl_sleeptimer_timer_handle_t *handle, void *contextData)
{
  (void)contextData;
  sl_sleeptimer_stop_timer(handle);
  appAction = APP_EVENT_ACTION_SEND_DECOMMISSION;
}
void sl_button_on_change(const sl_button_t *handle)
{
  sl_zigbee_gpd_t_t * gpd = sl_zigbee_gpd_get_gpd();
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED
      && (&BUTTON_INSTANCE_0 == handle)) {
    if (gpd->gpdState < SL_ZIGBEE_GPD_APP_STATE_OPERATIONAL) {
      appAction = APP_EVENT_ACTION_SEND_COMMISSION;
    } else {
      appAction = APP_EVENT_ACTION_SEND_REPORT;
    }
    sl_sleeptimer_restart_timer_ms(&button_release_timer_handle,
                                   5000,
                                   buttonReleaseTimeout,
                                   NULL,
                                   0,
                                   0);
  } else {
    sl_sleeptimer_stop_timer(&button_release_timer_handle);
  }
}

#endif // GPD_BUTTON_COUNT
