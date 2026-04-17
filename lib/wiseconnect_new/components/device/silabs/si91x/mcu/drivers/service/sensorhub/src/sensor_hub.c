/***************************************************************************/ /**
* @file sensor_hub.c
* @brief sensor_hub example
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 *    If you use this software In in a product,
 *    an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
/***************************************************************************
 * @brief: This file contains all the sensor-related operations.
 * Create and perform operations of the sensor/event manager and power tasks.
 * It will Perform the OS-related operations for the sensors
 * This file will act as a service layer between sensors and user applications.
 * It will perform the operations based on user configuration data.
 ******************************************************************************/

#include <string.h>
#include "sensor_hub.h"
#include "rsi_debug.h"
#include "rsi_os.h"
#include "hub_hal_intf.h"
#include "cmsis_os2.h"
#include "timers.h"
#include "sl_si91x_adc.h"
#include "adc_sensor_driver.h"
#include "sensorhub_error_codes.h"
#include "rsi_ps_ram_func.h"
#include "rsi_m4.h"
#include "sl_si91x_m4_ps.h"
#include "rsi_rom_clks.h"
#include "rsi_power_save.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_button_instances.h"
#include "sl_si91x_power_manager_wakeup_source_config.h"
#include "sl_si91x_gpio_common.h"
#include "sl_si91x_gpio.h"
#include "sl_gpio_board.h"
#include "sl_si91x_driver_gpio.h"
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"

/*******************************************************************************
 ********************** Sensor HUB Defines / Macros  ***************************
 ******************************************************************************/
#define SENSORS_RAM_SIZE            4096 //< This RAM is used the store all sensor's data
#define ALARM_WAKEUP_SOURCE         1    //< This is for the Alarm based wakup
#define GPIO_WAKEUP_SOURCE          0    //< This is for the UULP_GPIO based wakup
#define SL_SH_SOCLDOTURNONWAITTIME  31
#define SL_SH_PMUBUCKTURNONWAITTIME 31
#define SL_PWR_STATE_SWICTH_DONE    1
#define SL_SH_BUTTON_ENABLE         1
/*******************************************************************************
 ********************* Sleep&Wakeup Defines / Macros  **************************
 ******************************************************************************/
#define PS_EVENT_MASK                                                                                           \
  (SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS4 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS4   \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS3 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS3 \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_ENTERING_PS2 | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_PS2 \
   | SL_SI91X_POWER_MANAGER_EVENT_TRANSITION_LEAVING_SLEEP) // Ored value of event for which callback is subscribed
#define STACK_ADDRESS 0                                     //< Application Stack Start address
#define JUMP_CB_ADDRESS \
  (uint32_t) RSI_PS_RestoreCpuContext //< Restore the current CPU processing content from the (POP) stack
#define VECTOR_OFFSET 0               //< Vector offset address
#define MODE          3               //< Sleep/wakeup mode
#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
#if defined(SLI_SI917B0)
#define IVT_OFFSET_ADDR 0x8202000 /*<!Application IVT location VTOR offset for B0>  */
#else
#define IVT_OFFSET_ADDR 0x8212000 /*<!Application IVT location VTOR offset for A0>  */
#endif
#else
#define IVT_OFFSET_ADDR 0x8012000 /*<!Application IVT location VTOR offset for dual flash A0 and B0>  */
#endif
#if defined(SLI_SI917B0)
#define WKP_RAM_USAGE_LOCATION 0x24061EFC /*<!Bootloader RAM usage location upon wake up  for B0 */
#else
#define WKP_RAM_USAGE_LOCATION 0x24061000 /*<!Bootloader RAM usage location upon wake up for A0  */
#endif
/*******************************************************************************
 ******************* Event acknowledge Defines / Macros  ***********************
 ******************************************************************************/
#define SET_EVENT_ACK   1 ///< Setting the event acknowledgment bit while posting the data
#define CLEAR_EVENT_ACK 0 ///< Clear the event acknowledgment bit after getting the callback event from the application
#define EM_POST_TIME    osWaitForever ///< Time out(TICKS) for the EM post-event

static sl_si91x_gpio_pin_config_t sl_gpio_pin_config1 = { { SL_SI91X_UULP_GPIO_2_PORT, SL_SI91X_UULP_GPIO_2_PIN },
                                                          GPIO_INPUT };
#define AVL_INTR_NO 0 // available interrupt number
/*******************************************************************************
 ********************  Extern variables/structures   ***************************
 ******************************************************************************/
extern sl_sensor_impl_type_t sensor_impls[];                   //< Sensor operations Structure
extern ARM_DRIVER_I2C Driver_I2C2;                             //< I2C driver structure
extern ARM_DRIVER_SPI Driver_SSI_ULP_MASTER;                   //< SPI driver structure
extern sl_sensor_info_t sensor_hub_info_t[SL_MAX_NUM_SENSORS]; //< Sensor configuration structure
extern sl_bus_intf_config_t bus_intf_info;                     //< Bus interface configuration structure
sl_power_state_t sensorhub_current_powerstate = SL_SI91X_POWER_MANAGER_PS3;
osSemaphoreAttr_t sl_semaphore_attr_st;  //< Power task semaphore attributes
sl_sh_power_state_t sl_power_state_enum; //< Power state structure
osSemaphoreId_t sl_semaphore_em_task_id; //< EM task semaphore

extern uint8_t sdc_intr_done;
/*******************************************************************************
 ********************  Local static function prototypes   ***************************
 ******************************************************************************/
/**************************************************************************/ /**
 *  @fn       static uint8_t sensorhub_i2c_init()
 *  @brief       Initialize I2C Interface based on configuration
 *  @return      execution status
*******************************************************************************/
static int32_t sensorhub_i2c_init(void);
/**************************************************************************/ /**
*  @fn        static sl_status_t sensorhub_spi_init(void)
*  @brief       Initialize the SPI Interface based on the configuration.
*******************************************************************************/
static int32_t sensorhub_spi_init(void);
/**************************************************************************/ /**
 *  @fn     static void sensorhub_sensor_task(void)
 *  @brief       Task to handle the sensor operations.
*******************************************************************************/
static void sensorhub_sensor_task(void);
/**************************************************************************/ /**
 *  @fn        static void sensorhub_em_task(void)
 *  @brief       Task to handle the operations of the Event Manager(EM)
*******************************************************************************/
static void sensorhub_em_task(void);
/**************************************************************************/ /**
 *  @fn         static sl_status_t sensorhub_i2c_sensors_scan(uint16_t address)
 *  @brief       Scan the i2c sensors based on the address
 *  @param[in]   address Sensor address
 *  @return status 0 if successful, it will wait for the sensor response.
*******************************************************************************/
static int32_t sensorhub_i2c_sensors_scan(uint16_t address);
/**************************************************************************/ /**
 *  @fn         static void sensorhub_em_post_event(sl_sensor_id_en sensor_id,
 *  sl_sensorhub_event_en event, void* dataPtr, TickType_t ticks_to_wait)
 *  @brief       To post the events/notifications to event manager(EM) to be notified to the application
 *  @param[in]   sensor_id      id of the  sensor
 *  @param[in]   event          event/notifications
 *  @param[in]   dataPtr        pointer to the data
 *  @param[in]   ticks_to_wait  max time to wait for the post
*******************************************************************************/
static void sensorhub_em_post_event(sl_sensor_id_t sensor_id,
                                    sl_sensorhub_event_t event,
                                    void *dataPtr,
                                    TickType_t ticks_to_wait);
/**************************************************************************/ /**
 *  @fn         static sl_sensor_impl_type* sensorhub_get_sensor_implementation(int32_t sensor_id)
 *  @brief       To get the appropriate sensor driver implementation.
 *  @param[in]   sensor_id     ID of the target sensor
 *  @return      Reference to the sensor driver
*******************************************************************************/
static sl_sensor_impl_type_t *sensorhub_get_sensor_implementation(int32_t sensor_id);
/**************************************************************************/ /**
 *  @fn        static int32_t sensorhub_create_sensor_list_index(void)
 *  @brief       To create a sensor index in the list.
 * @return if successful, returns the sensor index
 *         else it will break the sensor list.
*******************************************************************************/
static int32_t sensorhub_create_sensor_list_index(void);
/**************************************************************************/ /**
 *  @fn      static int32_t sensorhub_get_sensor_index(sl_sensor_id_en sensor_id)
 *  @brief       To get the index of the sensor present in the list
 *  @param[in]   sensor_id     id of the target sensor
 *  @return if successful, returns the sensor index
 *         else it returns the 0XFF as a sensor index fail.
*******************************************************************************/
static uint32_t sensorhub_get_sensor_index(sl_sensor_id_t sensor_id);
/**************************************************************************/ /**
 *  @fn          static int32_t sensorhub_delete_sensor_list_index(sl_sensor_id_en sensor_id)
 *  @brief       To delete the sensor index from the list
 *  @param[in]   sensor_id     id of the target sensor
 *  @return      index of the sensor
*******************************************************************************/
static uint32_t sensorhub_delete_sensor_list_index(sl_sensor_id_t sensor_id);
/**************************************************************************/ /**
 *  @fn          static sl_sensor_info_t* sensorhub_get_sensor_info(sl_sensor_id_en sensor_id)
 *  @brief       To get the info on the configured sensor
 *  @param[in]   sensor_id     id of the target sensor
 *  @return      reference to the sensor info
*******************************************************************************/
static sl_sensor_info_t *sensorhub_get_sensor_info(sl_sensor_id_t sensor_id);
/**************************************************************************/ /**
 *  @fn          static void sensorhub_sensors_timer_cb(TimerHandle_t xTimer)
 *  @brief       software timer callback
 *  @param[in]   xTimer     handle to the timer
*******************************************************************************/
static void sensorhub_sensors_timer_cb(TimerHandle_t xTimer);
/**************************************************************************/ /**
 *  @fn          static void sensorhub_gpio_interrupt_config(uint16_t gpio_pin,sl_gpio_intr_type intr_type)
 *  @brief       Configuring the different types of NPSS GPIO interrupts.
 *  @param[in]   gpio_pin     GPIO pin number
 *  @param[in]   intr_type    type of interrupt.
*******************************************************************************/
static sl_status_t sensorhub_gpio_interrupt_config(uint16_t gpio_pin, sl_si91x_gpio_interrupt_config_flag_t intr_type);
/**************************************************************************/ /**
 *  @fn         static void sensorhub_gpio_interrupt_start(uint16_t gpio_pin)
 *  @brief       Enable and set the priority of GPIO interrupt.
 *  @param[in]   gpio_pin     GPIO pin number
*******************************************************************************/
static void sensorhub_gpio_interrupt_start(uint16_t gpio_pin);
/**************************************************************************/ /**
 *  @fn          static void sensorhub_gpio_interrupt_stop(uint16_t gpio_pin)
 *  @brief       Mask and Disable the GPIO interrupt.
 *  @param[in]   gpio_pin     GPIO pin number
*******************************************************************************/
static void sensorhub_gpio_interrupt_stop(uint16_t gpio_pin);

/**************************************************************************/ /**
 *  @fn          static uint8_t sensorhub_sdc_init(void)
 *  @brief       Initialize the sdc Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
#ifdef SH_SDC_ENABLE
static sl_status_t sensorhub_sdc_init(void);
#endif
/**************************************************************************/ /**
 *  @fn         static void sensorhub_sdc_config_params(sl_sdc_config_t * sdc_config)
 *  @brief       Initialize the sdc Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
#ifdef SH_SDC_ENABLE
static void sensorhub_sdc_config_params(sl_drv_sdc_config_t *sdc_config_st_p);
#endif
/**************************************************************************/ /**
 *  @fn          static sl_status_t sensorhub_adc_init(void)
 *  @brief       Initialize the ADC Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
static sl_status_t sensorhub_adc_init(void);
/*******************************************************************************
 ************************  Global structures   *********************************
 ******************************************************************************/
sl_sensor_list_t sensor_list;     //< Sensor List, to maintain info of sensors created
sl_intr_list_map_t int_list_map;  //< Map table for interrupt vs sensor*/
sl_sensorhub_errors_t bus_errors; //< structure to track the status of the sensor hub
/*******************************************************************************
 **************************  Global variables   ***********************************
 ******************************************************************************/
ARM_DRIVER_I2C *I2Cdrv       = &Driver_I2C2;           //< I2C driver operations
ARM_DRIVER_SPI *SPIdrv       = &Driver_SSI_ULP_MASTER; //< ULP SSI driver operations
uint8_t sl_sensor_wait_flags = 0;                      //< Store the sensor event bits

/*TODO: the sensor_data_ram must be mapped to ULP RAM*/
uint8_t sensor_data_ram[SENSORS_RAM_SIZE] __attribute__((aligned(4))); //< Ram using for the sensor data storage
uint32_t free_ram_index = 0;                                           //< ram index for the sensor
#ifdef SL_SH_ADC_CHANNEL0
extern uint16_t *adc_data_ptrs[];
#endif

/*******************************************************************************
 **************************  Callback function ***********************************
 ******************************************************************************/
sl_sensor_cb_info_t cb_info; //< sensor call back handler
/*******************************************************************************
 * ADC user callback
 * This function will be called from ADC interrupt handler
 *
 * @param[in] ADC channel number
 * @param[in] ADC callback event (ADC_STATIC_MODE_CALLBACK,
 *            ADC_THRSHOLD_CALLBACK, INTERNAL_DMA, FIFO_MODE_EVENT)
 *
*******************************************************************************/
void sl_si91x_adc_callback_v2(uint8_t channel_no, uint8_t event);

/*******************************************************************************
 **************************  Power Manager Variables ***************************
 ******************************************************************************/
static void transition_callback(sl_power_state_t from, sl_power_state_t to);
sl_power_manager_ps_transition_event_handle_t handle;
sl_power_manager_ps_transition_event_info_t info = { .event_mask = PS_EVENT_MASK, .on_event = transition_callback };

/*******************************************************************************
 ******************  CMSIS OS handlers/Variables   *****************************
 ******************************************************************************/
QueueHandle_t sl_event_queue_handler; //< Event queue handler

SemaphoreHandle_t sl_event_queue_mutex;   //< Event Mutux handler
SemaphoreHandle_t sl_sensor_mutex = NULL; //< Sensor Mutux handler

osMessageQueueAttr_t sl_osMessageQueueAttr; //< OS message queue attributes
osMutexAttr_t sl_em_osMutexAttr_t;          //< EM task mutux attributes
osMutexAttr_t sl_osMutexAttr_t;             //< Sensor task mutux attributes
osEventFlagsAttr_t sl_eventFlagsAttr;       //< Event creation attributes
EventGroupHandle_t sl_event_group = NULL;   //< Event group handler

osSemaphoreId_t sl_semaphore_power_task_id; //< Power task semaphore id
osSemaphoreAttr_t sl_semaphore_attr_st;     //< Power task semaphore attributes

/*******************************************************************************
 **************  Sensor Task Attributes structure for thread   *****************
 ******************************************************************************/
const osThreadAttr_t sensor_thread_attributes = {
  .name       = "Sensor Task", //< Name of thread
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = SL_SH_SENSOR_TASK_STACK_SIZE, //< Stack size of sensor task
  .priority   = osPriorityLow1,               //< Priority of Sensor task
  .tz_module  = 0,
  .reserved   = 0,
};

/*******************************************************************************
 **************  Event Manger Task Attributes structure for thread   ***********
 ******************************************************************************/
const osThreadAttr_t EM_thread_attributes = {
  .name       = "EM Task", //< Name of thread
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = SL_SH_EM_TASK_STACK_SIZE, //< Stack size of EM task
  .priority   = osPriorityLow2,           //< Priority of EM task
  .tz_module  = 0,
  .reserved   = 0,
};

/**************************************************************************/ /**
 * @fn           void RTC_ALARM_IRQHandler(void)
 * @brief        This function is to initialize the RTC alarm IRQHandler.
 * @param[in]    None
 * @param[out]   None
*******************************************************************************/
void RTC_ALARM_IRQHandler(void)
{
  volatile uint32_t statusRead = 0;
  /*Get the interrupt status */
  statusRead = RSI_RTC_GetIntrStatus();

  if (statusRead & NPSS_TO_MCU_ALARM_INTR) {
    /*Clear wake-up interrupt */
    RSI_RTC_IntrClear(RTC_ALARM_INTR);
  }
  return;
}

/* *******************************************************************************
 * Callback function for state transition.
 * Prints the state transition with the parameters from and to.
 ******************************************************************************/
static void transition_callback(sl_power_state_t from, sl_power_state_t to)
{
  // DEBUGINIT needs ULP Uart to be enabled in low power states.
  // This is a demonstration and it is not necessary to have debugout in callback function.
  // This callback function can be used to perform any activities after state transitions.
  DEBUGINIT();
  switch (from) {
    case SL_SI91X_POWER_MANAGER_PS4:
      // Previous state was PS4
      DEBUGOUT("Leaving PS4 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      // Previous state was PS3
      DEBUGOUT("Leaving PS3 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS2:
      // Previous state was PS2
      DEBUGOUT("Leaving PS2 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS1:
      // Wakeup from PS1
      DEBUGOUT("Leaving PS1 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_SLEEP:
      // Wakeup from sleep
      bus_errors.peripheral_global_status = 1;
      bus_errors.adc                      = false;
      bus_errors.i2c                      = false;
      bus_errors.spi                      = false;
      break;
    case SL_SI91X_POWER_MANAGER_STANDBY:
      // Wakeup from standby
      DEBUGOUT("Leaving Standby State \n");
      break;
    default:
      break;
  }

  switch (to) {
    case SL_SI91X_POWER_MANAGER_PS4:
      // Current state is PS4
      //      DEBUGOUT("Entering PS4 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS3:
      // Current state is PS3
      DEBUGOUT("Entering PS3 State \n");
      break;
    case SL_SI91X_POWER_MANAGER_PS2:
      // Current state is PS2
      DEBUGOUT("Entering PS2 State \n");
      break;
    default:
      break;
  }
}

/**************************************************************************/ /**
 *  @fn         static sl_sensor_impl_type* sensorhub_get_sensor_implementation(int32_t sensor_id)
 *  @brief       To get the appropriate sensor driver implementation.
 *  @param[in]   sensor_id     ID of the target sensor
 *  @return      Reference to the sensor driver
*******************************************************************************/
static sl_sensor_impl_type_t *sensorhub_get_sensor_implementation(int32_t sensor_id)
{
  uint32_t count = sl_si91x_get_implementation_size();

  for (uint32_t i = 0; i < count; i++) {
    if (sensor_impls[i].type == sensor_id) {
      return &sensor_impls[i];
    }
  }
  return NULL;
}

/**************************************************************************/ /**
 *  @fn        static int32_t sensorhub_create_sensor_list_index(void)
 *  @brief       To create a sensor index in the list.
 * @return if successful, returns the sensor index
 *         else it will break the sensor list.
*******************************************************************************/
static int32_t sensorhub_create_sensor_list_index(void)
{
  int32_t sensor_index;

  for (sensor_index = 0; sensor_index < SL_MAX_NUM_SENSORS; sensor_index++) {
    if (SL_SENSOR_INVALID == sensor_list.sl_sensors_st[sensor_index].sensor_status) {
      break;
    }
  }
  return sensor_index;
}

/**************************************************************************/ /**
 *  @fn      static    int32_t sensorhub_get_sensor_index(sl_sensor_id_en sensor_id)
 *  @brief       To get the index of the sensor present in the list
 *  @param[in]   sensor_id     id of the target sensor
 *  @return if successful, returns the sensor index
 *         else it returns the 0XFF as a sensor index fail.
*******************************************************************************/
static uint32_t sensorhub_get_sensor_index(sl_sensor_id_t sensor_id)
{
  uint32_t i;
  for (i = 0; i < SL_MAX_NUM_SENSORS; i++) {
    if ((sensor_list.sl_sensors_st[i].config_st->sensor_id == sensor_id)
        && (sensor_list.sl_sensors_st[i].sensor_status != SL_SENSOR_INVALID)) {
      return i;
    }
  }
  return SL_SH_SENSOR_INDEX_NOT_FOUND;
}

/**************************************************************************/ /**
 *  @fn          static int32_t sensorhub_delete_sensor_list_index(sl_sensor_id_en sensor_id)
 *  @brief       To delete the sensor index from the list
 *  @param[in]   sensor_id     id of the target sensor
 *  @return      index of the sensor
*******************************************************************************/
static uint32_t sensorhub_delete_sensor_list_index(sl_sensor_id_t sensor_id)
{
  uint32_t i;
  for (i = 0; i < SL_MAX_NUM_SENSORS; i++) {
    if (sensor_list.sl_sensors_st[i].config_st->sensor_id == sensor_id) {
      sensor_list.sl_sensors_st[i].config_st->sensor_id = 0;
      return i;
    }
  }
  return SL_SH_SENSOR_INDEX_NOT_FOUND;
}
/**************************************************************************/ /**
 *  @fn          static sl_sensor_info_t* sensorhub_get_sensor_info(sl_sensor_id_en sensor_id)
 *  @brief       To get the info on the configured sensor
 *  @param[in]   sensor_id     id of the target sensor
 *  @return      reference to the sensor info
*******************************************************************************/
static sl_sensor_info_t *sensorhub_get_sensor_info(sl_sensor_id_t sensor_id)
{
  uint8_t sens_index = 0;
  for (sens_index = 0; sens_index < SL_MAX_NUM_SENSORS; sens_index++) {
    if (sensor_hub_info_t[sens_index].sensor_id == sensor_id) {
      return &sensor_hub_info_t[sens_index];
    }
  }
  return NULL;
}

/**************************************************************************/ /**
 *  @fn          void sl_si91x_sensorhub_notify_cb_register(
 *                    sl_sensor_signalEvent_t cb_event, sl_sensor_id_en *cb_ack)
 *  @brief       To register the application callback handler
 *  @param[in]   cb_event     pointer to the event
 *  @param[in]   cb_ack       Pointer to the application handler
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_notify_cb_register(sl_sensor_signalEvent_t cb_event, sl_sensor_id_t *cb_ack)
{
  if (cb_event == NULL || cb_ack == NULL) {
    return SL_SH_INVALID_PARAMETERS;
  }
  //!register call back handler for application
  cb_info.cb_event     = cb_event;
  cb_info.cb_event_ack = cb_ack;

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          static void sensorhub_sensors_timer_cb(TimerHandle_t xTimer)
 *  @brief       software timer callback
 *  @param[in]   xTimer     handle to the timer
*******************************************************************************/
static void sensorhub_sensors_timer_cb(TimerHandle_t xTimer)
{
  uint32_t event_bit = (uint32_t)pvTimerGetTimerID(xTimer);
  osEventFlagsSet(sl_event_group, (0x01 << event_bit));
}

/**************************************************************************/ /**
 *  @fn          void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
 *  @brief       NPSS GPIO IRQ handler
*******************************************************************************/
void gpio_uulp_pin_interrupt_callback(uint32_t pin_intr)
{
  volatile uint32_t intrStatus = 0;
  intrStatus                   = pin_intr;
  for (uint8_t idx = 0; idx < int_list_map.map_index; idx++) {
    if (int_list_map.map_table[idx].intr == intrStatus) {
      RSI_NPSSGPIO_ClrIntr(BIT(int_list_map.map_table[idx].intr));
      RSI_NPSSGPIO_IntrMask(BIT(int_list_map.map_table[idx].intr));
      osEventFlagsSet(sl_event_group, (0x01 << int_list_map.map_table[idx].sensor_list_index));
    }
  }
}
void sl_si91x_button_isr(uint8_t pin, int8_t state)
{
  (void)state;
  gpio_uulp_pin_interrupt_callback(pin);
}
/**************************************************************************/ /**
 *  @fn          static void sensorhub_gpio_interrupt_config(uint16_t gpio_pin,sl_gpio_intr_type intr_type)
 *  @brief       Configuring the different types of NPSS GPIO interrupts.
 *  @param[in]   gpio_pin     GPIO pin number
 *  @param[in]   intr_type    type of interrupt.
*******************************************************************************/
static sl_status_t sensorhub_gpio_interrupt_config(uint16_t gpio_pin, sl_si91x_gpio_interrupt_config_flag_t intr_type)
{

  sl_status_t status;
  uulp_pad_config_t uulp_pad;
  do {
    uulp_pad.gpio_padnum = gpio_pin; // UULP GPIO pin number 2 is selected
    uulp_pad.pad_select  = SET;      // UULP GPIO PAD is selected
    uulp_pad.mode        = CLR;      // UULP GPIO mode 0 is selected
    uulp_pad.direction   = SET;      // UULP GPIO direction is selected
    uulp_pad.receiver    = SET;      // UULP GPIO receiver is enabled

    // Initialize the GPIOs by clearing all interrupts initially
    status = sl_gpio_driver_init();
    if (status != SL_STATUS_OK) {
      // Prints GPIO initialization fails
      DEBUGOUT("sl_gpio_driver_init, Error code: %lu", status);
      break; // breaks if error occurs
    }

    status = sl_gpio_set_configuration(sl_gpio_pin_config1);
    if (status != SL_STATUS_OK) {
      // Prints if pin configuration fails
      DEBUGOUT("sl_gpio_set_configuration, Error code: %lu", status);
      break; // breaks if error occurs
    }

    // Configure the UULP GPIO pin mode, receiver enable, direction and polarity.
    status = sl_si91x_gpio_driver_set_uulp_pad_configuration(&uulp_pad);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_si91x_gpio_driver_set_uulp_pad_configuration, Error code: %lu", status);
      break;
    }
    status = sl_gpio_driver_configure_interrupt(&sl_gpio_pin_config1.port_pin,
                                                gpio_pin,
                                                (sl_gpio_interrupt_flag_t)intr_type,
                                                (sl_gpio_irq_callback_t)&gpio_uulp_pin_interrupt_callback,
                                                AVL_INTR_NO);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("sl_gpio_configure_interrupt, Error code: %lu", status);
      break;
    }
  } while (false);
  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          static void sensorhub_gpio_interrupt_start(uint16_t gpio_pin)
 *  @brief       Enable and set the priority of GPIO interrupt.
 *  @param[in]   gpio_pin     GPIO pin number
*******************************************************************************/
static void sensorhub_gpio_interrupt_start(uint16_t gpio_pin)
{
  if (gpio_pin != 2) {

    /* unmask the NPSS GPIO interrupt */
    RSI_NPSSGPIO_IntrUnMask(BIT(gpio_pin));

    /* Set NVIC priority less than syscall priority */
    NVIC_SetPriority(NPSS_TO_MCU_GPIO_INTR_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);

    /*  NVIC Enable */
    NVIC_EnableIRQ(NPSS_TO_MCU_GPIO_INTR_IRQn);
  }
}

/**************************************************************************/ /**
 *  @fn          static void sensorhub_gpio_interrupt_stop(uint16_t gpio_pin)
 *  @brief       Mask and Disable the GPIO interrupt.
 *  @param[in]   gpio_pin     GPIO pin number
*******************************************************************************/
static void sensorhub_gpio_interrupt_stop(uint16_t gpio_pin)
{
  /*mask the NPSS GPIO interrupt */
  RSI_NPSSGPIO_IntrMask(BIT(gpio_pin));

  /*  NVIC Disable */
  NVIC_DisableIRQ(NPSS_TO_MCU_GPIO_INTR_IRQn);
}

/*******************************************************************************
 * ADC user callback
 * This function will be called from ADC interrupt handler
 *
 * @param[in] ADC channel number
 * @param[in] ADC callback event (ADC_STATIC_MODE_CALLBACK, 
 *            ADC_THRSHOLD_CALLBACK, INTERNAL_DMA, FIFO_MODE_EVENT)
 *
*******************************************************************************/
void sl_si91x_adc_callback_v2(uint8_t channel_no, uint8_t event)
{
  if (event == SL_INTERNAL_DMA) {
    bus_intf_info.adc_config.adc_data_ready |= BIT(channel_no);
    for (uint8_t idx = 0; (idx < int_list_map.map_index) && (int_list_map.map_table[idx].adc_intr_channel != 0);
         idx++) {
      // Checking if we got single channel interrupt, which is required for the respective sensor
      if ((IS_SINGLE_CHANNEL(int_list_map.map_table[idx].adc_intr_channel)
           && (int_list_map.map_table[idx].adc_intr_channel & bus_intf_info.adc_config.adc_data_ready))
          // Checking if we got multiple channels interrupt, which are required for the respective sensor
          || (int_list_map.map_table[idx].adc_intr_channel == bus_intf_info.adc_config.adc_data_ready)) {
        bus_intf_info.adc_config.adc_data_ready &= ~(int_list_map.map_table[idx].adc_intr_channel);
        osEventFlagsSet(sl_event_group, (0x01 << int_list_map.map_table[idx].sensor_list_index));
      }
    }
  }
}

/**************************************************************************/ /**
 *  @fn          void sl_si91x_sdc_intr_event_set(uint8_t channel_no, uint8_t event)
 *  @brief       Enable and set the priority of GPIO interrupt.
 *  @param[in]   channel_no     SDC Channel Number
 *  @param[in]   event          Event
 *  Return       None
*******************************************************************************/
void sl_si91x_sdc_intr_event_set(uint8_t channel_no, uint8_t event)
{
  (void)event;
  for (uint8_t idx = 0; idx < int_list_map.map_index; idx++) {
    int_list_map.map_table[idx].sdc_intr_channel |= sdc_intr_done;
    if (int_list_map.map_table[idx].sdc_intr_channel
        & BIT(channel_no)) { // in non-interrupt mode channel variable in map table will be always zero
      osEventFlagsSet(sl_event_group, (0x01 << int_list_map.map_table[idx].sensor_list_index));
    }
  }
}

/**************************************************************************/ /**
 *  @fn          static sl_status_t sensorhub_adc_init(void)
 *  @brief       Initialize the ADC Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
static sl_status_t sensorhub_adc_init(void)
{
  uint32_t status = 0;

  /* De-initialize the adc peripheral if already initialized */
  if (bus_intf_info.adc_config.adc_init == 1) {
    status = sl_si91x_adc_deinit(bus_intf_info.adc_config.adc_cfg);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n ADC DeInit Failed, Error Code : %ld\r\n", status);
      return SL_STATUS_FAIL;
    }
  }
  /* initialize adc peripheral */
  status =
    sl_si91x_adc_init(bus_intf_info.adc_config.adc_ch_cfg, bus_intf_info.adc_config.adc_cfg, SL_SH_ADC_VREF_VALUE);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n ADC Initialization Failed, Error Code : %ld\r\n", status);
    return SL_STATUS_FAIL;
  } else {
    bus_intf_info.adc_config.adc_init = 1;
  }

  /* set callback function for ADC interrupt only in FIFO mode*/
  if (bus_intf_info.adc_config.adc_cfg.operation_mode != SL_ADC_STATIC_MODE) {
    status = sl_si91x_adc_register_event_callback(sl_si91x_adc_callback_v2);
    if (status != SL_STATUS_OK) {
      DEBUGOUT("\r\n ADC callback event fail:%lu\r\n", status);
      return SL_STATUS_FAIL;
    }
  }
  /* Initialize rx buffer for each channels */
  for (uint8_t ch_no = 0; ch_no < SL_SH_ADC_NUM_CHANNELS_ENABLE; ch_no++) {
    bus_intf_info.adc_config.adc_ch_cfg.rx_buf[ch_no] = (int16_t *)adc_data_ptrs[ch_no];
  }

  /* start the adc peripheral */
  status = sl_si91x_adc_start(bus_intf_info.adc_config.adc_cfg);
  if (status != SL_STATUS_OK) {
    DEBUGOUT("\r\n ADC sensor start failed:%lu\r\n", status);
    return SL_STATUS_FAIL;
  }

  /* If ADC works on static mode, sensor should work only on SL_SH_POLLING_MODE. So, disabling the interrupt here*/
  if (bus_intf_info.adc_config.adc_cfg.operation_mode == SL_ADC_STATIC_MODE) {
    NVIC_DisableIRQ(ADC_IRQn);
  }

  //  DEBUGOUT("\r\n ADC Initialization Success\r\n");

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn         static void sensorhub_sdc_config_params(sl_sdc_config_t * sdc_config)
 *  @brief       Initialize the sdc Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
#ifdef SH_SDC_ENABLE
static void sensorhub_sdc_config_params(sl_drv_sdc_config_t *sdc_config_st_p)
{
  sdc_config_st_p->sdc_clk_div = SDC_CLK_DIV_VALUE;

  sdc_config_st_p->sdc_p_channel_sel[0]       = bus_intf_info.sh_sdc_config.sh_sdc_p_channel_sel[0];
  sdc_config_st_p->sdc_n_channel_sel[0]       = bus_intf_info.sh_sdc_config.sh_sdc_n_channel_sel[0];
  sdc_config_st_p->sdc_auxadc_diff_mode_ch[0] = bus_intf_info.sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[0];

#ifdef SDC_MUTI_CHANNEL_ENABLE
#ifdef SDC_CHANNEL_2
  sdc_config_st_p->sdc_p_channel_sel[1]       = bus_intf_info.sh_sdc_config.sh_sdc_p_channel_sel[1];
  sdc_config_st_p->sdc_n_channel_sel[1]       = bus_intf_info.sh_sdc_config.sh_sdc_n_channel_sel[1];
  sdc_config_st_p->sdc_auxadc_diff_mode_ch[1] = bus_intf_info.sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[1];
#endif

#ifdef SDC_CHANNEL_3
  sdc_config_st_p->sdc_p_channel_sel[2]       = bus_intf_info.sh_sdc_config.sh_sdc_p_channel_sel[2];
  sdc_config_st_p->sdc_n_channel_sel[2]       = bus_intf_info.sh_sdc_config.sh_sdc_n_channel_sel[2];
  sdc_config_st_p->sdc_auxadc_diff_mode_ch[2] = bus_intf_info.sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[2];
#endif

#ifdef SDC_CHANNEL_4
  sdc_config_st_p->sdc_p_channel_sel[4]       = bus_intf_info.sh_sdc_config.sh_sdc_p_channel_sel[4];
  sdc_config_st_p->sdc_n_channel_sel[4]       = bus_intf_info.sh_sdc_config.sh_sdc_n_channel_sel[4];
  sdc_config_st_p->sdc_auxadc_diff_mode_ch[4] = bus_intf_info.sh_sdc_config.sh_sdc_auxadc_diff_mode_ch[4];
#endif
#endif
  sdc_config_st_p->sdc_cnt_trig_evnt      = bus_intf_info.sh_sdc_config.sh_sdc_cnt_trig_evnt;
  sdc_config_st_p->sdc_data_trigger_sel   = bus_intf_info.sh_sdc_config.sh_sdc_data_trigger_sel;
  sdc_config_st_p->sdc_no_channel_sel     = bus_intf_info.sh_sdc_config.sh_sdc_no_channel_sel;
  sdc_config_st_p->sdc_sample_ther        = bus_intf_info.sh_sdc_config.sh_sdc_sample_ther;
  sdc_config_st_p->sdc_sample_trigger_sel = bus_intf_info.sh_sdc_config.sh_sdc_sample_trigger_sel;
}
#endif
/**************************************************************************/ /**
 *  @fn          static uint8_t sensorhub_sdc_init(void)
 *  @brief       Initialize the sdc Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
#ifdef SH_SDC_ENABLE
static sl_status_t sensorhub_sdc_init(void)
{
  sl_drv_sdc_config_t sli_sdc_config_st;

  sl_si91x_sh_rtc_start(); //start the RTC

  sensorhub_sdc_config_params(&sli_sdc_config_st);

  sdc_pin_mux(sli_sdc_config_st.sdc_p_channel_sel[0], sli_sdc_config_st.sdc_n_channel_sel[0], 0);

#ifdef SDC_MUTI_CHANNEL_ENABLE
#ifdef SDC_CHANNEL_2
  sdc_pin_mux(sli_sdc_config_st.sdc_p_channel_sel[1], sli_sdc_config_st.sdc_n_channel_sel[1], 0);
#endif
#ifdef SDC_CHANNEL_3
  sdc_pin_mux(sli_sdc_config_st.sdc_p_channel_sel[2], sli_sdc_config_st.sdc_n_channel_sel[2], 0);
#endif
#ifdef SDC_CHANNEL_4
  sdc_pin_mux(sli_sdc_config_st.sdc_p_channel_sel[3], sli_sdc_config_st.sdc_n_channel_sel[3], 0);
#endif
#endif
  sl_si91x_sdc_configure_clock(); //init the clocks

  sl_si91x_configure_sdcss(sli_sdc_config_st); //configure the sdc

  return SL_STATUS_OK;
}
#endif
/**************************************************************************/ /**
 *  @fn          uint8_t sl_si91x_fetch_adc_bus_intf_info(void)
 *  @brief       Fetch ADC bus interface information. This can be used by lower
 *               level layers
 *  @param[in]   None
 *  @return      Pointer to ADC configuration structure
*******************************************************************************/
sl_adc_cfg_t *sl_si91x_fetch_adc_bus_intf_info(void)
{
  return &bus_intf_info.adc_config;
}

/**************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_sensorhub_init()
 *  @brief       This function will initialize the Peripherals(I2C/SPI/ADC)
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_init()
{
  bus_errors.i2c = true;
  bus_errors.spi = true;
  bus_errors.adc = true;
  int32_t status = 0;
  /*TODO: SPI, ADC, and UART initializations should be handled */
  status = sensorhub_i2c_init();
  if (status != SL_STATUS_OK) {
    bus_errors.i2c = false;
    DEBUGOUT("\r\n I2C Init Fail \r\n");
  }
#if !(SH_ADC_ENABLE || SH_SDC_ENABLE)
  status = sensorhub_spi_init();
  if (status != SL_STATUS_OK) {
    bus_errors.spi = false;
    DEBUGOUT("\r\n SPI Init Fail \r\n");
  }
#endif
#ifdef SH_ADC_ENABLE
  status = sensorhub_adc_init();
  if (status != SL_STATUS_OK) {
    bus_errors.adc = false;
    DEBUGOUT("\r\n ADC Init Fail \r\n");
  }
#endif
#ifdef SH_SDC_ENABLE
  status = sensorhub_sdc_init();
  if (status != SL_STATUS_OK) {
    bus_errors.sdc = false;
    DEBUGOUT("\r\n sdc Init Fail \r\n");
  }
  DEBUGOUT("\r\n sdc Init done \r\n");
#endif
  if (!bus_errors.i2c && !bus_errors.spi && !bus_errors.adc && !bus_errors.sdc) {
    return SL_ALL_PERIPHERALS_INIT_FAILED;
  }
  bus_errors.peripheral_global_status = 0;
  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          int32_t sensor_hub_start()
 *  @brief       Start the sensor hub operations
 *  @return      execution status
*******************************************************************************/
sl_status_t sl_si91x_sensor_hub_start()
{
  osThreadId_t status = 0;
  sl_status_t pm_subs_status;

  // Subscribe the state transition callback events, the ored value of flag and function pointer is passed in this API.
  pm_subs_status = sl_si91x_power_manager_subscribe_ps_transition_event(&handle, &info);
  if (pm_subs_status != SL_STATUS_OK) {
    // If status is not OK, return with the error code.
    DEBUGOUT("Power Manager transition event subscription failed, Error Code: 0x%lX \n", pm_subs_status);
    return SL_STATUS_FAIL;
  }
  DEBUGOUT("Power Manager transition event is subscribed \n");

  if (sl_si91x_power_manager_get_current_state() == SL_SI91X_POWER_MANAGER_PS3) {
    sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
    sensorhub_current_powerstate = SL_SI91X_POWER_MANAGER_PS4;
  } else {
    sensorhub_current_powerstate = sl_si91x_power_manager_get_current_state();
  }

  status = osThreadNew((osThreadFunc_t)sensorhub_sensor_task, NULL, &sensor_thread_attributes);
  if (status == NULL) {
    DEBUGOUT("\r\n Sensor_Task create fail \r\n");
    return SL_SH_SENSOR_TASK_CREATION_FAILED;
  }
  DEBUGOUT("\r\n Sensor_Task:%p \r\n", status);

  status = osThreadNew((osThreadFunc_t)sensorhub_em_task, NULL, &EM_thread_attributes);
  if (status == NULL) {
    DEBUGOUT("\r\n EM_Task create fail \r\n");
    return SL_SH_EM_TASK_CREATION_FAILED;
  }
  DEBUGOUT("\r\n EM_Task:%p \r\n", status);

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          uint8_t sl_si91x_sensorhub_detect_sensors(sl_sensor_id_en
 *                                  sensor_id_info[],uint8_t num_of_sensors)
 *  @brief       Scan the available sensors in the sensor HUB.
 *  @param[in]   sensor_id_info    Reference to the detected sensor info
 *  @param[in]   num_of_sensors    Number of sensors to be scanned
 *  @return      if successful,    Return number of sensors is scanned
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail,
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_detect_sensors(sl_sensor_id_t *sensor_id_info, uint8_t num_of_sensors)
{
  sl_status_t cnt, sensors_detected = 0;
  int32_t status = SL_STATUS_FAIL;
  for (cnt = 0; cnt < num_of_sensors; cnt++) {
    switch (sensor_hub_info_t[cnt].sensor_bus) {
      case SL_SH_I2C:
        if (bus_errors.i2c) {
          status = sensorhub_i2c_sensors_scan(sensor_hub_info_t[cnt].address);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\n Failed to Scan sensor: %s I2C error code: %lu \r\n",
                     sensor_hub_info_t[cnt].sensor_name,
                     status);
          }
        }
        break;

      case SL_SH_SPI:
        status = SL_STATUS_OK;
        break;

      case SL_SH_ADC:
        status = SL_STATUS_OK;
        break;

      case SL_SH_GPIO:
        status = SL_STATUS_OK;
        break;

      default:
        DEBUGOUT("\r\n Failed to Scan sensor:%d cnt:%lu \r\n", sensor_hub_info_t[cnt].sensor_id, cnt);
        status = SL_STATUS_FAIL;
        break;
    }

    if (status == SL_STATUS_OK) {
      sensor_id_info[sensors_detected++] = sensor_hub_info_t[cnt].sensor_id;
    }
  }
  return sensors_detected;
}

/**************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_sensorhub_create_sensor(sl_sensor_id_en sensor_id)
 *  @brief       To Create a sensor instance in the sensor hub.
 *  @param[in]   sensor_id     ID of the sensor
 * @return       status 0 if successful, else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail, Create sensor instance failed
 *               \ref SL_STATUS _OK (0X000) - Create sensor instanceSuccess 
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_create_sensor(sl_sensor_id_t sensor_id)
{
  sl_sensor_info_t *local_info;
  uint8_t sensor_index;
  sl_status_t status         = 0;
  uint32_t ramAllocationSize = 0;
  char sl_sensor_timer_name[16];

  local_info = sensorhub_get_sensor_info(sensor_id);
  if (NULL == local_info) {
    /*If there is no config found for the given sensor then return an error*/
    return SL_SH_CONFIG_NOT_FOUND;
  }

  /*Add the sensor to the sensor list*/
  sensor_index = (uint8_t)sensorhub_create_sensor_list_index();

  if (sensor_index == SL_MAX_NUM_SENSORS) {
    /* Invalid sensor index */
    return SL_SH_MAX_SENSORS_REACHED;
  }

  sensor_list.sl_sensors_st[sensor_index].sensor_event_bit     = sensor_index;
  sensor_list.sl_sensors_st[sensor_index].config_st            = local_info;
  sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id = sensor_id;
  sl_sensor_wait_flags |= (uint8_t)(0x1 << sensor_list.sl_sensors_st[sensor_index].sensor_event_bit);

  /*Allocate a data RAM for the sensor to store the sampling data*/
  // if (sensor_list.sensors[sensor_index].config_st->sens_data_ptr == NULL) {

  /* We are allocating memory to store data for each ADC sensor and respecitve channel in adc_sensor_hal.c */
  /* We are doing this to match with ADC FIFO mode ping/pong memory operation */
  /* Therefore, we are allocating memory here to access that data using a pointer. */
  if (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_bus == SL_SH_ADC) {
    ramAllocationSize = 4 + sizeof(sl_sensor_data_t);
  } else {
    switch (sensor_list.sl_sensors_st[sensor_index].config_st->data_deliver.data_mode) {
      case SL_SH_THRESHOLD:
        ramAllocationSize = 4 + sizeof(sl_sensor_data_t);
        break;

      case SL_SH_TIMEOUT:
        ramAllocationSize = 4
                            + (sizeof(sl_sensor_data_t)
                               * ((sensor_list.sl_sensors_st[sensor_index].config_st->data_deliver.timeout
                                   / sensor_list.sl_sensors_st[sensor_index].config_st->sampling_interval)
                                  + 1));
        break;

      case SL_SH_NUM_OF_SAMPLES:
        ramAllocationSize =
          4
          + (sizeof(sl_sensor_data_t) * (sensor_list.sl_sensors_st[sensor_index].config_st->data_deliver.numofsamples));
        break;

      case SL_SH_NO_DATA_MODE:
        ramAllocationSize = 4 + sizeof(sl_sensor_data_t);
        break;

      default:
        return SL_SH_INVALID_DELIVERY_MODE;
        break;
    }
  }

  ramAllocationSize = ramAllocationSize + ((4 - (ramAllocationSize % 4))); //for 4 byte align and 4 byte boundary
  /*Check if Sensor RAM is available*/
  if ((free_ram_index + ramAllocationSize) > sizeof(sensor_data_ram)) {
    return SL_SH_MEMORY_LIMIT_EXCEEDED;
  }

  sensor_list.sl_sensors_st[sensor_index].config_st->sensor_data_ptr =
    (sl_sensor_data_group_t *)&sensor_data_ram[free_ram_index];
  sensor_list.sl_sensors_st[sensor_index].config_st->sensor_data_ptr->sensor_data =
    (void *)&sensor_data_ram[free_ram_index] + 4;
  free_ram_index += ramAllocationSize;
  sensor_list.sl_sensors_st[sensor_index].max_samples = (ramAllocationSize - 2) / sizeof(sl_sensor_data_t);
  //}

  /*Find the Sensor HAL Implementation*/
  sensor_list.sl_sensors_st[sensor_index].sensor_impl =
    sensorhub_get_sensor_implementation((sensor_id & SL_SENSOR_ID_MASK) >> SL_SENSOR_ID_OFFSET);
  if (sensor_list.sl_sensors_st[sensor_index].sensor_impl == NULL) {
    return SL_SH_SENSOR_IMPLEMENTATION_NOT_FOUND;
  }
  /*Create a sensor*/
  sensor_list.sl_sensors_st[sensor_index].sensor_handle = sensor_list.sl_sensors_st[sensor_index].sensor_impl->create(
    sensor_list.sl_sensors_st[sensor_index].config_st->sensor_bus,
    (sensor_list.sl_sensors_st[sensor_index].config_st->address));
  if (sensor_list.sl_sensors_st[sensor_index].sensor_handle == NULL) {
    return SL_SH_HAL_SENSOR_CREATION_FAILED;
  }

  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->control(
      &sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id,
      SL_COMMAND_SET_POWER,
      NULL);
  if (sensor_list.sl_sensors_st[sensor_index].ctrl_handle != NULL) {
    return SL_SH_COMMAND_SET_POWER_FAIL;
  }

  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->control(
      &sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id,
      SL_COMMAND_SET_RANGE,
      NULL);
  if (sensor_list.sl_sensors_st[sensor_index].ctrl_handle != NULL) {
    return SL_SH_COMMAND_SET_RANGE_FAIL;
  }

  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->control(
      &sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id,
      SL_COMMAND_SELF_TEST,
      NULL);
  if (sensor_list.sl_sensors_st[sensor_index].ctrl_handle != NULL) {
    return SL_SH_COMMAND_SELF_TEST_FAIL;
  }

  snprintf(sl_sensor_timer_name,
           sizeof(sl_sensor_timer_name) - 1,
           "%s%02x",
           sensor_list.sl_sensors_st[sensor_index].config_st->sensor_name,
           sensor_id);

  /*Check the sensor mode. poll/interrupt and create the timer*/
  switch (local_info->sensor_mode) {
    case SL_SH_POLLING_MODE:

      sensor_list.sl_sensors_st[sensor_index].timer_handle =
        xTimerCreate(sl_sensor_timer_name,
                     ((sensor_list.sl_sensors_st[sensor_index].config_st->sampling_interval)),
                     1,
                     (void *)(sensor_list.sl_sensors_st[sensor_index].sensor_event_bit),
                     sensorhub_sensors_timer_cb);

      if (sensor_list.sl_sensors_st[sensor_index].timer_handle == NULL) {
        DEBUGOUT("\r\n OS timer creation Failed \r\n");
        return SL_SH_TIMER_CREATION_FAILED;
      }
      break;

    case SL_SH_INTERRUPT_MODE:
      if (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_bus == SL_SH_ADC) {
        int_list_map.map_table[int_list_map.map_index].adc_intr_channel =
          sensor_list.sl_sensors_st[sensor_index].config_st->channel;
        int_list_map.map_table[int_list_map.map_index].sensor_list_index = sensor_index;
        int_list_map.map_index++;
      } else {
        if (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id != SL_GPIO_SENSE_BUTTON_ID) {
          status =
            sensorhub_gpio_interrupt_config(sensor_list.sl_sensors_st[sensor_index].config_st->sampling_intr_req_pin,
                                            sensor_list.sl_sensors_st[sensor_index].config_st->sensor_intr_type);
          if (status != SL_STATUS_OK) {
            return status;
          }
        }
        int_list_map.map_table[int_list_map.map_index].intr =
          sensor_list.sl_sensors_st[sensor_index].config_st->sampling_intr_req_pin;
        int_list_map.map_table[int_list_map.map_index].sensor_list_index = sensor_index;
        int_list_map.map_index++;
      }
      break;

    default:
      return SL_SH_INVALID_MODE;
      break;
  }

  /* update sensor status into sensor list */
  sensor_list.sl_sensors_st[sensor_index].sensor_status = SL_SENSOR_VALID;

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          int32_t sl_si91x_sensorhub_delete_sensor
 *                                      (sl_sensor_id_en sensor_id)
 *  @brief       To delete a sensor from the sensor hub.
 *  @param[in]   sensor_id     ID of the sensor
 * @return status 0 if successful, else error code
 *       \ref SL_STATUS_FAIL (0x0001) - Fail, Delete sensor failed \n
 *       \ref SL_STATUS_OK (0X000)   - Success, Delete sensor Success \n
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_delete_sensor(sl_sensor_id_t sensor_id)
{
  /*TODO: Need to verify the delete function*/
  uint32_t sensor_index;
  osStatus_t timer_status;
  /*Delete the sensor to the sensor list*/
  sensor_index = sensorhub_delete_sensor_list_index(sensor_id);
  if (sensor_index == SL_MAX_NUM_SENSORS) {
    /* Invalid sensor index */
    return SL_SH_SENSOR_INDEX_NOT_FOUND;
  }

  /*Check the sensor mode. poll/interrupt and create the timer*/
  switch (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_mode) {
    case SL_SH_POLLING_MODE:
      timer_status = osTimerDelete(sensor_list.sl_sensors_st[sensor_index].timer_handle); //cmsis v2 apis
      if (timer_status != osOK) {
        /* Post-event as SL_SENSOR_START_FAILED */
        DEBUGOUT("\r\n osTimer Delete failed:%d \r\n", timer_status);
        sensorhub_em_post_event(sensor_id, SL_SENSOR_DELETE_FAILED, NULL, EM_POST_TIME);
        return SL_SH_TIMER_DELETION_FAILED;
      }
      break;

    case SL_SH_INTERRUPT_MODE:
      sensorhub_gpio_interrupt_stop(sensor_list.sl_sensors_st[sensor_index].config_st->sampling_intr_req_pin);
      break;

    default:
      break;
  }

  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->control(
      &sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id,
      SL_COMMAND_SET_POWER,
      (void *)SL_SENSOR_POWER_MODE_SLEEP);
  if (sensor_list.sl_sensors_st[sensor_index].ctrl_handle != NULL) {
    return SL_SH_COMMAND_SET_POWER_FAIL;
  }

  //!delete sensor
  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->delete (
      (void *)&sensor_list.sl_sensors_st[sensor_index].sensor_handle);
  if (sensor_list.sl_sensors_st[sensor_index].ctrl_handle != NULL) {
    sensorhub_em_post_event(sensor_id, SL_SENSOR_DELETE_FAILED, NULL, EM_POST_TIME);

    return SL_SH_HAL_SENSOR_DELETION_FAILED;
  }

  /* update sensor status into sensor list */
  sensor_list.sl_sensors_st[sensor_index].sensor_status = SL_SENSOR_INVALID;

  //!post SENSOR_STARTED event to application
  sensorhub_em_post_event(sensor_id, SL_SENSOR_DELETED, NULL, EM_POST_TIME);

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_sensorhub_start_sensor(sl_sensor_id_en sensor_id)
 *  @brief       To start the sensor operation for the given sensor
 *  @param[in]   sensor_id     ID of the sensor
 *  @return status 0 if successful, else error code
 *       \ref SL_STATUS_FAIL (0x0001)- Fail, Sensor start failed
 *       \ref SL_STATUS _OK (0X000) - Success, Sensor start done properly
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_start_sensor(sl_sensor_id_t sensor_id)
{
  uint32_t sensor_index, status = 0;
  osStatus_t timer_status;
  sensor_index = sensorhub_get_sensor_index(sensor_id);
  if (sensor_index == SL_SH_SENSOR_INDEX_NOT_FOUND) {
    sensorhub_em_post_event(sensor_id, SL_SENSOR_CREATION_FAILED, NULL, EM_POST_TIME);
    return SL_SH_SENSOR_CREATE_FAIL;
  }

  switch (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_mode) {
    case SL_SH_POLLING_MODE:

      timer_status = xTimerStart(sensor_list.sl_sensors_st[sensor_index].timer_handle, portMAX_DELAY);
      if (timer_status != pdPASS) {
        /* Post event as SL_SENSOR_START_FAILED */
        sensorhub_em_post_event(sensor_id, SL_SENSOR_START_FAILED, NULL, EM_POST_TIME);
        return SL_SH_TIMER_START_FAIL;
      }

      break;

    case SL_SH_INTERRUPT_MODE:
      if (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_bus == SL_SH_ADC) {
        NVIC_SetPriority(ADC_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY - 1);
      } else {
        sensorhub_gpio_interrupt_start(sensor_list.sl_sensors_st[sensor_index].config_st->sampling_intr_req_pin);
      }
      break;
    default:
      //!Post-event to the application as sensor config_st invalid
      sensorhub_em_post_event(sensor_id, SL_SENSOR_CNFG_INVALID, NULL, EM_POST_TIME);
      return SL_SH_INVALID_MODE;
  }

  /* update sensor status into sensor list */
  sensor_list.sl_sensors_st[sensor_index].sensor_status = SL_SENSOR_START;

  //!post SENSOR_STARTED event to application
  sensorhub_em_post_event(sensor_id, SL_SENSOR_STARTED, NULL, EM_POST_TIME);

  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->control(
      &sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id,
      SL_COMMAND_SET_POWER,
      NULL);
  if (status != SL_STATUS_OK) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn          sl_status_t sl_si91x_sensorhub_stop_sensor(sl_sensor_id_en sensor_id)
 *  @brief       To stop the sensor operation for the given sensor
 *  @param[in]   sensor_id     ID of the sensor
 *  @return status 0 if successful, else error code
 *       \ref SL_STATUS_FAIL (0x0001)- Fail, Sensor stop failed
 *       \ref SL_STATUS _OK (0X000) - Success, Sensor stop done properly
*******************************************************************************/
sl_status_t sl_si91x_sensorhub_stop_sensor(sl_sensor_id_t sensor_id)
{
  /*TODO: Need to verify the delete function*/
  uint32_t sensor_index, status = 0;
  sensor_index = sensorhub_get_sensor_index(sensor_id);
  if (sensor_index == SL_SH_SENSOR_INDEX_NOT_FOUND) {
    sensorhub_em_post_event(sensor_id, SL_SENSOR_STOP_FAILED, NULL, EM_POST_TIME);
    return SL_SH_SENSOR_CREATE_FAIL;
  }
  switch (sensor_list.sl_sensors_st[sensor_index].config_st->sensor_mode) {
    case SL_SH_POLLING_MODE:
      status = osTimerStop(sensor_list.sl_sensors_st[sensor_index].timer_handle);
      if (status != osOK) {
        /* Post event as SL_SENSOR_STOP_FAILED */
        DEBUGOUT("\r\n osTimer stop failed:%lu \r\n", status);
        sensorhub_em_post_event(sensor_id, SL_SENSOR_STOP_FAILED, NULL, EM_POST_TIME);
        return SL_SH_TIMER_STOP_FAIL;
      }
      break;

    case SL_SH_INTERRUPT_MODE:
      sensorhub_gpio_interrupt_stop(sensor_list.sl_sensors_st[sensor_index].config_st->sampling_intr_req_pin);

      break;
    default:
      //!Post-event to the application as sensor config invalid
      sensorhub_em_post_event(sensor_id, SL_SENSOR_CNFG_INVALID, NULL, EM_POST_TIME);
      return SL_STATUS_FAIL;
  }

  sl_sensor_wait_flags &= (uint8_t)(~(0x1 << sensor_list.sl_sensors_st[sensor_index].sensor_event_bit));

  /* update sensor status into sensor list */
  sensor_list.sl_sensors_st[sensor_index].sensor_status = SL_SENSOR_STOP;

  //!post SENSOR_STOPPED event to application
  sensorhub_em_post_event(sensor_id, SL_SENSOR_STOPPED, NULL, EM_POST_TIME);

  sensor_list.sl_sensors_st[sensor_index].ctrl_handle =
    (void *)sensor_list.sl_sensors_st[sensor_index].sensor_impl->control(
      &sensor_list.sl_sensors_st[sensor_index].config_st->sensor_id,
      SL_COMMAND_SET_POWER,
      NULL);

  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 *  @fn        static void sensorhub_em_post_event(sl_sensor_id_en sensor_id,
 *  sl_sensorhub_event_en event, void* dataPtr, TickType_t ticks_to_wait)
 *  @brief       To post the events/notifications to event manager(EM) to be notified to the application
 *  @param[in]   sensor_id      id of the  sensor
 *  @param[in]   event          event/notifications
 *  @param[in]   dataPtr        pointer to the data
 *  @param[in]   ticks_to_wait  max time to wait for the post
*******************************************************************************/
static void sensorhub_em_post_event(sl_sensor_id_t sensor_id,
                                    sl_sensorhub_event_t event,
                                    void *dataPtr,
                                    TickType_t ticks_to_wait)
{
  sl_em_event_t em_event;
  em_event.sensor_id      = sensor_id;
  em_event.event          = event;
  em_event.em_sensor_data = dataPtr;

  BaseType_t mutex_result = pdFALSE;
  mutex_result            = osMutexAcquire(sl_event_queue_mutex, ticks_to_wait);
  if (mutex_result != osOK) {
    DEBUGOUT("\r\n osMutexAcquire failed in Sensor_Task:%u \r\n", (uint8_t)mutex_result);
  }
  mutex_result = osMessageQueuePut(sl_event_queue_handler, &em_event, 0U, 0U);
  if (mutex_result == osOK) {
    //xSemaphoreGiveRecursive(event_queue_mutex);
    mutex_result = osMutexRelease(sl_event_queue_mutex);

    if (mutex_result != osOK) {
      DEBUGOUT("\r\n osMutexRelease failed in em task:%u \r\n", (uint8_t)mutex_result);
    }
  }
}

/**************************************************************************/ /**
 *  @fn       static void sensorhub_em_task(void)
 *  @brief       Task to handle the operations of the Event Manager(EM)
*******************************************************************************/
static void sensorhub_em_task(void)
{
  sl_status_t status = SL_STATUS_OK;
  sl_em_event_t em_event;
  osStatus_t sl_semcq_status;
  sl_status_t em_wlan_status;
  sl_osMessageQueueAttr.name      = NULL;
  sl_osMessageQueueAttr.attr_bits = 0;
  sl_osMessageQueueAttr.cb_mem    = NULL;
  sl_osMessageQueueAttr.cb_size   = 0;
  sl_osMessageQueueAttr.mq_mem    = NULL;
  sl_osMessageQueueAttr.mq_size   = 0;

  sl_em_osMutexAttr_t.name      = NULL;
  sl_em_osMutexAttr_t.attr_bits = 0;
  sl_em_osMutexAttr_t.cb_mem    = NULL;
  sl_em_osMutexAttr_t.cb_size   = 0;

  osStatus_t sl_em_mutex_acc_status = 0, sl_em_mutex_rel_status = 0, sl_semrel_status = 0;
  sl_semaphore_em_task_id = osSemaphoreNew(1U, 0U, NULL);

  /*Create an Event Queue*/
  sl_event_queue_handler = osMessageQueueNew(20, sizeof(sl_em_event_t), &sl_osMessageQueueAttr); //CMSIS V2 API
  if (sl_event_queue_handler == NULL) {
    DEBUGOUT("create event queue failed");
    while (1)
      ;
  }

  sl_event_queue_mutex = osMutexNew(&sl_em_osMutexAttr_t);
  if (sl_event_queue_mutex == NULL) {
    DEBUGOUT("create event mutex failed");
    while (1)
      ;
  }

  while (1) {
    /*Wait for the Event on Queue*/
    while ((osMessageQueueGet(sl_event_queue_handler, &em_event, NULL, SL_EM_TASK_RUN_TICKS)) == osOK) {

      /*Acquire the mutex*/
      sl_em_mutex_acc_status = osMutexAcquire(sl_event_queue_mutex, osWaitForever);
      if (sl_em_mutex_acc_status != osOK) {
        DEBUGOUT("\r\n Mutex Acquire fail:%d \r\n", sl_em_mutex_acc_status);
      }
#ifdef SL_SH_POWER_STATE_TRANSITIONS
      if (em_event.event == SL_SENSOR_DATA_READY) {
        if (sl_si91x_power_manager_get_current_state() == SL_SI91X_POWER_MANAGER_PS2
            && sensorhub_current_powerstate == SL_SI91X_POWER_MANAGER_PS2) {
          status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\n Add PS requirement PS4 fail %lu", status);
          }
          status = sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\n Remove PS requirement PS2 fail %lu", status);
          }
          if (status == SL_STATUS_OK) {
            sensorhub_current_powerstate = SL_SI91X_POWER_MANAGER_PS4;
          }
        }
      }
#endif
      /*Call the handler and notify the event */
      cb_info.cb_event(em_event.sensor_id, em_event.event, em_event.em_sensor_data);

      if (*cb_info.cb_event_ack != SL_STATUS_OK) {
        for (uint8_t i = 0; i < SL_MAX_NUM_SENSORS; i++) {
          if (sensor_list.sl_sensors_st[i].config_st->sensor_id == *cb_info.cb_event_ack) {
            sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number = 0;
            sensor_list.sl_sensors_st[i].event_ack                          = CLEAR_EVENT_ACK;
            *cb_info.cb_event_ack                                           = (sl_sensor_id_t)NULL;
          }
        }
      }
      /*Release the mutex*/
      sl_em_mutex_rel_status = osMutexRelease(sl_event_queue_mutex);
      if (sl_em_mutex_rel_status != osOK) {
        DEBUGOUT("\r\n Mutex Release fail:%d\r\n", sl_em_mutex_rel_status);
      }

#ifdef SL_SH_POWER_STATE_TRANSITIONS
      if (em_event.event == SL_SENSOR_DATA_READY) {
        if (sl_si91x_power_manager_get_current_state() == SL_SI91X_POWER_MANAGER_PS4
            && sensorhub_current_powerstate == SL_SI91X_POWER_MANAGER_PS4) {
          status = sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS2);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\n ADD PS requirement PS2 fail %lu", status);
          }
          status = sl_si91x_power_manager_remove_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\n Remove PS requirement PS4 fail %lu", status);
          }
          if (status == SL_STATUS_OK) {
            sensorhub_current_powerstate = SL_SI91X_POWER_MANAGER_PS2;
          }
        }
      }
#endif
#ifdef SL_SH_PS1_STATE
      osStatus_t sl_semrel_status;
      if (em_event.event == SL_SENSOR_DATA_READY) {
        sl_power_state_enum = SL_SH_SLEEP_WAKEUP;
        sl_semrel_status    = osSemaphoreRelease(sl_semaphore_power_task_id);
        if (sl_semrel_status != osOK) {
          DEBUGOUT("\r\n SL_SH_SLEEP_WAKEUP Semaphore Release fail :%d \r\n", sl_semrel_status);
        }
      }
#endif
    } //end of os_message Queue
    if (*cb_info.cb_event_ack != SL_STATUS_OK) {
      for (uint8_t i = 0; i < SL_MAX_NUM_SENSORS; i++) {
        if (sensor_list.sl_sensors_st[i].config_st->sensor_id == *cb_info.cb_event_ack) {
          sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number = 0;
          sensor_list.sl_sensors_st[i].event_ack                          = CLEAR_EVENT_ACK;
          *cb_info.cb_event_ack                                           = (sl_sensor_id_t)NULL;
        }
      }
    } //end of if
  }   // end of while(1);
} //end of file

/**************************************************************************/ /**
 *  @fn      static void sensorhub_sensor_task(void)
 *  @brief       Task to handle the sensor operations.
*******************************************************************************/
static void sensorhub_sensor_task(void)
{
  uint32_t event_flags;
  static uint8_t i;
  osStatus_t sl_mutex_acc_status, sl_mutex_rel_status;
  int32_t status = 0;

  //Set event flags attributes
  sl_eventFlagsAttr.name      = NULL;
  sl_eventFlagsAttr.attr_bits = 0;
  sl_eventFlagsAttr.cb_mem    = NULL;
  sl_eventFlagsAttr.cb_size   = 0;

  //Set mutex flags attributes
  sl_osMutexAttr_t.name      = NULL;
  sl_osMutexAttr_t.attr_bits = 0;
  sl_osMutexAttr_t.cb_mem    = NULL;
  sl_osMutexAttr_t.cb_size   = 0;

  sl_event_group = osEventFlagsNew(&sl_eventFlagsAttr);
  if (sl_event_group == NULL) {
    // Event Flags object not created, handle failure
    DEBUGOUT("\r\n create sl_event_group failed in sensor HUB Task \r\n");
    while (1)
      ;
  }

  sl_sensor_mutex = osMutexNew(&sl_osMutexAttr_t);
  if (sl_sensor_mutex == NULL) {
    DEBUGOUT("create sensor mutex failed");
    while (1)
      ;
  }

  while (1) {

    event_flags = osEventFlagsWait(sl_event_group, sl_sensor_wait_flags, 0, (uint32_t)osWaitForever);

    sl_mutex_acc_status = osMutexAcquire(sl_sensor_mutex, osWaitForever);
    if (sl_mutex_acc_status != osOK) {
      DEBUGOUT("\r\n Mutex Acquire fail:%d \r\n", sl_mutex_acc_status);
    }
    i = 0;
    while (event_flags) {

      if ((event_flags & 0x1) == 0x1) {

        if (sensor_list.sl_sensors_st[i].event_ack == CLEAR_EVENT_ACK) {
          if (sensor_list.sl_sensors_st[i].config_st->data_deliver.data_mode == SL_SH_THRESHOLD) {
            sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number = 0;
          }
          if (sensor_list.sl_sensors_st[i].config_st->sensor_mode == SL_SH_POLLING_MODE) {
            if ((sensor_list.sl_sensors_st[i].config_st->sensor_bus == SL_SH_I2C) && !bus_errors.i2c) {
              sensorhub_i2c_init();
              bus_errors.i2c = true;
            }
            if ((sensor_list.sl_sensors_st[i].config_st->sensor_bus == SL_SH_SPI) && !bus_errors.spi) {
              sensorhub_spi_init();
              bus_errors.spi = true;
            }
            if ((sensor_list.sl_sensors_st[i].config_st->sensor_bus == SL_SH_ADC) && !bus_errors.adc) {
              sensorhub_adc_init();
              sl_status_t ret =
                sl_si91x_adc_channel_init(&bus_intf_info.adc_config.adc_ch_cfg, &bus_intf_info.adc_config.adc_cfg);
              if (ret != SL_STATUS_OK) {
                DEBUGOUT("\r\n ADC sensor channel init failed after wakeup \r\n");
              }
              bus_errors.adc = true;
            }
            status =
              sensor_list.sl_sensors_st[i].sensor_impl->sample(sensor_list.sl_sensors_st[i].sensor_handle,
                                                               sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr);
            if (status != SL_STATUS_OK) {
              DEBUGOUT("\r\n Sensor polling sample fail:%d \r\n", sensor_list.sl_sensors_st[i].config_st->sensor_id);
            }
          }
        }

        if (sensor_list.sl_sensors_st[i].config_st->sensor_mode == SL_SH_INTERRUPT_MODE) {
          status = sensor_list.sl_sensors_st[i].sensor_impl->sample(
            sensor_list.sl_sensors_st[i].sensor_handle,
            (sl_sensor_data_group_t *)sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr);
          if (status != SL_STATUS_OK) {
            DEBUGOUT("\r\n Sensor Interrupt sample fail %d \r\n", sensor_list.sl_sensors_st[i].config_st->sensor_id);
          }
        }

        switch (sensor_list.sl_sensors_st[i].config_st->data_deliver.data_mode) {
          case SL_SH_THRESHOLD:
            if (sensor_list.sl_sensors_st[i].config_st->sensor_bus == SL_SH_ADC) {
              if (sensor_list.sl_sensors_st[i]
                    .config_st->sensor_data_ptr
                    ->sensor_data[sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number - 1]
                    .adc[0]
                  >= sensor_list.sl_sensors_st[i].config_st->data_deliver.threshold) {
                if (sensor_list.sl_sensors_st[i].event_ack == CLEAR_EVENT_ACK) {
                  sensor_list.sl_sensors_st[i].event_ack = SET_EVENT_ACK;
                  sensorhub_em_post_event(sensor_list.sl_sensors_st[i].config_st->sensor_id,
                                          SL_SENSOR_DATA_READY,
                                          sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr,
                                          EM_POST_TIME);
                }
              }
            } else {
              if (sensor_list.sl_sensors_st[i]
                    .config_st->sensor_data_ptr
                    ->sensor_data[sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number - 1]
                    .light
                  >= sensor_list.sl_sensors_st[i].config_st->data_deliver.threshold) {
                if (sensor_list.sl_sensors_st[i].event_ack == CLEAR_EVENT_ACK) {
                  sensor_list.sl_sensors_st[i].event_ack = SET_EVENT_ACK;
                  sensorhub_em_post_event(sensor_list.sl_sensors_st[i].config_st->sensor_id,
                                          SL_SENSOR_DATA_READY,
                                          sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr,
                                          EM_POST_TIME);
                }
              }
            }
            break;

          case SL_SH_TIMEOUT:
            if (sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number
                == (sensor_list.sl_sensors_st[i].config_st->data_deliver.timeout
                    / sensor_list.sl_sensors_st[i].config_st->sampling_interval)) {
              if (sensor_list.sl_sensors_st[i].event_ack == CLEAR_EVENT_ACK) {
                sensor_list.sl_sensors_st[i].event_ack = SET_EVENT_ACK;
                sensorhub_em_post_event(sensor_list.sl_sensors_st[i].config_st->sensor_id,
                                        SL_SENSOR_DATA_READY,
                                        sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr,
                                        EM_POST_TIME);
              }
            }
            break;

          case SL_SH_NUM_OF_SAMPLES:
            if (sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr->number
                == (sensor_list.sl_sensors_st[i].config_st->data_deliver.numofsamples)) {
              if (sensor_list.sl_sensors_st[i].event_ack == CLEAR_EVENT_ACK) {
                sensor_list.sl_sensors_st[i].event_ack = SET_EVENT_ACK;
                sensorhub_em_post_event(sensor_list.sl_sensors_st[i].config_st->sensor_id,
                                        SL_SENSOR_DATA_READY,
                                        sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr,
                                        EM_POST_TIME);
              }
            }

            break;

          case SL_SH_NO_DATA_MODE:
            // Reference for the Sensor interrupt Mode
            if (sensor_list.sl_sensors_st[i].event_ack == CLEAR_EVENT_ACK) {
              sensor_list.sl_sensors_st[i].event_ack = SET_EVENT_ACK;
              sensorhub_em_post_event(sensor_list.sl_sensors_st[i].config_st->sensor_id,
                                      SL_SENSOR_DATA_READY,
                                      sensor_list.sl_sensors_st[i].config_st->sensor_data_ptr,
                                      EM_POST_TIME);
              RSI_NPSSGPIO_IntrUnMask(BIT(sensor_list.sl_sensors_st[i].config_st->sampling_intr_req_pin));
            }
            break;

          default:
            /*Do nothing*/
            break;
        }
      }
      i++;
      event_flags = event_flags >> 1;
    } // events while();

    sl_mutex_rel_status = osMutexRelease(sl_sensor_mutex);
    if (sl_mutex_rel_status != osOK) {
      DEBUGOUT("\r\n Mutex Release fail:%d \r\n", sl_mutex_rel_status);
    }
  } //end of while(1);
} //end of sensor task

/**************************************************************************/ /**
* @fn       void mySPI_callback(uint32_t event)
* @brief  SPI callback handler
* @param[in]event SPI transmit and receive events
* @return   None
*******************************************************************************/
void mySPI_callback(uint32_t event)
{
  switch (event) {
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
      break;
    case ARM_SPI_EVENT_DATA_LOST:
      /*        Occurs in slave mode when data is requested/sent by the master
            but the send/receive/transfer operation has not been started
            and indicates that data is lost. Occurs also in master mode
            when the driver cannot transfer data fast enough.*/
      DEBUGOUT("\r\n ARM_SPI_EVENT_DATA_LOST \r\n");
      break;
    case ARM_SPI_EVENT_MODE_FAULT:
      /*        Occurs in master mode when Slave Select is deactivated and
            indicates Master Mode Fault.*/
      DEBUGOUT("\r\n ARM_SPI_EVENT_MODE_FAULT \r\n");
      break;
  }
}

/**************************************************************************/ /**
*  @fn          sl_status_t sensorhub_spi_init(void)
*  @brief       Initialize the SPI Interface based on the configuration.
*******************************************************************************/
static int32_t sensorhub_spi_init(void)
{
  int32_t status = 0;

  // UNInitialize the SPI driver
  status = (int32_t)(SPIdrv->Uninitialize());

  // Initialize the SPI driver
  status = (int32_t)SPIdrv->Initialize(mySPI_callback);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n SPI Initialization Failed, Error Code : %ld\r\n", status);
    return status;
  } else {
    //DEBUGOUT("\r\n SPI Initialization Success\r\n");
  }

  RSI_SPI_SetSlaveSelectNumber(bus_intf_info.spi_config.spi_cs_number);

  // Power up the SPI peripheral
  status = (int32_t)SPIdrv->PowerControl(bus_intf_info.spi_config.spi_power_state);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Set Power to SPI, Error Code : %ld \r\n", status);
    return status;
  }
  // Configure the SPI to Master, 16-bit mode @10000 kBits/sec
  status = (int32_t)SPIdrv->Control(bus_intf_info.spi_config.spi_mode | bus_intf_info.spi_config.spi_control_mode
                                      | bus_intf_info.spi_config.spi_cs_mode
                                      | ARM_SPI_DATA_BITS(bus_intf_info.spi_config.spi_bit_width),
                                    bus_intf_info.spi_config.spi_baud);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Set Configuration Parameters to SPI, Error Code : %ld \r\n", status);
    return status;
  }
  // SS line = ACTIVE = LOW
  status =
    (int32_t)SPIdrv->Control(bus_intf_info.spi_config.spi_cs_misc_mode, bus_intf_info.spi_config.spi_sec_sel_sig);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\n Failed to Set Configuration Parameters to SPI, Error Code : %ld \r\n", status);
    return status;
  }
  //  DEBUGOUT("\r\n SPI Initialization Success\r\n");
  return SL_STATUS_OK;
}

/**************************************************************************/ /**
 * @fn         void ARM_I2C_SignalEvent(uint32_t event)
 * @brief      I2C event handler
 * @param[in]  event I2C transmit and receive events
 * @return     None
*******************************************************************************/
void ARM_I2C_SignalEvent(uint32_t event)
{
  switch (event) {
    case ARM_I2C_EVENT_TRANSFER_DONE:
      break;
    case ARM_I2C_EVENT_TRANSFER_INCOMPLETE:
      break;
    case ARM_I2C_EVENT_SLAVE_TRANSMIT:
      break;
    case ARM_I2C_EVENT_SLAVE_RECEIVE:
      break;
    case ARM_I2C_EVENT_ADDRESS_NACK:
      break;
    case ARM_I2C_EVENT_GENERAL_CALL:
      break;
    case ARM_I2C_EVENT_ARBITRATION_LOST:
      break;
    case ARM_I2C_EVENT_BUS_ERROR:
      break;
    case ARM_I2C_EVENT_BUS_CLEAR:
      break;
  }
  return;
}

/**************************************************************************/ /**
 *  @fn         static int32_t sensorhub_i2c_init(void)
 *  @brief       Initialize I2C Interface based on configuration
 *  @return      execution status
*******************************************************************************/
static int32_t sensorhub_i2c_init(void)
{
  int32_t status = ARM_DRIVER_OK;

  status = (int32_t)I2Cdrv->Uninitialize();

  status = (int32_t)I2Cdrv->Initialize(ARM_I2C_SignalEvent);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\nI2C Initialization Failed Error Code:%ld \r\n", status);
    return status;
  } else {
    //DEBUGOUT("\r\n I2C Initialization Success \r\n");
  }

  status = (int32_t)I2Cdrv->PowerControl(bus_intf_info.i2c_config.i2c_power_state);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\nFailed to Set Power to I2C, Error Code : %ld\r\n", status);
    return status;
  } else {
    //DEBUGOUT("\r\n Set Power mode to I2C is Success \r\n");
  }

  status = (int32_t)I2Cdrv->Control(bus_intf_info.i2c_config.i2c_control_mode, bus_intf_info.i2c_config.i2c_bus_speed);
  if (status != ARM_DRIVER_OK) {
    DEBUGOUT("\r\nFailed to Set Configuration Parameters to I2C, Error Code : %ld\r\n", status);
    return status;
  } else {
    //DEBUGOUT("\r\n Set Control mode to I2C is Success\r\n");
  }
  // DEBUGOUT("\r\n I2C Initialization Success \r\n");
  return status;
}

/**************************************************************************/ /**
 *  @fn      static sl_status_t sensorhub_i2c_sensors_scan(uint16_t address)
 *  @brief       Scan the i2c sensors based on the address
 *  @param[in]   address Sensor address
 *  @return status 0 if successful, it will wait for the sensor response.
*******************************************************************************/
static int32_t sensorhub_i2c_sensors_scan(uint16_t address)
{
  uint8_t a[2] = "\0";
  int32_t status;

  a[0] = (uint8_t)(address & 0xFF);

  status = (int32_t)I2Cdrv->MasterTransmit(address, a, 1, false);
  if (status != ARM_DRIVER_OK) {
    return status;
  }
  while (I2Cdrv->GetStatus().busy)
    ;
  return SL_STATUS_OK;
}
/**************************************************************************/ /**
 *  @fn          uint8_t sli_si91x_i2c_init() is deprecated API not longer in use
 *  @brief       Initialize I2C Interface based on configuration
 *  @return      execution status
*******************************************************************************/
int32_t sli_si91x_i2c_init(void)
{
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
*  @fn          sl_status_t sli_si91x_spi_init(void)is deprecated API not longer in use
*  @brief       Initialize the SPI Interface based on the configuration.
*******************************************************************************/
int32_t sli_si91x_spi_init(void)
{
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn          void sl_si91x_sensor_task(void)is deprecated API not longer in use
 *  @brief       Task to handle the sensor operations.
*******************************************************************************/
void sl_si91x_sensor_task(void)
{
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 *  @fn          void EM_Task(void)is deprecated API not longer in use
 *  @brief       Task to handle the operations of the Event Manager(EM)
*******************************************************************************/
void sl_si91x_em_task(void)
{
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 *  @fn         sl_status_t sli_si91x_i2c_sensors_scan(uint16_t address)is deprecated API not longer in use
 *  @brief       Scan the i2c sensors based on the address
 *  @param[in]   address Sensor address
 *  @return status 0 if successful, it will wait for the sensor response.
*******************************************************************************/
int32_t sli_si91x_i2c_sensors_scan(uint16_t address)
{
  (void)address;
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn         void sl_si91x_em_post_event(sl_sensor_id_en sensor_id,
 *  sl_sensorhub_event_en event, void* dataPtr, TickType_t ticks_to_wait)is deprecated API not longer in use
 *  @brief       To post the events/notifications to event manager(EM) to be notified to the application
 *  @param[in]   sensor_id      id of the  sensor
 *  @param[in]   event          event/notifications
 *  @param[in]   dataPtr        pointer to the data
 *  @param[in]   ticks_to_wait  max time to wait for the post
*******************************************************************************/
void sl_si91x_em_post_event(sl_sensor_id_t sensor_id,
                            sl_sensorhub_event_t event,
                            void *dataPtr,
                            TickType_t ticks_to_wait)
{
  (void)sensor_id;
  (void)event;
  (void)dataPtr;
  (void)ticks_to_wait;
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 *  @fn          sl_sensor_impl_type* sli_si91x_get_sensor_implementation(int32_t sensor_id)is deprecated API not longer in use
 *  @brief       To get the appropriate sensor driver implementation.
 *  @param[in]   sensor_id     ID of the target sensor
 *  @return      Reference to the sensor driver
*******************************************************************************/
sl_sensor_impl_type_t *sli_si91x_get_sensor_implementation(int32_t sensor_id)
{
  (void)sensor_id;
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn        int32_t sli_si91x_create_sensor_list_index()is deprecated API not longer in use
 *  @brief       To create a sensor index in the list.
 * @return if successful, returns the sensor index
 *         else it will break the sensor list.
*******************************************************************************/
int32_t sli_si91x_create_sensor_list_index()
{
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn       int32_t sli_si91x_get_sensor_index(sl_sensor_id_en sensor_id)is deprecated API not longer in use
 *  @brief       To get the index of the sensor present in the list
 *  @param[in]   sensor_id     id of the target sensor
 *  @return if successful, returns the sensor index
 *         else it returns the 0XFF as a sensor index fail.
*******************************************************************************/
uint32_t sli_si91x_get_sensor_index(sl_sensor_id_t sensor_id)
{
  (void)sensor_id;
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn          int32_t sli_si91x_delete_sensor_list_index(sl_sensor_id_en sensor_id)is deprecated API not longer in use
 *  @brief       To delete the sensor index from the list
 *  @param[in]   sensor_id     id of the target sensor
 *  @return      index of the sensor
*******************************************************************************/
uint32_t sli_si91x_delete_sensor_list_index(sl_sensor_id_t sensor_id)
{
  (void)sensor_id;
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn         sl_sensor_info_t* sli_si91x_get_sensor_info(sl_sensor_id_en sensor_id)is deprecated API not longer in use
 *  @brief       To get the info on the configured sensor
 *  @param[in]   sensor_id     id of the target sensor
 *  @return      reference to the sensor info
*******************************************************************************/
sl_sensor_info_t *sli_si91x_get_sensor_info(sl_sensor_id_t sensor_id)
{
  (void)sensor_id;
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn          void sl_si91x_sensors_timer_cb(TimerHandle_t xTimer)is deprecated API not longer in use
 *  @brief       software timer callback
 *  @param[in]   xTimer     handle to the timer
*******************************************************************************/
void sl_si91x_sensors_timer_cb(TimerHandle_t xTimer)
{
  (void)xTimer;
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 *  @fn          void sl_si91x_gpio_interrupt_config(uint16_t gpio_pin,sl_gpio_intr_type intr_type)is deprecated API not longer in use
 *  @brief       Configuring the different types of NPSS GPIO interrupts.
 *  @param[in]   gpio_pin     GPIO pin number
 *  @param[in]   intr_type    type of interrupt.
*******************************************************************************/
sl_status_t sl_si91x_gpio_interrupt_config(uint16_t gpio_pin, sl_si91x_gpio_interrupt_config_flag_t intr_type)
{
  (void)gpio_pin;
  (void)intr_type;
  /* Deprecated API no longer in use */
  return 0;
}
/**************************************************************************/ /**
 *  @fn         void sl_si91x_gpio_interrupt_start(uint16_t gpio_pin)is deprecated API not longer in use
 *  @brief       Enable and set the priority of GPIO interrupt.
 *  @param[in]   gpio_pin     GPIO pin number
*******************************************************************************/
void sl_si91x_gpio_interrupt_start(uint16_t gpio_pin)
{
  (void)gpio_pin;
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 *  @fn          void sl_si91x_gpio_interrupt_stop(uint16_t gpio_pin)is deprecated API not longer in use
 *  @brief       Mask and Disable the GPIO interrupt.
 *  @param[in]   gpio_pin     GPIO pin number
*******************************************************************************/
void sl_si91x_gpio_interrupt_stop(uint16_t gpio_pin)
{
  (void)gpio_pin;
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 * @fn            void sli_si91x_sensorhub_ps4tops2_state(void)is deprecated API not longer in use
 * @brief        This function changed the system status from PS4 to PS2.
 * @param[in]    None
 * @param[out]   None
*******************************************************************************/
#ifdef SL_SH_POWER_STATE_TRANSITIONS
void sli_si91x_sensorhub_ps4tops2_state(void)
{
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 * @fn           void sli_si91x_sensorhub_ps2tops4_state(void)is deprecated API not longer in use
 * @brief        This function changed the system status from PS4 to PS2.
 * @param[in]    None
 * @param[out]   None
*******************************************************************************/
void sli_si91x_sensorhub_ps2tops4_state(void)
{
  /* Deprecated API no longer in use */
  return;
}

#endif
/***************************************************************************/ /**
  * @fn         void sli_si91x_set_alarm_intr_time(uint16_t interval) is deprecated API no longer in use
* @brief To set the alarm interrupt time.
*
* @details
* This function will set the alarm interrupt based on the periodic time.
*
* @param[in] interval   -   interval time
*
******************************************************************************/
void sli_si91x_set_alarm_intr_time(uint16_t interval)
{
  (void)interval;
  /* Deprecated API no longer in use */
  return;
}

/**************************************************************************/ /**
 *  @fn          sl_status_t sli_si91x_adc_init(void) is deprecated API no longer in use
 *  @brief       Initialize the ADC Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
sl_status_t sli_si91x_adc_init(void)
{
  /* Deprecated API no longer in use */
  return 0;
}

/***************************************************************************/ /**
* @fn         void sli_si91x_init_m4alarm_config(void) is deprecated API not longer in use
* @brief To initialize the Alarm block 
* @details
* This function will initialize the Alarm block.
*
*
******************************************************************************/
void sli_si91x_init_m4alarm_config(void)
{
  /* Deprecated API no longer in use */
  return;
}
/***************************************************************************/ /**
* @fn         void sl_si91x_power_state_task(void) is deprecated API not longer in use
* @brief Task to handle the system power operations
*
* @details
* Power state task changes the system from one power save mode to another power save mode like(PS4 to PS2),(PS2toPS4),(Sleep_mode) using Binary semaphore.
*
******************************************************************************/
void sl_si91x_power_state_task(void)
{
  /* Deprecated API no longer in use */
  return;
}

/***************************************************************************/ /**
* @fn         void sli_si91x_config_wakeup_source(uint16_t sleep_time) is deprecated API not longer in use
* @brief To configure wake-up source for the system
*
* @details
* This function will configure the wake-up source to the system.
*
* @param[in] sleep_time  -   Sleep time for the sensor hub.
*
******************************************************************************/
void sli_si91x_config_wakeup_source(uint16_t sleep_time)
{
  (void)sleep_time;
  /* Deprecated API no longer in use */
  return;
}
/***************************************************************************/ /**
* @fn         void sli_si91x_sleep_wakeup(uint16_t sh_sleep_time) is deprecated API not longer in use
* @brief To configures sleep/wakeup sources for the system.
*
* @details
* This function will configure sleep/wakeup sources.
*
* @param[in] sh_sleep_time  -   Sleep time for the sensor hub, in ADC PS-1 no parameters.
*
******************************************************************************/
#ifdef SL_SH_PS1_STATE
void sli_si91x_sleep_wakeup(void)
{
  /* Deprecated API no longer in use */
  return;
}
#else
void sli_si91x_sleep_wakeup(uint16_t sh_sleep_time)
{
  (void)sh_sleep_time;
  /* Deprecated API no longer in use */
  return;
}
#endif
/*******************************************************************************
 * ADC user callback
 * This function will be called from ADC interrupt handler
 *
 * @param[in] ADC channel number
 * @param[in] ADC callback event (ADC_STATIC_MODE_CALLBACK,
 *            ADC_THRSHOLD_CALLBACK, INTERNAL_DMA, FIFO_MODE_EVENT)
 *
*******************************************************************************/
void sl_si91x_adc_callback(uint8_t channel_no, uint8_t event)
{
  (void)channel_no;
  (void)event;
  /* Deprecated API no longer in use */
  return;
}
/**************************************************************************/ /**
 *  @fn          uint8_t sli_si91x_sdc_init(void)is deprecated API not longer in use
 *  @brief       Initialize the sdc Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
#ifdef SH_SDC_ENABLE
sl_status_t sli_si91x_sdc_init(void)
{
  /* Deprecated API no longer in use */
  return 0;
}
#endif
/**************************************************************************/ /**
 *  @fn         void sli_config_sdc_params(sl_sdc_config_t * sdc_config)is deprecated API not longer in use
 *  @brief       Initialize the sdc Interface based on the configuration.
 *  @param[in]   None
 *  @return      status 0 if successful,
 *               else error code
 *               \ref SL_STATUS_FAIL (0x0001)- Fail ,
 *               \ref SL_STATUS_OK (0X000)- Success,
*******************************************************************************/
#ifdef SH_SDC_ENABLE
void sli_config_sdc_params(sl_drv_sdc_config_t *sdc_config_st_p)
{
  (void)sdc_config_st_p;
  /* Deprecated API no longer in use */
  return;
}
#endif
///**************************************************************************/ /**
/** @} */
