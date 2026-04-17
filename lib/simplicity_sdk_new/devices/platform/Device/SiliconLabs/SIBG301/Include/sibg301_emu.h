/**************************************************************************//**
 * @file
 * @brief SIBG301 EMU register and bit field definitions
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
#ifndef SIBG301_EMU_H
#define SIBG301_EMU_H
#define EMU_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_EMU EMU
 * @{
 * @brief SIBG301 EMU Register Declaration.
 *****************************************************************************/

/** EMU Register Declaration. */
typedef struct emu_typedef{
  uint32_t       RESERVED0[4U];                 /**< Reserved for future use                            */
  __IOM uint32_t DECBOD;                        /**< DECOUPLE LVBOD  Control register                   */
  uint32_t       RESERVED1[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t BOD3SENSE;                     /**< BOD3SENSE Control register                         */
  uint32_t       RESERVED2[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t FLREG;                         /**< FLREG Control register                             */
  uint32_t       RESERVED3[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t FLBOD;                         /**< FLBOD control register                             */
  uint32_t       RESERVED4[3U];                 /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION;                     /**< IP Version                                         */
  __IOM uint32_t LOCK;                          /**< EMU Configuration lock register                    */
  __IOM uint32_t IF;                            /**< Interrupt Flags                                    */
  __IOM uint32_t IEN;                           /**< Interrupt Enables                                  */
  __IOM uint32_t EM4CTRL;                       /**< EM4 Control                                        */
  __IOM uint32_t CMD;                           /**< EMU Command register                               */
  __IOM uint32_t CTRL;                          /**< EMU Control register                               */
  __IOM uint32_t TEMPLIMITS;                    /**< EMU Temperature thresholds                         */
  uint32_t       RESERVED5[2U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS;                        /**< EMU Status register                                */
  __IM uint32_t  TEMP;                          /**< Temperature                                        */
  uint32_t       RESERVED6[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t RSTCTRL;                       /**< Reset Management Control register                  */
  __IM uint32_t  RSTCAUSE;                      /**< Reset cause                                        */
  uint32_t       RESERVED7[10U];                /**< Reserved for future use                            */
  uint32_t       RESERVED8[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED9[27U];                /**< Reserved for future use                            */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED11[3U];                /**< Reserved for future use                            */
  uint32_t       RESERVED12[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED13[18U];               /**< Reserved for future use                            */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED15[924U];              /**< Reserved for future use                            */
  uint32_t       RESERVED16[4U];                /**< Reserved for future use                            */
  __IOM uint32_t DECBOD_SET;                    /**< DECOUPLE LVBOD  Control register                   */
  uint32_t       RESERVED17[3U];                /**< Reserved for future use                            */
  __IOM uint32_t BOD3SENSE_SET;                 /**< BOD3SENSE Control register                         */
  uint32_t       RESERVED18[7U];                /**< Reserved for future use                            */
  __IOM uint32_t FLREG_SET;                     /**< FLREG Control register                             */
  uint32_t       RESERVED19[2U];                /**< Reserved for future use                            */
  __IOM uint32_t FLBOD_SET;                     /**< FLBOD control register                             */
  uint32_t       RESERVED20[3U];                /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP Version                                         */
  __IOM uint32_t LOCK_SET;                      /**< EMU Configuration lock register                    */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enables                                  */
  __IOM uint32_t EM4CTRL_SET;                   /**< EM4 Control                                        */
  __IOM uint32_t CMD_SET;                       /**< EMU Command register                               */
  __IOM uint32_t CTRL_SET;                      /**< EMU Control register                               */
  __IOM uint32_t TEMPLIMITS_SET;                /**< EMU Temperature thresholds                         */
  uint32_t       RESERVED21[2U];                /**< Reserved for future use                            */
  __IM uint32_t  STATUS_SET;                    /**< EMU Status register                                */
  __IM uint32_t  TEMP_SET;                      /**< Temperature                                        */
  uint32_t       RESERVED22[1U];                /**< Reserved for future use                            */
  __IOM uint32_t RSTCTRL_SET;                   /**< Reset Management Control register                  */
  __IM uint32_t  RSTCAUSE_SET;                  /**< Reset cause                                        */
  uint32_t       RESERVED23[10U];               /**< Reserved for future use                            */
  uint32_t       RESERVED24[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED25[27U];               /**< Reserved for future use                            */
  uint32_t       RESERVED26[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED27[3U];                /**< Reserved for future use                            */
  uint32_t       RESERVED28[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED29[18U];               /**< Reserved for future use                            */
  uint32_t       RESERVED30[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED31[924U];              /**< Reserved for future use                            */
  uint32_t       RESERVED32[4U];                /**< Reserved for future use                            */
  __IOM uint32_t DECBOD_CLR;                    /**< DECOUPLE LVBOD  Control register                   */
  uint32_t       RESERVED33[3U];                /**< Reserved for future use                            */
  __IOM uint32_t BOD3SENSE_CLR;                 /**< BOD3SENSE Control register                         */
  uint32_t       RESERVED34[7U];                /**< Reserved for future use                            */
  __IOM uint32_t FLREG_CLR;                     /**< FLREG Control register                             */
  uint32_t       RESERVED35[2U];                /**< Reserved for future use                            */
  __IOM uint32_t FLBOD_CLR;                     /**< FLBOD control register                             */
  uint32_t       RESERVED36[3U];                /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP Version                                         */
  __IOM uint32_t LOCK_CLR;                      /**< EMU Configuration lock register                    */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enables                                  */
  __IOM uint32_t EM4CTRL_CLR;                   /**< EM4 Control                                        */
  __IOM uint32_t CMD_CLR;                       /**< EMU Command register                               */
  __IOM uint32_t CTRL_CLR;                      /**< EMU Control register                               */
  __IOM uint32_t TEMPLIMITS_CLR;                /**< EMU Temperature thresholds                         */
  uint32_t       RESERVED37[2U];                /**< Reserved for future use                            */
  __IM uint32_t  STATUS_CLR;                    /**< EMU Status register                                */
  __IM uint32_t  TEMP_CLR;                      /**< Temperature                                        */
  uint32_t       RESERVED38[1U];                /**< Reserved for future use                            */
  __IOM uint32_t RSTCTRL_CLR;                   /**< Reset Management Control register                  */
  __IM uint32_t  RSTCAUSE_CLR;                  /**< Reset cause                                        */
  uint32_t       RESERVED39[10U];               /**< Reserved for future use                            */
  uint32_t       RESERVED40[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED41[27U];               /**< Reserved for future use                            */
  uint32_t       RESERVED42[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED43[3U];                /**< Reserved for future use                            */
  uint32_t       RESERVED44[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED45[18U];               /**< Reserved for future use                            */
  uint32_t       RESERVED46[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED47[924U];              /**< Reserved for future use                            */
  uint32_t       RESERVED48[4U];                /**< Reserved for future use                            */
  __IOM uint32_t DECBOD_TGL;                    /**< DECOUPLE LVBOD  Control register                   */
  uint32_t       RESERVED49[3U];                /**< Reserved for future use                            */
  __IOM uint32_t BOD3SENSE_TGL;                 /**< BOD3SENSE Control register                         */
  uint32_t       RESERVED50[7U];                /**< Reserved for future use                            */
  __IOM uint32_t FLREG_TGL;                     /**< FLREG Control register                             */
  uint32_t       RESERVED51[2U];                /**< Reserved for future use                            */
  __IOM uint32_t FLBOD_TGL;                     /**< FLBOD control register                             */
  uint32_t       RESERVED52[3U];                /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP Version                                         */
  __IOM uint32_t LOCK_TGL;                      /**< EMU Configuration lock register                    */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enables                                  */
  __IOM uint32_t EM4CTRL_TGL;                   /**< EM4 Control                                        */
  __IOM uint32_t CMD_TGL;                       /**< EMU Command register                               */
  __IOM uint32_t CTRL_TGL;                      /**< EMU Control register                               */
  __IOM uint32_t TEMPLIMITS_TGL;                /**< EMU Temperature thresholds                         */
  uint32_t       RESERVED53[2U];                /**< Reserved for future use                            */
  __IM uint32_t  STATUS_TGL;                    /**< EMU Status register                                */
  __IM uint32_t  TEMP_TGL;                      /**< Temperature                                        */
  uint32_t       RESERVED54[1U];                /**< Reserved for future use                            */
  __IOM uint32_t RSTCTRL_TGL;                   /**< Reset Management Control register                  */
  __IM uint32_t  RSTCAUSE_TGL;                  /**< Reset cause                                        */
  uint32_t       RESERVED55[10U];               /**< Reserved for future use                            */
  uint32_t       RESERVED56[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED57[27U];               /**< Reserved for future use                            */
  uint32_t       RESERVED58[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED59[3U];                /**< Reserved for future use                            */
  uint32_t       RESERVED60[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED61[18U];               /**< Reserved for future use                            */
  uint32_t       RESERVED62[1U];                /**< Reserved for future use                            */
} EMU_TypeDef;
/** @} End of group SIBG301_EMU */

/**************************************************************************//**
 * @addtogroup SIBG301_EMU
 * @{
 * @defgroup SIBG301_EMU_BitFields EMU Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for EMU DECBOD */
#define _EMU_DECBOD_RESETVALUE                  0x00000022UL                             /**< Default value for EMU_DECBOD                */
#define _EMU_DECBOD_MASK                        0x00000033UL                             /**< Mask for EMU_DECBOD                         */
#define EMU_DECBOD_DECBODEN                     (0x1UL << 0)                             /**< DECBOD enable                               */
#define _EMU_DECBOD_DECBODEN_SHIFT              0                                        /**< Shift value for EMU_DECBODEN                */
#define _EMU_DECBOD_DECBODEN_MASK               0x1UL                                    /**< Bit mask for EMU_DECBODEN                   */
#define _EMU_DECBOD_DECBODEN_DEFAULT            0x00000000UL                             /**< Mode DEFAULT for EMU_DECBOD                 */
#define EMU_DECBOD_DECBODEN_DEFAULT             (_EMU_DECBOD_DECBODEN_DEFAULT << 0)      /**< Shifted mode DEFAULT for EMU_DECBOD         */
#define EMU_DECBOD_DECBODMASK                   (0x1UL << 1)                             /**< DECBOD Mask                                 */
#define _EMU_DECBOD_DECBODMASK_SHIFT            1                                        /**< Shift value for EMU_DECBODMASK              */
#define _EMU_DECBOD_DECBODMASK_MASK             0x2UL                                    /**< Bit mask for EMU_DECBODMASK                 */
#define _EMU_DECBOD_DECBODMASK_DEFAULT          0x00000001UL                             /**< Mode DEFAULT for EMU_DECBOD                 */
#define EMU_DECBOD_DECBODMASK_DEFAULT           (_EMU_DECBOD_DECBODMASK_DEFAULT << 1)    /**< Shifted mode DEFAULT for EMU_DECBOD         */
#define EMU_DECBOD_DECOVMBODEN                  (0x1UL << 4)                             /**< Over Voltage Monitor enable                 */
#define _EMU_DECBOD_DECOVMBODEN_SHIFT           4                                        /**< Shift value for EMU_DECOVMBODEN             */
#define _EMU_DECBOD_DECOVMBODEN_MASK            0x10UL                                   /**< Bit mask for EMU_DECOVMBODEN                */
#define _EMU_DECBOD_DECOVMBODEN_DEFAULT         0x00000000UL                             /**< Mode DEFAULT for EMU_DECBOD                 */
#define EMU_DECBOD_DECOVMBODEN_DEFAULT          (_EMU_DECBOD_DECOVMBODEN_DEFAULT << 4)   /**< Shifted mode DEFAULT for EMU_DECBOD         */
#define EMU_DECBOD_DECOVMBODMASK                (0x1UL << 5)                             /**< Over Voltage Monitor Mask                   */
#define _EMU_DECBOD_DECOVMBODMASK_SHIFT         5                                        /**< Shift value for EMU_DECOVMBODMASK           */
#define _EMU_DECBOD_DECOVMBODMASK_MASK          0x20UL                                   /**< Bit mask for EMU_DECOVMBODMASK              */
#define _EMU_DECBOD_DECOVMBODMASK_DEFAULT       0x00000001UL                             /**< Mode DEFAULT for EMU_DECBOD                 */
#define EMU_DECBOD_DECOVMBODMASK_DEFAULT        (_EMU_DECBOD_DECOVMBODMASK_DEFAULT << 5) /**< Shifted mode DEFAULT for EMU_DECBOD         */

/* Bit fields for EMU BOD3SENSE */
#define _EMU_BOD3SENSE_RESETVALUE               0x00000000UL                              /**< Default value for EMU_BOD3SENSE             */
#define _EMU_BOD3SENSE_MASK                     0x00000077UL                              /**< Mask for EMU_BOD3SENSE                      */
#define EMU_BOD3SENSE_AVDDBODEN                 (0x1UL << 0)                              /**< AVDD BOD enable                             */
#define _EMU_BOD3SENSE_AVDDBODEN_SHIFT          0                                         /**< Shift value for EMU_AVDDBODEN               */
#define _EMU_BOD3SENSE_AVDDBODEN_MASK           0x1UL                                     /**< Bit mask for EMU_AVDDBODEN                  */
#define _EMU_BOD3SENSE_AVDDBODEN_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for EMU_BOD3SENSE              */
#define EMU_BOD3SENSE_AVDDBODEN_DEFAULT         (_EMU_BOD3SENSE_AVDDBODEN_DEFAULT << 0)   /**< Shifted mode DEFAULT for EMU_BOD3SENSE      */
#define EMU_BOD3SENSE_VDDIO0BODEN               (0x1UL << 1)                              /**< VDDIO0 BOD enable                           */
#define _EMU_BOD3SENSE_VDDIO0BODEN_SHIFT        1                                         /**< Shift value for EMU_VDDIO0BODEN             */
#define _EMU_BOD3SENSE_VDDIO0BODEN_MASK         0x2UL                                     /**< Bit mask for EMU_VDDIO0BODEN                */
#define _EMU_BOD3SENSE_VDDIO0BODEN_DEFAULT      0x00000000UL                              /**< Mode DEFAULT for EMU_BOD3SENSE              */
#define EMU_BOD3SENSE_VDDIO0BODEN_DEFAULT       (_EMU_BOD3SENSE_VDDIO0BODEN_DEFAULT << 1) /**< Shifted mode DEFAULT for EMU_BOD3SENSE      */
#define EMU_BOD3SENSE_VDDIO1BODEN               (0x1UL << 2)                              /**< VDDIO1 BOD enable                           */
#define _EMU_BOD3SENSE_VDDIO1BODEN_SHIFT        2                                         /**< Shift value for EMU_VDDIO1BODEN             */
#define _EMU_BOD3SENSE_VDDIO1BODEN_MASK         0x4UL                                     /**< Bit mask for EMU_VDDIO1BODEN                */
#define _EMU_BOD3SENSE_VDDIO1BODEN_DEFAULT      0x00000000UL                              /**< Mode DEFAULT for EMU_BOD3SENSE              */
#define EMU_BOD3SENSE_VDDIO1BODEN_DEFAULT       (_EMU_BOD3SENSE_VDDIO1BODEN_DEFAULT << 2) /**< Shifted mode DEFAULT for EMU_BOD3SENSE      */

/* Bit fields for EMU FLREG */
#define _EMU_FLREG_RESETVALUE                   0x00030202UL                             /**< Default value for EMU_FLREG                 */
#define _EMU_FLREG_MASK                         0xF043DF9BUL                             /**< Mask for EMU_FLREG                          */
#define EMU_FLREG_EN                            (0x1UL << 0)                             /**< Enable FLREG                                */
#define _EMU_FLREG_EN_SHIFT                     0                                        /**< Shift value for EMU_EN                      */
#define _EMU_FLREG_EN_MASK                      0x1UL                                    /**< Bit mask for EMU_EN                         */
#define _EMU_FLREG_EN_DEFAULT                   0x00000000UL                             /**< Mode DEFAULT for EMU_FLREG                  */
#define EMU_FLREG_EN_DEFAULT                    (_EMU_FLREG_EN_DEFAULT << 0)             /**< Shifted mode DEFAULT for EMU_FLREG          */
#define EMU_FLREG_DISCLAMP                      (0x1UL << 1)                             /**< Disable soft pulldown                       */
#define _EMU_FLREG_DISCLAMP_SHIFT               1                                        /**< Shift value for EMU_DISCLAMP                */
#define _EMU_FLREG_DISCLAMP_MASK                0x2UL                                    /**< Bit mask for EMU_DISCLAMP                   */
#define _EMU_FLREG_DISCLAMP_DEFAULT             0x00000001UL                             /**< Mode DEFAULT for EMU_FLREG                  */
#define EMU_FLREG_DISCLAMP_DEFAULT              (_EMU_FLREG_DISCLAMP_DEFAULT << 1)       /**< Shifted mode DEFAULT for EMU_FLREG          */
#define EMU_FLREG_OVRENLP                       (0x1UL << 30)                            /**< Override Low Power Enable                   */
#define _EMU_FLREG_OVRENLP_SHIFT                30                                       /**< Shift value for EMU_OVRENLP                 */
#define _EMU_FLREG_OVRENLP_MASK                 0x40000000UL                             /**< Bit mask for EMU_OVRENLP                    */
#define _EMU_FLREG_OVRENLP_DEFAULT              0x00000000UL                             /**< Mode DEFAULT for EMU_FLREG                  */
#define EMU_FLREG_OVRENLP_DEFAULT               (_EMU_FLREG_OVRENLP_DEFAULT << 30)       /**< Shifted mode DEFAULT for EMU_FLREG          */
#define EMU_FLREG_OVROVERRIDEEN                 (0x1UL << 31)                            /**< Enable Override                             */
#define _EMU_FLREG_OVROVERRIDEEN_SHIFT          31                                       /**< Shift value for EMU_OVROVERRIDEEN           */
#define _EMU_FLREG_OVROVERRIDEEN_MASK           0x80000000UL                             /**< Bit mask for EMU_OVROVERRIDEEN              */
#define _EMU_FLREG_OVROVERRIDEEN_DEFAULT        0x00000000UL                             /**< Mode DEFAULT for EMU_FLREG                  */
#define EMU_FLREG_OVROVERRIDEEN_DEFAULT         (_EMU_FLREG_OVROVERRIDEEN_DEFAULT << 31) /**< Shifted mode DEFAULT for EMU_FLREG          */

/* Bit fields for EMU FLBOD */
#define _EMU_FLBOD_RESETVALUE                   0x00000106UL                            /**< Default value for EMU_FLBOD                 */
#define _EMU_FLBOD_MASK                         0x900307F7UL                            /**< Mask for EMU_FLBOD                          */
#define EMU_FLBOD_EN                            (0x1UL << 0)                            /**< FLBOD Enable                                */
#define _EMU_FLBOD_EN_SHIFT                     0                                       /**< Shift value for EMU_EN                      */
#define _EMU_FLBOD_EN_MASK                      0x1UL                                   /**< Bit mask for EMU_EN                         */
#define _EMU_FLBOD_EN_DEFAULT                   0x00000000UL                            /**< Mode DEFAULT for EMU_FLBOD                  */
#define EMU_FLBOD_EN_DEFAULT                    (_EMU_FLBOD_EN_DEFAULT << 0)            /**< Shifted mode DEFAULT for EMU_FLBOD          */
#define EMU_FLBOD_BODMASK                       (0x1UL << 1)                            /**< FLBOD Mask                                  */
#define _EMU_FLBOD_BODMASK_SHIFT                1                                       /**< Shift value for EMU_BODMASK                 */
#define _EMU_FLBOD_BODMASK_MASK                 0x2UL                                   /**< Bit mask for EMU_BODMASK                    */
#define _EMU_FLBOD_BODMASK_DEFAULT              0x00000001UL                            /**< Mode DEFAULT for EMU_FLBOD                  */
#define EMU_FLBOD_BODMASK_DEFAULT               (_EMU_FLBOD_BODMASK_DEFAULT << 1)       /**< Shifted mode DEFAULT for EMU_FLBOD          */

/* Bit fields for EMU IPVERSION */
#define _EMU_IPVERSION_RESETVALUE               0x00000009UL                            /**< Default value for EMU_IPVERSION             */
#define _EMU_IPVERSION_MASK                     0xFFFFFFFFUL                            /**< Mask for EMU_IPVERSION                      */
#define _EMU_IPVERSION_IPVERSION_SHIFT          0                                       /**< Shift value for EMU_IPVERSION               */
#define _EMU_IPVERSION_IPVERSION_MASK           0xFFFFFFFFUL                            /**< Bit mask for EMU_IPVERSION                  */
#define _EMU_IPVERSION_IPVERSION_DEFAULT        0x00000009UL                            /**< Mode DEFAULT for EMU_IPVERSION              */
#define EMU_IPVERSION_IPVERSION_DEFAULT         (_EMU_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for EMU_IPVERSION      */

/* Bit fields for EMU LOCK */
#define _EMU_LOCK_RESETVALUE                    0x0000ADE8UL                            /**< Default value for EMU_LOCK                  */
#define _EMU_LOCK_MASK                          0x0000FFFFUL                            /**< Mask for EMU_LOCK                           */
#define _EMU_LOCK_LOCKKEY_SHIFT                 0                                       /**< Shift value for EMU_LOCKKEY                 */
#define _EMU_LOCK_LOCKKEY_MASK                  0xFFFFUL                                /**< Bit mask for EMU_LOCKKEY                    */
#define _EMU_LOCK_LOCKKEY_DEFAULT               0x0000ADE8UL                            /**< Mode DEFAULT for EMU_LOCK                   */
#define _EMU_LOCK_LOCKKEY_UNLOCK                0x0000ADE8UL                            /**< Mode UNLOCK for EMU_LOCK                    */
#define EMU_LOCK_LOCKKEY_DEFAULT                (_EMU_LOCK_LOCKKEY_DEFAULT << 0)        /**< Shifted mode DEFAULT for EMU_LOCK           */
#define EMU_LOCK_LOCKKEY_UNLOCK                 (_EMU_LOCK_LOCKKEY_UNLOCK << 0)         /**< Shifted mode UNLOCK for EMU_LOCK            */

/* Bit fields for EMU IF */
#define _EMU_IF_RESETVALUE                      0x00000000UL                            /**< Default value for EMU_IF                    */
#define _EMU_IF_MASK                            0xEB070000UL                            /**< Mask for EMU_IF                             */
#define EMU_IF_AVDDBOD                          (0x1UL << 16)                           /**< AVDD BOD Interrupt flag                     */
#define _EMU_IF_AVDDBOD_SHIFT                   16                                      /**< Shift value for EMU_AVDDBOD                 */
#define _EMU_IF_AVDDBOD_MASK                    0x10000UL                               /**< Bit mask for EMU_AVDDBOD                    */
#define _EMU_IF_AVDDBOD_DEFAULT                 0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_AVDDBOD_DEFAULT                  (_EMU_IF_AVDDBOD_DEFAULT << 16)         /**< Shifted mode DEFAULT for EMU_IF             */
#define EMU_IF_IOVDD0BOD                        (0x1UL << 17)                           /**< VDDIO0 BOD Interrupt flag                   */
#define _EMU_IF_IOVDD0BOD_SHIFT                 17                                      /**< Shift value for EMU_IOVDD0BOD               */
#define _EMU_IF_IOVDD0BOD_MASK                  0x20000UL                               /**< Bit mask for EMU_IOVDD0BOD                  */
#define _EMU_IF_IOVDD0BOD_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_IOVDD0BOD_DEFAULT                (_EMU_IF_IOVDD0BOD_DEFAULT << 17)       /**< Shifted mode DEFAULT for EMU_IF             */
#define EMU_IF_IOVDD1BOD                        (0x1UL << 18)                           /**< VDDIO1 BOD Interrupt flag                   */
#define _EMU_IF_IOVDD1BOD_SHIFT                 18                                      /**< Shift value for EMU_IOVDD1BOD               */
#define _EMU_IF_IOVDD1BOD_MASK                  0x40000UL                               /**< Bit mask for EMU_IOVDD1BOD                  */
#define _EMU_IF_IOVDD1BOD_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_IOVDD1BOD_DEFAULT                (_EMU_IF_IOVDD1BOD_DEFAULT << 18)       /**< Shifted mode DEFAULT for EMU_IF             */
#define EMU_IF_TEMPAVG                          (0x1UL << 27)                           /**< Temperature Average Interrupt flag          */
#define _EMU_IF_TEMPAVG_SHIFT                   27                                      /**< Shift value for EMU_TEMPAVG                 */
#define _EMU_IF_TEMPAVG_MASK                    0x8000000UL                             /**< Bit mask for EMU_TEMPAVG                    */
#define _EMU_IF_TEMPAVG_DEFAULT                 0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_TEMPAVG_DEFAULT                  (_EMU_IF_TEMPAVG_DEFAULT << 27)         /**< Shifted mode DEFAULT for EMU_IF             */
#define EMU_IF_TEMP                             (0x1UL << 29)                           /**< Temperature Interrupt flag                  */
#define _EMU_IF_TEMP_SHIFT                      29                                      /**< Shift value for EMU_TEMP                    */
#define _EMU_IF_TEMP_MASK                       0x20000000UL                            /**< Bit mask for EMU_TEMP                       */
#define _EMU_IF_TEMP_DEFAULT                    0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_TEMP_DEFAULT                     (_EMU_IF_TEMP_DEFAULT << 29)            /**< Shifted mode DEFAULT for EMU_IF             */
#define EMU_IF_TEMPLOW                          (0x1UL << 30)                           /**< Temperature low Interrupt flag              */
#define _EMU_IF_TEMPLOW_SHIFT                   30                                      /**< Shift value for EMU_TEMPLOW                 */
#define _EMU_IF_TEMPLOW_MASK                    0x40000000UL                            /**< Bit mask for EMU_TEMPLOW                    */
#define _EMU_IF_TEMPLOW_DEFAULT                 0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_TEMPLOW_DEFAULT                  (_EMU_IF_TEMPLOW_DEFAULT << 30)         /**< Shifted mode DEFAULT for EMU_IF             */
#define EMU_IF_TEMPHIGH                         (0x1UL << 31)                           /**< Temperature high Interrupt flag             */
#define _EMU_IF_TEMPHIGH_SHIFT                  31                                      /**< Shift value for EMU_TEMPHIGH                */
#define _EMU_IF_TEMPHIGH_MASK                   0x80000000UL                            /**< Bit mask for EMU_TEMPHIGH                   */
#define _EMU_IF_TEMPHIGH_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for EMU_IF                     */
#define EMU_IF_TEMPHIGH_DEFAULT                 (_EMU_IF_TEMPHIGH_DEFAULT << 31)        /**< Shifted mode DEFAULT for EMU_IF             */

/* Bit fields for EMU IEN */
#define _EMU_IEN_RESETVALUE                     0x00000000UL                            /**< Default value for EMU_IEN                   */
#define _EMU_IEN_MASK                           0xEB070000UL                            /**< Mask for EMU_IEN                            */
#define EMU_IEN_AVDDBOD                         (0x1UL << 16)                           /**< AVDD BOD Interrupt enable                   */
#define _EMU_IEN_AVDDBOD_SHIFT                  16                                      /**< Shift value for EMU_AVDDBOD                 */
#define _EMU_IEN_AVDDBOD_MASK                   0x10000UL                               /**< Bit mask for EMU_AVDDBOD                    */
#define _EMU_IEN_AVDDBOD_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_AVDDBOD_DEFAULT                 (_EMU_IEN_AVDDBOD_DEFAULT << 16)        /**< Shifted mode DEFAULT for EMU_IEN            */
#define EMU_IEN_IOVDD0BOD                       (0x1UL << 17)                           /**< VDDIO0 BOD Interrupt enable                 */
#define _EMU_IEN_IOVDD0BOD_SHIFT                17                                      /**< Shift value for EMU_IOVDD0BOD               */
#define _EMU_IEN_IOVDD0BOD_MASK                 0x20000UL                               /**< Bit mask for EMU_IOVDD0BOD                  */
#define _EMU_IEN_IOVDD0BOD_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_IOVDD0BOD_DEFAULT               (_EMU_IEN_IOVDD0BOD_DEFAULT << 17)      /**< Shifted mode DEFAULT for EMU_IEN            */
#define EMU_IEN_IOVDD1BOD                       (0x1UL << 18)                           /**< VDDIO1 BOD Interrupt enable                 */
#define _EMU_IEN_IOVDD1BOD_SHIFT                18                                      /**< Shift value for EMU_IOVDD1BOD               */
#define _EMU_IEN_IOVDD1BOD_MASK                 0x40000UL                               /**< Bit mask for EMU_IOVDD1BOD                  */
#define _EMU_IEN_IOVDD1BOD_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_IOVDD1BOD_DEFAULT               (_EMU_IEN_IOVDD1BOD_DEFAULT << 18)      /**< Shifted mode DEFAULT for EMU_IEN            */
#define EMU_IEN_TEMPAVG                         (0x1UL << 27)                           /**< Temperature Interrupt enable                */
#define _EMU_IEN_TEMPAVG_SHIFT                  27                                      /**< Shift value for EMU_TEMPAVG                 */
#define _EMU_IEN_TEMPAVG_MASK                   0x8000000UL                             /**< Bit mask for EMU_TEMPAVG                    */
#define _EMU_IEN_TEMPAVG_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_TEMPAVG_DEFAULT                 (_EMU_IEN_TEMPAVG_DEFAULT << 27)        /**< Shifted mode DEFAULT for EMU_IEN            */
#define EMU_IEN_TEMP                            (0x1UL << 29)                           /**< Temperature Interrupt enable                */
#define _EMU_IEN_TEMP_SHIFT                     29                                      /**< Shift value for EMU_TEMP                    */
#define _EMU_IEN_TEMP_MASK                      0x20000000UL                            /**< Bit mask for EMU_TEMP                       */
#define _EMU_IEN_TEMP_DEFAULT                   0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_TEMP_DEFAULT                    (_EMU_IEN_TEMP_DEFAULT << 29)           /**< Shifted mode DEFAULT for EMU_IEN            */
#define EMU_IEN_TEMPLOW                         (0x1UL << 30)                           /**< Temperature low Interrupt enable            */
#define _EMU_IEN_TEMPLOW_SHIFT                  30                                      /**< Shift value for EMU_TEMPLOW                 */
#define _EMU_IEN_TEMPLOW_MASK                   0x40000000UL                            /**< Bit mask for EMU_TEMPLOW                    */
#define _EMU_IEN_TEMPLOW_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_TEMPLOW_DEFAULT                 (_EMU_IEN_TEMPLOW_DEFAULT << 30)        /**< Shifted mode DEFAULT for EMU_IEN            */
#define EMU_IEN_TEMPHIGH                        (0x1UL << 31)                           /**< Temperature high Interrupt enable           */
#define _EMU_IEN_TEMPHIGH_SHIFT                 31                                      /**< Shift value for EMU_TEMPHIGH                */
#define _EMU_IEN_TEMPHIGH_MASK                  0x80000000UL                            /**< Bit mask for EMU_TEMPHIGH                   */
#define _EMU_IEN_TEMPHIGH_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_IEN                    */
#define EMU_IEN_TEMPHIGH_DEFAULT                (_EMU_IEN_TEMPHIGH_DEFAULT << 31)       /**< Shifted mode DEFAULT for EMU_IEN            */

/* Bit fields for EMU EM4CTRL */
#define _EMU_EM4CTRL_RESETVALUE                 0x00000000UL                               /**< Default value for EMU_EM4CTRL               */
#define _EMU_EM4CTRL_MASK                       0x00000133UL                               /**< Mask for EMU_EM4CTRL                        */
#define _EMU_EM4CTRL_EM4ENTRY_SHIFT             0                                          /**< Shift value for EMU_EM4ENTRY                */
#define _EMU_EM4CTRL_EM4ENTRY_MASK              0x3UL                                      /**< Bit mask for EMU_EM4ENTRY                   */
#define _EMU_EM4CTRL_EM4ENTRY_DEFAULT           0x00000000UL                               /**< Mode DEFAULT for EMU_EM4CTRL                */
#define EMU_EM4CTRL_EM4ENTRY_DEFAULT            (_EMU_EM4CTRL_EM4ENTRY_DEFAULT << 0)       /**< Shifted mode DEFAULT for EMU_EM4CTRL        */
#define _EMU_EM4CTRL_EM4IORETMODE_SHIFT         4                                          /**< Shift value for EMU_EM4IORETMODE            */
#define _EMU_EM4CTRL_EM4IORETMODE_MASK          0x30UL                                     /**< Bit mask for EMU_EM4IORETMODE               */
#define _EMU_EM4CTRL_EM4IORETMODE_DEFAULT       0x00000000UL                               /**< Mode DEFAULT for EMU_EM4CTRL                */
#define _EMU_EM4CTRL_EM4IORETMODE_DISABLE       0x00000000UL                               /**< Mode DISABLE for EMU_EM4CTRL                */
#define _EMU_EM4CTRL_EM4IORETMODE_EM4EXIT       0x00000001UL                               /**< Mode EM4EXIT for EMU_EM4CTRL                */
#define _EMU_EM4CTRL_EM4IORETMODE_SWUNLATCH     0x00000002UL                               /**< Mode SWUNLATCH for EMU_EM4CTRL              */
#define EMU_EM4CTRL_EM4IORETMODE_DEFAULT        (_EMU_EM4CTRL_EM4IORETMODE_DEFAULT << 4)   /**< Shifted mode DEFAULT for EMU_EM4CTRL        */
#define EMU_EM4CTRL_EM4IORETMODE_DISABLE        (_EMU_EM4CTRL_EM4IORETMODE_DISABLE << 4)   /**< Shifted mode DISABLE for EMU_EM4CTRL        */
#define EMU_EM4CTRL_EM4IORETMODE_EM4EXIT        (_EMU_EM4CTRL_EM4IORETMODE_EM4EXIT << 4)   /**< Shifted mode EM4EXIT for EMU_EM4CTRL        */
#define EMU_EM4CTRL_EM4IORETMODE_SWUNLATCH      (_EMU_EM4CTRL_EM4IORETMODE_SWUNLATCH << 4) /**< Shifted mode SWUNLATCH for EMU_EM4CTRL      */
#define EMU_EM4CTRL_BOD3SENSEEM4WU              (0x1UL << 8)                               /**< Set BOD3SENSE as EM4 wakeup                 */
#define _EMU_EM4CTRL_BOD3SENSEEM4WU_SHIFT       8                                          /**< Shift value for EMU_BOD3SENSEEM4WU          */
#define _EMU_EM4CTRL_BOD3SENSEEM4WU_MASK        0x100UL                                    /**< Bit mask for EMU_BOD3SENSEEM4WU             */
#define _EMU_EM4CTRL_BOD3SENSEEM4WU_DEFAULT     0x00000000UL                               /**< Mode DEFAULT for EMU_EM4CTRL                */
#define EMU_EM4CTRL_BOD3SENSEEM4WU_DEFAULT      (_EMU_EM4CTRL_BOD3SENSEEM4WU_DEFAULT << 8) /**< Shifted mode DEFAULT for EMU_EM4CTRL        */

/* Bit fields for EMU CMD */
#define _EMU_CMD_RESETVALUE                     0x00000000UL                            /**< Default value for EMU_CMD                   */
#define _EMU_CMD_MASK                           0x00060812UL                            /**< Mask for EMU_CMD                            */
#define EMU_CMD_EM4UNLATCH                      (0x1UL << 1)                            /**< EM4 unlatch                                 */
#define _EMU_CMD_EM4UNLATCH_SHIFT               1                                       /**< Shift value for EMU_EM4UNLATCH              */
#define _EMU_CMD_EM4UNLATCH_MASK                0x2UL                                   /**< Bit mask for EMU_EM4UNLATCH                 */
#define _EMU_CMD_EM4UNLATCH_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for EMU_CMD                    */
#define EMU_CMD_EM4UNLATCH_DEFAULT              (_EMU_CMD_EM4UNLATCH_DEFAULT << 1)      /**< Shifted mode DEFAULT for EMU_CMD            */
#define EMU_CMD_TEMPAVGREQ                      (0x1UL << 4)                            /**< Temperature Average Request                 */
#define _EMU_CMD_TEMPAVGREQ_SHIFT               4                                       /**< Shift value for EMU_TEMPAVGREQ              */
#define _EMU_CMD_TEMPAVGREQ_MASK                0x10UL                                  /**< Bit mask for EMU_TEMPAVGREQ                 */
#define _EMU_CMD_TEMPAVGREQ_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for EMU_CMD                    */
#define EMU_CMD_TEMPAVGREQ_DEFAULT              (_EMU_CMD_TEMPAVGREQ_DEFAULT << 4)      /**< Shifted mode DEFAULT for EMU_CMD            */
#define EMU_CMD_RSTCAUSECLR                     (0x1UL << 17)                           /**< Reset Cause Clear                           */
#define _EMU_CMD_RSTCAUSECLR_SHIFT              17                                      /**< Shift value for EMU_RSTCAUSECLR             */
#define _EMU_CMD_RSTCAUSECLR_MASK               0x20000UL                               /**< Bit mask for EMU_RSTCAUSECLR                */
#define _EMU_CMD_RSTCAUSECLR_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for EMU_CMD                    */
#define EMU_CMD_RSTCAUSECLR_DEFAULT             (_EMU_CMD_RSTCAUSECLR_DEFAULT << 17)    /**< Shifted mode DEFAULT for EMU_CMD            */

/* Bit fields for EMU CTRL */
#define _EMU_CTRL_RESETVALUE                    0x00000000UL                                 /**< Default value for EMU_CTRL                  */
#define _EMU_CTRL_MASK                          0xE0810009UL                                 /**< Mask for EMU_CTRL                           */
#define EMU_CTRL_TEMPAVGNUM                     (0x1UL << 3)                                 /**< Averaged Temperature samples num            */
#define _EMU_CTRL_TEMPAVGNUM_SHIFT              3                                            /**< Shift value for EMU_TEMPAVGNUM              */
#define _EMU_CTRL_TEMPAVGNUM_MASK               0x8UL                                        /**< Bit mask for EMU_TEMPAVGNUM                 */
#define _EMU_CTRL_TEMPAVGNUM_DEFAULT            0x00000000UL                                 /**< Mode DEFAULT for EMU_CTRL                   */
#define _EMU_CTRL_TEMPAVGNUM_N16                0x00000000UL                                 /**< Mode N16 for EMU_CTRL                       */
#define _EMU_CTRL_TEMPAVGNUM_N64                0x00000001UL                                 /**< Mode N64 for EMU_CTRL                       */
#define EMU_CTRL_TEMPAVGNUM_DEFAULT             (_EMU_CTRL_TEMPAVGNUM_DEFAULT << 3)          /**< Shifted mode DEFAULT for EMU_CTRL           */
#define EMU_CTRL_TEMPAVGNUM_N16                 (_EMU_CTRL_TEMPAVGNUM_N16 << 3)              /**< Shifted mode N16 for EMU_CTRL               */
#define EMU_CTRL_TEMPAVGNUM_N64                 (_EMU_CTRL_TEMPAVGNUM_N64 << 3)              /**< Shifted mode N64 for EMU_CTRL               */
#define EMU_CTRL_FLASHPWRUPONDEMAND             (0x1UL << 16)                                /**< Enable flash on demand wakeup               */
#define _EMU_CTRL_FLASHPWRUPONDEMAND_SHIFT      16                                           /**< Shift value for EMU_FLASHPWRUPONDEMAND      */
#define _EMU_CTRL_FLASHPWRUPONDEMAND_MASK       0x10000UL                                    /**< Bit mask for EMU_FLASHPWRUPONDEMAND         */
#define _EMU_CTRL_FLASHPWRUPONDEMAND_DEFAULT    0x00000000UL                                 /**< Mode DEFAULT for EMU_CTRL                   */
#define EMU_CTRL_FLASHPWRUPONDEMAND_DEFAULT     (_EMU_CTRL_FLASHPWRUPONDEMAND_DEFAULT << 16) /**< Shifted mode DEFAULT for EMU_CTRL           */

/* Bit fields for EMU TEMPLIMITS */
#define _EMU_TEMPLIMITS_RESETVALUE              0x01FF0000UL                             /**< Default value for EMU_TEMPLIMITS            */
#define _EMU_TEMPLIMITS_MASK                    0x01FF01FFUL                             /**< Mask for EMU_TEMPLIMITS                     */
#define _EMU_TEMPLIMITS_TEMPLOW_SHIFT           0                                        /**< Shift value for EMU_TEMPLOW                 */
#define _EMU_TEMPLIMITS_TEMPLOW_MASK            0x1FFUL                                  /**< Bit mask for EMU_TEMPLOW                    */
#define _EMU_TEMPLIMITS_TEMPLOW_DEFAULT         0x00000000UL                             /**< Mode DEFAULT for EMU_TEMPLIMITS             */
#define EMU_TEMPLIMITS_TEMPLOW_DEFAULT          (_EMU_TEMPLIMITS_TEMPLOW_DEFAULT << 0)   /**< Shifted mode DEFAULT for EMU_TEMPLIMITS     */
#define _EMU_TEMPLIMITS_TEMPHIGH_SHIFT          16                                       /**< Shift value for EMU_TEMPHIGH                */
#define _EMU_TEMPLIMITS_TEMPHIGH_MASK           0x1FF0000UL                              /**< Bit mask for EMU_TEMPHIGH                   */
#define _EMU_TEMPLIMITS_TEMPHIGH_DEFAULT        0x000001FFUL                             /**< Mode DEFAULT for EMU_TEMPLIMITS             */
#define EMU_TEMPLIMITS_TEMPHIGH_DEFAULT         (_EMU_TEMPLIMITS_TEMPHIGH_DEFAULT << 16) /**< Shifted mode DEFAULT for EMU_TEMPLIMITS     */

/* Bit fields for EMU STATUS */
#define _EMU_STATUS_RESETVALUE                  0x00000080UL                             /**< Default value for EMU_STATUS                */
#define _EMU_STATUS_MASK                        0x00000FFFUL                             /**< Mask for EMU_STATUS                         */
#define EMU_STATUS_LOCK                         (0x1UL << 0)                             /**< Lock status                                 */
#define _EMU_STATUS_LOCK_SHIFT                  0                                        /**< Shift value for EMU_LOCK                    */
#define _EMU_STATUS_LOCK_MASK                   0x1UL                                    /**< Bit mask for EMU_LOCK                       */
#define _EMU_STATUS_LOCK_DEFAULT                0x00000000UL                             /**< Mode DEFAULT for EMU_STATUS                 */
#define _EMU_STATUS_LOCK_UNLOCKED               0x00000000UL                             /**< Mode UNLOCKED for EMU_STATUS                */
#define _EMU_STATUS_LOCK_LOCKED                 0x00000001UL                             /**< Mode LOCKED for EMU_STATUS                  */
#define EMU_STATUS_LOCK_DEFAULT                 (_EMU_STATUS_LOCK_DEFAULT << 0)          /**< Shifted mode DEFAULT for EMU_STATUS         */
#define EMU_STATUS_LOCK_UNLOCKED                (_EMU_STATUS_LOCK_UNLOCKED << 0)         /**< Shifted mode UNLOCKED for EMU_STATUS        */
#define EMU_STATUS_LOCK_LOCKED                  (_EMU_STATUS_LOCK_LOCKED << 0)           /**< Shifted mode LOCKED for EMU_STATUS          */
#define EMU_STATUS_FIRSTTEMPDONE                (0x1UL << 1)                             /**< First Temp done                             */
#define _EMU_STATUS_FIRSTTEMPDONE_SHIFT         1                                        /**< Shift value for EMU_FIRSTTEMPDONE           */
#define _EMU_STATUS_FIRSTTEMPDONE_MASK          0x2UL                                    /**< Bit mask for EMU_FIRSTTEMPDONE              */
#define _EMU_STATUS_FIRSTTEMPDONE_DEFAULT       0x00000000UL                             /**< Mode DEFAULT for EMU_STATUS                 */
#define EMU_STATUS_FIRSTTEMPDONE_DEFAULT        (_EMU_STATUS_FIRSTTEMPDONE_DEFAULT << 1) /**< Shifted mode DEFAULT for EMU_STATUS         */
#define EMU_STATUS_TEMPACTIVE                   (0x1UL << 2)                             /**< Temp active                                 */
#define _EMU_STATUS_TEMPACTIVE_SHIFT            2                                        /**< Shift value for EMU_TEMPACTIVE              */
#define _EMU_STATUS_TEMPACTIVE_MASK             0x4UL                                    /**< Bit mask for EMU_TEMPACTIVE                 */
#define _EMU_STATUS_TEMPACTIVE_DEFAULT          0x00000000UL                             /**< Mode DEFAULT for EMU_STATUS                 */
#define EMU_STATUS_TEMPACTIVE_DEFAULT           (_EMU_STATUS_TEMPACTIVE_DEFAULT << 2)    /**< Shifted mode DEFAULT for EMU_STATUS         */
#define EMU_STATUS_TEMPAVGACTIVE                (0x1UL << 3)                             /**< Temp Average active                         */
#define _EMU_STATUS_TEMPAVGACTIVE_SHIFT         3                                        /**< Shift value for EMU_TEMPAVGACTIVE           */
#define _EMU_STATUS_TEMPAVGACTIVE_MASK          0x8UL                                    /**< Bit mask for EMU_TEMPAVGACTIVE              */
#define _EMU_STATUS_TEMPAVGACTIVE_DEFAULT       0x00000000UL                             /**< Mode DEFAULT for EMU_STATUS                 */
#define EMU_STATUS_TEMPAVGACTIVE_DEFAULT        (_EMU_STATUS_TEMPAVGACTIVE_DEFAULT << 3) /**< Shifted mode DEFAULT for EMU_STATUS         */
#define EMU_STATUS_RACACTIVE                    (0x1UL << 8)                             /**< RAC active                                  */
#define _EMU_STATUS_RACACTIVE_SHIFT             8                                        /**< Shift value for EMU_RACACTIVE               */
#define _EMU_STATUS_RACACTIVE_MASK              0x100UL                                  /**< Bit mask for EMU_RACACTIVE                  */
#define _EMU_STATUS_RACACTIVE_DEFAULT           0x00000000UL                             /**< Mode DEFAULT for EMU_STATUS                 */
#define EMU_STATUS_RACACTIVE_DEFAULT            (_EMU_STATUS_RACACTIVE_DEFAULT << 8)     /**< Shifted mode DEFAULT for EMU_STATUS         */
#define EMU_STATUS_EM4IORET                     (0x1UL << 9)                             /**< EM4 IO retention status                     */
#define _EMU_STATUS_EM4IORET_SHIFT              9                                        /**< Shift value for EMU_EM4IORET                */
#define _EMU_STATUS_EM4IORET_MASK               0x200UL                                  /**< Bit mask for EMU_EM4IORET                   */
#define _EMU_STATUS_EM4IORET_DEFAULT            0x00000000UL                             /**< Mode DEFAULT for EMU_STATUS                 */
#define EMU_STATUS_EM4IORET_DEFAULT             (_EMU_STATUS_EM4IORET_DEFAULT << 9)      /**< Shifted mode DEFAULT for EMU_STATUS         */

/* Bit fields for EMU TEMP */
#define _EMU_TEMP_RESETVALUE                    0x00000000UL                            /**< Default value for EMU_TEMP                  */
#define _EMU_TEMP_MASK                          0x07FF07FFUL                            /**< Mask for EMU_TEMP                           */
#define _EMU_TEMP_TEMPLSB_SHIFT                 0                                       /**< Shift value for EMU_TEMPLSB                 */
#define _EMU_TEMP_TEMPLSB_MASK                  0x3UL                                   /**< Bit mask for EMU_TEMPLSB                    */
#define _EMU_TEMP_TEMPLSB_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_TEMP                   */
#define EMU_TEMP_TEMPLSB_DEFAULT                (_EMU_TEMP_TEMPLSB_DEFAULT << 0)        /**< Shifted mode DEFAULT for EMU_TEMP           */
#define _EMU_TEMP_TEMP_SHIFT                    2                                       /**< Shift value for EMU_TEMP                    */
#define _EMU_TEMP_TEMP_MASK                     0x7FCUL                                 /**< Bit mask for EMU_TEMP                       */
#define _EMU_TEMP_TEMP_DEFAULT                  0x00000000UL                            /**< Mode DEFAULT for EMU_TEMP                   */
#define EMU_TEMP_TEMP_DEFAULT                   (_EMU_TEMP_TEMP_DEFAULT << 2)           /**< Shifted mode DEFAULT for EMU_TEMP           */
#define _EMU_TEMP_TEMPAVG_SHIFT                 16                                      /**< Shift value for EMU_TEMPAVG                 */
#define _EMU_TEMP_TEMPAVG_MASK                  0x7FF0000UL                             /**< Bit mask for EMU_TEMPAVG                    */
#define _EMU_TEMP_TEMPAVG_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_TEMP                   */
#define EMU_TEMP_TEMPAVG_DEFAULT                (_EMU_TEMP_TEMPAVG_DEFAULT << 16)       /**< Shifted mode DEFAULT for EMU_TEMP           */

/* Bit fields for EMU RSTCTRL */
#define _EMU_RSTCTRL_RESETVALUE                 0x00060407UL                                /**< Default value for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_MASK                       0xC006CDCFUL                                /**< Mask for EMU_RSTCTRL                        */
#define EMU_RSTCTRL_WDOG0RMODE                  (0x1UL << 0)                                /**< Enable WDOG0 reset                          */
#define _EMU_RSTCTRL_WDOG0RMODE_SHIFT           0                                           /**< Shift value for EMU_WDOG0RMODE              */
#define _EMU_RSTCTRL_WDOG0RMODE_MASK            0x1UL                                       /**< Bit mask for EMU_WDOG0RMODE                 */
#define _EMU_RSTCTRL_WDOG0RMODE_DEFAULT         0x00000001UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_WDOG0RMODE_DISABLED        0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_WDOG0RMODE_ENABLED         0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_WDOG0RMODE_DEFAULT          (_EMU_RSTCTRL_WDOG0RMODE_DEFAULT << 0)      /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_WDOG0RMODE_DISABLED         (_EMU_RSTCTRL_WDOG0RMODE_DISABLED << 0)     /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_WDOG0RMODE_ENABLED          (_EMU_RSTCTRL_WDOG0RMODE_ENABLED << 0)      /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_WDOG1RMODE                  (0x1UL << 1)                                /**< Enable WDOG1 reset                          */
#define _EMU_RSTCTRL_WDOG1RMODE_SHIFT           1                                           /**< Shift value for EMU_WDOG1RMODE              */
#define _EMU_RSTCTRL_WDOG1RMODE_MASK            0x2UL                                       /**< Bit mask for EMU_WDOG1RMODE                 */
#define _EMU_RSTCTRL_WDOG1RMODE_DEFAULT         0x00000001UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_WDOG1RMODE_DISABLED        0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_WDOG1RMODE_ENABLED         0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_WDOG1RMODE_DEFAULT          (_EMU_RSTCTRL_WDOG1RMODE_DEFAULT << 1)      /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_WDOG1RMODE_DISABLED         (_EMU_RSTCTRL_WDOG1RMODE_DISABLED << 1)     /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_WDOG1RMODE_ENABLED          (_EMU_RSTCTRL_WDOG1RMODE_ENABLED << 1)      /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_SYSRMODE                    (0x1UL << 2)                                /**< Enable M33 System reset                     */
#define _EMU_RSTCTRL_SYSRMODE_SHIFT             2                                           /**< Shift value for EMU_SYSRMODE                */
#define _EMU_RSTCTRL_SYSRMODE_MASK              0x4UL                                       /**< Bit mask for EMU_SYSRMODE                   */
#define _EMU_RSTCTRL_SYSRMODE_DEFAULT           0x00000001UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_SYSRMODE_DISABLED          0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_SYSRMODE_ENABLED           0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_SYSRMODE_DEFAULT            (_EMU_RSTCTRL_SYSRMODE_DEFAULT << 2)        /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_SYSRMODE_DISABLED           (_EMU_RSTCTRL_SYSRMODE_DISABLED << 2)       /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_SYSRMODE_ENABLED            (_EMU_RSTCTRL_SYSRMODE_ENABLED << 2)        /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_LOCKUPRMODE                 (0x1UL << 3)                                /**< Enable M33 Lockup reset                     */
#define _EMU_RSTCTRL_LOCKUPRMODE_SHIFT          3                                           /**< Shift value for EMU_LOCKUPRMODE             */
#define _EMU_RSTCTRL_LOCKUPRMODE_MASK           0x8UL                                       /**< Bit mask for EMU_LOCKUPRMODE                */
#define _EMU_RSTCTRL_LOCKUPRMODE_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_LOCKUPRMODE_DISABLED       0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_LOCKUPRMODE_ENABLED        0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_LOCKUPRMODE_DEFAULT         (_EMU_RSTCTRL_LOCKUPRMODE_DEFAULT << 3)     /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_LOCKUPRMODE_DISABLED        (_EMU_RSTCTRL_LOCKUPRMODE_DISABLED << 3)    /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_LOCKUPRMODE_ENABLED         (_EMU_RSTCTRL_LOCKUPRMODE_ENABLED << 3)     /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_AVDDBODRMODE                (0x1UL << 6)                                /**< Enable AVDD BOD reset                       */
#define _EMU_RSTCTRL_AVDDBODRMODE_SHIFT         6                                           /**< Shift value for EMU_AVDDBODRMODE            */
#define _EMU_RSTCTRL_AVDDBODRMODE_MASK          0x40UL                                      /**< Bit mask for EMU_AVDDBODRMODE               */
#define _EMU_RSTCTRL_AVDDBODRMODE_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_AVDDBODRMODE_DISABLED      0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_AVDDBODRMODE_ENABLED       0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_AVDDBODRMODE_DEFAULT        (_EMU_RSTCTRL_AVDDBODRMODE_DEFAULT << 6)    /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_AVDDBODRMODE_DISABLED       (_EMU_RSTCTRL_AVDDBODRMODE_DISABLED << 6)   /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_AVDDBODRMODE_ENABLED        (_EMU_RSTCTRL_AVDDBODRMODE_ENABLED << 6)    /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_IOVDD0BODRMODE              (0x1UL << 7)                                /**< Enable VDDIO0 BOD reset                     */
#define _EMU_RSTCTRL_IOVDD0BODRMODE_SHIFT       7                                           /**< Shift value for EMU_IOVDD0BODRMODE          */
#define _EMU_RSTCTRL_IOVDD0BODRMODE_MASK        0x80UL                                      /**< Bit mask for EMU_IOVDD0BODRMODE             */
#define _EMU_RSTCTRL_IOVDD0BODRMODE_DEFAULT     0x00000000UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_IOVDD0BODRMODE_DISABLED    0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_IOVDD0BODRMODE_ENABLED     0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_IOVDD0BODRMODE_DEFAULT      (_EMU_RSTCTRL_IOVDD0BODRMODE_DEFAULT << 7)  /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_IOVDD0BODRMODE_DISABLED     (_EMU_RSTCTRL_IOVDD0BODRMODE_DISABLED << 7) /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_IOVDD0BODRMODE_ENABLED      (_EMU_RSTCTRL_IOVDD0BODRMODE_ENABLED << 7)  /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_IOVDD1BODRMODE              (0x1UL << 8)                                /**< Enable VDDIO1 BOD reset                     */
#define _EMU_RSTCTRL_IOVDD1BODRMODE_SHIFT       8                                           /**< Shift value for EMU_IOVDD1BODRMODE          */
#define _EMU_RSTCTRL_IOVDD1BODRMODE_MASK        0x100UL                                     /**< Bit mask for EMU_IOVDD1BODRMODE             */
#define _EMU_RSTCTRL_IOVDD1BODRMODE_DEFAULT     0x00000000UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_IOVDD1BODRMODE_DISABLED    0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_IOVDD1BODRMODE_ENABLED     0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_IOVDD1BODRMODE_DEFAULT      (_EMU_RSTCTRL_IOVDD1BODRMODE_DEFAULT << 8)  /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_IOVDD1BODRMODE_DISABLED     (_EMU_RSTCTRL_IOVDD1BODRMODE_DISABLED << 8) /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_IOVDD1BODRMODE_ENABLED      (_EMU_RSTCTRL_IOVDD1BODRMODE_ENABLED << 8)  /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_DECBODRMODE                 (0x1UL << 10)                               /**< Enable DECBOD reset                         */
#define _EMU_RSTCTRL_DECBODRMODE_SHIFT          10                                          /**< Shift value for EMU_DECBODRMODE             */
#define _EMU_RSTCTRL_DECBODRMODE_MASK           0x400UL                                     /**< Bit mask for EMU_DECBODRMODE                */
#define _EMU_RSTCTRL_DECBODRMODE_DEFAULT        0x00000001UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_DECBODRMODE_DISABLED       0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_DECBODRMODE_ENABLED        0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_DECBODRMODE_DEFAULT         (_EMU_RSTCTRL_DECBODRMODE_DEFAULT << 10)    /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_DECBODRMODE_DISABLED        (_EMU_RSTCTRL_DECBODRMODE_DISABLED << 10)   /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_DECBODRMODE_ENABLED         (_EMU_RSTCTRL_DECBODRMODE_ENABLED << 10)    /**< Shifted mode ENABLED for EMU_RSTCTRL        */
#define EMU_RSTCTRL_FLBODRMODE                  (0x1UL << 11)                               /**< Enable FLBOD reset                          */
#define _EMU_RSTCTRL_FLBODRMODE_SHIFT           11                                          /**< Shift value for EMU_FLBODRMODE              */
#define _EMU_RSTCTRL_FLBODRMODE_MASK            0x800UL                                     /**< Bit mask for EMU_FLBODRMODE                 */
#define _EMU_RSTCTRL_FLBODRMODE_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for EMU_RSTCTRL                */
#define _EMU_RSTCTRL_FLBODRMODE_DISABLED        0x00000000UL                                /**< Mode DISABLED for EMU_RSTCTRL               */
#define _EMU_RSTCTRL_FLBODRMODE_ENABLED         0x00000001UL                                /**< Mode ENABLED for EMU_RSTCTRL                */
#define EMU_RSTCTRL_FLBODRMODE_DEFAULT          (_EMU_RSTCTRL_FLBODRMODE_DEFAULT << 11)     /**< Shifted mode DEFAULT for EMU_RSTCTRL        */
#define EMU_RSTCTRL_FLBODRMODE_DISABLED         (_EMU_RSTCTRL_FLBODRMODE_DISABLED << 11)    /**< Shifted mode DISABLED for EMU_RSTCTRL       */
#define EMU_RSTCTRL_FLBODRMODE_ENABLED          (_EMU_RSTCTRL_FLBODRMODE_ENABLED << 11)     /**< Shifted mode ENABLED for EMU_RSTCTRL        */

/* Bit fields for EMU RSTCAUSE */
#define _EMU_RSTCAUSE_RESETVALUE                0x00000000UL                            /**< Default value for EMU_RSTCAUSE              */
#define _EMU_RSTCAUSE_MASK                      0x0007FFFFUL                            /**< Mask for EMU_RSTCAUSE                       */
#define EMU_RSTCAUSE_POR                        (0x1UL << 0)                            /**< Power On Reset                              */
#define _EMU_RSTCAUSE_POR_SHIFT                 0                                       /**< Shift value for EMU_POR                     */
#define _EMU_RSTCAUSE_POR_MASK                  0x1UL                                   /**< Bit mask for EMU_POR                        */
#define _EMU_RSTCAUSE_POR_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_POR_DEFAULT                (_EMU_RSTCAUSE_POR_DEFAULT << 0)        /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_PIN                        (0x1UL << 1)                            /**< Pin Reset                                   */
#define _EMU_RSTCAUSE_PIN_SHIFT                 1                                       /**< Shift value for EMU_PIN                     */
#define _EMU_RSTCAUSE_PIN_MASK                  0x2UL                                   /**< Bit mask for EMU_PIN                        */
#define _EMU_RSTCAUSE_PIN_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_PIN_DEFAULT                (_EMU_RSTCAUSE_PIN_DEFAULT << 1)        /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_EM4                        (0x1UL << 2)                            /**< EM4 Wakeup Reset                            */
#define _EMU_RSTCAUSE_EM4_SHIFT                 2                                       /**< Shift value for EMU_EM4                     */
#define _EMU_RSTCAUSE_EM4_MASK                  0x4UL                                   /**< Bit mask for EMU_EM4                        */
#define _EMU_RSTCAUSE_EM4_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_EM4_DEFAULT                (_EMU_RSTCAUSE_EM4_DEFAULT << 2)        /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_WDOG0                      (0x1UL << 3)                            /**< Watchdog 0 Reset                            */
#define _EMU_RSTCAUSE_WDOG0_SHIFT               3                                       /**< Shift value for EMU_WDOG0                   */
#define _EMU_RSTCAUSE_WDOG0_MASK                0x8UL                                   /**< Bit mask for EMU_WDOG0                      */
#define _EMU_RSTCAUSE_WDOG0_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_WDOG0_DEFAULT              (_EMU_RSTCAUSE_WDOG0_DEFAULT << 3)      /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_WDOG1                      (0x1UL << 4)                            /**< Watchdog 1 Reset                            */
#define _EMU_RSTCAUSE_WDOG1_SHIFT               4                                       /**< Shift value for EMU_WDOG1                   */
#define _EMU_RSTCAUSE_WDOG1_MASK                0x10UL                                  /**< Bit mask for EMU_WDOG1                      */
#define _EMU_RSTCAUSE_WDOG1_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_WDOG1_DEFAULT              (_EMU_RSTCAUSE_WDOG1_DEFAULT << 4)      /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_LOCKUP                     (0x1UL << 5)                            /**< M33 Core Lockup Reset                       */
#define _EMU_RSTCAUSE_LOCKUP_SHIFT              5                                       /**< Shift value for EMU_LOCKUP                  */
#define _EMU_RSTCAUSE_LOCKUP_MASK               0x20UL                                  /**< Bit mask for EMU_LOCKUP                     */
#define _EMU_RSTCAUSE_LOCKUP_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_LOCKUP_DEFAULT             (_EMU_RSTCAUSE_LOCKUP_DEFAULT << 5)     /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_SYSREQ                     (0x1UL << 6)                            /**< M33 Core Sys Reset                          */
#define _EMU_RSTCAUSE_SYSREQ_SHIFT              6                                       /**< Shift value for EMU_SYSREQ                  */
#define _EMU_RSTCAUSE_SYSREQ_MASK               0x40UL                                  /**< Bit mask for EMU_SYSREQ                     */
#define _EMU_RSTCAUSE_SYSREQ_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_SYSREQ_DEFAULT             (_EMU_RSTCAUSE_SYSREQ_DEFAULT << 6)     /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_DVDDBOD                    (0x1UL << 7)                            /**< HVBOD Reset                                 */
#define _EMU_RSTCAUSE_DVDDBOD_SHIFT             7                                       /**< Shift value for EMU_DVDDBOD                 */
#define _EMU_RSTCAUSE_DVDDBOD_MASK              0x80UL                                  /**< Bit mask for EMU_DVDDBOD                    */
#define _EMU_RSTCAUSE_DVDDBOD_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_DVDDBOD_DEFAULT            (_EMU_RSTCAUSE_DVDDBOD_DEFAULT << 7)    /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_DVDDLEBOD                  (0x1UL << 8)                            /**< LEBOD Reset                                 */
#define _EMU_RSTCAUSE_DVDDLEBOD_SHIFT           8                                       /**< Shift value for EMU_DVDDLEBOD               */
#define _EMU_RSTCAUSE_DVDDLEBOD_MASK            0x100UL                                 /**< Bit mask for EMU_DVDDLEBOD                  */
#define _EMU_RSTCAUSE_DVDDLEBOD_DEFAULT         0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_DVDDLEBOD_DEFAULT          (_EMU_RSTCAUSE_DVDDLEBOD_DEFAULT << 8)  /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_DECBOD                     (0x1UL << 9)                            /**< LVBOD Reset                                 */
#define _EMU_RSTCAUSE_DECBOD_SHIFT              9                                       /**< Shift value for EMU_DECBOD                  */
#define _EMU_RSTCAUSE_DECBOD_MASK               0x200UL                                 /**< Bit mask for EMU_DECBOD                     */
#define _EMU_RSTCAUSE_DECBOD_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_DECBOD_DEFAULT             (_EMU_RSTCAUSE_DECBOD_DEFAULT << 9)     /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_AVDDBOD                    (0x1UL << 10)                           /**< LEBOD1 Reset                                */
#define _EMU_RSTCAUSE_AVDDBOD_SHIFT             10                                      /**< Shift value for EMU_AVDDBOD                 */
#define _EMU_RSTCAUSE_AVDDBOD_MASK              0x400UL                                 /**< Bit mask for EMU_AVDDBOD                    */
#define _EMU_RSTCAUSE_AVDDBOD_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_AVDDBOD_DEFAULT            (_EMU_RSTCAUSE_AVDDBOD_DEFAULT << 10)   /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_IOVDD0BOD                  (0x1UL << 11)                           /**< LEBOD2 Reset                                */
#define _EMU_RSTCAUSE_IOVDD0BOD_SHIFT           11                                      /**< Shift value for EMU_IOVDD0BOD               */
#define _EMU_RSTCAUSE_IOVDD0BOD_MASK            0x800UL                                 /**< Bit mask for EMU_IOVDD0BOD                  */
#define _EMU_RSTCAUSE_IOVDD0BOD_DEFAULT         0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_IOVDD0BOD_DEFAULT          (_EMU_RSTCAUSE_IOVDD0BOD_DEFAULT << 11) /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_IOVDD1BOD                  (0x1UL << 12)                           /**< LEBOD3 Reset                                */
#define _EMU_RSTCAUSE_IOVDD1BOD_SHIFT           12                                      /**< Shift value for EMU_IOVDD1BOD               */
#define _EMU_RSTCAUSE_IOVDD1BOD_MASK            0x1000UL                                /**< Bit mask for EMU_IOVDD1BOD                  */
#define _EMU_RSTCAUSE_IOVDD1BOD_DEFAULT         0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_IOVDD1BOD_DEFAULT          (_EMU_RSTCAUSE_IOVDD1BOD_DEFAULT << 12) /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_FLBOD                      (0x1UL << 13)                           /**< FLBOD Reset                                 */
#define _EMU_RSTCAUSE_FLBOD_SHIFT               13                                      /**< Shift value for EMU_FLBOD                   */
#define _EMU_RSTCAUSE_FLBOD_MASK                0x2000UL                                /**< Bit mask for EMU_FLBOD                      */
#define _EMU_RSTCAUSE_FLBOD_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_FLBOD_DEFAULT              (_EMU_RSTCAUSE_FLBOD_DEFAULT << 13)     /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */
#define EMU_RSTCAUSE_SETAMPER                   (0x1UL << 14)                           /**< SE Tamper event Reset                       */
#define _EMU_RSTCAUSE_SETAMPER_SHIFT            14                                      /**< Shift value for EMU_SETAMPER                */
#define _EMU_RSTCAUSE_SETAMPER_MASK             0x4000UL                                /**< Bit mask for EMU_SETAMPER                   */
#define _EMU_RSTCAUSE_SETAMPER_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for EMU_RSTCAUSE               */
#define EMU_RSTCAUSE_SETAMPER_DEFAULT           (_EMU_RSTCAUSE_SETAMPER_DEFAULT << 14)  /**< Shifted mode DEFAULT for EMU_RSTCAUSE       */

/** @} End of group SIBG301_EMU_BitFields */
/** @} End of group SIBG301_EMU */
/**************************************************************************//**
 * @defgroup SIBG301_EMU_CFGNS EMU_CFGNS
 * @{
 * @brief SIBG301 EMU_CFGNS Register Declaration.
 *****************************************************************************/

/** EMU_CFGNS Register Declaration. */
typedef struct emu_cfgns_typedef{
  uint32_t       RESERVED0[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t HOSTIF;                        /**< Host interrupt Flags                               */
  __IOM uint32_t HOSTIEN;                       /**< Host interrupt Enables                             */
  __IM uint32_t  TEMPERATURE;                   /**< Duplicate of EMU_TEMP                              */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED2[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED3[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED4[1015U];              /**< Reserved for future use                            */
  uint32_t       RESERVED5[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t HOSTIF_SET;                    /**< Host interrupt Flags                               */
  __IOM uint32_t HOSTIEN_SET;                   /**< Host interrupt Enables                             */
  __IM uint32_t  TEMPERATURE_SET;               /**< Duplicate of EMU_TEMP                              */
  uint32_t       RESERVED6[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED7[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED8[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED9[1015U];              /**< Reserved for future use                            */
  uint32_t       RESERVED10[3U];                /**< Reserved for future use                            */
  __IOM uint32_t HOSTIF_CLR;                    /**< Host interrupt Flags                               */
  __IOM uint32_t HOSTIEN_CLR;                   /**< Host interrupt Enables                             */
  __IM uint32_t  TEMPERATURE_CLR;               /**< Duplicate of EMU_TEMP                              */
  uint32_t       RESERVED11[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED12[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED13[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED14[1015U];             /**< Reserved for future use                            */
  uint32_t       RESERVED15[3U];                /**< Reserved for future use                            */
  __IOM uint32_t HOSTIF_TGL;                    /**< Host interrupt Flags                               */
  __IOM uint32_t HOSTIEN_TGL;                   /**< Host interrupt Enables                             */
  __IM uint32_t  TEMPERATURE_TGL;               /**< Duplicate of EMU_TEMP                              */
  uint32_t       RESERVED16[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED17[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED18[1U];                /**< Reserved for future use                            */
} EMU_CFGNS_TypeDef;
/** @} End of group SIBG301_EMU_CFGNS */

/**************************************************************************//**
 * @addtogroup SIBG301_EMU_CFGNS
 * @{
 * @defgroup SIBG301_EMU_CFGNS_BitFields EMU_CFGNS Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for EMU HOSTIF */
#define _EMU_HOSTIF_RESETVALUE                     0x00000000UL                                 /**< Default value for EMU_HOSTIF                */
#define _EMU_HOSTIF_MASK                           0xE1000000UL                                 /**< Mask for EMU_HOSTIF                         */
#define EMU_HOSTIF_EM23WAKEUPHOSTIF                (0x1UL << 24)                                /**< EM23 Wake up Interrupt flag                 */
#define _EMU_HOSTIF_EM23WAKEUPHOSTIF_SHIFT         24                                           /**< Shift value for EMU_EM23WAKEUPHOSTIF        */
#define _EMU_HOSTIF_EM23WAKEUPHOSTIF_MASK          0x1000000UL                                  /**< Bit mask for EMU_EM23WAKEUPHOSTIF           */
#define _EMU_HOSTIF_EM23WAKEUPHOSTIF_DEFAULT       0x00000000UL                                 /**< Mode DEFAULT for EMU_HOSTIF                 */
#define EMU_HOSTIF_EM23WAKEUPHOSTIF_DEFAULT        (_EMU_HOSTIF_EM23WAKEUPHOSTIF_DEFAULT << 24) /**< Shifted mode DEFAULT for EMU_HOSTIF         */
#define EMU_HOSTIF_TEMPHOSTIF                      (0x1UL << 29)                                /**< Temperature Interrupt flag                  */
#define _EMU_HOSTIF_TEMPHOSTIF_SHIFT               29                                           /**< Shift value for EMU_TEMPHOSTIF              */
#define _EMU_HOSTIF_TEMPHOSTIF_MASK                0x20000000UL                                 /**< Bit mask for EMU_TEMPHOSTIF                 */
#define _EMU_HOSTIF_TEMPHOSTIF_DEFAULT             0x00000000UL                                 /**< Mode DEFAULT for EMU_HOSTIF                 */
#define EMU_HOSTIF_TEMPHOSTIF_DEFAULT              (_EMU_HOSTIF_TEMPHOSTIF_DEFAULT << 29)       /**< Shifted mode DEFAULT for EMU_HOSTIF         */
#define EMU_HOSTIF_TEMPLOWHOSTIF                   (0x1UL << 30)                                /**< Temperature low Interrupt flag              */
#define _EMU_HOSTIF_TEMPLOWHOSTIF_SHIFT            30                                           /**< Shift value for EMU_TEMPLOWHOSTIF           */
#define _EMU_HOSTIF_TEMPLOWHOSTIF_MASK             0x40000000UL                                 /**< Bit mask for EMU_TEMPLOWHOSTIF              */
#define _EMU_HOSTIF_TEMPLOWHOSTIF_DEFAULT          0x00000000UL                                 /**< Mode DEFAULT for EMU_HOSTIF                 */
#define EMU_HOSTIF_TEMPLOWHOSTIF_DEFAULT           (_EMU_HOSTIF_TEMPLOWHOSTIF_DEFAULT << 30)    /**< Shifted mode DEFAULT for EMU_HOSTIF         */
#define EMU_HOSTIF_TEMPHIGHHOSTIF                  (0x1UL << 31)                                /**< Temperature high Interrupt flag             */
#define _EMU_HOSTIF_TEMPHIGHHOSTIF_SHIFT           31                                           /**< Shift value for EMU_TEMPHIGHHOSTIF          */
#define _EMU_HOSTIF_TEMPHIGHHOSTIF_MASK            0x80000000UL                                 /**< Bit mask for EMU_TEMPHIGHHOSTIF             */
#define _EMU_HOSTIF_TEMPHIGHHOSTIF_DEFAULT         0x00000000UL                                 /**< Mode DEFAULT for EMU_HOSTIF                 */
#define EMU_HOSTIF_TEMPHIGHHOSTIF_DEFAULT          (_EMU_HOSTIF_TEMPHIGHHOSTIF_DEFAULT << 31)   /**< Shifted mode DEFAULT for EMU_HOSTIF         */

/* Bit fields for EMU HOSTIEN */
#define _EMU_HOSTIEN_RESETVALUE                    0x00000000UL                                   /**< Default value for EMU_HOSTIEN               */
#define _EMU_HOSTIEN_MASK                          0xE1000000UL                                   /**< Mask for EMU_HOSTIEN                        */
#define EMU_HOSTIEN_EM23WAKEUPHOSTIEN              (0x1UL << 24)                                  /**< EM23 Wake up Interrupt enable               */
#define _EMU_HOSTIEN_EM23WAKEUPHOSTIEN_SHIFT       24                                             /**< Shift value for EMU_EM23WAKEUPHOSTIEN       */
#define _EMU_HOSTIEN_EM23WAKEUPHOSTIEN_MASK        0x1000000UL                                    /**< Bit mask for EMU_EM23WAKEUPHOSTIEN          */
#define _EMU_HOSTIEN_EM23WAKEUPHOSTIEN_DEFAULT     0x00000000UL                                   /**< Mode DEFAULT for EMU_HOSTIEN                */
#define EMU_HOSTIEN_EM23WAKEUPHOSTIEN_DEFAULT      (_EMU_HOSTIEN_EM23WAKEUPHOSTIEN_DEFAULT << 24) /**< Shifted mode DEFAULT for EMU_HOSTIEN        */
#define EMU_HOSTIEN_TEMPHOSTIEN                    (0x1UL << 29)                                  /**< Temperature Interrupt enable                */
#define _EMU_HOSTIEN_TEMPHOSTIEN_SHIFT             29                                             /**< Shift value for EMU_TEMPHOSTIEN             */
#define _EMU_HOSTIEN_TEMPHOSTIEN_MASK              0x20000000UL                                   /**< Bit mask for EMU_TEMPHOSTIEN                */
#define _EMU_HOSTIEN_TEMPHOSTIEN_DEFAULT           0x00000000UL                                   /**< Mode DEFAULT for EMU_HOSTIEN                */
#define EMU_HOSTIEN_TEMPHOSTIEN_DEFAULT            (_EMU_HOSTIEN_TEMPHOSTIEN_DEFAULT << 29)       /**< Shifted mode DEFAULT for EMU_HOSTIEN        */
#define EMU_HOSTIEN_TEMPLOWHOSTIEN                 (0x1UL << 30)                                  /**< Temperature low Interrupt enable            */
#define _EMU_HOSTIEN_TEMPLOWHOSTIEN_SHIFT          30                                             /**< Shift value for EMU_TEMPLOWHOSTIEN          */
#define _EMU_HOSTIEN_TEMPLOWHOSTIEN_MASK           0x40000000UL                                   /**< Bit mask for EMU_TEMPLOWHOSTIEN             */
#define _EMU_HOSTIEN_TEMPLOWHOSTIEN_DEFAULT        0x00000000UL                                   /**< Mode DEFAULT for EMU_HOSTIEN                */
#define EMU_HOSTIEN_TEMPLOWHOSTIEN_DEFAULT         (_EMU_HOSTIEN_TEMPLOWHOSTIEN_DEFAULT << 30)    /**< Shifted mode DEFAULT for EMU_HOSTIEN        */
#define EMU_HOSTIEN_TEMPHIGHHOSTIEN                (0x1UL << 31)                                  /**< Temperature high Interrupt enable           */
#define _EMU_HOSTIEN_TEMPHIGHHOSTIEN_SHIFT         31                                             /**< Shift value for EMU_TEMPHIGHHOSTIEN         */
#define _EMU_HOSTIEN_TEMPHIGHHOSTIEN_MASK          0x80000000UL                                   /**< Bit mask for EMU_TEMPHIGHHOSTIEN            */
#define _EMU_HOSTIEN_TEMPHIGHHOSTIEN_DEFAULT       0x00000000UL                                   /**< Mode DEFAULT for EMU_HOSTIEN                */
#define EMU_HOSTIEN_TEMPHIGHHOSTIEN_DEFAULT        (_EMU_HOSTIEN_TEMPHIGHHOSTIEN_DEFAULT << 31)   /**< Shifted mode DEFAULT for EMU_HOSTIEN        */

/* Bit fields for EMU TEMPERATURE */
#define _EMU_TEMPERATURE_RESETVALUE                0x00000000UL                                    /**< Default value for EMU_TEMPERATURE           */
#define _EMU_TEMPERATURE_MASK                      0x07FF07FFUL                                    /**< Mask for EMU_TEMPERATURE                    */
#define _EMU_TEMPERATURE_TEMPERATURELSB_SHIFT      0                                               /**< Shift value for EMU_TEMPERATURELSB          */
#define _EMU_TEMPERATURE_TEMPERATURELSB_MASK       0x3UL                                           /**< Bit mask for EMU_TEMPERATURELSB             */
#define _EMU_TEMPERATURE_TEMPERATURELSB_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for EMU_TEMPERATURE            */
#define EMU_TEMPERATURE_TEMPERATURELSB_DEFAULT     (_EMU_TEMPERATURE_TEMPERATURELSB_DEFAULT << 0)  /**< Shifted mode DEFAULT for EMU_TEMPERATURE    */
#define _EMU_TEMPERATURE_TEMPERATURE_SHIFT         2                                               /**< Shift value for EMU_TEMPERATURE             */
#define _EMU_TEMPERATURE_TEMPERATURE_MASK          0x7FCUL                                         /**< Bit mask for EMU_TEMPERATURE                */
#define _EMU_TEMPERATURE_TEMPERATURE_DEFAULT       0x00000000UL                                    /**< Mode DEFAULT for EMU_TEMPERATURE            */
#define EMU_TEMPERATURE_TEMPERATURE_DEFAULT        (_EMU_TEMPERATURE_TEMPERATURE_DEFAULT << 2)     /**< Shifted mode DEFAULT for EMU_TEMPERATURE    */
#define _EMU_TEMPERATURE_TEMPERATUREAVG_SHIFT      16                                              /**< Shift value for EMU_TEMPERATUREAVG          */
#define _EMU_TEMPERATURE_TEMPERATUREAVG_MASK       0x7FF0000UL                                     /**< Bit mask for EMU_TEMPERATUREAVG             */
#define _EMU_TEMPERATURE_TEMPERATUREAVG_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for EMU_TEMPERATURE            */
#define EMU_TEMPERATURE_TEMPERATUREAVG_DEFAULT     (_EMU_TEMPERATURE_TEMPERATUREAVG_DEFAULT << 16) /**< Shifted mode DEFAULT for EMU_TEMPERATURE    */

/** @} End of group SIBG301_EMU_CFGNS_BitFields */
/** @} End of group SIBG301_EMU_CFGNS */
/** @} End of group Parts */

#endif // SIBG301_EMU_H
