/**************************************************************************//**
 * @file
 * @brief SIMG301 KSU register and bit field definitions
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
#ifndef SIMG301_KSU_H
#define SIMG301_KSU_H
#define KSU_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_KSU KSU
 * @{
 * @brief SIMG301 KSU Register Declaration.
 *****************************************************************************/

/** KSU Register Declaration. */
typedef struct ksu_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP Version ID                                      */
  __IOM uint32_t EN;                            /**< Module enable                                      */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  __IOM uint32_t IRKCMD;                        /**< IRK command Register                               */
  __IOM uint32_t CRYPTO0CMD;                    /**< Crypto0 command register                           */
  __IOM uint32_t CRYPTO1CMD;                    /**< Crypto1 command register                           */
  __IOM uint32_t INFO;                          /**< Info handshake bit                                 */
  __IM uint32_t  STATUS;                        /**< Status register                                    */
  __IOM uint32_t IRKIEN;                        /**< IRK Interrupt Enable                               */
  __IOM uint32_t IRKIF;                         /**< IRK Interrupt Flag                                 */
  __IOM uint32_t CRYPTO0IEN;                    /**< Crypto0 Interrupt Enable                           */
  __IOM uint32_t CRYPTO0IF;                     /**< Crypto0 Interrupt Flag                             */
  __IOM uint32_t CRYPTO1IEN;                    /**< Crypto1 Interrupt Enable                           */
  __IOM uint32_t CRYPTO1IF;                     /**< Crypto1 Interrupt Flag                             */
  __IM uint32_t  IRKECCERRKEYSLOT;              /**< IRK error slot                                     */
  __IM uint32_t  CRYPTO0ECCERRKEYSLOT;          /**< Crypto0 error slot                                 */
  __IM uint32_t  CRYPTO1ECCERRKEYSLOT;          /**< Crypto1 error slot                                 */
  __IOM uint32_t CORERESET;                     /**< User reset                                         */
  __IOM uint32_t MISC;                          /**< Miscellaneous registers                            */
  uint32_t       RESERVED0[13U];                /**< Reserved for future use                            */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED2[991U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP Version ID                                      */
  __IOM uint32_t EN_SET;                        /**< Module enable                                      */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  __IOM uint32_t IRKCMD_SET;                    /**< IRK command Register                               */
  __IOM uint32_t CRYPTO0CMD_SET;                /**< Crypto0 command register                           */
  __IOM uint32_t CRYPTO1CMD_SET;                /**< Crypto1 command register                           */
  __IOM uint32_t INFO_SET;                      /**< Info handshake bit                                 */
  __IM uint32_t  STATUS_SET;                    /**< Status register                                    */
  __IOM uint32_t IRKIEN_SET;                    /**< IRK Interrupt Enable                               */
  __IOM uint32_t IRKIF_SET;                     /**< IRK Interrupt Flag                                 */
  __IOM uint32_t CRYPTO0IEN_SET;                /**< Crypto0 Interrupt Enable                           */
  __IOM uint32_t CRYPTO0IF_SET;                 /**< Crypto0 Interrupt Flag                             */
  __IOM uint32_t CRYPTO1IEN_SET;                /**< Crypto1 Interrupt Enable                           */
  __IOM uint32_t CRYPTO1IF_SET;                 /**< Crypto1 Interrupt Flag                             */
  __IM uint32_t  IRKECCERRKEYSLOT_SET;          /**< IRK error slot                                     */
  __IM uint32_t  CRYPTO0ECCERRKEYSLOT_SET;      /**< Crypto0 error slot                                 */
  __IM uint32_t  CRYPTO1ECCERRKEYSLOT_SET;      /**< Crypto1 error slot                                 */
  __IOM uint32_t CORERESET_SET;                 /**< User reset                                         */
  __IOM uint32_t MISC_SET;                      /**< Miscellaneous registers                            */
  uint32_t       RESERVED3[13U];                /**< Reserved for future use                            */
  uint32_t       RESERVED4[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED5[991U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP Version ID                                      */
  __IOM uint32_t EN_CLR;                        /**< Module enable                                      */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  __IOM uint32_t IRKCMD_CLR;                    /**< IRK command Register                               */
  __IOM uint32_t CRYPTO0CMD_CLR;                /**< Crypto0 command register                           */
  __IOM uint32_t CRYPTO1CMD_CLR;                /**< Crypto1 command register                           */
  __IOM uint32_t INFO_CLR;                      /**< Info handshake bit                                 */
  __IM uint32_t  STATUS_CLR;                    /**< Status register                                    */
  __IOM uint32_t IRKIEN_CLR;                    /**< IRK Interrupt Enable                               */
  __IOM uint32_t IRKIF_CLR;                     /**< IRK Interrupt Flag                                 */
  __IOM uint32_t CRYPTO0IEN_CLR;                /**< Crypto0 Interrupt Enable                           */
  __IOM uint32_t CRYPTO0IF_CLR;                 /**< Crypto0 Interrupt Flag                             */
  __IOM uint32_t CRYPTO1IEN_CLR;                /**< Crypto1 Interrupt Enable                           */
  __IOM uint32_t CRYPTO1IF_CLR;                 /**< Crypto1 Interrupt Flag                             */
  __IM uint32_t  IRKECCERRKEYSLOT_CLR;          /**< IRK error slot                                     */
  __IM uint32_t  CRYPTO0ECCERRKEYSLOT_CLR;      /**< Crypto0 error slot                                 */
  __IM uint32_t  CRYPTO1ECCERRKEYSLOT_CLR;      /**< Crypto1 error slot                                 */
  __IOM uint32_t CORERESET_CLR;                 /**< User reset                                         */
  __IOM uint32_t MISC_CLR;                      /**< Miscellaneous registers                            */
  uint32_t       RESERVED6[13U];                /**< Reserved for future use                            */
  uint32_t       RESERVED7[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED8[991U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP Version ID                                      */
  __IOM uint32_t EN_TGL;                        /**< Module enable                                      */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  __IOM uint32_t IRKCMD_TGL;                    /**< IRK command Register                               */
  __IOM uint32_t CRYPTO0CMD_TGL;                /**< Crypto0 command register                           */
  __IOM uint32_t CRYPTO1CMD_TGL;                /**< Crypto1 command register                           */
  __IOM uint32_t INFO_TGL;                      /**< Info handshake bit                                 */
  __IM uint32_t  STATUS_TGL;                    /**< Status register                                    */
  __IOM uint32_t IRKIEN_TGL;                    /**< IRK Interrupt Enable                               */
  __IOM uint32_t IRKIF_TGL;                     /**< IRK Interrupt Flag                                 */
  __IOM uint32_t CRYPTO0IEN_TGL;                /**< Crypto0 Interrupt Enable                           */
  __IOM uint32_t CRYPTO0IF_TGL;                 /**< Crypto0 Interrupt Flag                             */
  __IOM uint32_t CRYPTO1IEN_TGL;                /**< Crypto1 Interrupt Enable                           */
  __IOM uint32_t CRYPTO1IF_TGL;                 /**< Crypto1 Interrupt Flag                             */
  __IM uint32_t  IRKECCERRKEYSLOT_TGL;          /**< IRK error slot                                     */
  __IM uint32_t  CRYPTO0ECCERRKEYSLOT_TGL;      /**< Crypto0 error slot                                 */
  __IM uint32_t  CRYPTO1ECCERRKEYSLOT_TGL;      /**< Crypto1 error slot                                 */
  __IOM uint32_t CORERESET_TGL;                 /**< User reset                                         */
  __IOM uint32_t MISC_TGL;                      /**< Miscellaneous registers                            */
  uint32_t       RESERVED9[13U];                /**< Reserved for future use                            */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
} KSU_TypeDef;
/** @} End of group SIMG301_KSU */

/**************************************************************************//**
 * @addtogroup SIMG301_KSU
 * @{
 * @defgroup SIMG301_KSU_BitFields KSU Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for KSU IPVERSION */
#define _KSU_IPVERSION_RESETVALUE                                 0x00000001UL                            /**< Default value for KSU_IPVERSION             */
#define _KSU_IPVERSION_MASK                                       0xFFFFFFFFUL                            /**< Mask for KSU_IPVERSION                      */
#define _KSU_IPVERSION_IPVERSION_SHIFT                            0                                       /**< Shift value for KSU_IPVERSION               */
#define _KSU_IPVERSION_IPVERSION_MASK                             0xFFFFFFFFUL                            /**< Bit mask for KSU_IPVERSION                  */
#define _KSU_IPVERSION_IPVERSION_DEFAULT                          0x00000001UL                            /**< Mode DEFAULT for KSU_IPVERSION              */
#define KSU_IPVERSION_IPVERSION_DEFAULT                           (_KSU_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_IPVERSION      */

/* Bit fields for KSU EN */
#define _KSU_EN_RESETVALUE                                        0x00000000UL                     /**< Default value for KSU_EN                    */
#define _KSU_EN_MASK                                              0x00000003UL                     /**< Mask for KSU_EN                             */
#define KSU_EN_EN                                                 (0x1UL << 0)                     /**< KSU Enable                                  */
#define _KSU_EN_EN_SHIFT                                          0                                /**< Shift value for KSU_EN                      */
#define _KSU_EN_EN_MASK                                           0x1UL                            /**< Bit mask for KSU_EN                         */
#define _KSU_EN_EN_DEFAULT                                        0x00000000UL                     /**< Mode DEFAULT for KSU_EN                     */
#define KSU_EN_EN_DEFAULT                                         (_KSU_EN_EN_DEFAULT << 0)        /**< Shifted mode DEFAULT for KSU_EN             */
#define KSU_EN_DISABLING                                          (0x1UL << 1)                     /**< KSU Disable                                 */
#define _KSU_EN_DISABLING_SHIFT                                   1                                /**< Shift value for KSU_DISABLING               */
#define _KSU_EN_DISABLING_MASK                                    0x2UL                            /**< Bit mask for KSU_DISABLING                  */
#define _KSU_EN_DISABLING_DEFAULT                                 0x00000000UL                     /**< Mode DEFAULT for KSU_EN                     */
#define KSU_EN_DISABLING_DEFAULT                                  (_KSU_EN_DISABLING_DEFAULT << 1) /**< Shifted mode DEFAULT for KSU_EN             */

/* Bit fields for KSU CTRL */
#define _KSU_CTRL_RESETVALUE                                      0x00000000UL                      /**< Default value for KSU_CTRL                  */
#define _KSU_CTRL_MASK                                            0x00000001UL                      /**< Mask for KSU_CTRL                           */
#define KSU_CTRL_RETNCTRL                                         (0x1UL << 0)                      /**< enable of retention control                 */
#define _KSU_CTRL_RETNCTRL_SHIFT                                  0                                 /**< Shift value for KSU_RETNCTRL                */
#define _KSU_CTRL_RETNCTRL_MASK                                   0x1UL                             /**< Bit mask for KSU_RETNCTRL                   */
#define _KSU_CTRL_RETNCTRL_DEFAULT                                0x00000000UL                      /**< Mode DEFAULT for KSU_CTRL                   */
#define KSU_CTRL_RETNCTRL_DEFAULT                                 (_KSU_CTRL_RETNCTRL_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CTRL           */

/* Bit fields for KSU IRKCMD */
#define _KSU_IRKCMD_RESETVALUE                                    0x00000000UL                        /**< Default value for KSU_IRKCMD                */
#define _KSU_IRKCMD_MASK                                          0x00000001UL                        /**< Mask for KSU_IRKCMD                         */
#define KSU_IRKCMD_CLEARECC                                       (0x1UL << 0)                        /**< clear the error slots of DKP0               */
#define _KSU_IRKCMD_CLEARECC_SHIFT                                0                                   /**< Shift value for KSU_CLEARECC                */
#define _KSU_IRKCMD_CLEARECC_MASK                                 0x1UL                               /**< Bit mask for KSU_CLEARECC                   */
#define _KSU_IRKCMD_CLEARECC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for KSU_IRKCMD                 */
#define KSU_IRKCMD_CLEARECC_DEFAULT                               (_KSU_IRKCMD_CLEARECC_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_IRKCMD         */

/* Bit fields for KSU CRYPTO0CMD */
#define _KSU_CRYPTO0CMD_RESETVALUE                                0x00000000UL                            /**< Default value for KSU_CRYPTO0CMD            */
#define _KSU_CRYPTO0CMD_MASK                                      0x00000001UL                            /**< Mask for KSU_CRYPTO0CMD                     */
#define KSU_CRYPTO0CMD_CLEARECC                                   (0x1UL << 0)                            /**< clears the error slots for DKP0             */
#define _KSU_CRYPTO0CMD_CLEARECC_SHIFT                            0                                       /**< Shift value for KSU_CLEARECC                */
#define _KSU_CRYPTO0CMD_CLEARECC_MASK                             0x1UL                                   /**< Bit mask for KSU_CLEARECC                   */
#define _KSU_CRYPTO0CMD_CLEARECC_DEFAULT                          0x00000000UL                            /**< Mode DEFAULT for KSU_CRYPTO0CMD             */
#define KSU_CRYPTO0CMD_CLEARECC_DEFAULT                           (_KSU_CRYPTO0CMD_CLEARECC_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO0CMD     */

/* Bit fields for KSU CRYPTO1CMD */
#define _KSU_CRYPTO1CMD_RESETVALUE                                0x00000000UL                            /**< Default value for KSU_CRYPTO1CMD            */
#define _KSU_CRYPTO1CMD_MASK                                      0x00000001UL                            /**< Mask for KSU_CRYPTO1CMD                     */
#define KSU_CRYPTO1CMD_CLEARECC                                   (0x1UL << 0)                            /**< clears the error slot of DKP1               */
#define _KSU_CRYPTO1CMD_CLEARECC_SHIFT                            0                                       /**< Shift value for KSU_CLEARECC                */
#define _KSU_CRYPTO1CMD_CLEARECC_MASK                             0x1UL                                   /**< Bit mask for KSU_CLEARECC                   */
#define _KSU_CRYPTO1CMD_CLEARECC_DEFAULT                          0x00000000UL                            /**< Mode DEFAULT for KSU_CRYPTO1CMD             */
#define KSU_CRYPTO1CMD_CLEARECC_DEFAULT                           (_KSU_CRYPTO1CMD_CLEARECC_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO1CMD     */

/* Bit fields for KSU INFO */
#define _KSU_INFO_RESETVALUE                                      0x00000000UL                       /**< Default value for KSU_INFO                  */
#define _KSU_INFO_MASK                                            0x00000001UL                       /**< Mask for KSU_INFO                           */
#define KSU_INFO_SETUPDONE                                        (0x1UL << 0)                       /**< Handshake bit for SE and USER               */
#define _KSU_INFO_SETUPDONE_SHIFT                                 0                                  /**< Shift value for KSU_SETUPDONE               */
#define _KSU_INFO_SETUPDONE_MASK                                  0x1UL                              /**< Bit mask for KSU_SETUPDONE                  */
#define _KSU_INFO_SETUPDONE_DEFAULT                               0x00000000UL                       /**< Mode DEFAULT for KSU_INFO                   */
#define KSU_INFO_SETUPDONE_DEFAULT                                (_KSU_INFO_SETUPDONE_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_INFO           */

/* Bit fields for KSU STATUS */
#define _KSU_STATUS_RESETVALUE                                    0x00000000UL                             /**< Default value for KSU_STATUS                */
#define _KSU_STATUS_MASK                                          0x00000001UL                             /**< Mask for KSU_STATUS                         */
#define KSU_STATUS_KEYREQLOCKACK                                  (0x1UL << 0)                             /**< Key request stall                           */
#define _KSU_STATUS_KEYREQLOCKACK_SHIFT                           0                                        /**< Shift value for KSU_KEYREQLOCKACK           */
#define _KSU_STATUS_KEYREQLOCKACK_MASK                            0x1UL                                    /**< Bit mask for KSU_KEYREQLOCKACK              */
#define _KSU_STATUS_KEYREQLOCKACK_DEFAULT                         0x00000000UL                             /**< Mode DEFAULT for KSU_STATUS                 */
#define KSU_STATUS_KEYREQLOCKACK_DEFAULT                          (_KSU_STATUS_KEYREQLOCKACK_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_STATUS         */

/* Bit fields for KSU IRKIEN */
#define _KSU_IRKIEN_RESETVALUE                                    0x00000000UL                                     /**< Default value for KSU_IRKIEN                */
#define _KSU_IRKIEN_MASK                                          0x0000001FUL                                     /**< Mask for KSU_IRKIEN                         */
#define KSU_IRKIEN_IRKINVALIDCRYPTOIDIEN                          (0x1UL << 0)                                     /**< rpa engineid err                            */
#define _KSU_IRKIEN_IRKINVALIDCRYPTOIDIEN_SHIFT                   0                                                /**< Shift value for KSU_IRKINVALIDCRYPTOIDIEN   */
#define _KSU_IRKIEN_IRKINVALIDCRYPTOIDIEN_MASK                    0x1UL                                            /**< Bit mask for KSU_IRKINVALIDCRYPTOIDIEN      */
#define _KSU_IRKIEN_IRKINVALIDCRYPTOIDIEN_DEFAULT                 0x00000000UL                                     /**< Mode DEFAULT for KSU_IRKIEN                 */
#define KSU_IRKIEN_IRKINVALIDCRYPTOIDIEN_DEFAULT                  (_KSU_IRKIEN_IRKINVALIDCRYPTOIDIEN_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_IRKIEN         */
#define KSU_IRKIEN_IRKINVALIDKEYSLOTIEN                           (0x1UL << 1)                                     /**< rpa slotid err                              */
#define _KSU_IRKIEN_IRKINVALIDKEYSLOTIEN_SHIFT                    1                                                /**< Shift value for KSU_IRKINVALIDKEYSLOTIEN    */
#define _KSU_IRKIEN_IRKINVALIDKEYSLOTIEN_MASK                     0x2UL                                            /**< Bit mask for KSU_IRKINVALIDKEYSLOTIEN       */
#define _KSU_IRKIEN_IRKINVALIDKEYSLOTIEN_DEFAULT                  0x00000000UL                                     /**< Mode DEFAULT for KSU_IRKIEN                 */
#define KSU_IRKIEN_IRKINVALIDKEYSLOTIEN_DEFAULT                   (_KSU_IRKIEN_IRKINVALIDKEYSLOTIEN_DEFAULT << 1)  /**< Shifted mode DEFAULT for KSU_IRKIEN         */
#define KSU_IRKIEN_IRKKEYSIZEERRIEN                               (0x1UL << 2)                                     /**< rpa keysize err                             */
#define _KSU_IRKIEN_IRKKEYSIZEERRIEN_SHIFT                        2                                                /**< Shift value for KSU_IRKKEYSIZEERRIEN        */
#define _KSU_IRKIEN_IRKKEYSIZEERRIEN_MASK                         0x4UL                                            /**< Bit mask for KSU_IRKKEYSIZEERRIEN           */
#define _KSU_IRKIEN_IRKKEYSIZEERRIEN_DEFAULT                      0x00000000UL                                     /**< Mode DEFAULT for KSU_IRKIEN                 */
#define KSU_IRKIEN_IRKKEYSIZEERRIEN_DEFAULT                       (_KSU_IRKIEN_IRKKEYSIZEERRIEN_DEFAULT << 2)      /**< Shifted mode DEFAULT for KSU_IRKIEN         */
#define KSU_IRKIEN_IRKECC1BERRIEN                                 (0x1UL << 3)                                     /**< ECC 1b error for IRK interface              */
#define _KSU_IRKIEN_IRKECC1BERRIEN_SHIFT                          3                                                /**< Shift value for KSU_IRKECC1BERRIEN          */
#define _KSU_IRKIEN_IRKECC1BERRIEN_MASK                           0x8UL                                            /**< Bit mask for KSU_IRKECC1BERRIEN             */
#define _KSU_IRKIEN_IRKECC1BERRIEN_DEFAULT                        0x00000000UL                                     /**< Mode DEFAULT for KSU_IRKIEN                 */
#define KSU_IRKIEN_IRKECC1BERRIEN_DEFAULT                         (_KSU_IRKIEN_IRKECC1BERRIEN_DEFAULT << 3)        /**< Shifted mode DEFAULT for KSU_IRKIEN         */
#define KSU_IRKIEN_IRKECC2BERRIEN                                 (0x1UL << 4)                                     /**< ECC 2b error for IRK interface              */
#define _KSU_IRKIEN_IRKECC2BERRIEN_SHIFT                          4                                                /**< Shift value for KSU_IRKECC2BERRIEN          */
#define _KSU_IRKIEN_IRKECC2BERRIEN_MASK                           0x10UL                                           /**< Bit mask for KSU_IRKECC2BERRIEN             */
#define _KSU_IRKIEN_IRKECC2BERRIEN_DEFAULT                        0x00000000UL                                     /**< Mode DEFAULT for KSU_IRKIEN                 */
#define KSU_IRKIEN_IRKECC2BERRIEN_DEFAULT                         (_KSU_IRKIEN_IRKECC2BERRIEN_DEFAULT << 4)        /**< Shifted mode DEFAULT for KSU_IRKIEN         */

/* Bit fields for KSU IRKIF */
#define _KSU_IRKIF_RESETVALUE                                     0x00000000UL                                   /**< Default value for KSU_IRKIF                 */
#define _KSU_IRKIF_MASK                                           0x0000001FUL                                   /**< Mask for KSU_IRKIF                          */
#define KSU_IRKIF_IRKINVALIDCRYPTOIDIF                            (0x1UL << 0)                                   /**< rpa engineid err flag                       */
#define _KSU_IRKIF_IRKINVALIDCRYPTOIDIF_SHIFT                     0                                              /**< Shift value for KSU_IRKINVALIDCRYPTOIDIF    */
#define _KSU_IRKIF_IRKINVALIDCRYPTOIDIF_MASK                      0x1UL                                          /**< Bit mask for KSU_IRKINVALIDCRYPTOIDIF       */
#define _KSU_IRKIF_IRKINVALIDCRYPTOIDIF_DEFAULT                   0x00000000UL                                   /**< Mode DEFAULT for KSU_IRKIF                  */
#define KSU_IRKIF_IRKINVALIDCRYPTOIDIF_DEFAULT                    (_KSU_IRKIF_IRKINVALIDCRYPTOIDIF_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_IRKIF          */
#define KSU_IRKIF_IRKINVALIDKEYSLOTIF                             (0x1UL << 1)                                   /**< rpa slotid err flag                         */
#define _KSU_IRKIF_IRKINVALIDKEYSLOTIF_SHIFT                      1                                              /**< Shift value for KSU_IRKINVALIDKEYSLOTIF     */
#define _KSU_IRKIF_IRKINVALIDKEYSLOTIF_MASK                       0x2UL                                          /**< Bit mask for KSU_IRKINVALIDKEYSLOTIF        */
#define _KSU_IRKIF_IRKINVALIDKEYSLOTIF_DEFAULT                    0x00000000UL                                   /**< Mode DEFAULT for KSU_IRKIF                  */
#define KSU_IRKIF_IRKINVALIDKEYSLOTIF_DEFAULT                     (_KSU_IRKIF_IRKINVALIDKEYSLOTIF_DEFAULT << 1)  /**< Shifted mode DEFAULT for KSU_IRKIF          */
#define KSU_IRKIF_IRKKEYSIZEERRIF                                 (0x1UL << 2)                                   /**< rpa keysize err flag                        */
#define _KSU_IRKIF_IRKKEYSIZEERRIF_SHIFT                          2                                              /**< Shift value for KSU_IRKKEYSIZEERRIF         */
#define _KSU_IRKIF_IRKKEYSIZEERRIF_MASK                           0x4UL                                          /**< Bit mask for KSU_IRKKEYSIZEERRIF            */
#define _KSU_IRKIF_IRKKEYSIZEERRIF_DEFAULT                        0x00000000UL                                   /**< Mode DEFAULT for KSU_IRKIF                  */
#define KSU_IRKIF_IRKKEYSIZEERRIF_DEFAULT                         (_KSU_IRKIF_IRKKEYSIZEERRIF_DEFAULT << 2)      /**< Shifted mode DEFAULT for KSU_IRKIF          */
#define KSU_IRKIF_IRKECC1BERRIF                                   (0x1UL << 3)                                   /**< Ecc error 1b IRK intf                       */
#define _KSU_IRKIF_IRKECC1BERRIF_SHIFT                            3                                              /**< Shift value for KSU_IRKECC1BERRIF           */
#define _KSU_IRKIF_IRKECC1BERRIF_MASK                             0x8UL                                          /**< Bit mask for KSU_IRKECC1BERRIF              */
#define _KSU_IRKIF_IRKECC1BERRIF_DEFAULT                          0x00000000UL                                   /**< Mode DEFAULT for KSU_IRKIF                  */
#define KSU_IRKIF_IRKECC1BERRIF_DEFAULT                           (_KSU_IRKIF_IRKECC1BERRIF_DEFAULT << 3)        /**< Shifted mode DEFAULT for KSU_IRKIF          */
#define KSU_IRKIF_IRKECC2BERRIF                                   (0x1UL << 4)                                   /**< ECC error 2b IRK intf                       */
#define _KSU_IRKIF_IRKECC2BERRIF_SHIFT                            4                                              /**< Shift value for KSU_IRKECC2BERRIF           */
#define _KSU_IRKIF_IRKECC2BERRIF_MASK                             0x10UL                                         /**< Bit mask for KSU_IRKECC2BERRIF              */
#define _KSU_IRKIF_IRKECC2BERRIF_DEFAULT                          0x00000000UL                                   /**< Mode DEFAULT for KSU_IRKIF                  */
#define KSU_IRKIF_IRKECC2BERRIF_DEFAULT                           (_KSU_IRKIF_IRKECC2BERRIF_DEFAULT << 4)        /**< Shifted mode DEFAULT for KSU_IRKIF          */

/* Bit fields for KSU CRYPTO0IEN */
#define _KSU_CRYPTO0IEN_RESETVALUE                                0x00000000UL                                             /**< Default value for KSU_CRYPTO0IEN            */
#define _KSU_CRYPTO0IEN_MASK                                      0x0000000FUL                                             /**< Mask for KSU_CRYPTO0IEN                     */
#define KSU_CRYPTO0IEN_CRYPTO0INVALIDCRYPTOIDIEN                  (0x1UL << 0)                                             /**< DKP_IF0 engine id error                     */
#define _KSU_CRYPTO0IEN_CRYPTO0INVALIDCRYPTOIDIEN_SHIFT           0                                                        /**< Shift value for KSU_CRYPTO0INVALIDCRYPTOIDIEN*/
#define _KSU_CRYPTO0IEN_CRYPTO0INVALIDCRYPTOIDIEN_MASK            0x1UL                                                    /**< Bit mask for KSU_CRYPTO0INVALIDCRYPTOIDIEN  */
#define _KSU_CRYPTO0IEN_CRYPTO0INVALIDCRYPTOIDIEN_DEFAULT         0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO0IEN             */
#define KSU_CRYPTO0IEN_CRYPTO0INVALIDCRYPTOIDIEN_DEFAULT          (_KSU_CRYPTO0IEN_CRYPTO0INVALIDCRYPTOIDIEN_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO0IEN     */
#define KSU_CRYPTO0IEN_CRYPTO0INVALIDKEYSLOTIEN                   (0x1UL << 1)                                             /**< DKP_IF0 slot id error                       */
#define _KSU_CRYPTO0IEN_CRYPTO0INVALIDKEYSLOTIEN_SHIFT            1                                                        /**< Shift value for KSU_CRYPTO0INVALIDKEYSLOTIEN*/
#define _KSU_CRYPTO0IEN_CRYPTO0INVALIDKEYSLOTIEN_MASK             0x2UL                                                    /**< Bit mask for KSU_CRYPTO0INVALIDKEYSLOTIEN   */
#define _KSU_CRYPTO0IEN_CRYPTO0INVALIDKEYSLOTIEN_DEFAULT          0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO0IEN             */
#define KSU_CRYPTO0IEN_CRYPTO0INVALIDKEYSLOTIEN_DEFAULT           (_KSU_CRYPTO0IEN_CRYPTO0INVALIDKEYSLOTIEN_DEFAULT << 1)  /**< Shifted mode DEFAULT for KSU_CRYPTO0IEN     */
#define KSU_CRYPTO0IEN_CRYPTO0ECC1BERRIEN                         (0x1UL << 2)                                             /**< ECC 1b error crypto int 0                   */
#define _KSU_CRYPTO0IEN_CRYPTO0ECC1BERRIEN_SHIFT                  2                                                        /**< Shift value for KSU_CRYPTO0ECC1BERRIEN      */
#define _KSU_CRYPTO0IEN_CRYPTO0ECC1BERRIEN_MASK                   0x4UL                                                    /**< Bit mask for KSU_CRYPTO0ECC1BERRIEN         */
#define _KSU_CRYPTO0IEN_CRYPTO0ECC1BERRIEN_DEFAULT                0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO0IEN             */
#define KSU_CRYPTO0IEN_CRYPTO0ECC1BERRIEN_DEFAULT                 (_KSU_CRYPTO0IEN_CRYPTO0ECC1BERRIEN_DEFAULT << 2)        /**< Shifted mode DEFAULT for KSU_CRYPTO0IEN     */
#define KSU_CRYPTO0IEN_CRYPTO0ECC2BERRIEN                         (0x1UL << 3)                                             /**< ECC 2b error crypto int 0                   */
#define _KSU_CRYPTO0IEN_CRYPTO0ECC2BERRIEN_SHIFT                  3                                                        /**< Shift value for KSU_CRYPTO0ECC2BERRIEN      */
#define _KSU_CRYPTO0IEN_CRYPTO0ECC2BERRIEN_MASK                   0x8UL                                                    /**< Bit mask for KSU_CRYPTO0ECC2BERRIEN         */
#define _KSU_CRYPTO0IEN_CRYPTO0ECC2BERRIEN_DEFAULT                0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO0IEN             */
#define KSU_CRYPTO0IEN_CRYPTO0ECC2BERRIEN_DEFAULT                 (_KSU_CRYPTO0IEN_CRYPTO0ECC2BERRIEN_DEFAULT << 3)        /**< Shifted mode DEFAULT for KSU_CRYPTO0IEN     */

/* Bit fields for KSU CRYPTO0IF */
#define _KSU_CRYPTO0IF_RESETVALUE                                 0x00000000UL                                           /**< Default value for KSU_CRYPTO0IF             */
#define _KSU_CRYPTO0IF_MASK                                       0x0000000FUL                                           /**< Mask for KSU_CRYPTO0IF                      */
#define KSU_CRYPTO0IF_CRYPTO0INVALIDCRYPTOIDIF                    (0x1UL << 0)                                           /**< DKP_IF0 engineid error flag                 */
#define _KSU_CRYPTO0IF_CRYPTO0INVALIDCRYPTOIDIF_SHIFT             0                                                      /**< Shift value for KSU_CRYPTO0INVALIDCRYPTOIDIF*/
#define _KSU_CRYPTO0IF_CRYPTO0INVALIDCRYPTOIDIF_MASK              0x1UL                                                  /**< Bit mask for KSU_CRYPTO0INVALIDCRYPTOIDIF   */
#define _KSU_CRYPTO0IF_CRYPTO0INVALIDCRYPTOIDIF_DEFAULT           0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO0IF              */
#define KSU_CRYPTO0IF_CRYPTO0INVALIDCRYPTOIDIF_DEFAULT            (_KSU_CRYPTO0IF_CRYPTO0INVALIDCRYPTOIDIF_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO0IF      */
#define KSU_CRYPTO0IF_CRYPTO0INVALIDKEYSLOTIF                     (0x1UL << 1)                                           /**< DKP_IF0 slotid error flag                   */
#define _KSU_CRYPTO0IF_CRYPTO0INVALIDKEYSLOTIF_SHIFT              1                                                      /**< Shift value for KSU_CRYPTO0INVALIDKEYSLOTIF */
#define _KSU_CRYPTO0IF_CRYPTO0INVALIDKEYSLOTIF_MASK               0x2UL                                                  /**< Bit mask for KSU_CRYPTO0INVALIDKEYSLOTIF    */
#define _KSU_CRYPTO0IF_CRYPTO0INVALIDKEYSLOTIF_DEFAULT            0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO0IF              */
#define KSU_CRYPTO0IF_CRYPTO0INVALIDKEYSLOTIF_DEFAULT             (_KSU_CRYPTO0IF_CRYPTO0INVALIDKEYSLOTIF_DEFAULT << 1)  /**< Shifted mode DEFAULT for KSU_CRYPTO0IF      */
#define KSU_CRYPTO0IF_CRYPTO0ECC1BERRIF                           (0x1UL << 2)                                           /**< Ecc error 1b crypto intf0                   */
#define _KSU_CRYPTO0IF_CRYPTO0ECC1BERRIF_SHIFT                    2                                                      /**< Shift value for KSU_CRYPTO0ECC1BERRIF       */
#define _KSU_CRYPTO0IF_CRYPTO0ECC1BERRIF_MASK                     0x4UL                                                  /**< Bit mask for KSU_CRYPTO0ECC1BERRIF          */
#define _KSU_CRYPTO0IF_CRYPTO0ECC1BERRIF_DEFAULT                  0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO0IF              */
#define KSU_CRYPTO0IF_CRYPTO0ECC1BERRIF_DEFAULT                   (_KSU_CRYPTO0IF_CRYPTO0ECC1BERRIF_DEFAULT << 2)        /**< Shifted mode DEFAULT for KSU_CRYPTO0IF      */
#define KSU_CRYPTO0IF_CRYPTO0ECC2BERRIF                           (0x1UL << 3)                                           /**< ECC error 2b crypto intf0                   */
#define _KSU_CRYPTO0IF_CRYPTO0ECC2BERRIF_SHIFT                    3                                                      /**< Shift value for KSU_CRYPTO0ECC2BERRIF       */
#define _KSU_CRYPTO0IF_CRYPTO0ECC2BERRIF_MASK                     0x8UL                                                  /**< Bit mask for KSU_CRYPTO0ECC2BERRIF          */
#define _KSU_CRYPTO0IF_CRYPTO0ECC2BERRIF_DEFAULT                  0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO0IF              */
#define KSU_CRYPTO0IF_CRYPTO0ECC2BERRIF_DEFAULT                   (_KSU_CRYPTO0IF_CRYPTO0ECC2BERRIF_DEFAULT << 3)        /**< Shifted mode DEFAULT for KSU_CRYPTO0IF      */

/* Bit fields for KSU CRYPTO1IEN */
#define _KSU_CRYPTO1IEN_RESETVALUE                                0x00000000UL                                             /**< Default value for KSU_CRYPTO1IEN            */
#define _KSU_CRYPTO1IEN_MASK                                      0x0000000FUL                                             /**< Mask for KSU_CRYPTO1IEN                     */
#define KSU_CRYPTO1IEN_CRYPTO1INVALIDCRYPTOIDIEN                  (0x1UL << 0)                                             /**< DKP_IF1 engine id error                     */
#define _KSU_CRYPTO1IEN_CRYPTO1INVALIDCRYPTOIDIEN_SHIFT           0                                                        /**< Shift value for KSU_CRYPTO1INVALIDCRYPTOIDIEN*/
#define _KSU_CRYPTO1IEN_CRYPTO1INVALIDCRYPTOIDIEN_MASK            0x1UL                                                    /**< Bit mask for KSU_CRYPTO1INVALIDCRYPTOIDIEN  */
#define _KSU_CRYPTO1IEN_CRYPTO1INVALIDCRYPTOIDIEN_DEFAULT         0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO1IEN             */
#define KSU_CRYPTO1IEN_CRYPTO1INVALIDCRYPTOIDIEN_DEFAULT          (_KSU_CRYPTO1IEN_CRYPTO1INVALIDCRYPTOIDIEN_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO1IEN     */
#define KSU_CRYPTO1IEN_CRYPTO1INVALIDKEYSLOTIEN                   (0x1UL << 1)                                             /**< DKP_IF1 slot id error                       */
#define _KSU_CRYPTO1IEN_CRYPTO1INVALIDKEYSLOTIEN_SHIFT            1                                                        /**< Shift value for KSU_CRYPTO1INVALIDKEYSLOTIEN*/
#define _KSU_CRYPTO1IEN_CRYPTO1INVALIDKEYSLOTIEN_MASK             0x2UL                                                    /**< Bit mask for KSU_CRYPTO1INVALIDKEYSLOTIEN   */
#define _KSU_CRYPTO1IEN_CRYPTO1INVALIDKEYSLOTIEN_DEFAULT          0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO1IEN             */
#define KSU_CRYPTO1IEN_CRYPTO1INVALIDKEYSLOTIEN_DEFAULT           (_KSU_CRYPTO1IEN_CRYPTO1INVALIDKEYSLOTIEN_DEFAULT << 1)  /**< Shifted mode DEFAULT for KSU_CRYPTO1IEN     */
#define KSU_CRYPTO1IEN_CRYPTO1ECC1BERRIEN                         (0x1UL << 2)                                             /**< ECC 1b error crypto int 0                   */
#define _KSU_CRYPTO1IEN_CRYPTO1ECC1BERRIEN_SHIFT                  2                                                        /**< Shift value for KSU_CRYPTO1ECC1BERRIEN      */
#define _KSU_CRYPTO1IEN_CRYPTO1ECC1BERRIEN_MASK                   0x4UL                                                    /**< Bit mask for KSU_CRYPTO1ECC1BERRIEN         */
#define _KSU_CRYPTO1IEN_CRYPTO1ECC1BERRIEN_DEFAULT                0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO1IEN             */
#define KSU_CRYPTO1IEN_CRYPTO1ECC1BERRIEN_DEFAULT                 (_KSU_CRYPTO1IEN_CRYPTO1ECC1BERRIEN_DEFAULT << 2)        /**< Shifted mode DEFAULT for KSU_CRYPTO1IEN     */
#define KSU_CRYPTO1IEN_CRYPTO1ECC2BERRIEN                         (0x1UL << 3)                                             /**< ECC 2b error crypto int 0                   */
#define _KSU_CRYPTO1IEN_CRYPTO1ECC2BERRIEN_SHIFT                  3                                                        /**< Shift value for KSU_CRYPTO1ECC2BERRIEN      */
#define _KSU_CRYPTO1IEN_CRYPTO1ECC2BERRIEN_MASK                   0x8UL                                                    /**< Bit mask for KSU_CRYPTO1ECC2BERRIEN         */
#define _KSU_CRYPTO1IEN_CRYPTO1ECC2BERRIEN_DEFAULT                0x00000000UL                                             /**< Mode DEFAULT for KSU_CRYPTO1IEN             */
#define KSU_CRYPTO1IEN_CRYPTO1ECC2BERRIEN_DEFAULT                 (_KSU_CRYPTO1IEN_CRYPTO1ECC2BERRIEN_DEFAULT << 3)        /**< Shifted mode DEFAULT for KSU_CRYPTO1IEN     */

/* Bit fields for KSU CRYPTO1IF */
#define _KSU_CRYPTO1IF_RESETVALUE                                 0x00000000UL                                           /**< Default value for KSU_CRYPTO1IF             */
#define _KSU_CRYPTO1IF_MASK                                       0x0000000FUL                                           /**< Mask for KSU_CRYPTO1IF                      */
#define KSU_CRYPTO1IF_CRYPTO1INVALIDCRYPTOIDIF                    (0x1UL << 0)                                           /**< DKP_IF1 engineid error flag                 */
#define _KSU_CRYPTO1IF_CRYPTO1INVALIDCRYPTOIDIF_SHIFT             0                                                      /**< Shift value for KSU_CRYPTO1INVALIDCRYPTOIDIF*/
#define _KSU_CRYPTO1IF_CRYPTO1INVALIDCRYPTOIDIF_MASK              0x1UL                                                  /**< Bit mask for KSU_CRYPTO1INVALIDCRYPTOIDIF   */
#define _KSU_CRYPTO1IF_CRYPTO1INVALIDCRYPTOIDIF_DEFAULT           0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO1IF              */
#define KSU_CRYPTO1IF_CRYPTO1INVALIDCRYPTOIDIF_DEFAULT            (_KSU_CRYPTO1IF_CRYPTO1INVALIDCRYPTOIDIF_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO1IF      */
#define KSU_CRYPTO1IF_CRYPTO1INVALIDKEYSLOTIF                     (0x1UL << 1)                                           /**< DKP_IF1 slotid error flag                   */
#define _KSU_CRYPTO1IF_CRYPTO1INVALIDKEYSLOTIF_SHIFT              1                                                      /**< Shift value for KSU_CRYPTO1INVALIDKEYSLOTIF */
#define _KSU_CRYPTO1IF_CRYPTO1INVALIDKEYSLOTIF_MASK               0x2UL                                                  /**< Bit mask for KSU_CRYPTO1INVALIDKEYSLOTIF    */
#define _KSU_CRYPTO1IF_CRYPTO1INVALIDKEYSLOTIF_DEFAULT            0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO1IF              */
#define KSU_CRYPTO1IF_CRYPTO1INVALIDKEYSLOTIF_DEFAULT             (_KSU_CRYPTO1IF_CRYPTO1INVALIDKEYSLOTIF_DEFAULT << 1)  /**< Shifted mode DEFAULT for KSU_CRYPTO1IF      */
#define KSU_CRYPTO1IF_CRYPTO1ECC1BERRIF                           (0x1UL << 2)                                           /**< Ecc error 1b crypto intf0                   */
#define _KSU_CRYPTO1IF_CRYPTO1ECC1BERRIF_SHIFT                    2                                                      /**< Shift value for KSU_CRYPTO1ECC1BERRIF       */
#define _KSU_CRYPTO1IF_CRYPTO1ECC1BERRIF_MASK                     0x4UL                                                  /**< Bit mask for KSU_CRYPTO1ECC1BERRIF          */
#define _KSU_CRYPTO1IF_CRYPTO1ECC1BERRIF_DEFAULT                  0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO1IF              */
#define KSU_CRYPTO1IF_CRYPTO1ECC1BERRIF_DEFAULT                   (_KSU_CRYPTO1IF_CRYPTO1ECC1BERRIF_DEFAULT << 2)        /**< Shifted mode DEFAULT for KSU_CRYPTO1IF      */
#define KSU_CRYPTO1IF_CRYPTO1ECC2BERRIF                           (0x1UL << 3)                                           /**< ECC error 2b crypto intf0                   */
#define _KSU_CRYPTO1IF_CRYPTO1ECC2BERRIF_SHIFT                    3                                                      /**< Shift value for KSU_CRYPTO1ECC2BERRIF       */
#define _KSU_CRYPTO1IF_CRYPTO1ECC2BERRIF_MASK                     0x8UL                                                  /**< Bit mask for KSU_CRYPTO1ECC2BERRIF          */
#define _KSU_CRYPTO1IF_CRYPTO1ECC2BERRIF_DEFAULT                  0x00000000UL                                           /**< Mode DEFAULT for KSU_CRYPTO1IF              */
#define KSU_CRYPTO1IF_CRYPTO1ECC2BERRIF_DEFAULT                   (_KSU_CRYPTO1IF_CRYPTO1ECC2BERRIF_DEFAULT << 3)        /**< Shifted mode DEFAULT for KSU_CRYPTO1IF      */

/* Bit fields for KSU IRKECCERRKEYSLOT */
#define _KSU_IRKECCERRKEYSLOT_RESETVALUE                          0x00000000UL                                          /**< Default value for KSU_IRKECCERRKEYSLOT      */
#define _KSU_IRKECCERRKEYSLOT_MASK                                0x000000FFUL                                          /**< Mask for KSU_IRKECCERRKEYSLOT               */
#define _KSU_IRKECCERRKEYSLOT_IRKECCERRKEYSLOT_SHIFT              0                                                     /**< Shift value for KSU_IRKECCERRKEYSLOT        */
#define _KSU_IRKECCERRKEYSLOT_IRKECCERRKEYSLOT_MASK               0xFFUL                                                /**< Bit mask for KSU_IRKECCERRKEYSLOT           */
#define _KSU_IRKECCERRKEYSLOT_IRKECCERRKEYSLOT_DEFAULT            0x00000000UL                                          /**< Mode DEFAULT for KSU_IRKECCERRKEYSLOT       */
#define KSU_IRKECCERRKEYSLOT_IRKECCERRKEYSLOT_DEFAULT             (_KSU_IRKECCERRKEYSLOT_IRKECCERRKEYSLOT_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_IRKECCERRKEYSLOT*/

/* Bit fields for KSU CRYPTO0ECCERRKEYSLOT */
#define _KSU_CRYPTO0ECCERRKEYSLOT_RESETVALUE                      0x00000000UL                                                  /**< Default value for KSU_CRYPTO0ECCERRKEYSLOT  */
#define _KSU_CRYPTO0ECCERRKEYSLOT_MASK                            0x000000FFUL                                                  /**< Mask for KSU_CRYPTO0ECCERRKEYSLOT           */
#define _KSU_CRYPTO0ECCERRKEYSLOT_CRYPTO0ECCERRKEYSLOT_SHIFT      0                                                             /**< Shift value for KSU_CRYPTO0ECCERRKEYSLOT    */
#define _KSU_CRYPTO0ECCERRKEYSLOT_CRYPTO0ECCERRKEYSLOT_MASK       0xFFUL                                                        /**< Bit mask for KSU_CRYPTO0ECCERRKEYSLOT       */
#define _KSU_CRYPTO0ECCERRKEYSLOT_CRYPTO0ECCERRKEYSLOT_DEFAULT    0x00000000UL                                                  /**< Mode DEFAULT for KSU_CRYPTO0ECCERRKEYSLOT   */
#define KSU_CRYPTO0ECCERRKEYSLOT_CRYPTO0ECCERRKEYSLOT_DEFAULT     (_KSU_CRYPTO0ECCERRKEYSLOT_CRYPTO0ECCERRKEYSLOT_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO0ECCERRKEYSLOT*/

/* Bit fields for KSU CRYPTO1ECCERRKEYSLOT */
#define _KSU_CRYPTO1ECCERRKEYSLOT_RESETVALUE                      0x00000000UL                                                  /**< Default value for KSU_CRYPTO1ECCERRKEYSLOT  */
#define _KSU_CRYPTO1ECCERRKEYSLOT_MASK                            0x000000FFUL                                                  /**< Mask for KSU_CRYPTO1ECCERRKEYSLOT           */
#define _KSU_CRYPTO1ECCERRKEYSLOT_CRYPTO1ECCERRKEYSLOT_SHIFT      0                                                             /**< Shift value for KSU_CRYPTO1ECCERRKEYSLOT    */
#define _KSU_CRYPTO1ECCERRKEYSLOT_CRYPTO1ECCERRKEYSLOT_MASK       0xFFUL                                                        /**< Bit mask for KSU_CRYPTO1ECCERRKEYSLOT       */
#define _KSU_CRYPTO1ECCERRKEYSLOT_CRYPTO1ECCERRKEYSLOT_DEFAULT    0x00000000UL                                                  /**< Mode DEFAULT for KSU_CRYPTO1ECCERRKEYSLOT   */
#define KSU_CRYPTO1ECCERRKEYSLOT_CRYPTO1ECCERRKEYSLOT_DEFAULT     (_KSU_CRYPTO1ECCERRKEYSLOT_CRYPTO1ECCERRKEYSLOT_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CRYPTO1ECCERRKEYSLOT*/

/* Bit fields for KSU CORERESET */
#define _KSU_CORERESET_RESETVALUE                                 0x00000000UL                            /**< Default value for KSU_CORERESET             */
#define _KSU_CORERESET_MASK                                       0x00000001UL                            /**< Mask for KSU_CORERESET                      */
#define KSU_CORERESET_CORERESET                                   (0x1UL << 0)                            /**< resets the ksu                              */
#define _KSU_CORERESET_CORERESET_SHIFT                            0                                       /**< Shift value for KSU_CORERESET               */
#define _KSU_CORERESET_CORERESET_MASK                             0x1UL                                   /**< Bit mask for KSU_CORERESET                  */
#define _KSU_CORERESET_CORERESET_DEFAULT                          0x00000000UL                            /**< Mode DEFAULT for KSU_CORERESET              */
#define KSU_CORERESET_CORERESET_DEFAULT                           (_KSU_CORERESET_CORERESET_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_CORERESET      */

/* Bit fields for KSU MISC */
#define _KSU_MISC_RESETVALUE                                      0x00000007UL                  /**< Default value for KSU_MISC                  */
#define _KSU_MISC_MASK                                            0x00000007UL                  /**< Mask for KSU_MISC                           */
#define _KSU_MISC_SWAP_SHIFT                                      0                             /**< Shift value for KSU_SWAP                    */
#define _KSU_MISC_SWAP_MASK                                       0x7UL                         /**< Bit mask for KSU_SWAP                       */
#define _KSU_MISC_SWAP_DEFAULT                                    0x00000007UL                  /**< Mode DEFAULT for KSU_MISC                   */
#define KSU_MISC_SWAP_DEFAULT                                     (_KSU_MISC_SWAP_DEFAULT << 0) /**< Shifted mode DEFAULT for KSU_MISC           */

/** @} End of group SIMG301_KSU_BitFields */
/** @} End of group SIMG301_KSU */
/** @} End of group Parts */

#endif // SIMG301_KSU_H
