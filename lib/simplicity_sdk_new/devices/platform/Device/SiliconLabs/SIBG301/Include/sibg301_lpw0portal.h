/**************************************************************************//**
 * @file
 * @brief SIBG301 LPW0PORTAL register and bit field definitions
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
#ifndef SIBG301_LPW0PORTAL_H
#define SIBG301_LPW0PORTAL_H
#define LPW0PORTAL_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_LPW0PORTAL LPW0PORTAL
 * @{
 * @brief SIBG301 LPW0PORTAL Register Declaration.
 *****************************************************************************/

/** LPW0PORTAL Register Declaration. */
typedef struct lpw0portal_typedef{
  __IM uint32_t  IPID;                          /**< IP version ID                                      */
  __IOM uint32_t CTRL;                          /**< LPW0PORTAL Control Register                        */
  __IM uint32_t  STATUS;                        /**< LPW0PORTAL Status Register                         */
  __IOM uint32_t HOSTIF;                        /**< Host Interrupt Flag                                */
  __IOM uint32_t HOSTIEN;                       /**< Host Interrupt Enable                              */
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
  __IOM uint32_t CTRL_SET;                      /**< LPW0PORTAL Control Register                        */
  __IM uint32_t  STATUS_SET;                    /**< LPW0PORTAL Status Register                         */
  __IOM uint32_t HOSTIF_SET;                    /**< Host Interrupt Flag                                */
  __IOM uint32_t HOSTIEN_SET;                   /**< Host Interrupt Enable                              */
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
  __IOM uint32_t CTRL_CLR;                      /**< LPW0PORTAL Control Register                        */
  __IM uint32_t  STATUS_CLR;                    /**< LPW0PORTAL Status Register                         */
  __IOM uint32_t HOSTIF_CLR;                    /**< Host Interrupt Flag                                */
  __IOM uint32_t HOSTIEN_CLR;                   /**< Host Interrupt Enable                              */
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
  __IOM uint32_t CTRL_TGL;                      /**< LPW0PORTAL Control Register                        */
  __IM uint32_t  STATUS_TGL;                    /**< LPW0PORTAL Status Register                         */
  __IOM uint32_t HOSTIF_TGL;                    /**< Host Interrupt Flag                                */
  __IOM uint32_t HOSTIEN_TGL;                   /**< Host Interrupt Enable                              */
  __IOM uint32_t MAILBOXREG0_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG1_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG2_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG3_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG4_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG5_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG6_TGL;               /**< MB register                                        */
  __IOM uint32_t MAILBOXREG7_TGL;               /**< MB register                                        */
} LPW0PORTAL_TypeDef;
/** @} End of group SIBG301_LPW0PORTAL */

/**************************************************************************//**
 * @addtogroup SIBG301_LPW0PORTAL
 * @{
 * @defgroup SIBG301_LPW0PORTAL_BitFields LPW0PORTAL Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for LPW0PORTAL IPID */
#define _LPW0PORTAL_IPID_RESETVALUE                0x00000001UL                              /**< Default value for LPW0PORTAL_IPID           */
#define _LPW0PORTAL_IPID_MASK                      0xFFFFFFFFUL                              /**< Mask for LPW0PORTAL_IPID                    */
#define _LPW0PORTAL_IPID_IPVERSION_SHIFT           0                                         /**< Shift value for LPW0PORTAL_IPVERSION        */
#define _LPW0PORTAL_IPID_IPVERSION_MASK            0xFFFFFFFFUL                              /**< Bit mask for LPW0PORTAL_IPVERSION           */
#define _LPW0PORTAL_IPID_IPVERSION_DEFAULT         0x00000001UL                              /**< Mode DEFAULT for LPW0PORTAL_IPID            */
#define LPW0PORTAL_IPID_IPVERSION_DEFAULT          (_LPW0PORTAL_IPID_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_IPID    */

/* Bit fields for LPW0PORTAL CTRL */
#define _LPW0PORTAL_CTRL_RESETVALUE                0x00000000UL                                 /**< Default value for LPW0PORTAL_CTRL           */
#define _LPW0PORTAL_CTRL_MASK                      0x00000001UL                                 /**< Mask for LPW0PORTAL_CTRL                    */
#define LPW0PORTAL_CTRL_HOSTPWRUPREQ               (0x1UL << 0)                                 /**< HOST Power Up Request                       */
#define _LPW0PORTAL_CTRL_HOSTPWRUPREQ_SHIFT        0                                            /**< Shift value for LPW0PORTAL_HOSTPWRUPREQ     */
#define _LPW0PORTAL_CTRL_HOSTPWRUPREQ_MASK         0x1UL                                        /**< Bit mask for LPW0PORTAL_HOSTPWRUPREQ        */
#define _LPW0PORTAL_CTRL_HOSTPWRUPREQ_DEFAULT      0x00000000UL                                 /**< Mode DEFAULT for LPW0PORTAL_CTRL            */
#define LPW0PORTAL_CTRL_HOSTPWRUPREQ_DEFAULT       (_LPW0PORTAL_CTRL_HOSTPWRUPREQ_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_CTRL    */

/* Bit fields for LPW0PORTAL STATUS */
#define _LPW0PORTAL_STATUS_RESETVALUE              0x00000000UL                                   /**< Default value for LPW0PORTAL_STATUS         */
#define _LPW0PORTAL_STATUS_MASK                    0x00000001UL                                   /**< Mask for LPW0PORTAL_STATUS                  */
#define LPW0PORTAL_STATUS_HOSTPWRUPACK             (0x1UL << 0)                                   /**< Host Power up status                        */
#define _LPW0PORTAL_STATUS_HOSTPWRUPACK_SHIFT      0                                              /**< Shift value for LPW0PORTAL_HOSTPWRUPACK     */
#define _LPW0PORTAL_STATUS_HOSTPWRUPACK_MASK       0x1UL                                          /**< Bit mask for LPW0PORTAL_HOSTPWRUPACK        */
#define _LPW0PORTAL_STATUS_HOSTPWRUPACK_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_STATUS          */
#define LPW0PORTAL_STATUS_HOSTPWRUPACK_DEFAULT     (_LPW0PORTAL_STATUS_HOSTPWRUPACK_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_STATUS  */

/* Bit fields for LPW0PORTAL HOSTIF */
#define _LPW0PORTAL_HOSTIF_RESETVALUE              0x00000000UL                                /**< Default value for LPW0PORTAL_HOSTIF         */
#define _LPW0PORTAL_HOSTIF_MASK                    0xFFFFFFFFUL                                /**< Mask for LPW0PORTAL_HOSTIF                  */
#define LPW0PORTAL_HOSTIF_HOSTIF0                  (0x1UL << 0)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF0_SHIFT           0                                           /**< Shift value for LPW0PORTAL_HOSTIF0          */
#define _LPW0PORTAL_HOSTIF_HOSTIF0_MASK            0x1UL                                       /**< Bit mask for LPW0PORTAL_HOSTIF0             */
#define _LPW0PORTAL_HOSTIF_HOSTIF0_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF0_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF0_DEFAULT << 0)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF1                  (0x1UL << 1)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF1_SHIFT           1                                           /**< Shift value for LPW0PORTAL_HOSTIF1          */
#define _LPW0PORTAL_HOSTIF_HOSTIF1_MASK            0x2UL                                       /**< Bit mask for LPW0PORTAL_HOSTIF1             */
#define _LPW0PORTAL_HOSTIF_HOSTIF1_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF1_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF1_DEFAULT << 1)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF2                  (0x1UL << 2)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF2_SHIFT           2                                           /**< Shift value for LPW0PORTAL_HOSTIF2          */
#define _LPW0PORTAL_HOSTIF_HOSTIF2_MASK            0x4UL                                       /**< Bit mask for LPW0PORTAL_HOSTIF2             */
#define _LPW0PORTAL_HOSTIF_HOSTIF2_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF2_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF2_DEFAULT << 2)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF3                  (0x1UL << 3)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF3_SHIFT           3                                           /**< Shift value for LPW0PORTAL_HOSTIF3          */
#define _LPW0PORTAL_HOSTIF_HOSTIF3_MASK            0x8UL                                       /**< Bit mask for LPW0PORTAL_HOSTIF3             */
#define _LPW0PORTAL_HOSTIF_HOSTIF3_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF3_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF3_DEFAULT << 3)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF4                  (0x1UL << 4)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF4_SHIFT           4                                           /**< Shift value for LPW0PORTAL_HOSTIF4          */
#define _LPW0PORTAL_HOSTIF_HOSTIF4_MASK            0x10UL                                      /**< Bit mask for LPW0PORTAL_HOSTIF4             */
#define _LPW0PORTAL_HOSTIF_HOSTIF4_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF4_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF4_DEFAULT << 4)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF5                  (0x1UL << 5)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF5_SHIFT           5                                           /**< Shift value for LPW0PORTAL_HOSTIF5          */
#define _LPW0PORTAL_HOSTIF_HOSTIF5_MASK            0x20UL                                      /**< Bit mask for LPW0PORTAL_HOSTIF5             */
#define _LPW0PORTAL_HOSTIF_HOSTIF5_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF5_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF5_DEFAULT << 5)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF6                  (0x1UL << 6)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF6_SHIFT           6                                           /**< Shift value for LPW0PORTAL_HOSTIF6          */
#define _LPW0PORTAL_HOSTIF_HOSTIF6_MASK            0x40UL                                      /**< Bit mask for LPW0PORTAL_HOSTIF6             */
#define _LPW0PORTAL_HOSTIF_HOSTIF6_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF6_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF6_DEFAULT << 6)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF7                  (0x1UL << 7)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF7_SHIFT           7                                           /**< Shift value for LPW0PORTAL_HOSTIF7          */
#define _LPW0PORTAL_HOSTIF_HOSTIF7_MASK            0x80UL                                      /**< Bit mask for LPW0PORTAL_HOSTIF7             */
#define _LPW0PORTAL_HOSTIF_HOSTIF7_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF7_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF7_DEFAULT << 7)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF8                  (0x1UL << 8)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF8_SHIFT           8                                           /**< Shift value for LPW0PORTAL_HOSTIF8          */
#define _LPW0PORTAL_HOSTIF_HOSTIF8_MASK            0x100UL                                     /**< Bit mask for LPW0PORTAL_HOSTIF8             */
#define _LPW0PORTAL_HOSTIF_HOSTIF8_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF8_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF8_DEFAULT << 8)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF9                  (0x1UL << 9)                                /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF9_SHIFT           9                                           /**< Shift value for LPW0PORTAL_HOSTIF9          */
#define _LPW0PORTAL_HOSTIF_HOSTIF9_MASK            0x200UL                                     /**< Bit mask for LPW0PORTAL_HOSTIF9             */
#define _LPW0PORTAL_HOSTIF_HOSTIF9_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF9_DEFAULT          (_LPW0PORTAL_HOSTIF_HOSTIF9_DEFAULT << 9)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF10                 (0x1UL << 10)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF10_SHIFT          10                                          /**< Shift value for LPW0PORTAL_HOSTIF10         */
#define _LPW0PORTAL_HOSTIF_HOSTIF10_MASK           0x400UL                                     /**< Bit mask for LPW0PORTAL_HOSTIF10            */
#define _LPW0PORTAL_HOSTIF_HOSTIF10_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF10_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF10_DEFAULT << 10) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF11                 (0x1UL << 11)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF11_SHIFT          11                                          /**< Shift value for LPW0PORTAL_HOSTIF11         */
#define _LPW0PORTAL_HOSTIF_HOSTIF11_MASK           0x800UL                                     /**< Bit mask for LPW0PORTAL_HOSTIF11            */
#define _LPW0PORTAL_HOSTIF_HOSTIF11_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF11_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF11_DEFAULT << 11) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF12                 (0x1UL << 12)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF12_SHIFT          12                                          /**< Shift value for LPW0PORTAL_HOSTIF12         */
#define _LPW0PORTAL_HOSTIF_HOSTIF12_MASK           0x1000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIF12            */
#define _LPW0PORTAL_HOSTIF_HOSTIF12_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF12_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF12_DEFAULT << 12) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF13                 (0x1UL << 13)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF13_SHIFT          13                                          /**< Shift value for LPW0PORTAL_HOSTIF13         */
#define _LPW0PORTAL_HOSTIF_HOSTIF13_MASK           0x2000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIF13            */
#define _LPW0PORTAL_HOSTIF_HOSTIF13_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF13_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF13_DEFAULT << 13) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF14                 (0x1UL << 14)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF14_SHIFT          14                                          /**< Shift value for LPW0PORTAL_HOSTIF14         */
#define _LPW0PORTAL_HOSTIF_HOSTIF14_MASK           0x4000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIF14            */
#define _LPW0PORTAL_HOSTIF_HOSTIF14_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF14_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF14_DEFAULT << 14) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF15                 (0x1UL << 15)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF15_SHIFT          15                                          /**< Shift value for LPW0PORTAL_HOSTIF15         */
#define _LPW0PORTAL_HOSTIF_HOSTIF15_MASK           0x8000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIF15            */
#define _LPW0PORTAL_HOSTIF_HOSTIF15_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF15_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF15_DEFAULT << 15) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF16                 (0x1UL << 16)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF16_SHIFT          16                                          /**< Shift value for LPW0PORTAL_HOSTIF16         */
#define _LPW0PORTAL_HOSTIF_HOSTIF16_MASK           0x10000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIF16            */
#define _LPW0PORTAL_HOSTIF_HOSTIF16_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF16_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF16_DEFAULT << 16) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF17                 (0x1UL << 17)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF17_SHIFT          17                                          /**< Shift value for LPW0PORTAL_HOSTIF17         */
#define _LPW0PORTAL_HOSTIF_HOSTIF17_MASK           0x20000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIF17            */
#define _LPW0PORTAL_HOSTIF_HOSTIF17_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF17_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF17_DEFAULT << 17) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF18                 (0x1UL << 18)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF18_SHIFT          18                                          /**< Shift value for LPW0PORTAL_HOSTIF18         */
#define _LPW0PORTAL_HOSTIF_HOSTIF18_MASK           0x40000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIF18            */
#define _LPW0PORTAL_HOSTIF_HOSTIF18_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF18_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF18_DEFAULT << 18) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF19                 (0x1UL << 19)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF19_SHIFT          19                                          /**< Shift value for LPW0PORTAL_HOSTIF19         */
#define _LPW0PORTAL_HOSTIF_HOSTIF19_MASK           0x80000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIF19            */
#define _LPW0PORTAL_HOSTIF_HOSTIF19_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF19_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF19_DEFAULT << 19) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF20                 (0x1UL << 20)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF20_SHIFT          20                                          /**< Shift value for LPW0PORTAL_HOSTIF20         */
#define _LPW0PORTAL_HOSTIF_HOSTIF20_MASK           0x100000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIF20            */
#define _LPW0PORTAL_HOSTIF_HOSTIF20_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF20_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF20_DEFAULT << 20) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF21                 (0x1UL << 21)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF21_SHIFT          21                                          /**< Shift value for LPW0PORTAL_HOSTIF21         */
#define _LPW0PORTAL_HOSTIF_HOSTIF21_MASK           0x200000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIF21            */
#define _LPW0PORTAL_HOSTIF_HOSTIF21_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF21_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF21_DEFAULT << 21) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF22                 (0x1UL << 22)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF22_SHIFT          22                                          /**< Shift value for LPW0PORTAL_HOSTIF22         */
#define _LPW0PORTAL_HOSTIF_HOSTIF22_MASK           0x400000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIF22            */
#define _LPW0PORTAL_HOSTIF_HOSTIF22_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF22_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF22_DEFAULT << 22) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF23                 (0x1UL << 23)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF23_SHIFT          23                                          /**< Shift value for LPW0PORTAL_HOSTIF23         */
#define _LPW0PORTAL_HOSTIF_HOSTIF23_MASK           0x800000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIF23            */
#define _LPW0PORTAL_HOSTIF_HOSTIF23_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF23_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF23_DEFAULT << 23) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF24                 (0x1UL << 24)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF24_SHIFT          24                                          /**< Shift value for LPW0PORTAL_HOSTIF24         */
#define _LPW0PORTAL_HOSTIF_HOSTIF24_MASK           0x1000000UL                                 /**< Bit mask for LPW0PORTAL_HOSTIF24            */
#define _LPW0PORTAL_HOSTIF_HOSTIF24_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF24_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF24_DEFAULT << 24) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF25                 (0x1UL << 25)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF25_SHIFT          25                                          /**< Shift value for LPW0PORTAL_HOSTIF25         */
#define _LPW0PORTAL_HOSTIF_HOSTIF25_MASK           0x2000000UL                                 /**< Bit mask for LPW0PORTAL_HOSTIF25            */
#define _LPW0PORTAL_HOSTIF_HOSTIF25_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF25_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF25_DEFAULT << 25) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF26                 (0x1UL << 26)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF26_SHIFT          26                                          /**< Shift value for LPW0PORTAL_HOSTIF26         */
#define _LPW0PORTAL_HOSTIF_HOSTIF26_MASK           0x4000000UL                                 /**< Bit mask for LPW0PORTAL_HOSTIF26            */
#define _LPW0PORTAL_HOSTIF_HOSTIF26_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF26_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF26_DEFAULT << 26) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF27                 (0x1UL << 27)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF27_SHIFT          27                                          /**< Shift value for LPW0PORTAL_HOSTIF27         */
#define _LPW0PORTAL_HOSTIF_HOSTIF27_MASK           0x8000000UL                                 /**< Bit mask for LPW0PORTAL_HOSTIF27            */
#define _LPW0PORTAL_HOSTIF_HOSTIF27_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF27_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF27_DEFAULT << 27) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF28                 (0x1UL << 28)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF28_SHIFT          28                                          /**< Shift value for LPW0PORTAL_HOSTIF28         */
#define _LPW0PORTAL_HOSTIF_HOSTIF28_MASK           0x10000000UL                                /**< Bit mask for LPW0PORTAL_HOSTIF28            */
#define _LPW0PORTAL_HOSTIF_HOSTIF28_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF28_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF28_DEFAULT << 28) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF29                 (0x1UL << 29)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF29_SHIFT          29                                          /**< Shift value for LPW0PORTAL_HOSTIF29         */
#define _LPW0PORTAL_HOSTIF_HOSTIF29_MASK           0x20000000UL                                /**< Bit mask for LPW0PORTAL_HOSTIF29            */
#define _LPW0PORTAL_HOSTIF_HOSTIF29_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF29_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF29_DEFAULT << 29) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF30                 (0x1UL << 30)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF30_SHIFT          30                                          /**< Shift value for LPW0PORTAL_HOSTIF30         */
#define _LPW0PORTAL_HOSTIF_HOSTIF30_MASK           0x40000000UL                                /**< Bit mask for LPW0PORTAL_HOSTIF30            */
#define _LPW0PORTAL_HOSTIF_HOSTIF30_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF30_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF30_DEFAULT << 30) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */
#define LPW0PORTAL_HOSTIF_HOSTIF31                 (0x1UL << 31)                               /**< Interrupt Flag Register                     */
#define _LPW0PORTAL_HOSTIF_HOSTIF31_SHIFT          31                                          /**< Shift value for LPW0PORTAL_HOSTIF31         */
#define _LPW0PORTAL_HOSTIF_HOSTIF31_MASK           0x80000000UL                                /**< Bit mask for LPW0PORTAL_HOSTIF31            */
#define _LPW0PORTAL_HOSTIF_HOSTIF31_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LPW0PORTAL_HOSTIF          */
#define LPW0PORTAL_HOSTIF_HOSTIF31_DEFAULT         (_LPW0PORTAL_HOSTIF_HOSTIF31_DEFAULT << 31) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIF  */

/* Bit fields for LPW0PORTAL HOSTIEN */
#define _LPW0PORTAL_HOSTIEN_RESETVALUE             0x00000000UL                                  /**< Default value for LPW0PORTAL_HOSTIEN        */
#define _LPW0PORTAL_HOSTIEN_MASK                   0xFFFFFFFFUL                                  /**< Mask for LPW0PORTAL_HOSTIEN                 */
#define LPW0PORTAL_HOSTIEN_HOSTIEN0                (0x1UL << 0)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN0_SHIFT         0                                             /**< Shift value for LPW0PORTAL_HOSTIEN0         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN0_MASK          0x1UL                                         /**< Bit mask for LPW0PORTAL_HOSTIEN0            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN0_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN0_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN0_DEFAULT << 0)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN1                (0x1UL << 1)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN1_SHIFT         1                                             /**< Shift value for LPW0PORTAL_HOSTIEN1         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN1_MASK          0x2UL                                         /**< Bit mask for LPW0PORTAL_HOSTIEN1            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN1_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN1_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN1_DEFAULT << 1)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN2                (0x1UL << 2)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN2_SHIFT         2                                             /**< Shift value for LPW0PORTAL_HOSTIEN2         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN2_MASK          0x4UL                                         /**< Bit mask for LPW0PORTAL_HOSTIEN2            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN2_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN2_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN2_DEFAULT << 2)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN3                (0x1UL << 3)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN3_SHIFT         3                                             /**< Shift value for LPW0PORTAL_HOSTIEN3         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN3_MASK          0x8UL                                         /**< Bit mask for LPW0PORTAL_HOSTIEN3            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN3_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN3_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN3_DEFAULT << 3)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN4                (0x1UL << 4)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN4_SHIFT         4                                             /**< Shift value for LPW0PORTAL_HOSTIEN4         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN4_MASK          0x10UL                                        /**< Bit mask for LPW0PORTAL_HOSTIEN4            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN4_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN4_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN4_DEFAULT << 4)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN5                (0x1UL << 5)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN5_SHIFT         5                                             /**< Shift value for LPW0PORTAL_HOSTIEN5         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN5_MASK          0x20UL                                        /**< Bit mask for LPW0PORTAL_HOSTIEN5            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN5_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN5_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN5_DEFAULT << 5)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN6                (0x1UL << 6)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN6_SHIFT         6                                             /**< Shift value for LPW0PORTAL_HOSTIEN6         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN6_MASK          0x40UL                                        /**< Bit mask for LPW0PORTAL_HOSTIEN6            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN6_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN6_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN6_DEFAULT << 6)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN7                (0x1UL << 7)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN7_SHIFT         7                                             /**< Shift value for LPW0PORTAL_HOSTIEN7         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN7_MASK          0x80UL                                        /**< Bit mask for LPW0PORTAL_HOSTIEN7            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN7_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN7_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN7_DEFAULT << 7)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN8                (0x1UL << 8)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN8_SHIFT         8                                             /**< Shift value for LPW0PORTAL_HOSTIEN8         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN8_MASK          0x100UL                                       /**< Bit mask for LPW0PORTAL_HOSTIEN8            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN8_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN8_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN8_DEFAULT << 8)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN9                (0x1UL << 9)                                  /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN9_SHIFT         9                                             /**< Shift value for LPW0PORTAL_HOSTIEN9         */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN9_MASK          0x200UL                                       /**< Bit mask for LPW0PORTAL_HOSTIEN9            */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN9_DEFAULT       0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN9_DEFAULT        (_LPW0PORTAL_HOSTIEN_HOSTIEN9_DEFAULT << 9)   /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN10               (0x1UL << 10)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN10_SHIFT        10                                            /**< Shift value for LPW0PORTAL_HOSTIEN10        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN10_MASK         0x400UL                                       /**< Bit mask for LPW0PORTAL_HOSTIEN10           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN10_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN10_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN10_DEFAULT << 10) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN11               (0x1UL << 11)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN11_SHIFT        11                                            /**< Shift value for LPW0PORTAL_HOSTIEN11        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN11_MASK         0x800UL                                       /**< Bit mask for LPW0PORTAL_HOSTIEN11           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN11_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN11_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN11_DEFAULT << 11) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN12               (0x1UL << 12)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN12_SHIFT        12                                            /**< Shift value for LPW0PORTAL_HOSTIEN12        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN12_MASK         0x1000UL                                      /**< Bit mask for LPW0PORTAL_HOSTIEN12           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN12_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN12_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN12_DEFAULT << 12) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN13               (0x1UL << 13)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN13_SHIFT        13                                            /**< Shift value for LPW0PORTAL_HOSTIEN13        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN13_MASK         0x2000UL                                      /**< Bit mask for LPW0PORTAL_HOSTIEN13           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN13_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN13_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN13_DEFAULT << 13) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN14               (0x1UL << 14)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN14_SHIFT        14                                            /**< Shift value for LPW0PORTAL_HOSTIEN14        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN14_MASK         0x4000UL                                      /**< Bit mask for LPW0PORTAL_HOSTIEN14           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN14_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN14_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN14_DEFAULT << 14) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN15               (0x1UL << 15)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN15_SHIFT        15                                            /**< Shift value for LPW0PORTAL_HOSTIEN15        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN15_MASK         0x8000UL                                      /**< Bit mask for LPW0PORTAL_HOSTIEN15           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN15_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN15_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN15_DEFAULT << 15) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN16               (0x1UL << 16)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN16_SHIFT        16                                            /**< Shift value for LPW0PORTAL_HOSTIEN16        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN16_MASK         0x10000UL                                     /**< Bit mask for LPW0PORTAL_HOSTIEN16           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN16_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN16_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN16_DEFAULT << 16) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN17               (0x1UL << 17)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN17_SHIFT        17                                            /**< Shift value for LPW0PORTAL_HOSTIEN17        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN17_MASK         0x20000UL                                     /**< Bit mask for LPW0PORTAL_HOSTIEN17           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN17_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN17_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN17_DEFAULT << 17) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN18               (0x1UL << 18)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN18_SHIFT        18                                            /**< Shift value for LPW0PORTAL_HOSTIEN18        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN18_MASK         0x40000UL                                     /**< Bit mask for LPW0PORTAL_HOSTIEN18           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN18_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN18_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN18_DEFAULT << 18) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN19               (0x1UL << 19)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN19_SHIFT        19                                            /**< Shift value for LPW0PORTAL_HOSTIEN19        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN19_MASK         0x80000UL                                     /**< Bit mask for LPW0PORTAL_HOSTIEN19           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN19_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN19_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN19_DEFAULT << 19) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN20               (0x1UL << 20)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN20_SHIFT        20                                            /**< Shift value for LPW0PORTAL_HOSTIEN20        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN20_MASK         0x100000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIEN20           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN20_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN20_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN20_DEFAULT << 20) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN21               (0x1UL << 21)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN21_SHIFT        21                                            /**< Shift value for LPW0PORTAL_HOSTIEN21        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN21_MASK         0x200000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIEN21           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN21_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN21_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN21_DEFAULT << 21) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN22               (0x1UL << 22)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN22_SHIFT        22                                            /**< Shift value for LPW0PORTAL_HOSTIEN22        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN22_MASK         0x400000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIEN22           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN22_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN22_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN22_DEFAULT << 22) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN23               (0x1UL << 23)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN23_SHIFT        23                                            /**< Shift value for LPW0PORTAL_HOSTIEN23        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN23_MASK         0x800000UL                                    /**< Bit mask for LPW0PORTAL_HOSTIEN23           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN23_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN23_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN23_DEFAULT << 23) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN24               (0x1UL << 24)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN24_SHIFT        24                                            /**< Shift value for LPW0PORTAL_HOSTIEN24        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN24_MASK         0x1000000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIEN24           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN24_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN24_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN24_DEFAULT << 24) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN25               (0x1UL << 25)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN25_SHIFT        25                                            /**< Shift value for LPW0PORTAL_HOSTIEN25        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN25_MASK         0x2000000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIEN25           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN25_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN25_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN25_DEFAULT << 25) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN26               (0x1UL << 26)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN26_SHIFT        26                                            /**< Shift value for LPW0PORTAL_HOSTIEN26        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN26_MASK         0x4000000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIEN26           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN26_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN26_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN26_DEFAULT << 26) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN27               (0x1UL << 27)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN27_SHIFT        27                                            /**< Shift value for LPW0PORTAL_HOSTIEN27        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN27_MASK         0x8000000UL                                   /**< Bit mask for LPW0PORTAL_HOSTIEN27           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN27_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN27_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN27_DEFAULT << 27) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN28               (0x1UL << 28)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN28_SHIFT        28                                            /**< Shift value for LPW0PORTAL_HOSTIEN28        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN28_MASK         0x10000000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIEN28           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN28_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN28_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN28_DEFAULT << 28) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN29               (0x1UL << 29)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN29_SHIFT        29                                            /**< Shift value for LPW0PORTAL_HOSTIEN29        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN29_MASK         0x20000000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIEN29           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN29_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN29_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN29_DEFAULT << 29) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN30               (0x1UL << 30)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN30_SHIFT        30                                            /**< Shift value for LPW0PORTAL_HOSTIEN30        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN30_MASK         0x40000000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIEN30           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN30_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN30_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN30_DEFAULT << 30) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */
#define LPW0PORTAL_HOSTIEN_HOSTIEN31               (0x1UL << 31)                                 /**< Interrupt Enable Register                   */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN31_SHIFT        31                                            /**< Shift value for LPW0PORTAL_HOSTIEN31        */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN31_MASK         0x80000000UL                                  /**< Bit mask for LPW0PORTAL_HOSTIEN31           */
#define _LPW0PORTAL_HOSTIEN_HOSTIEN31_DEFAULT      0x00000000UL                                  /**< Mode DEFAULT for LPW0PORTAL_HOSTIEN         */
#define LPW0PORTAL_HOSTIEN_HOSTIEN31_DEFAULT       (_LPW0PORTAL_HOSTIEN_HOSTIEN31_DEFAULT << 31) /**< Shifted mode DEFAULT for LPW0PORTAL_HOSTIEN */

/* Bit fields for LPW0PORTAL MAILBOXREG0 */
#define _LPW0PORTAL_MAILBOXREG0_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG0    */
#define _LPW0PORTAL_MAILBOXREG0_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG0             */
#define _LPW0PORTAL_MAILBOXREG0_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG0_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG0_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG0     */
#define LPW0PORTAL_MAILBOXREG0_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG0_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG0*/

/* Bit fields for LPW0PORTAL MAILBOXREG1 */
#define _LPW0PORTAL_MAILBOXREG1_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG1    */
#define _LPW0PORTAL_MAILBOXREG1_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG1             */
#define _LPW0PORTAL_MAILBOXREG1_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG1_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG1_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG1     */
#define LPW0PORTAL_MAILBOXREG1_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG1_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG1*/

/* Bit fields for LPW0PORTAL MAILBOXREG2 */
#define _LPW0PORTAL_MAILBOXREG2_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG2    */
#define _LPW0PORTAL_MAILBOXREG2_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG2             */
#define _LPW0PORTAL_MAILBOXREG2_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG2_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG2_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG2     */
#define LPW0PORTAL_MAILBOXREG2_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG2_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG2*/

/* Bit fields for LPW0PORTAL MAILBOXREG3 */
#define _LPW0PORTAL_MAILBOXREG3_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG3    */
#define _LPW0PORTAL_MAILBOXREG3_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG3             */
#define _LPW0PORTAL_MAILBOXREG3_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG3_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG3_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG3     */
#define LPW0PORTAL_MAILBOXREG3_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG3_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG3*/

/* Bit fields for LPW0PORTAL MAILBOXREG4 */
#define _LPW0PORTAL_MAILBOXREG4_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG4    */
#define _LPW0PORTAL_MAILBOXREG4_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG4             */
#define _LPW0PORTAL_MAILBOXREG4_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG4_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG4_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG4     */
#define LPW0PORTAL_MAILBOXREG4_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG4_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG4*/

/* Bit fields for LPW0PORTAL MAILBOXREG5 */
#define _LPW0PORTAL_MAILBOXREG5_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG5    */
#define _LPW0PORTAL_MAILBOXREG5_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG5             */
#define _LPW0PORTAL_MAILBOXREG5_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG5_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG5_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG5     */
#define LPW0PORTAL_MAILBOXREG5_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG5_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG5*/

/* Bit fields for LPW0PORTAL MAILBOXREG6 */
#define _LPW0PORTAL_MAILBOXREG6_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG6    */
#define _LPW0PORTAL_MAILBOXREG6_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG6             */
#define _LPW0PORTAL_MAILBOXREG6_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG6_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG6_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG6     */
#define LPW0PORTAL_MAILBOXREG6_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG6_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG6*/

/* Bit fields for LPW0PORTAL MAILBOXREG7 */
#define _LPW0PORTAL_MAILBOXREG7_RESETVALUE         0x00000000UL                                   /**< Default value for LPW0PORTAL_MAILBOXREG7    */
#define _LPW0PORTAL_MAILBOXREG7_MASK               0xFFFFFFFFUL                                   /**< Mask for LPW0PORTAL_MAILBOXREG7             */
#define _LPW0PORTAL_MAILBOXREG7_MESSAGE_SHIFT      0                                              /**< Shift value for LPW0PORTAL_MESSAGE          */
#define _LPW0PORTAL_MAILBOXREG7_MESSAGE_MASK       0xFFFFFFFFUL                                   /**< Bit mask for LPW0PORTAL_MESSAGE             */
#define _LPW0PORTAL_MAILBOXREG7_MESSAGE_DEFAULT    0x00000000UL                                   /**< Mode DEFAULT for LPW0PORTAL_MAILBOXREG7     */
#define LPW0PORTAL_MAILBOXREG7_MESSAGE_DEFAULT     (_LPW0PORTAL_MAILBOXREG7_MESSAGE_DEFAULT << 0) /**< Shifted mode DEFAULT for LPW0PORTAL_MAILBOXREG7*/

/** @} End of group SIBG301_LPW0PORTAL_BitFields */
/** @} End of group SIBG301_LPW0PORTAL */
/** @} End of group Parts */

#endif // SIBG301_LPW0PORTAL_H
