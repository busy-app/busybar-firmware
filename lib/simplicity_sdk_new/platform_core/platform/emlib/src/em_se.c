/***************************************************************************//**
 * @file
 * @brief Secure Element API
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "em_se.h"

#if defined(SLI_EM_SE_HOST) || defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)

#if defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT)

#include "sl_core.h"
#include "sl_assert.h"

#endif

/***************************************************************************//**
 * @addtogroup se
 * @{
 ******************************************************************************/

/*******************************************************************************
 ******************************   DEFINES    ***********************************
 ******************************************************************************/
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
/* OTP initialization structure defines. */
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ENABLE (1 << 16)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_VERIFY_CERTIFICATE (1 << 17)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_ANTI_ROLLBACK (1 << 18)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_NARROW (1 << 19)
#define SE_OTP_MCU_SETTINGS_FLAG_SECURE_BOOT_PAGE_LOCK_FULL (1 << 20)

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
} root_InputMailbox_t;

// VSE Output Mailbox structure
typedef struct {
  volatile uint32_t magic;
  volatile uint32_t version;
  volatile uint32_t status;
  volatile uint32_t command;
  volatile uint32_t length;
  volatile uint32_t data[0];
} root_OutputMailbox_t;
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
__STATIC_INLINE void writeToFifo(uint32_t value)
{
  SEMAILBOX_HOST->FIFO = value;
}
#else
__STATIC_INLINE void writeToFifo(uint32_t value)
{
  SEMAILBOX_HOST->FIFO[0].DATA = value;
}
#endif

#endif // SEMAILBOX_PRESENT

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * @brief
 *   Add input data to a command
 *
 * @details
 *   This function adds a buffer of input data to the given SE command structure
 *   The buffer gets appended by reference at the end of the list of already
 *   added buffers.
 *
 * @note
 *   Note that this function does not copy either the data buffer or the buffer
 *   structure, so make sure to keep the data object in scope until the command
 *   has been executed by the secure element.
 *
 * @param[in]  command
 *   Pointer to an SE command structure.
 *
 * @param[in]  data
 *   Pointer to a data transfer structure.
 ******************************************************************************/
void SE_addDataInput(SE_Command_t *command, volatile SE_DataTransfer_t *data)
{
  if (command->data_in == NULL) {
    command->data_in = data;
  } else {
    volatile SE_DataTransfer_t *next = command->data_in;
    while (next->next != (void*)SE_DATATRANSFER_STOP) {
      next = (SE_DataTransfer_t*)next->next;
    }
    next->next = data;
  }
}

/***************************************************************************//**
 * @brief
 *   Add output data to a command
 *
 * @details
 *   This function adds a buffer of output data to the given command structure
 *   The buffer gets appended by reference at the end of the list of already
 *   added buffers.
 *
 * @note
 *   Note that this function does not copy either the data buffer or the buffer
 *   structure, so make sure to keep the data object in scope until the command
 *   has been executed by the secure element.
 *
 * @param[in]  command
 *   Pointer to an SE command structure.
 *
 * @param[in]  data
 *   Pointer to a data transfer structure.
 ******************************************************************************/
void SE_addDataOutput(SE_Command_t *command,
                      volatile SE_DataTransfer_t *data)
{
  if (command->data_out == NULL) {
    command->data_out = data;
  } else {
    volatile SE_DataTransfer_t *next = command->data_out;
    while (next->next != (void*)SE_DATATRANSFER_STOP) {
      next = (SE_DataTransfer_t*)next->next;
    }
    next->next = data;
  }
}

/***************************************************************************//**
 * @brief
 *   Add a parameter to a command
 *
 * @details
 *   This function adds a parameter word to the passed command.
 *
 * @note
 *   Make sure to not exceed @ref SE_MAX_PARAMETERS.
 *
 * @param[in]  command
 *   Pointer to a filled-out SE command structure.
 * @param[in]  parameter
 *   Parameter to add.
 ******************************************************************************/
void SE_addParameter(SE_Command_t *command, uint32_t parameter)
{
  if (command->num_parameters >= SE_MAX_PARAMETERS) {
    EFM_ASSERT(command->num_parameters < SE_MAX_PARAMETERS);
    return;
  }

  command->parameters[command->num_parameters] = parameter;
  command->num_parameters += 1;
}

#if !defined(SLI_EM_SE_HOST)
/***************************************************************************//**
 * @brief
 *   Execute the passed command
 *
 * @details
 *   This function starts the execution of the passed command by the secure
 *   element. When started, wait for the RXINT interrupt flag, or call
 *   @ref SE_waitCommandCompletion to busy-wait. After completion, you have to
 *   call @ref SE_readCommandResponse to get the command's execution status.
 *
 * @param[in]  command
 *   Pointer to a filled-out SE command structure.
 ******************************************************************************/
void SE_executeCommand(SE_Command_t *command)
{
  // Don't overflow our struct
  if (command->num_parameters > SE_MAX_PARAMETERS) {
    EFM_ASSERT(command->num_parameters <= SE_MAX_PARAMETERS);
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
  writeToFifo(0);
  #else
  // Write header (including message size) to start transaction
  SEMAILBOX_HOST->TX_HEADER = sizeof(uint32_t) * (4 + command->num_parameters);
  #endif

  // Write command into FIFO
  writeToFifo(command->command);

  // Write DMA descriptors into FIFO
  writeToFifo((uint32_t)command->data_in);
  writeToFifo((uint32_t)command->data_out);

  // Write applicable parameters into FIFO
  for (size_t i = 0; i < command->num_parameters; i++) {
    writeToFifo(command->parameters[i]);
  }

#elif defined(CRYPTOACC_PRESENT)
  // Prepare the VSE Mailbox within a critical section to prevent
  // the process from getting interrupted. At this point, the only option
  // we have is to go through a reset, so it is safe to enter the critical section.
  (void)CORE_EnterCritical();

  // Setup pointer to the VSE Mailbox Input data structure
  // (must be stored in a RAM area which is not used by the VSE)
  root_InputMailbox_t *rootInMb = (root_InputMailbox_t*)ROOT_MAILBOX_INPUT_BASE;
  uint32_t *mbData;
  unsigned int mbDataLen, inDataLen, i;
  volatile SE_DataTransfer_t *inDataDesc;
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

  rootInMb->magic   = SE_RESPONSE_MAILBOX_VALID;
  rootInMb->command = command->command;

  // Write applicable parameters into Mailbox DATA array
  mbData = (uint32_t*) rootInMb->data;
  for (mbDataLen = 0; mbDataLen < command->num_parameters; mbDataLen++) {
    mbData[mbDataLen] = command->parameters[mbDataLen];
  }

  // Write input data into Mailbox DATA array
  inDataLen = 0;
  for (inDataDesc = command->data_in; inDataDesc; inDataDesc = (SE_DataTransfer_t*) inDataDesc->next) {
    inData = (uint32_t*) inDataDesc->data;
    for (i = 0; i < (inDataDesc->length & SE_DATATRANSFER_LENGTH_MASK) / sizeof(uint32_t); i++) {
      // Make sure we do not overflow the input mailbox.
      EFM_ASSERT(mbDataLen < ROOT_MAILBOX_SIZE);
      mbData[mbDataLen++] = inData[i];
      inDataLen++;
    }
    if (inDataDesc->next == (void*)SE_DATATRANSFER_STOP) {
      break;
    }
  }

  // Write number of parameters and data words to 'length' field of mailbox.
  rootInMb->length =
    inDataLen | (command->num_parameters << ROOT_MB_LENGTH_PARAM_NUM_SHIFT);

  // Calculate checksum using bitwise XOR over the all words in the mailbox
  // data structure, minus the CHECKSUM word (32bit = 4bytes ) at the end.
  checksum = rootInMb->magic;
  checksum ^= rootInMb->command;
  checksum ^= rootInMb->length;
  for (i = 0; i < mbDataLen; i++) {
    checksum ^= mbData[i];
  }

  // Finally, write the calculated checksum to mailbox checksum field
  mbData[mbDataLen] = checksum;

  __NVIC_SystemReset();

#endif // #if defined(SEMAILBOX_PRESENT)
}

#endif // #if !defined(SLI_EM_SE_HOST)

#if defined(CRYPTOACC_PRESENT)
/***************************************************************************//**
 * @brief
 *   Check whether the VSE Output Mailbox is valid.
 *
 * @return True if the VSE Output Mailbox is valid (magic and checksum OK)
 ******************************************************************************/
bool rootIsOutputMailboxValid(void)
{
  // Setup pointer to the VSE Output Mailbox data structure
  // (must be stored in a RAM area which is not used by the VSE)
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if ((uint32_t)rootOutMb > ROOT_MAILBOX_OUTPUT_BASE_EXPECTED
      || (uint32_t)rootOutMb < RDMEM_FRCRAM_MEM_BASE) {
    return false;
  }

  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  uint32_t *mbPtr = (uint32_t*) rootOutMb;
  uint32_t checksum;
  unsigned int mbLen, cnt;

  // Verify magic word of mailbox
  if (rootOutMb->magic != SE_RESPONSE_MAILBOX_VALID) {
    return false;
  }

  // Get length of mailbox
  mbLen = sizeof(root_OutputMailbox_t) / sizeof(uint32_t) + rootOutMb->length;
  if (mbLen >= ROOT_MAILBOX_SIZE) {
    return false;
  }
  // Calculate checksum using bitwise XOR over all words in the mailbox
  // data structure, minus the CHECKSUM word at the end.
  for (checksum = 0, cnt = 0; cnt < mbLen; cnt++) {
    checksum ^= mbPtr[cnt];
  }

  // Verify that the calculated checksum is equal to the mailbox checksum.
  return (mbPtr[mbLen] == checksum);
}

/***************************************************************************//**
 * @brief
 *   Get current SE version
 *
 * @details
 *   This function returns the current VSE version
 *
 * @param[in]  version
 *   Pointer to location where to copy the version of VSE to.
 *
 * @return
 *   One of the SE_RESPONSE return codes:
 *   SE_RESPONSE_OK when the command was executed successfully
 *   SE_RESPONSE_INVALID_PARAMETER when an invalid parameter was passed
 *   SE_RESPONSE_MAILBOX_INVALID when the mailbox content is invalid
 ******************************************************************************/
SE_Response_t SE_getVersion(uint32_t *version)
{
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  if (version == NULL) {
    return SE_RESPONSE_INVALID_PARAMETER;
  }

  // First verify that the response is ok.
  if (!rootIsOutputMailboxValid()) {
    return SE_RESPONSE_MAILBOX_INVALID;
  }

  // Return the 'version' from the Output Mailbox
  *version = rootOutMb->version;

  return SE_RESPONSE_OK;
}

/***************************************************************************//**
 * @brief
 *   Get VSE configuration and status bits
 *
 * @details
 *   This function returns the current VSE configuration and status bits.
 *   The following list explains what the different bits in cfgStatus indicate.
 *   A bit value of 1 means enabled, while 0 means disabled:
 *    * [0]: Secure boot
 *    * [1]: Verify secure boot certificate
 *    * [2]: Anti-rollback
 *    * [3]: Narrow page lock
 *    * [4]: Full page lock
 *   The following status bits can be read with VSE versions
 *   higher than 1.2.2.
 *    * [10]: Debug port lock
 *    * [11]: Device erase enabled
 *    * [12]: Secure debug enabled
 *    * [15]: Debug port register state, 1 if the debug port is locked.
 *
 * @param[out]  cfgStatus
 *   Pointer to location to copy Configuration Status bits into.
 *
 * @note
 *   This function will check that the mailbox content is valid before
 *   reading the status bits. If the command response has already been read
 *   with a call to @ref SE_ackCommand(), the validity check will fail, and
 *   the config status bits cannot be read before a reset has occurred.
 *
 * @return
 *   One of the SE_RESPONSE return codes:
 *   SE_RESPONSE_OK when the command was executed successfully
 *   SE_RESPONSE_INVALID_PARAMETER when an invalid parameter was passed
 *   SE_RESPONSE_MAILBOX_INVALID when the mailbox content is invalid
 ******************************************************************************/
SE_Response_t SE_getConfigStatusBits(uint32_t *cfgStatus)
{
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  if (cfgStatus == NULL) {
    return SE_RESPONSE_INVALID_PARAMETER;
  }

  // First verify that the response is ok.
  if (!rootIsOutputMailboxValid()) {
    return SE_RESPONSE_MAILBOX_INVALID;
  }

  // Return the configuration status bits
  *cfgStatus = rootOutMb->status & ROOT_MB_OUTPUT_STATUS_CONFIG_BITS_MASK;

  return SE_RESPONSE_OK;
}

/***************************************************************************//**
 * @brief
 *  Get the version number of the OTP from the status field of the output
 *  mailbox
 * @details
 *  This function checks if the OTP version number flag is set in the output
 *  mailbox. If it is, the version number is writen to @ref otpVersion pointer
 *  location. If not, it returns error response.
 *
 * @param[out] otpVersion
 *  Pointer to location to copy OTP version number into.
 * @return
 *  One of the SE_RESPONSE return codes.
 * @retval SE_RESPONSE_OK when the command was executed successfully
 ******************************************************************************/
SE_Response_t SE_getOTPVersion(uint32_t *otpVersion)
{
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  if (otpVersion == NULL) {
    return SE_RESPONSE_INVALID_PARAMETER;
  }

  // First verify that the response is ok.
  if (!rootIsOutputMailboxValid()) {
    return SE_RESPONSE_MAILBOX_INVALID;
  }

  bool isOTPVersionSet = rootOutMb->status & SE_VSE_REPLY_STATUS_OTP_VERSION_SET;
  if (isOTPVersionSet) {
    // Return the OTP version from the status field.
    *otpVersion = (rootOutMb->status & SE_VSE_REPLY_STATUS_OTP_VERSION_MASK) >> SE_VSE_REPLY_STATUS_OTP_VERSION_SHIFT;
  } else {
    return SE_RESPONSE_INVALID_COMMAND;
  }

  return SE_RESPONSE_OK;
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
bool SE_isCommandCompleted(void)
{
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  // First verify that the response is ok
  if (!rootIsOutputMailboxValid()) {
    return false;
  }

  // Check status MB_DONE flag of the mailbox
  return ((rootOutMb->status & ROOT_MB_DONE) == ROOT_MB_DONE);
}

/***************************************************************************//**
 * @brief
 *   Read the previously executed command.
 *
 * @details
 *   This function reads the previously executed command.
 *
 * @return
 *   One of the SE command words.
 *   SE_RESPONSE_MAILBOX_INVALID when the mailbox content is invalid.
 ******************************************************************************/
uint32_t SE_readExecutedCommand(void)
{
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  // First verify that the Output Mailbox includes a valid response.
  if (!SE_isCommandCompleted()) {
    return SE_RESPONSE_MAILBOX_INVALID;
  }

  return rootOutMb->command;
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
 *   SE_RESPONSE_OK when the command was executed successfully or a signature
 *   was successfully verified,
 *   SE_RESPONSE_INVALID_COMMAND when the command ID was not recognized,
 *   SE_RESPONSE_AUTHORIZATION_ERROR when the command is not authorized,
 *   SE_RESPONSE_INVALID_SIGNATURE when signature verification failed,
 *   SE_RESPONSE_BUS_ERROR when a bus error was thrown during the command, e.g.
 *   because of conflicting Secure/Non-Secure memory accesses,
 *   SE_RESPONSE_CRYPTO_ERROR on an internal SE failure, or
 *   SE_RESPONSE_INVALID_PARAMETER when an invalid parameter was passed
 *   SE_RESPONSE_MAILBOX_INVALID when the mailbox content is invalid
 ******************************************************************************/
SE_Response_t SE_readCommandResponse(void)
{
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }

  // First verify that the Output Mailbox includes a valid response.
  if (!SE_isCommandCompleted()) {
    return SE_RESPONSE_MAILBOX_INVALID;
  }

  return (SE_Response_t)(rootOutMb->status & SE_RESPONSE_MASK);
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
 * @retval SE_RESPONSE_OK when the command was executed successfully or a
 *                        signature was successfully verified,
 * @retval SE_RESPONSE_INVALID_COMMAND when the command ID was not recognized,
 * @retval SE_RESPONSE_AUTHORIZATION_ERROR when the command is not authorized,
 * @retval SE_RESPONSE_INVALID_SIGNATURE when signature verification failed,
 * @retval SE_RESPONSE_BUS_ERROR when a bus error was thrown during the command,
 *                               e.g. because of conflicting Secure/Non-Secure
 *                               memory accesses,
 * @retval SE_RESPONSE_CRYPTO_ERROR on an internal SE failure, or
 * @retval SE_RESPONSE_INVALID_PARAMETER when an invalid parameter was passed
 * @retval SE_RESPONSE_MAILBOX_INVALID when mailbox command not done or invalid
 ******************************************************************************/
SE_Response_t SE_ackCommand(SE_Command_t *command)
{
  // Setup pointer to the VSE Output Mailbox data structure
  // (must be stored in a RAM area which is not used by the VSE)
  bool sysCfgClkWasEnabled = ((CMU->CLKEN0 & CMU_CLKEN0_SYSCFG) != 0);
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
  root_OutputMailbox_t *rootOutMb = (root_OutputMailbox_t *) ROOT_MAILBOX_OUTPUT_BASE;
  if (!sysCfgClkWasEnabled) {
    CMU->CLKEN0_CLR = CMU_CLKEN0_SYSCFG;
  }
  uint32_t *mbData = (uint32_t*) rootOutMb->data;
  volatile SE_DataTransfer_t *outDataDesc = command->data_out;
  unsigned int outDataLen, outDataCnt, i, outDescLen;
  uint32_t *outData;

  // First verify that the Output Mailbox includes a valid response.
  if (!SE_isCommandCompleted()) {
    return SE_RESPONSE_MAILBOX_INVALID;
  }

  // Get output data length
  outDataLen = rootOutMb->length;

  // Acknowledge the output mailbox response by invalidating checksum
  mbData[outDataLen] ^= 0xFFFFFFFFUL;

  // Check command status code
  if ((rootOutMb->status & SE_RESPONSE_MASK) != SE_RESPONSE_OK) {
    return rootOutMb->status & SE_RESPONSE_MASK;
  }

  // Copy data from the Output Mailbox to the linked list of output
  // buffers provided by the user
  outDataCnt = 0;
  while (outDataDesc && (outDataCnt < outDataLen)) {
    outData = (uint32_t*) outDataDesc->data;
    outDescLen =
      (outDataDesc->length & SE_DATATRANSFER_LENGTH_MASK) / sizeof(uint32_t);
    for (i = 0; (i < outDescLen) && (outDataCnt < outDataLen); i++) {
      outData[i] = mbData[outDataCnt++];
    }
    // If we have reached the end of a buffer, go to next buffer descriptor
    if (i == outDescLen) {
      outDataDesc = (SE_DataTransfer_t*)
                    ((uint32_t)outDataDesc->next & ~SE_DATATRANSFER_STOP);
    }
  }

  // Check if the output data list is too small to copy all output data in
  // mailbox.
  if ((outDataDesc == 0) && (outDataCnt < outDataLen)) {
    return SE_RESPONSE_INVALID_PARAMETER;
  }

  return SE_RESPONSE_OK;
}

#endif // #if defined(CRYPTOACC_PRESENT)

/*******************************************************************************
 *****************************   DEPRECATED    *********************************
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup se_deprecated
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
*  The following functions have been deprecated and will be removed in a future
*  version of emlib. All high-level functionality have been moved to the SE
*  manager.
*******************************************************************************/

#if !defined(SLI_EM_SE_HOST)
#if defined(SEMAILBOX_PRESENT)

/***************************************************************************//**
 * @brief
 * @deprecated
 *   This function has been moved to the SE manager, and will be removed in a
 *   future version of emlib.
 *
 *   Read pubkey or pubkey signature.
 *
 * @details
 *   Read out a public key stored in the SE, or its signature. The command can
 *   be used to read:
 *   * SE_KEY_TYPE_BOOT
 *   * SE_KEY_TYPE_AUTH
 *
 * @param[in] key_type
 *   ID of key type to read.
 *
 * @param[out] pubkey
 *   Pointer to a buffer to contain the returned public key.
 *   Must be word aligned and have a length of 64 bytes.
 *
 * @param[in] numBytes
 *   Length of pubkey buffer (64 bytes).
 *
 * @param[in] signature
 *   If true, the function will return the signature programmed for the
 *   specified public key instead of the public key itself.
 *
 * @return
 *   One of the SE_RESPONSE return codes.
 * @retval SE_RESPONSE_OK when the command was executed successfully
 * @retval SE_RESPONSE_TEST_FAILED when the pubkey is not set
 * @retval SE_RESPONSE_INVALID_PARAMETER when an invalid type is passed
 ******************************************************************************/
SE_Response_t SE_readPubkey(uint32_t key_type, void *pubkey, uint32_t numBytes, bool signature)
{
  EFM_ASSERT((key_type == SE_KEY_TYPE_BOOT)
             || (key_type == SE_KEY_TYPE_AUTH));

  EFM_ASSERT(numBytes == 64);
  EFM_ASSERT(!((size_t)pubkey & 3U));

  // SE command structures
  uint32_t commandWord =
    (signature) ? SE_COMMAND_READ_PUBKEY_SIGNATURE : SE_COMMAND_READ_PUBKEY;
  SE_Command_t command = SE_COMMAND_DEFAULT(commandWord | key_type);

  volatile SE_DataTransfer_t pubkeyData = SE_DATATRANSFER_DEFAULT(pubkey, numBytes);
  SE_addDataOutput(&command, &pubkeyData);

  SE_executeCommand(&command);
  SE_Response_t res = SE_readCommandResponse();
  return res;
}

#endif // #if defined(SEMAILBOX_PRESENT)
#endif // #if !defined(SLI_EM_SE_HOST)

/** @} (end addtogroup deprecated_se) */
/** @} (end addtogroup se) */

#endif /* defined(SEMAILBOX_PRESENT) || defined(CRYPTOACC_PRESENT) */
