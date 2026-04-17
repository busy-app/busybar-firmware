/**
 * @file cmds_management.c
 * @copyright 2022 Silicon Laboratories Inc.
 */
#include <assert.h>
#include <app.h>
#include <cmds_management.h>
#include <ZW_application_transport_interface.h>
#include <utils.h>
#include <MfgTokens.h>
#include <serialapi_file.h>
#include <ZAF_Common_interface.h>
#include <ZAF_types.h>
#include <ZAF_version.h>
#include <string.h>
#include <zpal_misc.h>

#ifdef ZW_CONTROLLER
#include <ZW_controller_api.h>
#endif // ZW_CONTROLLER

#define PUK_OFFSET  0x23
#define PRK_OFFSET  0x43
#define HW_VER_OFFSET 0x70
#define HW_VER_SIZE   1

/** Find the byte in which SERIAL_API_SETUP command will be written */
#define BYTE_INDEX(x) ((x - 1) / 8)
/** Find the offset in the byte of SERIAL_API_SETUP command */
#define BYTE_OFFSET(x) (1 << ((x - 1) % 8))
/** Add the SERIAL_API_SETUP command to the bitmask array */
#define BITMASK_ADD_CMD(bitmask, cmd) (bitmask[BYTE_INDEX(cmd)] |= BYTE_OFFSET(cmd))
#define CONTROLLER_IS_SUC                       0x10 /* - If this bit is set then this controller is a SUC */

#ifndef MAX
/** Return the larger of two values.
 *
 * \param x         An integer-valued expression without side effects.
 * \param y         An integer-valued expression without side effects.
 *
 * \return The larger of \p x and \p y.
 */
#define MAX(x, y) ( (x) > (y) ? (x) : (y) )
#endif // MAX

static const serial_api_setup_cmd_get_region_info_answer_t regions_info[] = {
  { .region = REGION_EU, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_US, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_ANZ, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_HK, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_IN, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_IL, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_RU, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_CN, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_US_LR, .zw_classic = 1, .zw_lr = 1, .reserved = 0, .included_region = REGION_US },
  { .region = REGION_EU_LR, .zw_classic = 1, .zw_lr = 1, .reserved = 0, .included_region = REGION_EU },
  { .region = REGION_JP, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
  { .region = REGION_KR, .zw_classic = 1, .zw_lr = 0, .reserved = 0, .included_region = REGION_UNDEFINED },
};
#define REGIONS_INFO_COUNT   (sizeof(regions_info) / sizeof(regions_info[0]))
//default answer in case the requested region is not found in the regions_info table.
static const serial_api_setup_cmd_get_region_info_answer_t unknown_region_info =
{
  .region = REGION_UNDEFINED,
  .zw_classic = 0,
  .zw_lr = 0,
  .reserved = 0,
  .included_region = 0
};
#define REGION_INFO_SIZE  (sizeof(serial_api_setup_cmd_get_region_info_answer_t))

void func_id_serial_api_get_init_data(__attribute__((unused)) uint8_t inputLength,
                                      __attribute__((unused)) const uint8_t *pInputBuffer,
                                      uint8_t *pOutputBuffer,
                                      uint8_t *pOutputLength)
{
  *pOutputLength = 5;
  pOutputBuffer[0] = SERIAL_API_VER;
  pOutputBuffer[1] = 0; /* Flag byte - default: controller api, no timer support, no primary, no SUC */
#ifdef ZW_CONTROLLER
  if (!IsPrimaryController()) {
    pOutputBuffer[1] |= GET_INIT_DATA_FLAG_SECONDARY_CTRL; /* Set Primary/secondary bit */
  }
  if (GetControllerCapabilities() & CONTROLLER_IS_SUC) { /* if (ZW_IS_SUC_ACTIVE()) */
    pOutputBuffer[1] |= GET_INIT_DATA_FLAG_IS_SUC; /* Set SUC bit if active */
  }

  /* compl_workbuf[1] is already set to controller api*/
  pOutputBuffer[2] = ZW_MAX_NODES / 8; /* node bitmask length */

  /* Clear the buffer */
  memset(pOutputBuffer + 3, 0, ZW_MAX_NODES / 8);

  /* Next ZW_MAX_NODES/8 = 29 bytes of compl_workbuf reserved for node bitmask */

  Get_included_nodes(pOutputBuffer + 3);

  pOutputBuffer[3 + (ZW_MAX_NODES / 8)] = zpal_get_chip_type();
  pOutputBuffer[4 + (ZW_MAX_NODES / 8)] = zpal_get_chip_revision();
  *pOutputLength += (ZW_MAX_NODES / 8);
  assert(*pOutputLength <= 34);  // Elsewhere, like in zwapi_init.c, the pOutputBuffer is hardcoded to 34 bytes in lenght.
#else
  pOutputBuffer[1] |= GET_INIT_DATA_FLAG_SLAVE_API; /* Flag byte */
  pOutputBuffer[2] = 0;                             /* node bitmask length */
  pOutputBuffer[3] = zpal_get_chip_type();
  pOutputBuffer[4] = zpal_get_chip_revision();
#endif
}

void func_id_serial_api_get_nls_nodes(__attribute__((unused)) uint8_t const inputLength,
                                      const uint8_t *pInputBuffer,
                                      uint8_t *pOutputBuffer,
                                      uint8_t *pOutputLength)
{
  //RES | 0xC0 | MORE_NODES | BITMASK_OFFSET | BITMASK_LEN | BITMASK_ARRAY

  uint8_t bitmaskOffset = pInputBuffer[0];
  uint8_t outputLength = 0;
  bool moreNodes = 0;
  uint8_t MAX_ALLOWED_BITMASK_OFFSET = (uint8_t)ceiling_division((uint32_t)(MAX_NODEMASK_LENGTH + MAX_LR_NODEMASK_LENGTH), (uint32_t)GET_NLS_NODES_LIST_LENGTH_MAX) - 1;

  if (bitmaskOffset > MAX_ALLOWED_BITMASK_OFFSET) {
    bitmaskOffset = MAX_ALLOWED_BITMASK_OFFSET;
  }

  Get_included_NLS_nodes(pOutputBuffer + 3, bitmaskOffset, &moreNodes, &outputLength);

  pOutputBuffer[0] = moreNodes ? MORE_NODES : NO_MORE_NODES;
  pOutputBuffer[1] = bitmaskOffset;
  pOutputBuffer[2] = outputLength;
  *pOutputLength = 3 + outputLength;
}

#ifdef ZW_CONTROLLER
void func_id_serial_api_get_LR_nodes(__attribute__((unused)) uint8_t inputLength,
                                     const uint8_t *pInputBuffer,
                                     uint8_t *pOutputBuffer,
                                     uint8_t *pOutputLength)
{
  //RES | 0xDA | MORE_NODES | BITMASK_OFFSET | BITMASK_LEN | BITMASK_ARRAY

  /*
   * The current implementation of this function is made on the fact
   * that there is no support in the Z-Wave protocol code for more than 1024 Long Range nodes in total.
   * This Assert is here to remind us to update this function, if in the future the number of supported nodes increases.
   * In which case the MAX_LR_NODEMASK_LENGTH define will become greater than 128
   */
  _Static_assert(MAX_LR_NODEMASK_LENGTH <= 128, "STATIC_ASSERT_MAX_LR_NODEMASK_LENGTH_to_big");

  uint8_t bitmaskOffset = pInputBuffer[0];
  *pOutputLength = 3 + MAX_LR_NODEMASK_LENGTH;
  pOutputBuffer[0] = 0; // MORE_NODES - No more nodes for now.
  // Allowed values for bitmaskOffset are 0, 1, 2, 3
  if (bitmaskOffset > 3) {
    bitmaskOffset = 3;
  }
  pOutputBuffer[1] = bitmaskOffset;

  // Clean output buffer first
  memset(pOutputBuffer + 3, 0, MAX_LR_NODEMASK_LENGTH);

  pOutputBuffer[2] = MAX_LR_NODEMASK_LENGTH; // BITMASK_LEN hardcoded
  if (bitmaskOffset < 1) {
    Get_included_lr_nodes(pOutputBuffer + 3);
  }
}
#endif

extern bool bTxStatusReportEnabled;

zpal_tx_power_decidbm_t
GetMaxSupportedTxPower(void)
{
  const SApplicationHandles *pAppHandles = ZAF_getAppHandle();
  SZwaveCommandPackage CommandPackage = {
    .eCommandType = EZWAVECOMMANDTYPE_ZW_GET_TX_POWER_MAX_SUPPORTED
  };
  // Put the Command on queue (and dont wait for it, queue must be empty)
  if (EQUEUENOTIFYING_STATUS_SUCCESS == QueueNotifyingSendToBack(pAppHandles->pZwCommandQueue, (uint8_t *)&CommandPackage, 0)) {
    // Wait for protocol to handle command
    SZwaveCommandStatusPackage result = { 0 };
    if (GetCommandResponse(&result, EZWAVECOMMANDSTATUS_ZW_GET_TX_POWER_MAX_SUPPORTED)) {
      return result.Content.GetTxPowerMaximumSupported.tx_power_max_supported;
    }
  }
  return ZW_TX_POWER_140_DDBM;
}

void func_id_serial_api_setup(uint8_t inputLength,
                              const uint8_t *pInputBuffer,
                              uint8_t *pOutputBuffer,
                              uint8_t *pOutputLength)
{
  uint8_t i = 0;
  uint8_t cmdRes;
  zpal_radio_region_t rfRegion;
  zpal_tx_power_decidbm_t iPowerLevel = 0;
  zpal_tx_power_decidbm_t iPower0dbmMeasured = 0;

  /* We assume operation is nonesuccessful */
  cmdRes = false;

  if (1 > inputLength) {
    /* Command length must be at least 1 byte. Return with negative response in the out buffer */
    pOutputBuffer[i++] = cmdRes;
    *pOutputLength = i;
    return;
  }

  pOutputBuffer[i++] = pInputBuffer[0];   /* Set output command ID equal input command ID */
  switch (pInputBuffer[0]) {
    /* Report which SerialAPI Setup commands are supported beside the SERIAL_API_SETUP_CMD_SUPPORTED */
    case SERIAL_API_SETUP_CMD_SUPPORTED:
      /* HOST->ZW: SERIAL_API_SETUP_CMD_SUPPORTED */
      /* ZW->HOST: SERIAL_API_SETUP_CMD_SUPPORTED |
       *              (SERIAL_API_SETUP_CMD_TX_STATUS_REPORT + SERIAL_API_SETUP_CMD_RF_REGION_GET + SERIAL_API_SETUP_CMD_RF_REGION_SET +
       *               SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET + SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET +
       *               SERIAL_API_SETUP_CMD_TX_GET_MAX_PAYLOAD_SIZE + SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET) | */
      /*               supportedBitmask */

      pOutputBuffer[i++] = SERIAL_API_SETUP_CMD_TX_STATUS_REPORT | SERIAL_API_SETUP_CMD_RF_REGION_GET
                           | SERIAL_API_SETUP_CMD_RF_REGION_SET | SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET
                           | SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET | SERIAL_API_SETUP_CMD_TX_GET_MAX_PAYLOAD_SIZE
                           | SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET | SERIAL_API_SETUP_CMD_SUPPORTED;

      /* Report all supported commands as bitmask of their values */
      uint8_t supportedBitmask[32];
      memset(supportedBitmask, 0, sizeof(supportedBitmask));
      /* For each command in eSerialAPISetupCmd, find a byte number in supportedBitmask where it should be,
       * and position (offset) in it and then add it to the array. */
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_SUPPORTED);                  // (1)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_STATUS_REPORT);           // (2)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET);          // (4)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET);          // (8)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_GET_MAX_PAYLOAD_SIZE);    // (16)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_RF_REGION_GET);              // (32)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_RF_REGION_SET);              // (64)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET);        // (128)

      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_SET);          // (3)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_GET);          // (5)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_GET_MAX_LR_PAYLOAD_SIZE); // (17)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET_16_BIT);   // (18)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET_16_BIT);   // (19)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_GET_SUPPORTED_REGION);       // (21)
      BITMASK_ADD_CMD(supportedBitmask, SERIAL_API_SETUP_CMD_GET_REGION_INFO);            // (22)

      /* Currently supported command with the highest value is SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET.
         No commands after it. */
      for (int j = 0; j <= SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET / 8; j++) {
        pOutputBuffer[i++] = supportedBitmask[j];
      }
      break;

    case SERIAL_API_SETUP_CMD_TX_STATUS_REPORT:
      /* HOST->ZW: SERIAL_API_SETUP_CMD_TX_STATUS_REPORT | EnableTxStatusReport */
      /* ZW->HOST: SERIAL_API_SETUP_CMD_TX_STATUS_REPORT | cmdRes */
      if (SERIAL_API_SETUP_CMD_TX_STATUS_REPORT_CMD_LENGTH_MIN <= inputLength) {
        /* Do we enable or disable */
        bTxStatusReportEnabled = (0 != pInputBuffer[1]);
        /* Operation successful */
        cmdRes = true;
      }
      pOutputBuffer[i++] = cmdRes;
      break;

    /* Report RF region configuration */
    case SERIAL_API_SETUP_CMD_RF_REGION_GET:
      /* HOST->ZW: SERIAL_API_SETUP_CMD_RF_REGION_GET */
      /* ZW->HOST: SERIAL_API_SETUP_CMD_RF_REGION_GET | rfRRegion */
      if (false == ReadApplicationRfRegion(&rfRegion)) {
        /* Error reading value from flash. (Should not happen). Return undefined value. */
        rfRegion = REGION_UNDEFINED;
      }
      pOutputBuffer[i++] = rfRegion;
      break;

    /* Set RF region configuration */
    case SERIAL_API_SETUP_CMD_RF_REGION_SET:
      /* HOST->ZW: SERIAL_API_SETUP_CMD_RF_REGION_SET | rfRegion */
      /* ZW->HOST: SERIAL_API_SETUP_CMD_RF_REGION_SET | cmdRes */
      if (SERIAL_API_SETUP_CMD_RF_REGION_SET_CMD_LENGTH_MIN <= inputLength) {
        rfRegion = pInputBuffer[1];
        /* Check if the RF Region value is valid, and then store it in flash  */
        if (true == isRfRegionValid(rfRegion)) {
          /* Save into nvm */
          cmdRes = SaveApplicationRfRegion(rfRegion);
        }
      }
      pOutputBuffer[i++] = cmdRes;
      break;

    case SERIAL_API_SETUP_CMD_GET_SUPPORTED_REGION:
    {
      uint8_t supported_region_count = 0;
      uint8_t region_count_index = i;
      i++; //skip suported region count, move to first region value;
      for (rfRegion = REGION_2CH_FIRST; rfRegion < REGION_2CH_END; rfRegion++) {
        if (true == isRfRegionValid(rfRegion)) {
          supported_region_count++;
          pOutputBuffer[i] = (uint8_t) rfRegion;
          i++;
        }
      }
      for (rfRegion = REGION_3CH_FIRST; rfRegion < REGION_3CH_END; rfRegion++) {
        if (true == isRfRegionValid(rfRegion)) {
          supported_region_count++;
          pOutputBuffer[i] = (uint8_t) rfRegion;
          i++;
        }
      }
      pOutputBuffer[region_count_index] = supported_region_count;
      break;
    }

    case SERIAL_API_SETUP_CMD_GET_REGION_INFO:
    {
      uint8_t info_idx;
      //search for the requested region in the regions_info table.
      for (info_idx = 0; info_idx < REGIONS_INFO_COUNT; info_idx++) {
        if (regions_info[info_idx].region == pInputBuffer[SAPI_SETUP_GET_REGION_INFO_RX_IDX_REGION]) {
          break;
        }
      }
      // Copy the answer in the output buffer.
      if (info_idx < REGIONS_INFO_COUNT) {
        memcpy(&(pOutputBuffer[i]), &(regions_info[info_idx]), REGION_INFO_SIZE);
      } else {
        //region not found, answer the unknown region info.
        memcpy(&(pOutputBuffer[i]), &unknown_region_info, REGION_INFO_SIZE);
      }
      i += REGION_INFO_SIZE;
      break;
    }

    case SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET:
    {
      zpal_tx_power_decidbm_t iTxPower;
      zpal_tx_power_decidbm_t iAdjust;
      /**
       *  HOST->ZW: SERIAL_API_SETUP_CMD_TX_POWER_SET | NormalTxPowerLevel | Measured0dBmPower
       *  ZW->HOST: SERIAL_API_SETUP_CMD_TX_POWER_SET | cmdRes
       */
      if (SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET_CMD_LENGTH_MIN <= inputLength) {
        iTxPower = (int8_t)pInputBuffer[1];
        iAdjust  = (int8_t)pInputBuffer[2];
        /**
         * The min and max boundaries of int8_t are valid boundaries of the parameters that are being stored.
         * However, this command does not support a higher value than 127 deci dBm or lower than -127 deci dBm
         * for the parameters as a limitation of this SerialAPI command.
         *
         * Please use SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET_16_BIT which support our entire tx power range.
         */
        cmdRes = SaveApplicationTxPowerlevel(iTxPower, iAdjust);
      }
      pOutputBuffer[i++] = cmdRes; // true if success
      break;
    }

    case SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET:
      /**
       *  HOST->ZW: SERIAL_API_SETUP_CMD_TX_POWER_GET
       *  ZW->HOST: SERIAL_API_SETUP_CMD_TX_POWER_GET | NormalTxPowerLevel | Measured0dBmPower
       */
      ReadApplicationTxPowerlevel(&iPowerLevel, &iPower0dbmMeasured);

      /**
       * This SerialAPI command has the following limitation that it cannot retrieve stored tx power values that are
       * larger than 127 deci dBm or lower than -127 deci dBm.
       */

      // Clamp values to fit into the return parameter type of int8_t.
      if (iPowerLevel > INT8_MAX) {
        iPowerLevel = INT8_MAX;
      } else if (iPowerLevel < INT8_MIN) {
        iPowerLevel = INT8_MIN;
      }

      if (iPower0dbmMeasured > INT8_MAX) {
        iPower0dbmMeasured = INT8_MAX;
      } else if (iPower0dbmMeasured < INT8_MIN) {
        iPower0dbmMeasured = INT8_MIN;
      }

      pOutputBuffer[i++] = (uint8_t)iPowerLevel;
      pOutputBuffer[i++] = (uint8_t)iPower0dbmMeasured;
      break;

    case SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET_16_BIT:
    {
      zpal_tx_power_decidbm_t iTxPower;
      zpal_tx_power_decidbm_t iAdjust;
      zpal_tx_power_decidbm_t iTxPowerMaxSupported;
      /**
       *  HOST->ZW: SERIAL_API_SETUP_CMD_TX_POWER_SET | NormalTxPowerLevel (MSB) |NormalTxPowerLevel (LSB) | Measured0dBmPower (MSB)| Measured0dBmPower (LSB)
       *  ZW->HOST: SERIAL_API_SETUP_CMD_TX_POWER_SET | cmdRes
       */
      if (SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET_CMD_LENGTH_MIN <= inputLength) {
        iTxPower = (zpal_tx_power_decidbm_t)GET_16BIT_VALUE(&pInputBuffer[1]);
        iAdjust  = (zpal_tx_power_decidbm_t)GET_16BIT_VALUE(&pInputBuffer[3]);
        iTxPowerMaxSupported = GetMaxSupportedTxPower();

        /**
         * Only allow power level between -10dBm and 14 or 20dBm if 20dBm OPN used (API is in deci dBm)
         * Only allow measured0dBmPower level between -10dBm and 10dBm
         */
        if ((iTxPower >= -ZW_TX_POWER_100_DDBM)
            && (iTxPower <=  iTxPowerMaxSupported)
            && (iAdjust  >= -ZW_TX_POWER_100_DDBM)
            && (iAdjust  <=  ZW_TX_POWER_100_DDBM)
            ) {
          cmdRes = SaveApplicationTxPowerlevel(iTxPower, iAdjust);
        }
      }
      pOutputBuffer[i++] = cmdRes; // true if success
      break;
    }

    case SERIAL_API_SETUP_CMD_TX_POWERLEVEL_GET_16_BIT:
      /**
       *  HOST->ZW: SERIAL_API_SETUP_CMD_TX_POWER_GET_2
       *  ZW->HOST: SERIAL_API_SETUP_CMD_TX_POWER_GET_2 | NormalTxPowerLevel (16bit) | Measured0dBmPower (16bit)
       */
      ReadApplicationTxPowerlevel(&iPowerLevel, &iPower0dbmMeasured);
      pOutputBuffer[i++] = (uint8_t)((iPowerLevel >> 8) & 0xFF); // Big-endian
      pOutputBuffer[i++] = (uint8_t)(iPowerLevel & 0xFF);
      pOutputBuffer[i++] = (uint8_t)((iPower0dbmMeasured >> 8) & 0xFF);
      pOutputBuffer[i++] = (uint8_t)(iPower0dbmMeasured & 0xFF);
      break;

    case SERIAL_API_SETUP_CMD_TX_GET_MAX_PAYLOAD_SIZE:
      pOutputBuffer[i++] = (uint8_t)ZAF_getAppHandle()->pNetworkInfo->MaxPayloadSize;
      break;

    case SERIAL_API_SETUP_CMD_TX_GET_MAX_LR_PAYLOAD_SIZE:
      pOutputBuffer[i++] = (uint8_t)ZAF_getAppHandle()->pLongRangeInfo->MaxLongRangePayloadSize;
      break;

    /* Set the Node ID base type */
    case SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET:
      /* HOST->ZW: SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET | type */
      /* ZW->HOST: SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET | cmdRes */
      nodeIdBaseType = SERIAL_API_SETUP_NODEID_BASE_TYPE_DEFAULT;
      if ( (SERIAL_API_SETUP_CMD_NODEID_BASETYPE_SET_CMD_LENGTH_MIN <= inputLength)
           && (0 < pInputBuffer[1])
           && (SERIAL_API_SETUP_NODEID_BASE_TYPE_LAST > pInputBuffer[1]) ) {
        /* Set the global Node ID base type if input value is valid */
        nodeIdBaseType = pInputBuffer[1];
        SaveApplicationNodeIdBaseType(nodeIdBaseType);
        cmdRes = true;
      }
      pOutputBuffer[i++] = cmdRes;
      break;
    case SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_SET:
    {
      /**
       *  HOST->ZW: SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_SET | maxtxpower (16-bit)
       *  ZW->HOST: SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_SET | cmdRes
       */
      zpal_tx_power_decidbm_t iTxPower;
      zpal_tx_power_decidbm_t iTxPowerMaxSupported;

      if (SERIAL_API_SETUP_CMD_TX_POWERLEVEL_SET_CMD_LENGTH_MIN <= inputLength) {
        iTxPower = (zpal_tx_power_decidbm_t)GET_16BIT_VALUE(&pInputBuffer[1]);
        iTxPowerMaxSupported = GetMaxSupportedTxPower();

        /**
         * Only allow power level between -10dBm and 14 or 20dBm if 20dBm OPN used (API is in deci dBm)
         */
        if ((iTxPower >= -ZW_TX_POWER_100_DDBM)
            && (iTxPower <=  iTxPowerMaxSupported)
            ) {
          cmdRes = SaveApplicationMaxLRTxPwr(iTxPower);
        }
      }
      pOutputBuffer[i++] = cmdRes; // true if success
      break;
    }

    case SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_GET:
      /**
       *  HOST->ZW: SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_GET
       *  ZW->HOST: SERIAL_API_SETUP_CMD_MAX_LR_TX_PWR_GET | maxtxpower (16-bit)
       */
    {
      int16_t readout = 0;
      ReadApplicationMaxLRTxPwr(&readout);
      pOutputBuffer[i++] = (uint8_t)((readout >> 8) & 0xFF);
      pOutputBuffer[i++] = (uint8_t)(readout & 0xFF);
    }
    break;

    default:
      /* HOST->ZW: [SomeUnsupportedCmd] | [SomeData] */
      /* ZW->HOST: SERIAL_API_SETUP_CMD_UNSUPPORTED | [SomeUnsupportedCmd] */
      /* All other commands are unsupported */
      pOutputBuffer[0] = SERIAL_API_SETUP_CMD_UNSUPPORTED;
      pOutputBuffer[i++] = pInputBuffer[0];
      break;
  }

  *pOutputLength = i;
}

void func_id_serial_api_get_nvr(__attribute__((unused)) uint8_t inputLength,
                                const uint8_t *pInputBuffer,
                                uint8_t *pOutputBuffer,
                                uint8_t *pOutputLength)
{
  uint8_t offset  = pInputBuffer[0];
  uint8_t bLength = pInputBuffer[1];
  uint8_t  dataLen = 0;
  if (PUK_OFFSET == offset) {
    dataLen = bLength;
    if (TOKEN_MFG_ZW_PUK_SIZE < bLength) {
      dataLen = TOKEN_MFG_ZW_PUK_SIZE;
    }
    ZW_GetMfgTokenData(pOutputBuffer, TOKEN_MFG_ZW_PUK_ID, dataLen);
  } else if (PRK_OFFSET == offset) {
    dataLen = bLength;
    if (TOKEN_MFG_ZW_PRK_SIZE < bLength) {
      dataLen = TOKEN_MFG_ZW_PRK_SIZE;
    }
    ZW_GetMfgTokenData(pOutputBuffer, TOKEN_MFG_ZW_PRK_ID, dataLen);
  } else if (HW_VER_OFFSET == offset) {
    dataLen = bLength;
    if (HW_VER_SIZE < bLength) {
      dataLen = HW_VER_SIZE;
    }
    *pOutputBuffer = 0xFF;
  }
  *pOutputLength = dataLen;
}

void func_id_zw_get_protocol_version(uint8_t inputLength,
                                     const uint8_t *pInputBuffer,
                                     uint8_t *pOutputBuffer,
                                     uint8_t *pOutputLength)
{
  (void)inputLength;
  (void)pInputBuffer;
  // Defined in the specs to be the max size of the git hash
  const uint8_t git_hash_max_size = 16;
  uint8_t len = 0;
  const uint8_t *git_hash_id = ZW_GetProtocolGitHash();

  const SApplicationHandles *pAppHandles = ZAF_getAppHandle();
  pOutputBuffer[len++] = pAppHandles->pProtocolInfo->eProtocolType;
  pOutputBuffer[len++] = pAppHandles->pProtocolInfo->ProtocolVersion.Major;
  pOutputBuffer[len++] = pAppHandles->pProtocolInfo->ProtocolVersion.Minor;
  pOutputBuffer[len++] = pAppHandles->pProtocolInfo->ProtocolVersion.Revision;
  pOutputBuffer[len++] =  (uint8_t)(ZAF_GetBuildNumber() >> 8);
  pOutputBuffer[len++] =  (uint8_t)(ZAF_GetBuildNumber() );
  for (uint32_t i = 0; i < git_hash_max_size; i++, len++) {
    pOutputBuffer[len] = git_hash_id[i];
  }
  *pOutputLength = len;
}

bool InitiateShutdown(ZW_Void_Callback_t pCallback)
{
  const SApplicationHandles *pAppHandles = ZAF_getAppHandle();
  SZwaveCommandPackage shutdown = {
    .eCommandType = EZWAVECOMMANDTYPE_ZW_INITIATE_SHUTDOWN,
    .uCommandParams.InitiateShutdown.Handle = pCallback
  };

  // Put the Command on queue (and dont wait for it, queue must be empty)
  if (EQUEUENOTIFYING_STATUS_SUCCESS == QueueNotifyingSendToBack(pAppHandles->pZwCommandQueue, (uint8_t *)&shutdown, 0)) {
    // Wait for protocol to handle command
    SZwaveCommandStatusPackage result = { .eStatusType = EZWAVECOMMANDSTATUS_ZW_INITIATE_SHUTDOWN };
    if (GetCommandResponse(&result, result.eStatusType)) {
      return result.Content.InitiateShutdownStatus.result;
    }
  }
  return false;
}
