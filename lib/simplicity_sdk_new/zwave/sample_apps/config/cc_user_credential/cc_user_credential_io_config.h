/***************************************************************************//**
 * # License
 * <b> Copyright 2024 Silicon Laboratories Inc. www.silabs.com </b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of the Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * https://www.silabs.com/about-us/legal/master-software-license-agreement
 * By installing, copying or otherwise using this software, you agree to the
 * terms of the MSLA.
 *
 ******************************************************************************/

#ifndef _CC_USER_CREDENTIAL_IO_CONFIG_H_
#define _CC_USER_CREDENTIAL_IO_CONFIG_H_

// <<< Use Configuration Wizard in Context Menu >>>

// <h> User Credential Command Class I/O Configuration

// <o U3C_BUFFER_SIZE_CREDENTIAL_DATA> Size of buffer for holding Credential Data <1..255:1> <f.d>
// <i> Default: 100
#define U3C_BUFFER_SIZE_CREDENTIAL_DATA  100

// <o U3C_BUFFER_SIZE_USER_DESCRIPTORS> [SoC NVM driver] User Descriptor table buffer size <1..65535:1> <f.d>
// <i> Size of the buffer used for holding the User Descriptor table when it is read from NVM
// <i> Default: 20
#define U3C_BUFFER_SIZE_USER_DESCRIPTORS  20

// <o U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS> [SoC NVM driver] Credential Descriptor table buffer size <1..65535:1> <f.d>
// <i> Size of the buffer used for holding the Credential Descriptor table when it is read from NVM
// <i> Default: 20
#define U3C_BUFFER_SIZE_CREDENTIAL_DESCRIPTORS  20

// </h>

// <<< end of configuration section >>>

#endif /* _CC_USER_CREDENTIAL_IO_CONFIG_H_ */
