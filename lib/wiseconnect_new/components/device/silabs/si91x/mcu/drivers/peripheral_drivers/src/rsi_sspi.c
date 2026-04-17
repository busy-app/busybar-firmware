/******************************************************************************
* @file  rsi_sspi.c
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
// Include Files

#include "rsi_sspi.h"

/*==============================================*/
/**
 * @fn           STATIC_INLINE void RSI_SSPI_WriteWord(SPI_SLAVE_Type *pSPI, int32_t byte)
 * @brief        This API is used to write a byte in SPI FIFO Register
 * @param[in]    pSPI    :  SPI base register pointer
 * @param[in]    byte    :  data byte which has to be written in SPI FIFO register  
 * @return       none
 */
__STATIC_INLINE void RSI_SSPI_WriteWord(SPI_SLAVE_Type *pSPI, int32_t byte)
{

  while (pSPI->SPI_STATUS_b.SPI_RFIFO_FULL)
    ;

  //    write byte in to SPI Write FIFO register
  //	SPI_SLAVE_RFIFO_DATA = byte;
  pSPI->SPI_WFIFO_DATA = byte;

  return;
}

/*==============================================*/
/**
 * @fn           STATIC_INLINE int32_t RSI_SSPI_ReadWord(SPI_SLAVE_Type *pSPI)
 * @brief        This API is used to read a byte from SPI FIFO Register
 * @param[in]    pSPI    :  SPI base register pointer
 * @return       Read byte from SPI FIFO register
 */
__STATIC_INLINE int32_t RSI_SSPI_ReadWord(SPI_SLAVE_Type *pSPI)
{
  uint32_t byte;

  //! write byte in to SPI Read FIFO register
  byte = pSPI->SPI_RFIFO_DATA;

  return byte;
}

/*==============================================*/
/**
 * @fn          void RSI_SSPI_IntEnable(RSI_SSPI_HANDLE_T pSPIHandle, uint16_t intmask)
 * @brief       This API enables the interrupts releated to SSPI
 * @param[in]   pSPIHandle    :  SPI base register Handle
 * @param[in]   intmask       :  interrupt mask
 * @return      none
 */
void RSI_SSPI_IntEnable(RSI_SSPI_HANDLE_T pSPIHandle, uint16_t intmask)
{

  RSI_SSPI_REGS_T *pregs = ((RSI_SSPI_DRIVER_T *)pSPIHandle)->pREGS;

  //! Enable the interrupts
  pregs->SPI_INTR_EN = intmask;

  return;
}

/*==============================================*/
/**
 * @fn          void RSI_SSPI_IntClear(RSI_SSPI_HANDLE_T pSPIHandle, uint32_t intmask)
 * @brief       This API clear the interrupts
 * @param[in]   pSPIHandle    :  SPI base register Handle
 * @param[in]   intmask       :  interrupt mask
 * @return      none
 */
void RSI_SSPI_IntClear(RSI_SSPI_HANDLE_T pSPIHandle, uint32_t intmask)
{

  RSI_SSPI_REGS_T *pregs = ((RSI_SSPI_DRIVER_T *)pSPIHandle)->pREGS;

  // UnMask the interrupts
  pregs->SPI_INTR_STATUS |= intmask;

  return;
}

/*==============================================*/
/**
 * @fn          void RSI_SSPI_IntDisable(RSI_SSPI_HANDLE_T pSPIHandle, uint32_t intmask)
 * @brief       This API disable interrupts
 * @param[in]   pSPIHandle    :  SPI base register Handle
 * @param[in]   intmask       :  interrupt mask
 * @return      none
 */
void RSI_SSPI_IntDisable(RSI_SSPI_HANDLE_T pSPIHandle, uint32_t intmask)
{
  RSI_SSPI_REGS_T *pregs = ((RSI_SSPI_DRIVER_T *)pSPIHandle)->pREGS;

  // Disable the interrupts
  pregs->SPI_INTR_MASK &= intmask;

  return;
}

/*==============================================*/
/**
 * @fn          void RSI_SSPI_InterruptHandler(RSI_SSPI_HANDLE_T pSPIHandle)
 * @brief       This API is the SSPI interrupt handler and calls the callback with appropriate event
 * @param[in]   pSPIHandle   : SPI base register Handle
 * @return      none 
 */
void RSI_SSPI_InterruptHandler(RSI_SSPI_HANDLE_T pSPIHandle)
{

  RSI_SSPI_DRIVER_T *pdrv = (RSI_SSPI_DRIVER_T *)pSPIHandle;

  if (pdrv->pREGS->SPI_INTR_STATUS_b.SPI_CS_DEASSERT) {
    // Clear the interrupt
    RSI_SSPI_IntClear(pSPIHandle, SSPI_INT_CS_DEASSERT);

    if (pdrv->cbTable[0] != NULL)
      // Call the registered call back with CS DeASSert event
      pdrv->cbTable[0](pSPIHandle, SSPI_EVENT_CS_DEASSERT, NULL);

  } else if (pdrv->pREGS->SPI_INTR_STATUS_b.SPI_WR_REQ) {
    // Clear the interrupt
    RSI_SSPI_IntClear(pSPIHandle, SSPI_INT_WR);

    if (pdrv->cbTable[0] != NULL)
      // Call the registered call back with  Write request event
      pdrv->cbTable[0](pSPIHandle, SSPI_EVENT_WRITE, NULL);

  } else if (pdrv->pREGS->SPI_INTR_STATUS_b.SPI_RD_REQ) {
    // Clear the interrupt
    RSI_SSPI_IntClear(pSPIHandle, SSPI_INT_RD);

    if (pdrv->cbTable[0] != NULL)
      // Call the registered call back with  Read request event
      pdrv->cbTable[0](pSPIHandle, SSPI_EVENT_READ, NULL);
  }

  return;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_SSPI_Send(RSI_SSPI_HANDLE_T pSPIHandle, const void *data, uint32_t num)
 * @brief       This API writes the data writes the data into SSPI FIFO 
 * @param[in]   pSPIHandle   : SPI base register handle
 * @param[in]   data         : pointer to data to be written in SSPI FIFO register
 * @param[in]   num          : num of data bytes to be written
 * @return      Number of bytes written in SSPI FIFO register
 */
uint32_t RSI_SSPI_Send(RSI_SSPI_HANDLE_T pSPIHandle, const void *data, uint32_t num)
{

  RSI_SSPI_DRIVER_T *pdrv = (RSI_SSPI_DRIVER_T *)pSPIHandle;

  RSI_SSPI_REGS_T *pspi = pdrv->pREGS;

  int sent_bytes = 0;

  int32_t *data32 = (int32_t *)data;

  // Send until the transmit FIFO is not full
  while ((sent_bytes < num) && (!pspi->SPI_STATUS_b.SPI_RFIFO_FULL)) {
    //! Write the byte in SSPI FIFO register
    RSI_SSPI_WriteWord(pspi, *data32);
    data32++;
    sent_bytes += 4;
    pdrv->tx.count += 4;
  }

  // Return number of bytes sent
  return sent_bytes;
}

/*==============================================*/
/**
 * @fn          uint32_t RSI_SSPI_SendBlocking(RSI_SSPI_HANDLE_T pSPIHandle, const void *data, uint32_t num)
 * @brief       This API writes the data writes the data into SSPI FIFO 
 * @param[in]   pSPIHandle  :  SPI base register Handle
 * @param[in]   data        :  pointer to data to be written in SSPI FIFO register
 * @param[in]   num         :  num of data bytes to be written
 * @return      Number of bytes written in SSPI FIFO register 
 */
uint32_t RSI_SSPI_SendBlocking(RSI_SSPI_HANDLE_T pSPIHandle, const void *data, uint32_t num)
{
  uint32_t *data_32 = (uint32_t *)data;
  uint32_t sent = 0, sent_success = 0;

  while (num > 0) {
    sent = RSI_SSPI_Send(pSPIHandle, data_32, num);
    num -= sent;
    sent_success += sent;
    data_32 += sent;
  }
  return sent_success;
}

/*==============================================*/
/**
 * @fn           uint32_t RSI_SSPI_Receive(RSI_SSPI_HANDLE_T pSPIHandle, void *data, uint32_t num)
 * @brief        Receives data from SPI FIFO if available 
 * @param[in]    pSPIHandle  :  SPI base register pointer
 * @param[in]    data        :  pointer data pointer in which read data from SSPI FIFO register is copied to
 * @param[in]    num         :  number of bytes to receive
 * @return       The number of bytes read from SPI FIFO register
 */
uint32_t RSI_SSPI_Receive(RSI_SSPI_HANDLE_T pSPIHandle, void *data, uint32_t num)
{
  RSI_SSPI_DRIVER_T *pdrv = (RSI_SSPI_DRIVER_T *)pSPIHandle;

  RSI_SSPI_REGS_T *pspi = pdrv->pREGS;

  uint32_t readBytes = 0;

  int32_t *data_32 = (int32_t *)data;

  // Allign to 4 byte
  num = ((num + 3) & ~3);

  // Check for status if atleast one byte is arrived in RBR register or not
  while ((readBytes < num) && (!pspi->SPI_STATUS_b.SPI_WFIFO_EMPTY)) {
    // Read the byte
    *data_32 = RSI_SSPI_ReadWord(pspi);

    data_32++;
    readBytes += 4;
    pdrv->rx.count += 4;
  }

  return readBytes;
}

/*==============================================*/
/**
 * @fn            uint32_t RSI_SSPI_ReceiveBlocking(RSI_SSPI_HANDLE_T pSPIHandle, void *data, uint32_t num)
 * @brief         Receives data from SPI FIFO 
 * @param[in]     pSPIHandle   :  SPI base register handle
 * @param[in]     data         :  pointer to data in which read data from SSPI FIFO register is copied to
 * @param[in]     num          :  number of bytes to receive 
 * @return        The number of bytes read from SPI FIFO register
 */
uint32_t RSI_SSPI_ReceiveBlocking(RSI_SSPI_HANDLE_T pSPIHandle, void *data, uint32_t num)
{
  uint32_t *data_32 = (uint32_t *)data;
  uint32_t recv = 0, recv_success = 0;
  //RSI_SSPI_REGS_T *pspi = ((RSI_SSPI_DRIVER_T *)pSPIHandle)->pREGS;

  // Allign to 4 byte
  num = ((num + 3) & ~3);

  while (num > 0) {
    recv = RSI_SSPI_Receive(pSPIHandle, data_32, num);
    num -= recv;
    recv_success += recv;
    data_32 += recv;
  }
  return recv_success;
}

/*==============================================*/
/**
 * @fn            int32_t RSI_SSPI_Transfer(RSI_SSPI_HANDLE_T *pSPIHandle, const void *data_out, void *data_in, uint32_t num)
 * @brief         This API programes DMA for TX and RX operation   
 * @param[in]     pSPIHandle   :  SPI base register pointer
 * @param[in]     data_out     :  buffer pointer to fill the received data 
 * @param[in]     data_in      :  buffer pointer to send the data 
 * @param[in]     num          :  number of bytes to send 
 * @return        SSPI_SUCCESS in case of success or relevant error code is returned
 */
int32_t RSI_SSPI_Transfer(RSI_SSPI_HANDLE_T *pSPIHandle, const void *data_out, void *data_in, uint32_t num)
{
  RSI_SSPI_DRIVER_T *pdrv = (RSI_SSPI_DRIVER_T *)pSPIHandle;

  // Check for presence of call back
  if (pdrv->cbTable[0] != NULL) {
    // call back is called for TX start,DMA can be programmed in the callback
    pdrv->cbTable[0](pSPIHandle, SSPI_EVENT_TX_START, NULL);

    // call back is called for RX start,DMA can be programmed in the callback
    pdrv->cbTable[0](pSPIHandle, SSPI_EVENT_RX_START, NULL);

  } else {
    return ERROR_SSPI_CB_ERROR;
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           RSI_SSPI_HANDLE_T RSI_SSPI_Initialize(void *drv_mem, uint32_t base_address, uint8_t sspi_mode)
 * @brief        This API is initialises the SPI pins and configures the required parameters
 * @param[in]    drv_mem        :  pointer to driver memory
 * @param[in]    base_address   :  base address SPI pins
 * @param[in]    sspi_mode      :  spi mode value. 1 - High speed mode enable 
 * @return       SSPI_SUCCESS in case of successfull initialisation.
 */
RSI_SSPI_HANDLE_T RSI_SSPI_Initialize(void *drv_mem, uint32_t base_address, uint8_t sspi_mode)
{

  RSI_SSPI_DRIVER_T *pspi = NULL;

  //int32_t ret_val;

  // Check if memory is alligned or not
  if (((uint32_t)drv_mem & 0x3) || (drv_mem == NULL)) {
    return NULL;
  }

  // memset the driver memory
  memset((void *)drv_mem, 0, sizeof(RSI_SSPI_DRIVER_T));

  // Assign the driver memory memory  handler
  pspi = (RSI_SSPI_DRIVER_T *)drv_mem;

  pspi->pREGS = (RSI_SSPI_REGS_T *)base_address;

  return (RSI_SSPI_HANDLE_T *)pspi;
}

/*==============================================*/
/**
 * @fn          rsi_error_t RSI_SSPI_UnInitialize()
 * @brief       This API is used to de init the SPI interface  
 * @return      SUCCESS in case of success or failure 
 */
rsi_error_t RSI_SSPI_UnInitialize()
{
  return RSI_OK;
}

/*==============================================*/
/** 
 * @fn         rsi_error_t RSI_SSPI_RegisterCallBacks(RSI_SSPI_HANDLE_T pSPIHandle, void (*psspi_cb)(RSI_SSPI_HANDLE_T, uint32_t, void *))
 * @brief      This API is used to register callbacks .
 * @param[in]  pSPIHandle  :  SPI base register pointer
 * @param[in]  psspi_cb    :  Call back function pointer
 * @return    SUCCESS in case of success or failure 
 */
rsi_error_t RSI_SSPI_RegisterCallBacks(RSI_SSPI_HANDLE_T pSPIHandle,
                                       void (*psspi_cb)(RSI_SSPI_HANDLE_T, uint32_t, void *))
{

  RSI_SSPI_DRIVER_T *pdrv = (RSI_SSPI_DRIVER_T *)pSPIHandle;

  if (psspi_cb == NULL) {
    return ERROR_SSPI_CB_ERROR;
  }
  pdrv->cbTable[0] = psspi_cb;

  return RSI_OK;
}

/*==============================================*/
/** 
 * @fn         void RSI_Enable_Hostspi_Intf_to_M4(void)
 * @brief      This API is used to enable HOST SPI to MCSS which is defaultly enabled for NWP.
 * @return     none
 */
void RSI_Enable_Hostspi_Intf_to_M4(void)
{
  //  Enable SPI access to MCSS
  (*(volatile uint32_t *)(NWPAON_MEM_HOST_ACCESS_CTRL_CLEAR)) = SDIO_HOST_CONTROL_TO_MCU;
  // Indication to the host that bootloading is done
  MCSS_MISC_CFG_HOST_CTRL |= READY_FROM_RCORE;
  // Overrides the Hardware detected Host with  S/W based Host on 1st interface
  MCSS_MISC_CFG_HOST_CTRL |= LOAD_HOST_MODE;
  // Selects the Host(SPI) as Interface
  MCSS_MISC_CFG_HOST_CTRL |= HOST_SEL_AS_SPI;
  // SDIO/SPI registers share the same based address. Based on this bit MCU can access either SDIO register or SPI registers
  MCSS_MISC_CFG_HOST_CTRL &= ~SDIO_SPI_PROG_SEL;
  // This bit is used to indicate if boot mode is enabled
  MISC_CFG_RST_LATCH_STATUS |= BOOT_MODE_EN;
}
