/*******************************************************************************
 * @file  sl_cpc_drv_secondary_sdio.h
 * @brief sdio secondary driver API's
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
#ifndef SL_CPC_DRV_SECONDARY_SDIO_H
#define SL_CPC_DRV_SECONDARY_SDIO_H

void sl_si91x_device_reset(void);
sl_cpc_system_status_t sl_si91x_read_reset_reason(void);

#endif
