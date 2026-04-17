/***************************************************************************/ /**
 * @file
 * @brief CPC Firmware Upgrade
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
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "sl_cpc_instance_handles.h"
#include "sl_cpc_weak_prototypes.h"
#include "sli_cpc_drv.h"
#include "sli_cpc_fwu.h"
#include "sli_cpc_instance.h"
#include "sli_cpc_reboot_sequence.h"
#include "sli_cpc_trace.h"

#if (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED >= 1)

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static bool do_firmware_upgrade_process = false;

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 * Gets whether or not a firmware upgrade is in progress
 *
 * @return bool : Firmware upgrade in process
 ******************************************************************************/
bool sli_cpc_is_fwu_in_progress(void)
{
  return do_firmware_upgrade_process;
}

/***************************************************************************//**
 * The firmware upgrade process action called by the main CPC process action
 ******************************************************************************/
void sli_cpc_fwu_process_action()
{
  // As long as the firmware upgrade has not been started, this process action is skipped
  if (!sli_cpc_is_fwu_in_progress()) {
    return;
  }

  if (!sli_cpc_is_bootloader_running()) {
    #if !(SL_CPC_PRIMARY_FIRMWARE_UPGRADE_RECOVERY_PINS_SUPPORT_ENABLED >= 1)
    // If we detected that the bootloader was not running, and the bootloader
    // recovery pins support is not enabled, then in order to switch the secondary
    // to the bootloader, we must pass by CPC to do that

    // The first thing the firmware upgrade process does is rebooting the secondary
    // in bootloader mode. Once done, skip over the reboot bootloader process action
    if (!sli_cpc_reboot_is_bootloader_sequence_done()) {
      sli_cpc_reboot_bootloader_sequence_process();
      return;
    }
    #endif
  }

  // The secondary has rebooted into bootloader

  // Initialize the FWU driver (executed only once)
  static bool fwu_driver_inited = false;

  if (fwu_driver_inited == false) {
    fwu_driver_inited = true;

    if (!sli_cpc_is_bootloader_running()) {
      g_instance.driver->ops.deinit(g_instance.driver);

      #if (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_RECOVERY_PINS_SUPPORT_ENABLED >= 1)
      // Use the bootloader pins only if the bootloader was not detected at startup
      sli_cpc_drv_fwu_enter_bootloader_via_recovery_pins();
      #endif
    }

    sli_cpc_drv_fwu_init();

    TRACE_FWU("FWU driver initialized");
  }

  // Execute the xmodem FWU state machine until it returns done
  sl_status_t fsm_status = sli_cpc_drv_fwu_step();

  if (fsm_status == SL_STATUS_OK || fsm_status == SL_STATUS_FAIL) {
    // Stop executing this process action
    do_firmware_upgrade_process = false;
  }
}

/***************************************************************************//**
 * Starts the firmware upgrade process of the secondary.
 *
 * @return sl_status :
 *   SL_STATUS_OK : The firmware update process has successfuly started. The CPC API will
 *                  now be blocked and the 'sl_cpc_get_fwu_chunk' callback implemented by the
 *                  user will now be continuously called until the firmware image has been
 *                  completely transfered.
 *
 *  If not SL_STATUS_OK, the return value will be the one returned by
 *  sl_cpc_is_firmware_upgrade_startable()
 ******************************************************************************/
sl_status_t sl_cpc_fwu_start(void)
{
  sl_status_t status;

  status = sl_cpc_is_fwu_startable();

  if (status != SL_STATUS_OK) {
    return status;
  }

  // activate the firmware upgrade process action
  do_firmware_upgrade_process = true;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Checks if a firmware upgrade can be started
 *
 * @return sl_status :
 *   SL_STATUS_OK : If a firmware upgrade can be started
 *   SL_STATUS_IN_PROGRESS : If a firmware upgrade is already in progress
 *   SL_STATUS_NOT_READY : If the firmware upgrade cannot yet be started. This
 *     would be because the reset sequence is not yet finished.
 *   SL_STATUS_NOT_AVAILABLE : If the user did not provide an implementation for
 *     "sl_cpc_get_fwu_chunk()"
 *   SL_STATUS_NOT_SUPPORTED : If after the reset sequence, the secondary returned
 *     that is does not have a suitable gecko bootloader.
 ******************************************************************************/
sl_status_t sl_cpc_is_fwu_startable(void)
{
  if (sli_cpc_is_fwu_in_progress()) {
    return SL_STATUS_IN_PROGRESS;
  }

  if (!sl_cpc_get_fwu_chunk) {
    // The user did not provide an implementation for sl_cpc_get_fwu_chunk
    return SL_STATUS_NOT_AVAILABLE;
  }

  if (!sli_cpc_instance_is_initialized(&g_instance)) {
    // Prevents a user from trying to do a firmware upgrade early during
    // the init process and before CPC init had the chance of probing
    // the bootloader to know if it is running or not
    return SL_STATUS_NOT_INITIALIZED;
  }

#if (SL_CPC_PRIMARY_FIRMWARE_UPGRADE_RECOVERY_PINS_SUPPORT_ENABLED >= 1)
  return SL_STATUS_OK;
#else
  if (!sli_cpc_is_bootloader_running()) {
    if (!sli_cpc_reboot_is_sequence_done(&g_instance.system_ep)) {
      return SL_STATUS_NOT_READY;
    }

    if (!sli_cpc_is_gecko_bootloader_present()) {
      return SL_STATUS_NOT_SUPPORTED;
    }
  }

  return SL_STATUS_OK;
#endif
}

/***************************************************************************//**
 * Probe secondary to detect if it booted in bootloader or CPC mode.
 ******************************************************************************/
sl_status_t sli_cpc_fwu_probe_bootloader(void)
{
  PRINT_INFO("Probing the bootloader");

  if (sli_cpc_is_bootloader_running()) {
    // CPC-1073 : Support asking the bootloader to trap to application
    // Ask the bootloader to jump to application, then only if it can't
    // print this message
    PRINT_INFO("The secondary is currently running the bootloader. "
               "CPC will not be initialized, only doing a firmware upgrade "
               "is possible");

    return SL_STATUS_NOT_READY;
  } else {
    PRINT_INFO("The secondary didn't respond to bootloader probing, assuming CPC is running");

    return SL_STATUS_OK;
  }
}
#else // SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED
/***************************************************************************//**
 * Stub implementation of probe bootloader, if the firmware upgrade is disabled.
 ******************************************************************************/
sl_status_t sli_cpc_fwu_probe_bootloader(void)
{
  PRINT_INFO("Firmware upgrade support disabled, "
             "skipping secondary bootloader probing");

  return SL_STATUS_OK;
}
#endif  // SL_CPC_PRIMARY_FIRMWARE_UPGRADE_SUPPORT_ENABLED
