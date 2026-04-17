/*******************************************************************************
* @file  sl_net_basic_certificate_store.c
* @brief 
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

#include "sl_net.h"

sl_status_t sl_net_set_certificate(sl_net_certificate_id_t id, const void *data, uint32_t data_length)
{
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(data_length);
  return SL_STATUS_NOT_SUPPORTED;
}
sl_status_t sl_net_get_certificate(sl_net_certificate_id_t id, const void *data, uint32_t data_length)
{
  UNUSED_PARAMETER(id);
  UNUSED_PARAMETER(data);
  UNUSED_PARAMETER(data_length);
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_net_verify_certificate()
{
  return SL_STATUS_NOT_SUPPORTED;
}
