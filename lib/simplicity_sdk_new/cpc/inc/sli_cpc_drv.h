/***************************************************************************/ /**
 * @file
 * @brief CPC Driver Interface
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_CPC_DRV_H
#define SLI_CPC_DRV_H

#include "sli_cpc.h"
#include "sl_status.h"
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

typedef struct {
  bool preprocess_hdlc_header;            ///< Is the HDLC header validated by driver
  bool uart_flowcontrol;                  ///< Is UART flow control enabled
} sli_cpc_drv_capabilities_t;

/* forward declare sli_cpc_drv_t as it's referenced in the ops structure */
struct sli_cpc_drv;
typedef struct sli_cpc_drv sli_cpc_drv_t;

typedef struct {
  /// Initialize only the hardware peripheral to be used in a standalone manner
  /// (during the bootloader poking)
  /// @return SL_STATUS_OK if successful. Error code otherwise
  sl_status_t (*hw_init)(sli_cpc_drv_t *driver);

  /// Initialize the rest of the driver after the hardware peripheral has been
  /// initialized in sli_cpc_drv_hw_init.
  /// @return SL_STATUS_OK if successful. Error code otherwise.
  sl_status_t (*init)(sli_cpc_drv_t *driver, sli_cpc_instance_t *inst);

  /// Gets CPC driver capabilities.
  /// @param capabilities Pointer to structure that will receive the driver
  ///                     capabilities.
  ///  @return SL_STATUS_OK if successful. Error code otherwise.
  sl_status_t (*get_capabilities)(sli_cpc_drv_t *driver, sli_cpc_drv_capabilities_t *capabilities);

  /// Start recveiving packets
  /// @return SL_STATUS_OK if successful. Error code otherwise.
  sl_status_t (*start_rx)(sli_cpc_drv_t *driver);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  /// De-Initializes CPC driver for the firmware upgrade to take over control.
  void (*deinit)(sli_cpc_drv_t *driver);
#endif

  /// Reads data from driver.
  /// @param buffer_handle Pointer to variable that will receive receive the buffer handle.
  /// @param payload_rx_len Pointer to variable that will receive the received
  ///                       payload length, in bytes.
  /// @return SL_STATUS_OK if successful. Error code otherwise.
  sl_status_t (*read)(sli_cpc_drv_t *driver, sl_cpc_buffer_handle_t **buffer_handle);

  /// Transmits data.
  /// @param buffer_handle Pointer to the buffer handle.
  /// @return SL_STATUS_OK if successful. Error code otherwise.
  sl_status_t (*write)(sli_cpc_drv_t *driver, sl_cpc_buffer_handle_t *buffer_handle);

  /// Checks if driver is ready to transmit.
  /// @return true if driver is ready to accept a tx. false otherwise.
  bool (*is_transmit_ready)(sli_cpc_drv_t *driver);

  /// Get currently configured bus bitrate
  uint32_t (*get_bus_bitrate)(sli_cpc_drv_t *driver);

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
  /// Set bus bitrate
  void (*set_bus_bitrate)(sli_cpc_drv_t *driver, uint32_t bitrate);
#endif

  /// Get maximum bus bitrate
  uint32_t (*get_bus_max_bitrate)(sli_cpc_drv_t *driver);

  /// Notification on freed RX buffer_handle
  void (*on_rx_buffer_handle_free)(sli_cpc_drv_t *driver);

  /// Notification on freed RX buffer
  void (*on_rx_buffer_free)(sli_cpc_drv_t *driver);
} sli_cpc_drv_ops_t;

struct sli_cpc_drv {
  sli_cpc_drv_ops_t ops;
};

#ifdef __cplusplus
extern "C"
{
#endif

// -----------------------------------------------------------------------------
// Core to driver commands

#if defined(SL_CATALOG_CPC_PRIMARY_PRESENT)
/***************************************************************************//**
 * Return a driver instance. Currently only driver instance can be enabled at
 * the same time. In the future, that limitation might be lifted.
 ******************************************************************************/
sli_cpc_drv_t* sli_cpc_drv_get_driver(void);
#else
#if defined(SL_CATALOG_CPC_DRIVER_UART_PRESENT)
extern sli_cpc_drv_t uart_driver;
#endif

#if defined(SL_CATALOG_CPC_DRIVER_SPI_PRESENT)
extern sli_cpc_drv_t spi_driver;
#endif

#if defined(SL_CATALOG_CPC_DRIVER_SDIO_PRESENT)
extern sli_cpc_drv_t sdio_driver;
#endif

#if defined(SL_CATALOG_CPC_DRIVER_EMUL_PRESENT)
extern sli_cpc_drv_t emul_driver;
#endif
#endif // defined(SL_CATALOG_CPC_PRIMARY_PRESENT)

/** @} (end addtogroup cpc) */

#ifdef __cplusplus
}
#endif

#endif // SLI_CPC_DRV_H
