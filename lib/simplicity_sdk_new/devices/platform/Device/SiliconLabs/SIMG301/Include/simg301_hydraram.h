/**************************************************************************//**
 * @file
 * @brief SIMG301 HYDRARAM register and bit field definitions
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
#ifndef SIMG301_HYDRARAM_H
#define SIMG301_HYDRARAM_H
#define HYDRARAM_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_HYDRARAM HYDRARAM
 * @{
 * @brief SIMG301 HYDRARAM Register Declaration.
 *****************************************************************************/

/** HYDRARAM Register Declaration. */
typedef struct hydraram_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP version ID                                      */
  __IOM uint32_t CMD;                           /**< Command Register                                   */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  __IOM uint32_t PORTPRIORITY;                  /**< Port Arbitration Priority Register                 */
  __IOM uint32_t INTERLEAVE2BANK;               /**< 2 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE4BANK;               /**< 4 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE8BANK;               /**< 8 Bank Interleaving Register                       */
  __IM uint32_t  ECCERRADDR;                    /**< ECC Error Address Register                         */
  __IM uint32_t  ECCMERRIND;                    /**< Multiple ECC Error Indication                      */
  __IOM uint32_t IF;                            /**< Interrupt Flags                                    */
  __IOM uint32_t IEN;                           /**< Interrupt Enable                                   */
  __IOM uint32_t BANKSCACHEEN;                  /**< RAM bank cache enable                              */
  uint32_t       RESERVED0[2U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED2[1009U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP version ID                                      */
  __IOM uint32_t CMD_SET;                       /**< Command Register                                   */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  __IOM uint32_t PORTPRIORITY_SET;              /**< Port Arbitration Priority Register                 */
  __IOM uint32_t INTERLEAVE2BANK_SET;           /**< 2 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE4BANK_SET;           /**< 4 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE8BANK_SET;           /**< 8 Bank Interleaving Register                       */
  __IM uint32_t  ECCERRADDR_SET;                /**< ECC Error Address Register                         */
  __IM uint32_t  ECCMERRIND_SET;                /**< Multiple ECC Error Indication                      */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable                                   */
  __IOM uint32_t BANKSCACHEEN_SET;              /**< RAM bank cache enable                              */
  uint32_t       RESERVED3[2U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED4[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED5[1009U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP version ID                                      */
  __IOM uint32_t CMD_CLR;                       /**< Command Register                                   */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  __IOM uint32_t PORTPRIORITY_CLR;              /**< Port Arbitration Priority Register                 */
  __IOM uint32_t INTERLEAVE2BANK_CLR;           /**< 2 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE4BANK_CLR;           /**< 4 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE8BANK_CLR;           /**< 8 Bank Interleaving Register                       */
  __IM uint32_t  ECCERRADDR_CLR;                /**< ECC Error Address Register                         */
  __IM uint32_t  ECCMERRIND_CLR;                /**< Multiple ECC Error Indication                      */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable                                   */
  __IOM uint32_t BANKSCACHEEN_CLR;              /**< RAM bank cache enable                              */
  uint32_t       RESERVED6[2U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED7[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED8[1009U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP version ID                                      */
  __IOM uint32_t CMD_TGL;                       /**< Command Register                                   */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  __IOM uint32_t PORTPRIORITY_TGL;              /**< Port Arbitration Priority Register                 */
  __IOM uint32_t INTERLEAVE2BANK_TGL;           /**< 2 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE4BANK_TGL;           /**< 4 Bank Interleaving Register                       */
  __IOM uint32_t INTERLEAVE8BANK_TGL;           /**< 8 Bank Interleaving Register                       */
  __IM uint32_t  ECCERRADDR_TGL;                /**< ECC Error Address Register                         */
  __IM uint32_t  ECCMERRIND_TGL;                /**< Multiple ECC Error Indication                      */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable                                   */
  __IOM uint32_t BANKSCACHEEN_TGL;              /**< RAM bank cache enable                              */
  uint32_t       RESERVED9[2U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
} HYDRARAM_TypeDef;
/** @} End of group SIMG301_HYDRARAM */

/**************************************************************************//**
 * @addtogroup SIMG301_HYDRARAM
 * @{
 * @defgroup SIMG301_HYDRARAM_BitFields HYDRARAM Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for HYDRARAM IPVERSION */
#define _HYDRARAM_IPVERSION_RESETVALUE                      0x00000001UL                                 /**< Default value for HYDRARAM_IPVERSION        */
#define _HYDRARAM_IPVERSION_MASK                            0x00000003UL                                 /**< Mask for HYDRARAM_IPVERSION                 */
#define _HYDRARAM_IPVERSION_IPVERSION_SHIFT                 0                                            /**< Shift value for HYDRARAM_IPVERSION          */
#define _HYDRARAM_IPVERSION_IPVERSION_MASK                  0x3UL                                        /**< Bit mask for HYDRARAM_IPVERSION             */
#define _HYDRARAM_IPVERSION_IPVERSION_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for HYDRARAM_IPVERSION         */
#define HYDRARAM_IPVERSION_IPVERSION_DEFAULT                (_HYDRARAM_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_IPVERSION */

/* Bit fields for HYDRARAM CMD */
#define _HYDRARAM_CMD_RESETVALUE                            0x00000000UL                          /**< Default value for HYDRARAM_CMD              */
#define _HYDRARAM_CMD_MASK                                  0x00000001UL                          /**< Mask for HYDRARAM_CMD                       */
#define HYDRARAM_CMD_CLEARECC                               (0x1UL << 0)                          /**< Clear ECCERRADDR and ECCMERRIND.MERRIND     */
#define _HYDRARAM_CMD_CLEARECC_SHIFT                        0                                     /**< Shift value for HYDRARAM_CLEARECC           */
#define _HYDRARAM_CMD_CLEARECC_MASK                         0x1UL                                 /**< Bit mask for HYDRARAM_CLEARECC              */
#define _HYDRARAM_CMD_CLEARECC_DEFAULT                      0x00000000UL                          /**< Mode DEFAULT for HYDRARAM_CMD               */
#define HYDRARAM_CMD_CLEARECC_DEFAULT                       (_HYDRARAM_CMD_CLEARECC_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_CMD       */

/* Bit fields for HYDRARAM CTRL */
#define _HYDRARAM_CTRL_RESETVALUE                           0x00000008UL                                /**< Default value for HYDRARAM_CTRL             */
#define _HYDRARAM_CTRL_MASK                                 0x0000000FUL                                /**< Mask for HYDRARAM_CTRL                      */
#define HYDRARAM_CTRL_ECCEN                                 (0x1UL << 0)                                /**< Enable ECC functionality                    */
#define _HYDRARAM_CTRL_ECCEN_SHIFT                          0                                           /**< Shift value for HYDRARAM_ECCEN              */
#define _HYDRARAM_CTRL_ECCEN_MASK                           0x1UL                                       /**< Bit mask for HYDRARAM_ECCEN                 */
#define _HYDRARAM_CTRL_ECCEN_DEFAULT                        0x00000000UL                                /**< Mode DEFAULT for HYDRARAM_CTRL              */
#define HYDRARAM_CTRL_ECCEN_DEFAULT                         (_HYDRARAM_CTRL_ECCEN_DEFAULT << 0)         /**< Shifted mode DEFAULT for HYDRARAM_CTRL      */
#define HYDRARAM_CTRL_ECCWEN                                (0x1UL << 1)                                /**< Enable ECC syndrome writes                  */
#define _HYDRARAM_CTRL_ECCWEN_SHIFT                         1                                           /**< Shift value for HYDRARAM_ECCWEN             */
#define _HYDRARAM_CTRL_ECCWEN_MASK                          0x2UL                                       /**< Bit mask for HYDRARAM_ECCWEN                */
#define _HYDRARAM_CTRL_ECCWEN_DEFAULT                       0x00000000UL                                /**< Mode DEFAULT for HYDRARAM_CTRL              */
#define HYDRARAM_CTRL_ECCWEN_DEFAULT                        (_HYDRARAM_CTRL_ECCWEN_DEFAULT << 1)        /**< Shifted mode DEFAULT for HYDRARAM_CTRL      */
#define HYDRARAM_CTRL_ECCERRFAULTEN                         (0x1UL << 2)                                /**< ECC Error bus fault enable                  */
#define _HYDRARAM_CTRL_ECCERRFAULTEN_SHIFT                  2                                           /**< Shift value for HYDRARAM_ECCERRFAULTEN      */
#define _HYDRARAM_CTRL_ECCERRFAULTEN_MASK                   0x4UL                                       /**< Bit mask for HYDRARAM_ECCERRFAULTEN         */
#define _HYDRARAM_CTRL_ECCERRFAULTEN_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for HYDRARAM_CTRL              */
#define HYDRARAM_CTRL_ECCERRFAULTEN_DEFAULT                 (_HYDRARAM_CTRL_ECCERRFAULTEN_DEFAULT << 2) /**< Shifted mode DEFAULT for HYDRARAM_CTRL      */
#define HYDRARAM_CTRL_ADDRFAULTEN                           (0x1UL << 3)                                /**< Address fault bus fault enable              */
#define _HYDRARAM_CTRL_ADDRFAULTEN_SHIFT                    3                                           /**< Shift value for HYDRARAM_ADDRFAULTEN        */
#define _HYDRARAM_CTRL_ADDRFAULTEN_MASK                     0x8UL                                       /**< Bit mask for HYDRARAM_ADDRFAULTEN           */
#define _HYDRARAM_CTRL_ADDRFAULTEN_DEFAULT                  0x00000001UL                                /**< Mode DEFAULT for HYDRARAM_CTRL              */
#define HYDRARAM_CTRL_ADDRFAULTEN_DEFAULT                   (_HYDRARAM_CTRL_ADDRFAULTEN_DEFAULT << 3)   /**< Shifted mode DEFAULT for HYDRARAM_CTRL      */

/* Bit fields for HYDRARAM PORTPRIORITY */
#define _HYDRARAM_PORTPRIORITY_RESETVALUE                   0x00000000UL                                        /**< Default value for HYDRARAM_PORTPRIORITY     */
#define _HYDRARAM_PORTPRIORITY_MASK                         0x00000055UL                                        /**< Mask for HYDRARAM_PORTPRIORITY              */
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY0                 (0x1UL << 0)                                        /**< Port0 Arbitration Priority                  */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY0_SHIFT          0                                                   /**< Shift value for HYDRARAM_PORTPRIORITY0      */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY0_MASK           0x1UL                                               /**< Bit mask for HYDRARAM_PORTPRIORITY0         */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY0_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for HYDRARAM_PORTPRIORITY      */
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY0_DEFAULT         (_HYDRARAM_PORTPRIORITY_PORTPRIORITY0_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_PORTPRIORITY*/
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY1                 (0x1UL << 2)                                        /**< Port1 Arbitration Priority                  */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY1_SHIFT          2                                                   /**< Shift value for HYDRARAM_PORTPRIORITY1      */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY1_MASK           0x4UL                                               /**< Bit mask for HYDRARAM_PORTPRIORITY1         */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY1_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for HYDRARAM_PORTPRIORITY      */
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY1_DEFAULT         (_HYDRARAM_PORTPRIORITY_PORTPRIORITY1_DEFAULT << 2) /**< Shifted mode DEFAULT for HYDRARAM_PORTPRIORITY*/
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY2                 (0x1UL << 4)                                        /**< Port2 Arbitration Priority                  */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY2_SHIFT          4                                                   /**< Shift value for HYDRARAM_PORTPRIORITY2      */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY2_MASK           0x10UL                                              /**< Bit mask for HYDRARAM_PORTPRIORITY2         */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY2_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for HYDRARAM_PORTPRIORITY      */
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY2_DEFAULT         (_HYDRARAM_PORTPRIORITY_PORTPRIORITY2_DEFAULT << 4) /**< Shifted mode DEFAULT for HYDRARAM_PORTPRIORITY*/
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY3                 (0x1UL << 6)                                        /**< Port3 Arbitration Priority                  */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY3_SHIFT          6                                                   /**< Shift value for HYDRARAM_PORTPRIORITY3      */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY3_MASK           0x40UL                                              /**< Bit mask for HYDRARAM_PORTPRIORITY3         */
#define _HYDRARAM_PORTPRIORITY_PORTPRIORITY3_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for HYDRARAM_PORTPRIORITY      */
#define HYDRARAM_PORTPRIORITY_PORTPRIORITY3_DEFAULT         (_HYDRARAM_PORTPRIORITY_PORTPRIORITY3_DEFAULT << 6) /**< Shifted mode DEFAULT for HYDRARAM_PORTPRIORITY*/

/* Bit fields for HYDRARAM INTERLEAVE2BANK */
#define _HYDRARAM_INTERLEAVE2BANK_RESETVALUE                0x00000000UL                                             /**< Default value for HYDRARAM_INTERLEAVE2BANK  */
#define _HYDRARAM_INTERLEAVE2BANK_MASK                      0x0000FFFFUL                                             /**< Mask for HYDRARAM_INTERLEAVE2BANK           */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT0              (0x1UL << 0)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT0_SHIFT       0                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT0      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT0_MASK        0x1UL                                                    /**< Bit mask for HYDRARAM_INTL2BANKBIT0         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT0_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT0_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT0_DEFAULT << 0)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT1              (0x1UL << 1)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT1_SHIFT       1                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT1      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT1_MASK        0x2UL                                                    /**< Bit mask for HYDRARAM_INTL2BANKBIT1         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT1_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT1_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT1_DEFAULT << 1)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT2              (0x1UL << 2)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT2_SHIFT       2                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT2      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT2_MASK        0x4UL                                                    /**< Bit mask for HYDRARAM_INTL2BANKBIT2         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT2_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT2_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT2_DEFAULT << 2)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT3              (0x1UL << 3)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT3_SHIFT       3                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT3      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT3_MASK        0x8UL                                                    /**< Bit mask for HYDRARAM_INTL2BANKBIT3         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT3_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT3_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT3_DEFAULT << 3)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT4              (0x1UL << 4)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT4_SHIFT       4                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT4      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT4_MASK        0x10UL                                                   /**< Bit mask for HYDRARAM_INTL2BANKBIT4         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT4_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT4_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT4_DEFAULT << 4)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT5              (0x1UL << 5)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT5_SHIFT       5                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT5      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT5_MASK        0x20UL                                                   /**< Bit mask for HYDRARAM_INTL2BANKBIT5         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT5_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT5_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT5_DEFAULT << 5)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT6              (0x1UL << 6)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT6_SHIFT       6                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT6      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT6_MASK        0x40UL                                                   /**< Bit mask for HYDRARAM_INTL2BANKBIT6         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT6_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT6_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT6_DEFAULT << 6)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT7              (0x1UL << 7)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT7_SHIFT       7                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT7      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT7_MASK        0x80UL                                                   /**< Bit mask for HYDRARAM_INTL2BANKBIT7         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT7_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT7_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT7_DEFAULT << 7)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT8              (0x1UL << 8)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT8_SHIFT       8                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT8      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT8_MASK        0x100UL                                                  /**< Bit mask for HYDRARAM_INTL2BANKBIT8         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT8_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT8_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT8_DEFAULT << 8)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT9              (0x1UL << 9)                                             /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT9_SHIFT       9                                                        /**< Shift value for HYDRARAM_INTL2BANKBIT9      */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT9_MASK        0x200UL                                                  /**< Bit mask for HYDRARAM_INTL2BANKBIT9         */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT9_DEFAULT     0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT9_DEFAULT      (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT9_DEFAULT << 9)   /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT10             (0x1UL << 10)                                            /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT10_SHIFT      10                                                       /**< Shift value for HYDRARAM_INTL2BANKBIT10     */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT10_MASK       0x400UL                                                  /**< Bit mask for HYDRARAM_INTL2BANKBIT10        */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT10_DEFAULT    0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT10_DEFAULT     (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT10_DEFAULT << 10) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT11             (0x1UL << 11)                                            /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT11_SHIFT      11                                                       /**< Shift value for HYDRARAM_INTL2BANKBIT11     */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT11_MASK       0x800UL                                                  /**< Bit mask for HYDRARAM_INTL2BANKBIT11        */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT11_DEFAULT    0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT11_DEFAULT     (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT11_DEFAULT << 11) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT12             (0x1UL << 12)                                            /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT12_SHIFT      12                                                       /**< Shift value for HYDRARAM_INTL2BANKBIT12     */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT12_MASK       0x1000UL                                                 /**< Bit mask for HYDRARAM_INTL2BANKBIT12        */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT12_DEFAULT    0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT12_DEFAULT     (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT12_DEFAULT << 12) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT13             (0x1UL << 13)                                            /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT13_SHIFT      13                                                       /**< Shift value for HYDRARAM_INTL2BANKBIT13     */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT13_MASK       0x2000UL                                                 /**< Bit mask for HYDRARAM_INTL2BANKBIT13        */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT13_DEFAULT    0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT13_DEFAULT     (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT13_DEFAULT << 13) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT14             (0x1UL << 14)                                            /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT14_SHIFT      14                                                       /**< Shift value for HYDRARAM_INTL2BANKBIT14     */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT14_MASK       0x4000UL                                                 /**< Bit mask for HYDRARAM_INTL2BANKBIT14        */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT14_DEFAULT    0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT14_DEFAULT     (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT14_DEFAULT << 14) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT15             (0x1UL << 15)                                            /**< Per 2 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT15_SHIFT      15                                                       /**< Shift value for HYDRARAM_INTL2BANKBIT15     */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT15_MASK       0x8000UL                                                 /**< Bit mask for HYDRARAM_INTL2BANKBIT15        */
#define _HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT15_DEFAULT    0x00000000UL                                             /**< Mode DEFAULT for HYDRARAM_INTERLEAVE2BANK   */
#define HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT15_DEFAULT     (_HYDRARAM_INTERLEAVE2BANK_INTL2BANKBIT15_DEFAULT << 15) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE2BANK*/

/* Bit fields for HYDRARAM INTERLEAVE4BANK */
#define _HYDRARAM_INTERLEAVE4BANK_RESETVALUE                0x00000000UL                                           /**< Default value for HYDRARAM_INTERLEAVE4BANK  */
#define _HYDRARAM_INTERLEAVE4BANK_MASK                      0x000000FFUL                                           /**< Mask for HYDRARAM_INTERLEAVE4BANK           */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT0              (0x1UL << 0)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT0_SHIFT       0                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT0      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT0_MASK        0x1UL                                                  /**< Bit mask for HYDRARAM_INTL4BANKBIT0         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT0_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT0_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT0_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT1              (0x1UL << 1)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT1_SHIFT       1                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT1      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT1_MASK        0x2UL                                                  /**< Bit mask for HYDRARAM_INTL4BANKBIT1         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT1_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT1_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT1_DEFAULT << 1) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT2              (0x1UL << 2)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT2_SHIFT       2                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT2      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT2_MASK        0x4UL                                                  /**< Bit mask for HYDRARAM_INTL4BANKBIT2         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT2_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT2_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT2_DEFAULT << 2) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT3              (0x1UL << 3)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT3_SHIFT       3                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT3      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT3_MASK        0x8UL                                                  /**< Bit mask for HYDRARAM_INTL4BANKBIT3         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT3_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT3_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT3_DEFAULT << 3) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT4              (0x1UL << 4)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT4_SHIFT       4                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT4      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT4_MASK        0x10UL                                                 /**< Bit mask for HYDRARAM_INTL4BANKBIT4         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT4_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT4_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT4_DEFAULT << 4) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT5              (0x1UL << 5)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT5_SHIFT       5                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT5      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT5_MASK        0x20UL                                                 /**< Bit mask for HYDRARAM_INTL4BANKBIT5         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT5_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT5_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT5_DEFAULT << 5) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT6              (0x1UL << 6)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT6_SHIFT       6                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT6      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT6_MASK        0x40UL                                                 /**< Bit mask for HYDRARAM_INTL4BANKBIT6         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT6_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT6_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT6_DEFAULT << 6) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT7              (0x1UL << 7)                                           /**< Per 4 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT7_SHIFT       7                                                      /**< Shift value for HYDRARAM_INTL4BANKBIT7      */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT7_MASK        0x80UL                                                 /**< Bit mask for HYDRARAM_INTL4BANKBIT7         */
#define _HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT7_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE4BANK   */
#define HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT7_DEFAULT      (_HYDRARAM_INTERLEAVE4BANK_INTL4BANKBIT7_DEFAULT << 7) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE4BANK*/

/* Bit fields for HYDRARAM INTERLEAVE8BANK */
#define _HYDRARAM_INTERLEAVE8BANK_RESETVALUE                0x00000000UL                                           /**< Default value for HYDRARAM_INTERLEAVE8BANK  */
#define _HYDRARAM_INTERLEAVE8BANK_MASK                      0x0000000FUL                                           /**< Mask for HYDRARAM_INTERLEAVE8BANK           */
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT0              (0x1UL << 0)                                           /**< Per 8 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT0_SHIFT       0                                                      /**< Shift value for HYDRARAM_INTL8BANKBIT0      */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT0_MASK        0x1UL                                                  /**< Bit mask for HYDRARAM_INTL8BANKBIT0         */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT0_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE8BANK   */
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT0_DEFAULT      (_HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT0_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE8BANK*/
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT1              (0x1UL << 1)                                           /**< Per 8 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT1_SHIFT       1                                                      /**< Shift value for HYDRARAM_INTL8BANKBIT1      */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT1_MASK        0x2UL                                                  /**< Bit mask for HYDRARAM_INTL8BANKBIT1         */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT1_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE8BANK   */
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT1_DEFAULT      (_HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT1_DEFAULT << 1) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE8BANK*/
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT2              (0x1UL << 2)                                           /**< Per 8 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT2_SHIFT       2                                                      /**< Shift value for HYDRARAM_INTL8BANKBIT2      */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT2_MASK        0x4UL                                                  /**< Bit mask for HYDRARAM_INTL8BANKBIT2         */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT2_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE8BANK   */
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT2_DEFAULT      (_HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT2_DEFAULT << 2) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE8BANK*/
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT3              (0x1UL << 3)                                           /**< Per 8 bank interleaving bit                 */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT3_SHIFT       3                                                      /**< Shift value for HYDRARAM_INTL8BANKBIT3      */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT3_MASK        0x8UL                                                  /**< Bit mask for HYDRARAM_INTL8BANKBIT3         */
#define _HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT3_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for HYDRARAM_INTERLEAVE8BANK   */
#define HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT3_DEFAULT      (_HYDRARAM_INTERLEAVE8BANK_INTL8BANKBIT3_DEFAULT << 3) /**< Shifted mode DEFAULT for HYDRARAM_INTERLEAVE8BANK*/

/* Bit fields for HYDRARAM ECCERRADDR */
#define _HYDRARAM_ECCERRADDR_RESETVALUE                     0x00000000UL                             /**< Default value for HYDRARAM_ECCERRADDR       */
#define _HYDRARAM_ECCERRADDR_MASK                           0xFFFFFFFFUL                             /**< Mask for HYDRARAM_ECCERRADDR                */
#define _HYDRARAM_ECCERRADDR_ADDR_SHIFT                     0                                        /**< Shift value for HYDRARAM_ADDR               */
#define _HYDRARAM_ECCERRADDR_ADDR_MASK                      0xFFFFFFFFUL                             /**< Bit mask for HYDRARAM_ADDR                  */
#define _HYDRARAM_ECCERRADDR_ADDR_DEFAULT                   0x00000000UL                             /**< Mode DEFAULT for HYDRARAM_ECCERRADDR        */
#define HYDRARAM_ECCERRADDR_ADDR_DEFAULT                    (_HYDRARAM_ECCERRADDR_ADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_ECCERRADDR*/

/* Bit fields for HYDRARAM ECCMERRIND */
#define _HYDRARAM_ECCMERRIND_RESETVALUE                     0x00000000UL                                /**< Default value for HYDRARAM_ECCMERRIND       */
#define _HYDRARAM_ECCMERRIND_MASK                           0x00000001UL                                /**< Mask for HYDRARAM_ECCMERRIND                */
#define HYDRARAM_ECCMERRIND_MERRIND                         (0x1UL << 0)                                /**< Multiple ECC errors on HYDRARAM             */
#define _HYDRARAM_ECCMERRIND_MERRIND_SHIFT                  0                                           /**< Shift value for HYDRARAM_MERRIND            */
#define _HYDRARAM_ECCMERRIND_MERRIND_MASK                   0x1UL                                       /**< Bit mask for HYDRARAM_MERRIND               */
#define _HYDRARAM_ECCMERRIND_MERRIND_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for HYDRARAM_ECCMERRIND        */
#define HYDRARAM_ECCMERRIND_MERRIND_DEFAULT                 (_HYDRARAM_ECCMERRIND_MERRIND_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_ECCMERRIND*/

/* Bit fields for HYDRARAM IF */
#define _HYDRARAM_IF_RESETVALUE                             0x00000000UL                             /**< Default value for HYDRARAM_IF               */
#define _HYDRARAM_IF_MASK                                   0x00000007UL                             /**< Mask for HYDRARAM_IF                        */
#define HYDRARAM_IF_ERR1B                                   (0x1UL << 0)                             /**< 1-bit ECC Error Interrupt Flag              */
#define _HYDRARAM_IF_ERR1B_SHIFT                            0                                        /**< Shift value for HYDRARAM_ERR1B              */
#define _HYDRARAM_IF_ERR1B_MASK                             0x1UL                                    /**< Bit mask for HYDRARAM_ERR1B                 */
#define _HYDRARAM_IF_ERR1B_DEFAULT                          0x00000000UL                             /**< Mode DEFAULT for HYDRARAM_IF                */
#define HYDRARAM_IF_ERR1B_DEFAULT                           (_HYDRARAM_IF_ERR1B_DEFAULT << 0)        /**< Shifted mode DEFAULT for HYDRARAM_IF        */
#define HYDRARAM_IF_ERR2B                                   (0x1UL << 1)                             /**< 2-bit ECC Error Interrupt Flag              */
#define _HYDRARAM_IF_ERR2B_SHIFT                            1                                        /**< Shift value for HYDRARAM_ERR2B              */
#define _HYDRARAM_IF_ERR2B_MASK                             0x2UL                                    /**< Bit mask for HYDRARAM_ERR2B                 */
#define _HYDRARAM_IF_ERR2B_DEFAULT                          0x00000000UL                             /**< Mode DEFAULT for HYDRARAM_IF                */
#define HYDRARAM_IF_ERR2B_DEFAULT                           (_HYDRARAM_IF_ERR2B_DEFAULT << 1)        /**< Shifted mode DEFAULT for HYDRARAM_IF        */
#define HYDRARAM_IF_INTEGRITYERR                            (0x1UL << 2)                             /**< Integrity check error Interrupt flag        */
#define _HYDRARAM_IF_INTEGRITYERR_SHIFT                     2                                        /**< Shift value for HYDRARAM_INTEGRITYERR       */
#define _HYDRARAM_IF_INTEGRITYERR_MASK                      0x4UL                                    /**< Bit mask for HYDRARAM_INTEGRITYERR          */
#define _HYDRARAM_IF_INTEGRITYERR_DEFAULT                   0x00000000UL                             /**< Mode DEFAULT for HYDRARAM_IF                */
#define HYDRARAM_IF_INTEGRITYERR_DEFAULT                    (_HYDRARAM_IF_INTEGRITYERR_DEFAULT << 2) /**< Shifted mode DEFAULT for HYDRARAM_IF        */

/* Bit fields for HYDRARAM IEN */
#define _HYDRARAM_IEN_RESETVALUE                            0x00000000UL                              /**< Default value for HYDRARAM_IEN              */
#define _HYDRARAM_IEN_MASK                                  0x00000007UL                              /**< Mask for HYDRARAM_IEN                       */
#define HYDRARAM_IEN_ERR1B                                  (0x1UL << 0)                              /**< 1-bit ECC Error Interrupt Enable            */
#define _HYDRARAM_IEN_ERR1B_SHIFT                           0                                         /**< Shift value for HYDRARAM_ERR1B              */
#define _HYDRARAM_IEN_ERR1B_MASK                            0x1UL                                     /**< Bit mask for HYDRARAM_ERR1B                 */
#define _HYDRARAM_IEN_ERR1B_DEFAULT                         0x00000000UL                              /**< Mode DEFAULT for HYDRARAM_IEN               */
#define HYDRARAM_IEN_ERR1B_DEFAULT                          (_HYDRARAM_IEN_ERR1B_DEFAULT << 0)        /**< Shifted mode DEFAULT for HYDRARAM_IEN       */
#define HYDRARAM_IEN_ERR2B                                  (0x1UL << 1)                              /**< 2-bit ECC Error Interrupt Enable            */
#define _HYDRARAM_IEN_ERR2B_SHIFT                           1                                         /**< Shift value for HYDRARAM_ERR2B              */
#define _HYDRARAM_IEN_ERR2B_MASK                            0x2UL                                     /**< Bit mask for HYDRARAM_ERR2B                 */
#define _HYDRARAM_IEN_ERR2B_DEFAULT                         0x00000000UL                              /**< Mode DEFAULT for HYDRARAM_IEN               */
#define HYDRARAM_IEN_ERR2B_DEFAULT                          (_HYDRARAM_IEN_ERR2B_DEFAULT << 1)        /**< Shifted mode DEFAULT for HYDRARAM_IEN       */
#define HYDRARAM_IEN_INTEGRITYERR                           (0x1UL << 2)                              /**< Integrity check error Interrupt Enable      */
#define _HYDRARAM_IEN_INTEGRITYERR_SHIFT                    2                                         /**< Shift value for HYDRARAM_INTEGRITYERR       */
#define _HYDRARAM_IEN_INTEGRITYERR_MASK                     0x4UL                                     /**< Bit mask for HYDRARAM_INTEGRITYERR          */
#define _HYDRARAM_IEN_INTEGRITYERR_DEFAULT                  0x00000000UL                              /**< Mode DEFAULT for HYDRARAM_IEN               */
#define HYDRARAM_IEN_INTEGRITYERR_DEFAULT                   (_HYDRARAM_IEN_INTEGRITYERR_DEFAULT << 2) /**< Shifted mode DEFAULT for HYDRARAM_IEN       */

/* Bit fields for HYDRARAM BANKSCACHEEN */
#define _HYDRARAM_BANKSCACHEEN_RESETVALUE                   0x00000000UL                                       /**< Default value for HYDRARAM_BANKSCACHEEN     */
#define _HYDRARAM_BANKSCACHEEN_MASK                         0xFFFFFFFFUL                                       /**< Mask for HYDRARAM_BANKSCACHEEN              */
#define _HYDRARAM_BANKSCACHEEN_BANKSCACHEEN_SHIFT           0                                                  /**< Shift value for HYDRARAM_BANKSCACHEEN       */
#define _HYDRARAM_BANKSCACHEEN_BANKSCACHEEN_MASK            0xFFFFFFFFUL                                       /**< Bit mask for HYDRARAM_BANKSCACHEEN          */
#define _HYDRARAM_BANKSCACHEEN_BANKSCACHEEN_DEFAULT         0x00000000UL                                       /**< Mode DEFAULT for HYDRARAM_BANKSCACHEEN      */
#define HYDRARAM_BANKSCACHEEN_BANKSCACHEEN_DEFAULT          (_HYDRARAM_BANKSCACHEEN_BANKSCACHEEN_DEFAULT << 0) /**< Shifted mode DEFAULT for HYDRARAM_BANKSCACHEEN*/

/** @} End of group SIMG301_HYDRARAM_BitFields */
/** @} End of group SIMG301_HYDRARAM */
/** @} End of group Parts */

#endif // SIMG301_HYDRARAM_H
