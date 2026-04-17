/***************************************************************************//**
 * @file sl_wsncp_cpc.c
 * @brief Wi-SUN NCP CPC interface implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include <stdio.h>
#include <sl_cpc.h>
#include <sli_cpc.h>

#include "sl_wsncp_interface.h"

#if __has_include("sl_cpc_drv_secondary_spi_config.h")
#include "sl_cpc_drv_secondary_spi_config.h"
#endif

#if defined(SL_CPC_DRV_SPI_EXP_BITRATE) && SL_CPC_DRV_SPI_EXP_BITRATE != 1000000
#error SPI can only be used with a 1MHz clock
#endif

// Static variables
static sl_cpc_endpoint_handle_t cpc_ep;

// CPC callback forward declarations
static void sl_wsncp_cpc_on_receive(uint8_t endpoint_id, void *user_param);
static void sl_wsncp_cpc_transmit_complete(sl_cpc_user_endpoint_id_t endpoint_id,
                                           void *buffer,
                                           void *user_param,
                                           sl_status_t status);
static void sl_wsncp_cpc_error(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *user_param);


// CPC interface implementation
sl_status_t sl_wsncp_interface_init(void)
{
    sl_status_t status;

    // Open CPC endpoint with correct parameters
    status = sli_cpc_open_service_endpoint(&cpc_ep, 
                                           SL_CPC_ENDPOINT_WISUN,
                                           SL_CPC_OPEN_ENDPOINT_FLAG_NONE,
                                           1); // TX window size of 1
    if (status != SL_STATUS_OK) {
        return status;
    }

    // Register callbacks directly to CPC
    status = sl_cpc_set_endpoint_option(&cpc_ep,
                                        SL_CPC_ENDPOINT_ON_IFRAME_RECEIVE,
                                        (void *)&sl_wsncp_cpc_on_receive);
    if (status != SL_STATUS_OK) {
        sl_cpc_close_endpoint(&cpc_ep);
        return status;
    }

    status = sl_cpc_set_endpoint_option(&cpc_ep,
                                        SL_CPC_ENDPOINT_ON_IFRAME_WRITE_COMPLETED,
                                        (void *)&sl_wsncp_cpc_transmit_complete);
    if (status != SL_STATUS_OK) {
        sl_cpc_close_endpoint(&cpc_ep);
        return status;
    }

    status = sl_cpc_set_endpoint_option(&cpc_ep,
                                        SL_CPC_ENDPOINT_ON_ERROR,
                                        (void *)&sl_wsncp_cpc_error);
    if (status != SL_STATUS_OK) {
        sl_cpc_close_endpoint(&cpc_ep);
        return status;
    }

    return SL_STATUS_OK;
}

sl_status_t sl_wsncp_interface_deinit(void)
{
    sl_status_t status;

    status = sl_cpc_close_endpoint(&cpc_ep);
    if (status != SL_STATUS_OK) {
        return status;
    }

    return SL_STATUS_OK;
}

sl_status_t sl_wsncp_interface_transmit(uint16_t len, void *data)
{
    // Transmit data through CPC using zero-copy - non-blocking
    return sl_cpc_write(&cpc_ep, data, len, 0, NULL);
}

// CPC callback implementations
static void sl_wsncp_cpc_on_receive(uint8_t endpoint_id, void *user_param)
{
    sl_status_t status;
    void *rx_buffer;
    uint16_t len;

    (void)endpoint_id;
    (void)user_param;

    // Start receiving through CPC
    status = sl_cpc_read(&cpc_ep, &rx_buffer, &len, 0, 0);
    if (status == SL_STATUS_OK) {
        // Call NCP callback directly
        sl_wsncp_on_receive_cb(SL_STATUS_OK, len, rx_buffer);
        // Free the RX buffer
        sl_cpc_free_rx_buffer(rx_buffer);
    }
}

static void sl_wsncp_cpc_transmit_complete(sl_cpc_user_endpoint_id_t endpoint_id,
                                           void *buffer,
                                           void *user_param,
                                           sl_status_t status)
{
    (void)endpoint_id;
    (void)buffer;
    (void)user_param;

    // Call NCP callback directly - this will clear the busy flag
    sl_wsncp_on_transmit_complete_cb(status);
}

static void sl_wsncp_cpc_error(sl_cpc_user_endpoint_id_t endpoint_id,
                               void *user_param)
{
    (void)endpoint_id;
    (void)user_param;

    // Handle error - what shall we do ?
}