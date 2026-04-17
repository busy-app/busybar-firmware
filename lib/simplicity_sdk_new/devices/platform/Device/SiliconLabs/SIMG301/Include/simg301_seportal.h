/**************************************************************************//**
 * @file
 * @brief SIMG301 SEPORTAL register and bit field definitions
 ******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories, Inc. www.silabs.com</b>
 ******************************************************************************
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
 *****************************************************************************/
#ifndef SIMG301_SEPORTAL_H
#define SIMG301_SEPORTAL_H
#define SEPORTAL_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_SEPORTAL SEPORTAL
 * @{
 * @brief SIMG301 SEPORTAL Register Declaration.
 *****************************************************************************/

/** SEPORTAL Register Declaration. */
typedef struct seportal_typedef{
  uint32_t       RESERVED0[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  IPID;                          /**< IP version ID                                      */
  __IOM uint32_t CTRL;                          /**< SEPORTAL Control Register                          */
  __IOM uint32_t STATUS;                        /**< SEPORTAL Status Register                           */
  uint32_t       RESERVED1[1017U];              /**< Reserved for future use                            */
  uint32_t       RESERVED2[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  IPID_SET;                      /**< IP version ID                                      */
  __IOM uint32_t CTRL_SET;                      /**< SEPORTAL Control Register                          */
  __IOM uint32_t STATUS_SET;                    /**< SEPORTAL Status Register                           */
  uint32_t       RESERVED3[1017U];              /**< Reserved for future use                            */
  uint32_t       RESERVED4[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  IPID_CLR;                      /**< IP version ID                                      */
  __IOM uint32_t CTRL_CLR;                      /**< SEPORTAL Control Register                          */
  __IOM uint32_t STATUS_CLR;                    /**< SEPORTAL Status Register                           */
  uint32_t       RESERVED5[1017U];              /**< Reserved for future use                            */
  uint32_t       RESERVED6[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  IPID_TGL;                      /**< IP version ID                                      */
  __IOM uint32_t CTRL_TGL;                      /**< SEPORTAL Control Register                          */
  __IOM uint32_t STATUS_TGL;                    /**< SEPORTAL Status Register                           */
} SEPORTAL_TypeDef;
/** @} End of group SIMG301_SEPORTAL */

/**************************************************************************//**
 * @addtogroup SIMG301_SEPORTAL
 * @{
 * @defgroup SIMG301_SEPORTAL_BitFields SEPORTAL Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SEPORTAL IPID */
#define _SEPORTAL_IPID_RESETVALUE                0x00000001UL                            /**< Default value for SEPORTAL_IPID             */
#define _SEPORTAL_IPID_MASK                      0xFFFFFFFFUL                            /**< Mask for SEPORTAL_IPID                      */
#define _SEPORTAL_IPID_IPVERSION_SHIFT           0                                       /**< Shift value for SEPORTAL_IPVERSION          */
#define _SEPORTAL_IPID_IPVERSION_MASK            0xFFFFFFFFUL                            /**< Bit mask for SEPORTAL_IPVERSION             */
#define _SEPORTAL_IPID_IPVERSION_DEFAULT         0x00000001UL                            /**< Mode DEFAULT for SEPORTAL_IPID              */
#define SEPORTAL_IPID_IPVERSION_DEFAULT          (_SEPORTAL_IPID_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SEPORTAL_IPID      */

/* Bit fields for SEPORTAL CTRL */
#define _SEPORTAL_CTRL_RESETVALUE                0x00000000UL                               /**< Default value for SEPORTAL_CTRL             */
#define _SEPORTAL_CTRL_MASK                      0x00000001UL                               /**< Mask for SEPORTAL_CTRL                      */
#define SEPORTAL_CTRL_LPW0PWRUPREQ               (0x1UL << 0)                               /**< LPW0 Power Up Request                       */
#define _SEPORTAL_CTRL_LPW0PWRUPREQ_SHIFT        0                                          /**< Shift value for SEPORTAL_LPW0PWRUPREQ       */
#define _SEPORTAL_CTRL_LPW0PWRUPREQ_MASK         0x1UL                                      /**< Bit mask for SEPORTAL_LPW0PWRUPREQ          */
#define _SEPORTAL_CTRL_LPW0PWRUPREQ_DEFAULT      0x00000000UL                               /**< Mode DEFAULT for SEPORTAL_CTRL              */
#define SEPORTAL_CTRL_LPW0PWRUPREQ_DEFAULT       (_SEPORTAL_CTRL_LPW0PWRUPREQ_DEFAULT << 0) /**< Shifted mode DEFAULT for SEPORTAL_CTRL      */

/* Bit fields for SEPORTAL STATUS */
#define _SEPORTAL_STATUS_RESETVALUE              0x00000000UL                                 /**< Default value for SEPORTAL_STATUS           */
#define _SEPORTAL_STATUS_MASK                    0x00000001UL                                 /**< Mask for SEPORTAL_STATUS                    */
#define SEPORTAL_STATUS_LPW0PWRUPACK             (0x1UL << 0)                                 /**< LPW0 Power up status                        */
#define _SEPORTAL_STATUS_LPW0PWRUPACK_SHIFT      0                                            /**< Shift value for SEPORTAL_LPW0PWRUPACK       */
#define _SEPORTAL_STATUS_LPW0PWRUPACK_MASK       0x1UL                                        /**< Bit mask for SEPORTAL_LPW0PWRUPACK          */
#define _SEPORTAL_STATUS_LPW0PWRUPACK_DEFAULT    0x00000000UL                                 /**< Mode DEFAULT for SEPORTAL_STATUS            */
#define SEPORTAL_STATUS_LPW0PWRUPACK_DEFAULT     (_SEPORTAL_STATUS_LPW0PWRUPACK_DEFAULT << 0) /**< Shifted mode DEFAULT for SEPORTAL_STATUS    */

/** @} End of group SIMG301_SEPORTAL_BitFields */
/** @} End of group SIMG301_SEPORTAL */
/** @} End of group Parts */

#endif // SIMG301_SEPORTAL_H
