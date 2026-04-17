/**************************************************************************//**
 * @file
 * @brief SIMG301 HOSTPORTAL register and bit field definitions
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
#ifndef SIMG301_HOSTPORTAL_H
#define SIMG301_HOSTPORTAL_H
#define HOSTPORTAL_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_HOSTPORTAL HOSTPORTAL
 * @{
 * @brief SIMG301 HOSTPORTAL Register Declaration.
 *****************************************************************************/

/** HOSTPORTAL Register Declaration. */
typedef struct hostportal_typedef{
  __IM uint32_t  IPID;                          /**< IP version ID                                      */
  __IOM uint32_t CTRL;                          /**< HOSTPORTAL Control Register                        */
  __IOM uint32_t STATUS;                        /**< HOSTPORTAL Status Register                         */
  __IOM uint32_t LPW0IF;                        /**< LPW0 Interrupt Flags                               */
  __IOM uint32_t LPW0IEN;                       /**< LPW0 Interrupt Enables                             */
  __IOM uint32_t MAILBOXREG0;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG1;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG2;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG3;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG4;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG5;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG6;                   /**< MB register                                        */
  __IOM uint32_t MAILBOXREG7;                   /**< MB register                                        */
  uint32_t       RESERVED0[1011U];              /**< Reserved for future use                            */
  __IM uint32_t  IPID_SET;                      /**< IP version ID                                      */
  __IOM uint32_t CTRL_SET;                      /**< HOSTPORTAL Control Register                        */
  __IOM uint32_t STATUS_SET;                    /**< HOSTPORTAL Status Register                         */
  __IOM uint32_t LPW0IF_SET;                    /**< LPW0 Interrupt Flags                               */
  __IOM uint32_t LPW0IEN_SET;                   /**< LPW0 Interrupt Enables                             */
  __IOM uint32_t MAILBOXREG0_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG1_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG2_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG3_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG4_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG5_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG6_SET;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG7_SET;               /**< MB register                                        */
  uint32_t       RESERVED1[1011U];              /**< Reserved for future use                            */
  __IM uint32_t  IPID_CLR;                      /**< IP version ID                                      */
  __IOM uint32_t CTRL_CLR;                      /**< HOSTPORTAL Control Register                        */
  __IOM uint32_t STATUS_CLR;                    /**< HOSTPORTAL Status Register                         */
  __IOM uint32_t LPW0IF_CLR;                    /**< LPW0 Interrupt Flags                               */
  __IOM uint32_t LPW0IEN_CLR;                   /**< LPW0 Interrupt Enables                             */
  __IOM uint32_t MAILBOXREG0_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG1_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG2_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG3_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG4_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG5_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG6_CLR;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG7_CLR;               /**< MB register                                        */
  uint32_t       RESERVED2[1011U];              /**< Reserved for future use                            */
  __IM uint32_t  IPID_TGL;                      /**< IP version ID                                      */
  __IOM uint32_t CTRL_TGL;                      /**< HOSTPORTAL Control Register                        */
  __IOM uint32_t STATUS_TGL;                    /**< HOSTPORTAL Status Register                         */
  __IOM uint32_t LPW0IF_TGL;                    /**< LPW0 Interrupt Flags                               */
  __IOM uint32_t LPW0IEN_TGL;                   /**< LPW0 Interrupt Enables                             */
  __IOM uint32_t MAILBOXREG0_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG1_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG2_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG3_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG4_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG5_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG6_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG7_TGL;               /**< MB register                                        */
} HOSTPORTAL_TypeDef;
/** @} End of group SIMG301_HOSTPORTAL */

/**************************************************************************//**
 * @addtogroup SIMG301_HOSTPORTAL
 * @{
 * @defgroup SIMG301_HOSTPORTAL_BitFields HOSTPORTAL Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for HOSTPORTAL IPID */
#define _HOSTPORTAL_IPID_RESETVALUE                0x00000001UL                              /**< Default value for HOSTPORTAL_IPID           */
#define _HOSTPORTAL_IPID_MASK                      0xFFFFFFFFUL                              /**< Mask for HOSTPORTAL_IPID                    */
#define _HOSTPORTAL_IPID_IPVERSION_SHIFT           0                                         /**< Shift value for HOSTPORTAL_IPVERSION        */
#define _HOSTPORTAL_IPID_IPVERSION_MASK            0xFFFFFFFFUL                              /**< Bit mask for HOSTPORTAL_IPVERSION           */
#define _HOSTPORTAL_IPID_IPVERSION_DEFAULT         0x00000001UL                              /**< Mode DEFAULT for HOSTPORTAL_IPID            */
#define HOSTPORTAL_IPID_IPVERSION_DEFAULT          (_HOSTPORTAL_IPID_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_IPID    */

/* Bit fields for HOSTPORTAL CTRL */
#define _HOSTPORTAL_CTRL_RESETVALUE                0x00000000UL                                 /**< Default value for HOSTPORTAL_CTRL           */
#define _HOSTPORTAL_CTRL_MASK                      0x00000001UL                                 /**< Mask for HOSTPORTAL_CTRL                    */
#define HOSTPORTAL_CTRL_LPW0PWRUPREQ               (0x1UL << 0)                                 /**< LPW0 Power Up Request                       */
#define _HOSTPORTAL_CTRL_LPW0PWRUPREQ_SHIFT        0                                            /**< Shift value for HOSTPORTAL_LPW0PWRUPREQ     */
#define _HOSTPORTAL_CTRL_LPW0PWRUPREQ_MASK         0x1UL                                        /**< Bit mask for HOSTPORTAL_LPW0PWRUPREQ        */
#define _HOSTPORTAL_CTRL_LPW0PWRUPREQ_DEFAULT      0x00000000UL                                 /**< Mode DEFAULT for HOSTPORTAL_CTRL            */
#define HOSTPORTAL_CTRL_LPW0PWRUPREQ_DEFAULT       (_HOSTPORTAL_CTRL_LPW0PWRUPREQ_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_CTRL    */

/* Bit fields for HOSTPORTAL STATUS */
#define _HOSTPORTAL_STATUS_RESETVALUE              0x00000000UL                                   /**< Default value for HOSTPORTAL_STATUS         */
#define _HOSTPORTAL_STATUS_MASK                    0x00000001UL                                   /**< Mask for HOSTPORTAL_STATUS                  */
#define HOSTPORTAL_STATUS_LPW0PWRUPACK             (0x1UL << 0)                                   /**< LPW0 Power up status                        */
#define _HOSTPORTAL_STATUS_LPW0PWRUPACK_SHIFT      0                                              /**< Shift value for HOSTPORTAL_LPW0PWRUPACK     */
#define _HOSTPORTAL_STATUS_LPW0PWRUPACK_MASK       0x1UL                                          /**< Bit mask for HOSTPORTAL_LPW0PWRUPACK        */
#define _HOSTPORTAL_STATUS_LPW0PWRUPACK_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_STATUS          */
#define HOSTPORTAL_STATUS_LPW0PWRUPACK_DEFAULT     (_HOSTPORTAL_STATUS_LPW0PWRUPACK_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_STATUS  */

/* Bit fields for HOSTPORTAL LPW0IF */
#define _HOSTPORTAL_LPW0IF_RESETVALUE              0x00000000UL                                /**< Default value for HOSTPORTAL_LPW0IF         */
#define _HOSTPORTAL_LPW0IF_MASK                    0xFFFFFFFFUL                                /**< Mask for HOSTPORTAL_LPW0IF                  */
#define HOSTPORTAL_LPW0IF_LPW0IF0                  (0x1UL << 0)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF0_SHIFT           0                                           /**< Shift value for HOSTPORTAL_LPW0IF0          */
#define _HOSTPORTAL_LPW0IF_LPW0IF0_MASK            0x1UL                                       /**< Bit mask for HOSTPORTAL_LPW0IF0             */
#define _HOSTPORTAL_LPW0IF_LPW0IF0_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF0_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF0_DEFAULT << 0)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF1                  (0x1UL << 1)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF1_SHIFT           1                                           /**< Shift value for HOSTPORTAL_LPW0IF1          */
#define _HOSTPORTAL_LPW0IF_LPW0IF1_MASK            0x2UL                                       /**< Bit mask for HOSTPORTAL_LPW0IF1             */
#define _HOSTPORTAL_LPW0IF_LPW0IF1_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF1_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF1_DEFAULT << 1)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF2                  (0x1UL << 2)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF2_SHIFT           2                                           /**< Shift value for HOSTPORTAL_LPW0IF2          */
#define _HOSTPORTAL_LPW0IF_LPW0IF2_MASK            0x4UL                                       /**< Bit mask for HOSTPORTAL_LPW0IF2             */
#define _HOSTPORTAL_LPW0IF_LPW0IF2_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF2_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF2_DEFAULT << 2)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF3                  (0x1UL << 3)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF3_SHIFT           3                                           /**< Shift value for HOSTPORTAL_LPW0IF3          */
#define _HOSTPORTAL_LPW0IF_LPW0IF3_MASK            0x8UL                                       /**< Bit mask for HOSTPORTAL_LPW0IF3             */
#define _HOSTPORTAL_LPW0IF_LPW0IF3_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF3_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF3_DEFAULT << 3)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF4                  (0x1UL << 4)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF4_SHIFT           4                                           /**< Shift value for HOSTPORTAL_LPW0IF4          */
#define _HOSTPORTAL_LPW0IF_LPW0IF4_MASK            0x10UL                                      /**< Bit mask for HOSTPORTAL_LPW0IF4             */
#define _HOSTPORTAL_LPW0IF_LPW0IF4_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF4_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF4_DEFAULT << 4)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF5                  (0x1UL << 5)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF5_SHIFT           5                                           /**< Shift value for HOSTPORTAL_LPW0IF5          */
#define _HOSTPORTAL_LPW0IF_LPW0IF5_MASK            0x20UL                                      /**< Bit mask for HOSTPORTAL_LPW0IF5             */
#define _HOSTPORTAL_LPW0IF_LPW0IF5_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF5_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF5_DEFAULT << 5)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF6                  (0x1UL << 6)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF6_SHIFT           6                                           /**< Shift value for HOSTPORTAL_LPW0IF6          */
#define _HOSTPORTAL_LPW0IF_LPW0IF6_MASK            0x40UL                                      /**< Bit mask for HOSTPORTAL_LPW0IF6             */
#define _HOSTPORTAL_LPW0IF_LPW0IF6_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF6_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF6_DEFAULT << 6)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF7                  (0x1UL << 7)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF7_SHIFT           7                                           /**< Shift value for HOSTPORTAL_LPW0IF7          */
#define _HOSTPORTAL_LPW0IF_LPW0IF7_MASK            0x80UL                                      /**< Bit mask for HOSTPORTAL_LPW0IF7             */
#define _HOSTPORTAL_LPW0IF_LPW0IF7_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF7_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF7_DEFAULT << 7)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF8                  (0x1UL << 8)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF8_SHIFT           8                                           /**< Shift value for HOSTPORTAL_LPW0IF8          */
#define _HOSTPORTAL_LPW0IF_LPW0IF8_MASK            0x100UL                                     /**< Bit mask for HOSTPORTAL_LPW0IF8             */
#define _HOSTPORTAL_LPW0IF_LPW0IF8_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF8_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF8_DEFAULT << 8)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF9                  (0x1UL << 9)                                /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF9_SHIFT           9                                           /**< Shift value for HOSTPORTAL_LPW0IF9          */
#define _HOSTPORTAL_LPW0IF_LPW0IF9_MASK            0x200UL                                     /**< Bit mask for HOSTPORTAL_LPW0IF9             */
#define _HOSTPORTAL_LPW0IF_LPW0IF9_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF9_DEFAULT          (_HOSTPORTAL_LPW0IF_LPW0IF9_DEFAULT << 9)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF10                 (0x1UL << 10)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF10_SHIFT          10                                          /**< Shift value for HOSTPORTAL_LPW0IF10         */
#define _HOSTPORTAL_LPW0IF_LPW0IF10_MASK           0x400UL                                     /**< Bit mask for HOSTPORTAL_LPW0IF10            */
#define _HOSTPORTAL_LPW0IF_LPW0IF10_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF10_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF10_DEFAULT << 10) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF11                 (0x1UL << 11)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF11_SHIFT          11                                          /**< Shift value for HOSTPORTAL_LPW0IF11         */
#define _HOSTPORTAL_LPW0IF_LPW0IF11_MASK           0x800UL                                     /**< Bit mask for HOSTPORTAL_LPW0IF11            */
#define _HOSTPORTAL_LPW0IF_LPW0IF11_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF11_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF11_DEFAULT << 11) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF12                 (0x1UL << 12)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF12_SHIFT          12                                          /**< Shift value for HOSTPORTAL_LPW0IF12         */
#define _HOSTPORTAL_LPW0IF_LPW0IF12_MASK           0x1000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IF12            */
#define _HOSTPORTAL_LPW0IF_LPW0IF12_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF12_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF12_DEFAULT << 12) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF13                 (0x1UL << 13)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF13_SHIFT          13                                          /**< Shift value for HOSTPORTAL_LPW0IF13         */
#define _HOSTPORTAL_LPW0IF_LPW0IF13_MASK           0x2000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IF13            */
#define _HOSTPORTAL_LPW0IF_LPW0IF13_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF13_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF13_DEFAULT << 13) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF14                 (0x1UL << 14)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF14_SHIFT          14                                          /**< Shift value for HOSTPORTAL_LPW0IF14         */
#define _HOSTPORTAL_LPW0IF_LPW0IF14_MASK           0x4000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IF14            */
#define _HOSTPORTAL_LPW0IF_LPW0IF14_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF14_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF14_DEFAULT << 14) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF15                 (0x1UL << 15)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF15_SHIFT          15                                          /**< Shift value for HOSTPORTAL_LPW0IF15         */
#define _HOSTPORTAL_LPW0IF_LPW0IF15_MASK           0x8000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IF15            */
#define _HOSTPORTAL_LPW0IF_LPW0IF15_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF15_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF15_DEFAULT << 15) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF16                 (0x1UL << 16)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF16_SHIFT          16                                          /**< Shift value for HOSTPORTAL_LPW0IF16         */
#define _HOSTPORTAL_LPW0IF_LPW0IF16_MASK           0x10000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IF16            */
#define _HOSTPORTAL_LPW0IF_LPW0IF16_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF16_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF16_DEFAULT << 16) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF17                 (0x1UL << 17)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF17_SHIFT          17                                          /**< Shift value for HOSTPORTAL_LPW0IF17         */
#define _HOSTPORTAL_LPW0IF_LPW0IF17_MASK           0x20000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IF17            */
#define _HOSTPORTAL_LPW0IF_LPW0IF17_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF17_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF17_DEFAULT << 17) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF18                 (0x1UL << 18)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF18_SHIFT          18                                          /**< Shift value for HOSTPORTAL_LPW0IF18         */
#define _HOSTPORTAL_LPW0IF_LPW0IF18_MASK           0x40000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IF18            */
#define _HOSTPORTAL_LPW0IF_LPW0IF18_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF18_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF18_DEFAULT << 18) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF19                 (0x1UL << 19)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF19_SHIFT          19                                          /**< Shift value for HOSTPORTAL_LPW0IF19         */
#define _HOSTPORTAL_LPW0IF_LPW0IF19_MASK           0x80000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IF19            */
#define _HOSTPORTAL_LPW0IF_LPW0IF19_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF19_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF19_DEFAULT << 19) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF20                 (0x1UL << 20)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF20_SHIFT          20                                          /**< Shift value for HOSTPORTAL_LPW0IF20         */
#define _HOSTPORTAL_LPW0IF_LPW0IF20_MASK           0x100000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IF20            */
#define _HOSTPORTAL_LPW0IF_LPW0IF20_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF20_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF20_DEFAULT << 20) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF21                 (0x1UL << 21)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF21_SHIFT          21                                          /**< Shift value for HOSTPORTAL_LPW0IF21         */
#define _HOSTPORTAL_LPW0IF_LPW0IF21_MASK           0x200000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IF21            */
#define _HOSTPORTAL_LPW0IF_LPW0IF21_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF21_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF21_DEFAULT << 21) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF22                 (0x1UL << 22)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF22_SHIFT          22                                          /**< Shift value for HOSTPORTAL_LPW0IF22         */
#define _HOSTPORTAL_LPW0IF_LPW0IF22_MASK           0x400000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IF22            */
#define _HOSTPORTAL_LPW0IF_LPW0IF22_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF22_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF22_DEFAULT << 22) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF23                 (0x1UL << 23)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF23_SHIFT          23                                          /**< Shift value for HOSTPORTAL_LPW0IF23         */
#define _HOSTPORTAL_LPW0IF_LPW0IF23_MASK           0x800000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IF23            */
#define _HOSTPORTAL_LPW0IF_LPW0IF23_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF23_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF23_DEFAULT << 23) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF24                 (0x1UL << 24)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF24_SHIFT          24                                          /**< Shift value for HOSTPORTAL_LPW0IF24         */
#define _HOSTPORTAL_LPW0IF_LPW0IF24_MASK           0x1000000UL                                 /**< Bit mask for HOSTPORTAL_LPW0IF24            */
#define _HOSTPORTAL_LPW0IF_LPW0IF24_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF24_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF24_DEFAULT << 24) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF25                 (0x1UL << 25)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF25_SHIFT          25                                          /**< Shift value for HOSTPORTAL_LPW0IF25         */
#define _HOSTPORTAL_LPW0IF_LPW0IF25_MASK           0x2000000UL                                 /**< Bit mask for HOSTPORTAL_LPW0IF25            */
#define _HOSTPORTAL_LPW0IF_LPW0IF25_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF25_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF25_DEFAULT << 25) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF26                 (0x1UL << 26)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF26_SHIFT          26                                          /**< Shift value for HOSTPORTAL_LPW0IF26         */
#define _HOSTPORTAL_LPW0IF_LPW0IF26_MASK           0x4000000UL                                 /**< Bit mask for HOSTPORTAL_LPW0IF26            */
#define _HOSTPORTAL_LPW0IF_LPW0IF26_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF26_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF26_DEFAULT << 26) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF27                 (0x1UL << 27)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF27_SHIFT          27                                          /**< Shift value for HOSTPORTAL_LPW0IF27         */
#define _HOSTPORTAL_LPW0IF_LPW0IF27_MASK           0x8000000UL                                 /**< Bit mask for HOSTPORTAL_LPW0IF27            */
#define _HOSTPORTAL_LPW0IF_LPW0IF27_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF27_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF27_DEFAULT << 27) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF28                 (0x1UL << 28)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF28_SHIFT          28                                          /**< Shift value for HOSTPORTAL_LPW0IF28         */
#define _HOSTPORTAL_LPW0IF_LPW0IF28_MASK           0x10000000UL                                /**< Bit mask for HOSTPORTAL_LPW0IF28            */
#define _HOSTPORTAL_LPW0IF_LPW0IF28_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF28_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF28_DEFAULT << 28) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF29                 (0x1UL << 29)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF29_SHIFT          29                                          /**< Shift value for HOSTPORTAL_LPW0IF29         */
#define _HOSTPORTAL_LPW0IF_LPW0IF29_MASK           0x20000000UL                                /**< Bit mask for HOSTPORTAL_LPW0IF29            */
#define _HOSTPORTAL_LPW0IF_LPW0IF29_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF29_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF29_DEFAULT << 29) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF30                 (0x1UL << 30)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF30_SHIFT          30                                          /**< Shift value for HOSTPORTAL_LPW0IF30         */
#define _HOSTPORTAL_LPW0IF_LPW0IF30_MASK           0x40000000UL                                /**< Bit mask for HOSTPORTAL_LPW0IF30            */
#define _HOSTPORTAL_LPW0IF_LPW0IF30_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF30_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF30_DEFAULT << 30) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */
#define HOSTPORTAL_LPW0IF_LPW0IF31                 (0x1UL << 31)                               /**< Interrupt Flag Register                     */
#define _HOSTPORTAL_LPW0IF_LPW0IF31_SHIFT          31                                          /**< Shift value for HOSTPORTAL_LPW0IF31         */
#define _HOSTPORTAL_LPW0IF_LPW0IF31_MASK           0x80000000UL                                /**< Bit mask for HOSTPORTAL_LPW0IF31            */
#define _HOSTPORTAL_LPW0IF_LPW0IF31_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for HOSTPORTAL_LPW0IF          */
#define HOSTPORTAL_LPW0IF_LPW0IF31_DEFAULT         (_HOSTPORTAL_LPW0IF_LPW0IF31_DEFAULT << 31) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IF  */

/* Bit fields for HOSTPORTAL LPW0IEN */
#define _HOSTPORTAL_LPW0IEN_RESETVALUE             0x00000000UL                                  /**< Default value for HOSTPORTAL_LPW0IEN        */
#define _HOSTPORTAL_LPW0IEN_MASK                   0xFFFFFFFFUL                                  /**< Mask for HOSTPORTAL_LPW0IEN                 */
#define HOSTPORTAL_LPW0IEN_LPW0IEN0                (0x1UL << 0)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN0_SHIFT         0                                             /**< Shift value for HOSTPORTAL_LPW0IEN0         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN0_MASK          0x1UL                                         /**< Bit mask for HOSTPORTAL_LPW0IEN0            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN0_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN0_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN0_DEFAULT << 0)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN1                (0x1UL << 1)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN1_SHIFT         1                                             /**< Shift value for HOSTPORTAL_LPW0IEN1         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN1_MASK          0x2UL                                         /**< Bit mask for HOSTPORTAL_LPW0IEN1            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN1_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN1_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN1_DEFAULT << 1)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN2                (0x1UL << 2)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN2_SHIFT         2                                             /**< Shift value for HOSTPORTAL_LPW0IEN2         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN2_MASK          0x4UL                                         /**< Bit mask for HOSTPORTAL_LPW0IEN2            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN2_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN2_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN2_DEFAULT << 2)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN3                (0x1UL << 3)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN3_SHIFT         3                                             /**< Shift value for HOSTPORTAL_LPW0IEN3         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN3_MASK          0x8UL                                         /**< Bit mask for HOSTPORTAL_LPW0IEN3            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN3_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN3_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN3_DEFAULT << 3)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN4                (0x1UL << 4)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN4_SHIFT         4                                             /**< Shift value for HOSTPORTAL_LPW0IEN4         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN4_MASK          0x10UL                                        /**< Bit mask for HOSTPORTAL_LPW0IEN4            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN4_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN4_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN4_DEFAULT << 4)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN5                (0x1UL << 5)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN5_SHIFT         5                                             /**< Shift value for HOSTPORTAL_LPW0IEN5         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN5_MASK          0x20UL                                        /**< Bit mask for HOSTPORTAL_LPW0IEN5            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN5_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN5_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN5_DEFAULT << 5)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN6                (0x1UL << 6)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN6_SHIFT         6                                             /**< Shift value for HOSTPORTAL_LPW0IEN6         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN6_MASK          0x40UL                                        /**< Bit mask for HOSTPORTAL_LPW0IEN6            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN6_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN6_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN6_DEFAULT << 6)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN7                (0x1UL << 7)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN7_SHIFT         7                                             /**< Shift value for HOSTPORTAL_LPW0IEN7         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN7_MASK          0x80UL                                        /**< Bit mask for HOSTPORTAL_LPW0IEN7            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN7_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN7_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN7_DEFAULT << 7)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN8                (0x1UL << 8)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN8_SHIFT         8                                             /**< Shift value for HOSTPORTAL_LPW0IEN8         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN8_MASK          0x100UL                                       /**< Bit mask for HOSTPORTAL_LPW0IEN8            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN8_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN8_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN8_DEFAULT << 8)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN9                (0x1UL << 9)                                  /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN9_SHIFT         9                                             /**< Shift value for HOSTPORTAL_LPW0IEN9         */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN9_MASK          0x200UL                                       /**< Bit mask for HOSTPORTAL_LPW0IEN9            */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN9_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN9_DEFAULT        (_HOSTPORTAL_LPW0IEN_LPW0IEN9_DEFAULT << 9)   /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN10               (0x1UL << 10)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN10_SHIFT        10                                            /**< Shift value for HOSTPORTAL_LPW0IEN10        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN10_MASK         0x400UL                                       /**< Bit mask for HOSTPORTAL_LPW0IEN10           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN10_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN10_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN10_DEFAULT << 10) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN11               (0x1UL << 11)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN11_SHIFT        11                                            /**< Shift value for HOSTPORTAL_LPW0IEN11        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN11_MASK         0x800UL                                       /**< Bit mask for HOSTPORTAL_LPW0IEN11           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN11_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN11_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN11_DEFAULT << 11) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN12               (0x1UL << 12)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN12_SHIFT        12                                            /**< Shift value for HOSTPORTAL_LPW0IEN12        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN12_MASK         0x1000UL                                      /**< Bit mask for HOSTPORTAL_LPW0IEN12           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN12_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN12_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN12_DEFAULT << 12) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN13               (0x1UL << 13)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN13_SHIFT        13                                            /**< Shift value for HOSTPORTAL_LPW0IEN13        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN13_MASK         0x2000UL                                      /**< Bit mask for HOSTPORTAL_LPW0IEN13           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN13_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN13_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN13_DEFAULT << 13) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN14               (0x1UL << 14)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN14_SHIFT        14                                            /**< Shift value for HOSTPORTAL_LPW0IEN14        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN14_MASK         0x4000UL                                      /**< Bit mask for HOSTPORTAL_LPW0IEN14           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN14_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN14_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN14_DEFAULT << 14) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN15               (0x1UL << 15)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN15_SHIFT        15                                            /**< Shift value for HOSTPORTAL_LPW0IEN15        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN15_MASK         0x8000UL                                      /**< Bit mask for HOSTPORTAL_LPW0IEN15           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN15_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN15_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN15_DEFAULT << 15) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN16               (0x1UL << 16)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN16_SHIFT        16                                            /**< Shift value for HOSTPORTAL_LPW0IEN16        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN16_MASK         0x10000UL                                     /**< Bit mask for HOSTPORTAL_LPW0IEN16           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN16_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN16_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN16_DEFAULT << 16) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN17               (0x1UL << 17)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN17_SHIFT        17                                            /**< Shift value for HOSTPORTAL_LPW0IEN17        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN17_MASK         0x20000UL                                     /**< Bit mask for HOSTPORTAL_LPW0IEN17           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN17_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN17_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN17_DEFAULT << 17) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN18               (0x1UL << 18)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN18_SHIFT        18                                            /**< Shift value for HOSTPORTAL_LPW0IEN18        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN18_MASK         0x40000UL                                     /**< Bit mask for HOSTPORTAL_LPW0IEN18           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN18_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN18_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN18_DEFAULT << 18) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN19               (0x1UL << 19)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN19_SHIFT        19                                            /**< Shift value for HOSTPORTAL_LPW0IEN19        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN19_MASK         0x80000UL                                     /**< Bit mask for HOSTPORTAL_LPW0IEN19           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN19_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN19_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN19_DEFAULT << 19) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN20               (0x1UL << 20)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN20_SHIFT        20                                            /**< Shift value for HOSTPORTAL_LPW0IEN20        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN20_MASK         0x100000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IEN20           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN20_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN20_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN20_DEFAULT << 20) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN21               (0x1UL << 21)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN21_SHIFT        21                                            /**< Shift value for HOSTPORTAL_LPW0IEN21        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN21_MASK         0x200000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IEN21           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN21_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN21_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN21_DEFAULT << 21) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN22               (0x1UL << 22)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN22_SHIFT        22                                            /**< Shift value for HOSTPORTAL_LPW0IEN22        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN22_MASK         0x400000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IEN22           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN22_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN22_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN22_DEFAULT << 22) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN23               (0x1UL << 23)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN23_SHIFT        23                                            /**< Shift value for HOSTPORTAL_LPW0IEN23        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN23_MASK         0x800000UL                                    /**< Bit mask for HOSTPORTAL_LPW0IEN23           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN23_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN23_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN23_DEFAULT << 23) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN24               (0x1UL << 24)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN24_SHIFT        24                                            /**< Shift value for HOSTPORTAL_LPW0IEN24        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN24_MASK         0x1000000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IEN24           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN24_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN24_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN24_DEFAULT << 24) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN25               (0x1UL << 25)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN25_SHIFT        25                                            /**< Shift value for HOSTPORTAL_LPW0IEN25        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN25_MASK         0x2000000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IEN25           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN25_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN25_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN25_DEFAULT << 25) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN26               (0x1UL << 26)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN26_SHIFT        26                                            /**< Shift value for HOSTPORTAL_LPW0IEN26        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN26_MASK         0x4000000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IEN26           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN26_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN26_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN26_DEFAULT << 26) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN27               (0x1UL << 27)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN27_SHIFT        27                                            /**< Shift value for HOSTPORTAL_LPW0IEN27        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN27_MASK         0x8000000UL                                   /**< Bit mask for HOSTPORTAL_LPW0IEN27           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN27_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN27_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN27_DEFAULT << 27) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN28               (0x1UL << 28)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN28_SHIFT        28                                            /**< Shift value for HOSTPORTAL_LPW0IEN28        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN28_MASK         0x10000000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IEN28           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN28_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN28_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN28_DEFAULT << 28) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN29               (0x1UL << 29)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN29_SHIFT        29                                            /**< Shift value for HOSTPORTAL_LPW0IEN29        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN29_MASK         0x20000000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IEN29           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN29_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN29_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN29_DEFAULT << 29) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN30               (0x1UL << 30)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN30_SHIFT        30                                            /**< Shift value for HOSTPORTAL_LPW0IEN30        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN30_MASK         0x40000000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IEN30           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN30_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN30_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN30_DEFAULT << 30) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */
#define HOSTPORTAL_LPW0IEN_LPW0IEN31               (0x1UL << 31)                                 /**< Interrupt Enable Register                   */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN31_SHIFT        31                                            /**< Shift value for HOSTPORTAL_LPW0IEN31        */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN31_MASK         0x80000000UL                                  /**< Bit mask for HOSTPORTAL_LPW0IEN31           */
#define _HOSTPORTAL_LPW0IEN_LPW0IEN31_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for HOSTPORTAL_LPW0IEN         */
#define HOSTPORTAL_LPW0IEN_LPW0IEN31_DEFAULT       (_HOSTPORTAL_LPW0IEN_LPW0IEN31_DEFAULT << 31) /**< Shifted mode DEFAULT for HOSTPORTAL_LPW0IEN */

/* Bit fields for HOSTPORTAL MAILBOXREG0 */
#define _HOSTPORTAL_MAILBOXREG0_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG0    */
#define _HOSTPORTAL_MAILBOXREG0_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG0             */
#define _HOSTPORTAL_MAILBOXREG0_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG0_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG0_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG0     */
#define HOSTPORTAL_MAILBOXREG0_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG0_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG0*/

/* Bit fields for HOSTPORTAL MAILBOXREG1 */
#define _HOSTPORTAL_MAILBOXREG1_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG1    */
#define _HOSTPORTAL_MAILBOXREG1_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG1             */
#define _HOSTPORTAL_MAILBOXREG1_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG1_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG1_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG1     */
#define HOSTPORTAL_MAILBOXREG1_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG1_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG1*/

/* Bit fields for HOSTPORTAL MAILBOXREG2 */
#define _HOSTPORTAL_MAILBOXREG2_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG2    */
#define _HOSTPORTAL_MAILBOXREG2_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG2             */
#define _HOSTPORTAL_MAILBOXREG2_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG2_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG2_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG2     */
#define HOSTPORTAL_MAILBOXREG2_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG2_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG2*/

/* Bit fields for HOSTPORTAL MAILBOXREG3 */
#define _HOSTPORTAL_MAILBOXREG3_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG3    */
#define _HOSTPORTAL_MAILBOXREG3_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG3             */
#define _HOSTPORTAL_MAILBOXREG3_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG3_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG3_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG3     */
#define HOSTPORTAL_MAILBOXREG3_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG3_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG3*/

/* Bit fields for HOSTPORTAL MAILBOXREG4 */
#define _HOSTPORTAL_MAILBOXREG4_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG4    */
#define _HOSTPORTAL_MAILBOXREG4_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG4             */
#define _HOSTPORTAL_MAILBOXREG4_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG4_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG4_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG4     */
#define HOSTPORTAL_MAILBOXREG4_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG4_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG4*/

/* Bit fields for HOSTPORTAL MAILBOXREG5 */
#define _HOSTPORTAL_MAILBOXREG5_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG5    */
#define _HOSTPORTAL_MAILBOXREG5_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG5             */
#define _HOSTPORTAL_MAILBOXREG5_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG5_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG5_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG5     */
#define HOSTPORTAL_MAILBOXREG5_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG5_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG5*/

/* Bit fields for HOSTPORTAL MAILBOXREG6 */
#define _HOSTPORTAL_MAILBOXREG6_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG6    */
#define _HOSTPORTAL_MAILBOXREG6_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG6             */
#define _HOSTPORTAL_MAILBOXREG6_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG6_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG6_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG6     */
#define HOSTPORTAL_MAILBOXREG6_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG6_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG6*/

/* Bit fields for HOSTPORTAL MAILBOXREG7 */
#define _HOSTPORTAL_MAILBOXREG7_RESETVALUE         0x00000000UL                                   /**< Default value for HOSTPORTAL_MAILBOXREG7    */
#define _HOSTPORTAL_MAILBOXREG7_MASK               0xFFFFFFFFUL                                   /**< Mask for HOSTPORTAL_MAILBOXREG7             */
#define _HOSTPORTAL_MAILBOXREG7_MESSAGE_SHIFT      0                                              /**< Shift value for HOSTPORTAL_MESSAGE          */
#define _HOSTPORTAL_MAILBOXREG7_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for HOSTPORTAL_MESSAGE             */
#define _HOSTPORTAL_MAILBOXREG7_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for HOSTPORTAL_MAILBOXREG7     */
#define HOSTPORTAL_MAILBOXREG7_MESSAGE_DEFAULT     (_HOSTPORTAL_MAILBOXREG7_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for HOSTPORTAL_MAILBOXREG7*/

/** @} End of group SIMG301_HOSTPORTAL_BitFields */
/** @} End of group SIMG301_HOSTPORTAL */
/** @} End of group Parts */

#endif // SIMG301_HOSTPORTAL_H
