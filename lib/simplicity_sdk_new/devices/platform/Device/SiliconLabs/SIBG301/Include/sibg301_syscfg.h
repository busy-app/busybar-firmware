/**************************************************************************//**
 * @file
 * @brief SIBG301 SYSCFG register and bit field definitions
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
#ifndef SIBG301_SYSCFG_H
#define SIBG301_SYSCFG_H
#define SYSCFG_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_SYSCFG SYSCFG
 * @{
 * @brief SIBG301 SYSCFG Register Declaration.
 *****************************************************************************/

/** SYSCFG Register Declaration. */
typedef struct syscfg_typedef{
  uint32_t       RESERVED0[1U];                 /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION;                     /**< IP version ID                                      */
  __IOM uint32_t IF;                            /**< Interrupt Flag                                     */
  __IOM uint32_t IEN;                           /**< Interrupt Enable                                   */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t CHIPREVHW;                     /**< Chip Revision, Hard-wired                          */
  __IOM uint32_t CHIPREV;                       /**< Chip Revision                                      */
  uint32_t       RESERVED2[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFGSYSTIC;                     /**< SysTick clock source                               */
  uint32_t       RESERVED3[3U];                 /**< Reserved for future use                            */
  __IM uint32_t  ROMREVHW;                      /**< ROM Revision, Hard-wire                            */
  uint32_t       RESERVED4[50U];                /**< Reserved for future use                            */
  uint32_t       RESERVED5[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED6[63U];                /**< Reserved for future use                            */
  __IOM uint32_t CTRL;                          /**< Control                                            */
  uint32_t       RESERVED7[255U];               /**< Reserved for future use                            */
  __IOM uint32_t ROOTDATA0;                     /**< Data Register 0                                    */
  __IOM uint32_t ROOTDATA1;                     /**< Data Register 1                                    */
  __IM uint32_t  ROOTLOCKSTATUS;                /**< Lock Status                                        */
  __IOM uint32_t ROOTSESWVERSION;               /**< SE SW Version                                      */
  __IOM uint32_t ROOTSEROOTINFO;                /**< SE Root Info                                       */
  __IM uint32_t  ROOTSEROOTINFOSTATUS;          /**< SE Root Info Status                                */
  uint32_t       RESERVED8[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED9[633U];               /**< Reserved for future use                            */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP version ID                                      */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag                                     */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable                                   */
  uint32_t       RESERVED11[1U];                /**< Reserved for future use                            */
  __IOM uint32_t CHIPREVHW_SET;                 /**< Chip Revision, Hard-wired                          */
  __IOM uint32_t CHIPREV_SET;                   /**< Chip Revision                                      */
  uint32_t       RESERVED12[2U];                /**< Reserved for future use                            */
  __IOM uint32_t CFGSYSTIC_SET;                 /**< SysTick clock source                               */
  uint32_t       RESERVED13[3U];                /**< Reserved for future use                            */
  __IM uint32_t  ROMREVHW_SET;                  /**< ROM Revision, Hard-wire                            */
  uint32_t       RESERVED14[50U];               /**< Reserved for future use                            */
  uint32_t       RESERVED15[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED16[63U];               /**< Reserved for future use                            */
  __IOM uint32_t CTRL_SET;                      /**< Control                                            */
  uint32_t       RESERVED17[255U];              /**< Reserved for future use                            */
  __IOM uint32_t ROOTDATA0_SET;                 /**< Data Register 0                                    */
  __IOM uint32_t ROOTDATA1_SET;                 /**< Data Register 1                                    */
  __IM uint32_t  ROOTLOCKSTATUS_SET;            /**< Lock Status                                        */
  __IOM uint32_t ROOTSESWVERSION_SET;           /**< SE SW Version                                      */
  __IOM uint32_t ROOTSEROOTINFO_SET;            /**< SE Root Info                                       */
  __IM uint32_t  ROOTSEROOTINFOSTATUS_SET;      /**< SE Root Info Status                                */
  uint32_t       RESERVED18[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED19[633U];              /**< Reserved for future use                            */
  uint32_t       RESERVED20[1U];                /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP version ID                                      */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag                                     */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable                                   */
  uint32_t       RESERVED21[1U];                /**< Reserved for future use                            */
  __IOM uint32_t CHIPREVHW_CLR;                 /**< Chip Revision, Hard-wired                          */
  __IOM uint32_t CHIPREV_CLR;                   /**< Chip Revision                                      */
  uint32_t       RESERVED22[2U];                /**< Reserved for future use                            */
  __IOM uint32_t CFGSYSTIC_CLR;                 /**< SysTick clock source                               */
  uint32_t       RESERVED23[3U];                /**< Reserved for future use                            */
  __IM uint32_t  ROMREVHW_CLR;                  /**< ROM Revision, Hard-wire                            */
  uint32_t       RESERVED24[50U];               /**< Reserved for future use                            */
  uint32_t       RESERVED25[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED26[63U];               /**< Reserved for future use                            */
  __IOM uint32_t CTRL_CLR;                      /**< Control                                            */
  uint32_t       RESERVED27[255U];              /**< Reserved for future use                            */
  __IOM uint32_t ROOTDATA0_CLR;                 /**< Data Register 0                                    */
  __IOM uint32_t ROOTDATA1_CLR;                 /**< Data Register 1                                    */
  __IM uint32_t  ROOTLOCKSTATUS_CLR;            /**< Lock Status                                        */
  __IOM uint32_t ROOTSESWVERSION_CLR;           /**< SE SW Version                                      */
  __IOM uint32_t ROOTSEROOTINFO_CLR;            /**< SE Root Info                                       */
  __IM uint32_t  ROOTSEROOTINFOSTATUS_CLR;      /**< SE Root Info Status                                */
  uint32_t       RESERVED28[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED29[633U];              /**< Reserved for future use                            */
  uint32_t       RESERVED30[1U];                /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP version ID                                      */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag                                     */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable                                   */
  uint32_t       RESERVED31[1U];                /**< Reserved for future use                            */
  __IOM uint32_t CHIPREVHW_TGL;                 /**< Chip Revision, Hard-wired                          */
  __IOM uint32_t CHIPREV_TGL;                   /**< Chip Revision                                      */
  uint32_t       RESERVED32[2U];                /**< Reserved for future use                            */
  __IOM uint32_t CFGSYSTIC_TGL;                 /**< SysTick clock source                               */
  uint32_t       RESERVED33[3U];                /**< Reserved for future use                            */
  __IM uint32_t  ROMREVHW_TGL;                  /**< ROM Revision, Hard-wire                            */
  uint32_t       RESERVED34[50U];               /**< Reserved for future use                            */
  uint32_t       RESERVED35[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED36[63U];               /**< Reserved for future use                            */
  __IOM uint32_t CTRL_TGL;                      /**< Control                                            */
  uint32_t       RESERVED37[255U];              /**< Reserved for future use                            */
  __IOM uint32_t ROOTDATA0_TGL;                 /**< Data Register 0                                    */
  __IOM uint32_t ROOTDATA1_TGL;                 /**< Data Register 1                                    */
  __IM uint32_t  ROOTLOCKSTATUS_TGL;            /**< Lock Status                                        */
  __IOM uint32_t ROOTSESWVERSION_TGL;           /**< SE SW Version                                      */
  __IOM uint32_t ROOTSEROOTINFO_TGL;            /**< SE Root Info                                       */
  __IM uint32_t  ROOTSEROOTINFOSTATUS_TGL;      /**< SE Root Info Status                                */
  uint32_t       RESERVED38[1U];                /**< Reserved for future use                            */
} SYSCFG_TypeDef;
/** @} End of group SIBG301_SYSCFG */

/**************************************************************************//**
 * @addtogroup SIBG301_SYSCFG
 * @{
 * @defgroup SIBG301_SYSCFG_BitFields SYSCFG Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SYSCFG IPVERSION */
#define _SYSCFG_IPVERSION_RESETVALUE                          0x0000000AUL                               /**< Default value for SYSCFG_IPVERSION          */
#define _SYSCFG_IPVERSION_MASK                                0xFFFFFFFFUL                               /**< Mask for SYSCFG_IPVERSION                   */
#define _SYSCFG_IPVERSION_IPVERSION_SHIFT                     0                                          /**< Shift value for SYSCFG_IPVERSION            */
#define _SYSCFG_IPVERSION_IPVERSION_MASK                      0xFFFFFFFFUL                               /**< Bit mask for SYSCFG_IPVERSION               */
#define _SYSCFG_IPVERSION_IPVERSION_DEFAULT                   0x0000000AUL                               /**< Mode DEFAULT for SYSCFG_IPVERSION           */
#define SYSCFG_IPVERSION_IPVERSION_DEFAULT                    (_SYSCFG_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_IPVERSION   */

/* Bit fields for SYSCFG IF */
#define _SYSCFG_IF_RESETVALUE                                 0x00000000UL                        /**< Default value for SYSCFG_IF                 */
#define _SYSCFG_IF_MASK                                       0x00033F0FUL                        /**< Mask for SYSCFG_IF                          */
#define SYSCFG_IF_SW0                                         (0x1UL << 0)                        /**< Software Interrupt Flag                     */
#define _SYSCFG_IF_SW0_SHIFT                                  0                                   /**< Shift value for SYSCFG_SW0                  */
#define _SYSCFG_IF_SW0_MASK                                   0x1UL                               /**< Bit mask for SYSCFG_SW0                     */
#define _SYSCFG_IF_SW0_DEFAULT                                0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_SW0_DEFAULT                                 (_SYSCFG_IF_SW0_DEFAULT << 0)       /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_SW1                                         (0x1UL << 1)                        /**< Software Interrupt Flag                     */
#define _SYSCFG_IF_SW1_SHIFT                                  1                                   /**< Shift value for SYSCFG_SW1                  */
#define _SYSCFG_IF_SW1_MASK                                   0x2UL                               /**< Bit mask for SYSCFG_SW1                     */
#define _SYSCFG_IF_SW1_DEFAULT                                0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_SW1_DEFAULT                                 (_SYSCFG_IF_SW1_DEFAULT << 1)       /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_SW2                                         (0x1UL << 2)                        /**< Software Interrupt Flag                     */
#define _SYSCFG_IF_SW2_SHIFT                                  2                                   /**< Shift value for SYSCFG_SW2                  */
#define _SYSCFG_IF_SW2_MASK                                   0x4UL                               /**< Bit mask for SYSCFG_SW2                     */
#define _SYSCFG_IF_SW2_DEFAULT                                0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_SW2_DEFAULT                                 (_SYSCFG_IF_SW2_DEFAULT << 2)       /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_SW3                                         (0x1UL << 3)                        /**< Software Interrupt Flag                     */
#define _SYSCFG_IF_SW3_SHIFT                                  3                                   /**< Shift value for SYSCFG_SW3                  */
#define _SYSCFG_IF_SW3_MASK                                   0x8UL                               /**< Bit mask for SYSCFG_SW3                     */
#define _SYSCFG_IF_SW3_DEFAULT                                0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_SW3_DEFAULT                                 (_SYSCFG_IF_SW3_DEFAULT << 3)       /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_FPIOC                                       (0x1UL << 8)                        /**< FPU Invalid Operation interrupt flag        */
#define _SYSCFG_IF_FPIOC_SHIFT                                8                                   /**< Shift value for SYSCFG_FPIOC                */
#define _SYSCFG_IF_FPIOC_MASK                                 0x100UL                             /**< Bit mask for SYSCFG_FPIOC                   */
#define _SYSCFG_IF_FPIOC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_FPIOC_DEFAULT                               (_SYSCFG_IF_FPIOC_DEFAULT << 8)     /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_FPDZC                                       (0x1UL << 9)                        /**< FPU Divide by zero interrupt flag           */
#define _SYSCFG_IF_FPDZC_SHIFT                                9                                   /**< Shift value for SYSCFG_FPDZC                */
#define _SYSCFG_IF_FPDZC_MASK                                 0x200UL                             /**< Bit mask for SYSCFG_FPDZC                   */
#define _SYSCFG_IF_FPDZC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_FPDZC_DEFAULT                               (_SYSCFG_IF_FPDZC_DEFAULT << 9)     /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_FPUFC                                       (0x1UL << 10)                       /**< FPU Underflow interrupt flag                */
#define _SYSCFG_IF_FPUFC_SHIFT                                10                                  /**< Shift value for SYSCFG_FPUFC                */
#define _SYSCFG_IF_FPUFC_MASK                                 0x400UL                             /**< Bit mask for SYSCFG_FPUFC                   */
#define _SYSCFG_IF_FPUFC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_FPUFC_DEFAULT                               (_SYSCFG_IF_FPUFC_DEFAULT << 10)    /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_FPOFC                                       (0x1UL << 11)                       /**< FPU Overflow interrupt flag                 */
#define _SYSCFG_IF_FPOFC_SHIFT                                11                                  /**< Shift value for SYSCFG_FPOFC                */
#define _SYSCFG_IF_FPOFC_MASK                                 0x800UL                             /**< Bit mask for SYSCFG_FPOFC                   */
#define _SYSCFG_IF_FPOFC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_FPOFC_DEFAULT                               (_SYSCFG_IF_FPOFC_DEFAULT << 11)    /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_FPIDC                                       (0x1UL << 12)                       /**< FPU Input denormal interrupt flag           */
#define _SYSCFG_IF_FPIDC_SHIFT                                12                                  /**< Shift value for SYSCFG_FPIDC                */
#define _SYSCFG_IF_FPIDC_MASK                                 0x1000UL                            /**< Bit mask for SYSCFG_FPIDC                   */
#define _SYSCFG_IF_FPIDC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_FPIDC_DEFAULT                               (_SYSCFG_IF_FPIDC_DEFAULT << 12)    /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_FPIXC                                       (0x1UL << 13)                       /**< FPU Inexact interrupt flag                  */
#define _SYSCFG_IF_FPIXC_SHIFT                                13                                  /**< Shift value for SYSCFG_FPIXC                */
#define _SYSCFG_IF_FPIXC_MASK                                 0x2000UL                            /**< Bit mask for SYSCFG_FPIXC                   */
#define _SYSCFG_IF_FPIXC_DEFAULT                              0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_FPIXC_DEFAULT                               (_SYSCFG_IF_FPIXC_DEFAULT << 13)    /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_TEALSYS                                     (0x1UL << 16)                       /**< TEALSYS bus error                           */
#define _SYSCFG_IF_TEALSYS_SHIFT                              16                                  /**< Shift value for SYSCFG_TEALSYS              */
#define _SYSCFG_IF_TEALSYS_MASK                               0x10000UL                           /**< Bit mask for SYSCFG_TEALSYS                 */
#define _SYSCFG_IF_TEALSYS_DEFAULT                            0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_TEALSYS_DEFAULT                             (_SYSCFG_IF_TEALSYS_DEFAULT << 16)  /**< Shifted mode DEFAULT for SYSCFG_IF          */
#define SYSCFG_IF_L1CACHE0                                    (0x1UL << 17)                       /**< L1CACHE bus error                           */
#define _SYSCFG_IF_L1CACHE0_SHIFT                             17                                  /**< Shift value for SYSCFG_L1CACHE0             */
#define _SYSCFG_IF_L1CACHE0_MASK                              0x20000UL                           /**< Bit mask for SYSCFG_L1CACHE0                */
#define _SYSCFG_IF_L1CACHE0_DEFAULT                           0x00000000UL                        /**< Mode DEFAULT for SYSCFG_IF                  */
#define SYSCFG_IF_L1CACHE0_DEFAULT                            (_SYSCFG_IF_L1CACHE0_DEFAULT << 17) /**< Shifted mode DEFAULT for SYSCFG_IF          */

/* Bit fields for SYSCFG IEN */
#define _SYSCFG_IEN_RESETVALUE                                0x00000000UL                         /**< Default value for SYSCFG_IEN                */
#define _SYSCFG_IEN_MASK                                      0x00033F0FUL                         /**< Mask for SYSCFG_IEN                         */
#define SYSCFG_IEN_SW0                                        (0x1UL << 0)                         /**< Software Interrupt Enable                   */
#define _SYSCFG_IEN_SW0_SHIFT                                 0                                    /**< Shift value for SYSCFG_SW0                  */
#define _SYSCFG_IEN_SW0_MASK                                  0x1UL                                /**< Bit mask for SYSCFG_SW0                     */
#define _SYSCFG_IEN_SW0_DEFAULT                               0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_SW0_DEFAULT                                (_SYSCFG_IEN_SW0_DEFAULT << 0)       /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_SW1                                        (0x1UL << 1)                         /**< Software Interrupt Enable                   */
#define _SYSCFG_IEN_SW1_SHIFT                                 1                                    /**< Shift value for SYSCFG_SW1                  */
#define _SYSCFG_IEN_SW1_MASK                                  0x2UL                                /**< Bit mask for SYSCFG_SW1                     */
#define _SYSCFG_IEN_SW1_DEFAULT                               0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_SW1_DEFAULT                                (_SYSCFG_IEN_SW1_DEFAULT << 1)       /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_SW2                                        (0x1UL << 2)                         /**< Software Interrupt Enable                   */
#define _SYSCFG_IEN_SW2_SHIFT                                 2                                    /**< Shift value for SYSCFG_SW2                  */
#define _SYSCFG_IEN_SW2_MASK                                  0x4UL                                /**< Bit mask for SYSCFG_SW2                     */
#define _SYSCFG_IEN_SW2_DEFAULT                               0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_SW2_DEFAULT                                (_SYSCFG_IEN_SW2_DEFAULT << 2)       /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_SW3                                        (0x1UL << 3)                         /**< Software Interrupt Enable                   */
#define _SYSCFG_IEN_SW3_SHIFT                                 3                                    /**< Shift value for SYSCFG_SW3                  */
#define _SYSCFG_IEN_SW3_MASK                                  0x8UL                                /**< Bit mask for SYSCFG_SW3                     */
#define _SYSCFG_IEN_SW3_DEFAULT                               0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_SW3_DEFAULT                                (_SYSCFG_IEN_SW3_DEFAULT << 3)       /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_FPIOC                                      (0x1UL << 8)                         /**< FPU Invalid Operation Interrupt Enable      */
#define _SYSCFG_IEN_FPIOC_SHIFT                               8                                    /**< Shift value for SYSCFG_FPIOC                */
#define _SYSCFG_IEN_FPIOC_MASK                                0x100UL                              /**< Bit mask for SYSCFG_FPIOC                   */
#define _SYSCFG_IEN_FPIOC_DEFAULT                             0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_FPIOC_DEFAULT                              (_SYSCFG_IEN_FPIOC_DEFAULT << 8)     /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_FPDZC                                      (0x1UL << 9)                         /**< FPU Divide by zero Interrupt Enable         */
#define _SYSCFG_IEN_FPDZC_SHIFT                               9                                    /**< Shift value for SYSCFG_FPDZC                */
#define _SYSCFG_IEN_FPDZC_MASK                                0x200UL                              /**< Bit mask for SYSCFG_FPDZC                   */
#define _SYSCFG_IEN_FPDZC_DEFAULT                             0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_FPDZC_DEFAULT                              (_SYSCFG_IEN_FPDZC_DEFAULT << 9)     /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_FPUFC                                      (0x1UL << 10)                        /**< FPU Underflow Interrupt Enable              */
#define _SYSCFG_IEN_FPUFC_SHIFT                               10                                   /**< Shift value for SYSCFG_FPUFC                */
#define _SYSCFG_IEN_FPUFC_MASK                                0x400UL                              /**< Bit mask for SYSCFG_FPUFC                   */
#define _SYSCFG_IEN_FPUFC_DEFAULT                             0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_FPUFC_DEFAULT                              (_SYSCFG_IEN_FPUFC_DEFAULT << 10)    /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_FPOFC                                      (0x1UL << 11)                        /**< FPU Overflow Interrupt Enable               */
#define _SYSCFG_IEN_FPOFC_SHIFT                               11                                   /**< Shift value for SYSCFG_FPOFC                */
#define _SYSCFG_IEN_FPOFC_MASK                                0x800UL                              /**< Bit mask for SYSCFG_FPOFC                   */
#define _SYSCFG_IEN_FPOFC_DEFAULT                             0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_FPOFC_DEFAULT                              (_SYSCFG_IEN_FPOFC_DEFAULT << 11)    /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_FPIDC                                      (0x1UL << 12)                        /**< FPU Input denormal Interrupt Enable         */
#define _SYSCFG_IEN_FPIDC_SHIFT                               12                                   /**< Shift value for SYSCFG_FPIDC                */
#define _SYSCFG_IEN_FPIDC_MASK                                0x1000UL                             /**< Bit mask for SYSCFG_FPIDC                   */
#define _SYSCFG_IEN_FPIDC_DEFAULT                             0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_FPIDC_DEFAULT                              (_SYSCFG_IEN_FPIDC_DEFAULT << 12)    /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_FPIXC                                      (0x1UL << 13)                        /**< FPU Inexact Interrupt Enable                */
#define _SYSCFG_IEN_FPIXC_SHIFT                               13                                   /**< Shift value for SYSCFG_FPIXC                */
#define _SYSCFG_IEN_FPIXC_MASK                                0x2000UL                             /**< Bit mask for SYSCFG_FPIXC                   */
#define _SYSCFG_IEN_FPIXC_DEFAULT                             0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_FPIXC_DEFAULT                              (_SYSCFG_IEN_FPIXC_DEFAULT << 13)    /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_TEALSYS                                    (0x1UL << 16)                        /**< TEALSYS bus error enable                    */
#define _SYSCFG_IEN_TEALSYS_SHIFT                             16                                   /**< Shift value for SYSCFG_TEALSYS              */
#define _SYSCFG_IEN_TEALSYS_MASK                              0x10000UL                            /**< Bit mask for SYSCFG_TEALSYS                 */
#define _SYSCFG_IEN_TEALSYS_DEFAULT                           0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_TEALSYS_DEFAULT                            (_SYSCFG_IEN_TEALSYS_DEFAULT << 16)  /**< Shifted mode DEFAULT for SYSCFG_IEN         */
#define SYSCFG_IEN_L1CACHE0                                   (0x1UL << 17)                        /**< L1CACHE bus error enable                    */
#define _SYSCFG_IEN_L1CACHE0_SHIFT                            17                                   /**< Shift value for SYSCFG_L1CACHE0             */
#define _SYSCFG_IEN_L1CACHE0_MASK                             0x20000UL                            /**< Bit mask for SYSCFG_L1CACHE0                */
#define _SYSCFG_IEN_L1CACHE0_DEFAULT                          0x00000000UL                         /**< Mode DEFAULT for SYSCFG_IEN                 */
#define SYSCFG_IEN_L1CACHE0_DEFAULT                           (_SYSCFG_IEN_L1CACHE0_DEFAULT << 17) /**< Shifted mode DEFAULT for SYSCFG_IEN         */

/* Bit fields for SYSCFG CHIPREVHW */
#define _SYSCFG_CHIPREVHW_RESETVALUE                          0x00020030UL                                /**< Default value for SYSCFG_CHIPREVHW          */
#define _SYSCFG_CHIPREVHW_MASK                                0xFF0FFFFFUL                                /**< Mask for SYSCFG_CHIPREVHW                   */
#define _SYSCFG_CHIPREVHW_PARTNUMBER_SHIFT                    0                                           /**< Shift value for SYSCFG_PARTNUMBER           */
#define _SYSCFG_CHIPREVHW_PARTNUMBER_MASK                     0xFFFUL                                     /**< Bit mask for SYSCFG_PARTNUMBER              */
#define _SYSCFG_CHIPREVHW_PARTNUMBER_DEFAULT                  0x00000030UL                                /**< Mode DEFAULT for SYSCFG_CHIPREVHW           */
#define SYSCFG_CHIPREVHW_PARTNUMBER_DEFAULT                   (_SYSCFG_CHIPREVHW_PARTNUMBER_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_CHIPREVHW   */
#define _SYSCFG_CHIPREVHW_MINOR_SHIFT                         12                                          /**< Shift value for SYSCFG_MINOR                */
#define _SYSCFG_CHIPREVHW_MINOR_MASK                          0xF000UL                                    /**< Bit mask for SYSCFG_MINOR                   */
#define _SYSCFG_CHIPREVHW_MINOR_DEFAULT                       0x00000000UL                                /**< Mode DEFAULT for SYSCFG_CHIPREVHW           */
#define SYSCFG_CHIPREVHW_MINOR_DEFAULT                        (_SYSCFG_CHIPREVHW_MINOR_DEFAULT << 12)     /**< Shifted mode DEFAULT for SYSCFG_CHIPREVHW   */
#define _SYSCFG_CHIPREVHW_MAJOR_SHIFT                         16                                          /**< Shift value for SYSCFG_MAJOR                */
#define _SYSCFG_CHIPREVHW_MAJOR_MASK                          0xF0000UL                                   /**< Bit mask for SYSCFG_MAJOR                   */
#define _SYSCFG_CHIPREVHW_MAJOR_DEFAULT                       0x00000002UL                                /**< Mode DEFAULT for SYSCFG_CHIPREVHW           */
#define SYSCFG_CHIPREVHW_MAJOR_DEFAULT                        (_SYSCFG_CHIPREVHW_MAJOR_DEFAULT << 16)     /**< Shifted mode DEFAULT for SYSCFG_CHIPREVHW   */

/* Bit fields for SYSCFG CHIPREV */
#define _SYSCFG_CHIPREV_RESETVALUE                            0x00000000UL                              /**< Default value for SYSCFG_CHIPREV            */
#define _SYSCFG_CHIPREV_MASK                                  0x000FFFFFUL                              /**< Mask for SYSCFG_CHIPREV                     */
#define _SYSCFG_CHIPREV_PARTNUMBER_SHIFT                      0                                         /**< Shift value for SYSCFG_PARTNUMBER           */
#define _SYSCFG_CHIPREV_PARTNUMBER_MASK                       0xFFFUL                                   /**< Bit mask for SYSCFG_PARTNUMBER              */
#define _SYSCFG_CHIPREV_PARTNUMBER_DEFAULT                    0x00000000UL                              /**< Mode DEFAULT for SYSCFG_CHIPREV             */
#define SYSCFG_CHIPREV_PARTNUMBER_DEFAULT                     (_SYSCFG_CHIPREV_PARTNUMBER_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_CHIPREV     */
#define _SYSCFG_CHIPREV_MINOR_SHIFT                           12                                        /**< Shift value for SYSCFG_MINOR                */
#define _SYSCFG_CHIPREV_MINOR_MASK                            0xF000UL                                  /**< Bit mask for SYSCFG_MINOR                   */
#define _SYSCFG_CHIPREV_MINOR_DEFAULT                         0x00000000UL                              /**< Mode DEFAULT for SYSCFG_CHIPREV             */
#define SYSCFG_CHIPREV_MINOR_DEFAULT                          (_SYSCFG_CHIPREV_MINOR_DEFAULT << 12)     /**< Shifted mode DEFAULT for SYSCFG_CHIPREV     */
#define _SYSCFG_CHIPREV_MAJOR_SHIFT                           16                                        /**< Shift value for SYSCFG_MAJOR                */
#define _SYSCFG_CHIPREV_MAJOR_MASK                            0xF0000UL                                 /**< Bit mask for SYSCFG_MAJOR                   */
#define _SYSCFG_CHIPREV_MAJOR_DEFAULT                         0x00000000UL                              /**< Mode DEFAULT for SYSCFG_CHIPREV             */
#define SYSCFG_CHIPREV_MAJOR_DEFAULT                          (_SYSCFG_CHIPREV_MAJOR_DEFAULT << 16)     /**< Shifted mode DEFAULT for SYSCFG_CHIPREV     */

/* Bit fields for SYSCFG CFGSYSTIC */
#define _SYSCFG_CFGSYSTIC_RESETVALUE                          0x00000000UL                                    /**< Default value for SYSCFG_CFGSYSTIC          */
#define _SYSCFG_CFGSYSTIC_MASK                                0x00000001UL                                    /**< Mask for SYSCFG_CFGSYSTIC                   */
#define SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN                       (0x1UL << 0)                                    /**< SysTick External Clock Enable               */
#define _SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_SHIFT                0                                               /**< Shift value for SYSCFG_SYSTICEXTCLKEN       */
#define _SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_MASK                 0x1UL                                           /**< Bit mask for SYSCFG_SYSTICEXTCLKEN          */
#define _SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_DEFAULT              0x00000000UL                                    /**< Mode DEFAULT for SYSCFG_CFGSYSTIC           */
#define SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_DEFAULT               (_SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_CFGSYSTIC   */

/* Bit fields for SYSCFG ROMREVHW */
#define _SYSCFG_ROMREVHW_RESETVALUE                           0x30000005UL                                /**< Default value for SYSCFG_ROMREVHW           */
#define _SYSCFG_ROMREVHW_MASK                                 0xFF0000FFUL                                /**< Mask for SYSCFG_ROMREVHW                    */
#define _SYSCFG_ROMREVHW_SEROMREV_SHIFT                       0                                           /**< Shift value for SYSCFG_SEROMREV             */
#define _SYSCFG_ROMREVHW_SEROMREV_MASK                        0xFFUL                                      /**< Bit mask for SYSCFG_SEROMREV                */
#define _SYSCFG_ROMREVHW_SEROMREV_DEFAULT                     0x00000005UL                                /**< Mode DEFAULT for SYSCFG_ROMREVHW            */
#define SYSCFG_ROMREVHW_SEROMREV_DEFAULT                      (_SYSCFG_ROMREVHW_SEROMREV_DEFAULT << 0)    /**< Shifted mode DEFAULT for SYSCFG_ROMREVHW    */
#define _SYSCFG_ROMREVHW_SECOMPATID_SHIFT                     24                                          /**< Shift value for SYSCFG_SECOMPATID           */
#define _SYSCFG_ROMREVHW_SECOMPATID_MASK                      0xFF000000UL                                /**< Bit mask for SYSCFG_SECOMPATID              */
#define _SYSCFG_ROMREVHW_SECOMPATID_DEFAULT                   0x00000030UL                                /**< Mode DEFAULT for SYSCFG_ROMREVHW            */
#define SYSCFG_ROMREVHW_SECOMPATID_DEFAULT                    (_SYSCFG_ROMREVHW_SECOMPATID_DEFAULT << 24) /**< Shifted mode DEFAULT for SYSCFG_ROMREVHW    */

/* Bit fields for SYSCFG CTRL */
#define _SYSCFG_CTRL_RESETVALUE                               0x00000003UL                              /**< Default value for SYSCFG_CTRL               */
#define _SYSCFG_CTRL_MASK                                     0x00000003UL                              /**< Mask for SYSCFG_CTRL                        */
#define SYSCFG_CTRL_ADDRFAULTEN                               (0x1UL << 0)                              /**< Invalid Address Bus Fault Response Enabl    */
#define _SYSCFG_CTRL_ADDRFAULTEN_SHIFT                        0                                         /**< Shift value for SYSCFG_ADDRFAULTEN          */
#define _SYSCFG_CTRL_ADDRFAULTEN_MASK                         0x1UL                                     /**< Bit mask for SYSCFG_ADDRFAULTEN             */
#define _SYSCFG_CTRL_ADDRFAULTEN_DEFAULT                      0x00000001UL                              /**< Mode DEFAULT for SYSCFG_CTRL                */
#define SYSCFG_CTRL_ADDRFAULTEN_DEFAULT                       (_SYSCFG_CTRL_ADDRFAULTEN_DEFAULT << 0)   /**< Shifted mode DEFAULT for SYSCFG_CTRL        */
#define SYSCFG_CTRL_CLKDISFAULTEN                             (0x1UL << 1)                              /**< Disabled Clkbus Bus Fault Enable            */
#define _SYSCFG_CTRL_CLKDISFAULTEN_SHIFT                      1                                         /**< Shift value for SYSCFG_CLKDISFAULTEN        */
#define _SYSCFG_CTRL_CLKDISFAULTEN_MASK                       0x2UL                                     /**< Bit mask for SYSCFG_CLKDISFAULTEN           */
#define _SYSCFG_CTRL_CLKDISFAULTEN_DEFAULT                    0x00000001UL                              /**< Mode DEFAULT for SYSCFG_CTRL                */
#define SYSCFG_CTRL_CLKDISFAULTEN_DEFAULT                     (_SYSCFG_CTRL_CLKDISFAULTEN_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSCFG_CTRL        */

/* Bit fields for SYSCFG ROOTDATA0 */
#define _SYSCFG_ROOTDATA0_RESETVALUE                          0x00000000UL                          /**< Default value for SYSCFG_ROOTDATA0          */
#define _SYSCFG_ROOTDATA0_MASK                                0xFFFFFFFFUL                          /**< Mask for SYSCFG_ROOTDATA0                   */
#define _SYSCFG_ROOTDATA0_DATA_SHIFT                          0                                     /**< Shift value for SYSCFG_DATA                 */
#define _SYSCFG_ROOTDATA0_DATA_MASK                           0xFFFFFFFFUL                          /**< Bit mask for SYSCFG_DATA                    */
#define _SYSCFG_ROOTDATA0_DATA_DEFAULT                        0x00000000UL                          /**< Mode DEFAULT for SYSCFG_ROOTDATA0           */
#define SYSCFG_ROOTDATA0_DATA_DEFAULT                         (_SYSCFG_ROOTDATA0_DATA_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_ROOTDATA0   */

/* Bit fields for SYSCFG ROOTDATA1 */
#define _SYSCFG_ROOTDATA1_RESETVALUE                          0x00000000UL                          /**< Default value for SYSCFG_ROOTDATA1          */
#define _SYSCFG_ROOTDATA1_MASK                                0xFFFFFFFFUL                          /**< Mask for SYSCFG_ROOTDATA1                   */
#define _SYSCFG_ROOTDATA1_DATA_SHIFT                          0                                     /**< Shift value for SYSCFG_DATA                 */
#define _SYSCFG_ROOTDATA1_DATA_MASK                           0xFFFFFFFFUL                          /**< Bit mask for SYSCFG_DATA                    */
#define _SYSCFG_ROOTDATA1_DATA_DEFAULT                        0x00000000UL                          /**< Mode DEFAULT for SYSCFG_ROOTDATA1           */
#define SYSCFG_ROOTDATA1_DATA_DEFAULT                         (_SYSCFG_ROOTDATA1_DATA_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_ROOTDATA1   */

/* Bit fields for SYSCFG ROOTLOCKSTATUS */
#define _SYSCFG_ROOTLOCKSTATUS_RESETVALUE                     0x007F0107UL                                          /**< Default value for SYSCFG_ROOTLOCKSTATUS     */
#define _SYSCFG_ROOTLOCKSTATUS_MASK                           0x807F0107UL                                          /**< Mask for SYSCFG_ROOTLOCKSTATUS              */
#define SYSCFG_ROOTLOCKSTATUS_BUSLOCK                         (0x1UL << 0)                                          /**< Bus Lock                                    */
#define _SYSCFG_ROOTLOCKSTATUS_BUSLOCK_SHIFT                  0                                                     /**< Shift value for SYSCFG_BUSLOCK              */
#define _SYSCFG_ROOTLOCKSTATUS_BUSLOCK_MASK                   0x1UL                                                 /**< Bit mask for SYSCFG_BUSLOCK                 */
#define _SYSCFG_ROOTLOCKSTATUS_BUSLOCK_DEFAULT                0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_BUSLOCK_DEFAULT                 (_SYSCFG_ROOTLOCKSTATUS_BUSLOCK_DEFAULT << 0)         /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_REGLOCK                         (0x1UL << 1)                                          /**< Register Lock                               */
#define _SYSCFG_ROOTLOCKSTATUS_REGLOCK_SHIFT                  1                                                     /**< Shift value for SYSCFG_REGLOCK              */
#define _SYSCFG_ROOTLOCKSTATUS_REGLOCK_MASK                   0x2UL                                                 /**< Bit mask for SYSCFG_REGLOCK                 */
#define _SYSCFG_ROOTLOCKSTATUS_REGLOCK_DEFAULT                0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_REGLOCK_DEFAULT                 (_SYSCFG_ROOTLOCKSTATUS_REGLOCK_DEFAULT << 1)         /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_MFRLOCK                         (0x1UL << 2)                                          /**< Manufacture Lock                            */
#define _SYSCFG_ROOTLOCKSTATUS_MFRLOCK_SHIFT                  2                                                     /**< Shift value for SYSCFG_MFRLOCK              */
#define _SYSCFG_ROOTLOCKSTATUS_MFRLOCK_MASK                   0x4UL                                                 /**< Bit mask for SYSCFG_MFRLOCK                 */
#define _SYSCFG_ROOTLOCKSTATUS_MFRLOCK_DEFAULT                0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_MFRLOCK_DEFAULT                 (_SYSCFG_ROOTLOCKSTATUS_MFRLOCK_DEFAULT << 2)         /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_ROOTDBGLOCK                     (0x1UL << 8)                                          /**< Root Debug Lock                             */
#define _SYSCFG_ROOTLOCKSTATUS_ROOTDBGLOCK_SHIFT              8                                                     /**< Shift value for SYSCFG_ROOTDBGLOCK          */
#define _SYSCFG_ROOTLOCKSTATUS_ROOTDBGLOCK_MASK               0x100UL                                               /**< Bit mask for SYSCFG_ROOTDBGLOCK             */
#define _SYSCFG_ROOTLOCKSTATUS_ROOTDBGLOCK_DEFAULT            0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_ROOTDBGLOCK_DEFAULT             (_SYSCFG_ROOTLOCKSTATUS_ROOTDBGLOCK_DEFAULT << 8)     /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_USERDBGAPLOCK                   (0x1UL << 16)                                         /**< User Debug Access Port Lock                 */
#define _SYSCFG_ROOTLOCKSTATUS_USERDBGAPLOCK_SHIFT            16                                                    /**< Shift value for SYSCFG_USERDBGAPLOCK        */
#define _SYSCFG_ROOTLOCKSTATUS_USERDBGAPLOCK_MASK             0x10000UL                                             /**< Bit mask for SYSCFG_USERDBGAPLOCK           */
#define _SYSCFG_ROOTLOCKSTATUS_USERDBGAPLOCK_DEFAULT          0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_USERDBGAPLOCK_DEFAULT           (_SYSCFG_ROOTLOCKSTATUS_USERDBGAPLOCK_DEFAULT << 16)  /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_USERDBGLOCK                     (0x1UL << 17)                                         /**< User Invasive Debug Lock                    */
#define _SYSCFG_ROOTLOCKSTATUS_USERDBGLOCK_SHIFT              17                                                    /**< Shift value for SYSCFG_USERDBGLOCK          */
#define _SYSCFG_ROOTLOCKSTATUS_USERDBGLOCK_MASK               0x20000UL                                             /**< Bit mask for SYSCFG_USERDBGLOCK             */
#define _SYSCFG_ROOTLOCKSTATUS_USERDBGLOCK_DEFAULT            0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_USERDBGLOCK_DEFAULT             (_SYSCFG_ROOTLOCKSTATUS_USERDBGLOCK_DEFAULT << 17)    /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_USERNIDLOCK                     (0x1UL << 18)                                         /**< User Non-invasive Debug Lock                */
#define _SYSCFG_ROOTLOCKSTATUS_USERNIDLOCK_SHIFT              18                                                    /**< Shift value for SYSCFG_USERNIDLOCK          */
#define _SYSCFG_ROOTLOCKSTATUS_USERNIDLOCK_MASK               0x40000UL                                             /**< Bit mask for SYSCFG_USERNIDLOCK             */
#define _SYSCFG_ROOTLOCKSTATUS_USERNIDLOCK_DEFAULT            0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_USERNIDLOCK_DEFAULT             (_SYSCFG_ROOTLOCKSTATUS_USERNIDLOCK_DEFAULT << 18)    /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_USERSPIDLOCK                    (0x1UL << 19)                                         /**< User Secure Invasive Debug Lock             */
#define _SYSCFG_ROOTLOCKSTATUS_USERSPIDLOCK_SHIFT             19                                                    /**< Shift value for SYSCFG_USERSPIDLOCK         */
#define _SYSCFG_ROOTLOCKSTATUS_USERSPIDLOCK_MASK              0x80000UL                                             /**< Bit mask for SYSCFG_USERSPIDLOCK            */
#define _SYSCFG_ROOTLOCKSTATUS_USERSPIDLOCK_DEFAULT           0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_USERSPIDLOCK_DEFAULT            (_SYSCFG_ROOTLOCKSTATUS_USERSPIDLOCK_DEFAULT << 19)   /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_USERSPNIDLOCK                   (0x1UL << 20)                                         /**< User Secure Non-invasive Debug Lock         */
#define _SYSCFG_ROOTLOCKSTATUS_USERSPNIDLOCK_SHIFT            20                                                    /**< Shift value for SYSCFG_USERSPNIDLOCK        */
#define _SYSCFG_ROOTLOCKSTATUS_USERSPNIDLOCK_MASK             0x100000UL                                            /**< Bit mask for SYSCFG_USERSPNIDLOCK           */
#define _SYSCFG_ROOTLOCKSTATUS_USERSPNIDLOCK_DEFAULT          0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_USERSPNIDLOCK_DEFAULT           (_SYSCFG_ROOTLOCKSTATUS_USERSPNIDLOCK_DEFAULT << 20)  /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_LPWCPU0DBGLOCK                  (0x1UL << 21)                                         /**< LPW CPU0 Debug Lock                         */
#define _SYSCFG_ROOTLOCKSTATUS_LPWCPU0DBGLOCK_SHIFT           21                                                    /**< Shift value for SYSCFG_LPWCPU0DBGLOCK       */
#define _SYSCFG_ROOTLOCKSTATUS_LPWCPU0DBGLOCK_MASK            0x200000UL                                            /**< Bit mask for SYSCFG_LPWCPU0DBGLOCK          */
#define _SYSCFG_ROOTLOCKSTATUS_LPWCPU0DBGLOCK_DEFAULT         0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_LPWCPU0DBGLOCK_DEFAULT          (_SYSCFG_ROOTLOCKSTATUS_LPWCPU0DBGLOCK_DEFAULT << 21) /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_LPWCPU1DBGLOCK                  (0x1UL << 22)                                         /**< LPW CPU1 Debug Lock                         */
#define _SYSCFG_ROOTLOCKSTATUS_LPWCPU1DBGLOCK_SHIFT           22                                                    /**< Shift value for SYSCFG_LPWCPU1DBGLOCK       */
#define _SYSCFG_ROOTLOCKSTATUS_LPWCPU1DBGLOCK_MASK            0x400000UL                                            /**< Bit mask for SYSCFG_LPWCPU1DBGLOCK          */
#define _SYSCFG_ROOTLOCKSTATUS_LPWCPU1DBGLOCK_DEFAULT         0x00000001UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_LPWCPU1DBGLOCK_DEFAULT          (_SYSCFG_ROOTLOCKSTATUS_LPWCPU1DBGLOCK_DEFAULT << 22) /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/
#define SYSCFG_ROOTLOCKSTATUS_EFUSEUNLOCKED                   (0x1UL << 31)                                         /**< E-Fuse Unlocked                             */
#define _SYSCFG_ROOTLOCKSTATUS_EFUSEUNLOCKED_SHIFT            31                                                    /**< Shift value for SYSCFG_EFUSEUNLOCKED        */
#define _SYSCFG_ROOTLOCKSTATUS_EFUSEUNLOCKED_MASK             0x80000000UL                                          /**< Bit mask for SYSCFG_EFUSEUNLOCKED           */
#define _SYSCFG_ROOTLOCKSTATUS_EFUSEUNLOCKED_DEFAULT          0x00000000UL                                          /**< Mode DEFAULT for SYSCFG_ROOTLOCKSTATUS      */
#define SYSCFG_ROOTLOCKSTATUS_EFUSEUNLOCKED_DEFAULT           (_SYSCFG_ROOTLOCKSTATUS_EFUSEUNLOCKED_DEFAULT << 31)  /**< Shifted mode DEFAULT for SYSCFG_ROOTLOCKSTATUS*/

/* Bit fields for SYSCFG ROOTSESWVERSION */
#define _SYSCFG_ROOTSESWVERSION_RESETVALUE                    0x00000000UL                                     /**< Default value for SYSCFG_ROOTSESWVERSION    */
#define _SYSCFG_ROOTSESWVERSION_MASK                          0xFFFFFFFFUL                                     /**< Mask for SYSCFG_ROOTSESWVERSION             */
#define _SYSCFG_ROOTSESWVERSION_SWVERSION_SHIFT               0                                                /**< Shift value for SYSCFG_SWVERSION            */
#define _SYSCFG_ROOTSESWVERSION_SWVERSION_MASK                0xFFFFFFFFUL                                     /**< Bit mask for SYSCFG_SWVERSION               */
#define _SYSCFG_ROOTSESWVERSION_SWVERSION_DEFAULT             0x00000000UL                                     /**< Mode DEFAULT for SYSCFG_ROOTSESWVERSION     */
#define SYSCFG_ROOTSESWVERSION_SWVERSION_DEFAULT              (_SYSCFG_ROOTSESWVERSION_SWVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_ROOTSESWVERSION*/

/* Bit fields for SYSCFG ROOTSEROOTINFO */
#define _SYSCFG_ROOTSEROOTINFO_RESETVALUE                     0x00000000UL                                            /**< Default value for SYSCFG_ROOTSEROOTINFO     */
#define _SYSCFG_ROOTSEROOTINFO_MASK                           0x00000701UL                                            /**< Mask for SYSCFG_ROOTSEROOTINFO              */
#define SYSCFG_ROOTSEROOTINFO_ROOTSECOMPROMISED               (0x1UL << 0)                                            /**< ROOT SE COMPROMISED                         */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSECOMPROMISED_SHIFT        0                                                       /**< Shift value for SYSCFG_ROOTSECOMPROMISED    */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSECOMPROMISED_MASK         0x1UL                                                   /**< Bit mask for SYSCFG_ROOTSECOMPROMISED       */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSECOMPROMISED_DEFAULT      0x00000000UL                                            /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFO      */
#define SYSCFG_ROOTSEROOTINFO_ROOTSECOMPROMISED_DEFAULT       (_SYSCFG_ROOTSEROOTINFO_ROOTSECOMPROMISED_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFO*/
#define SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE0                (0x1UL << 8)                                            /**< ROOTSELIFECYCLE0                            */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE0_SHIFT         8                                                       /**< Shift value for SYSCFG_ROOTSELIFECYCLE0     */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE0_MASK          0x100UL                                                 /**< Bit mask for SYSCFG_ROOTSELIFECYCLE0        */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE0_DEFAULT       0x00000000UL                                            /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFO      */
#define SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE0_DEFAULT        (_SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE0_DEFAULT << 8)  /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFO*/
#define SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE1                (0x1UL << 9)                                            /**< ROOTSELIFECYCLE1                            */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE1_SHIFT         9                                                       /**< Shift value for SYSCFG_ROOTSELIFECYCLE1     */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE1_MASK          0x200UL                                                 /**< Bit mask for SYSCFG_ROOTSELIFECYCLE1        */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE1_DEFAULT       0x00000000UL                                            /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFO      */
#define SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE1_DEFAULT        (_SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE1_DEFAULT << 9)  /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFO*/
#define SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE2                (0x1UL << 10)                                           /**< ROOTSELIFECYCLE2                            */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE2_SHIFT         10                                                      /**< Shift value for SYSCFG_ROOTSELIFECYCLE2     */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE2_MASK          0x400UL                                                 /**< Bit mask for SYSCFG_ROOTSELIFECYCLE2        */
#define _SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE2_DEFAULT       0x00000000UL                                            /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFO      */
#define SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE2_DEFAULT        (_SYSCFG_ROOTSEROOTINFO_ROOTSELIFECYCLE2_DEFAULT << 10) /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFO*/

/* Bit fields for SYSCFG ROOTSEROOTINFOSTATUS */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_RESETVALUE               0x00000000UL                                              /**< Default value for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define _SYSCFG_ROOTSEROOTINFOSTATUS_MASK                     0x00000701UL                                              /**< Mask for SYSCFG_ROOTSEROOTINFOSTATUS        */
#define SYSCFG_ROOTSEROOTINFOSTATUS_SECOMPROMISED             (0x1UL << 0)                                              /**< SECOMPROMISED                               */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SECOMPROMISED_SHIFT      0                                                         /**< Shift value for SYSCFG_SECOMPROMISED        */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SECOMPROMISED_MASK       0x1UL                                                     /**< Bit mask for SYSCFG_SECOMPROMISED           */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SECOMPROMISED_DEFAULT    0x00000000UL                                              /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SECOMPROMISED_DEFAULT     (_SYSCFG_ROOTSEROOTINFOSTATUS_SECOMPROMISED_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE0              (0x1UL << 8)                                              /**< SELIFECYCLE0                                */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE0_SHIFT       8                                                         /**< Shift value for SYSCFG_SELIFECYCLE0         */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE0_MASK        0x100UL                                                   /**< Bit mask for SYSCFG_SELIFECYCLE0            */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE0_DEFAULT     0x00000000UL                                              /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE0_DEFAULT      (_SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE0_DEFAULT << 8)  /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE1              (0x1UL << 9)                                              /**< SELIFECYCLE1                                */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE1_SHIFT       9                                                         /**< Shift value for SYSCFG_SELIFECYCLE1         */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE1_MASK        0x200UL                                                   /**< Bit mask for SYSCFG_SELIFECYCLE1            */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE1_DEFAULT     0x00000000UL                                              /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE1_DEFAULT      (_SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE1_DEFAULT << 9)  /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE2              (0x1UL << 10)                                             /**< SELIFECYCLE2                                */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE2_SHIFT       10                                                        /**< Shift value for SYSCFG_SELIFECYCLE2         */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE2_MASK        0x400UL                                                   /**< Bit mask for SYSCFG_SELIFECYCLE2            */
#define _SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE2_DEFAULT     0x00000000UL                                              /**< Mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/
#define SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE2_DEFAULT      (_SYSCFG_ROOTSEROOTINFOSTATUS_SELIFECYCLE2_DEFAULT << 10) /**< Shifted mode DEFAULT for SYSCFG_ROOTSEROOTINFOSTATUS*/

/** @} End of group SIBG301_SYSCFG_BitFields */
/** @} End of group SIBG301_SYSCFG */
/**************************************************************************//**
 * @defgroup SIBG301_SYSCFG_CFGNS SYSCFG_CFGNS
 * @{
 * @brief SIBG301 SYSCFG_CFGNS Register Declaration.
 *****************************************************************************/

/** SYSCFG_CFGNS Register Declaration. */
typedef struct syscfg_cfgns_typedef{
  uint32_t       RESERVED0[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFGNSTCALIB;                   /**< Configure Non-Secure Sys-Tick cal.                 */
  uint32_t       RESERVED1[376U];               /**< Reserved for future use                            */
  __IOM uint32_t ROOTNSDATA0;                   /**< Data Register 0                                    */
  __IOM uint32_t ROOTNSDATA1;                   /**< Data Register 1                                    */
  uint32_t       RESERVED2[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED3[637U];               /**< Reserved for future use                            */
  uint32_t       RESERVED4[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFGNSTCALIB_SET;               /**< Configure Non-Secure Sys-Tick cal.                 */
  uint32_t       RESERVED5[376U];               /**< Reserved for future use                            */
  __IOM uint32_t ROOTNSDATA0_SET;               /**< Data Register 0                                    */
  __IOM uint32_t ROOTNSDATA1_SET;               /**< Data Register 1                                    */
  uint32_t       RESERVED6[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED7[637U];               /**< Reserved for future use                            */
  uint32_t       RESERVED8[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFGNSTCALIB_CLR;               /**< Configure Non-Secure Sys-Tick cal.                 */
  uint32_t       RESERVED9[376U];               /**< Reserved for future use                            */
  __IOM uint32_t ROOTNSDATA0_CLR;               /**< Data Register 0                                    */
  __IOM uint32_t ROOTNSDATA1_CLR;               /**< Data Register 1                                    */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED11[637U];              /**< Reserved for future use                            */
  uint32_t       RESERVED12[7U];                /**< Reserved for future use                            */
  __IOM uint32_t CFGNSTCALIB_TGL;               /**< Configure Non-Secure Sys-Tick cal.                 */
  uint32_t       RESERVED13[376U];              /**< Reserved for future use                            */
  __IOM uint32_t ROOTNSDATA0_TGL;               /**< Data Register 0                                    */
  __IOM uint32_t ROOTNSDATA1_TGL;               /**< Data Register 1                                    */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
} SYSCFG_CFGNS_TypeDef;
/** @} End of group SIBG301_SYSCFG_CFGNS */

/**************************************************************************//**
 * @addtogroup SIBG301_SYSCFG_CFGNS
 * @{
 * @defgroup SIBG301_SYSCFG_CFGNS_BitFields SYSCFG_CFGNS Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SYSCFG CFGNSTCALIB */
#define _SYSCFG_CFGNSTCALIB_RESETVALUE       0x01000000UL                               /**< Default value for SYSCFG_CFGNSTCALIB        */
#define _SYSCFG_CFGNSTCALIB_MASK             0x03FFFFFFUL                               /**< Mask for SYSCFG_CFGNSTCALIB                 */
#define _SYSCFG_CFGNSTCALIB_TENMS_SHIFT      0                                          /**< Shift value for SYSCFG_TENMS                */
#define _SYSCFG_CFGNSTCALIB_TENMS_MASK       0xFFFFFFUL                                 /**< Bit mask for SYSCFG_TENMS                   */
#define _SYSCFG_CFGNSTCALIB_TENMS_DEFAULT    0x00000000UL                               /**< Mode DEFAULT for SYSCFG_CFGNSTCALIB         */
#define SYSCFG_CFGNSTCALIB_TENMS_DEFAULT     (_SYSCFG_CFGNSTCALIB_TENMS_DEFAULT << 0)   /**< Shifted mode DEFAULT for SYSCFG_CFGNSTCALIB */
#define SYSCFG_CFGNSTCALIB_SKEW              (0x1UL << 24)                              /**< Skew                                        */
#define _SYSCFG_CFGNSTCALIB_SKEW_SHIFT       24                                         /**< Shift value for SYSCFG_SKEW                 */
#define _SYSCFG_CFGNSTCALIB_SKEW_MASK        0x1000000UL                                /**< Bit mask for SYSCFG_SKEW                    */
#define _SYSCFG_CFGNSTCALIB_SKEW_DEFAULT     0x00000001UL                               /**< Mode DEFAULT for SYSCFG_CFGNSTCALIB         */
#define SYSCFG_CFGNSTCALIB_SKEW_DEFAULT      (_SYSCFG_CFGNSTCALIB_SKEW_DEFAULT << 24)   /**< Shifted mode DEFAULT for SYSCFG_CFGNSTCALIB */
#define SYSCFG_CFGNSTCALIB_NOREF             (0x1UL << 25)                              /**< No Reference                                */
#define _SYSCFG_CFGNSTCALIB_NOREF_SHIFT      25                                         /**< Shift value for SYSCFG_NOREF                */
#define _SYSCFG_CFGNSTCALIB_NOREF_MASK       0x2000000UL                                /**< Bit mask for SYSCFG_NOREF                   */
#define _SYSCFG_CFGNSTCALIB_NOREF_DEFAULT    0x00000000UL                               /**< Mode DEFAULT for SYSCFG_CFGNSTCALIB         */
#define _SYSCFG_CFGNSTCALIB_NOREF_REF        0x00000000UL                               /**< Mode REF for SYSCFG_CFGNSTCALIB             */
#define _SYSCFG_CFGNSTCALIB_NOREF_NOREF      0x00000001UL                               /**< Mode NOREF for SYSCFG_CFGNSTCALIB           */
#define SYSCFG_CFGNSTCALIB_NOREF_DEFAULT     (_SYSCFG_CFGNSTCALIB_NOREF_DEFAULT << 25)  /**< Shifted mode DEFAULT for SYSCFG_CFGNSTCALIB */
#define SYSCFG_CFGNSTCALIB_NOREF_REF         (_SYSCFG_CFGNSTCALIB_NOREF_REF << 25)      /**< Shifted mode REF for SYSCFG_CFGNSTCALIB     */
#define SYSCFG_CFGNSTCALIB_NOREF_NOREF       (_SYSCFG_CFGNSTCALIB_NOREF_NOREF << 25)    /**< Shifted mode NOREF for SYSCFG_CFGNSTCALIB   */

/* Bit fields for SYSCFG ROOTNSDATA0 */
#define _SYSCFG_ROOTNSDATA0_RESETVALUE       0x00000000UL                               /**< Default value for SYSCFG_ROOTNSDATA0        */
#define _SYSCFG_ROOTNSDATA0_MASK             0xFFFFFFFFUL                               /**< Mask for SYSCFG_ROOTNSDATA0                 */
#define _SYSCFG_ROOTNSDATA0_DATA_SHIFT       0                                          /**< Shift value for SYSCFG_DATA                 */
#define _SYSCFG_ROOTNSDATA0_DATA_MASK        0xFFFFFFFFUL                               /**< Bit mask for SYSCFG_DATA                    */
#define _SYSCFG_ROOTNSDATA0_DATA_DEFAULT     0x00000000UL                               /**< Mode DEFAULT for SYSCFG_ROOTNSDATA0         */
#define SYSCFG_ROOTNSDATA0_DATA_DEFAULT      (_SYSCFG_ROOTNSDATA0_DATA_DEFAULT << 0)    /**< Shifted mode DEFAULT for SYSCFG_ROOTNSDATA0 */

/* Bit fields for SYSCFG ROOTNSDATA1 */
#define _SYSCFG_ROOTNSDATA1_RESETVALUE       0x00000000UL                               /**< Default value for SYSCFG_ROOTNSDATA1        */
#define _SYSCFG_ROOTNSDATA1_MASK             0xFFFFFFFFUL                               /**< Mask for SYSCFG_ROOTNSDATA1                 */
#define _SYSCFG_ROOTNSDATA1_DATA_SHIFT       0                                          /**< Shift value for SYSCFG_DATA                 */
#define _SYSCFG_ROOTNSDATA1_DATA_MASK        0xFFFFFFFFUL                               /**< Bit mask for SYSCFG_DATA                    */
#define _SYSCFG_ROOTNSDATA1_DATA_DEFAULT     0x00000000UL                               /**< Mode DEFAULT for SYSCFG_ROOTNSDATA1         */
#define SYSCFG_ROOTNSDATA1_DATA_DEFAULT      (_SYSCFG_ROOTNSDATA1_DATA_DEFAULT << 0)    /**< Shifted mode DEFAULT for SYSCFG_ROOTNSDATA1 */

/** @} End of group SIBG301_SYSCFG_CFGNS_BitFields */
/** @} End of group SIBG301_SYSCFG_CFGNS */
/** @} End of group Parts */

#endif // SIBG301_SYSCFG_H
