/***************************************************************************/ /**
 * @file
 * @brief
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
#include "sli_wifi_utility.h"
#include "sl_si91x_status.h"
#include "sl_si91x_types.h"
#include "sl_si91x_constants.h"
#include "sl_si91x_spi_constants.h"
#include "sl_si91x_host_interface.h"
#include "sl_si91x_driver.h"
#include "sl_status.h"
#include "sl_additional_status.h"
#include "sl_wifi_constants.h"
#include "sl_constants.h"
#include "sl_rsi_utility.h"
#include <stdint.h>
#include <stddef.h>

// This macro converts a 32-bit value from host to little-endian byte order
#define htole32(x) (x)

// This macro converts a 16-bit value from host to little-endian byte order
#define htole16(x) (x)
#define SECONDS    (1000)

//! @cond Doxygen_Suppress
#define SLI_SPI_VERIFY_STATUS(s)       \
  do {                                 \
    if (s != SL_STATUS_OK) {           \
      sl_si91x_host_spi_cs_deassert(); \
      return s;                        \
    }                                  \
  } while (0)

#ifdef SLI_BIT_32_SUPPORT
#define SLI_C2_READ_WRITE_SIZE SLI_C2RDWR4BYTE
#else
#define SLI_C2_READ_WRITE_SIZE SLI_C2RDWR1BYTE
#endif

// Timeout value for waiting for the start token
#define SLI_START_TOKEN_TIMEOUT (10 * SECONDS)

#ifndef SL_WIFI_BOARD_READY_WAIT_TIME
#define SL_WIFI_BOARD_READY_WAIT_TIME \
  40000 //some scenarios like after firmware upgrade, it will take 40 seconds to boad ready
#endif

sl_status_t sli_verify_device_boot(uint32_t *rom_version);
sl_status_t sli_wifi_select_option(const uint8_t configuration);

/************************************************************************************
 ******************************** Static Functions *********************************
************************************************************************************/
static sl_status_t sli_send_c1c2(uint16_t data)
{
  sl_status_t status;
  uint8_t rx_buffer[2] = { 0, 0 };

  const uint32_t timestamp = sl_si91x_host_get_timestamp();

  do {
    // Send C1/C2 and receive the response in rx_buffer
    status = sl_si91x_host_spi_transfer(&data, rx_buffer, 2);

    // Check if there was an error or if the response indicates success or idle state
    if (status != SL_STATUS_OK || rx_buffer[1] == SLI_SPI_FAIL) {
      return SL_STATUS_FAIL;
    }
    if ((rx_buffer[1] == SLI_SPI_SUCCESS) || (rx_buffer[1] == 0x00)) {
      return SL_STATUS_OK;
    }
    // If a timeout occurs while waiting for a response, return a timeout status
  } while (sl_si91x_host_elapsed_time(timestamp) < 1000);

  return SL_STATUS_TIMEOUT;
}

static sl_status_t sli_wait_start_token(uint32_t timeout)
{
  sl_status_t status = SL_STATUS_OK;
#ifdef SLI_BIT_32_SUPPORT
  // Char to send/receive data in
  uint32_t temp = 0;
#else
  uint8_t temp = 0;
#endif
  // Look for start token
  // Send a character, could be any character, and check the response for a start token
  // If not found within the timeout time, error out
  // Timeout value needs to be passed since context is important
  const uint32_t timestamp = sl_si91x_host_get_timestamp();

  while (temp != SLI_SPI_START_TOKEN) {
    if (sl_si91x_host_elapsed_time(timestamp) > timeout) {
      return SL_STATUS_BUSY;
    }

    // Continuously send/receive data until the start token is found
    status = sl_si91x_host_spi_transfer(NULL, &temp, sizeof(temp));
  }
  return status;
}

static sl_status_t sli_basic_data_transfer(uint16_t c1c2, uint16_t length, void *tx_data, void *rx_data)
{
  sl_status_t status;

  // Send C1/C2 control information
  status = sli_send_c1c2(c1c2);
  SLI_VERIFY_STATUS(status);

  // Send the length of data to be transferred
  status = sl_si91x_host_spi_transfer(&length, NULL, 2);
  SLI_VERIFY_STATUS(status);

  // Wait for start token
  if (rx_data != NULL) {
    status = sli_wait_start_token(SLI_START_TOKEN_TIMEOUT);
    SLI_VERIFY_STATUS(status);
  }

  // Perform the actual SPI data transfer
  status = sl_si91x_host_spi_transfer(tx_data, rx_data, length);
  return status;
}

static sl_status_t sli_packet_read_with_dummy_data(void *rx_data, uint16_t dummy_length, uint16_t total_length)
{
  sl_status_t status;
  uint32_t aligned_len = ((total_length) + 3) & ~3;

  // Send C1/C2 control information for reading with dummy data
  status = sli_send_c1c2(SLI_C1FRMRD16BIT1BYTE | (SLI_C2SPIADDR1BYTE << 8));
  SLI_VERIFY_STATUS(status);

  // Send the aligned length of data to be transferred
  status = sl_si91x_host_spi_transfer(&aligned_len, NULL, 2);
  SLI_VERIFY_STATUS(status);

  // Wait for start token
  status = sli_wait_start_token(SLI_START_TOKEN_TIMEOUT);
  SLI_VERIFY_STATUS(status);

  // Transfer dummy data (if present)
  status = sl_si91x_host_spi_transfer(NULL, NULL, dummy_length);
  SLI_VERIFY_STATUS(status);

  // Read the actual data and store it in rx_data.
  status = sl_si91x_host_spi_transfer(NULL, rx_data, (aligned_len - dummy_length));
  SLI_VERIFY_STATUS(status);

  return status;
}

/************************************************************************************
 ******************************** Public Functions *********************************
************************************************************************************/
sl_status_t sl_si91x_bus_init(void)
{
  uint32_t temp = htole32(SLI_SI91X_INIT_CMD);
  // Create a buffer to receive the response
  uint8_t rx_buffer[4] = { 0, 0, 0, 0 };
  // Create a timestamp to track elapsed time
  uint32_t timestamp;

  timestamp = sl_si91x_host_get_timestamp();
  sl_si91x_host_spi_cs_assert();
  do {
    sl_si91x_host_spi_transfer(&temp, rx_buffer, sizeof(uint32_t));
    // Check if the response indicates success
    if (rx_buffer[3] == SLI_SPI_SUCCESS) {
      sl_si91x_host_spi_cs_deassert();
      return SL_STATUS_OK;
    }
    // Keep looping as long as the time remaining is less than 10 milliseconds.
  } while (sl_si91x_host_elapsed_time(timestamp) < 10);
  sl_si91x_host_spi_cs_deassert();

  // If the initialization did not succeed within the timeout, return SPI busy status
  return SL_STATUS_SPI_BUSY;
}

sl_status_t sl_si91x_bus_write_memory(uint32_t addr, uint16_t length, const uint8_t *buffer)
{
  uint32_t temp32;
  uint32_t temp16;
  sl_status_t status;

  sl_si91x_host_spi_cs_assert();
  // Send C1/C2 control information to initiate the memory write operation
  status = sli_send_c1c2(SLI_C1MEMWR16BIT4BYTE | (SLI_C2_READ_WRITE_SIZE << 8));
  SLI_SPI_VERIFY_STATUS(status);

  // Send C3/C4 control information with the length of data to be written
  temp16 = htole16(length);
  status = sl_si91x_host_spi_transfer(&temp16, NULL, sizeof(uint16_t));
  SLI_SPI_VERIFY_STATUS(status);

  // Send the 4-byte memory address
  temp32 = htole32(addr);
  status = sl_si91x_host_spi_transfer(&temp32, NULL, sizeof(uint32_t));
  SLI_SPI_VERIFY_STATUS(status);

  // Send the data
  status = sl_si91x_host_spi_transfer(buffer, NULL, length);

  sl_si91x_host_spi_cs_deassert();
  return status;
}

sl_status_t sli_si91x_bus_write_slave(uint32_t data_length, const uint8_t *buffer)
{
  uint32_t temp32;
  uint32_t temp16;
  sl_status_t status;

  sl_si91x_host_spi_cs_assert();
  // Send C1/C2 control information to initiate the memory write operation
  status = sli_send_c1c2(SLI_C1FRMWR16BIT4BYTE | (SLI_C2_READ_WRITE_SIZE << 8));
  SLI_VERIFY_STATUS(status);

  // Send C3/C4 control information with the length of data to be written. 16 extra bytes are expected by SPI.
  temp16 = htole16(data_length + 16);
  status = sl_si91x_host_spi_transfer(&temp16, NULL, sizeof(uint16_t));
  SLI_SPI_VERIFY_STATUS(status);

  // Send the 4-byte actual data length
  temp32 = htole32(data_length);
  status = sl_si91x_host_spi_transfer(&temp32, NULL, sizeof(uint32_t));
  SLI_SPI_VERIFY_STATUS(status);

  // Send the data
  // Si917 Bootloader receives the data in DMA mode with descriptor where the first
  // descriptor expects dummy 12 bytes
  status = sl_si91x_host_spi_transfer(buffer, NULL, 12);
  SLI_SPI_VERIFY_STATUS(status);
  if (data_length > 1024) {
    // SPI master cannot send more than 1024 bytes. So, the data is split.
    for (unsigned int i = 0; i < (data_length / 1024); i++) {
      status = sl_si91x_host_spi_transfer(&buffer[(i * 1024)], NULL, 1024);
      SLI_SPI_VERIFY_STATUS(status);
    }
  } else {
    status = sl_si91x_host_spi_transfer(buffer, NULL, 1024);
    SLI_SPI_VERIFY_STATUS(status);
  }

  sl_si91x_host_spi_cs_deassert();
  return status;
}

sl_status_t sl_si91x_bus_read_memory(uint32_t addr, uint16_t length, const uint8_t *buffer)
{
  //  uint8_t rx_buffer[4];
  sl_status_t status;
  uint32_t temp32;
  uint16_t temp16;

  sl_si91x_host_spi_cs_assert();
  // Send C1/C2 control information to initiate the memory read operation
  status = sli_send_c1c2(SLI_C1MEMRD16BIT4BYTE | (SLI_C2_READ_WRITE_SIZE << 8));
  SLI_SPI_VERIFY_STATUS(status);

  // Send C3/C4 control information with the length of data to be read
  temp16 = htole16(length);
  status = sl_si91x_host_spi_transfer(&temp16, NULL, 2);
  SLI_SPI_VERIFY_STATUS(status);

  // Send the 4-byte memory address
  temp32 = htole32(addr);
  status = sl_si91x_host_spi_transfer(&temp32, NULL, sizeof(uint32_t));
  SLI_SPI_VERIFY_STATUS(status);

  // Wait for the start token
  status = sli_wait_start_token(10 * SECONDS);
  SLI_SPI_VERIFY_STATUS(status);

  // Read in the memory data
  status = sl_si91x_host_spi_transfer(NULL, (uint8_t *)buffer, length);

  sl_si91x_host_spi_cs_deassert();
  return status;
}

// address is 6-bits long, upper two bits must be cleared for byte mode
sl_status_t sli_si91x_bus_write_register(uint8_t address, uint8_t register_size, uint16_t data)
{
  sl_status_t status;

  sl_si91x_host_spi_cs_assert();
  // Send C1/C2 control information with register address and write operation
  status = sli_send_c1c2(SLI_C1INTWRITE2BYTES | (address << 8));
  SLI_SPI_VERIFY_STATUS(status);

  // Send the data to be written to the register
  status = sl_si91x_host_spi_transfer(&data, NULL, register_size);

  sl_si91x_host_spi_cs_deassert();
  return status;
}

// Reading data from a specific register address on the SI91x bus
// register_size must be 1 or 2
sl_status_t sli_si91x_bus_read_register(uint8_t address, uint8_t register_size, uint16_t *output)
{
  sl_status_t status;
  uint16_t temp16 = (register_size == 1 ? SLI_C1INTREAD1BYTES : SLI_C1INTREAD2BYTES) | (address << 8);

  //  SL_ASSERT(register_size == 1 || register_size == 2);

  sl_si91x_host_spi_cs_assert();
  // Send C1C2
  status = sli_send_c1c2(temp16);
  SLI_SPI_VERIFY_STATUS(status);

  // Wait for start token
  status = sli_wait_start_token(10 * SECONDS);
  SLI_SPI_VERIFY_STATUS(status);

  // Start token found now read the byte/s of data
  status = sl_si91x_host_spi_transfer(NULL, output, register_size);

  sl_si91x_host_spi_cs_deassert();
  return status;
}

sl_status_t sli_si91x_bus_write_frame(sl_wifi_system_packet_t *packet, const uint8_t *payloadparam, uint16_t size_param)
{
  UNUSED_PARAMETER(payloadparam); // Unused parameter(to suppress compiler warnings)
  sl_status_t status;

  sl_si91x_host_spi_cs_assert();
#ifdef DEBUG_PACKET_EXCHANGE
  // If debugging is enabled, log debugging information about the frame
  memset(debug_output, '\0', sizeof(debug_output));
  sprintf(debug_output, "[WR(%d)]", rsi_hal_gettickcount());
  for (uint32_t i = 0; i < 16; i++) {
    sprintf(debug_output, "%s%.2x", debug_output, ((char *)(uFrameDscFrame))[i]);
  }
  if (size_param) {
    sprintf(debug_output, "%s | ", debug_output);
    // Print max of 8 bytes of payload
    for (uint32_t i = 0; i < size_param && i < MAX_PRINT_PAYLOAD_LEN; i++) {
      sprintf(debug_output, "%s%.2x", debug_output, payloadparam[i]);
    }
  }
  LOG_PRINT("%s\r\n", debug_output);
#endif

#ifdef RSI_CHIP_MFG_EN
  // If a specific chip manufacturing feature is enabled, adjust the size_param
  size_param += RSI_HOST_DESC_LENGTH;
  status = sli_basic_data_transfer(c1c2, size_param, uFrameDscFrame, NULL);
  sl_si91x_host_spi_cs_deassert();
  return status;
#endif

  // Write host descriptor
  status = sli_basic_data_transfer(SLI_C1FRMWR16BIT4BYTE | (SLI_C2_READ_WRITE_SIZE << 8),
                                   SLI_FRAME_DESC_LEN,
                                   &packet->desc,
                                   NULL);
  SLI_SPI_VERIFY_STATUS(status);

  // Write payload if present
  if (size_param) {
    // 4 byte align for payload size
    size_param = (size_param + 3) & ~3;
    status =
      sli_basic_data_transfer(SLI_C1FRMWR16BIT4BYTE | (SLI_C2_READ_WRITE_SIZE << 8), size_param, &packet->data, NULL);
    SLI_SPI_VERIFY_STATUS(status);
  }

  sl_si91x_host_spi_cs_deassert();
  return status;
}

sl_status_t sli_si91x_bus_read_frame(sl_wifi_buffer_t **buffer)
{
  sl_status_t status;
  uint16_t local_buffer[2];
  uint8_t *data;
  uint16_t temp;

  sl_si91x_host_spi_cs_assert();
#ifndef RSI_CHIP_MFG_EN
  // Read the first 4 bytes to determine the frame size
  status = sli_basic_data_transfer(SLI_C1FRMRD16BIT4BYTE | (SLI_C2_READ_WRITE_SIZE << 8), 4, NULL, &local_buffer);
  SLI_SPI_VERIFY_STATUS(status);

  // Round up total size (local_buffer[0]) to 4 bytes
  local_buffer[0] = (htole16(local_buffer[0]) - 4 + 3) & ~3;
  local_buffer[1] = htole16(local_buffer[1]) - 4;

  // Allocate a buffer for the frame using sli_si91x_host_allocate_buffer
  status = sli_si91x_host_allocate_buffer(buffer, SL_WIFI_RX_FRAME_BUFFER, local_buffer[0], 10000);
  if (status != SL_STATUS_OK) {
    sl_si91x_host_spi_cs_deassert();
    SL_DEBUG_LOG("\r\n HEAP EXHAUSTED DURING ALLOCATION \r\n");
    sli_command_engine_status_queue_enqueue_and_set_event(SL_STATUS_ALLOCATION_FAILED);
    return SL_STATUS_ALLOCATION_FAILED;
  }

  data = (uint8_t *)sli_wifi_host_get_buffer_data(*buffer, 0, &temp);

  // Read complete RX packet
  if (local_buffer[1] == 0) {
    status = sli_basic_data_transfer(SLI_C1FRMRD16BIT1BYTE | (SLI_C2SPIADDR1BYTE << 8), local_buffer[0], NULL, data);
  } else {
    status = sli_packet_read_with_dummy_data(data, local_buffer[1], local_buffer[0]);
  }
  sl_si91x_host_spi_cs_deassert();
  return status;

#else
  // Read first 4 bytes
  retval = rsi_spi_pkt_len(&local_buffer[0]);
  if (retval != 0x00) {
    sl_si91x_host_spi_cs_deassert();
    return retval;
  }
  retval = rsi_nlink_pkt_rd(pkt_buffer, rsi_bytes2R_to_uint16(&local_buffer[0]) & 0xFFF);
  if (retval != 0x00) {
    sl_si91x_host_spi_cs_deassert();
    return retval;
  }
  sl_si91x_host_spi_cs_deassert();
#endif
}

// Function for reading the interrupt status
sl_status_t sli_si91x_bus_read_interrupt_status(uint16_t *interrupt_status)
{
  sl_status_t status;
  uint32_t timestamp = sl_si91x_host_get_timestamp();

  do {
    // Read the interrupt register
    status = sli_si91x_bus_read_register(SLI_SPI_INT_REG_ADDR, 1, interrupt_status);
    if (status != SL_STATUS_BUSY) {
      return status;
    }
    // Keep looping while the elapsed time is less than 1000 milliseconds
  } while (sl_si91x_host_elapsed_time(timestamp) < 1000);

  // If the interrupt status cannot be read within the timeout, return a timeout
  return SL_STATUS_TIMEOUT;
}

sl_status_t sli_si91x_bootup_firmware(const uint8_t select_option, uint8_t image_number)
{
  UNUSED_PARAMETER(image_number); // Unused parameter(to suppress compiler warnings)
  uint32_t rom_version;
  uint32_t timestamp;
  sl_status_t status = SL_STATUS_FAIL;
  //const uint8_t select_option = config->boot_option; //LOAD_NWP_FW;
  timestamp = sl_si91x_host_get_timestamp();
  do {
    if (sl_si91x_host_elapsed_time(timestamp) > SL_WIFI_BOARD_READY_WAIT_TIME) {
      return SL_STATUS_TIMEOUT;
    }
    status = sli_verify_device_boot(&rom_version);
  } while (status == SL_STATUS_WAITING_FOR_BOARD_READY || status == SL_STATUS_SPI_BUSY);
  VERIFY_STATUS_AND_RETURN(status);

  // selecting bootloader options
  if (select_option == LOAD_NWP_FW) {
    status = sli_si91x_bus_set_interrupt_mask(SLI_ACTIVE_HIGH_INTR);
  } else if (select_option == LOAD_DEFAULT_NWP_FW_ACTIVE_LOW) {
    status = sli_si91x_bus_set_interrupt_mask(SLI_ACTIVE_LOW_INTR);
  }
  VERIFY_STATUS_AND_RETURN(status);

#if SL_SI91X_FAST_FW_UP
  status = sl_si91x_set_fast_fw_up();
  VERIFY_STATUS_AND_RETURN(status);
#endif

  // Select boot option
  status = sli_wifi_select_option(select_option);
  VERIFY_STATUS_AND_RETURN(status);
  return status;
}

sl_status_t sli_si91x_bus_rx_irq_handler(void)
{
  sli_wifi_set_event(SL_SI91X_NCP_HOST_BUS_RX_EVENT);
  return SL_STATUS_OK;
}

void sli_si91x_bus_rx_done_handler(void)
{
  return;
}

//! Initialize with modules Slave SPI interface on ulp wakeup.
void sli_si91x_ulp_wakeup_init(void)
{
  uint8_t txCmd[4];
  uint8_t rxbuff[2];

  // Prepare the initialization command
  txCmd[0] = (SLI_SI91X_INIT_CMD & 0xFF);
  txCmd[1] = ((SLI_SI91X_INIT_CMD >> 8) & 0xFF);
  txCmd[2] = ((SLI_SI91X_INIT_CMD >> 16) & 0xFF);
  txCmd[3] = ((SLI_SI91X_INIT_CMD >> 24) & 0xFF);
  sl_si91x_host_spi_cs_assert();

  while (1) {
    // Transfer the initialization command to the SI91x module and check the response
    sl_si91x_host_spi_transfer(txCmd, rxbuff, 2);

    if (rxbuff[1] == SLI_SPI_FAIL) {
      break; // Initialization failed, exit the loop
    } else if (rxbuff[1] == 0x00) {
      // If the response indicates success, transfer the remaining part of the command
      sl_si91x_host_spi_transfer(&txCmd[2], rxbuff, 2);
      if (rxbuff[1] == SLI_SPI_SUCCESS) {
        break; // Initialization succeeded, exit the loop
      }
    }
  }
  sl_si91x_host_spi_cs_deassert();
}
