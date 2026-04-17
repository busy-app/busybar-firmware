/**
 * @file
 * @copyright 2019 Silicon Laboratories Inc.
 */
#ifndef ZWAVE_API_ZW_H_
#define ZWAVE_API_ZW_H_

/**
 * @addtogroup ZWaveAPI
 * @{
 */

/**
 * @addtogroup ZW_SET_LEARN_MODE ZW Learn Mode defines
 * Mode parameters to ZW_SetLearnMode
 * @{
 */
typedef enum {
  ZW_SET_LEARN_MODE_DISABLE = 0,
  ZW_SET_LEARN_MODE_CLASSIC = 1,
  ZW_SET_LEARN_MODE_NWI = 2,
  ZW_SET_LEARN_MODE_NWE = 3,
  ZW_SET_LEARN_MODE_SMARTSTART = 4
} ZW_LearnMode_t;

///@}

///@}

#endif /* ZWAVE_API_ZW_H_ */
