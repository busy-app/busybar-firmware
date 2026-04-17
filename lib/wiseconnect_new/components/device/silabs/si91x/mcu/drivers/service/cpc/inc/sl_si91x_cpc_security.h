/*******************************************************************************
 * @file  sl_si91x_cpc_security.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SL_SI91X_CPC_SECURITY_H_
#define SL_SI91X_CPC_SECURITY_H_

#include "cmsis_os2.h"

extern osSemaphoreId_t sl_si91x_cpc_crypto_init_semaphore_id;
extern osStatus_t sl_si91x_cpc_crypto_init_semaphore_status;

void sl_ta_trng_init(void);
void sl_si91x_cpc_crypto_init(void);
void sl_si91x_cpc_crypto_init_task(void);
void sl_si91x_cpc_crypto_semaphore_init(void);

#endif /* SL_SI91X_CPC_SECURITY_H_ */