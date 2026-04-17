#include "zniffer_handler.h"

#include <stdbool.h>

#include <string.h>
#include "SizeOf.h"
#include <assert.h>
#include "zpal_log.h"
#include <zpal_watchdog.h>
#include <zpal_misc.h>
#include <zpal_radio.h>

#include "comm_interface.h"
#include "commands.h"

#define BUF_SIZE (32)

uint8_t compl_workbuf[BUF_SIZE];

enum {
  stateIdle,
  stateFrameParse,
};

//----- type & variables to handle region
typedef struct {
  zpal_radio_region_t region_no;   ///< index of the region used in the zniffer API.
  uint8_t channels;
  bool lr_support;                 ///< Region to provide to zpal layer.
  char* description;
} region_descriptor_t;

#define MAX_NO_OF_REGION_SUPPORTED 12

region_descriptor_t SUPPORTED_REGION_LIST[MAX_NO_OF_REGION_SUPPORTED] =
{
  { .region_no = REGION_EU, .channels = 2, .lr_support = false, .description = "EU" },
  { .region_no = REGION_US, .channels = 2, .lr_support = false, .description = "US" },
  { .region_no = REGION_ANZ, .channels = 2, .lr_support = false, .description = "ANZ" },
  { .region_no = REGION_HK, .channels = 2, .lr_support = false, .description = "HK" },
  { .region_no = REGION_IN, .channels = 2, .lr_support = false, .description = "IN" },
  { .region_no = REGION_IL, .channels = 2, .lr_support = false, .description = "IL" },
  { .region_no = REGION_RU, .channels = 2, .lr_support = false, .description = "RU" },
  { .region_no = REGION_CN, .channels = 2, .lr_support = false, .description = "CN" },
  { .region_no = REGION_US_LR, .channels = 4, .lr_support = true, .description = "US_LR" },
  { .region_no = REGION_JP, .channels = 3, .lr_support = false, .description = "JP" },
  { .region_no = REGION_KR, .channels = 3, .lr_support = false, .description = "KR" },
  { .region_no = REGION_EU_LR, .channels = 4, .lr_support = true, .description = "EU_LR" },
};

//----- type & variables to handle channel configuration
//structure used to describe a channel configuration
typedef struct {
  zpal_radio_lr_channel_config_t ch_config; /**< Channel configuration, also used on serial API*/
  uint8_t channels;
  char* description;
} channel_config_descriptor_t;

//count of channel configuration supported by the zniffer
#define MAX_NO_OF_CH_CFG_SUPPORTED 3

/*List of channel configuration supported by the zniffer.
   ZPAL_RADIO_LR_CH_CFG_NO_LR is not listed because it is the same as using the region without LR.*/
channel_config_descriptor_t SUPPORTED_CH_CFG_LIST[MAX_NO_OF_CH_CFG_SUPPORTED] =
{
  { .ch_config = ZPAL_RADIO_LR_CH_CFG1, .channels = 4, .description = "CH_CFG1 (Ch classic & LR_A)" },
  { .ch_config = ZPAL_RADIO_LR_CH_CFG2, .channels = 4, .description = "CH_CFG2 (Ch classic & LR_B)" },
  { .ch_config = ZPAL_RADIO_LR_CH_CFG3, .channels = 1, .description = "CH_CFG3 (Ch LR_A & LR_B)" },
};

//variable used to store the last channel configuration set from the serial API.
zpal_radio_lr_channel_config_t last_set_ch_config = ZPAL_RADIO_LR_CH_CFG3;

static uint8_t state = stateIdle;
static bool stopSniffer = true;

void Respond(uint8_t cmd, uint8_t const *pData, uint8_t len)
{
  comm_interface_transmit_frame(cmd, CMD_FRAME, pData, len, NULL);
}

void DoRespond(void)
{
  Respond(serial_frame->cmd, NULL, 0);
}

void DoRespond_workbuf(uint8_t cnt)
{
  Respond(serial_frame->cmd, compl_workbuf, cnt);
}

uint8_t GetRadioSpeed(zpal_radio_speed_t speed)
{
  switch (speed) {
    case ZPAL_RADIO_SPEED_9600:
      return 0;
    case ZPAL_RADIO_SPEED_40K:
      return 1;
    case ZPAL_RADIO_SPEED_100K:
      return 2;
    case ZPAL_RADIO_SPEED_100KLR:
      return 3;
    default:
      return 0;
  }
}

uint8_t GetRadioChannel(zpal_radio_rx_parameters_t * pRxParameters)
{
  uint8_t region = zpal_radio_get_region();
  switch (region) {
    case REGION_EU:   __attribute__ ((fallthrough));
    case REGION_US:   __attribute__ ((fallthrough));
    case REGION_ANZ:  __attribute__ ((fallthrough));
    case REGION_HK:   __attribute__ ((fallthrough));
    case REGION_IN:   __attribute__ ((fallthrough));
    case REGION_IL:   __attribute__ ((fallthrough));
    case REGION_RU:   __attribute__ ((fallthrough));
    case REGION_CN:
      if (ZPAL_RADIO_SPEED_9600 == pRxParameters->speed || ZPAL_RADIO_SPEED_40K == pRxParameters->speed) {
        return 1;
      }
      return 0;

    case REGION_US_LR:  __attribute__ ((fallthrough));
    case REGION_EU_LR:
      if (ZPAL_RADIO_SPEED_9600 == pRxParameters->speed || ZPAL_RADIO_SPEED_40K == pRxParameters->speed) {
        return 1;
      }
      return pRxParameters->channel_id;

    case REGION_JP:   __attribute__ ((fallthrough));
    case REGION_KR:
      return pRxParameters->channel_id;

    default:
      return 0;
  }
}

uint8_t GetBeamSpeed(zpal_radio_zwave_channel_t channel)
{
  const zpal_radio_profile_t * pRfProfile = zpal_radio_get_rf_profile();
  if (NULL == pRfProfile) {
    assert(false);
    return 0;
  }
  switch (pRfProfile->region) {
    case REGION_EU:
    case REGION_US:
    case REGION_ANZ:
    case REGION_HK:
    case REGION_IN:
    case REGION_IL:
    case REGION_RU:
    case REGION_CN:
      return GetRadioSpeed(ZPAL_RADIO_SPEED_40K);
    case REGION_US_LR:  __attribute__ ((fallthrough));
    case REGION_EU_LR:
      if (ZPAL_RADIO_LR_CH_CFG3 == pRfProfile->active_lr_channel_config) {
        // only LR channels
        return GetRadioSpeed(ZPAL_RADIO_SPEED_100KLR);
      } else {
        // Any non-LR channels will use 40K
        if (channel <= ZPAL_RADIO_ZWAVE_CHANNEL_2) {
          return GetRadioSpeed(ZPAL_RADIO_SPEED_40K);
        }
        return GetRadioSpeed(ZPAL_RADIO_SPEED_100KLR);
      }
    case REGION_JP:
    case REGION_KR:
      return GetRadioSpeed(ZPAL_RADIO_SPEED_100K);
    default:
      assert(false);
      return 0;
  }
}

bool handling_beam = false;
uint16_t counter = 0;

tx_beam_start_frame_t beam_start_frame;
tx_beam_stop_frame_t beam_stop_frame;
tx_data_frame_t frame;

void beamReceiveHandler(uint16_t beamNodeId, uint16_t homeId, uint8_t channel, uint8_t rssi)
{
  if (!stopSniffer) {
    ++counter;
    if (!handling_beam) {
      handling_beam = true;
      beam_start_frame.sof = '!';
      beam_start_frame.type = BEAM_START;
      beam_start_frame.timestamp1 = 0;
      beam_start_frame.timestamp2 = 0;
      beam_start_frame.ch_speed = (channel << 5) | GetBeamSpeed(channel);
      beam_start_frame.region_no = zpal_radio_get_region();
      beam_start_frame.rssi = rssi >> 2; // TODO
      uint8_t a[4] = { 0x55, 0x00, 0x01, 0x00 };
      a[1] = beamNodeId;
      a[3] = homeId;
      memcpy(beam_start_frame.payload, a, 4);
      comm_interface_transmit_frame(BEAM_START, BEAM_FRAME, (uint8_t *)&beam_start_frame, 0, NULL);
    }
  }
}

void llRxFrameHandler(zpal_radio_receive_frame_t * pFrame)
{
  if (stopSniffer || NULL == pFrame) {
    return;
  }
  if (handling_beam) {
    handling_beam = false;
    beam_stop_frame.sof = '!';
    beam_stop_frame.type = BEAM_STOP;
    beam_stop_frame.timestamp1 = 0;
    beam_stop_frame.timestamp2 = 0;
    beam_stop_frame.rssi = ((uint8_t)pFrame->rx_parameters.rssi) >> 2; // TODO
    beam_stop_frame.counter = counter;
    comm_interface_transmit_frame(BEAM_STOP, BEAM_FRAME, (uint8_t *)&beam_stop_frame, 0, NULL);
    comm_interface_wait_transmit_done();
  }
  counter = 0;
  frame.sof = '!';
  frame.type = 1;
  frame.timestamp1 = 0;
  frame.timestamp2 = 0;
  frame.ch_speed = (GetRadioChannel(&pFrame->rx_parameters) << 5) | GetRadioSpeed(pFrame->rx_parameters.speed);
  frame.region_no = zpal_radio_get_region();
  frame.rssi = ((uint8_t)pFrame->rx_parameters.rssi) >> 2; // TODO
  frame.sodm = '!';
  frame.sod = 0x3;
  frame.len = pFrame->frame_content_length;
  memcpy(frame.payload, pFrame->frame_content, pFrame->frame_content_length);
  comm_interface_transmit_frame(0, DATA_FRAME, (uint8_t *)&frame, pFrame->frame_content_length, NULL);
}

/** This function is used to get the index of the given region in SUPPORTED_REGION_LIST
 * @param region_no the region to look for
 * @return index of the active region in SUPPORTED_REGION_LIST, MAX_NO_OF_REGION_SUPPORTED if not found
 */
uint8_t zn_getRegionIdx(zpal_radio_region_t region_no)
{
  uint8_t idx = 0;
  for (idx = 0; idx < MAX_NO_OF_REGION_SUPPORTED; idx++) {
    if ( region_no == SUPPORTED_REGION_LIST[idx].region_no ) {
      break;
    }
  }
  return idx;
}

/** This function is used to get the index of the given channel configuration in SUPPORTED_CH_CFG_LIST
 * @param ch_config the channel configuration to look for
 * @return index of the active channel configuration in SUPPORTED_CH_CFG_LIST, MAX_NO_OF_CH_CFG_SUPPORTED if not found
 */
uint8_t zn_getChCfgIdx(zpal_radio_lr_channel_config_t ch_config)
{
  uint8_t idx = 0;
  for (idx = 0; idx < MAX_NO_OF_CH_CFG_SUPPORTED; idx++) {
    if ( ch_config == SUPPORTED_CH_CFG_LIST[idx].ch_config ) {
      break;
    }
  }
  return idx;
}

void run_zniffer(void)
{
  comm_interface_init();

  for (;;) {
    switch (state) {
      case stateIdle:
      {
        zpal_feed_watchdog();

        uint8_t conState = comm_interface_parse_data();

        /* Nothing to transmit. Check if we received anything */
        if (conState == PARSE_FRAME_RECEIVED) {
          /* We got a frame... */
          state = stateFrameParse;
        } else if (conState == PARSE_IDLE) {
        }
      }
      break;
      case stateFrameParse:
      {
        uint8_t rxCmd = serial_frame->cmd;

        state = stateIdle;

        switch (rxCmd) {
          case FUNC_ID_ZN_GET_VERSION:
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_GET_VERSION\n");
            compl_workbuf[0] = zpal_get_chip_type();
            compl_workbuf[1] = zpal_get_chip_revision();
            compl_workbuf[2] = zpal_get_app_version_major();
            compl_workbuf[3] = zpal_get_app_version_minor();
            DoRespond_workbuf(4);
            break;

          case FUNC_ID_ZN_SET_REGION:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_SET_REGION\n");
            uint8_t region_no = serial_frame->payload[0];
            uint8_t region_idx = zn_getRegionIdx(region_no);

            if (MAX_NO_OF_REGION_SUPPORTED > region_idx) {
              zpal_radio_lr_channel_config_t ch_config_no;
              if ( true == SUPPORTED_REGION_LIST[region_idx].lr_support ) {
                //Long range region, use the latest channel configuration set by FUNC_ID_ZN_SET_LR_CH_CONFIG.
                ch_config_no = last_set_ch_config;
              } else {
                //not a long range region, disable long range channels.
                ch_config_no = ZPAL_RADIO_LR_CH_CFG_NO_LR;
              }
              zpal_radio_change_region(SUPPORTED_REGION_LIST[region_idx].region_no, ch_config_no);
              DoRespond();
            }
            /* if region_no is not found in SUPPORTED_REGION_LIST, that mean that the request region
               is not supported. In this case, do nothing.*/
          }
          break;

          case FUNC_ID_ZN_GET_REGIONS:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_GET_REGIONS\n");
            const zpal_radio_profile_t * pRfProfile = zpal_radio_get_rf_profile();
            uint8_t region_idx = 0;
            for (uint8_t i = 0; i < MAX_NO_OF_REGION_SUPPORTED; i++) {
              //fill region list
              compl_workbuf[i + 1] = SUPPORTED_REGION_LIST[i].region_no;
              //search for the active region
              if ( pRfProfile->region == SUPPORTED_REGION_LIST[i].region_no ) {
                region_idx = i;
              }
            }
            //fill active region
            compl_workbuf[0] = SUPPORTED_REGION_LIST[region_idx].region_no;
            DoRespond_workbuf(MAX_NO_OF_REGION_SUPPORTED + 1);
          }
          break;

          case FUNC_ID_ZN_START:
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_START\n");
            handling_beam = false;
            stopSniffer = false;
            DoRespond();
            break;

          case FUNC_ID_ZN_STOP:
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_STOP\n");
            handling_beam = false;
            stopSniffer = true;
            DoRespond();
            break;

          case FUNC_ID_ZN_SET_LR_CH_CONFIG:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_SET_LR_CH_CONFIG\n");
            uint8_t ch_config_no = serial_frame->payload[0];
            uint8_t ch_config_idx = zn_getChCfgIdx(ch_config_no);
            if ( MAX_NO_OF_CH_CFG_SUPPORTED  > ch_config_idx ) {
              last_set_ch_config = ch_config_no;
              zpal_radio_change_region(zpal_radio_get_region(), ch_config_no);
              DoRespond();
            }
            /* if ch_config_no is not found in SUPPORTED_CH_CFG_LIST, that mean that the request channel
               configuration is not supported. In this case, do nothing.*/
          }
          break;

          case FUNC_ID_ZN_GET_LR_CH_CONFIGS:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_GET_LR_CH_CONFIGS\n");
            const zpal_radio_profile_t * pRfProfile = zpal_radio_get_rf_profile();
            uint8_t ch_config_idx = 0;
            for (uint8_t i = 0; i < MAX_NO_OF_CH_CFG_SUPPORTED; i++) {
              //fill channel configuration list
              compl_workbuf[i + 1] = SUPPORTED_CH_CFG_LIST[i].ch_config;
              //search for the active channel configuration
              if ( pRfProfile->active_lr_channel_config == SUPPORTED_CH_CFG_LIST[i].ch_config ) {
                ch_config_idx = i;
              }
            }
            //fill active channel configuration
            compl_workbuf[0] = SUPPORTED_CH_CFG_LIST[ch_config_idx].ch_config;
            DoRespond_workbuf(MAX_NO_OF_CH_CFG_SUPPORTED + 1);
          }
          break;

          case FUNC_ID_ZN_GET_LR_REGIONS:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_GET_LR_REGIONS\n");
            uint8_t buf_idx = 0;
            for (uint8_t tableIdx = 0; tableIdx < MAX_NO_OF_REGION_SUPPORTED; tableIdx++) {
              if ( true == SUPPORTED_REGION_LIST[tableIdx].lr_support ) {
                compl_workbuf[buf_idx++] = SUPPORTED_REGION_LIST[tableIdx].region_no;
              }
            }
            DoRespond_workbuf(buf_idx);
          }
          break;

          case FUNC_ID_ZN_SET_BAUD_RATE:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_SET_BAUD_RATE\n");
            uint8_t baud_rate = serial_frame->payload[0];
            if (0 == baud_rate) {
              DoRespond();
            } else if (1 == baud_rate) {
              // Not supported
            } else {
              break;
            }
          }
          break;

          case FUNC_ID_ZN_GET_REGION_STR:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_GET_REGION_STR\n");
            uint8_t region_no = serial_frame->payload[0];
            uint8_t i;
            for (i = 0; i < MAX_NO_OF_REGION_SUPPORTED; ++i) {
              if (SUPPORTED_REGION_LIST[i].region_no == region_no) {
                break;
              }
            }
            if (MAX_NO_OF_REGION_SUPPORTED > i) {
              char* str;

              str = SUPPORTED_REGION_LIST[i].description;
              compl_workbuf[0] = region_no;
              compl_workbuf[1] = SUPPORTED_REGION_LIST[i].channels;
              i = 0;
              while ('\0' != *str) {
                compl_workbuf[i + 2] = *str++;
                ++i;
              }
              DoRespond_workbuf(i + 2);
            } else {
              DoRespond();
            }
          }
          break;

          case FUNC_ID_ZN_GET_LR_CH_CONFIG_STR:
          {
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "FUNC_ID_ZN_GET_LR_CH_CONFIG_STR\n");
            uint8_t ch_config_no = serial_frame->payload[0];
            uint8_t i;
            uint8_t ch_config_idx = zn_getChCfgIdx(ch_config_no);
            if (MAX_NO_OF_CH_CFG_SUPPORTED > ch_config_idx) {
              char* str;

              str = SUPPORTED_CH_CFG_LIST[ch_config_idx].description;
              compl_workbuf[0] = ch_config_no;
              compl_workbuf[1] = SUPPORTED_CH_CFG_LIST[ch_config_idx].channels;
              i = 0;
              while ('\0' != *str) {
                compl_workbuf[i + 2] = *str++;
                ++i;
              }
              DoRespond_workbuf(i + 2);
            } else {
              DoRespond();
            }
          }
          break;

          default:
            ZPAL_LOG_DEBUG(ZPAL_LOG_APP, "rxCmd = %u\n", rxCmd);
            state = stateIdle;
            break;
        }
      }
      break;
    }
  }
}
