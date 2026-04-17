/*******************************************************************************
 * @file  sl_si91x_common_flash_intf.h
 * @brief
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

/**
 * Includes
 */
#ifndef SL_SI91x_COMMON_FLASH_INTF_H_
#define SL_SI91x_COMMON_FLASH_INTF_H_

#include "base_types.h"
#include "stdint.h"
#include "sl_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************/ /**
 *  @fn          void rsi_flash_init(void)
 *  @pre         None
 *  @brief       This function initializes GPIO, QSPI and flash
 *  @param       None
 *  @return      None
 ******************************************************************************/
void rsi_flash_init(void);

/***************************************************************************/ /**
 *  @fn          sl_status_t rsi_flash_erase_sector(uint32_t *sector_address)
 *  @pre         None
 *  @brief       This function erases the flash. Erase each sector from the given
 *               sector address to the configured page size. It will wait until
 *               sector erase is complete.
 *  @param[in]   sector_address     Start address of the flash sector
 *  @return      SL_STATUS_OK on success or SL_STATUS code on failure
 ******************************************************************************/
sl_status_t rsi_flash_erase_sector(uint32_t *sector_address);

/***************************************************************************/ /**
 *  @fn          sl_status_t rsi_flash_write(uint32_t *address, unsigned char *data, uint32_t length)
 *  @pre         None
 *  @brief       This function writes to destination flash address location
 *  @param[in]   address     Destination address of NVM3 write
 *  @param[in]   data        A pointer to the object data to write
 *  @param[in]   length      The size of the object data in number of bytes
 *  @return      SL_STATUS_OK on success or SL_STATUS code on failure
 ******************************************************************************/
sl_status_t rsi_flash_write(uint32_t *address, unsigned char *data, uint32_t length);

/***************************************************************************/ /**
 *  @fn          sl_status_t rsi_flash_read(uint32_t *address, unsigned char *data, uint32_t length, uint8_t auto_mode);
 *  @pre         None
 *  @brief       Reads data from the address in selected mode
 *  @param[in]   address     Source address to read the data
 *  @param[in]   length      The size of the object data in number of bytes
 *  @param[in]   auto_mode   Mode of read operation from flash
 *  @param[out]  data        Destination address of NVM3 read to write data
 *  @return      SL_STATUS_OK
 ******************************************************************************/
sl_status_t rsi_flash_read(uint32_t *address, unsigned char *data, uint32_t length, uint8_t auto_mode);

/***************************************************************************/ /**
 *  @fn          void rsi_flash_uninitialize(void)
 *  @pre         None
 *  @brief       Uninitialize QSPI module
 *  @return      None
 ******************************************************************************/
void rsi_flash_uninitialize(void);

#ifdef __cplusplus
}
#endif

#endif /* SL_SI91x_COMMON_FLASH_INTF_H_*/
