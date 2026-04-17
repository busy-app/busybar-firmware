/***************************************************************************//**
 * @file
 * @brief ESL Tag core interface declarations for abstract image
 *        functionalities.
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef ESL_TAG_IMAGE_CORE_H
#define ESL_TAG_IMAGE_CORE_H

/***********************************************************************************************//**
 * @addtogroup esl_tag_core
 * @{
 **************************************************************************************************/
#include <stdint.h>
#include "sl_status.h"

/// ESL Service Specification d09r18, Section 3.7.2.4: 48 bit Image Object ID
typedef uint8_t                   esl_image_object_id_t[6];

/// ESL Service Specification d09r18, Section 3.7.2.4: 48 bit Image Object ID
#define ESL_IMAGE_OBJECT_BASE     0x100u

/**************************************************************************//**
 * ESL Tag image init function. ESL Core component will call this during the
 * initialization of application. This call is hidden and happens automatically.
 *****************************************************************************/
void esl_image_init(void);

/**************************************************************************//**
 * ESL Tag image characteristic update. ESL Core component will call this
 * automatically on the bluetooth stack boot event. The real implementation in
 * the ESL Tag Image component will get the display info and write it to the ESL
 * Image Information Characteristic value for the lifecycle of the tag.
 *****************************************************************************/
void esl_image_characteristic_update(void);

/**************************************************************************//**
 * Getter for an ESL Tag image raw data chunk
 *
 * @param[in]  uint8_t image_index Index of the image to get raw data chunk of
 * @param[out] uint16_t *offset Size of data already read out
 * @param[in]  uint16_t buf_size Actual size of the target buffer
 * @param[out] uint8_t *target_buf Buffer address to copy the image chunk into
 * @note To get full image data this function needs to be called repeatedly
 *       until the offset value increases - offset in caller has to be
 *       persistent during the process, and usually its value must be set to 0,
 *       initially
 * @return sl_status_t
 *****************************************************************************/
sl_status_t esl_image_get_data(uint8_t image_index, uint16_t* offset,
                               uint16_t buf_size, uint8_t *target_buf);

/**************************************************************************//**
 * ESL Tag maximum image count getter
 * @return Number of available images
 * @note: To be implemented with each custom image storage implementation!
 *****************************************************************************/
uint8_t esl_image_get_count(void);

/***************************************************************************//**
 * Reset image storage objects
 * @note: To be implemented with each custom image storage implementation!
 ******************************************************************************/
void esl_image_reset_storage(void);

/***************************************************************************//**
 * Reinit image compression algorithm if it is used
 ******************************************************************************/
void esl_image_unpack_init(void);

/**************************************************************************//**
 * Unpack single chunk of received image data if compression is used
 *
 * @param *code Pointer to the current incoming data chunk
 * @param chunk_size Size of the incoming data in bytes
 * @param *data Pointer to the target area of unpacking
 * @param data_offset Current write offset from the start of the unpacking area
 * @param data_length Maximum size of the unpacking area
 * @param decompressed_length Size of the last chunk written to target area
 * @returns Returns SL_STATUS_OK on success, SL_STATUS_FAIL if decompression
 *          fails or SL_STATUS_WOULD_OVERFLOW if an uncompressed data chunk is
 *          too big to write to the target area.
 *
 * Unpacks data slices if the @ref otf_decompressor component is used or copy
 * uncompressed data to the target area, otherwise.
 *****************************************************************************/
sl_status_t esl_image_unpack_chunk(const void *code,
                                   const size_t chunk_size,
                                   void *data,
                                   size_t data_offset,
                                   size_t data_length,
                                   size_t* decompressed_length);
/** @} (end addtogroup esl_tag_core) */
#endif // ESL_TAG_IMAGE_CORE_H
