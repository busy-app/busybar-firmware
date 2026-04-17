/***************************************************************************//**
 * @file
 * @brief Silicon Labs Secure Engine Manager API.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_se_manager_util.h"
#include "sl_se_manager_types.h"
#include <stdint.h>

#if defined(SLI_MAILBOX_COMMAND_SUPPORTED) || defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)

#include "sli_se_manager_internal.h"
#include "sli_se_manager_mailbox.h"
#include "sl_assert.h"

#if defined(SLI_SE_MAJOR_VERSION_ONE)
  #include "em_system.h"
#endif

#include "sl_core.h"

/// @addtogroup sl_se_manager
/// @{

// -----------------------------------------------------------------------------
// Defines

// OTP initialization structure defines.
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ENABLE (1 << 16)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_VERIFY_CERTIFICATE (1 << 17)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ANTI_ROLLBACK (1 << 18)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_NARROW (1 << 19)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_FULL (1 << 20)

// -----------------------------------------------------------------------------
// Local Functions

/***************************************************************************//**
 * @brief
 *   Decode debug status word (as received from the SE).
 *
 * @return N/A
 ******************************************************************************/
#if defined(SLI_MAILBOX_COMMAND_SUPPORTED)
static void decode_debug_status(sl_se_debug_status_t *debug_status,
                                uint32_t status_word)
{
  debug_status->debug_port_lock_applied = status_word & (1 << 0);
  debug_status->device_erase_enabled = status_word & (1 << 1);
  debug_status->secure_debug_enabled = status_word & (1 << 2);
  debug_status->debug_port_lock_state = status_word & (1 << 5);
  debug_status->options_state.non_secure_invasive_debug =
    (status_word & (1 << 6)) == 0;
  debug_status->options_state.non_secure_non_invasive_debug =
    (status_word & (1 << 7)) == 0;
  debug_status->options_state.secure_invasive_debug =
    (status_word & (1 << 8)) == 0;
  debug_status->options_state.secure_non_invasive_debug =
    (status_word & (1 << 9)) == 0;
  debug_status->options_config.non_secure_invasive_debug =
    (status_word & (1 << 10)) == 0;
  debug_status->options_config.non_secure_non_invasive_debug =
    (status_word & (1 << 11)) == 0;
  debug_status->options_config.secure_invasive_debug =
    (status_word & (1 << 12)) == 0;
  debug_status->options_config.secure_non_invasive_debug =
    (status_word & (1 << 13)) == 0;
}
#elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
static void decode_debug_status(sl_se_debug_status_t *debug_status,
                                uint32_t status_word)
{
  debug_status->debug_port_lock_applied = status_word & (1 << 10);
  debug_status->device_erase_enabled = status_word & (1 << 11);
  debug_status->secure_debug_enabled = status_word & (1 << 12);
  debug_status->debug_port_lock_state = status_word & (1 << 15);
}
#endif // defined(SLI_MAILBOX_COMMAND_SUPPORTED)

// -----------------------------------------------------------------------------
// Global Functions

/***************************************************************************//**
 * Validate SE firmware image.
 ******************************************************************************/
sl_status_t sl_se_check_se_image(sl_se_command_context_t *cmd_ctx,
                                 void *image_addr)
{
  if (cmd_ctx == NULL || image_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CHECK_SE_IMAGE);

  sli_se_mailbox_command_add_parameter(se_cmd, (uint32_t)image_addr);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Apply SE firmware image.
 ******************************************************************************/
sl_status_t sl_se_apply_se_image(sl_se_command_context_t *cmd_ctx,
                                 void *image_addr)
{
  if (cmd_ctx == NULL || image_addr == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_APPLY_SE_IMAGE);

  sli_se_mailbox_command_add_parameter(se_cmd, (uint32_t)image_addr);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Get upgrade status of SE firmware image.
 ******************************************************************************/
sl_status_t sl_se_get_upgrade_status_se_image(sl_se_command_context_t *cmd_ctx,
                                              uint32_t *status,
                                              uint32_t *prev_version)
{
  if (cmd_ctx == NULL || status == NULL || prev_version == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_STATUS_SE_IMAGE);

  volatile uint32_t out_buf[2];
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(out_buf,
                                                               sizeof(out_buf));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  sl_status_t ret = sli_se_execute_and_wait(cmd_ctx);

  if (ret == SL_STATUS_OK) {
    *status = out_buf[0];
    *prev_version = out_buf[1];
  }

  return ret;
}

/***************************************************************************//**
 * Validate Host firmware image.
 ******************************************************************************/
sl_status_t sl_se_check_host_image(sl_se_command_context_t *cmd_ctx,
                                   void *image_addr,
                                   uint32_t size)
{
  if (cmd_ctx == NULL || image_addr == NULL || size == 0UL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_CHECK_HOST_IMAGE);

  sli_se_mailbox_command_add_parameter(se_cmd, (uint32_t)image_addr);
  sli_se_mailbox_command_add_parameter(se_cmd, size);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Apply Host firmware image.
 ******************************************************************************/
sl_status_t sl_se_apply_host_image(sl_se_command_context_t *cmd_ctx,
                                   void *image_addr,
                                   uint32_t size)
{
  if (cmd_ctx == NULL || image_addr == NULL || size == 0UL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_APPLY_HOST_IMAGE);

  sli_se_mailbox_command_add_parameter(se_cmd, (uint32_t)image_addr);
  sli_se_mailbox_command_add_parameter(se_cmd, size);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Get upgrade status of Host firmware image.
 ******************************************************************************/
sl_status_t
sl_se_get_upgrade_status_host_image(sl_se_command_context_t *cmd_ctx,
                                    uint32_t *status,
                                    uint32_t *prev_version)
{
  if (cmd_ctx == NULL || status == NULL || prev_version == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_STATUS_HOST_IMAGE);

  volatile uint32_t out_buf[2];
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(out_buf,
                                                               sizeof(out_buf));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  sl_status_t ret = sli_se_execute_and_wait(cmd_ctx);

  if (ret == SL_STATUS_OK) {
    *status = out_buf[0];
    *prev_version = out_buf[1];
  }

  return ret;
}

/***************************************************************************//**
 * Initialize key to be stored in the SE OTP flash.
 ******************************************************************************/
sl_status_t sl_se_init_otp_key(sl_se_command_context_t *cmd_ctx,
                               sl_se_device_key_type_t key_type,
                               void *key,
                               uint32_t num_bytes)
{
  if (cmd_ctx == NULL || key == NULL || num_bytes == 0UL || (size_t)key & 3U) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  #if defined(SLI_MAILBOX_COMMAND_SUPPORTED)
  if (key_type == SL_SE_KEY_TYPE_IMMUTABLE_AES_128) {
    if (num_bytes != 16UL) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  } else {
    if (num_bytes != 64UL) {
      return SL_STATUS_INVALID_PARAMETER;
    }
  }
  #elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
  if (num_bytes != 64UL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  #endif

  uint32_t command_word;
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  uint32_t se_key_type;
  switch (key_type) {
    case SL_SE_KEY_TYPE_IMMUTABLE_BOOT:
      se_key_type = SLI_SE_KEY_TYPE_BOOT;
      break;

    case SL_SE_KEY_TYPE_IMMUTABLE_AUTH:
      se_key_type = SLI_SE_KEY_TYPE_AUTH;
      break;

    #if defined(SLI_MAILBOX_COMMAND_SUPPORTED)
    case SL_SE_KEY_TYPE_IMMUTABLE_AES_128:
      se_key_type = SLI_SE_IMMUTABLE_KEY_TYPE_AES_128;
      break;
    #endif // SLI_MAILBOX_COMMAND_SUPPORTED

    default:
      return SL_STATUS_INVALID_PARAMETER;
      break;
  }

  // Find parity word
  volatile uint32_t parity = 0;
  for (size_t i = 0; i < num_bytes / 4; i++) {
    parity = parity ^ ((uint32_t *)key)[i];
  }

  // SE command structures
  #if defined(SLI_MAILBOX_COMMAND_SUPPORTED)
  command_word = key_type == SL_SE_KEY_TYPE_IMMUTABLE_AES_128
                 ? SLI_SE_COMMAND_INIT_AES_128_KEY : SLI_SE_COMMAND_INIT_PUBKEY;
  #elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
  command_word = SLI_SE_COMMAND_INIT_PUBKEY;
  #endif

  sli_se_command_init(cmd_ctx, (command_word | se_key_type));

  sli_se_datatransfer_t parity_data = SLI_SE_DATATRANSFER_DEFAULT(&parity, 4);
  sli_se_mailbox_command_add_input(se_cmd, &parity_data);

  sli_se_datatransfer_t key_data = SLI_SE_DATATRANSFER_DEFAULT(key, num_bytes);
  sli_se_mailbox_command_add_input(se_cmd, &key_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Read a public key stored in the SE.
 ******************************************************************************/
sl_status_t sl_se_read_pubkey(sl_se_command_context_t *cmd_ctx,
                              sl_se_device_key_type_t key_type,
                              void *key,
                              uint32_t num_bytes)
{
  if (cmd_ctx == NULL || key == NULL || num_bytes != 64UL || (size_t)key & 3U) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  uint32_t se_key_type;
  uint32_t command_word = SLI_SE_COMMAND_READ_PUBKEY;
  switch (key_type) {
    case SL_SE_KEY_TYPE_IMMUTABLE_BOOT:
      se_key_type = SLI_SE_KEY_TYPE_BOOT;
      break;

    case SL_SE_KEY_TYPE_IMMUTABLE_AUTH:
      se_key_type = SLI_SE_KEY_TYPE_AUTH;
      break;
    #if defined(SLI_MAILBOX_COMMAND_SUPPORTED) && (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
    case SL_SE_KEY_TYPE_IMMUTABLE_SE_ATTESTATION:
      command_word = command_word & ~0x1;
    // Intentional fallthrough
    case SL_SE_KEY_TYPE_IMMUTABLE_ATTESTATION:
      se_key_type = SLI_SE_KEY_TYPE_ATTEST;
      break;
    #endif // _SILICON_LABS_SECURITY_FEATURE_VAULT

    default:
      return SL_STATUS_INVALID_PARAMETER;
      break;
  }

  // SE command structures
  sli_se_command_init(cmd_ctx, command_word | se_key_type);

  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(key, num_bytes);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Read the SE firmware version.
 ******************************************************************************/
sl_status_t sl_se_get_se_version(sl_se_command_context_t *cmd_ctx,
                                 uint32_t *version)
{
  if (cmd_ctx == NULL || version == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  #if defined(SLI_MAILBOX_COMMAND_SUPPORTED)

  #if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)

  // Get SE Version via SE Mailbox command
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_STATUS_SE_VERSION);
  volatile sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(version, sizeof(uint32_t));

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);

  #else

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // Read state of CMU_CLKEN0_SYSCFG
  bool syscfg_clock_was_enabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;

  // Read SE FW version from SYSCFG
  *version = (uint32_t)(SYSCFG->ROOTSESWVERSION);

  if (!syscfg_clock_was_enabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  CORE_EXIT_CRITICAL();

  #if defined(_SILICON_LABS_32B_SERIES_3)
  // Omit compatibility information
  *version = ((*version) & 0x00FFFFFF);
  #endif

  return SL_STATUS_OK;
  #endif

  #elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)

  sl_status_t status = SL_STATUS_OK;
  sli_se_mailbox_response_t command_response;

  // Try to acquire SE lock.
  // Need to protect VSE mailbox from being written by e.g. SE_ackCommand()
  status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Read SE version from VSE mailbox.
  command_response = sli_vse_mailbox_get_version(version);

  // Release SE lock
  status = sli_se_lock_release();

  // Return sl_status_t code.
  if (command_response == SLI_SE_RESPONSE_OK) {
    return status;
  } else {
    // Convert from sli_se_mailbox_response_t to sl_status_t code and return.
    return sli_se_to_sl_status(command_response);
  }

  #endif
}
/***************************************************************************//**
 * Enables the debug lock for the part.
 ******************************************************************************/
sl_status_t sl_se_apply_debug_lock(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DBG_LOCK_APPLY);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Returns the current debug lock configuration.
 ******************************************************************************/
sl_status_t sl_se_get_debug_lock_status(sl_se_command_context_t *cmd_ctx,
                                        sl_se_debug_status_t *debug_status)
{
  if (cmd_ctx == NULL || debug_status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  #if defined(SLI_MAILBOX_COMMAND_SUPPORTED)
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  volatile uint32_t status_word = 0;
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(&status_word, 4);

  // Initialize SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DBG_LOCK_STATUS);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  sl_status_t ret = sli_se_execute_and_wait(cmd_ctx);

  if (ret == SL_STATUS_OK) {
    decode_debug_status(debug_status, status_word);
  }

  return ret;
  #elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
  uint32_t vse_version = 0;
  uint32_t debug_lock_flags = 0;

  // Try to acquire SE lock
  sl_status_t status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Read SE version from VSE mailbox.
  sli_se_mailbox_response_t vse_mbx_status = sli_vse_mailbox_get_version(&vse_version);

  // Reading debug lock status is not supported on VSE with versions <= 1.2.2.
  if ((vse_version <= 0x1010202UL) || (vse_mbx_status != SLI_SE_RESPONSE_OK)) {
    sli_se_lock_release();
    return SL_STATUS_COMMAND_IS_INVALID;
  }

  vse_mbx_status = sli_vse_mailbox_get_cfg_status(&debug_lock_flags);
  // Release SE lock
  status = sli_se_lock_release();

  if (vse_mbx_status != SLI_SE_RESPONSE_OK) {
    return sli_se_to_sl_status(vse_mbx_status);
  } else if (status != SL_STATUS_OK) {
    return status;
  }

  decode_debug_status(debug_status, debug_lock_flags);

  return SL_STATUS_OK;
  #endif
}

#if defined(SLI_MAILBOX_COMMAND_SUPPORTED)

/***************************************************************************//**
 * Initialize SE OTP configuration.
 ******************************************************************************/
sl_status_t sl_se_init_otp(sl_se_command_context_t *cmd_ctx,
                           sl_se_otp_init_t *otp_init)
{
  if (cmd_ctx == NULL || otp_init == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  uint32_t mcu_settings_flags = 0;

  sl_status_t status;

  if (otp_init->enable_secure_boot) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ENABLE;

    // Check for installed boot pubkey before OTP initialization
    uint8_t pubkey[64];
    status =
      sl_se_read_pubkey(cmd_ctx, SL_SE_KEY_TYPE_IMMUTABLE_BOOT, &pubkey, 64);
    if (status != SL_STATUS_OK) {
      return SL_STATUS_ABORT;
    }
  }
  if (otp_init->verify_secure_boot_certificate) {
    mcu_settings_flags |=
      SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_VERIFY_CERTIFICATE;
  }
  if (otp_init->enable_anti_rollback) {
    // Verify firmware compatibility before enabling anti-rollback
    #if defined(SLI_SE_MAJOR_VERSION_ONE)
    uint16_t part_number = SYSTEM_GetPartNumber();
    if (part_number == 1010 || part_number == 1020) {
      if (SYSTEM_GetProdRev() < 16) {
        sl_se_status_t se_status;
        status = sl_se_get_status(cmd_ctx, &se_status);
        if ((status != SL_STATUS_OK)
            || (se_status.se_fw_version < 0x00010201)) {
          // If the following error is returned, the SE firmware version
          // needs to be upgraded to v1.2.1 or higher before enabling
          // anti-rollback.
          EFM_ASSERT(false);
          return SL_STATUS_ABORT;
        }
      }
    }
    #endif

    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ANTI_ROLLBACK;
  }

  // Narrow and Full page lock are not supported in S3
  #if !defined(_SILICON_LABS_32B_SERIES_3)
  if (otp_init->secure_boot_page_lock_narrow) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_NARROW;
  }
  if (otp_init->secure_boot_page_lock_full) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_FULL;
  }
  #endif

  #if (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
  static struct {
    uint8_t levels[SL_SE_TAMPER_SIGNAL_NUM_SIGNALS / 2];
    uint8_t period;
    uint8_t threshold;
    uint8_t flags;
    uint8_t reset_threshold;
  } otp_tamper_settings;

  // Check for reserved sources
  if ((otp_init->tamper_levels[SL_SE_TAMPER_SIGNAL_RESERVED_1] != SL_SE_TAMPER_LEVEL_IGNORE)
      || (otp_init->tamper_levels[SL_SE_TAMPER_SIGNAL_RESERVED_2] != SL_SE_TAMPER_LEVEL_IGNORE)
      || (otp_init->tamper_levels[SL_SE_TAMPER_SIGNAL_RESERVED_3] != SL_SE_TAMPER_LEVEL_IGNORE)
      || (otp_init->tamper_levels[SL_SE_TAMPER_SIGNAL_RESERVED_4] != SL_SE_TAMPER_LEVEL_IGNORE)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Combine tamper levels, two per byte
  for (size_t i = 0; i < SL_SE_TAMPER_SIGNAL_NUM_SIGNALS; i += 2) {
    // Check for reserved levels
    for (size_t offset = 0; offset < 2; ++offset) {
      switch (otp_init->tamper_levels[i + offset]) {
        case SL_SE_TAMPER_LEVEL_IGNORE:
        case SL_SE_TAMPER_LEVEL_INTERRUPT:
        case SL_SE_TAMPER_LEVEL_FILTER:
        case SL_SE_TAMPER_LEVEL_RESET:
        case SL_SE_TAMPER_LEVEL_PERMANENTLY_ERASE_OTP:
          break;
        default:
          return SL_STATUS_INVALID_PARAMETER;
      }
    }

    otp_tamper_settings.levels[i / 2] = (otp_init->tamper_levels[i] & 0x7)
                                        | ((otp_init->tamper_levels[i + 1] & 0x7) << 4);
  }
  // Limit period and threshold input
  otp_tamper_settings.period = otp_init->tamper_filter_period & 0x1f;
  otp_tamper_settings.threshold = otp_init->tamper_filter_threshold & 0x7;

  #if !defined(SLI_SE_TAMPER_FLAG_KEEP_TAMPER_ALIVE_AVAILABLE)
  if ((otp_init->tamper_flags & SL_SE_TAMPER_FLAG_KEEP_TAMPER_ALIVE_DURING_SLEEP)
      == SL_SE_TAMPER_FLAG_KEEP_TAMPER_ALIVE_DURING_SLEEP) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  #endif
  otp_tamper_settings.flags = otp_init->tamper_flags;
  otp_tamper_settings.reset_threshold = otp_init->tamper_reset_threshold;
  #else
  static struct otp_tamper_settings {
    uint8_t reserved1[16];
    uint8_t reserved2[2];
    uint8_t reserved3[2];
  } otp_tamper_settings = {
    { 0x00 },
    { 0xFF, 0xFF },
    { 0x00 }
  };
  #endif

  // Find parity word
  volatile uint32_t parity = 0;
  parity = parity ^ mcu_settings_flags;
  for (size_t i = 0; i < 5; i++) {
    parity = parity ^ ((uint32_t*)(&otp_tamper_settings))[i];
  }

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_INIT_OTP);

  volatile uint32_t parameters[2] = {
    parity,
    sizeof(mcu_settings_flags)
    + sizeof(otp_tamper_settings)
  };
  sli_se_datatransfer_t parameters_data = SLI_SE_DATATRANSFER_DEFAULT(&parameters, 8);
  sli_se_mailbox_command_add_input(se_cmd, &parameters_data);

  sli_se_datatransfer_t mcu_settings_flags_data =
    SLI_SE_DATATRANSFER_DEFAULT((volatile void *)&mcu_settings_flags, sizeof(mcu_settings_flags));
  sli_se_mailbox_command_add_input(se_cmd, &mcu_settings_flags_data);

  sli_se_datatransfer_t tamper_settings_data =
    SLI_SE_DATATRANSFER_DEFAULT((volatile void *)&otp_tamper_settings, sizeof(otp_tamper_settings));
  sli_se_mailbox_command_add_input(se_cmd, &tamper_settings_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Read SE OTP configuration.
 ******************************************************************************/
sl_status_t sl_se_read_otp(sl_se_command_context_t *cmd_ctx,
                           sl_se_otp_init_t *otp_settings)
{
  if (cmd_ctx == NULL || otp_settings == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sl_status_t status;

  #if (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
  volatile struct {
    uint32_t mcu_settings_flags;
    uint8_t levels[SL_SE_TAMPER_SIGNAL_NUM_SIGNALS / 2];
    uint8_t period;
    uint8_t threshold;
    uint8_t flags;
    uint8_t reset_threshold;
  } otp_raw;
  #else
  volatile struct {
    uint32_t mcu_settings_flags;
    uint8_t reserved1[16];
    uint8_t reserved2[2];
    uint8_t reserved3[2];
  } otp_raw;
  #endif

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_READ_OTP);

  sli_se_datatransfer_t otp_raw_data =
    SLI_SE_DATATRANSFER_DEFAULT(&otp_raw, sizeof(otp_raw));
  sli_se_mailbox_command_add_output(se_cmd, &otp_raw_data);

  status = sli_se_execute_and_wait(cmd_ctx);

  if (status != SL_STATUS_OK) {
    return status;
  }

  otp_settings->enable_secure_boot =
    (otp_raw.mcu_settings_flags
     & SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ENABLE);
  otp_settings->verify_secure_boot_certificate =
    (otp_raw.mcu_settings_flags
     & SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_VERIFY_CERTIFICATE);
  otp_settings->enable_anti_rollback =
    (otp_raw.mcu_settings_flags
     & SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ANTI_ROLLBACK);

  // Narrow and Full page lock are not supported in S3
  #if !defined(_SILICON_LABS_32B_SERIES_3)
  otp_settings->secure_boot_page_lock_narrow =
    (otp_raw.mcu_settings_flags
     & SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_NARROW);
  otp_settings->secure_boot_page_lock_full =
    (otp_raw.mcu_settings_flags
     & SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_FULL);
  #endif

  #if (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
  // Split levels
  for (size_t i = 0; i < sizeof(otp_raw.levels); i++) {
    otp_settings->tamper_levels[2 * i] = (otp_raw.levels[i]) & 0x7;
    otp_settings->tamper_levels[2 * i + 1] = (otp_raw.levels[i] >> 4) & 0x7;
  }

  otp_settings->tamper_filter_period = otp_raw.period;
  otp_settings->tamper_filter_threshold = otp_raw.threshold;

  otp_settings->tamper_flags = otp_raw.flags;
  otp_settings->tamper_reset_threshold = otp_raw.reset_threshold;
  #endif

  return SL_STATUS_OK;
}

#elif defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)

sl_status_t sl_se_init_otp(sl_se_command_context_t *cmd_ctx,
                           sl_se_otp_init_t *otp_init)
{
  if (cmd_ctx == NULL || otp_init == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  volatile uint32_t mcu_settings_flags = 0;

  if (otp_init->enable_secure_boot) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ENABLE;
  }
  if (otp_init->verify_secure_boot_certificate) {
    mcu_settings_flags |=
      SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_VERIFY_CERTIFICATE;
  }
  if (otp_init->enable_anti_rollback) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ANTI_ROLLBACK;
  }
  if (otp_init->secure_boot_page_lock_narrow) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_NARROW;
  }
  if (otp_init->secure_boot_page_lock_full) {
    mcu_settings_flags |= SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_FULL;
  }

  // Find parity word
  uint32_t parity = 0;
  parity = parity ^ mcu_settings_flags;

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_INIT_OTP);

  volatile uint32_t parameters[2] = {
    parity,
    sizeof(mcu_settings_flags)
  };
  sli_se_datatransfer_t parameters_data = SLI_SE_DATATRANSFER_DEFAULT(&parameters, 8);
  sli_se_mailbox_command_add_input(se_cmd, &parameters_data);

  sli_se_datatransfer_t mcu_settings_flags_data =
    SLI_SE_DATATRANSFER_DEFAULT(&mcu_settings_flags, sizeof(mcu_settings_flags));
  sli_se_mailbox_command_add_input(se_cmd, &mcu_settings_flags_data);

  sli_se_mailbox_execute_command(se_cmd); // TODO this should use sli_se_execute_and_wait
  return SL_STATUS_FAIL; // Should never get to this point
}

/***************************************************************************//**
 * Read the OTP firmware version of the SE module.
 ******************************************************************************/
sl_status_t sl_se_get_otp_version(sl_se_command_context_t *cmd_ctx,
                                  uint32_t *version)
{
  if (cmd_ctx == NULL || version == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Try to acquire SE lock
  sl_status_t lock_status = sli_se_lock_acquire();
  if (lock_status != SL_STATUS_OK) {
    return lock_status;
  }

  sli_se_mailbox_response_t otp_status = sli_vse_mailbox_get_otp_version(version);

  // Release SE lock
  sli_se_lock_release();

  if (otp_status == SLI_SE_RESPONSE_OK) {
    return SL_STATUS_OK;
  }

  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_se_read_otp(sl_se_command_context_t *cmd_ctx,
                           sl_se_otp_init_t *otp_settings)
{
  if (cmd_ctx == NULL || otp_settings == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Try to acquire SE lock
  sl_status_t status = sli_se_lock_acquire();
  if (status != SL_STATUS_OK) {
    return status;
  }

  uint32_t mcu_settings_flags = 0;
  sli_se_mailbox_response_t vse_mbx_status = sli_vse_mailbox_get_cfg_status(&mcu_settings_flags);

  // Release SE lock
  status = sli_se_lock_release();

  if (vse_mbx_status != SLI_SE_RESPONSE_OK) {
    return sli_se_to_sl_status(vse_mbx_status);
  } else if (status != SL_STATUS_OK) {
    return status;
  }

  otp_settings->enable_secure_boot =
    (mcu_settings_flags
     & (SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ENABLE >> SL_SE_ROOT_CONFIG_MCU_SETTINGS_SHIFT));
  otp_settings->verify_secure_boot_certificate =
    (mcu_settings_flags
     & (SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_VERIFY_CERTIFICATE >> SL_SE_ROOT_CONFIG_MCU_SETTINGS_SHIFT));
  otp_settings->enable_anti_rollback =
    (mcu_settings_flags
     & (SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ANTI_ROLLBACK >> SL_SE_ROOT_CONFIG_MCU_SETTINGS_SHIFT));
  otp_settings->secure_boot_page_lock_narrow =
    (mcu_settings_flags
     & (SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_NARROW >> SL_SE_ROOT_CONFIG_MCU_SETTINGS_SHIFT));
  otp_settings->secure_boot_page_lock_full =
    (mcu_settings_flags
     & (SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_FULL >> SL_SE_ROOT_CONFIG_MCU_SETTINGS_SHIFT));

  return SL_STATUS_OK;
}
#endif

#if defined(SLI_MAILBOX_COMMAND_SUPPORTED)

#if defined(_SILICON_LABS_32B_SERIES_3)

/***************************************************************************//**
 * @brief
 *   Writes data to User Data section in MTP. The full MTP element is written every
 *   time, so length of write data (num_bytes) must always be equal to
 *   \ref SL_SE_USER_DATA_SIZE.
 ******************************************************************************/
sl_status_t sl_se_write_user_data(sl_se_command_context_t *cmd_ctx,
                                  const void *data,
                                  size_t num_bytes)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (data == NULL && num_bytes > 0UL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_bytes != SL_SE_USER_DATA_SIZE) {
    // We only support writing the full MTP region
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Setup SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(data, num_bytes);

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_USER_DATA);
  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  sli_se_mailbox_command_add_parameter(se_cmd, num_bytes);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Retrieves the data from the user data section in MTP.
 ******************************************************************************/
sl_status_t sl_se_get_user_data(sl_se_command_context_t *cmd_ctx,
                                void *output_data,
                                size_t num_bytes)
{
  if (cmd_ctx == NULL || output_data == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (num_bytes != SL_SE_USER_DATA_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Setup SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(output_data, SL_SE_USER_DATA_SIZE);

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_USER_DATA);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

#else

/***************************************************************************//**
 * Writes data to User Data section in MTP. Write data must be aligned to
 * word size and contain a number of bytes that is divisable by four.
 ******************************************************************************/
sl_status_t sl_se_write_user_data(sl_se_command_context_t *cmd_ctx,
                                  uint32_t offset,
                                  void *data,
                                  uint32_t num_bytes)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (data == NULL && num_bytes > 0UL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Setup SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(data, num_bytes);

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_WRITE_USER_DATA);
  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  sli_se_mailbox_command_add_parameter(se_cmd, offset);
  sli_se_mailbox_command_add_parameter(se_cmd, num_bytes);

  // Execute and wait
  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Erases User Data section in MTP.
 ******************************************************************************/
sl_status_t sl_se_erase_user_data(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ERASE_USER_DATA);

  sli_se_mailbox_command_add_parameter(se_cmd, SLI_SE_COMMAND_OPTION_ERASE_UD);

  // Execute and wait.
  return sli_se_execute_and_wait(cmd_ctx);
}
#endif //defined(_SILICON_LABS_32B_SERIES_3)

/***************************************************************************//**
 * Returns the current boot status, versions and system configuration.
 ******************************************************************************/
sl_status_t sl_se_get_status(sl_se_command_context_t *cmd_ctx,
                             sl_se_status_t *status)
{
  if (cmd_ctx == NULL || status == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

#if defined(_SILICON_LABS_32B_SERIES_3)
  volatile uint32_t output[10] = { 0 };
#else
  volatile uint32_t output[9] = { 0 };
#endif

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_STATUS);
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(output, sizeof(output));

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  sl_status_t ret = sli_se_execute_and_wait(cmd_ctx);

  if (ret == SL_STATUS_OK) {
    // Tamper status
    status->tamper_status = output[0];
    status->tamper_status_raw = output[2];

    // Update status object
    status->boot_status = output[4];

    #if defined(_SILICON_LABS_32B_SERIES_3)
    // Omit compatibility information
    status->se_fw_version = output[5] & 0x00FFFFFF;
    #else
    status->se_fw_version = output[5];
    #endif

    status->host_fw_version = output[6];

    // Decode debug status
    decode_debug_status(&status->debug_status, output[7]);

    // Decode secure boot mode
    status->secure_boot_enabled =
      ((output[8] & 0x1U) && ((output[8] & ~0x1U) == 0));

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG) && (_SILICON_LABS_32B_SERIES_2_CONFIG < 3)
    uint32_t active_mode_shift = 16;
#else
    uint32_t active_mode_shift = 8;
#endif
    status->active_mode_enabled =
      (status->boot_status >> active_mode_shift) & 0x1;
  }

#if defined(_SILICON_LABS_32B_SERIES_3)
  status->rom_revision = output[9] & 0xFF;
  status->otp_patch_sequence = (output[9] >> 8) & 0xFF;
#endif

  return ret;
}

/***************************************************************************//**
 * Read the serial number of the SE module.
 ******************************************************************************/
sl_status_t sl_se_get_serialnumber(sl_se_command_context_t *cmd_ctx,
                                   void *serial)
{
  if (cmd_ctx == NULL || serial == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_READ_SERIAL);
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(serial, 16);

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Read the OTP firmware version of the SE module.
 ******************************************************************************/
sl_status_t sl_se_get_otp_version(sl_se_command_context_t *cmd_ctx,
                                  uint32_t *version)
{
  if (cmd_ctx == NULL || version == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  #if defined(_SILICON_LABS_32B_SERIES_3)

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // Read state of CMU_CLKEN0_SYSCFG
  bool syscfg_clock_was_enabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;

  // Read SE FW version from SYSCFG
  *version = (uint32_t)(((SYSCFG->ROOTSESWVERSION) & 0xFF000000) >> 24);
  *version -= (uint32_t)((SYSCFG->ROMREVHW) & 0x000000FF);

  if (!syscfg_clock_was_enabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }
  CORE_EXIT_CRITICAL();

  return SL_STATUS_OK;

  #else
  // SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_STATUS_OTP_VERSION);
  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(version, sizeof(uint32_t));

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
  #endif
}

#if defined(SLI_SE_COMMAND_STATUS_READ_RSTCAUSE_AVAILABLE)
/***************************************************************************//**
 * Read the EMU->RSTCAUSE after a tamper reset. This function should be called
 * if EMU->RSTCAUSE has been cleared upon boot.
 ******************************************************************************/
sl_status_t sl_se_get_reset_cause(sl_se_command_context_t *cmd_ctx,
                                  uint32_t *reset_cause)
{
  if (cmd_ctx == NULL || reset_cause == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_STATUS_READ_RSTCAUSE);
  sli_se_datatransfer_t out_data =
    SLI_SE_DATATRANSFER_DEFAULT(reset_cause, sizeof(uint32_t));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);
  return sli_se_execute_and_wait(cmd_ctx);
}
#endif // SLI_SE_COMMAND_STATUS_READ_RSTCAUSE_AVAILABLE

#if defined(SLI_SE_COMMAND_READ_TAMPER_RESET_CAUSE_AVAILABLE)
/***************************************************************************//**
 * Read the cached value of the EMU->TAMPERRSTCAUSE register after a tamper
 * reset. This function should be called if EMU->TAMPERRSTCAUSE has been cleared
 * upon boot.
 ******************************************************************************/
sl_status_t sl_se_get_tamper_reset_cause(sl_se_command_context_t *cmd_ctx,
                                         bool *was_tamper_reset,
                                         uint32_t *reset_cause)
{
  if (cmd_ctx == NULL || reset_cause == NULL || was_tamper_reset == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  // Use a local cache to convert from bitfield to integer
  uint32_t tamper_cause_ret = 0;

  // SE command structures
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_READ_TAMPER_RESET_CAUSE);
  sli_se_datatransfer_t out_data =
    SLI_SE_DATATRANSFER_DEFAULT(&tamper_cause_ret, sizeof(uint32_t));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);
  sl_status_t status = sli_se_execute_and_wait(cmd_ctx);
  if (status != SL_STATUS_OK) {
    return status;
  }

  // Update indication if the reset was because of a tamper event or not.
  *was_tamper_reset = tamper_cause_ret > 0 ? true : false;

  // If there is a tamper cause the returned value(tamper_cause_ret) has a
  // single bit set at the position of the tamper cause.
  // Find the position of the set bit and return it.
  uint32_t set_bit_position = 0;
  while (tamper_cause_ret > 1) {
    tamper_cause_ret >>= 1;
    set_bit_position++;
  }

  *reset_cause = set_bit_position;
  return status;
}
#endif // SLI_SE_COMMAND_READ_TAMPER_RESET_CAUSE_AVAILABLE

#if defined(_SILICON_LABS_32B_SERIES_3)
/***************************************************************************//**
 * Reads out traceable event flags from the OTP
 ******************************************************************************/
sl_status_t sl_se_get_lifecycle_event_flags(sl_se_command_context_t *cmd_ctx, uint64_t *event_flags)
{
  sl_status_t status;
  if (cmd_ctx == NULL || event_flags == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  uint32_t se_version = 0;
  status = sl_se_get_se_version(cmd_ctx, &se_version);
  if (status != SL_STATUS_OK) {
    return status;
  }
  if (se_version < 0x030002UL) {
    // SE FW version lower than 3.0.2 does not support reading trace flags
    return SL_STATUS_NOT_SUPPORTED;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_READ_TRACE_FLAGS);

  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(event_flags, 8);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);
  return sli_se_execute_and_wait(cmd_ctx);
}
#endif
/***************************************************************************//**
 * Enables the secure debug functionality.
 ******************************************************************************/
sl_status_t sl_se_enable_secure_debug(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DBG_LOCK_ENABLE_SECURE);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Disables the secure debug functionality.
 ******************************************************************************/
sl_status_t sl_se_disable_secure_debug(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DBG_LOCK_DISABLE_SECURE);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Set options on the debug interface.
 ******************************************************************************/
sl_status_t sl_se_set_debug_options(sl_se_command_context_t *cmd_ctx,
                                    const sl_se_debug_options_t *debug_options)
{
  if (cmd_ctx == NULL || debug_options == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  uint32_t restriction_bits = 0x0;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DBG_SET_RESTRICTIONS);

  /// Encode restricted debug options parameter.
  restriction_bits |= debug_options->non_secure_invasive_debug ? 0 : 1UL << 0;
  restriction_bits |= debug_options->non_secure_non_invasive_debug ? 0 : 1UL << 1;
  restriction_bits |= debug_options->secure_invasive_debug ? 0 : 1UL << 2;
  restriction_bits |= debug_options->secure_non_invasive_debug ? 0 : 1UL << 3;

  sli_se_mailbox_command_add_parameter(se_cmd, restriction_bits);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Performs a device mass erase and debug unlock.
 ******************************************************************************/
sl_status_t sl_se_erase_device(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DEVICE_ERASE);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Disabled device erase functionality.
 ******************************************************************************/
sl_status_t sl_se_disable_device_erase(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DEVICE_ERASE_DISABLE);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Request challenge from SE which can be used to open debug access.
 ******************************************************************************/
sl_status_t sl_se_get_challenge(sl_se_command_context_t *cmd_ctx,
                                sl_se_challenge_t challenge)
{
  if (cmd_ctx == NULL || challenge == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t out_data =
    SLI_SE_DATATRANSFER_DEFAULT(challenge, sizeof(sl_se_challenge_t));

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_CHALLENGE);

  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Invalidate current challenge and make a new challenge.
 ******************************************************************************/
sl_status_t sl_se_roll_challenge(sl_se_command_context_t *cmd_ctx)
{
  sl_se_challenge_t new_challenge;
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_datatransfer_t out_data =
    SLI_SE_DATATRANSFER_DEFAULT(new_challenge, sizeof(sl_se_challenge_t));

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ROLL_CHALLENGE);
  sli_se_mailbox_command_add_output(&cmd_ctx->command, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Unlock debug access using certificate signed with challenge.
 ******************************************************************************/
sl_status_t sl_se_open_debug(sl_se_command_context_t *cmd_ctx,
                             void *cert, uint32_t len,
                             const sl_se_debug_options_t *debug_options)
{
  if (cmd_ctx == NULL || cert == NULL || debug_options == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(cert, len);
  uint32_t unlock_bits = 1UL << 1;  // Always request to unlock debug access port

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_OPEN_DEBUG);

  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  /** Encode parameter that holds debug options to unlock. */
  unlock_bits |= debug_options->non_secure_invasive_debug     ? 1UL << 2 : 0;
  unlock_bits |= debug_options->non_secure_non_invasive_debug ? 1UL << 3 : 0;
  unlock_bits |= debug_options->secure_invasive_debug         ? 1UL << 4 : 0;
  unlock_bits |= debug_options->secure_non_invasive_debug     ? 1UL << 5 : 0;

  sli_se_mailbox_command_add_parameter(se_cmd, unlock_bits);

  return sli_se_execute_and_wait(cmd_ctx);
}

#if (_SILICON_LABS_SECURITY_FEATURE == _SILICON_LABS_SECURITY_FEATURE_VAULT)
/***************************************************************************//**
 * Temporarily disable tamper configuration using certificate signed with
 * challenge.
 ******************************************************************************/
sl_status_t sl_se_disable_tamper(sl_se_command_context_t *cmd_ctx,
                                 void *cert,
                                 uint32_t len,
                                 sl_se_tamper_signals_t tamper_signals)
{
  if (cmd_ctx == NULL || cert == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  sli_se_datatransfer_t in_data = SLI_SE_DATATRANSFER_DEFAULT(cert, len);

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_DISABLE_TAMPER);

  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  sli_se_mailbox_command_add_parameter(se_cmd, tamper_signals);

  return sli_se_execute_and_wait(cmd_ctx);
}

#endif

/***************************************************************************//**
 * Read size of stored certificates in SE.
 ******************************************************************************/
sl_status_t sl_se_read_cert_size(sl_se_command_context_t *cmd_ctx,
                                 sl_se_cert_size_type_t *cert_size)
{
  if (cmd_ctx == NULL || cert_size == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_READ_USER_CERT_SIZE);

  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(cert_size, 12UL);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Read stored certificates in SE.
 ******************************************************************************/
sl_status_t sl_se_read_cert(sl_se_command_context_t *cmd_ctx,
                            sl_se_cert_type_t cert_type,
                            void *cert,
                            uint32_t num_bytes)
{
  if (cmd_ctx == NULL || cert == NULL || num_bytes == 0UL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;
  uint32_t se_cert_type;

  switch (cert_type) {
    case SL_SE_CERT_BATCH:
      se_cert_type = SLI_SE_COMMAND_CERT_BATCH;
      break;

    case SL_SE_CERT_DEVICE_SE:
      se_cert_type = SLI_SE_COMMAND_CERT_SE;
      break;

    case SL_SE_CERT_DEVICE_HOST:
      se_cert_type = SLI_SE_COMMAND_CERT_HOST;
      break;

    default:
      return SL_STATUS_INVALID_PARAMETER;
      break;
  }

  // SE command structures
  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_READ_USER_CERT | se_cert_type);

#if SLI_MINIMUM_REQUIRED_NUMBER_PARAMS == 1
  // One parameter is required, but has no effect
  sli_se_mailbox_command_add_parameter(se_cmd, 0);
#endif

  sli_se_datatransfer_t out_data = SLI_SE_DATATRANSFER_DEFAULT(cert, num_bytes);
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Enter active mode.
 ******************************************************************************/
sl_status_t sl_se_enter_active_mode(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_ENTER_ACTIVE_MODE);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Exit active mode.
 ******************************************************************************/
sl_status_t sl_se_exit_active_mode(sl_se_command_context_t *cmd_ctx)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_EXIT_ACTIVE_MODE);

  return sli_se_execute_and_wait(cmd_ctx);
}

#endif // defined(SLI_MAILBOX_COMMAND_SUPPORTED)

#if defined(_SILICON_LABS_32B_SERIES_3)
/***************************************************************************//**
 * @brief
 *   Read the OTP rollback counter.
 ******************************************************************************/
sl_status_t sl_se_get_rollback_counter(sl_se_command_context_t *cmd_ctx,
                                       uint32_t *rollback_counter)
{
  if ((cmd_ctx == NULL) || (rollback_counter == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_ROLLBACK_COUNTER);

  sli_se_datatransfer_t out_data
    = SLI_SE_DATATRANSFER_DEFAULT(rollback_counter, sizeof(uint32_t));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * @brief
 *   Increment the OTP rollback counter.
 ******************************************************************************/
sl_status_t sl_se_increment_rollback_counter(sl_se_command_context_t *cmd_ctx,
                                             uint32_t *rollback_counter)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, (SLI_SE_COMMAND_GET_ROLLBACK_COUNTER | 0x00000100));

  uint32_t output = 0;

  sli_se_datatransfer_t out_data
    = SLI_SE_DATATRANSFER_DEFAULT(&output, sizeof(uint32_t));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  sl_status_t status = sli_se_execute_and_wait(cmd_ctx);

  if (rollback_counter != NULL) {
    *rollback_counter = output;
  }
  return status;
}

/***************************************************************************//**
 * Reads back the stored upgrade file version.
 ******************************************************************************/
sl_status_t sl_se_get_upgrade_file_version(sl_se_command_context_t *cmd_ctx,
                                           uint32_t *version)
{
  if ((cmd_ctx == NULL) || (version == NULL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_GET_HOST_UPGRADE_FILE_VERSION);

  sli_se_datatransfer_t out_data
    = SLI_SE_DATATRANSFER_DEFAULT(version, sizeof(uint32_t));
  sli_se_mailbox_command_add_output(se_cmd, &out_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

/***************************************************************************//**
 * Records a new upgrade file version.
 ******************************************************************************/
sl_status_t sl_se_set_upgrade_file_version(sl_se_command_context_t *cmd_ctx,
                                           uint32_t version)
{
  if (cmd_ctx == NULL) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_se_mailbox_command_t *se_cmd = &cmd_ctx->command;

  sli_se_command_init(cmd_ctx, SLI_SE_COMMAND_SET_HOST_UPGRADE_FILE_VERSION);

  sli_se_datatransfer_t in_data
    = SLI_SE_DATATRANSFER_DEFAULT(&version, sizeof(uint32_t));
  sli_se_mailbox_command_add_input(se_cmd, &in_data);

  return sli_se_execute_and_wait(cmd_ctx);
}

#endif // defined(_SILICON_LABS_32B_SERIES_3)

/// @} (end addtogroup sl_se)

#endif // defined(SLI_MAILBOX_COMMAND_SUPPORTED) || defined(SLI_VSE_MAILBOX_COMMAND_SUPPORTED)
