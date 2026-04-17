/***************************************************************************/ /**
 * @file
 * @brief Firmware flashing Example Application
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_net.h"
#include "cmsis_os2.h"
#include "sl_wifi_callback_framework.h"
#include "string.h"
#include "em_gpio.h"
#include "sl_board_configuration.h"
#include "em_cmu.h"
#include "em_eusart.h"
#include "sl_wifi.h"
#include "sl_si91x_driver.h"
#include "em_msc.h"
#include "sl_constants.h"
/******************************************************
 *               Variable Definitions
 ******************************************************/
#define XMODEM_SOH       1
#define XMODEM_EOT       4
#define XMODEM_ACK       6
#define XMODEM_NAK       21
#define XMODEM_CAN       24
#define XMODEM_NCG       67
#define XMODEM_DATA_SIZE 128
#define XMODEM_UART      EUSART0

#define SI91X_CHUNK_SIZE      4096UL
#define SI91X_IN_BETWEEN_FILE 0UL
#define SI91X_START_OF_FILE   1UL
#define SI91X_END_OF_FILE     2UL
#define SI91X_FW_VER_SIZE     20UL
#define FW_HEADER_SIZE        64UL
#define XMODEM_CHUNK_SIZE     128UL
#define FIRST_PKT_XMODEM_CNT  32UL

//! Type of FW update
#define M4_FW_UPDATE  0 // Only Supported for SoC
#define NWP_FW_UPDATE 1

//! Set FW update type
#define FW_UPDATE_TYPE NWP_FW_UPDATE

typedef struct {
  uint8_t padding; /* Padding to make sure data is 32 bit aligned. */
  uint8_t header;
  uint8_t packetNumber;
  uint8_t packetNumberC;
  uint8_t data[XMODEM_DATA_SIZE];
  uint8_t crcHigh;
  uint8_t crcLow;
} XMODEM_packet;

const osThreadAttr_t thread_attributes = {
  .name       = "app",
  .attr_bits  = 0,
  .cb_mem     = 0,
  .cb_size    = 0,
  .stack_mem  = 0,
  .stack_size = 3072,
  .priority   = osPriorityLow,
  .tz_module  = 0,
  .reserved   = 0,
};

//! Enumeration for states in application
typedef enum si91x_wlan_app_state_e {
  SI91X_WLAN_INITIAL_STATE    = 0,
  SI91X_WLAN_RADIO_INIT_STATE = 1,
  SI91X_WLAN_FW_UPGRADE       = 2,
  SI91X_WLAN_FW_UPGRADE_DONE  = 3
} si91x_wlan_app_state_t;

//! wlan application control block
typedef struct si91x_wlan_app_cb_s {
  //! wlan application state
  si91x_wlan_app_state_t state;

  //! length of buffer to copy
  uint32_t length;

  //! to check application buffer availability
  uint8_t buf_in_use;

  //! application events bit map
  uint32_t event_map;

} si91x_wlan_app_cb_t;

//! Firmware up request structure
typedef struct fwupeq_s {
  uint16_t control_flags;
  uint16_t sha_type;
  uint32_t magic_no;
  uint32_t image_size;
  uint32_t fw_version;
  uint32_t flash_loc;
  uint32_t crc;
} fwreq_t;

static const sl_wifi_device_configuration_t firmware_update_configuration = {
#if (FW_UPDATE_TYPE == NWP_FW_UPDATE)
  .boot_option = BURN_NWP_FW,
#else
  .boot_option = BURN_M4_FW,
#endif
  .mac_address = NULL,
  .band        = SL_SI91X_WIFI_BAND_2_4GHZ,
  .region_code = US,
  .boot_config = { .oper_mode              = SL_SI91X_CLIENT_MODE,
                   .coex_mode              = SL_SI91X_WLAN_ONLY_MODE,
                   .feature_bit_map        = (SL_WIFI_FEAT_SECURITY_PSK | SL_WIFI_FEAT_AGGREGATION),
                   .tcp_ip_feature_bit_map = (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT),
                   .custom_feature_bit_map = (SL_WIFI_SYSTEM_CUSTOM_FEAT_EXTENSION_VALID),
                   .ext_custom_feature_bit_map =
                     (SL_SI91X_EXT_FEAT_XTAL_CLK | SL_SI91X_EXT_FEAT_UART_SEL_FOR_DEBUG_PRINTS | MEMORY_CONFIG
#ifdef SLI_SI917
                      | SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif
                      ),
                   .bt_feature_bit_map         = 0,
                   .ext_tcp_ip_feature_bit_map = SL_SI91X_CONFIG_FEAT_EXTENTION_VALID,
                   .ble_feature_bit_map        = 0,
                   .ble_ext_feature_bit_map    = 0,
                   .config_feature_bit_map     = 0 }
};

extern uint8_t recv_buffer[];
extern volatile uint32_t remaining_bytes;
extern volatile uint8_t xmodem_download;
uint8_t rawPacket[2][sizeof(XMODEM_packet)] = { 0 };
volatile uint8_t xmodem_download            = 0;
si91x_wlan_app_cb_t si91x_wlan_app_cb;

uint32_t chunk_cnt = 0u, chunk_check = 0u, offset = 0u, fw_image_size = 0u;
int32_t status                        = SL_STATUS_OK;
uint8_t recv_buffer[SI91X_CHUNK_SIZE] = { 0 };
sl_wifi_firmware_version_t fw_version = { 0 };
uint8_t one_time                      = 1;
volatile uint32_t offset_xmodem       = 0u;
volatile uint32_t remaining_bytes     = 0u;
uint32_t xmodem_chunk_cnt             = 0;
uint32_t xmodem_chunk_rem             = 0;

uint32_t t_start   = 0;
uint32_t t_end     = 0;
uint32_t xfer_time = 0;
float secs         = 0;

/******************************************************
 *               Function Declarations
 ******************************************************/
void usart_init(void);
void xmodem_data(uint8_t *data, uint32_t size);
void XMODEM_download(uint32_t baseAddress, uint32_t endAddress);
static void application_start(void *argument);
int32_t app_task_fw_update_via_xmodem(uint8_t *rx_data, uint32_t size);

/******************************************************
 *               Function Definitions
 ******************************************************/

void app_init(void)
{
  osThreadNew((osThreadFunc_t)application_start, NULL, &thread_attributes);
}

static void application_start(void *argument)
{
  UNUSED_PARAMETER(argument);
  sl_status_t status;

  status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, &firmware_update_configuration, NULL, NULL);
  if (status == SL_STATUS_OK) {
    printf("Wi-Fi initialized successfully for firmware update.\n");
  } else {
    printf("Wi-Fi init failed with error: 0x%lX\n", status);
    return;
  }

  usart_init();

  while (1) {
    //xmodem call
    if (xmodem_download == 0) {
      XMODEM_download(0, 0);
    } else {
      // print the firmware version
      app_task_fw_update_via_xmodem(NULL, 0);
    }
  }

  while (1) {
#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)
    // Let the CPU go to sleep if the system allows it.
    sl_power_manager_sleep();
#else
    osDelay(osWaitForever);
#endif
  }
}

void usart_init(void)
{
  // Default asynchronous initializer (8N1, no flow control)
  EUSART_UartInit_TypeDef init = EUSART_UART_INIT_DEFAULT_HF;

  // Setting baudrate = 0 enables autobaud detection
  init.baudrate = 115200;

  // Enable oscillator to NCP USART module
  CMU_ClockEnable(cmuClock_EUSART0, true);

  // UART Rx
  GPIO_PinModeSet(NCP_UART_RX_PIN.port, NCP_UART_RX_PIN.pin, gpioModeInput, 0);
  // UART Tx
  GPIO_PinModeSet(NCP_UART_TX_PIN.port, NCP_UART_TX_PIN.pin, gpioModePushPull, 1);

  // Initialize EUART asynchronous mode and route pins
  EUSART_UartInitHf(XMODEM_UART, &init);

  GPIO->EUSARTROUTE[0].RXROUTE = (NCP_UART_RX_PIN.port << _GPIO_USART_RXROUTE_PORT_SHIFT)
                                 | (NCP_UART_RX_PIN.pin << _GPIO_USART_RXROUTE_PIN_SHIFT);
  GPIO->EUSARTROUTE[0].TXROUTE = (NCP_UART_TX_PIN.port << _GPIO_USART_TXROUTE_PORT_SHIFT)
                                 | (NCP_UART_TX_PIN.pin << _GPIO_USART_TXROUTE_PIN_SHIFT);

  // Enable USART interface pins
  GPIO->EUSARTROUTE[0].ROUTEEN = GPIO_USART_ROUTEEN_RXPEN | GPIO_USART_ROUTEEN_TXPEN;
}

/*******************************************************************************
 * Name   : get_fw_size
 * Brief  : This function gets the size of the firmware
 * return : firmware size
 * @return
 *******************************************************************************/
uint32_t get_fw_size(char *buffer)
{
  fwreq_t *fw = (fwreq_t *)buffer;
  return fw->image_size;
}

/*******************************************************************************
 * Name   : xmodem_data
 * Brief  : its combine 128 bytes packet into 4096 bytes packet.
 * return : none
 * @return
 *******************************************************************************/
void xmodem_data(uint8_t *data, uint32_t size)
{
  static uint32_t xmodem_cnt = 0;

  memcpy(&recv_buffer[offset_xmodem], data, size);
  offset_xmodem = offset_xmodem + size;

  xmodem_cnt++;
  // send pkt to boot loader in 3 conditions
  //1: 1st pkt
  //2: if packet size is 4096 byte
  //3: last pkt
  if (((((xmodem_cnt % 32) == 0) && (xmodem_cnt <= xmodem_chunk_cnt)) || (xmodem_cnt == xmodem_chunk_cnt))
      || (xmodem_cnt == FIRST_PKT_XMODEM_CNT)) {
    if ((xmodem_cnt == xmodem_chunk_cnt)) {
      memset(&recv_buffer[remaining_bytes + FW_HEADER_SIZE], 0, (SI91X_CHUNK_SIZE - remaining_bytes - FW_HEADER_SIZE));
    }

    app_task_fw_update_via_xmodem(recv_buffer, SI91X_CHUNK_SIZE);

    remaining_bytes = remaining_bytes - SI91X_CHUNK_SIZE;
    offset_xmodem   = 0u;
  }
}
/*******************************************************************************
 * Name   : app_task_fw_update_via_xmodem
 * Brief  : Its sends the firmware image packet chunk by chunk to the device.
 * return : -1 if error
 * @return
 *******************************************************************************/
int32_t app_task_fw_update_via_xmodem(uint8_t *rx_data, uint32_t size)
{
  UNUSED_PARAMETER(size);
  switch (si91x_wlan_app_cb.state) {
    case SI91X_WLAN_INITIAL_STATE:
    case SI91X_WLAN_FW_UPGRADE: {

      if (one_time == 1) {
        printf("DEMO STARTED\n");
        //! Send the first chunk to extract header
        fw_image_size = get_fw_size((char *)rx_data);

        remaining_bytes = fw_image_size; // lask pkt set the data via XMODEM

        xmodem_chunk_rem = ((remaining_bytes + FW_HEADER_SIZE) % XMODEM_CHUNK_SIZE);
        xmodem_chunk_cnt = ((remaining_bytes + FW_HEADER_SIZE) / XMODEM_CHUNK_SIZE);

        if (xmodem_chunk_rem != 0) {
          xmodem_chunk_cnt++;
        }

        //!caluculate the total number of chunks
        chunk_check = (fw_image_size / SI91X_CHUNK_SIZE);
        if (fw_image_size % SI91X_CHUNK_SIZE) {
          chunk_check += 1;
        }

        one_time = 0;
      }

      if (offset <= fw_image_size) {
        if (chunk_cnt == chunk_check) {
          printf("fw_upgrade Start\n");
          break;
        }
        if (chunk_cnt != 0) {
          printf("chunk_cnt: %lu\n", chunk_cnt);
        }
        if (chunk_cnt == 0) {
          printf("fw_upgrade Start\n");
          status = sl_si91x_bl_upgrade_firmware((uint8_t *)rx_data, SI91X_CHUNK_SIZE, SI91X_START_OF_FILE);
          if (status != SL_STATUS_OK) {
            printf("ERROR: 0x%lx\n", status);
            break;
          }
        } else if (chunk_cnt == (chunk_check - 1)) {
          status = sl_si91x_bl_upgrade_firmware((uint8_t *)rx_data, SI91X_CHUNK_SIZE, SI91X_END_OF_FILE);
          if (status != SL_STATUS_OK) {
            printf("ERROR: 0x%lx\n", status);
            break;
          }
          printf("\r\nfw_upgrade Success\n");
          si91x_wlan_app_cb.state = SI91X_WLAN_FW_UPGRADE_DONE;
        } else {
          status = sl_si91x_bl_upgrade_firmware((uint8_t *)rx_data, SI91X_CHUNK_SIZE, SI91X_IN_BETWEEN_FILE);
          if (status != SL_STATUS_OK) {
            printf("ERROR: 0x%lx\n", status);
            break;
          }
        }
        offset += SI91X_CHUNK_SIZE;
        memset(recv_buffer, 0, sizeof(recv_buffer));
        chunk_cnt++;
      }
    } break;
    case SI91X_WLAN_FW_UPGRADE_DONE: {

      //! De-initialize the client network interface before reinitializing
      status = sl_net_deinit(SL_NET_WIFI_CLIENT_INTERFACE);
      if (status == SL_STATUS_OK) {
        printf("Wi-Fi deinitialized successfully after firmware upgrade.\n");
      } else {
        printf("Wi-Fi deinit failed with error: 0x%lX\n", status);
        return status;
      }
      //! WiSeConnect initialization
      status = sl_net_init(SL_NET_WIFI_CLIENT_INTERFACE, NULL, NULL, NULL);
      if (status == SL_STATUS_OK) {
        printf("Wi-Fi initialized successfully after firmware upgrade.\n");
      } else {
        printf("Wi-Fi init failed with error: 0x%lX\n", status);
        return status;
      }

      status = sl_wifi_get_firmware_version(&fw_version);
      if (status != SL_STATUS_OK) {
        printf("reading fw version failed\n");
        break;
      }
      printf("fw version after upgrade is:");
      print_firmware_version(&fw_version);
      t_end     = osKernelGetTickCount();
      xfer_time = t_end - t_start;
      secs      = xfer_time / 1000;
      printf("\r\nTime taken for firmware flashing:%d secs\r\n", (int)secs);
      printf("\r\nDEMO DONE\r\n");
      while (1)
        ;
    } break;
    default:
      break;
  }
  return status;
}
/**************************************************************************/ /**
 * @brief
 *   This function calculates the CRC-16-CCIT checksum of a memory range.
 *
 * @note
 *   This implementation uses an initial value of 0, while some implementations
 *   of CRC-16-CCIT uses an initial value of 0xFFFF. If you wish to
 *   precalculate the CRC before uploading the binary to the bootloader you
 *   can use this function. However, keep in mind that the 'v' and 'c' commands
 *   computes the crc of the entire flash, so any bytes not used by your
 *   application will have the value 0xFF.
 *
 * @param start
 *   Pointer to the start of the memory block
 *
 * @param end
 *   Pointer to the end of the block. This byte is not included in the computed
 *   CRC.
 *
 * @return
 *   The computed CRC value.
 *****************************************************************************/
RAMFUNC_DECLARATOR uint16_t CRC_calc(uint8_t *start, uint8_t *end)
{
  uint16_t crc = 0x0;
  uint8_t *data;

  for (data = start; data < end; data++) {
    crc = (crc >> 8) | (crc << 8);
    crc ^= *data;
    crc ^= (crc & 0xff) >> 4;
    crc ^= crc << 12;
    crc ^= (crc & 0xff) << 5;
  }
  return crc;
}
/**************************************************************************/ /**
 * @brief Verifies checksum, packet numbering and CRC of a XMODEM packet.
 *
 * @param pkt The packet to verify
 * @param sequenceNumber The current sequence number.
 * @returns -1 on packet error, 0 otherwise
 *****************************************************************************/
int XMODEM_verifyPacketChecksum(XMODEM_packet *pkt, int sequenceNumber)
{
  uint16_t packetCRC     = 0u;
  uint16_t calculatedCRC = 0u;

  // Check the packet number integrity.
  if (pkt->packetNumber + pkt->packetNumberC != 255) {
    return -1;
  }

  // Check that the packet number matches the excpected number.
  if (pkt->packetNumber != (sequenceNumber % 256)) {
    return -1;
  }

  calculatedCRC = CRC_calc((uint8_t *)pkt->data, (uint8_t *)&(pkt->crcHigh));
  packetCRC     = pkt->crcHigh << 8 | pkt->crcLow;

  // Check the CRC value.
  if (calculatedCRC != packetCRC) {
    return -1;
  }
  return 0;
}

/**************************************************************************/ /**
 * @brief Starts a XMODEM download.
 *
 * @param baseAddress
 *   The address to start writing from
 *
 * @param endAddress
 *   The last address. This is only used for clearing the flash
 *****************************************************************************/
//RAMFUNC_DECLARATOR void XMODEM_download(uint32_t baseAddress, uint32_t endAddress)
void XMODEM_download(uint32_t baseAddress, uint32_t endAddress)
{
  UNUSED_PARAMETER(baseAddress);
  UNUSED_PARAMETER(endAddress);
  XMODEM_packet *pkt      = NULL;
  uint32_t byte           = 0u;
  uint32_t sequenceNumber = 1;

  // Send one start transmission packet. Wait for a response. If there is no
  // response, we resend the start transmission packet.
  // Note: This is a fairly long delay between retransmissions(~6 s).
  EUSART_Tx(XMODEM_UART, XMODEM_NCG);
  t_start = osKernelGetTickCount();
  while (1) {
    // Swap buffer for packet buffer.
    pkt = (XMODEM_packet *)rawPacket[sequenceNumber & 1];

    // Fetch the first byte of the packet explicitly, as it defines the
    // rest of the packet.
    pkt->header = EUSART_Rx(XMODEM_UART);

    // Check for end of transfer.
    if (pkt->header == XMODEM_EOT) {
      // Acknowledget End of transfer.
      EUSART_Tx(XMODEM_UART, XMODEM_ACK);

      break;
    }

    // If the header is not a start of header (SOH), then cancel
    // the transfer.
    if (pkt->header != XMODEM_SOH) {
      return;
    }

    // Fill the remaining bytes packet.
    // Byte 0 is padding, byte 1 is header.
    for (byte = 2; byte < sizeof(XMODEM_packet); byte++) {
      *(((uint8_t *)pkt) + byte) = EUSART_Rx(XMODEM_UART);
    }
    if (XMODEM_verifyPacketChecksum(pkt, sequenceNumber) != 0) {
      // On a malformed packet, we send a NAK, and start over.
      EUSART_Tx(XMODEM_UART, XMODEM_NAK);
      continue;
    }
    //send data to bootloader
    xmodem_data((uint8_t *)pkt->data, XMODEM_DATA_SIZE);

    sequenceNumber++;
    // Send ACK.
    EUSART_Tx(XMODEM_UART, XMODEM_ACK);

    //clear the pkt data buffer
    memset(pkt->data, 0, XMODEM_DATA_SIZE);
  }
  // set flag for xmodem download is done
  xmodem_download = 1;
}
