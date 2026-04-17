/***************************************************************************//**
 * @file sl_rail_sdk_packet_assistant.c
 * @brief RAIL SDK - RAIL Packet Assistant Component
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

// -----------------------------------------------------------------------------
//                                   Includes
// -----------------------------------------------------------------------------
#include "sl_rail_sdk_packet_assistant.h"
#include "sl_rail_sdk_packet_assistant_config.h"
#include "sl_component_catalog.h"

#if defined(SL_CATALOG_APP_LOG_PRESENT)
#include "app_log.h"
#endif

// -----------------------------------------------------------------------------
//                              Macros and Typedefs
// -----------------------------------------------------------------------------
typedef uint16_t (*unpack_packet_func_t)(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
typedef void (*prepare_packet_func_t)(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
// -----------------------------------------------------------------------------
//                          Static Function Declarations
// -----------------------------------------------------------------------------
uint16_t unpack_packet_wisun_fsk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_wisun_fsk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
uint16_t unpack_packet_wisun_ofdm(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_wisun_ofdm(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
uint16_t unpack_packet_sun_oqpsk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_sun_oqpsk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
uint16_t unpack_packet_sidewalk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_sidewalk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
uint16_t unpack_packet_longrange(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_longrange(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
uint16_t unpack_packet_bpsk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_bpsk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);
uint16_t unpack_packet_base(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload);
void prepare_packet_base(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length);

// -----------------------------------------------------------------------------
//                                Global Variables
// -----------------------------------------------------------------------------
unpack_packet_func_t unpack_packet_fnc = NULL;
prepare_packet_func_t prepare_packet_fnc = NULL;
// -----------------------------------------------------------------------------
//                                Static Variables
// -----------------------------------------------------------------------------
static uint8_t print_packet_info = PRINT_PACKET_INFO;

static uint8_t wisun_fsk_fcs = WISUN_FSK_FCS_TYPE;
static uint8_t wisun_fsk_whitening = WISUN_FSK_WHITENING;
static uint8_t wisun_ofdm_rate = WISUN_OFDM_RATE;
// rate: 5 bits wide, The Rate field (RA4-RA0) specifies the data rate of the payload and is equal to the numerical value of the MCS
// 0x0 BPSK, coding rate 1/2, 4 x frequency repetition
// 0x1 BPSK, coding rate 1/2, 2 x frequency repetition
// 0x2 QPSK, coding rate 1/2, 2 x frequency repetition
// 0x3 QPSK, coding rate 1/2
// 0x4 QPSK, coding rate 3/4
// 0x5 16-QAM, coding rate 1/2
// 0x6 16-QAM, coding rate 3/4
static uint8_t wisun_ofdm_scrambler = WISUN_OFDM_SCRAMBLER; // scrambler: 2 bits wide, The Scrambler field (S1-S0) specifies the scrambling seed
static bool sun_oqpsk_spreading_mode = (bool)SUN_OQPSK_SPREADINGMODE;
static uint8_t sun_oqpsk_rate_mode = SUN_OQPSK_RATEMODE; // rateMode: 2 bits wide
static uint8_t sidewalk_fcs_type = SIDEWALK_FSK_FCS_TYPE;
static uint8_t sidewalk_whitening = SIDEWALK_FSK_WHITENING;

#if defined(SL_CATALOG_APP_LOG_PRESENT)
static RAIL_SDK_Protocol_t current_protocol = UNDEFINED;
#endif

// -----------------------------------------------------------------------------
//                          Public Function Definitions
// -----------------------------------------------------------------------------
void prepare_packet(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  if (prepare_packet_fnc == NULL) {
    update_assistant_pointers(0);
  }
  prepare_packet_fnc(rail_handle, out_data, length);
}

uint16_t unpack_packet(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  if (unpack_packet_fnc == NULL) {
    update_assistant_pointers(0);
  }
  return unpack_packet_fnc(rail_handle, rx_destination, packet_information, start_of_payload);
}

void update_assistant_pointers(uint8_t new_phy_index)
{
  static uint8_t stack_info[2] = { 0U, 0U };
  if (channelConfigs[new_phy_index]->configs[0].stackInfo != NULL) {
    stack_info[0] = channelConfigs[new_phy_index]->configs[0].stackInfo[0];
    stack_info[1] = channelConfigs[new_phy_index]->configs[0].stackInfo[1];
  } else {
    stack_info[0] = 0;
    stack_info[1] = 0;
  }

  #if defined(SL_CATALOG_APP_LOG_PRESENT)
  current_protocol = (RAIL_SDK_Protocol_t) stack_info[0];
  #endif

  switch (stack_info[0]) {
    case CUSTOM_AND_SUN_OQPSK:
      if (stack_info[1] == 0x60 || stack_info[1] == 0x70) {
        unpack_packet_fnc = &unpack_packet_sun_oqpsk;
        prepare_packet_fnc = &prepare_packet_sun_oqpsk;
      } else {
        unpack_packet_fnc = &unpack_packet_base;
        prepare_packet_fnc = &prepare_packet_base;
      }
      break;
    case WISUN:
      if (stack_info[1] >= 0x20) {
        unpack_packet_fnc = &unpack_packet_wisun_ofdm;
        prepare_packet_fnc = &prepare_packet_wisun_ofdm;
      } else {
        unpack_packet_fnc = &unpack_packet_wisun_fsk;
        prepare_packet_fnc = &prepare_packet_wisun_fsk;
      }
      break;
    case SIDEWALK:
      unpack_packet_fnc = &unpack_packet_sidewalk;
      prepare_packet_fnc = &prepare_packet_sidewalk;
      break;
    case CONNECT:
      if (stack_info[1] >= 0x20) {
        unpack_packet_fnc = &unpack_packet_wisun_ofdm;
        prepare_packet_fnc = &prepare_packet_wisun_ofdm;
      } else if (stack_info[1] == 0x01) {
        unpack_packet_fnc = &unpack_packet_wisun_fsk;
        prepare_packet_fnc = &prepare_packet_wisun_fsk;
      } else {
        unpack_packet_fnc = &unpack_packet_base;
        prepare_packet_fnc = &prepare_packet_base;
      }
      break;
    case LONGRANGE:
      unpack_packet_fnc = &unpack_packet_longrange;
      prepare_packet_fnc = &prepare_packet_longrange;
      break;
    case BPSK:
      unpack_packet_fnc = &unpack_packet_bpsk;
      prepare_packet_fnc = &prepare_packet_bpsk;
      break;
    default:
      unpack_packet_fnc = &unpack_packet_base;
      prepare_packet_fnc = &prepare_packet_base;
      break;
  }
}

uint8_t get_print_packet_info(void)
{
  return print_packet_info;
}

void set_print_packet_info(uint8_t new_print_packet_info)
{
  print_packet_info = new_print_packet_info;
}

uint8_t get_wisun_fsk_fcs(void)
{
  return wisun_fsk_fcs;
}

uint8_t set_wisun_fsk_fcs(uint8_t new_fcs)
{
  wisun_fsk_fcs = (new_fcs == 0) ? 0 : 1;
  return 1;
}

uint8_t get_wisun_fsk_whitening(void)
{
  return wisun_fsk_whitening;
}

uint8_t set_wisun_fsk_whitening(uint8_t new_whitening)
{
  wisun_fsk_whitening = (new_whitening == 0) ? 0 : 1;
  return 1;
}

uint8_t get_wisun_ofdm_rate(void)
{
  return wisun_ofdm_rate;
}

uint8_t set_wisun_ofdm_rate(uint8_t new_rate)
{
  if (new_rate < 0x7) {
    wisun_ofdm_rate = new_rate;
    return 1;
  } else {
    return 0;
  }
}

uint8_t get_wisun_ofdm_scrambler(void)
{
  return wisun_ofdm_scrambler;
}

uint8_t set_wisun_ofdm_scrambler(uint8_t new_scrambler)
{
  wisun_ofdm_scrambler = (new_scrambler & 0x3);
  return 1;
}

uint8_t get_sun_oqpsk_spreading_mode(void)
{
  return sun_oqpsk_spreading_mode;
}

uint8_t set_sun_oqpsk_spreading_mode(uint8_t new_spreading_mode)
{
  sun_oqpsk_spreading_mode = (new_spreading_mode == 0) ? false : true;
  return 1;
}

uint8_t get_sun_oqpsk_rate_mode(void)
{
  return sun_oqpsk_rate_mode;
}

uint8_t set_sun_oqpsk_rate_mode(uint8_t new_rate_mode)
{
  sun_oqpsk_rate_mode = (new_rate_mode & 0x3);
  return 1;
}

uint8_t get_sidewalk_fcs_type(void)
{
  return sidewalk_fcs_type;
}

uint8_t set_sidewalk_fcs_type(uint8_t new_fcs)
{
  sidewalk_fcs_type = (new_fcs == 0) ? 0 : 1;
  return 1;
}

uint8_t get_sidewalk_whitening(void)
{
  return sidewalk_whitening;
}

uint8_t set_sidewalk_whitening(uint8_t new_whitening)
{
  sidewalk_whitening = (new_whitening == 0) ? 0 : 1;
  return 1;
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_wisun_fsk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  uint16_t payload_size = 0;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  uint8_t fcsType = 0U;
  uint8_t whitening = 0U;
  *start_of_payload
    = sl_rail_sdk_802154_packet_unpack_sunfsk_2byte_data_frame(packet_information,
                                                               &fcsType,
                                                               &whitening,
                                                               &payload_size,
                                                               rx_destination);
  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    if (current_protocol == CONNECT) {
      app_log_info("Connect SUN_FSK Packet is ready, %d bytes payload read with %d fcsType and %d whitening\n ", payload_size, fcsType, whitening);
    } else {
      app_log_info("WISUN_FSK Packet is ready, %d bytes payload read with %d fcsType and %d whitening\n ", payload_size, fcsType, whitening);
    }
    #endif
  }
  return payload_size;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_wisun_fsk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  uint16_t packet_size = 0U;
  uint8_t tx_frame_buffer[256];
  sl_rail_sdk_802154_packet_pack_sunfsk_2bytes_data_frame(wisun_fsk_fcs,
                                                          wisun_fsk_whitening,
                                                          length,
                                                          out_data,
                                                          &packet_size,
                                                          tx_frame_buffer);
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, tx_frame_buffer, packet_size, true);
  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    if (current_protocol == CONNECT) {
      app_log_info("Connect SUN_FSK Packet is ready, %d bytes written with %d fcsType and %d whitening\n ", bytes_written_in_fifo, wisun_fsk_fcs, wisun_fsk_whitening);
    } else {
      app_log_info("WISUN_FSK Packet is ready, %d bytes written with %d fcsType and %d whitening\n ", bytes_written_in_fifo, wisun_fsk_fcs, wisun_fsk_whitening);
    }
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_wisun_ofdm(sl_rail_handle_t rail_handle, uint8_t *rx_destination,
                                  const sl_rail_rx_packet_info_t *packet_information,
                                  uint8_t **start_of_payload)
{
  uint16_t payload_size = 0U;
  uint8_t rate = 0U;
  uint8_t scrambler = 0U;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  *start_of_payload = sl_rail_sdk_802154_packet_unpack_ofdm_data_frame(packet_information,
                                                                       &rate,
                                                                       &scrambler,
                                                                       &payload_size,
                                                                       rx_destination);
  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    if (current_protocol == CONNECT) {
      app_log_info("Connect_OFDM Packet is ready, %d bytes payload read with %d rate and %d scrambler\n ", payload_size, rate, scrambler);
    } else {
      app_log_info("WISUN_OFDM Packet is ready, %d bytes payload read with %d rate and %d scrambler\n ", payload_size, rate, scrambler);
    }
    #endif
  }
  return payload_size;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_wisun_ofdm(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  uint16_t packet_size = 0U;
  uint8_t tx_frame_buffer[256];
  sl_rail_sdk_802154_packet_pack_ofdm_data_frame(wisun_ofdm_rate,
                                                 wisun_ofdm_scrambler,
                                                 length,
                                                 out_data,
                                                 &packet_size,
                                                 tx_frame_buffer);
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, tx_frame_buffer, packet_size, true);

  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    if (current_protocol == CONNECT) {
      app_log_info("Connect_OFDM Packet is ready, %d bytes written with %d rate and %d scrambler\n ", bytes_written_in_fifo, wisun_ofdm_rate, wisun_ofdm_scrambler);
    } else {
      app_log_info("WISUN_OFDM Packet is ready, %d bytes written with %d rate and %d scrambler\n ", bytes_written_in_fifo, wisun_ofdm_rate, wisun_ofdm_scrambler);
    }
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_sun_oqpsk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  uint16_t payload_size = 0U;
  bool spreadingMode = false;
  uint8_t rateMode = 0U;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  *start_of_payload = sl_rail_sdk_802154_packet_unpack_oqpsk_data_frame(packet_information,
                                                                        &spreadingMode,
                                                                        &rateMode,
                                                                        &payload_size,
                                                                        rx_destination);
  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("SUN_OQPSK Packet is ready, %d bytes payload read with %d spreading mode and %d rate mode\n ", payload_size, spreadingMode, rateMode);
    #endif
  }
  return payload_size;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_sun_oqpsk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  uint16_t packet_size = 0U;
  uint8_t tx_frame_buffer[256];

  sl_rail_sdk_802154_packet_pack_oqpsk_data_frame(sun_oqpsk_spreading_mode,
                                                  sun_oqpsk_rate_mode,
                                                  length,
                                                  out_data,
                                                  &packet_size,
                                                  tx_frame_buffer);

  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, tx_frame_buffer, packet_size, true);
  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("SUN_OQPSK Packet is ready, %d bytes written with %d spreading mode and %d rate mode\n ", bytes_written_in_fifo, sun_oqpsk_spreading_mode, sun_oqpsk_rate_mode);
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_sidewalk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  uint16_t payload_size = 0;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  uint8_t fcsType = 0U;
  uint8_t whitening = 0U;
  *start_of_payload
    = sl_rail_sdk_802154_packet_unpack_sidewalk_data_frame(packet_information,
                                                           &fcsType,
                                                           &whitening,
                                                           &payload_size,
                                                           rx_destination);
  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("SideWalk Packet is ready, %d bytes payload read with %d fcsType and %d whitening\n ", payload_size, fcsType, whitening);
    #endif
  }
  return payload_size;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_sidewalk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  uint16_t packet_size = 0U;
  uint8_t tx_frame_buffer[256];
  sl_rail_sdk_802154_packet_pack_sidewalk_data_frame(sidewalk_fcs_type,
                                                     sidewalk_whitening,
                                                     length,
                                                     out_data,
                                                     &packet_size,
                                                     tx_frame_buffer);
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, tx_frame_buffer, packet_size, true);
  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("SideWalk Packet is ready, %d bytes written with %d fcsType and %d whitening\n ", bytes_written_in_fifo, sidewalk_fcs_type, sidewalk_whitening);
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_longrange(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  uint16_t payload_size = 0;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  *start_of_payload
    = sl_rail_sdk_802154_packet_unpack_longrange_data_frame(packet_information,
                                                            &payload_size,
                                                            rx_destination);
  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Long Range Packet is ready, %d bytes payload read\n ", payload_size);
    #endif
  }
  return payload_size;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_longrange(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  uint16_t packet_size = 0U;
  uint8_t tx_frame_buffer[256];
  sl_rail_sdk_802154_packet_pack_longrange_data_frame(length,
                                                      out_data,
                                                      &packet_size,
                                                      tx_frame_buffer);
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, tx_frame_buffer, packet_size, true);
  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Long Range Packet is ready, %d bytes written\n ", bytes_written_in_fifo);
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_bpsk(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  uint16_t payload_size = 0;

  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  *start_of_payload
    = sl_rail_sdk_802154_packet_unpack_bpsk_data_frame(packet_information,
                                                       &payload_size,
                                                       rx_destination);
  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("BPSK Packet is ready, %d bytes payload read\n ", payload_size);
    #endif
  }
  return payload_size;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_bpsk(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  uint16_t packet_size = 0U;
  uint8_t tx_frame_buffer[256];
  sl_rail_sdk_802154_packet_pack_bpsk_data_frame(length,
                                                 out_data,
                                                 &packet_size,
                                                 tx_frame_buffer);
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, tx_frame_buffer, packet_size, true);
  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == packet_size,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             packet_size);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("BPSK Packet is ready, %d bytes written\n ", bytes_written_in_fifo);
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API helps to unpack the received packet, point to the payload and returns the length.
 *****************************************************************************/
uint16_t unpack_packet_base(sl_rail_handle_t rail_handle, uint8_t *rx_destination, const sl_rail_rx_packet_info_t *packet_information, uint8_t **start_of_payload)
{
  sl_rail_status_t result = sl_rail_copy_rx_packet(rail_handle, rx_destination, packet_information);
  if (result != SL_RAIL_STATUS_NO_ERROR) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_warning("sl_rail_copy_rx_packet failed with error: %ld\n", result);
    #endif
  }
  *start_of_payload = rx_destination;

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Packet is ready, %d bytes read\n", packet_information->packet_bytes);
    #endif
  }

  return packet_information->packet_bytes;
}

/******************************************************************************
 * The API prepares the packet for sending and load it in the RAIL TX FIFO
 *****************************************************************************/
void prepare_packet_base(sl_rail_handle_t rail_handle, uint8_t *out_data, uint16_t length)
{
  // Check if write fifo has written all bytes
  uint16_t bytes_written_in_fifo = 0;
  bytes_written_in_fifo = sl_rail_write_tx_fifo(rail_handle, out_data, length, true);
  #if defined(SL_CATALOG_APP_ASSERT_PRESENT)
  app_assert(bytes_written_in_fifo == length,
             "sl_rail_write_tx_fifo() failed to write in fifo (%d bytes instead of %d bytes)\n",
             bytes_written_in_fifo,
             length);
  #endif

  if (print_packet_info) {
    #if defined(SL_CATALOG_APP_LOG_PRESENT)
    app_log_info("Packet is ready, %d bytes written\n", bytes_written_in_fifo);
    #endif
  }

  #if !defined(SL_CATALOG_APP_ASSERT_PRESENT) && !defined(SL_CATALOG_APP_LOG_PRESENT)
  // Avoid unused variable warning
  (void)bytes_written_in_fifo;
  #endif
}

/******************************************************************************
 * The API forwards the received rx packet on CLI
 *****************************************************************************/
void printf_rx_packet(const uint8_t * const rx_buffer, uint16_t length)
{
  #if defined(SL_CATALOG_APP_LOG_PRESENT)
  if (rx_buffer == NULL) {
    app_log_error("rx_buffer pointer is NULL!\n");
  } else if (length == 0) {
    app_log_error("packet length is zero!\n");
  } else {
    app_log_info("Packet has been received: ");
    for (uint16_t i = 0; i < length; i++) {
      // last byte
      if (i == length - 1) {
        app_log_info("0x%02X", rx_buffer[i]);
      } else {
        app_log_info("0x%02X, ", rx_buffer[i]);
      }
    }
    app_log_info("\n");
  }
  #else
  (void)rx_buffer;
  (void)length;
  #endif
}

/*****************************************************************************
* Checks phy setting to avoid errors at packet sending
*****************************************************************************/
void validation_check(void)
{
  _Static_assert(SL_RAIL_UTIL_INIT_PROTOCOL_INST0_DEFAULT == SL_RAIL_UTIL_PROTOCOL_PROPRIETARY,
                 "Please use the RAIL - Simple TRX Standards sample app instead, which is designed to show the protocol usage.");
}
