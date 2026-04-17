/***************************************************************************//**
 * @file
 * @brief SE Mailbox API
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

#include "sli_se_manager_mailbox.h"

#if defined(SLI_SE_MAILBOX_HOST_SYSTEM) || defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)

#if !defined(SLI_SE_MAILBOX_HOST_SYSTEM)

#if defined(CRYPTOACC_PRESENT)
#include "sl_core.h"
#endif

#include "sl_assert.h"
#include "sl_code_classification.h"

#endif

/***************************************************************************//**
 * @addtogroup se
 * @{
 ******************************************************************************/

/*******************************************************************************
 ******************************   DEFINES    ***********************************
 ******************************************************************************/
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */

#if defined(CRYPTOACC_PRESENT)

/// Signal that OTP version is incorporated into the status field of the output
#define SE_VSE_REPLY_STATUS_OTP_VERSION_SET (1 << 21)
/// Mask defining the region of the status field that contains the OTP version
/// number.
#define SE_VSE_REPLY_STATUS_OTP_VERSION_MASK (0xFF000000UL)
/// Shift to insert a number into the otp version part of the status field
#define SE_VSE_REPLY_STATUS_OTP_VERSION_SHIFT (24)

/* Size of VSE Mailbox instance.
   There are two instances, input and output. */
#define ROOT_MAILBOX_SIZE  (512UL)

/* Base addresses of the VSE Input and Output Mailbox data structures.
   (Must be stored in a RAM area which is not used by the VSE)
   We use the upper 1KB of FRC RAM for the VSE mailboxes. */
#define ROOT_MAILBOX_OUTPUT_S_BASE (RDMEM_FRCRAM_S_MEM_END + 1 - ROOT_MAILBOX_SIZE)
#define ROOT_MAILBOX_INPUT_S_BASE  (ROOT_MAILBOX_OUTPUT_S_BASE - ROOT_MAILBOX_SIZE)

// SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S is defined in sl_trustzone_secure_config.h
#if ((defined(SL_TRUSTZONE_SECURE) && !defined(SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S)) \
  || (defined(SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S) && SL_TRUSTZONE_PERIPHERAL_AHBRADIO_S))

#define RDMEM_FRCRAM_MEM_BASE RDMEM_FRCRAM_S_MEM_BASE

#define ROOT_MAILBOX_OUTPUT_BASE SYSCFG->ROOTDATA1;
#define ROOT_MAILBOX_OUTPUT_BASE_EXPECTED ROOT_MAILBOX_OUTPUT_S_BASE
#else
#define RDMEM_FRCRAM_MEM_BASE RDMEM_FRCRAM_NS_MEM_BASE

// VSE will always output the secure address, if NS is desired, caculate the NS address.
#define ROOT_MAILBOX_OUTPUT_BASE (SYSCFG->ROOTDATA1 - RDMEM_FRCRAM_S_MEM_BASE + RDMEM_FRCRAM_NS_MEM_BASE);
#define ROOT_MAILBOX_OUTPUT_BASE_EXPECTED (RDMEM_FRCRAM_NS_MEM_END + 1 - ROOT_MAILBOX_SIZE)
#endif
#define ROOT_MAILBOX_INPUT_BASE  (ROOT_MAILBOX_OUTPUT_BASE_EXPECTED - ROOT_MAILBOX_SIZE)

/* Position of parameter number field in VSE Input Mailbox LENGTH field.*/
#define ROOT_MB_LENGTH_PARAM_NUM_SHIFT (24)

/* Done flag indicating that the VSE Mailbox handler has completed
   processing the mailbox command. */
#define ROOT_MB_DONE  (1 << 23)

/* VSE Configuration Status bits mask */
#define ROOT_MB_OUTPUT_STATUS_CONFIG_BITS_MASK  (0xFFFF)

#endif // #if defined(CRYPTOACC_PRESENT)
/** @endcond */

/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/
#if defined(CRYPTOACC_PRESENT)
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
// VSE Input Mailbox structure
typedef struct {
  volatile uint32_t magic;
  volatile uint32_t command;
  volatile uint32_t length;
  volatile uint32_t data[0];
} root_mailbox_input_t;

// VSE Output Mailbox structure
typedef struct {
  volatile uint32_t magic;
  volatile uint32_t version;
  volatile uint32_t status;
  volatile uint32_t command;
  volatile uint32_t length;
  volatile uint32_t data[0];
} root_mailbox_output_t;
/** @endcond */

#endif // #if defined(CRYPTOACC_PRESENT)

/*******************************************************************************
 **************************   STATIC FUNCTIONS   *******************************
 ******************************************************************************/

#if defined(SEMAILBOX_PRESENT)
/***************************************************************************//**
 * @brief
 *   Write to FIFO
 *
 * @param value
 *   Value to write to FIFO
 ******************************************************************************/
#if defined(_SEMAILBOX_FIFO_RESETVALUE)
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
__STATIC_INLINE void write_to_fifo(uint32_t value)
{
  SEMAILBOX_HOST->FIFO = value;
}
#else
__STATIC_INLINE void write_to_fifo(uint32_t value)
{
  SEMAILBOX_HOST->FIFO[0].DATA = value;
}
#endif

#elif defined(CRYPTOACC_PRESENT)

/***************************************************************************//**
 * @brief
 *   Return a pointer to the root mailbox output
 *
 * @return root_mailbox_output_t pointer
 ******************************************************************************/
static root_mailbox_output_t* get_root_mailbox_output(void)
{
  // Setup pointer to the VSE Output Mailbox data structure
  // (must be stored in a RAM area which is not used by the VSE)
  bool sys_cfg_clk_enabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_mailbox_output_t *mailbox_output = (root_mailbox_output_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sys_cfg_clk_enabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }
  return mailbox_output;
}

/***************************************************************************//**
 * @brief
 *   Check whether the running command has completed.
 *
 * @details
 *   This function polls the SE-to-host mailbox interrupt flag.
 *
 * @return True if a command has completed and the result is available
 ******************************************************************************/
static bool vse_is_command_completed(root_mailbox_output_t *root_mailbox_output)
{
  // First verify that the response is ok
  if (!sli_vse_mailbox_is_output_valid()) {
    return false;
  }

  // Check status MB_DONE flag of the mailbox
  return ((root_mailbox_output->status & ROOT_MB_DONE) == ROOT_MB_DONE);
}
#endif // SEMAILBOX_PRESENT

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Add input data to a mailbox command
 ******************************************************************************/
void sli_se_mailbox_command_add_input(sli_se_mailbox_command_t *command, volatile sli_se_datatransfer_t *data)
{
  if (command->data_in == NULL) {
    command->data_in = data;
  } else {
    volatile sli_se_datatransfer_t *next = command->data_in;
    while (next->next != (void*)SLI_SE_DATATRANSFER_STOP) {
      next = (volatile sli_se_datatransfer_t*)next->next;
    }
    next->next = data;
  }
}

/***************************************************************************//**
 * @brief
 *   Add output data to a mailbox command
 ******************************************************************************/
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_SE_MANAGER, SL_CODE_CLASS_TIME_CRITICAL)
void sli_se_mailbox_command_add_output(sli_se_mailbox_command_t *command, volatile sli_se_datatransfer_t *data)
{
  if (command->data_out == NULL) {
    command->data_out = data;
  } else {
    volatile sli_se_datatransfer_t *next = command->data_out;
    while (next->next != (void*)SLI_SE_DATATRANSFER_STOP) {
      next = (volatile sli_se_datatransfer_t*)next->next;
    }
    next->next = data;
  }
}

/***************************************************************************//**
 * @brief
 *   Add a parameter to a mailbox command
 ******************************************************************************/
void sli_se_mailbox_command_add_parameter(sli_se_mailbox_command_t *command, uint32_t parameter)
{
  if (command->num_parameters >= SLI_SE_COMMAND_MAX_PARAMETERS) {
    EFM_ASSERT(command->num_parameters < SLI_SE_COMMAND_MAX_PARAMETERS);
    return;
  }

  command->parameters[command->num_parameters] = parameter;
  command->num_parameters += 1;
}

#if !defined(SLI_SE_MAILBOX_HOST_SYSTEM)

/***************************************************************************//**
 * @brief
 *   Execute a command on the SE by writing the command to SEMAILBOX->FIFO
 ******************************************************************************/
void sli_se_mailbox_execute_command(sli_se_mailbox_command_t *command)
{
  // Don't overflow our struct
  if (command->num_parameters > SLI_SE_COMMAND_MAX_PARAMETERS) {
    EFM_ASSERT(command->num_parameters <= SLI_SE_COMMAND_MAX_PARAMETERS);
    return;
  }

#if defined(SEMAILBOX_PRESENT)

  // Wait for room available in the mailbox
  while (!(SEMAILBOX_HOST->TX_STATUS & SEMAILBOX_TX_STATUS_TXINT)) {
  }

  #if (_SILICON_LABS_32B_SERIES == 3)
  // Write header (including message size) to start transaction
  SEMAILBOX_HOST->TX_HEADER = sizeof(uint32_t) * (5 + command->num_parameters);
  // Write a 32-bit word to the FIFO ( potentially used as command handle )
  write_to_fifo(0);
  #else
  // Write header (including message size) to start transaction
  SEMAILBOX_HOST->TX_HEADER = sizeof(uint32_t) * (4 + command->num_parameters);
  #endif

  // Write command into FIFO
  write_to_fifo(command->command);

  // Write DMA descriptors into FIFO
  write_to_fifo((uint32_t)command->data_in);
  write_to_fifo((uint32_t)command->data_out);

  // Write applicable parameters into FIFO
  for (size_t i = 0; i < command->num_parameters; i++) {
    write_to_fifo(command->parameters[i]);
  }

#elif defined(CRYPTOACC_PRESENT)
// Prepare the VSE Mailbox within a critical section to prevent
// the process from getting interrupted. At this point, the only option
// we have is to go through a reset, so it is safe to enter the critical section.
  (void)CORE_EnterCritical();

  // Setup pointer to the VSE Mailbox Input data structure
  // (must be stored in a RAM area which is not used by the VSE)
  root_mailbox_input_t *mailbox_input = (root_mailbox_input_t*)ROOT_MAILBOX_INPUT_BASE;
  uint32_t *mailbox_data;
  unsigned int mailbox_dataLen, inDataLen, i;
  volatile sli_se_datatransfer_t *inDataDesc;
  uint32_t *inData;
  uint32_t checksum;
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;

  // Store the secure memory base addresses for VSE to be able to read from the address
  // Set base of Mailbox Input data structure in SYSCFG register in order
  // for VSE to find it.
  SYSCFG->ROOTDATA0 = ROOT_MAILBOX_INPUT_S_BASE;
  // Set base of Mailbox Output data structure in SYSCFG register in order
  // for VSE to know where to write output data.
  // Write command into FIFO
  SYSCFG->ROOTDATA1 = ROOT_MAILBOX_OUTPUT_S_BASE;

  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  mailbox_input->magic   = SLI_SE_RESPONSE_MAILBOX_VALID;
  mailbox_input->command = command->command;

  // Write applicable parameters into Mailbox DATA array
  mailbox_data = (uint32_t*) mailbox_input->data;
  for (mailbox_dataLen = 0; mailbox_dataLen < command->num_parameters; mailbox_dataLen++) {
    mailbox_data[mailbox_dataLen] = command->parameters[mailbox_dataLen];
  }

  // Write input data into Mailbox DATA array
  inDataLen = 0;
  for (inDataDesc = command->data_in; inDataDesc; inDataDesc = (sli_se_datatransfer_t*) inDataDesc->next) {
    inData = (uint32_t*) inDataDesc->data;
    for (i = 0; i < (inDataDesc->length & SLI_SE_DATATRANSFER_LENGTH_MASK) / sizeof(uint32_t); i++) {
      // Make sure we do not overflow the input mailbox.
      EFM_ASSERT(mailbox_dataLen < ROOT_MAILBOX_SIZE);
      mailbox_data[mailbox_dataLen++] = inData[i];
      inDataLen++;
    }
    if (inDataDesc->next == (void*)SLI_SE_DATATRANSFER_STOP) {
      break;
    }
  }

  // Write number of parameters and data words to 'length' field of mailbox.
  mailbox_input->length =
    inDataLen | (command->num_parameters << ROOT_MB_LENGTH_PARAM_NUM_SHIFT);

  // Calculate checksum using bitwise XOR over the all words in the mailbox
  // data structure, minus the CHECKSUM word (32bit = 4bytes ) at the end.
  checksum = mailbox_input->magic;
  checksum ^= mailbox_input->command;
  checksum ^= mailbox_input->length;
  for (i = 0; i < mailbox_dataLen; i++) {
    checksum ^= mailbox_data[i];
  }

  // Finally, write the calculated checksum to mailbox checksum field
  mailbox_data[mailbox_dataLen] = checksum;

  __NVIC_SystemReset();

#endif // #if defined(SEMAILBOX_PRESENT)
}
#endif // #if !defined(SLI_SE_MAILBOX_HOST_SYSTEM)

#if defined(CRYPTOACC_PRESENT)

/***************************************************************************//**
 * @brief
 *   Get current SE version
 ******************************************************************************/
sli_se_mailbox_response_t sli_vse_mailbox_get_version(uint32_t *version)
{
  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();

  if (version == NULL) {
    return SLI_SE_RESPONSE_INVALID_PARAMETER;
  }

  // First verify that the response is ok.
  if (!sli_vse_mailbox_is_output_valid()) {
    return SLI_SE_RESPONSE_MAILBOX_INVALID;
  }

  // Return the 'version' from the Output Mailbox
  *version = root_mailbox_output->version;

  return SLI_SE_RESPONSE_OK;
}

/***************************************************************************//**
 * @brief
 *   Get VSE configuration and status bits
 ******************************************************************************/
sli_se_mailbox_response_t sli_vse_mailbox_get_cfg_status(uint32_t *cfg_status)
{
  if (cfg_status == NULL) {
    return SLI_SE_RESPONSE_INVALID_PARAMETER;
  }

  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();

  // First verify that the response is ok.
  if (!sli_vse_mailbox_is_output_valid()) {
    return SLI_SE_RESPONSE_MAILBOX_INVALID;
  }

  // Return the configuration status bits
  *cfg_status = root_mailbox_output->status & ROOT_MB_OUTPUT_STATUS_CONFIG_BITS_MASK;

  return SLI_SE_RESPONSE_OK;
}

/***************************************************************************//**
 * @brief
 *  Get the version number of the OTP from the status field of the output
 *  mailbox
 ******************************************************************************/
sli_se_mailbox_response_t sli_vse_mailbox_get_otp_version(uint32_t *otp_version)
{
  if (otp_version == NULL) {
    return SLI_SE_RESPONSE_INVALID_PARAMETER;
  }

  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();

  // First verify that the response is ok.
  if (!sli_vse_mailbox_is_output_valid()) {
    return SLI_SE_RESPONSE_MAILBOX_INVALID;
  }

  bool is_otp_version_set = root_mailbox_output->status & SE_VSE_REPLY_STATUS_OTP_VERSION_SET;
  if (is_otp_version_set) {
    // Return the OTP version from the status field.
    *otp_version = (root_mailbox_output->status & SE_VSE_REPLY_STATUS_OTP_VERSION_MASK) >> SE_VSE_REPLY_STATUS_OTP_VERSION_SHIFT;
  } else {
    return SLI_SE_RESPONSE_INVALID_COMMAND;
  }

  return SLI_SE_RESPONSE_OK;
}

/***************************************************************************//**
 * @brief
 *   Read the previously executed command.
 ******************************************************************************/
uint32_t sli_vse_mailbox_read_executed_command(void)
{
  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();

  // First verify that the Output Mailbox includes a valid response.
  if (!vse_is_command_completed(root_mailbox_output)) {
    return SLI_SE_RESPONSE_MAILBOX_INVALID;
  }

  return root_mailbox_output->command;
}

/***************************************************************************//**
 * @brief
 *   Read the status of the previously executed command.
 *
 * @details
 *   This function reads the status of the previously executed command.
 *
 * @return
 *   One of the SE_RESPONSE return codes:
 *   SLI_SE_RESPONSE_OK when the command was executed successfully or a signature
 *   was successfully verified,
 *   SLI_SE_RESPONSE_INVALID_COMMAND when the command ID was not recognized,
 *   SE_RESPONSE_AUTHORIZATION_ERROR when the command is not authorized,
 *   SE_RESPONSE_INVALID_SIGNATURE when signature verification failed,
 *   SE_RESPONSE_BUS_ERROR when a bus error was thrown during the command, e.g.
 *   because of conflicting Secure/Non-Secure memory accesses,
 *   SE_RESPONSE_CRYPTO_ERROR on an internal SE failure, or
 *   SLI_SE_RESPONSE_INVALID_PARAMETER when an invalid parameter was passed
 *   SLI_SE_RESPONSE_MAILBOX_INVALID when the mailbox content is invalid
 ******************************************************************************/
sli_se_mailbox_response_t sli_se_mailbox_read_response(void)
{
  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();

  // First verify that the Output Mailbox includes a valid response.
  if (!vse_is_command_completed(root_mailbox_output)) {
    return SLI_SE_RESPONSE_MAILBOX_INVALID;
  }

  return (sli_se_mailbox_response_t)(root_mailbox_output->status & SLI_SE_RESPONSE_MASK);
}

/***************************************************************************//**
 * @brief
 *   Acknowledge and get status and output data of a completed command.
 *
 * @details
 *   This function acknowledges and gets the status and output data of a
 *   completed mailbox command.
 *   The mailbox command is acknowledged by inverting all bits in the checksum
 *   (XOR with 0xFFFFFFFF).
 *   The output data is copied into the linked list of output buffers pointed
 *   to in the given command data structure.
 *
 * @param[in]  command
 *   Pointer to an SE command structure.
 *
 * @return
 *   One of the SE_RESPONSE return codes.
 * @retval SLI_SE_RESPONSE_OK when the command was executed successfully or a
 *                        signature was successfully verified,
 * @retval SLI_SE_RESPONSE_INVALID_COMMAND when the command ID was not recognized,
 * @retval SE_RESPONSE_AUTHORIZATION_ERROR when the command is not authorized,
 * @retval SE_RESPONSE_INVALID_SIGNATURE when signature verification failed,
 * @retval SE_RESPONSE_BUS_ERROR when a bus error was thrown during the command,
 *                               e.g. because of conflicting Secure/Non-Secure
 *                               memory accesses,
 * @retval SE_RESPONSE_CRYPTO_ERROR on an internal SE failure, or
 * @retval SLI_SE_RESPONSE_INVALID_PARAMETER when an invalid parameter was passed
 * @retval SLI_SE_RESPONSE_MAILBOX_INVALID when mailbox command not done or invalid
 ******************************************************************************/
sli_se_mailbox_response_t sli_vse_mailbox_ack_command(sli_se_mailbox_command_t *command)
{
  // Setup pointer to the VSE Output Mailbox data structure
  // (must be stored in a RAM area which is not used by the VSE)
  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();

  uint32_t *mbData = (uint32_t*) root_mailbox_output->data;
  volatile sli_se_datatransfer_t *outDataDesc = command->data_out;
  unsigned int outDataLen, outDataCnt, i, outDescLen;
  uint32_t *outData;

  // First verify that the Output Mailbox includes a valid response.
  if (!vse_is_command_completed(root_mailbox_output)) {
    return SLI_SE_RESPONSE_MAILBOX_INVALID;
  }

  // Get output data length
  outDataLen = root_mailbox_output->length;

  // Acknowledge the output mailbox response by invalidating checksum
  mbData[outDataLen] ^= 0xFFFFFFFFUL;

  // Check command status code
  if ((root_mailbox_output->status & SLI_SE_RESPONSE_MASK) != SLI_SE_RESPONSE_OK) {
    return root_mailbox_output->status & SLI_SE_RESPONSE_MASK;
  }

  // Copy data from the Output Mailbox to the linked list of output
  // buffers provided by the user
  outDataCnt = 0;
  while (outDataDesc && (outDataCnt < outDataLen)) {
    outData = (uint32_t*) outDataDesc->data;
    outDescLen =
      (outDataDesc->length & SLI_SE_DATATRANSFER_LENGTH_MASK) / sizeof(uint32_t);
    for (i = 0; (i < outDescLen) && (outDataCnt < outDataLen); i++) {
      outData[i] = mbData[outDataCnt++];
    }
    // If we have reached the end of a buffer, go to next buffer descriptor
    if (i == outDescLen) {
      outDataDesc = (sli_se_datatransfer_t*)
                    ((uint32_t)outDataDesc->next & ~SLI_SE_DATATRANSFER_STOP);
    }
  }

  // Check if the output data list is too small to copy all output data in
  // mailbox.
  if ((outDataDesc == 0) && (outDataCnt < outDataLen)) {
    return SLI_SE_RESPONSE_INVALID_PARAMETER;
  }

  return SLI_SE_RESPONSE_OK;
}

/***************************************************************************//**
 * @brief
 *   Check whether the VSE Output Mailbox is valid.
 *
 * @return True if the VSE Output Mailbox is valid (magic and checksum OK)
 ******************************************************************************/
bool sli_vse_mailbox_is_output_valid(void)
{
  root_mailbox_output_t *root_mailbox_output = get_root_mailbox_output();
  uint32_t *mb_ptr32 = (uint32_t*) root_mailbox_output;
  uint32_t checksum;
  unsigned int mb_len, cnt;

  if ((uint32_t)root_mailbox_output > ROOT_MAILBOX_OUTPUT_BASE_EXPECTED
      || (uint32_t)root_mailbox_output < RDMEM_FRCRAM_MEM_BASE) {
    return false;
  }

  // Verify magic word of mailbox
  if (root_mailbox_output->magic != SLI_SE_RESPONSE_MAILBOX_VALID) {
    return false;
  }

  // Get length of mailbox
  mb_len = sizeof(root_mailbox_output_t) / sizeof(uint32_t) + root_mailbox_output->length;
  if (mb_len >= ROOT_MAILBOX_SIZE) {
    return false;
  }
  // Calculate checksum using bitwise XOR over all words in the mailbox
  // data structure, minus the CHECKSUM word at the end.
  for (checksum = 0, cnt = 0; cnt < mb_len; cnt++) {
    checksum ^= mb_ptr32[cnt];
  }

  // Verify that the calculated checksum is equal to the mailbox checksum.
  return (mb_ptr32[mb_len] == checksum);
}

#endif // #if defined(CRYPTOACC_PRESENT)

/** @} (end addtogroup se) */

#endif /* defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT) */
