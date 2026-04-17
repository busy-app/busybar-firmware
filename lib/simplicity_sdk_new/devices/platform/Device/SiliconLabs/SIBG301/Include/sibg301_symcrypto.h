/**************************************************************************//**
 * @file
 * @brief SIBG301 SYMCRYPTO register and bit field definitions
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
#ifndef SIBG301_SYMCRYPTO_H
#define SIBG301_SYMCRYPTO_H

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_SYMCRYPTO SYMCRYPTO
 * @{
 * @brief SIBG301 SYMCRYPTO Register Declaration.
 *****************************************************************************/

/** SYMCRYPTO Register Declaration. */
typedef struct symcrypto_typedef{
  __IOM uint32_t FETCHADDR;                     /**< Fetcher Address                                    */
  uint32_t       RESERVED0[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t FETCHLEN;                      /**< Fetcher Length                                     */
  __IOM uint32_t FETCHTAG;                      /**< Fetcher Tag                                        */
  __IOM uint32_t PUSHADDR;                      /**< Pusher Address                                     */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t PUSHLEN;                       /**< Pusher Length                                      */
  __IOM uint32_t IEN;                           /**< Interrupt Enable                                   */
  __IOM uint32_t IEN_SET;                       /**< Interrupt enable set                               */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt enable clear                             */
  __IM uint32_t  IF;                            /**< Interrupt Flags                                    */
  __IM uint32_t  IFMASKED;                      /**< Interrupt status                                   */
  __IOM uint32_t IF_CLR;                        /**< Interrupt status clear                             */
  __IOM uint32_t CTRL;                          /**< Control register                                   */
  __IOM uint32_t CMD;                           /**< Command register                                   */
  __IM uint32_t  STATUS;                        /**< Status register                                    */
  uint32_t       RESERVED2[240U];               /**< Reserved for future use                            */
  __IM uint32_t  SYMCRYPTO_INCL_IPS_HW_CFG;     /**< INCL_IPS_HW_CFG                                    */
  __IM uint32_t  SYMCRYPTO_BA411E_HW_CFG_1;     /**< BA411E_HW_CFG_1                                    */
  __IM uint32_t  SYMCRYPTO_BA411E_HW_CFG_2;     /**< BA411E_HW_CFG_2                                    */
  __IM uint32_t  SYMCRYPTO_BA413_HW_CFG_0;      /**< BA413_HW_CFG_0                                     */
  __IM uint32_t  SYMCRYPTO_BA418_HW_CFG;        /**< BA418_HW_CFG                                       */
  __IM uint32_t  SYMCRYPTO_BA419_HW_CFG;        /**< BA419_HW_CFG                                       */
  __IM uint32_t  SYMCRYPTO_BA424_HW_CFG;        /**< BA424_HW_CFG                                       */
} SYMCRYPTO_TypeDef;
/** @} End of group SIBG301_SYMCRYPTO */

/**************************************************************************//**
 * @addtogroup SIBG301_SYMCRYPTO
 * @{
 * @defgroup SIBG301_SYMCRYPTO_BitFields SYMCRYPTO Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SYMCRYPTO FETCHADDR */
#define _SYMCRYPTO_FETCHADDR_RESETVALUE                                       0x00000000UL                             /**< Default value for SYMCRYPTO_FETCHADDR       */
#define _SYMCRYPTO_FETCHADDR_MASK                                             0xFFFFFFFFUL                             /**< Mask for SYMCRYPTO_FETCHADDR                */
#define _SYMCRYPTO_FETCHADDR_ADDR_SHIFT                                       0                                        /**< Shift value for SYMCRYPTO_ADDR              */
#define _SYMCRYPTO_FETCHADDR_ADDR_MASK                                        0xFFFFFFFFUL                             /**< Bit mask for SYMCRYPTO_ADDR                 */
#define _SYMCRYPTO_FETCHADDR_ADDR_DEFAULT                                     0x00000000UL                             /**< Mode DEFAULT for SYMCRYPTO_FETCHADDR        */
#define SYMCRYPTO_FETCHADDR_ADDR_DEFAULT                                      (_SYMCRYPTO_FETCHADDR_ADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_FETCHADDR*/

/* Bit fields for SYMCRYPTO FETCHLEN */
#define _SYMCRYPTO_FETCHLEN_RESETVALUE                                        0x00000000UL                                  /**< Default value for SYMCRYPTO_FETCHLEN        */
#define _SYMCRYPTO_FETCHLEN_MASK                                              0x7FFFFFFFUL                                  /**< Mask for SYMCRYPTO_FETCHLEN                 */
#define _SYMCRYPTO_FETCHLEN_LENGTH_SHIFT                                      0                                             /**< Shift value for SYMCRYPTO_LENGTH            */
#define _SYMCRYPTO_FETCHLEN_LENGTH_MASK                                       0xFFFFFFFUL                                   /**< Bit mask for SYMCRYPTO_LENGTH               */
#define _SYMCRYPTO_FETCHLEN_LENGTH_DEFAULT                                    0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_FETCHLEN         */
#define SYMCRYPTO_FETCHLEN_LENGTH_DEFAULT                                     (_SYMCRYPTO_FETCHLEN_LENGTH_DEFAULT << 0)     /**< Shifted mode DEFAULT for SYMCRYPTO_FETCHLEN */
#define SYMCRYPTO_FETCHLEN_CONSTADDR                                          (0x1UL << 28)                                 /**< Constant address                            */
#define _SYMCRYPTO_FETCHLEN_CONSTADDR_SHIFT                                   28                                            /**< Shift value for SYMCRYPTO_CONSTADDR         */
#define _SYMCRYPTO_FETCHLEN_CONSTADDR_MASK                                    0x10000000UL                                  /**< Bit mask for SYMCRYPTO_CONSTADDR            */
#define _SYMCRYPTO_FETCHLEN_CONSTADDR_DEFAULT                                 0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_FETCHLEN         */
#define SYMCRYPTO_FETCHLEN_CONSTADDR_DEFAULT                                  (_SYMCRYPTO_FETCHLEN_CONSTADDR_DEFAULT << 28) /**< Shifted mode DEFAULT for SYMCRYPTO_FETCHLEN */
#define SYMCRYPTO_FETCHLEN_REALIGN                                            (0x1UL << 29)                                 /**< Realign lengh                               */
#define _SYMCRYPTO_FETCHLEN_REALIGN_SHIFT                                     29                                            /**< Shift value for SYMCRYPTO_REALIGN           */
#define _SYMCRYPTO_FETCHLEN_REALIGN_MASK                                      0x20000000UL                                  /**< Bit mask for SYMCRYPTO_REALIGN              */
#define _SYMCRYPTO_FETCHLEN_REALIGN_DEFAULT                                   0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_FETCHLEN         */
#define SYMCRYPTO_FETCHLEN_REALIGN_DEFAULT                                    (_SYMCRYPTO_FETCHLEN_REALIGN_DEFAULT << 29)   /**< Shifted mode DEFAULT for SYMCRYPTO_FETCHLEN */
#define SYMCRYPTO_FETCHLEN_ZPADDING                                           (0x1UL << 30)                                 /**< Generate zeroes                             */
#define _SYMCRYPTO_FETCHLEN_ZPADDING_SHIFT                                    30                                            /**< Shift value for SYMCRYPTO_ZPADDING          */
#define _SYMCRYPTO_FETCHLEN_ZPADDING_MASK                                     0x40000000UL                                  /**< Bit mask for SYMCRYPTO_ZPADDING             */
#define _SYMCRYPTO_FETCHLEN_ZPADDING_DEFAULT                                  0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_FETCHLEN         */
#define SYMCRYPTO_FETCHLEN_ZPADDING_DEFAULT                                   (_SYMCRYPTO_FETCHLEN_ZPADDING_DEFAULT << 30)  /**< Shifted mode DEFAULT for SYMCRYPTO_FETCHLEN */

/* Bit fields for SYMCRYPTO FETCHTAG */
#define _SYMCRYPTO_FETCHTAG_RESETVALUE                                        0x00000000UL                           /**< Default value for SYMCRYPTO_FETCHTAG        */
#define _SYMCRYPTO_FETCHTAG_MASK                                              0xFFFFFFFFUL                           /**< Mask for SYMCRYPTO_FETCHTAG                 */
#define _SYMCRYPTO_FETCHTAG_TAG_SHIFT                                         0                                      /**< Shift value for SYMCRYPTO_TAG               */
#define _SYMCRYPTO_FETCHTAG_TAG_MASK                                          0xFFFFFFFFUL                           /**< Bit mask for SYMCRYPTO_TAG                  */
#define _SYMCRYPTO_FETCHTAG_TAG_DEFAULT                                       0x00000000UL                           /**< Mode DEFAULT for SYMCRYPTO_FETCHTAG         */
#define SYMCRYPTO_FETCHTAG_TAG_DEFAULT                                        (_SYMCRYPTO_FETCHTAG_TAG_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_FETCHTAG */

/* Bit fields for SYMCRYPTO PUSHADDR */
#define _SYMCRYPTO_PUSHADDR_RESETVALUE                                        0x00000000UL                            /**< Default value for SYMCRYPTO_PUSHADDR        */
#define _SYMCRYPTO_PUSHADDR_MASK                                              0xFFFFFFFFUL                            /**< Mask for SYMCRYPTO_PUSHADDR                 */
#define _SYMCRYPTO_PUSHADDR_ADDR_SHIFT                                        0                                       /**< Shift value for SYMCRYPTO_ADDR              */
#define _SYMCRYPTO_PUSHADDR_ADDR_MASK                                         0xFFFFFFFFUL                            /**< Bit mask for SYMCRYPTO_ADDR                 */
#define _SYMCRYPTO_PUSHADDR_ADDR_DEFAULT                                      0x00000000UL                            /**< Mode DEFAULT for SYMCRYPTO_PUSHADDR         */
#define SYMCRYPTO_PUSHADDR_ADDR_DEFAULT                                       (_SYMCRYPTO_PUSHADDR_ADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_PUSHADDR */

/* Bit fields for SYMCRYPTO PUSHLEN */
#define _SYMCRYPTO_PUSHLEN_RESETVALUE                                         0x00000000UL                                 /**< Default value for SYMCRYPTO_PUSHLEN         */
#define _SYMCRYPTO_PUSHLEN_MASK                                               0x7FFFFFFFUL                                 /**< Mask for SYMCRYPTO_PUSHLEN                  */
#define _SYMCRYPTO_PUSHLEN_LENGTH_SHIFT                                       0                                            /**< Shift value for SYMCRYPTO_LENGTH            */
#define _SYMCRYPTO_PUSHLEN_LENGTH_MASK                                        0xFFFFFFFUL                                  /**< Bit mask for SYMCRYPTO_LENGTH               */
#define _SYMCRYPTO_PUSHLEN_LENGTH_DEFAULT                                     0x00000000UL                                 /**< Mode DEFAULT for SYMCRYPTO_PUSHLEN          */
#define SYMCRYPTO_PUSHLEN_LENGTH_DEFAULT                                      (_SYMCRYPTO_PUSHLEN_LENGTH_DEFAULT << 0)     /**< Shifted mode DEFAULT for SYMCRYPTO_PUSHLEN  */
#define SYMCRYPTO_PUSHLEN_CONSTADDR                                           (0x1UL << 28)                                /**< Constant address                            */
#define _SYMCRYPTO_PUSHLEN_CONSTADDR_SHIFT                                    28                                           /**< Shift value for SYMCRYPTO_CONSTADDR         */
#define _SYMCRYPTO_PUSHLEN_CONSTADDR_MASK                                     0x10000000UL                                 /**< Bit mask for SYMCRYPTO_CONSTADDR            */
#define _SYMCRYPTO_PUSHLEN_CONSTADDR_DEFAULT                                  0x00000000UL                                 /**< Mode DEFAULT for SYMCRYPTO_PUSHLEN          */
#define SYMCRYPTO_PUSHLEN_CONSTADDR_DEFAULT                                   (_SYMCRYPTO_PUSHLEN_CONSTADDR_DEFAULT << 28) /**< Shifted mode DEFAULT for SYMCRYPTO_PUSHLEN  */
#define SYMCRYPTO_PUSHLEN_REALIGN                                             (0x1UL << 29)                                /**< Realign length                              */
#define _SYMCRYPTO_PUSHLEN_REALIGN_SHIFT                                      29                                           /**< Shift value for SYMCRYPTO_REALIGN           */
#define _SYMCRYPTO_PUSHLEN_REALIGN_MASK                                       0x20000000UL                                 /**< Bit mask for SYMCRYPTO_REALIGN              */
#define _SYMCRYPTO_PUSHLEN_REALIGN_DEFAULT                                    0x00000000UL                                 /**< Mode DEFAULT for SYMCRYPTO_PUSHLEN          */
#define SYMCRYPTO_PUSHLEN_REALIGN_DEFAULT                                     (_SYMCRYPTO_PUSHLEN_REALIGN_DEFAULT << 29)   /**< Shifted mode DEFAULT for SYMCRYPTO_PUSHLEN  */
#define SYMCRYPTO_PUSHLEN_DISCARD                                             (0x1UL << 30)                                /**< Discard data                                */
#define _SYMCRYPTO_PUSHLEN_DISCARD_SHIFT                                      30                                           /**< Shift value for SYMCRYPTO_DISCARD           */
#define _SYMCRYPTO_PUSHLEN_DISCARD_MASK                                       0x40000000UL                                 /**< Bit mask for SYMCRYPTO_DISCARD              */
#define _SYMCRYPTO_PUSHLEN_DISCARD_DEFAULT                                    0x00000000UL                                 /**< Mode DEFAULT for SYMCRYPTO_PUSHLEN          */
#define SYMCRYPTO_PUSHLEN_DISCARD_DEFAULT                                     (_SYMCRYPTO_PUSHLEN_DISCARD_DEFAULT << 30)   /**< Shifted mode DEFAULT for SYMCRYPTO_PUSHLEN  */

/* Bit fields for SYMCRYPTO IEN */
#define _SYMCRYPTO_IEN_RESETVALUE                                             0x00000000UL                                    /**< Default value for SYMCRYPTO_IEN             */
#define _SYMCRYPTO_IEN_MASK                                                   0x0000003FUL                                    /**< Mask for SYMCRYPTO_IEN                      */
#define SYMCRYPTO_IEN_FETCHERENDOFBLOCK                                       (0x1UL << 0)                                    /**< End of block interrupt enable               */
#define _SYMCRYPTO_IEN_FETCHERENDOFBLOCK_SHIFT                                0                                               /**< Shift value for SYMCRYPTO_FETCHERENDOFBLOCK */
#define _SYMCRYPTO_IEN_FETCHERENDOFBLOCK_MASK                                 0x1UL                                           /**< Bit mask for SYMCRYPTO_FETCHERENDOFBLOCK    */
#define _SYMCRYPTO_IEN_FETCHERENDOFBLOCK_DEFAULT                              0x00000000UL                                    /**< Mode DEFAULT for SYMCRYPTO_IEN              */
#define SYMCRYPTO_IEN_FETCHERENDOFBLOCK_DEFAULT                               (_SYMCRYPTO_IEN_FETCHERENDOFBLOCK_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_IEN      */
#define SYMCRYPTO_IEN_FETCHERSTOPPED                                          (0x1UL << 1)                                    /**< Stopped interrupt enable                    */
#define _SYMCRYPTO_IEN_FETCHERSTOPPED_SHIFT                                   1                                               /**< Shift value for SYMCRYPTO_FETCHERSTOPPED    */
#define _SYMCRYPTO_IEN_FETCHERSTOPPED_MASK                                    0x2UL                                           /**< Bit mask for SYMCRYPTO_FETCHERSTOPPED       */
#define _SYMCRYPTO_IEN_FETCHERSTOPPED_DEFAULT                                 0x00000000UL                                    /**< Mode DEFAULT for SYMCRYPTO_IEN              */
#define SYMCRYPTO_IEN_FETCHERSTOPPED_DEFAULT                                  (_SYMCRYPTO_IEN_FETCHERSTOPPED_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_IEN      */
#define SYMCRYPTO_IEN_FETCHERERROR                                            (0x1UL << 2)                                    /**< Error interrupt enable                      */
#define _SYMCRYPTO_IEN_FETCHERERROR_SHIFT                                     2                                               /**< Shift value for SYMCRYPTO_FETCHERERROR      */
#define _SYMCRYPTO_IEN_FETCHERERROR_MASK                                      0x4UL                                           /**< Bit mask for SYMCRYPTO_FETCHERERROR         */
#define _SYMCRYPTO_IEN_FETCHERERROR_DEFAULT                                   0x00000000UL                                    /**< Mode DEFAULT for SYMCRYPTO_IEN              */
#define SYMCRYPTO_IEN_FETCHERERROR_DEFAULT                                    (_SYMCRYPTO_IEN_FETCHERERROR_DEFAULT << 2)      /**< Shifted mode DEFAULT for SYMCRYPTO_IEN      */
#define SYMCRYPTO_IEN_PUSHERENDOFBLOCK                                        (0x1UL << 3)                                    /**< End of block interrupt enable               */
#define _SYMCRYPTO_IEN_PUSHERENDOFBLOCK_SHIFT                                 3                                               /**< Shift value for SYMCRYPTO_PUSHERENDOFBLOCK  */
#define _SYMCRYPTO_IEN_PUSHERENDOFBLOCK_MASK                                  0x8UL                                           /**< Bit mask for SYMCRYPTO_PUSHERENDOFBLOCK     */
#define _SYMCRYPTO_IEN_PUSHERENDOFBLOCK_DEFAULT                               0x00000000UL                                    /**< Mode DEFAULT for SYMCRYPTO_IEN              */
#define SYMCRYPTO_IEN_PUSHERENDOFBLOCK_DEFAULT                                (_SYMCRYPTO_IEN_PUSHERENDOFBLOCK_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYMCRYPTO_IEN      */
#define SYMCRYPTO_IEN_PUSHERSTOPPED                                           (0x1UL << 4)                                    /**< Stopped interrupt enable                    */
#define _SYMCRYPTO_IEN_PUSHERSTOPPED_SHIFT                                    4                                               /**< Shift value for SYMCRYPTO_PUSHERSTOPPED     */
#define _SYMCRYPTO_IEN_PUSHERSTOPPED_MASK                                     0x10UL                                          /**< Bit mask for SYMCRYPTO_PUSHERSTOPPED        */
#define _SYMCRYPTO_IEN_PUSHERSTOPPED_DEFAULT                                  0x00000000UL                                    /**< Mode DEFAULT for SYMCRYPTO_IEN              */
#define SYMCRYPTO_IEN_PUSHERSTOPPED_DEFAULT                                   (_SYMCRYPTO_IEN_PUSHERSTOPPED_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_IEN      */
#define SYMCRYPTO_IEN_PUSHERERROR                                             (0x1UL << 5)                                    /**< Error interrupt enable                      */
#define _SYMCRYPTO_IEN_PUSHERERROR_SHIFT                                      5                                               /**< Shift value for SYMCRYPTO_PUSHERERROR       */
#define _SYMCRYPTO_IEN_PUSHERERROR_MASK                                       0x20UL                                          /**< Bit mask for SYMCRYPTO_PUSHERERROR          */
#define _SYMCRYPTO_IEN_PUSHERERROR_DEFAULT                                    0x00000000UL                                    /**< Mode DEFAULT for SYMCRYPTO_IEN              */
#define SYMCRYPTO_IEN_PUSHERERROR_DEFAULT                                     (_SYMCRYPTO_IEN_PUSHERERROR_DEFAULT << 5)       /**< Shifted mode DEFAULT for SYMCRYPTO_IEN      */

/* Bit fields for SYMCRYPTO IEN_SET */
#define _SYMCRYPTO_IEN_SET_RESETVALUE                                         0x00000000UL                                        /**< Default value for SYMCRYPTO_IEN_SET         */
#define _SYMCRYPTO_IEN_SET_MASK                                               0x0000003FUL                                        /**< Mask for SYMCRYPTO_IEN_SET                  */
#define SYMCRYPTO_IEN_SET_FETCHERENDOFBLOCK                                   (0x1UL << 0)                                        /**< End of block interrupt enable set           */
#define _SYMCRYPTO_IEN_SET_FETCHERENDOFBLOCK_SHIFT                            0                                                   /**< Shift value for SYMCRYPTO_FETCHERENDOFBLOCK */
#define _SYMCRYPTO_IEN_SET_FETCHERENDOFBLOCK_MASK                             0x1UL                                               /**< Bit mask for SYMCRYPTO_FETCHERENDOFBLOCK    */
#define _SYMCRYPTO_IEN_SET_FETCHERENDOFBLOCK_DEFAULT                          0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_SET          */
#define SYMCRYPTO_IEN_SET_FETCHERENDOFBLOCK_DEFAULT                           (_SYMCRYPTO_IEN_SET_FETCHERENDOFBLOCK_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_SET  */
#define SYMCRYPTO_IEN_SET_FETCHERSTOPPED                                      (0x1UL << 1)                                        /**< Stopped interrupt enable set                */
#define _SYMCRYPTO_IEN_SET_FETCHERSTOPPED_SHIFT                               1                                                   /**< Shift value for SYMCRYPTO_FETCHERSTOPPED    */
#define _SYMCRYPTO_IEN_SET_FETCHERSTOPPED_MASK                                0x2UL                                               /**< Bit mask for SYMCRYPTO_FETCHERSTOPPED       */
#define _SYMCRYPTO_IEN_SET_FETCHERSTOPPED_DEFAULT                             0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_SET          */
#define SYMCRYPTO_IEN_SET_FETCHERSTOPPED_DEFAULT                              (_SYMCRYPTO_IEN_SET_FETCHERSTOPPED_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_SET  */
#define SYMCRYPTO_IEN_SET_FETCHERERROR                                        (0x1UL << 2)                                        /**< Error interrupt enable set                  */
#define _SYMCRYPTO_IEN_SET_FETCHERERROR_SHIFT                                 2                                                   /**< Shift value for SYMCRYPTO_FETCHERERROR      */
#define _SYMCRYPTO_IEN_SET_FETCHERERROR_MASK                                  0x4UL                                               /**< Bit mask for SYMCRYPTO_FETCHERERROR         */
#define _SYMCRYPTO_IEN_SET_FETCHERERROR_DEFAULT                               0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_SET          */
#define SYMCRYPTO_IEN_SET_FETCHERERROR_DEFAULT                                (_SYMCRYPTO_IEN_SET_FETCHERERROR_DEFAULT << 2)      /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_SET  */
#define SYMCRYPTO_IEN_SET_PUSHERENDOFBLOCK                                    (0x1UL << 3)                                        /**< End of block interrupt enable set           */
#define _SYMCRYPTO_IEN_SET_PUSHERENDOFBLOCK_SHIFT                             3                                                   /**< Shift value for SYMCRYPTO_PUSHERENDOFBLOCK  */
#define _SYMCRYPTO_IEN_SET_PUSHERENDOFBLOCK_MASK                              0x8UL                                               /**< Bit mask for SYMCRYPTO_PUSHERENDOFBLOCK     */
#define _SYMCRYPTO_IEN_SET_PUSHERENDOFBLOCK_DEFAULT                           0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_SET          */
#define SYMCRYPTO_IEN_SET_PUSHERENDOFBLOCK_DEFAULT                            (_SYMCRYPTO_IEN_SET_PUSHERENDOFBLOCK_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_SET  */
#define SYMCRYPTO_IEN_SET_PUSHERSTOPPED                                       (0x1UL << 4)                                        /**< Stopped interrupt enable set                */
#define _SYMCRYPTO_IEN_SET_PUSHERSTOPPED_SHIFT                                4                                                   /**< Shift value for SYMCRYPTO_PUSHERSTOPPED     */
#define _SYMCRYPTO_IEN_SET_PUSHERSTOPPED_MASK                                 0x10UL                                              /**< Bit mask for SYMCRYPTO_PUSHERSTOPPED        */
#define _SYMCRYPTO_IEN_SET_PUSHERSTOPPED_DEFAULT                              0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_SET          */
#define SYMCRYPTO_IEN_SET_PUSHERSTOPPED_DEFAULT                               (_SYMCRYPTO_IEN_SET_PUSHERSTOPPED_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_SET  */
#define SYMCRYPTO_IEN_SET_PUSHERERROR                                         (0x1UL << 5)                                        /**< Error interrupt enable set                  */
#define _SYMCRYPTO_IEN_SET_PUSHERERROR_SHIFT                                  5                                                   /**< Shift value for SYMCRYPTO_PUSHERERROR       */
#define _SYMCRYPTO_IEN_SET_PUSHERERROR_MASK                                   0x20UL                                              /**< Bit mask for SYMCRYPTO_PUSHERERROR          */
#define _SYMCRYPTO_IEN_SET_PUSHERERROR_DEFAULT                                0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_SET          */
#define SYMCRYPTO_IEN_SET_PUSHERERROR_DEFAULT                                 (_SYMCRYPTO_IEN_SET_PUSHERERROR_DEFAULT << 5)       /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_SET  */

/* Bit fields for SYMCRYPTO IEN_CLR */
#define _SYMCRYPTO_IEN_CLR_RESETVALUE                                         0x00000000UL                                        /**< Default value for SYMCRYPTO_IEN_CLR         */
#define _SYMCRYPTO_IEN_CLR_MASK                                               0x0000003FUL                                        /**< Mask for SYMCRYPTO_IEN_CLR                  */
#define SYMCRYPTO_IEN_CLR_FETCHERENDOFBLOCK                                   (0x1UL << 0)                                        /**< End of block interrupt enable clear         */
#define _SYMCRYPTO_IEN_CLR_FETCHERENDOFBLOCK_SHIFT                            0                                                   /**< Shift value for SYMCRYPTO_FETCHERENDOFBLOCK */
#define _SYMCRYPTO_IEN_CLR_FETCHERENDOFBLOCK_MASK                             0x1UL                                               /**< Bit mask for SYMCRYPTO_FETCHERENDOFBLOCK    */
#define _SYMCRYPTO_IEN_CLR_FETCHERENDOFBLOCK_DEFAULT                          0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_CLR          */
#define SYMCRYPTO_IEN_CLR_FETCHERENDOFBLOCK_DEFAULT                           (_SYMCRYPTO_IEN_CLR_FETCHERENDOFBLOCK_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_CLR  */
#define SYMCRYPTO_IEN_CLR_FETCHERSTOPPED                                      (0x1UL << 1)                                        /**< Stopped interrupt enable clear              */
#define _SYMCRYPTO_IEN_CLR_FETCHERSTOPPED_SHIFT                               1                                                   /**< Shift value for SYMCRYPTO_FETCHERSTOPPED    */
#define _SYMCRYPTO_IEN_CLR_FETCHERSTOPPED_MASK                                0x2UL                                               /**< Bit mask for SYMCRYPTO_FETCHERSTOPPED       */
#define _SYMCRYPTO_IEN_CLR_FETCHERSTOPPED_DEFAULT                             0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_CLR          */
#define SYMCRYPTO_IEN_CLR_FETCHERSTOPPED_DEFAULT                              (_SYMCRYPTO_IEN_CLR_FETCHERSTOPPED_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_CLR  */
#define SYMCRYPTO_IEN_CLR_FETCHERERROR                                        (0x1UL << 2)                                        /**< Error interrupt enable clear                */
#define _SYMCRYPTO_IEN_CLR_FETCHERERROR_SHIFT                                 2                                                   /**< Shift value for SYMCRYPTO_FETCHERERROR      */
#define _SYMCRYPTO_IEN_CLR_FETCHERERROR_MASK                                  0x4UL                                               /**< Bit mask for SYMCRYPTO_FETCHERERROR         */
#define _SYMCRYPTO_IEN_CLR_FETCHERERROR_DEFAULT                               0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_CLR          */
#define SYMCRYPTO_IEN_CLR_FETCHERERROR_DEFAULT                                (_SYMCRYPTO_IEN_CLR_FETCHERERROR_DEFAULT << 2)      /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_CLR  */
#define SYMCRYPTO_IEN_CLR_PUSHERENDOFBLOCK                                    (0x1UL << 3)                                        /**< End of block interrupt enable clear         */
#define _SYMCRYPTO_IEN_CLR_PUSHERENDOFBLOCK_SHIFT                             3                                                   /**< Shift value for SYMCRYPTO_PUSHERENDOFBLOCK  */
#define _SYMCRYPTO_IEN_CLR_PUSHERENDOFBLOCK_MASK                              0x8UL                                               /**< Bit mask for SYMCRYPTO_PUSHERENDOFBLOCK     */
#define _SYMCRYPTO_IEN_CLR_PUSHERENDOFBLOCK_DEFAULT                           0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_CLR          */
#define SYMCRYPTO_IEN_CLR_PUSHERENDOFBLOCK_DEFAULT                            (_SYMCRYPTO_IEN_CLR_PUSHERENDOFBLOCK_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_CLR  */
#define SYMCRYPTO_IEN_CLR_PUSHERSTOPPED                                       (0x1UL << 4)                                        /**< Stopped interrupt enable clear              */
#define _SYMCRYPTO_IEN_CLR_PUSHERSTOPPED_SHIFT                                4                                                   /**< Shift value for SYMCRYPTO_PUSHERSTOPPED     */
#define _SYMCRYPTO_IEN_CLR_PUSHERSTOPPED_MASK                                 0x10UL                                              /**< Bit mask for SYMCRYPTO_PUSHERSTOPPED        */
#define _SYMCRYPTO_IEN_CLR_PUSHERSTOPPED_DEFAULT                              0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_CLR          */
#define SYMCRYPTO_IEN_CLR_PUSHERSTOPPED_DEFAULT                               (_SYMCRYPTO_IEN_CLR_PUSHERSTOPPED_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_CLR  */
#define SYMCRYPTO_IEN_CLR_PUSHERERROR                                         (0x1UL << 5)                                        /**< Error interrupt enable clear                */
#define _SYMCRYPTO_IEN_CLR_PUSHERERROR_SHIFT                                  5                                                   /**< Shift value for SYMCRYPTO_PUSHERERROR       */
#define _SYMCRYPTO_IEN_CLR_PUSHERERROR_MASK                                   0x20UL                                              /**< Bit mask for SYMCRYPTO_PUSHERERROR          */
#define _SYMCRYPTO_IEN_CLR_PUSHERERROR_DEFAULT                                0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_IEN_CLR          */
#define SYMCRYPTO_IEN_CLR_PUSHERERROR_DEFAULT                                 (_SYMCRYPTO_IEN_CLR_PUSHERERROR_DEFAULT << 5)       /**< Shifted mode DEFAULT for SYMCRYPTO_IEN_CLR  */

/* Bit fields for SYMCRYPTO IF */
#define _SYMCRYPTO_IF_RESETVALUE                                              0x00000000UL                                   /**< Default value for SYMCRYPTO_IF              */
#define _SYMCRYPTO_IF_MASK                                                    0x0000003FUL                                   /**< Mask for SYMCRYPTO_IF                       */
#define SYMCRYPTO_IF_FETCHERENDOFBLOCK                                        (0x1UL << 0)                                   /**< End of block interrupt flag                 */
#define _SYMCRYPTO_IF_FETCHERENDOFBLOCK_SHIFT                                 0                                              /**< Shift value for SYMCRYPTO_FETCHERENDOFBLOCK */
#define _SYMCRYPTO_IF_FETCHERENDOFBLOCK_MASK                                  0x1UL                                          /**< Bit mask for SYMCRYPTO_FETCHERENDOFBLOCK    */
#define _SYMCRYPTO_IF_FETCHERENDOFBLOCK_DEFAULT                               0x00000000UL                                   /**< Mode DEFAULT for SYMCRYPTO_IF               */
#define SYMCRYPTO_IF_FETCHERENDOFBLOCK_DEFAULT                                (_SYMCRYPTO_IF_FETCHERENDOFBLOCK_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_IF       */
#define SYMCRYPTO_IF_FETCHERSTOPPED                                           (0x1UL << 1)                                   /**< Stopped interrupt flag                      */
#define _SYMCRYPTO_IF_FETCHERSTOPPED_SHIFT                                    1                                              /**< Shift value for SYMCRYPTO_FETCHERSTOPPED    */
#define _SYMCRYPTO_IF_FETCHERSTOPPED_MASK                                     0x2UL                                          /**< Bit mask for SYMCRYPTO_FETCHERSTOPPED       */
#define _SYMCRYPTO_IF_FETCHERSTOPPED_DEFAULT                                  0x00000000UL                                   /**< Mode DEFAULT for SYMCRYPTO_IF               */
#define SYMCRYPTO_IF_FETCHERSTOPPED_DEFAULT                                   (_SYMCRYPTO_IF_FETCHERSTOPPED_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_IF       */
#define SYMCRYPTO_IF_FETCHERERROR                                             (0x1UL << 2)                                   /**< Error interrupt flag                        */
#define _SYMCRYPTO_IF_FETCHERERROR_SHIFT                                      2                                              /**< Shift value for SYMCRYPTO_FETCHERERROR      */
#define _SYMCRYPTO_IF_FETCHERERROR_MASK                                       0x4UL                                          /**< Bit mask for SYMCRYPTO_FETCHERERROR         */
#define _SYMCRYPTO_IF_FETCHERERROR_DEFAULT                                    0x00000000UL                                   /**< Mode DEFAULT for SYMCRYPTO_IF               */
#define SYMCRYPTO_IF_FETCHERERROR_DEFAULT                                     (_SYMCRYPTO_IF_FETCHERERROR_DEFAULT << 2)      /**< Shifted mode DEFAULT for SYMCRYPTO_IF       */
#define SYMCRYPTO_IF_PUSHERENDOFBLOCK                                         (0x1UL << 3)                                   /**< End of block interrupt flag                 */
#define _SYMCRYPTO_IF_PUSHERENDOFBLOCK_SHIFT                                  3                                              /**< Shift value for SYMCRYPTO_PUSHERENDOFBLOCK  */
#define _SYMCRYPTO_IF_PUSHERENDOFBLOCK_MASK                                   0x8UL                                          /**< Bit mask for SYMCRYPTO_PUSHERENDOFBLOCK     */
#define _SYMCRYPTO_IF_PUSHERENDOFBLOCK_DEFAULT                                0x00000000UL                                   /**< Mode DEFAULT for SYMCRYPTO_IF               */
#define SYMCRYPTO_IF_PUSHERENDOFBLOCK_DEFAULT                                 (_SYMCRYPTO_IF_PUSHERENDOFBLOCK_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYMCRYPTO_IF       */
#define SYMCRYPTO_IF_PUSHERSTOPPED                                            (0x1UL << 4)                                   /**< Stopped interrupt flag                      */
#define _SYMCRYPTO_IF_PUSHERSTOPPED_SHIFT                                     4                                              /**< Shift value for SYMCRYPTO_PUSHERSTOPPED     */
#define _SYMCRYPTO_IF_PUSHERSTOPPED_MASK                                      0x10UL                                         /**< Bit mask for SYMCRYPTO_PUSHERSTOPPED        */
#define _SYMCRYPTO_IF_PUSHERSTOPPED_DEFAULT                                   0x00000000UL                                   /**< Mode DEFAULT for SYMCRYPTO_IF               */
#define SYMCRYPTO_IF_PUSHERSTOPPED_DEFAULT                                    (_SYMCRYPTO_IF_PUSHERSTOPPED_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_IF       */
#define SYMCRYPTO_IF_PUSHERERROR                                              (0x1UL << 5)                                   /**< Error interrupt flag                        */
#define _SYMCRYPTO_IF_PUSHERERROR_SHIFT                                       5                                              /**< Shift value for SYMCRYPTO_PUSHERERROR       */
#define _SYMCRYPTO_IF_PUSHERERROR_MASK                                        0x20UL                                         /**< Bit mask for SYMCRYPTO_PUSHERERROR          */
#define _SYMCRYPTO_IF_PUSHERERROR_DEFAULT                                     0x00000000UL                                   /**< Mode DEFAULT for SYMCRYPTO_IF               */
#define SYMCRYPTO_IF_PUSHERERROR_DEFAULT                                      (_SYMCRYPTO_IF_PUSHERERROR_DEFAULT << 5)       /**< Shifted mode DEFAULT for SYMCRYPTO_IF       */

/* Bit fields for SYMCRYPTO IFMASKED */
#define _SYMCRYPTO_IFMASKED_RESETVALUE                                        0x00000000UL                                         /**< Default value for SYMCRYPTO_IFMASKED        */
#define _SYMCRYPTO_IFMASKED_MASK                                              0x0000003FUL                                         /**< Mask for SYMCRYPTO_IFMASKED                 */
#define SYMCRYPTO_IFMASKED_FETCHERENDOFBLOCK                                  (0x1UL << 0)                                         /**< End of block interrupt flag masked          */
#define _SYMCRYPTO_IFMASKED_FETCHERENDOFBLOCK_SHIFT                           0                                                    /**< Shift value for SYMCRYPTO_FETCHERENDOFBLOCK */
#define _SYMCRYPTO_IFMASKED_FETCHERENDOFBLOCK_MASK                            0x1UL                                                /**< Bit mask for SYMCRYPTO_FETCHERENDOFBLOCK    */
#define _SYMCRYPTO_IFMASKED_FETCHERENDOFBLOCK_DEFAULT                         0x00000000UL                                         /**< Mode DEFAULT for SYMCRYPTO_IFMASKED         */
#define SYMCRYPTO_IFMASKED_FETCHERENDOFBLOCK_DEFAULT                          (_SYMCRYPTO_IFMASKED_FETCHERENDOFBLOCK_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_IFMASKED */
#define SYMCRYPTO_IFMASKED_FETCHERSTOPPED                                     (0x1UL << 1)                                         /**< Stopped interrupt flag masked               */
#define _SYMCRYPTO_IFMASKED_FETCHERSTOPPED_SHIFT                              1                                                    /**< Shift value for SYMCRYPTO_FETCHERSTOPPED    */
#define _SYMCRYPTO_IFMASKED_FETCHERSTOPPED_MASK                               0x2UL                                                /**< Bit mask for SYMCRYPTO_FETCHERSTOPPED       */
#define _SYMCRYPTO_IFMASKED_FETCHERSTOPPED_DEFAULT                            0x00000000UL                                         /**< Mode DEFAULT for SYMCRYPTO_IFMASKED         */
#define SYMCRYPTO_IFMASKED_FETCHERSTOPPED_DEFAULT                             (_SYMCRYPTO_IFMASKED_FETCHERSTOPPED_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_IFMASKED */
#define SYMCRYPTO_IFMASKED_FETCHERERROR                                       (0x1UL << 2)                                         /**< Error interrupt flag masked                 */
#define _SYMCRYPTO_IFMASKED_FETCHERERROR_SHIFT                                2                                                    /**< Shift value for SYMCRYPTO_FETCHERERROR      */
#define _SYMCRYPTO_IFMASKED_FETCHERERROR_MASK                                 0x4UL                                                /**< Bit mask for SYMCRYPTO_FETCHERERROR         */
#define _SYMCRYPTO_IFMASKED_FETCHERERROR_DEFAULT                              0x00000000UL                                         /**< Mode DEFAULT for SYMCRYPTO_IFMASKED         */
#define SYMCRYPTO_IFMASKED_FETCHERERROR_DEFAULT                               (_SYMCRYPTO_IFMASKED_FETCHERERROR_DEFAULT << 2)      /**< Shifted mode DEFAULT for SYMCRYPTO_IFMASKED */
#define SYMCRYPTO_IFMASKED_PUSHERENDOFBLOCK                                   (0x1UL << 3)                                         /**< End of block interrupt flag masked          */
#define _SYMCRYPTO_IFMASKED_PUSHERENDOFBLOCK_SHIFT                            3                                                    /**< Shift value for SYMCRYPTO_PUSHERENDOFBLOCK  */
#define _SYMCRYPTO_IFMASKED_PUSHERENDOFBLOCK_MASK                             0x8UL                                                /**< Bit mask for SYMCRYPTO_PUSHERENDOFBLOCK     */
#define _SYMCRYPTO_IFMASKED_PUSHERENDOFBLOCK_DEFAULT                          0x00000000UL                                         /**< Mode DEFAULT for SYMCRYPTO_IFMASKED         */
#define SYMCRYPTO_IFMASKED_PUSHERENDOFBLOCK_DEFAULT                           (_SYMCRYPTO_IFMASKED_PUSHERENDOFBLOCK_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYMCRYPTO_IFMASKED */
#define SYMCRYPTO_IFMASKED_PUSHERSTOPPED                                      (0x1UL << 4)                                         /**< Stopped interrupt flag masked               */
#define _SYMCRYPTO_IFMASKED_PUSHERSTOPPED_SHIFT                               4                                                    /**< Shift value for SYMCRYPTO_PUSHERSTOPPED     */
#define _SYMCRYPTO_IFMASKED_PUSHERSTOPPED_MASK                                0x10UL                                               /**< Bit mask for SYMCRYPTO_PUSHERSTOPPED        */
#define _SYMCRYPTO_IFMASKED_PUSHERSTOPPED_DEFAULT                             0x00000000UL                                         /**< Mode DEFAULT for SYMCRYPTO_IFMASKED         */
#define SYMCRYPTO_IFMASKED_PUSHERSTOPPED_DEFAULT                              (_SYMCRYPTO_IFMASKED_PUSHERSTOPPED_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_IFMASKED */
#define SYMCRYPTO_IFMASKED_PUSHERERROR                                        (0x1UL << 5)                                         /**< Error interrupt flag masked                 */
#define _SYMCRYPTO_IFMASKED_PUSHERERROR_SHIFT                                 5                                                    /**< Shift value for SYMCRYPTO_PUSHERERROR       */
#define _SYMCRYPTO_IFMASKED_PUSHERERROR_MASK                                  0x20UL                                               /**< Bit mask for SYMCRYPTO_PUSHERERROR          */
#define _SYMCRYPTO_IFMASKED_PUSHERERROR_DEFAULT                               0x00000000UL                                         /**< Mode DEFAULT for SYMCRYPTO_IFMASKED         */
#define SYMCRYPTO_IFMASKED_PUSHERERROR_DEFAULT                                (_SYMCRYPTO_IFMASKED_PUSHERERROR_DEFAULT << 5)       /**< Shifted mode DEFAULT for SYMCRYPTO_IFMASKED */

/* Bit fields for SYMCRYPTO IF_CLR */
#define _SYMCRYPTO_IF_CLR_RESETVALUE                                          0x00000000UL                                          /**< Default value for SYMCRYPTO_IF_CLR          */
#define _SYMCRYPTO_IF_CLR_MASK                                                0x0000003FUL                                          /**< Mask for SYMCRYPTO_IF_CLR                   */
#define SYMCRYPTO_IF_CLR_FETCHERENDOFBLOCKIFC                                 (0x1UL << 0)                                          /**< End of block interrupt flag clear           */
#define _SYMCRYPTO_IF_CLR_FETCHERENDOFBLOCKIFC_SHIFT                          0                                                     /**< Shift value for SYMCRYPTO_FETCHERENDOFBLOCKIFC*/
#define _SYMCRYPTO_IF_CLR_FETCHERENDOFBLOCKIFC_MASK                           0x1UL                                                 /**< Bit mask for SYMCRYPTO_FETCHERENDOFBLOCKIFC */
#define _SYMCRYPTO_IF_CLR_FETCHERENDOFBLOCKIFC_DEFAULT                        0x00000000UL                                          /**< Mode DEFAULT for SYMCRYPTO_IF_CLR           */
#define SYMCRYPTO_IF_CLR_FETCHERENDOFBLOCKIFC_DEFAULT                         (_SYMCRYPTO_IF_CLR_FETCHERENDOFBLOCKIFC_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_IF_CLR   */
#define SYMCRYPTO_IF_CLR_FETCHERSTOPPEDIFC                                    (0x1UL << 1)                                          /**< Stopped interrupt flag clear                */
#define _SYMCRYPTO_IF_CLR_FETCHERSTOPPEDIFC_SHIFT                             1                                                     /**< Shift value for SYMCRYPTO_FETCHERSTOPPEDIFC */
#define _SYMCRYPTO_IF_CLR_FETCHERSTOPPEDIFC_MASK                              0x2UL                                                 /**< Bit mask for SYMCRYPTO_FETCHERSTOPPEDIFC    */
#define _SYMCRYPTO_IF_CLR_FETCHERSTOPPEDIFC_DEFAULT                           0x00000000UL                                          /**< Mode DEFAULT for SYMCRYPTO_IF_CLR           */
#define SYMCRYPTO_IF_CLR_FETCHERSTOPPEDIFC_DEFAULT                            (_SYMCRYPTO_IF_CLR_FETCHERSTOPPEDIFC_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_IF_CLR   */
#define SYMCRYPTO_IF_CLR_FETCHERERRORIFC                                      (0x1UL << 2)                                          /**< Error interrupt flag clear                  */
#define _SYMCRYPTO_IF_CLR_FETCHERERRORIFC_SHIFT                               2                                                     /**< Shift value for SYMCRYPTO_FETCHERERRORIFC   */
#define _SYMCRYPTO_IF_CLR_FETCHERERRORIFC_MASK                                0x4UL                                                 /**< Bit mask for SYMCRYPTO_FETCHERERRORIFC      */
#define _SYMCRYPTO_IF_CLR_FETCHERERRORIFC_DEFAULT                             0x00000000UL                                          /**< Mode DEFAULT for SYMCRYPTO_IF_CLR           */
#define SYMCRYPTO_IF_CLR_FETCHERERRORIFC_DEFAULT                              (_SYMCRYPTO_IF_CLR_FETCHERERRORIFC_DEFAULT << 2)      /**< Shifted mode DEFAULT for SYMCRYPTO_IF_CLR   */
#define SYMCRYPTO_IF_CLR_PUSHERENDOFBLOCKIFC                                  (0x1UL << 3)                                          /**< End of block interrupt flag clear           */
#define _SYMCRYPTO_IF_CLR_PUSHERENDOFBLOCKIFC_SHIFT                           3                                                     /**< Shift value for SYMCRYPTO_PUSHERENDOFBLOCKIFC*/
#define _SYMCRYPTO_IF_CLR_PUSHERENDOFBLOCKIFC_MASK                            0x8UL                                                 /**< Bit mask for SYMCRYPTO_PUSHERENDOFBLOCKIFC  */
#define _SYMCRYPTO_IF_CLR_PUSHERENDOFBLOCKIFC_DEFAULT                         0x00000000UL                                          /**< Mode DEFAULT for SYMCRYPTO_IF_CLR           */
#define SYMCRYPTO_IF_CLR_PUSHERENDOFBLOCKIFC_DEFAULT                          (_SYMCRYPTO_IF_CLR_PUSHERENDOFBLOCKIFC_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYMCRYPTO_IF_CLR   */
#define SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC                                     (0x1UL << 4)                                          /**< Stopped interrupt flag clear                */
#define _SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC_SHIFT                              4                                                     /**< Shift value for SYMCRYPTO_PUSHERSTOPPEDIFC  */
#define _SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC_MASK                               0x10UL                                                /**< Bit mask for SYMCRYPTO_PUSHERSTOPPEDIFC     */
#define _SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC_DEFAULT                            0x00000000UL                                          /**< Mode DEFAULT for SYMCRYPTO_IF_CLR           */
#define SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC_DEFAULT                             (_SYMCRYPTO_IF_CLR_PUSHERSTOPPEDIFC_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_IF_CLR   */
#define SYMCRYPTO_IF_CLR_PUSHERERRORIFC                                       (0x1UL << 5)                                          /**< Error interrupt flag clear                  */
#define _SYMCRYPTO_IF_CLR_PUSHERERRORIFC_SHIFT                                5                                                     /**< Shift value for SYMCRYPTO_PUSHERERRORIFC    */
#define _SYMCRYPTO_IF_CLR_PUSHERERRORIFC_MASK                                 0x20UL                                                /**< Bit mask for SYMCRYPTO_PUSHERERRORIFC       */
#define _SYMCRYPTO_IF_CLR_PUSHERERRORIFC_DEFAULT                              0x00000000UL                                          /**< Mode DEFAULT for SYMCRYPTO_IF_CLR           */
#define SYMCRYPTO_IF_CLR_PUSHERERRORIFC_DEFAULT                               (_SYMCRYPTO_IF_CLR_PUSHERERRORIFC_DEFAULT << 5)       /**< Shifted mode DEFAULT for SYMCRYPTO_IF_CLR   */

/* Bit fields for SYMCRYPTO CTRL */
#define _SYMCRYPTO_CTRL_RESETVALUE                                            0x00000000UL                                        /**< Default value for SYMCRYPTO_CTRL            */
#define _SYMCRYPTO_CTRL_MASK                                                  0x0000001FUL                                        /**< Mask for SYMCRYPTO_CTRL                     */
#define SYMCRYPTO_CTRL_FETCHERSCATTERGATHER                                   (0x1UL << 0)                                        /**< Fetcher scatter/gather                      */
#define _SYMCRYPTO_CTRL_FETCHERSCATTERGATHER_SHIFT                            0                                                   /**< Shift value for SYMCRYPTO_FETCHERSCATTERGATHER*/
#define _SYMCRYPTO_CTRL_FETCHERSCATTERGATHER_MASK                             0x1UL                                               /**< Bit mask for SYMCRYPTO_FETCHERSCATTERGATHER */
#define _SYMCRYPTO_CTRL_FETCHERSCATTERGATHER_DEFAULT                          0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_CTRL             */
#define SYMCRYPTO_CTRL_FETCHERSCATTERGATHER_DEFAULT                           (_SYMCRYPTO_CTRL_FETCHERSCATTERGATHER_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_CTRL     */
#define SYMCRYPTO_CTRL_PUSHERSCATTERGATHER                                    (0x1UL << 1)                                        /**< Pusher scatter/gather                       */
#define _SYMCRYPTO_CTRL_PUSHERSCATTERGATHER_SHIFT                             1                                                   /**< Shift value for SYMCRYPTO_PUSHERSCATTERGATHER*/
#define _SYMCRYPTO_CTRL_PUSHERSCATTERGATHER_MASK                              0x2UL                                               /**< Bit mask for SYMCRYPTO_PUSHERSCATTERGATHER  */
#define _SYMCRYPTO_CTRL_PUSHERSCATTERGATHER_DEFAULT                           0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_CTRL             */
#define SYMCRYPTO_CTRL_PUSHERSCATTERGATHER_DEFAULT                            (_SYMCRYPTO_CTRL_PUSHERSCATTERGATHER_DEFAULT << 1)  /**< Shifted mode DEFAULT for SYMCRYPTO_CTRL     */
#define SYMCRYPTO_CTRL_STOPFETCHER                                            (0x1UL << 2)                                        /**< Stop fetcher                                */
#define _SYMCRYPTO_CTRL_STOPFETCHER_SHIFT                                     2                                                   /**< Shift value for SYMCRYPTO_STOPFETCHER       */
#define _SYMCRYPTO_CTRL_STOPFETCHER_MASK                                      0x4UL                                               /**< Bit mask for SYMCRYPTO_STOPFETCHER          */
#define _SYMCRYPTO_CTRL_STOPFETCHER_DEFAULT                                   0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_CTRL             */
#define SYMCRYPTO_CTRL_STOPFETCHER_DEFAULT                                    (_SYMCRYPTO_CTRL_STOPFETCHER_DEFAULT << 2)          /**< Shifted mode DEFAULT for SYMCRYPTO_CTRL     */
#define SYMCRYPTO_CTRL_STOPPUSHER                                             (0x1UL << 3)                                        /**< Stop pusher                                 */
#define _SYMCRYPTO_CTRL_STOPPUSHER_SHIFT                                      3                                                   /**< Shift value for SYMCRYPTO_STOPPUSHER        */
#define _SYMCRYPTO_CTRL_STOPPUSHER_MASK                                       0x8UL                                               /**< Bit mask for SYMCRYPTO_STOPPUSHER           */
#define _SYMCRYPTO_CTRL_STOPPUSHER_DEFAULT                                    0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_CTRL             */
#define SYMCRYPTO_CTRL_STOPPUSHER_DEFAULT                                     (_SYMCRYPTO_CTRL_STOPPUSHER_DEFAULT << 3)           /**< Shifted mode DEFAULT for SYMCRYPTO_CTRL     */
#define SYMCRYPTO_CTRL_SWRESET                                                (0x1UL << 4)                                        /**< Software reset                              */
#define _SYMCRYPTO_CTRL_SWRESET_SHIFT                                         4                                                   /**< Shift value for SYMCRYPTO_SWRESET           */
#define _SYMCRYPTO_CTRL_SWRESET_MASK                                          0x10UL                                              /**< Bit mask for SYMCRYPTO_SWRESET              */
#define _SYMCRYPTO_CTRL_SWRESET_DEFAULT                                       0x00000000UL                                        /**< Mode DEFAULT for SYMCRYPTO_CTRL             */
#define SYMCRYPTO_CTRL_SWRESET_DEFAULT                                        (_SYMCRYPTO_CTRL_SWRESET_DEFAULT << 4)              /**< Shifted mode DEFAULT for SYMCRYPTO_CTRL     */

/* Bit fields for SYMCRYPTO CMD */
#define _SYMCRYPTO_CMD_RESETVALUE                                             0x00000000UL                               /**< Default value for SYMCRYPTO_CMD             */
#define _SYMCRYPTO_CMD_MASK                                                   0x00000003UL                               /**< Mask for SYMCRYPTO_CMD                      */
#define SYMCRYPTO_CMD_STARTFETCHER                                            (0x1UL << 0)                               /**< Start fetch                                 */
#define _SYMCRYPTO_CMD_STARTFETCHER_SHIFT                                     0                                          /**< Shift value for SYMCRYPTO_STARTFETCHER      */
#define _SYMCRYPTO_CMD_STARTFETCHER_MASK                                      0x1UL                                      /**< Bit mask for SYMCRYPTO_STARTFETCHER         */
#define _SYMCRYPTO_CMD_STARTFETCHER_DEFAULT                                   0x00000000UL                               /**< Mode DEFAULT for SYMCRYPTO_CMD              */
#define SYMCRYPTO_CMD_STARTFETCHER_DEFAULT                                    (_SYMCRYPTO_CMD_STARTFETCHER_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_CMD      */
#define SYMCRYPTO_CMD_STARTPUSHER                                             (0x1UL << 1)                               /**< Start push                                  */
#define _SYMCRYPTO_CMD_STARTPUSHER_SHIFT                                      1                                          /**< Shift value for SYMCRYPTO_STARTPUSHER       */
#define _SYMCRYPTO_CMD_STARTPUSHER_MASK                                       0x2UL                                      /**< Bit mask for SYMCRYPTO_STARTPUSHER          */
#define _SYMCRYPTO_CMD_STARTPUSHER_DEFAULT                                    0x00000000UL                               /**< Mode DEFAULT for SYMCRYPTO_CMD              */
#define SYMCRYPTO_CMD_STARTPUSHER_DEFAULT                                     (_SYMCRYPTO_CMD_STARTPUSHER_DEFAULT << 1)  /**< Shifted mode DEFAULT for SYMCRYPTO_CMD      */

/* Bit fields for SYMCRYPTO STATUS */
#define _SYMCRYPTO_STATUS_RESETVALUE                                          0x00000000UL                                  /**< Default value for SYMCRYPTO_STATUS          */
#define _SYMCRYPTO_STATUS_MASK                                                0xFFFF0073UL                                  /**< Mask for SYMCRYPTO_STATUS                   */
#define SYMCRYPTO_STATUS_FETCHERBSY                                           (0x1UL << 0)                                  /**< Fetcher busy                                */
#define _SYMCRYPTO_STATUS_FETCHERBSY_SHIFT                                    0                                             /**< Shift value for SYMCRYPTO_FETCHERBSY        */
#define _SYMCRYPTO_STATUS_FETCHERBSY_MASK                                     0x1UL                                         /**< Bit mask for SYMCRYPTO_FETCHERBSY           */
#define _SYMCRYPTO_STATUS_FETCHERBSY_DEFAULT                                  0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_STATUS           */
#define SYMCRYPTO_STATUS_FETCHERBSY_DEFAULT                                   (_SYMCRYPTO_STATUS_FETCHERBSY_DEFAULT << 0)   /**< Shifted mode DEFAULT for SYMCRYPTO_STATUS   */
#define SYMCRYPTO_STATUS_PUSHERBSY                                            (0x1UL << 1)                                  /**< Pusher busy                                 */
#define _SYMCRYPTO_STATUS_PUSHERBSY_SHIFT                                     1                                             /**< Shift value for SYMCRYPTO_PUSHERBSY         */
#define _SYMCRYPTO_STATUS_PUSHERBSY_MASK                                      0x2UL                                         /**< Bit mask for SYMCRYPTO_PUSHERBSY            */
#define _SYMCRYPTO_STATUS_PUSHERBSY_DEFAULT                                   0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_STATUS           */
#define SYMCRYPTO_STATUS_PUSHERBSY_DEFAULT                                    (_SYMCRYPTO_STATUS_PUSHERBSY_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYMCRYPTO_STATUS   */
#define SYMCRYPTO_STATUS_NOTEMPTY                                             (0x1UL << 4)                                  /**< Not empty flag from input FIFO (fetcher)    */
#define _SYMCRYPTO_STATUS_NOTEMPTY_SHIFT                                      4                                             /**< Shift value for SYMCRYPTO_NOTEMPTY          */
#define _SYMCRYPTO_STATUS_NOTEMPTY_MASK                                       0x10UL                                        /**< Bit mask for SYMCRYPTO_NOTEMPTY             */
#define _SYMCRYPTO_STATUS_NOTEMPTY_DEFAULT                                    0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_STATUS           */
#define SYMCRYPTO_STATUS_NOTEMPTY_DEFAULT                                     (_SYMCRYPTO_STATUS_NOTEMPTY_DEFAULT << 4)     /**< Shifted mode DEFAULT for SYMCRYPTO_STATUS   */
#define SYMCRYPTO_STATUS_WAITING                                              (0x1UL << 5)                                  /**< Pusher waiting for FIFO                     */
#define _SYMCRYPTO_STATUS_WAITING_SHIFT                                       5                                             /**< Shift value for SYMCRYPTO_WAITING           */
#define _SYMCRYPTO_STATUS_WAITING_MASK                                        0x20UL                                        /**< Bit mask for SYMCRYPTO_WAITING              */
#define _SYMCRYPTO_STATUS_WAITING_DEFAULT                                     0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_STATUS           */
#define SYMCRYPTO_STATUS_WAITING_DEFAULT                                      (_SYMCRYPTO_STATUS_WAITING_DEFAULT << 5)      /**< Shifted mode DEFAULT for SYMCRYPTO_STATUS   */
#define SYMCRYPTO_STATUS_SOFTRSTBSY                                           (0x1UL << 6)                                  /**< Software reset busy                         */
#define _SYMCRYPTO_STATUS_SOFTRSTBSY_SHIFT                                    6                                             /**< Shift value for SYMCRYPTO_SOFTRSTBSY        */
#define _SYMCRYPTO_STATUS_SOFTRSTBSY_MASK                                     0x40UL                                        /**< Bit mask for SYMCRYPTO_SOFTRSTBSY           */
#define _SYMCRYPTO_STATUS_SOFTRSTBSY_DEFAULT                                  0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_STATUS           */
#define SYMCRYPTO_STATUS_SOFTRSTBSY_DEFAULT                                   (_SYMCRYPTO_STATUS_SOFTRSTBSY_DEFAULT << 6)   /**< Shifted mode DEFAULT for SYMCRYPTO_STATUS   */
#define _SYMCRYPTO_STATUS_FIFODATANUM_SHIFT                                   16                                            /**< Shift value for SYMCRYPTO_FIFODATANUM       */
#define _SYMCRYPTO_STATUS_FIFODATANUM_MASK                                    0xFFFF0000UL                                  /**< Bit mask for SYMCRYPTO_FIFODATANUM          */
#define _SYMCRYPTO_STATUS_FIFODATANUM_DEFAULT                                 0x00000000UL                                  /**< Mode DEFAULT for SYMCRYPTO_STATUS           */
#define SYMCRYPTO_STATUS_FIFODATANUM_DEFAULT                                  (_SYMCRYPTO_STATUS_FIFODATANUM_DEFAULT << 16) /**< Shifted mode DEFAULT for SYMCRYPTO_STATUS   */

/* Bit fields for SYMCRYPTO SYMCRYPTO_INCL_IPS_HW_CFG */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_RESETVALUE                       0x00000011UL                                                               /**< Default value for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_MASK                             0x00007FFFUL                                                               /**< Mask for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAES                      (0x1UL << 0)                                                               /**< Includes AES                                */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAES_SHIFT               0                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEAES      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAES_MASK                0x1UL                                                                      /**< Bit mask for SYMCRYPTO_G_INCLUDEAES         */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAES_DEFAULT             0x00000001UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAES_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAES_DEFAULT << 0)           /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESGCM                   (0x1UL << 1)                                                               /**< Includes AESGCM                             */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESGCM_SHIFT            1                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEAESGCM   */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESGCM_MASK             0x2UL                                                                      /**< Bit mask for SYMCRYPTO_G_INCLUDEAESGCM      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESGCM_DEFAULT          0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESGCM_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESGCM_DEFAULT << 1)        /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESXTS                   (0x1UL << 2)                                                               /**< Includes AESXTS                             */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESXTS_SHIFT            2                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEAESXTS   */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESXTS_MASK             0x4UL                                                                      /**< Bit mask for SYMCRYPTO_G_INCLUDEAESXTS      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESXTS_DEFAULT          0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESXTS_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEAESXTS_DEFAULT << 2)        /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEDES                      (0x1UL << 3)                                                               /**< Includes DES                                */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEDES_SHIFT               3                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEDES      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEDES_MASK                0x8UL                                                                      /**< Bit mask for SYMCRYPTO_G_INCLUDEDES         */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEDES_DEFAULT             0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEDES_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEDES_DEFAULT << 3)           /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHASH                     (0x1UL << 4)                                                               /**< Includes HASH                               */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHASH_SHIFT              4                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEHASH     */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHASH_MASK               0x10UL                                                                     /**< Bit mask for SYMCRYPTO_G_INCLUDEHASH        */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHASH_DEFAULT            0x00000001UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHASH_DEFAULT             (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHASH_DEFAULT << 4)          /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDECHACHAPOLY               (0x1UL << 5)                                                               /**< Includes ChachaPoly                         */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDECHACHAPOLY_SHIFT        5                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDECHACHAPOLY*/
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDECHACHAPOLY_MASK         0x20UL                                                                     /**< Bit mask for SYMCRYPTO_G_INCLUDECHACHAPOLY  */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDECHACHAPOLY_DEFAULT      0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDECHACHAPOLY_DEFAULT       (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDECHACHAPOLY_DEFAULT << 5)    /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESHA3                     (0x1UL << 6)                                                               /**< Includes SHA3                               */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESHA3_SHIFT              6                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDESHA3     */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESHA3_MASK               0x40UL                                                                     /**< Bit mask for SYMCRYPTO_G_INCLUDESHA3        */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESHA3_DEFAULT            0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESHA3_DEFAULT             (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESHA3_DEFAULT << 6)          /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEZUC                      (0x1UL << 7)                                                               /**< Includes ZUC                                */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEZUC_SHIFT               7                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEZUC      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEZUC_MASK                0x80UL                                                                     /**< Bit mask for SYMCRYPTO_G_INCLUDEZUC         */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEZUC_DEFAULT             0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEZUC_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEZUC_DEFAULT << 7)           /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESM4                      (0x1UL << 8)                                                               /**< Includes SM4                                */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESM4_SHIFT               8                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDESM4      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESM4_MASK                0x100UL                                                                    /**< Bit mask for SYMCRYPTO_G_INCLUDESM4         */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESM4_DEFAULT             0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESM4_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESM4_DEFAULT << 8)           /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEPKE                      (0x1UL << 9)                                                               /**< Includes PKE                                */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEPKE_SHIFT               9                                                                          /**< Shift value for SYMCRYPTO_G_INCLUDEPKE      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEPKE_MASK                0x200UL                                                                    /**< Bit mask for SYMCRYPTO_G_INCLUDEPKE         */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEPKE_DEFAULT             0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEPKE_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEPKE_DEFAULT << 9)           /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDENDRNG                    (0x1UL << 10)                                                              /**< Includes NDRNG                              */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDENDRNG_SHIFT             10                                                                         /**< Shift value for SYMCRYPTO_G_INCLUDENDRNG    */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDENDRNG_MASK              0x400UL                                                                    /**< Bit mask for SYMCRYPTO_G_INCLUDENDRNG       */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDENDRNG_DEFAULT           0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDENDRNG_DEFAULT            (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDENDRNG_DEFAULT << 10)        /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHPCHACHAPOLY             (0x1UL << 11)                                                              /**< Includes HP-ChaChaPoly                      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHPCHACHAPOLY_SHIFT      11                                                                         /**< Shift value for SYMCRYPTO_G_INCLUDEHPCHACHAPOLY*/
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHPCHACHAPOLY_MASK       0x800UL                                                                    /**< Bit mask for SYMCRYPTO_G_INCLUDEHPCHACHAPOLY*/
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHPCHACHAPOLY_DEFAULT    0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHPCHACHAPOLY_DEFAULT     (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEHPCHACHAPOLY_DEFAULT << 11) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESNOW3G                   (0x1UL << 12)                                                              /**< Includes Snow3G                             */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESNOW3G_SHIFT            12                                                                         /**< Shift value for SYMCRYPTO_G_INCLUDESNOW3G   */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESNOW3G_MASK             0x1000UL                                                                   /**< Bit mask for SYMCRYPTO_G_INCLUDESNOW3G      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESNOW3G_DEFAULT          0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESNOW3G_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDESNOW3G_DEFAULT << 12)       /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEKASUMI                   (0x1UL << 13)                                                              /**< Includes Kasumi                             */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEKASUMI_SHIFT            13                                                                         /**< Shift value for SYMCRYPTO_G_INCLUDEKASUMI   */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEKASUMI_MASK             0x2000UL                                                                   /**< Bit mask for SYMCRYPTO_G_INCLUDEKASUMI      */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEKASUMI_DEFAULT          0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEKASUMI_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEKASUMI_DEFAULT << 13)       /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEARIA                     (0x1UL << 14)                                                              /**< Includes Aria                               */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEARIA_SHIFT              14                                                                         /**< Shift value for SYMCRYPTO_G_INCLUDEARIA     */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEARIA_MASK               0x4000UL                                                                   /**< Bit mask for SYMCRYPTO_G_INCLUDEARIA        */
#define _SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEARIA_DEFAULT            0x00000000UL                                                               /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEARIA_DEFAULT             (_SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG_G_INCLUDEARIA_DEFAULT << 14)         /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_INCL_IPS_HW_CFG*/

/* Bit fields for SYMCRYPTO SYMCRYPTO_BA411E_HW_CFG_1 */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_RESETVALUE                       0x0F03017FUL                                                            /**< Default value for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_MASK                             0x1F0301FFUL                                                            /**< Mask for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_AESMODESPOSS_SHIFT             0                                                                       /**< Shift value for SYMCRYPTO_G_AESMODESPOSS    */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_AESMODESPOSS_MASK              0x1FFUL                                                                 /**< Bit mask for SYMCRYPTO_G_AESMODESPOSS       */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_AESMODESPOSS_DEFAULT           0x0000017FUL                                                            /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_AESMODESPOSS_DEFAULT            (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_AESMODESPOSS_DEFAULT << 0)      /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CS                              (0x1UL << 16)                                                           /**< Generic g_CS value                          */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CS_SHIFT                       16                                                                      /**< Shift value for SYMCRYPTO_G_CS              */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CS_MASK                        0x10000UL                                                               /**< Bit mask for SYMCRYPTO_G_CS                 */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CS_DEFAULT                     0x00000001UL                                                            /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CS_DEFAULT                      (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CS_DEFAULT << 16)               /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_USEMASKING                      (0x1UL << 17)                                                           /**< Generic g_UseMasking value                  */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_USEMASKING_SHIFT               17                                                                      /**< Shift value for SYMCRYPTO_G_USEMASKING      */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_USEMASKING_MASK                0x20000UL                                                               /**< Bit mask for SYMCRYPTO_G_USEMASKING         */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_USEMASKING_DEFAULT             0x00000001UL                                                            /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_USEMASKING_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_USEMASKING_DEFAULT << 17)       /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_KEYSIZE_SHIFT                  24                                                                      /**< Shift value for SYMCRYPTO_G_KEYSIZE         */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_KEYSIZE_MASK                   0x7000000UL                                                             /**< Bit mask for SYMCRYPTO_G_KEYSIZE            */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_KEYSIZE_DEFAULT                0x00000007UL                                                            /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_KEYSIZE_DEFAULT                 (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_KEYSIZE_DEFAULT << 24)          /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CXSWITCH                        (0x1UL << 27)                                                           /**< Generic g_CxSwitch value                    */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CXSWITCH_SHIFT                 27                                                                      /**< Shift value for SYMCRYPTO_G_CXSWITCH        */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CXSWITCH_MASK                  0x8000000UL                                                             /**< Bit mask for SYMCRYPTO_G_CXSWITCH           */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CXSWITCH_DEFAULT               0x00000001UL                                                            /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CXSWITCH_DEFAULT                (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_CXSWITCH_DEFAULT << 27)         /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_GLITCHPROTECTION                (0x1UL << 28)                                                           /**< Generic g_GlitchProtection value            */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_GLITCHPROTECTION_SHIFT         28                                                                      /**< Shift value for SYMCRYPTO_G_GLITCHPROTECTION*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_GLITCHPROTECTION_MASK          0x10000000UL                                                            /**< Bit mask for SYMCRYPTO_G_GLITCHPROTECTION   */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_GLITCHPROTECTION_DEFAULT       0x00000000UL                                                            /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_GLITCHPROTECTION_DEFAULT        (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1_G_GLITCHPROTECTION_DEFAULT << 28) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_1*/

/* Bit fields for SYMCRYPTO SYMCRYPTO_BA411E_HW_CFG_2 */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_RESETVALUE                       0x00100080UL                                                           /**< Default value for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_MASK                             0x0FF0FFFFUL                                                           /**< Mask for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_CTRSIZE_SHIFT                  0                                                                      /**< Shift value for SYMCRYPTO_G_CTRSIZE         */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_CTRSIZE_MASK                   0xFFFFUL                                                               /**< Bit mask for SYMCRYPTO_G_CTRSIZE            */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_CTRSIZE_DEFAULT                0x00000080UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_CTRSIZE_DEFAULT                 (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_CTRSIZE_DEFAULT << 0)          /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_EXT_NB_AES_KEYS_SHIFT          20                                                                     /**< Shift value for SYMCRYPTO_G_EXT_NB_AES_KEYS */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_EXT_NB_AES_KEYS_MASK           0xF00000UL                                                             /**< Bit mask for SYMCRYPTO_G_EXT_NB_AES_KEYS    */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_EXT_NB_AES_KEYS_DEFAULT        0x00000001UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_EXT_NB_AES_KEYS_DEFAULT         (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_EXT_NB_AES_KEYS_DEFAULT << 20) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_IKG_NB_AES_KEYS_SHIFT          24                                                                     /**< Shift value for SYMCRYPTO_G_IKG_NB_AES_KEYS */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_IKG_NB_AES_KEYS_MASK           0xF000000UL                                                            /**< Bit mask for SYMCRYPTO_G_IKG_NB_AES_KEYS    */
#define _SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_IKG_NB_AES_KEYS_DEFAULT        0x00000000UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/
#define SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_IKG_NB_AES_KEYS_DEFAULT         (_SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2_G_IKG_NB_AES_KEYS_DEFAULT << 24) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA411E_HW_CFG_2*/

/* Bit fields for SYMCRYPTO SYMCRYPTO_BA413_HW_CFG_0 */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_RESETVALUE                        0x0013007FUL                                                           /**< Default value for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_MASK                              0x0FF7007FUL                                                           /**< Mask for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0 */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHMASKFUNC_SHIFT              0                                                                      /**< Shift value for SYMCRYPTO_G_HASHMASKFUNC    */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHMASKFUNC_MASK               0x7FUL                                                                 /**< Bit mask for SYMCRYPTO_G_HASHMASKFUNC       */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHMASKFUNC_DEFAULT            0x0000007FUL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHMASKFUNC_DEFAULT             (_SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHMASKFUNC_DEFAULT << 0)      /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHPADDING                      (0x1UL << 16)                                                          /**< Generic g_HashPadding value                 */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHPADDING_SHIFT               16                                                                     /**< Shift value for SYMCRYPTO_G_HASHPADDING     */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHPADDING_MASK                0x10000UL                                                              /**< Bit mask for SYMCRYPTO_G_HASHPADDING        */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHPADDING_DEFAULT             0x00000001UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHPADDING_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHPADDING_DEFAULT << 16)      /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HMAC_ENABLED                     (0x1UL << 17)                                                          /**< Generic g_HMAC_enabled value                */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HMAC_ENABLED_SHIFT              17                                                                     /**< Shift value for SYMCRYPTO_G_HMAC_ENABLED    */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HMAC_ENABLED_MASK               0x20000UL                                                              /**< Bit mask for SYMCRYPTO_G_HMAC_ENABLED       */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HMAC_ENABLED_DEFAULT            0x00000001UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HMAC_ENABLED_DEFAULT             (_SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HMAC_ENABLED_DEFAULT << 17)     /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHVERIFYDIGEST                 (0x1UL << 18)                                                          /**< Generic g_HashVerifyDigest value            */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHVERIFYDIGEST_SHIFT          18                                                                     /**< Shift value for SYMCRYPTO_G_HASHVERIFYDIGEST*/
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHVERIFYDIGEST_MASK           0x40000UL                                                              /**< Bit mask for SYMCRYPTO_G_HASHVERIFYDIGEST   */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHVERIFYDIGEST_DEFAULT        0x00000000UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHVERIFYDIGEST_DEFAULT         (_SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_HASHVERIFYDIGEST_DEFAULT << 18) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_EXT_NB_HASH_KEYS_SHIFT          20                                                                     /**< Shift value for SYMCRYPTO_G_EXT_NB_HASH_KEYS*/
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_EXT_NB_HASH_KEYS_MASK           0xF00000UL                                                             /**< Bit mask for SYMCRYPTO_G_EXT_NB_HASH_KEYS   */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_EXT_NB_HASH_KEYS_DEFAULT        0x00000001UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_EXT_NB_HASH_KEYS_DEFAULT         (_SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_EXT_NB_HASH_KEYS_DEFAULT << 20) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_IKG_NB_HASH_KEYS_SHIFT          24                                                                     /**< Shift value for SYMCRYPTO_G_IKG_NB_HASH_KEYS*/
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_IKG_NB_HASH_KEYS_MASK           0xF000000UL                                                            /**< Bit mask for SYMCRYPTO_G_IKG_NB_HASH_KEYS   */
#define _SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_IKG_NB_HASH_KEYS_DEFAULT        0x00000000UL                                                           /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/
#define SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_IKG_NB_HASH_KEYS_DEFAULT         (_SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0_G_IKG_NB_HASH_KEYS_DEFAULT << 24) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA413_HW_CFG_0*/

/* Bit fields for SYMCRYPTO SYMCRYPTO_BA418_HW_CFG */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_RESETVALUE                          0x00120001UL                                                         /**< Default value for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_MASK                                0x0FF60001UL                                                         /**< Mask for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG   */
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_SHA3CTXTEN                         (0x1UL << 0)                                                         /**< Generic g_Sha3CtxtEn value                  */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_SHA3CTXTEN_SHIFT                  0                                                                    /**< Shift value for SYMCRYPTO_G_SHA3CTXTEN      */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_SHA3CTXTEN_MASK                   0x1UL                                                                /**< Bit mask for SYMCRYPTO_G_SHA3CTXTEN         */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_SHA3CTXTEN_DEFAULT                0x00000001UL                                                         /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_SHA3CTXTEN_DEFAULT                 (_SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_SHA3CTXTEN_DEFAULT << 0)        /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HMAC_ENABLED                       (0x1UL << 17)                                                        /**< Generic g_HMAC_enabled value                */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HMAC_ENABLED_SHIFT                17                                                                   /**< Shift value for SYMCRYPTO_G_HMAC_ENABLED    */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HMAC_ENABLED_MASK                 0x20000UL                                                            /**< Bit mask for SYMCRYPTO_G_HMAC_ENABLED       */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HMAC_ENABLED_DEFAULT              0x00000001UL                                                         /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HMAC_ENABLED_DEFAULT               (_SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HMAC_ENABLED_DEFAULT << 17)     /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HASHVERIFYDIGEST                   (0x1UL << 18)                                                        /**< Generic g_HashVerifyDigest value            */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HASHVERIFYDIGEST_SHIFT            18                                                                   /**< Shift value for SYMCRYPTO_G_HASHVERIFYDIGEST*/
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HASHVERIFYDIGEST_MASK             0x40000UL                                                            /**< Bit mask for SYMCRYPTO_G_HASHVERIFYDIGEST   */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HASHVERIFYDIGEST_DEFAULT          0x00000000UL                                                         /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HASHVERIFYDIGEST_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_HASHVERIFYDIGEST_DEFAULT << 18) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_EXT_NB_HASH_KEYS_SHIFT            20                                                                   /**< Shift value for SYMCRYPTO_G_EXT_NB_HASH_KEYS*/
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_EXT_NB_HASH_KEYS_MASK             0xF00000UL                                                           /**< Bit mask for SYMCRYPTO_G_EXT_NB_HASH_KEYS   */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_EXT_NB_HASH_KEYS_DEFAULT          0x00000001UL                                                         /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_EXT_NB_HASH_KEYS_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_EXT_NB_HASH_KEYS_DEFAULT << 20) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_IKG_NB_HASH_KEYS_SHIFT            24                                                                   /**< Shift value for SYMCRYPTO_G_IKG_NB_HASH_KEYS*/
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_IKG_NB_HASH_KEYS_MASK             0xF000000UL                                                          /**< Bit mask for SYMCRYPTO_G_IKG_NB_HASH_KEYS   */
#define _SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_IKG_NB_HASH_KEYS_DEFAULT          0x00000000UL                                                         /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_IKG_NB_HASH_KEYS_DEFAULT           (_SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG_G_IKG_NB_HASH_KEYS_DEFAULT << 24) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA418_HW_CFG*/

/* Bit fields for SYMCRYPTO SYMCRYPTO_BA419_HW_CFG */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_RESETVALUE                          0x00000000UL                                                      /**< Default value for SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG*/
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_MASK                                0x000201FFUL                                                      /**< Mask for SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG   */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4MODESPOSS_SHIFT                0                                                                 /**< Shift value for SYMCRYPTO_G_SM4MODESPOSS    */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4MODESPOSS_MASK                 0x1FFUL                                                           /**< Bit mask for SYMCRYPTO_G_SM4MODESPOSS       */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4MODESPOSS_DEFAULT              0x00000000UL                                                      /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4MODESPOSS_DEFAULT               (_SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4MODESPOSS_DEFAULT << 0)   /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4USEMASKING                      (0x1UL << 17)                                                     /**< Generic g_SM4UseMasking value               */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4USEMASKING_SHIFT               17                                                                /**< Shift value for SYMCRYPTO_G_SM4USEMASKING   */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4USEMASKING_MASK                0x20000UL                                                         /**< Bit mask for SYMCRYPTO_G_SM4USEMASKING      */
#define _SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4USEMASKING_DEFAULT             0x00000000UL                                                      /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4USEMASKING_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG_G_SM4USEMASKING_DEFAULT << 17) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA419_HW_CFG*/

/* Bit fields for SYMCRYPTO SYMCRYPTO_BA424_HW_CFG */
#define _SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_RESETVALUE                          0x00000000UL                                                     /**< Default value for SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG*/
#define _SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_MASK                                0x000001FFUL                                                     /**< Mask for SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG   */
#define _SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_G_ARIA_MODEPOSS_SHIFT               0                                                                /**< Shift value for SYMCRYPTO_G_ARIA_MODEPOSS   */
#define _SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_G_ARIA_MODEPOSS_MASK                0x1FFUL                                                          /**< Bit mask for SYMCRYPTO_G_ARIA_MODEPOSS      */
#define _SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_G_ARIA_MODEPOSS_DEFAULT             0x00000000UL                                                     /**< Mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG*/
#define SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_G_ARIA_MODEPOSS_DEFAULT              (_SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG_G_ARIA_MODEPOSS_DEFAULT << 0) /**< Shifted mode DEFAULT for SYMCRYPTO_SYMCRYPTO_BA424_HW_CFG*/

/** @} End of group SIBG301_SYMCRYPTO_BitFields */
/** @} End of group SIBG301_SYMCRYPTO */
/** @} End of group Parts */

#endif // SIBG301_SYMCRYPTO_H
