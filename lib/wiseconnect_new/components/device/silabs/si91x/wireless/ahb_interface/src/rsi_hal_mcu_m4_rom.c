/***************************************************************************/ /**
 * @file  rsi_hal_mcu_m4_rom.c
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sli_wifi_utility.h"
#include "sl_si91x_types.h"
#include "sl_constants.h"
#include "sl_status.h"
#include "sl_device.h"
#include "sl_rsi_utility.h"
#include "rsi_m4.h"
#include "rsi_ipmu.h"

#ifdef SL_WIFI_COMPONENT_INCLUDED
#include "sl_si91x_host_interface.h"
#endif

#include "cmsis_os2.h"
osEventFlagsId_t ta_events = NULL;
#define TA_PKT_TX_DONE (1 << 1)
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
#define SIDE_BAND_DONE (1 << 2)
#endif

static bool m4_is_using_xtal_without_ta_notification;
static bool m4_using_xtal;

/** @addtogroup SOC4
* @{
*/
/**
 * @fn           void sli_si91x_raise_pkt_pending_interrupt_to_ta(void)
 * @brief        Raise the packet pending interrupt to NWP
 * @param[in]    void  
 * @return       void
 */
void sli_si91x_raise_pkt_pending_interrupt_to_ta(void)
{
  // Write the packet pending interrupt to NWP register
  M4SS_P2P_INTR_SET_REG = TX_PKT_PENDING_INTERRUPT;
  osEventFlagsWait(ta_events, TA_PKT_TX_DONE, osFlagsWaitAny, osWaitForever);
}
/**
 * @fn          bool sli_si91x_is_m4_using_xtal(void);
 * @brief       This API is used to get the whether XTAL is enabled by M4 without notifying NWP
 * @return      true  : XTAL is enabled by M4 without notifying NWP
 *              false : XTAL is not enabled by M4
 */
bool sli_si91x_is_m4_using_xtal(void)
{
  return m4_is_using_xtal_without_ta_notification;
}
/**
 * @fn          void sli_si91x_set_m4_is_using_xtal(void);
 * @brief       This API is set  XTAL is enabled by M4 without notifying NWP
 */
void sli_si91x_set_m4_is_using_xtal(void)
{
  m4_is_using_xtal_without_ta_notification = true;
}

/**
 * @fn          bool sli_si91x_is_xtal_in_use_by_m4(void);
 * @brief       This API is used to get the whether XTAL is used by M4 or any of HP peripherals
 * @return      true  : XTAL is being used by M4 or HP peripherals
 *              false : XTAL is not being used
 */
bool sli_si91x_is_xtal_in_use_by_m4(void)
{
  return m4_using_xtal;
}

/**
 * @fn          void sli_si91x_set_xtal_in_use_by_m4(void);
 * @brief       This API is used set XTAL is used by M4 or any of HP peripherals
 */
void sli_si91x_set_xtal_in_use_by_m4(void)
{
  m4_using_xtal = true;
}

/**
 * @fn          void sli_si91x_clear_xtal_in_use_by_m4(void);
 * @brief       This API is used to clear XTAL usage by M4 or any of HP peripherals
 */
void sli_si91x_clear_xtal_in_use_by_m4(void)
{
  m4_using_xtal = false;
}
/**
 * @fn          void sli_si91x_xtal_turn_off_request_from_m4_to_TA(void);
 * @brief       This API is used to Notify NWP that M4 doesn't requires XTAL clock source
 */
void sli_si91x_xtal_turn_off_request_from_m4_to_TA(void)
{
  if (sli_si91x_is_xtal_in_use_by_m4() == true) {
    /* If M4 is using XTAL then request NWP to turn OFF XTAL as M4 is going to sleep */
    sli_si91x_raise_xtal_interrupt_to_ta(TURN_OFF_XTAL_REQUEST);
    sli_si91x_clear_xtal_in_use_by_m4();
  }
}
/**
 * @fn          void sli_si91x_xtal_turn_on_request_from_m4_to_TA(void);
 * @brief       This API is used to Notify NWP that M4 requires XTAL clock source 
 */
void sli_si91x_xtal_turn_on_request_from_m4_to_TA(void)
{
  if (sli_si91x_is_xtal_in_use_by_m4() == false) {
    if (TASS_P2P_INTR_CLEAR_REG & TURN_ON_XTAL_REQUEST) {
      clear_ta_to_m4_interrupt(TURN_ON_XTAL_REQUEST);
    } else {

      /* Confirm if the NWP has completed its initialization process */
      if (sl_si91x_is_device_initialized()) {
        /* Raise the turn ON xtal interrupt to NWP */
        sli_si91x_raise_xtal_interrupt_to_ta(TURN_ON_XTAL_REQUEST);
        /* If M4 is using XTAL then notify NWP to turn ON XTAL during programing common flash*/
        sli_si91x_raise_xtal_interrupt_to_ta(M4_IS_USING_XTAL_REQUEST);
      }
      /*If the 'M4 Enabled XTAL without NWP Notification,
* then after net initialization (NWP device initialization), a request to turn on the XTAL will be sent to the NWP*/
      else {
        /* set  XTAL is enabled by M4 without notifying NWP */
        sli_si91x_set_m4_is_using_xtal();
      }
    }
    /* Set M4 XTAL usage flag */
    sli_si91x_set_xtal_in_use_by_m4();
  }
}

/**
 * @fn           void sli_si91x_raise_xtal_interrupt_to_ta(uint16_t interrupt_no)
 * @brief        Raise the turn on/off xtal interrupt to NWP
 * @param[in]    xtal_enable - true to enable xtal, false to disable xtal
 * @return       void
 */
void sli_si91x_raise_xtal_interrupt_to_ta(uint16_t interrupt_no)
{
  //! Wake up NWP
  P2P_STATUS_REG |= M4_WAKEUP_TA;

  //!wait for NWP active
  while (!(P2P_STATUS_REG & TA_IS_ACTIVE))
    ;

  // Write the turn_on_xtal interrupt to NWP register
  M4SS_P2P_INTR_SET_REG = interrupt_no;

  //! Poll for bit to clear
  //!Wait for NWP using flash bit
  while (!(TASS_P2P_INTR_CLEAR_REG & interrupt_no))
    ;
  clear_ta_to_m4_interrupt(interrupt_no);

  sl_si91x_host_clear_sleep_indicator();
}

/**
 * @fn          void sli_si91x_send_m4_xtal_usage_notification_to_ta(void);
 * @brief        This API sends a notification to the NWP indicating whether
 *               the M4 core is currently utilizing the XTAL as its clock source.
 */
void sli_si91x_send_m4_xtal_usage_notification_to_ta(void)
{

#if !(SLI_SI91X_MCU_PSRAM_PRESENT)
  /* Check whether M4 is using XTAL */
  if (sli_si91x_is_m4_using_xtal() == true)
#endif
  {
    /* If M4 is using XTAL then request NWP to turn ON XTAL*/
    sli_si91x_raise_xtal_interrupt_to_ta(TURN_ON_XTAL_REQUEST);
    /* If M4 is using XTAL then notify NWP to turn ON XTAL during programing common flash*/
    sli_si91x_raise_xtal_interrupt_to_ta(M4_IS_USING_XTAL_REQUEST);
  }
}

#ifdef SL_SI91X_SIDE_BAND_CRYPTO
/**
 * @fn           void sli_si91x_raise_side_band_interrupt_to_ta(void)
 * @brief        Raise the side band interrupt to NWP
 * @param[in]    void  
 * @return       void
 */
void sli_si91x_raise_side_band_interrupt_to_ta(void)
{
  // Write the packet pending interrupt to NWP register
  M4SS_P2P_INTR_SET_REG = SIDE_BAND_CRYPTO_INTR;
}
#endif

/**
 * @fn          void raise_m4_to_ta_interrupt(uint32_t interrupt_no)
 * @brief       Set interrupt.
 * @param[in]   interrupt_no - Process of a interrupt number 
 * @return      void 
 */

void raise_m4_to_ta_interrupt(uint32_t interrupt_no)
{
  M4SS_P2P_INTR_SET_REG = interrupt_no;
}

/**
 * @fn          void mask_ta_interrupt(uint32_t interrupt_no)
 * @brief       Process a interrupt mask.
 * @param[in]   void  
 * @return      void
 */
void mask_ta_interrupt(uint32_t interrupt_no)
{
  TASS_P2P_INTR_MASK_SET = interrupt_no;
}

/**
 * @fn          void clear_ta_to_m4_interrupt(uint32_t interrupt_no)
 * @brief       Clear interrupt raised by NWP.
 * @param[in]   interrupt_no - Process of a interrupt number 
 * @return      void 
 */
void clear_ta_to_m4_interrupt(uint32_t interrupt_no)
{
  TASS_P2P_INTR_CLEAR   = interrupt_no;
  TASS_P2P_INTR_CLR_REG = interrupt_no;
}

/**
 * @fn           void sli_m4_interrupt_isr(void)
 * @brief        Raise the packet pending interrupt to NWP
 * @param[in]    void
 * @return       void
 */

sl_status_t sli_m4_interrupt_isr(void)
{
  if (TASS_P2P_INTR_CLEAR & TX_PKT_TRANSFER_DONE_INTERRUPT) {

    osEventFlagsSet(ta_events, TA_PKT_TX_DONE);
    // Clear the interrupt
    clear_ta_to_m4_interrupt(TX_PKT_TRANSFER_DONE_INTERRUPT);

  } else if (TASS_P2P_INTR_CLEAR & RX_PKT_TRANSFER_DONE_INTERRUPT) {

    // Call done interrupt isr
    sl_status_t status = sli_receive_from_ta_done_isr();
    VERIFY_STATUS_AND_RETURN(status);

    // Clear the interrupt
    clear_ta_to_m4_interrupt(RX_PKT_TRANSFER_DONE_INTERRUPT);

  } else if (TASS_P2P_INTR_CLEAR & TA_RSI_BUFFER_FULL_CLEAR_EVENT) {

    mask_ta_interrupt(TA_RSI_BUFFER_FULL_CLEAR_EVENT);

    sli_wifi_set_event(SL_SI91X_TA_BUFFER_FULL_CLEAR_EVENT);

    // Clear the interrupt
    clear_ta_to_m4_interrupt(TA_RSI_BUFFER_FULL_CLEAR_EVENT);
  }
#if defined(SLI_SI917)
  else if (TASS_P2P_INTR_CLEAR & TA_WRITING_ON_COMM_FLASH) {
    //! moves m4 app to RAM and polls for NWP done
    sli_mv_m4_app_from_flash_to_ram(TA_WRITES_ON_COMM_FLASH);
    // Clear the interrupt
    clear_ta_to_m4_interrupt(TA_WRITING_ON_COMM_FLASH);
  } else if (TASS_P2P_INTR_CLEAR & NWP_DEINIT_IN_COMM_FLASH) {
    //! moves m4 app to RAM and polls for NWP done
    sli_mv_m4_app_from_flash_to_ram(M4_WAIT_FOR_NWP_DEINIT);
    // Clear the interrupt
    clear_ta_to_m4_interrupt(NWP_DEINIT_IN_COMM_FLASH);
  }
  //! Below changes are requried for M4 Image upgration in dual flash config
  else if (TASS_P2P_INTR_CLEAR & M4_IMAGE_UPGRADATION_PENDING_INTERRUPT) {
    //! moves m4 app to RAM and polls for NWP done
    sli_mv_m4_app_from_flash_to_ram(UPGRADE_M4_IMAGE_OTA);
    // Clear the interrupt
    clear_ta_to_m4_interrupt(M4_IMAGE_UPGRADATION_PENDING_INTERRUPT);
  }
#endif
#ifdef SL_SI91X_SIDE_BAND_CRYPTO
  //! Below changes are requried for SIDE BAND CRYPTO
  else if (TASS_P2P_INTR_CLEAR & SIDE_BAND_CRYPTO_DONE) {
    osEventFlagsSet(ta_events, SIDE_BAND_DONE);
    // Clear the interrupt
    clear_ta_to_m4_interrupt(SIDE_BAND_CRYPTO_DONE);
  }
#endif
  else {
    SL_DEBUG_LOG("\r\n INVALID INTERRUPT \r\n", 0);
    return SL_STATUS_FAIL;
  }
  return SL_STATUS_OK;
}

/**
 * @fn           sl_status_t sli_receive_from_ta_done_isr(void)
 * @brief        Called when DMA done for RX packet is received
 * @param[in]    global_cb_p - pointer to the global control block
 * @return       void
 */
sl_status_t sli_receive_from_ta_done_isr(void)
{
#ifdef SL_WIFI_COMPONENT_INCLUDED
  extern sl_wifi_buffer_t *rx_pkt_buffer;
  extern sli_wifi_buffer_queue_t sli_ahb_bus_rx_queue;
  // Add to rx packet to CCP queue
  sl_status_t status = sli_si91x_add_to_queue(&sli_ahb_bus_rx_queue, rx_pkt_buffer);
  VERIFY_STATUS_AND_RETURN(status);

  //! Set event RX pending event to host
  sli_wifi_set_event(SL_SI91X_NCP_HOST_BUS_RX_EVENT);
#endif

  return SL_STATUS_OK;
}

/*==================================================*/
/**
 * @fn          sl_status_t sli_si91x_bus_read_interrupt_status(uint8_t *int_status)
 * @brief       Returns the value of the Interrupt register
 * @param[in]   status
 * @param[out]  buffer full status reg value
 * @return      errorcode
 *               0 = Success
 *              -2 = Reg read failure
 */
sl_status_t sli_si91x_bus_read_interrupt_status(uint16_t *int_status)
{
  *int_status = (uint8_t)HOST_INTR_STATUS_REG;

  return RSI_SUCCESS;
}

sl_status_t sli_si91x_req_wakeup(void)
{
  P2P_STATUS_REG |= M4_wakeup_TA;
  if (!(P2P_STATUS_REG & TA_is_active)) {
    //!TBD Need add timeout
    while (!(P2P_STATUS_REG & TA_is_active))
      ;
  }
  return SL_STATUS_OK;
}

void sl_si91x_host_clear_sleep_indicator(void)
{
  P2P_STATUS_REG &= ~M4_wakeup_TA;
}

void IRQ074_Handler(void)
{
  sli_m4_interrupt_isr();
}

void sli_si91x_ta_events_init(void)
{
  if (ta_events == NULL) {
    ta_events = osEventFlagsNew(NULL);
  }
}
/** @} */
