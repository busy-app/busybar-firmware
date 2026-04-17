/***************************************************************************//**
 * Copyright 2023-2024 Silicon Laboratories Inc. www.silabs.com
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available here[1]. This software is distributed to you in
 * Source Code format and is governed by the sections of the MSLA applicable to
 * Source Code.
 *
 * [1] www.silabs.com/about-us/legal/master-software-license-agreement
 *
 ******************************************************************************/
#include "sl_component_catalog.h"
#include "sl_wsrcp_btl_interface.h"
#include "sl_wsrcp_log.h"

#include <btl_interface.h>

void wisun_rcp_btl_reset(void)
{
    int32_t ret;

    ret = bootloader_init();
    BUG_ON(ret != BOOTLOADER_OK);
    bootloader_rebootAndInstall();
}
