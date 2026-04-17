/***************************************************************************//**
 * @file
 * @brief ESL Tag interface declarations for OTS image transfer functionalities.
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
#ifndef ESL_TAG_OTS_SERVER_H
#define ESL_TAG_OTS_SERVER_H
#include <stdint.h>
#include "sl_status.h"
#include "sl_bt_ots_datatypes.h"
#include "esl_tag_image_core.h"
#include "esl_tag_display_core.h"

/// UUID structure for SiLabs' vendor specific OTS Type ID for ESL images
typedef PACKSTRUCT (union {
  uuid_128      uuid;         ///< 128 bit UUID generic data
  struct {
    uint16_t    uuid128_hdr;  ///< 128 bit UUID object type custom header
    uint8_t     custom_flags; ///< 128 bit UUID object modifier flags
    uint8_t     display_type; ///< 128 bit UUID object display type
    uint8_t     reserved[12];
  };
}) sl_bt_esl_image_ots_uuid_t;

/// Custom modifier flags for ESL images
typedef uint8_t                   esl_image_ots_flags_t;

/// Object property reserved for internal read-only image objects (if any)
#define ESL_OTS_EMBEDDED_IMAGE    SL_BT_OTS_WRITE_MODE_NONE

/// Object property for standard ESL Image objects
#define ESL_OTS_IMAGE_OBJECT      (SL_BT_OTS_OBJECT_PROPERTY_WRITE_MASK \
                                   | SL_BT_OTS_OBJECT_PROPERTY_TRUNCATE_MASK)

/// Custom image modifier flags definition
#define ESL_IMAGE_FLAGS_NONE          0x00u ///< No modifiers
/// Custom image modifier flags definition for rotation
#define ESL_IMAGE_FLAGS_ROTATION_MASK 0x03u ///< For rotation flag masking
#define ESL_IMAGE_FLAGS_ROTATE_0      0x00u ///< No rotation
#define ESL_IMAGE_FLAGS_ROTATE_90     0x01u ///< Rotate by 90 degree clockwise
#define ESL_IMAGE_FLAGS_ROTATE_180    0x02u ///< Rotate by 180 degree
#define ESL_IMAGE_FLAGS_ROTATE_270    0x03u ///< Rotate by 270 degree clockwise
/// Custom image modifier flags definition for data format
#define ESL_IMAGE_FLAGS_FORMAT_MASK   0x30u ///< For format flag masking
#define ESL_IMAGE_FLAGS_FORMAT_RAW    0x00u ///< Raw (uncompressed) data
#define ESL_IMAGE_FLAGS_FORMAT_LZJB   0x10u ///< LZJB compressed data
#define ESL_IMAGE_FLAGS_FORMAT_RLE    0x20u ///< Run-length encoded data
#define ESL_IMAGE_FLAGS_FORMAT_PNG    0x30u ///< Portable Network Graphics
/// Custom image modifier flags definition for font size adjustments
#define ESL_IMAGE_FLAGS_FONT_MASK     0x0cu ///< For font flag masking
#define ESL_IMAGE_FLAGS_FONT_DEFAULT  0x00u ///< No font size adjustment
#define ESL_IMAGE_FLAGS_FONT_LARGE    0x04u ///< Slightly enlarged fonts (x1.5)
#define ESL_IMAGE_FLAGS_FONT_LARGER   0x08u ///< Enlarged fonts for Hi-DPI (x2)
#define ESL_IMAGE_FLAGS_FONT_LARGEST  0x0cu ///< Largest font (x3)
/// Custom image modifier flags definition for automatic cropping/fitting method
#define ESL_IMAGE_FLAGS_NO_CROP_FIT   0x00u ///< Keep full input image
#define ESL_IMAGE_FLAGS_CROP_FIT      0x40u ///< Crop image for best fitting
/// Custom image modifier flags definition for data bit reversal
#define ESL_IMAGE_FLAGS_NO_BIT_FLIP   0x00u ///< Normal bit order within bytes
#define ESL_IMAGE_FLAGS_BIT_FLIP      0x80u ///< Invert bit order within bytes

/**************************************************************************//**
 * Bluetooth stack event handler for ESL OTS Server component.
 * This one runs parallel to the user implementation (usually in app.c).
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void esl_tag_ots_server_bt_on_event(sl_bt_msg_t *evt);

/**************************************************************************//**
 * Create a custom 128 bit UUID for ESL custom OTS image storage type.
 *
 * @param[in] display_type Assigned Number, see in @ref esl_tag_display_core.h
 * @param[in] ots_flags Custom format modifier flags for the image object
 * @param[in|out] type_uuid Pointer to an OTS Object Type descriptor
 * @return sl_status_t
 *
 * @note The proper type_uuid (expected by the SiLabs ESL Access Point example)
 * is set on success for the vendor specific image storage object, or becomes
 * unspecified on failure.
 *****************************************************************************/
sl_status_t esl_tag_ots_prepare_object_type(esl_display_type_t display_type,
                                            esl_image_ots_flags_t ots_flags,
                                            sl_bt_ots_object_type_t *type);

/**************************************************************************//**
 * Add an ESL Tag image to the list of available OTS objects. Any images on a
 * Tag can be only written after adding it to the OTS object list. This function
 * has to be called from @ref esl_image_add method.
 *
 * @param[in] poperties      OTS Object property flags
 * @param[in] allocated_size Allocated size for ESL Image static object
 * @param[in] type           Pointer to an OTS Object Type descriptor
 * @return    sl_status_t
 *****************************************************************************/
sl_status_t esl_tag_ots_add_object(const sl_bt_ots_object_type_t* type,
                                   uint32_t allocated_size,
                                   uint32_t properties);

#endif // ESL_TAG_OTS_SERVER_H
