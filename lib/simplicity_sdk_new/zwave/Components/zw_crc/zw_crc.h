/**
 * @file
 * Functions for calculation of CRC.
 * @copyright 2018 Silicon Laboratories Inc.
 */
#ifndef _ZW_CRC_H_
#define _ZW_CRC_H_

/****************************************************************************/
/*                              INCLUDE FILES                               */
/****************************************************************************/

#include <stdint.h>

/****************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                       */
/****************************************************************************/

/****************************************************************************/
/*                              EXPORTED DATA                               */
/****************************************************************************/

/****************************************************************************/
/*                           EXPORTED FUNCTIONS                             */
/****************************************************************************/

#define ZW_CRC_INITIAL_VALUE                    0x1D0Fu

/**
 * Returns a CRC calculation using the CCITT polynomial (0x1021).
 *
 * @param crc Initial value set to ZW_CRC_INITIAL_VALUE unless calculating multiple parts of a frame. In that case
 *            the value should be set to the result of the previous calculation.
 * @param pDataAddr Pointer to the array of data.
 * @param bDataLen Length of the data.
 * @return CRC value
 */
uint16_t zw_crc_check_crc16(
  uint16_t crc,
  const uint8_t *pDataAddr,
  uint16_t bDataLen
  );

#endif /* _ZW_CRC_H_ */
