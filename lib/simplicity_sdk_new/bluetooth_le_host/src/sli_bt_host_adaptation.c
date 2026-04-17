/***************************************************************************//**
 * @brief System adaptation layer used internally by Bluetooth host stack
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_bt_host_adaptation.h"
#include "sl_bluetooth_config.h"
#include "sl_bt_api.h"
#include "sl_assert.h"
#include "sl_core.h"
#include "em_device.h"
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
#include "btl_interface.h"
#endif

#if defined(SL_CATALOG_KERNEL_PRESENT)
#include "sl_bt_rtos_adaptation.h"

// When a kernel is present, the task wakeups are handled by RTOS adaptation
sli_bt_linklayer_wakeup_t *const sli_bt_host_adaptation_linklayer_wakeup = sli_bt_rtos_ll_callback;
sli_bt_host_wakeup_t *const sli_bt_host_adaptation_host_wakeup = sli_bt_rtos_stack_callback;

#else // defined(SL_CATALOG_KERNEL_PRESENT)

// When a kernel is not present, the host task wakeup is not needed, as the
// baremetal main loop will service the host stack. The linklayer task is
// handled as a PendSV interrupt.

/**
 * Override @ref PendSV_Handler for the Link Layer task when Bluetooth runs
 * in baremetal mode. The override must not exist when Bluetooth runs in an RTOS
 * where the link layer task runs in a thread.
 */
void PendSV_Handler()
{
  sl_bt_priority_handle();
}

static void sli_bt_trigger_pendsv_interrupt(void)
{
  // Trigger the PendSV interrupt
  SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

sli_bt_linklayer_wakeup_t *const sli_bt_host_adaptation_linklayer_wakeup = sli_bt_trigger_pendsv_interrupt;
sli_bt_host_wakeup_t *const sli_bt_host_adaptation_host_wakeup = NULL;

#endif // defined(SL_CATALOG_KERNEL_PRESENT)

// Get the bootloader version information
sl_status_t sli_bt_host_adaptation_get_bootloader_version(uint32_t *bootloader_version)
{
  EFM_ASSERT(bootloader_version != NULL);

#if defined(SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT)
  // Bootloader is available. Query the version from there.
  BootloaderInformation_t btl_info;
  memset(&btl_info, 0, sizeof(btl_info));
  bootloader_getInfo(&btl_info);
  *bootloader_version = btl_info.version;
  return SL_STATUS_OK;
#else // SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT
  // Bootloader is not available
  *bootloader_version = 0;
  return SL_STATUS_NOT_AVAILABLE;
#endif // SL_CATALOG_GECKO_BOOTLOADER_INTERFACE_PRESENT
}

// Reset the chip
void sli_bt_host_adaptation_chip_reset(void)
{
  CORE_ResetSystem();
}

#if defined(SL_CATALOG_NVM3_PRESENT)
#include "nvm3_generic.h"
#include "nvm3_default.h"

/**
 * @brief NVM3 handle used for NVM3 in the Bluetooth host stack
 */
#define SL_BT_NVM3 (nvm3_defaultHandle)

/**
 * @brief NVM3 key for custom Bluetooth address type. Value 1 byte: 0 for public type, 1 for static
 */
#define SLI_BT_NVM3_LOCAL_BD_ADDR_TYPE 0x4003c

/**
 * @brief NVM3 key for custom Bluetooth address. Value: 6 bytes in little endian
 */
#define SLI_BT_NVM3_LOCAL_BD_ADDR      0x4002c

/**
 * @brief Length of a Bluetooth address
 */
#define SLI_BT_BD_ADDRESS_LEN      6

sl_status_t sli_bt_host_adaptation_write_custom_address(uint8_t address_type,
                                                        const uint8_t *address)
{
  if ((address_type != sl_bt_gap_public_address)
      && (address_type != sl_bt_gap_static_address)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t result = nvm3_open(SL_BT_NVM3, nvm3_defaultInit);
  if (result != SL_STATUS_OK) {
    return result;
  }

  // 00:00:00:00:00:00 or ff:ff:ff:ff:ff:ff is used to reset the custom address.
  // Delete the NVM3 keys and the default address will be used in the next reboot.
  uint8_t all_bit_1_address[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
  uint8_t all_bit_0_address[SLI_BT_BD_ADDRESS_LEN] = { 0x00 };
  if ((memcmp(address, all_bit_1_address, SLI_BT_BD_ADDRESS_LEN) == 0)
      || (memcmp(address, all_bit_0_address, SLI_BT_BD_ADDRESS_LEN) == 0)) {
    result = nvm3_deleteObject(SL_BT_NVM3, SLI_BT_NVM3_LOCAL_BD_ADDR_TYPE);
    if ((result == SL_STATUS_OK) || (result == SL_STATUS_NOT_FOUND)) {
      result = nvm3_deleteObject(SL_BT_NVM3, SLI_BT_NVM3_LOCAL_BD_ADDR);
    }
    if (result == SL_STATUS_NOT_FOUND) {
      result = SL_STATUS_OK;
    }
    return result;
  }

  // A static device address needs set two MSB bits to 1
  if (address_type == sl_bt_gap_static_address && (address[5] >> 6) != 3) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (address_type == sl_bt_gap_public_address) {
    result = nvm3_deleteObject(SL_BT_NVM3, SLI_BT_NVM3_LOCAL_BD_ADDR_TYPE);
    if (result == SL_STATUS_NOT_FOUND) {
      result = SL_STATUS_OK;
    }
  } else {
    result = nvm3_writeData(SL_BT_NVM3,
                            SLI_BT_NVM3_LOCAL_BD_ADDR_TYPE,
                            (void*) &address_type,
                            sizeof(address_type));
  }

  if (result == SL_STATUS_OK) {
    result = nvm3_writeData(SL_BT_NVM3,
                            SLI_BT_NVM3_LOCAL_BD_ADDR,
                            (void*) address,
                            SLI_BT_BD_ADDRESS_LEN);
  }

  return result;
}
#else
sl_status_t sli_bt_host_adaptation_write_custom_address(uint8_t address_type,
                                                        const uint8_t *address)
{
  (void) address_type;
  (void) address;
  // The user application does not use NVM3
  return SL_STATUS_NOT_AVAILABLE;
}
#endif // SL_CATALOG_NVM3_PRESENT

// Load custom Bluetooth identity address from NVM3
sl_status_t sli_bt_host_adaptation_read_custom_address(uint8_t *address_type,
                                                       uint8_t *address)
{
#if (SL_BT_CONFIG_SET_CUSTOM_ADDRESS_FROM_NVM3 == 1) && defined(SL_CATALOG_NVM3_PRESENT)
  if (!address_type || !address) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sl_status_t result = nvm3_open(SL_BT_NVM3, nvm3_defaultInit);

  if (result == SL_STATUS_OK) {
    // Read the address in NVM3
    result = nvm3_readData(SL_BT_NVM3, SLI_BT_NVM3_LOCAL_BD_ADDR, address, SLI_BT_BD_ADDRESS_LEN);
  }

  if (result == SL_STATUS_OK) {
    // Read the address type in NVM3. Treat the address as public device address if error
    sl_status_t read_type_result = nvm3_readData(SL_BT_NVM3, SLI_BT_NVM3_LOCAL_BD_ADDR_TYPE, address_type, sizeof(*address_type));
    if (read_type_result != SL_STATUS_OK) {
      *address_type = 0;
    }
  }

  return result;
#else
  // The user application does not use custom address, or NVM3 does not present
  (void) address_type;
  (void) address;
  return SL_STATUS_NOT_FOUND;
#endif // (SL_BT_CONFIG_SET_CUSTOM_ADDRESS_FROM_NVM3 == 1) && defined(SL_CATALOG_NVM3_PRESENT)
}

#if (SL_BT_CONFIG_SET_CTUNE_FROM_NVM3 == 1) && defined(SL_CATALOG_NVM3_PRESENT)
#include "em_cmu.h"

/**
 * @brief NVM3 key for custom CTUNE in Bluetooth NVM3 space. Value 2 bytes
 */
#define SLI_BT_NVM3_CTUNE 0x40032

// Set HFXO CTUNE using the value stored in the Bluetooth space of NVM3.
void sli_bt_host_adaptation_read_and_set_ctune()
{
  uint16_t ctune;
  sl_status_t err = nvm3_open(SL_BT_NVM3, nvm3_defaultInit);
  if (err == SL_STATUS_OK) {
    err = nvm3_readData(SL_BT_NVM3, SLI_BT_NVM3_CTUNE, &ctune, sizeof(ctune));
  }
  if (err == SL_STATUS_OK) {
    // Setting CTUNE could fail and we do it with best effort. Please find CTUNE and
    // its calibration related details in the platform.
    err = CMU_HFXOCTuneSet(ctune);
    (void) err;
  }
}
#else
void sli_bt_host_adaptation_read_and_set_ctune()
{
  // Feature is disabled. Do nothing.
}
#endif // #if (SL_BT_CONFIG_SET_CTUNE_FROM_NVM3 == 1) && defined(SL_CATALOG_NVM3_PRESENT)
