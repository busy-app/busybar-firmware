/***************************************************************************/ /**
 * @file
 * @brief SPI abstraction used by memory lcd display
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_MEMLCD_SPI_H
#define SL_MEMLCD_SPI_H
typedef __UINT32_TYPE__ __uint32_t;

#ifndef _UINT32_T_DECLARED
typedef __uint32_t uint32_t;
#define _UINT32_T_DECLARED
#endif

typedef uint32_t sl_status_t;

#ifdef __cplusplus
extern "C" {
#endif

#define SL_MEMLCD_SPI_VALUE_NONE 0xFF

/***************************************************************************/ /**
 * @addtogroup memlcd
 * @{
 ******************************************************************************/

/***************************************************************************/ /**
 * @brief
 *   Initialize the SPI interface.
 *
 * @param[in] handle
 *   Handle to the SPI interface.
 *
 * @param[in] baudrate
 *   Requested frequency of the SPI interface. This will determine the speed
 *   of transfers.
 *
 * @param[in] mode
 *   This parameter determines the clock phase and clock polarity.
 *
 * @return
 *   If all operations completed sucessfully SL_STATUS_OK is returned. On
 *   failure a different status code is returned specifying the error.
 *****************************************************************************/
sl_status_t sli_memlcd_spi_init(uint32_t spi_baud);

/***************************************************************************/ /**
 * @brief
 *   Shutdown the SPI interface.
 *
 * @detail
 *   Note that this function will also disable the clock to the SPI inteface.
 *   So this function should not be called if there are multiple users of the
 *   same SPI interface.
 *
 * @param[in] handle
 *   Handle to the SPI interface.
 *
 * @return
 *   If all operations completed sucessfully SL_STATUS_OK is returned. On
 *   failure a different status code is returned specifying the error.
 *****************************************************************************/
sl_status_t sli_memlcd_spi_shutdown();

/***************************************************************************/ /**
 * @brief
 *   Transmit data on the SPI interface connected to a display.
 *
 * @param[in] handle
 *   Handle to the SPI interface.
 *
 * @param[in] data
 *   Pointer to the data to be transmitted.
 *
 * @param[in] len
 *   Length of data to transmit.
 *
 * @return
 *   If all operations completed sucessfully SL_STATUS_OK is returned. On
 *   failure a different status code is returned specifying the error.
 *****************************************************************************/
sl_status_t sli_memlcd_spi_tx(const void *data, unsigned len);

/***************************************************************************/ /**
 * @brief
 *   Wait for the SPI interface to complete all the pending transfers.
 *
 * @param[in] handle
 *   Handle to the SPI interface.
 *****************************************************************************/
void sli_memlcd_spi_wait();

/***************************************************************************/ /**
 * @brief
 *   Empty RX FIFO.
 *
 * @param[in] handle
 *   Handle to the SPI interface.
 *****************************************************************************/
void sli_memlcd_spi_rx_flush();

void mySPI_callback_spi(uint32_t event);

/** @} */
#ifdef __cplusplus
}
#endif

#endif