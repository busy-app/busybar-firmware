/***************************************************************************//**
 * @file
 * @brief FIPS-180-2 compliant SHA-1 & SHA-256 implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: APACHE-2.0
 *
 * This software is subject to an open source license and is distributed by
 * Silicon Laboratories Inc. pursuant to the terms of the Apache License,
 * Version 2.0 available at https://www.apache.org/licenses/LICENSE-2.0.
 * Such terms and conditions may be further supplemented by the Silicon Labs
 * Master Software License Agreement (MSLA) available at www.silabs.com and its
 * sections applicable to open source software.
 *
 ******************************************************************************/
/*
 *  The SHA-256 Secure Hash Standard was published by NIST in 2002.
 *  http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
 */

#include <mbedtls/build_info.h>
#include "em_device.h"

#if defined(SEMAILBOX_PRESENT)
#include "sli_se_manager_mailbox.h"
#include "security/sha/btl_sha256.h"
#include "mbedtls/error.h"

int sha_x_process(SHA_Type_t algo,
                  uint8_t* state_in,
                  const unsigned char *blockdata,
                  uint8_t* state_out,
                  uint32_t num_blocks)
{
#if defined(_CMU_CLKEN1_SEMAILBOXHOST_MASK)
  CMU->CLKEN1_SET = CMU_CLKEN1_SEMAILBOXHOST;
#endif

  sli_se_mailbox_command_t command = SLI_SE_MAILBOX_COMMAND_DEFAULT(SLI_SE_COMMAND_HASHUPDATE);
  sli_se_datatransfer_t data_in = SLI_SE_DATATRANSFER_DEFAULT((void *)blockdata, 0);
  sli_se_datatransfer_t iv_in = SLI_SE_DATATRANSFER_DEFAULT(state_in, 0);
  sli_se_datatransfer_t iv_out = SLI_SE_DATATRANSFER_DEFAULT(state_out, 0);

  if (algo == SHA256) {
    command.command |= SLI_SE_COMMAND_OPTION_HASH_SHA256;
    // SHA256 block size is 64 bytes
    sli_se_mailbox_command_add_parameter(&command, 64 * num_blocks);
    data_in.length |= 64 * num_blocks;
    // SHA256 state size is 32 bytes
    iv_in.length |= 32;
    iv_out.length |= 32;
  } else {
    return MBEDTLS_ERR_PLATFORM_FEATURE_UNSUPPORTED;
  }

  sli_se_mailbox_command_add_input(&command, &iv_in);
  sli_se_mailbox_command_add_input(&command, &data_in);
  sli_se_mailbox_command_add_output(&command, &iv_out);

  sli_se_mailbox_execute_command(&command);
  sli_se_mailbox_response_t res = sli_se_mailbox_read_response();

  if (res == SLI_SE_RESPONSE_OK) {
    return 0;
  } else {
    return MBEDTLS_ERR_PLATFORM_HW_ACCEL_FAILED;
  }
}

#endif // #if defined(SEMAILBOX_PRESENT)
