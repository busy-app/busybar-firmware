/**************************************************************************//**
 * @file
 * @brief SIMG301 SOCPLL register and bit field definitions
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
#ifndef SIMG301_SOCPLL_H
#define SIMG301_SOCPLL_H
#define SOCPLL_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_SOCPLL SOCPLL
 * @{
 * @brief SIMG301 SOCPLL Register Declaration.
 *****************************************************************************/

/** SOCPLL Register Declaration. */
typedef struct socpll_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP VERSION Register                                */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  uint32_t       RESERVED0[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  __IOM uint32_t IF;                            /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN;                           /**< Interrupt Enable Register                          */
  __IOM uint32_t LOCK;                          /**< Configuration Lock Register                        */
  uint32_t       RESERVED1[1014U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP VERSION Register                                */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  uint32_t       RESERVED2[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable Register                          */
  __IOM uint32_t LOCK_SET;                      /**< Configuration Lock Register                        */
  uint32_t       RESERVED3[1014U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP VERSION Register                                */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  uint32_t       RESERVED4[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable Register                          */
  __IOM uint32_t LOCK_CLR;                      /**< Configuration Lock Register                        */
  uint32_t       RESERVED5[1014U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP VERSION Register                                */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  uint32_t       RESERVED6[4U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable Register                          */
  __IOM uint32_t LOCK_TGL;                      /**< Configuration Lock Register                        */
} SOCPLL_TypeDef;
/** @} End of group SIMG301_SOCPLL */

/**************************************************************************//**
 * @addtogroup SIMG301_SOCPLL
 * @{
 * @defgroup SIMG301_SOCPLL_BitFields SOCPLL Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SOCPLL IPVERSION */
#define _SOCPLL_IPVERSION_RESETVALUE           0x00000002UL                               /**< Default value for SOCPLL_IPVERSION          */
#define _SOCPLL_IPVERSION_MASK                 0xFFFFFFFFUL                               /**< Mask for SOCPLL_IPVERSION                   */
#define _SOCPLL_IPVERSION_IPVERSION_SHIFT      0                                          /**< Shift value for SOCPLL_IPVERSION            */
#define _SOCPLL_IPVERSION_IPVERSION_MASK       0xFFFFFFFFUL                               /**< Bit mask for SOCPLL_IPVERSION               */
#define _SOCPLL_IPVERSION_IPVERSION_DEFAULT    0x00000002UL                               /**< Mode DEFAULT for SOCPLL_IPVERSION           */
#define SOCPLL_IPVERSION_IPVERSION_DEFAULT     (_SOCPLL_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SOCPLL_IPVERSION   */

/* Bit fields for SOCPLL CTRL */
#define _SOCPLL_CTRL_RESETVALUE                0x92761604UL                                /**< Default value for SOCPLL_CTRL               */
#define _SOCPLL_CTRL_MASK                      0xFBFF1F3FUL                                /**< Mask for SOCPLL_CTRL                        */
#define SOCPLL_CTRL_FORCEEN                    (0x1UL << 0)                                /**< Force Enable                                */
#define _SOCPLL_CTRL_FORCEEN_SHIFT             0                                           /**< Shift value for SOCPLL_FORCEEN              */
#define _SOCPLL_CTRL_FORCEEN_MASK              0x1UL                                       /**< Bit mask for SOCPLL_FORCEEN                 */
#define _SOCPLL_CTRL_FORCEEN_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_FORCEEN_DEFAULT            (_SOCPLL_CTRL_FORCEEN_DEFAULT << 0)         /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define SOCPLL_CTRL_DISONDEMAND                (0x1UL << 1)                                /**< Disable on Demand                           */
#define _SOCPLL_CTRL_DISONDEMAND_SHIFT         1                                           /**< Shift value for SOCPLL_DISONDEMAND          */
#define _SOCPLL_CTRL_DISONDEMAND_MASK          0x2UL                                       /**< Bit mask for SOCPLL_DISONDEMAND             */
#define _SOCPLL_CTRL_DISONDEMAND_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_DISONDEMAND_DEFAULT        (_SOCPLL_CTRL_DISONDEMAND_DEFAULT << 1)     /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define SOCPLL_CTRL_ENFRACN                    (0x1UL << 2)                                /**< Frac-N Mode Enable                          */
#define _SOCPLL_CTRL_ENFRACN_SHIFT             2                                           /**< Shift value for SOCPLL_ENFRACN              */
#define _SOCPLL_CTRL_ENFRACN_MASK              0x4UL                                       /**< Bit mask for SOCPLL_ENFRACN                 */
#define _SOCPLL_CTRL_ENFRACN_DEFAULT           0x00000001UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_ENFRACN_DEFAULT            (_SOCPLL_CTRL_ENFRACN_DEFAULT << 2)         /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define SOCPLL_CTRL_ENOPENLOOP                 (0x1UL << 3)                                /**< Open loop mode enable                       */
#define _SOCPLL_CTRL_ENOPENLOOP_SHIFT          3                                           /**< Shift value for SOCPLL_ENOPENLOOP           */
#define _SOCPLL_CTRL_ENOPENLOOP_MASK           0x8UL                                       /**< Bit mask for SOCPLL_ENOPENLOOP              */
#define _SOCPLL_CTRL_ENOPENLOOP_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_ENOPENLOOP_DEFAULT         (_SOCPLL_CTRL_ENOPENLOOP_DEFAULT << 3)      /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define _SOCPLL_CTRL_REFCLKSEL_SHIFT           4                                           /**< Shift value for SOCPLL_REFCLKSEL            */
#define _SOCPLL_CTRL_REFCLKSEL_MASK            0x30UL                                      /**< Bit mask for SOCPLL_REFCLKSEL               */
#define _SOCPLL_CTRL_REFCLKSEL_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define _SOCPLL_CTRL_REFCLKSEL_REF_HFXO        0x00000000UL                                /**< Mode REF_HFXO for SOCPLL_CTRL               */
#define _SOCPLL_CTRL_REFCLKSEL_REF_HFRCO       0x00000001UL                                /**< Mode REF_HFRCO for SOCPLL_CTRL              */
#define _SOCPLL_CTRL_REFCLKSEL_REF_EXTCLK      0x00000002UL                                /**< Mode REF_EXTCLK for SOCPLL_CTRL             */
#define SOCPLL_CTRL_REFCLKSEL_DEFAULT          (_SOCPLL_CTRL_REFCLKSEL_DEFAULT << 4)       /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define SOCPLL_CTRL_REFCLKSEL_REF_HFXO         (_SOCPLL_CTRL_REFCLKSEL_REF_HFXO << 4)      /**< Shifted mode REF_HFXO for SOCPLL_CTRL       */
#define SOCPLL_CTRL_REFCLKSEL_REF_HFRCO        (_SOCPLL_CTRL_REFCLKSEL_REF_HFRCO << 4)     /**< Shifted mode REF_HFRCO for SOCPLL_CTRL      */
#define SOCPLL_CTRL_REFCLKSEL_REF_EXTCLK       (_SOCPLL_CTRL_REFCLKSEL_REF_EXTCLK << 4)    /**< Shifted mode REF_EXTCLK for SOCPLL_CTRL     */
#define _SOCPLL_CTRL_DIVN_SHIFT                8                                           /**< Shift value for SOCPLL_DIVN                 */
#define _SOCPLL_CTRL_DIVN_MASK                 0x1F00UL                                    /**< Bit mask for SOCPLL_DIVN                    */
#define _SOCPLL_CTRL_DIVN_DEFAULT              0x00000016UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_DIVN_DEFAULT               (_SOCPLL_CTRL_DIVN_DEFAULT << 8)            /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define _SOCPLL_CTRL_DIVF_SHIFT                16                                          /**< Shift value for SOCPLL_DIVF                 */
#define _SOCPLL_CTRL_DIVF_MASK                 0x3FF0000UL                                 /**< Bit mask for SOCPLL_DIVF                    */
#define _SOCPLL_CTRL_DIVF_DEFAULT              0x00000276UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_DIVF_DEFAULT               (_SOCPLL_CTRL_DIVF_DEFAULT << 16)           /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define SOCPLL_CTRL_ENSOCCLK0                  (0x1UL << 27)                               /**< Enable flash phy clock                      */
#define _SOCPLL_CTRL_ENSOCCLK0_SHIFT           27                                          /**< Shift value for SOCPLL_ENSOCCLK0            */
#define _SOCPLL_CTRL_ENSOCCLK0_MASK            0x8000000UL                                 /**< Bit mask for SOCPLL_ENSOCCLK0               */
#define _SOCPLL_CTRL_ENSOCCLK0_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define SOCPLL_CTRL_ENSOCCLK0_DEFAULT          (_SOCPLL_CTRL_ENSOCCLK0_DEFAULT << 27)      /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define _SOCPLL_CTRL_PROGRDY_SHIFT             30                                          /**< Shift value for SOCPLL_PROGRDY              */
#define _SOCPLL_CTRL_PROGRDY_MASK              0xC0000000UL                                /**< Bit mask for SOCPLL_PROGRDY                 */
#define _SOCPLL_CTRL_PROGRDY_DEFAULT           0x00000002UL                                /**< Mode DEFAULT for SOCPLL_CTRL                */
#define _SOCPLL_CTRL_PROGRDY_RDY_DELAY_0US     0x00000000UL                                /**< Mode RDY_DELAY_0US for SOCPLL_CTRL          */
#define _SOCPLL_CTRL_PROGRDY_RDY_DELAY_4US     0x00000001UL                                /**< Mode RDY_DELAY_4US for SOCPLL_CTRL          */
#define _SOCPLL_CTRL_PROGRDY_RDY_DELAY_8US     0x00000002UL                                /**< Mode RDY_DELAY_8US for SOCPLL_CTRL          */
#define _SOCPLL_CTRL_PROGRDY_RDY_DELAY_16US    0x00000003UL                                /**< Mode RDY_DELAY_16US for SOCPLL_CTRL         */
#define SOCPLL_CTRL_PROGRDY_DEFAULT            (_SOCPLL_CTRL_PROGRDY_DEFAULT << 30)        /**< Shifted mode DEFAULT for SOCPLL_CTRL        */
#define SOCPLL_CTRL_PROGRDY_RDY_DELAY_0US      (_SOCPLL_CTRL_PROGRDY_RDY_DELAY_0US << 30)  /**< Shifted mode RDY_DELAY_0US for SOCPLL_CTRL  */
#define SOCPLL_CTRL_PROGRDY_RDY_DELAY_4US      (_SOCPLL_CTRL_PROGRDY_RDY_DELAY_4US << 30)  /**< Shifted mode RDY_DELAY_4US for SOCPLL_CTRL  */
#define SOCPLL_CTRL_PROGRDY_RDY_DELAY_8US      (_SOCPLL_CTRL_PROGRDY_RDY_DELAY_8US << 30)  /**< Shifted mode RDY_DELAY_8US for SOCPLL_CTRL  */
#define SOCPLL_CTRL_PROGRDY_RDY_DELAY_16US     (_SOCPLL_CTRL_PROGRDY_RDY_DELAY_16US << 30) /**< Shifted mode RDY_DELAY_16US for SOCPLL_CTRL */

/* Bit fields for SOCPLL STATUS */
#define _SOCPLL_STATUS_RESETVALUE              0x00000000UL                             /**< Default value for SOCPLL_STATUS             */
#define _SOCPLL_STATUS_MASK                    0x800001FFUL                             /**< Mask for SOCPLL_STATUS                      */
#define SOCPLL_STATUS_RDY                      (0x1UL << 0)                             /**< SOCPLL Ready                                */
#define _SOCPLL_STATUS_RDY_SHIFT               0                                        /**< Shift value for SOCPLL_RDY                  */
#define _SOCPLL_STATUS_RDY_MASK                0x1UL                                    /**< Bit mask for SOCPLL_RDY                     */
#define _SOCPLL_STATUS_RDY_DEFAULT             0x00000000UL                             /**< Mode DEFAULT for SOCPLL_STATUS              */
#define SOCPLL_STATUS_RDY_DEFAULT              (_SOCPLL_STATUS_RDY_DEFAULT << 0)        /**< Shifted mode DEFAULT for SOCPLL_STATUS      */
#define SOCPLL_STATUS_PLLLOCK                  (0x1UL << 1)                             /**< Goes high within TDC range                  */
#define _SOCPLL_STATUS_PLLLOCK_SHIFT           1                                        /**< Shift value for SOCPLL_PLLLOCK              */
#define _SOCPLL_STATUS_PLLLOCK_MASK            0x2UL                                    /**< Bit mask for SOCPLL_PLLLOCK                 */
#define _SOCPLL_STATUS_PLLLOCK_DEFAULT         0x00000000UL                             /**< Mode DEFAULT for SOCPLL_STATUS              */
#define SOCPLL_STATUS_PLLLOCK_DEFAULT          (_SOCPLL_STATUS_PLLLOCK_DEFAULT << 1)    /**< Shifted mode DEFAULT for SOCPLL_STATUS      */
#define SOCPLL_STATUS_ENS                      (0x1UL << 8)                             /**< SOCPLL Enable                               */
#define _SOCPLL_STATUS_ENS_SHIFT               8                                        /**< Shift value for SOCPLL_ENS                  */
#define _SOCPLL_STATUS_ENS_MASK                0x100UL                                  /**< Bit mask for SOCPLL_ENS                     */
#define _SOCPLL_STATUS_ENS_DEFAULT             0x00000000UL                             /**< Mode DEFAULT for SOCPLL_STATUS              */
#define SOCPLL_STATUS_ENS_DEFAULT              (_SOCPLL_STATUS_ENS_DEFAULT << 8)        /**< Shifted mode DEFAULT for SOCPLL_STATUS      */
#define SOCPLL_STATUS_LOCK                     (0x1UL << 31)                            /**< Lock Status                                 */
#define _SOCPLL_STATUS_LOCK_SHIFT              31                                       /**< Shift value for SOCPLL_LOCK                 */
#define _SOCPLL_STATUS_LOCK_MASK               0x80000000UL                             /**< Bit mask for SOCPLL_LOCK                    */
#define _SOCPLL_STATUS_LOCK_DEFAULT            0x00000000UL                             /**< Mode DEFAULT for SOCPLL_STATUS              */
#define _SOCPLL_STATUS_LOCK_UNLOCKED           0x00000000UL                             /**< Mode UNLOCKED for SOCPLL_STATUS             */
#define _SOCPLL_STATUS_LOCK_LOCKED             0x00000001UL                             /**< Mode LOCKED for SOCPLL_STATUS               */
#define SOCPLL_STATUS_LOCK_DEFAULT             (_SOCPLL_STATUS_LOCK_DEFAULT << 31)      /**< Shifted mode DEFAULT for SOCPLL_STATUS      */
#define SOCPLL_STATUS_LOCK_UNLOCKED            (_SOCPLL_STATUS_LOCK_UNLOCKED << 31)     /**< Shifted mode UNLOCKED for SOCPLL_STATUS     */
#define SOCPLL_STATUS_LOCK_LOCKED              (_SOCPLL_STATUS_LOCK_LOCKED << 31)       /**< Shifted mode LOCKED for SOCPLL_STATUS       */

/* Bit fields for SOCPLL IF */
#define _SOCPLL_IF_RESETVALUE                  0x00000000UL                             /**< Default value for SOCPLL_IF                 */
#define _SOCPLL_IF_MASK                        0x00000003UL                             /**< Mask for SOCPLL_IF                          */
#define SOCPLL_IF_RDY                          (0x1UL << 0)                             /**< Ready Interrupt Flag                        */
#define _SOCPLL_IF_RDY_SHIFT                   0                                        /**< Shift value for SOCPLL_RDY                  */
#define _SOCPLL_IF_RDY_MASK                    0x1UL                                    /**< Bit mask for SOCPLL_RDY                     */
#define _SOCPLL_IF_RDY_DEFAULT                 0x00000000UL                             /**< Mode DEFAULT for SOCPLL_IF                  */
#define SOCPLL_IF_RDY_DEFAULT                  (_SOCPLL_IF_RDY_DEFAULT << 0)            /**< Shifted mode DEFAULT for SOCPLL_IF          */
#define SOCPLL_IF_LOSSOFLOCK                   (0x1UL << 1)                             /**< Loss of PLL Lock Flag                       */
#define _SOCPLL_IF_LOSSOFLOCK_SHIFT            1                                        /**< Shift value for SOCPLL_LOSSOFLOCK           */
#define _SOCPLL_IF_LOSSOFLOCK_MASK             0x2UL                                    /**< Bit mask for SOCPLL_LOSSOFLOCK              */
#define _SOCPLL_IF_LOSSOFLOCK_DEFAULT          0x00000000UL                             /**< Mode DEFAULT for SOCPLL_IF                  */
#define SOCPLL_IF_LOSSOFLOCK_DEFAULT           (_SOCPLL_IF_LOSSOFLOCK_DEFAULT << 1)     /**< Shifted mode DEFAULT for SOCPLL_IF          */

/* Bit fields for SOCPLL IEN */
#define _SOCPLL_IEN_RESETVALUE                 0x00000000UL                             /**< Default value for SOCPLL_IEN                */
#define _SOCPLL_IEN_MASK                       0x00000003UL                             /**< Mask for SOCPLL_IEN                         */
#define SOCPLL_IEN_RDY                         (0x1UL << 0)                             /**< RDY Interrupt Enable                        */
#define _SOCPLL_IEN_RDY_SHIFT                  0                                        /**< Shift value for SOCPLL_RDY                  */
#define _SOCPLL_IEN_RDY_MASK                   0x1UL                                    /**< Bit mask for SOCPLL_RDY                     */
#define _SOCPLL_IEN_RDY_DEFAULT                0x00000000UL                             /**< Mode DEFAULT for SOCPLL_IEN                 */
#define SOCPLL_IEN_RDY_DEFAULT                 (_SOCPLL_IEN_RDY_DEFAULT << 0)           /**< Shifted mode DEFAULT for SOCPLL_IEN         */
#define SOCPLL_IEN_LOSSOFLOCK                  (0x1UL << 1)                             /**< LOSSOFLOCK Interrupt Enable                 */
#define _SOCPLL_IEN_LOSSOFLOCK_SHIFT           1                                        /**< Shift value for SOCPLL_LOSSOFLOCK           */
#define _SOCPLL_IEN_LOSSOFLOCK_MASK            0x2UL                                    /**< Bit mask for SOCPLL_LOSSOFLOCK              */
#define _SOCPLL_IEN_LOSSOFLOCK_DEFAULT         0x00000000UL                             /**< Mode DEFAULT for SOCPLL_IEN                 */
#define SOCPLL_IEN_LOSSOFLOCK_DEFAULT          (_SOCPLL_IEN_LOSSOFLOCK_DEFAULT << 1)    /**< Shifted mode DEFAULT for SOCPLL_IEN         */

/* Bit fields for SOCPLL LOCK */
#define _SOCPLL_LOCK_RESETVALUE                0x000081A6UL                             /**< Default value for SOCPLL_LOCK               */
#define _SOCPLL_LOCK_MASK                      0x0000FFFFUL                             /**< Mask for SOCPLL_LOCK                        */
#define _SOCPLL_LOCK_LOCKKEY_SHIFT             0                                        /**< Shift value for SOCPLL_LOCKKEY              */
#define _SOCPLL_LOCK_LOCKKEY_MASK              0xFFFFUL                                 /**< Bit mask for SOCPLL_LOCKKEY                 */
#define _SOCPLL_LOCK_LOCKKEY_DEFAULT           0x000081A6UL                             /**< Mode DEFAULT for SOCPLL_LOCK                */
#define _SOCPLL_LOCK_LOCKKEY_UNLOCK            0x000081A6UL                             /**< Mode UNLOCK for SOCPLL_LOCK                 */
#define SOCPLL_LOCK_LOCKKEY_DEFAULT            (_SOCPLL_LOCK_LOCKKEY_DEFAULT << 0)      /**< Shifted mode DEFAULT for SOCPLL_LOCK        */
#define SOCPLL_LOCK_LOCKKEY_UNLOCK             (_SOCPLL_LOCK_LOCKKEY_UNLOCK << 0)       /**< Shifted mode UNLOCK for SOCPLL_LOCK         */

/** @} End of group SIMG301_SOCPLL_BitFields */
/** @} End of group SIMG301_SOCPLL */
/** @} End of group Parts */

#endif // SIMG301_SOCPLL_H
