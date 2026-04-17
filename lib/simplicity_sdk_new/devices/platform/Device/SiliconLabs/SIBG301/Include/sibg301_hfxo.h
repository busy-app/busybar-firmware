/**************************************************************************//**
 * @file
 * @brief SIBG301 HFXO register and bit field definitions
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
#ifndef SIBG301_HFXO_H
#define SIBG301_HFXO_H
#define HFXO_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_HFXO HFXO
 * @{
 * @brief SIBG301 HFXO Register Declaration.
 *****************************************************************************/

/** HFXO Register Declaration. */
typedef struct hfxo_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP version ID                                      */
  uint32_t       RESERVED0[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t XTALCFG;                       /**< Crystal Configuration Register                     */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t XTALCTRL;                      /**< Crystal Control Register                           */
  uint32_t       RESERVED2[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFG;                           /**< Configuration Register                             */
  uint32_t       RESERVED3[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t SLEEPYXTALCFG0;                /**< Sleepy Crystal Configuration 0                     */
  __IOM uint32_t SLEEPYXTALCFG1;                /**< Sleepy Crystal Configuration 1                     */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  uint32_t       RESERVED4[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t EXTCLKPKDETCTRL;               /**< Peak Detector Control Register                     */
  uint32_t       RESERVED5[4U];                 /**< Reserved for future use                            */
  __IOM uint32_t CMD;                           /**< Command Register                                   */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  __IM uint32_t  AVGSTARTUPTIME;                /**< Average Startup Time Register                      */
  uint32_t       RESERVED6[4U];                 /**< Reserved for future use                            */
  __IOM uint32_t IF;                            /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN;                           /**< Interrupt Enable Register                          */
  uint32_t       RESERVED7[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t LOCK;                          /**< Configuration Lock Register                        */
  uint32_t       RESERVED8[7U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED9[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED10[983U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP version ID                                      */
  uint32_t       RESERVED11[3U];                /**< Reserved for future use                            */
  __IOM uint32_t XTALCFG_SET;                   /**< Crystal Configuration Register                     */
  uint32_t       RESERVED12[1U];                /**< Reserved for future use                            */
  __IOM uint32_t XTALCTRL_SET;                  /**< Crystal Control Register                           */
  uint32_t       RESERVED13[1U];                /**< Reserved for future use                            */
  __IOM uint32_t CFG_SET;                       /**< Configuration Register                             */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
  __IOM uint32_t SLEEPYXTALCFG0_SET;            /**< Sleepy Crystal Configuration 0                     */
  __IOM uint32_t SLEEPYXTALCFG1_SET;            /**< Sleepy Crystal Configuration 1                     */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  uint32_t       RESERVED15[3U];                /**< Reserved for future use                            */
  __IOM uint32_t EXTCLKPKDETCTRL_SET;           /**< Peak Detector Control Register                     */
  uint32_t       RESERVED16[4U];                /**< Reserved for future use                            */
  __IOM uint32_t CMD_SET;                       /**< Command Register                                   */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  __IM uint32_t  AVGSTARTUPTIME_SET;            /**< Average Startup Time Register                      */
  uint32_t       RESERVED17[4U];                /**< Reserved for future use                            */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED18[2U];                /**< Reserved for future use                            */
  __IOM uint32_t LOCK_SET;                      /**< Configuration Lock Register                        */
  uint32_t       RESERVED19[7U];                /**< Reserved for future use                            */
  uint32_t       RESERVED20[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED21[983U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP version ID                                      */
  uint32_t       RESERVED22[3U];                /**< Reserved for future use                            */
  __IOM uint32_t XTALCFG_CLR;                   /**< Crystal Configuration Register                     */
  uint32_t       RESERVED23[1U];                /**< Reserved for future use                            */
  __IOM uint32_t XTALCTRL_CLR;                  /**< Crystal Control Register                           */
  uint32_t       RESERVED24[1U];                /**< Reserved for future use                            */
  __IOM uint32_t CFG_CLR;                       /**< Configuration Register                             */
  uint32_t       RESERVED25[1U];                /**< Reserved for future use                            */
  __IOM uint32_t SLEEPYXTALCFG0_CLR;            /**< Sleepy Crystal Configuration 0                     */
  __IOM uint32_t SLEEPYXTALCFG1_CLR;            /**< Sleepy Crystal Configuration 1                     */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  uint32_t       RESERVED26[3U];                /**< Reserved for future use                            */
  __IOM uint32_t EXTCLKPKDETCTRL_CLR;           /**< Peak Detector Control Register                     */
  uint32_t       RESERVED27[4U];                /**< Reserved for future use                            */
  __IOM uint32_t CMD_CLR;                       /**< Command Register                                   */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  __IM uint32_t  AVGSTARTUPTIME_CLR;            /**< Average Startup Time Register                      */
  uint32_t       RESERVED28[4U];                /**< Reserved for future use                            */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED29[2U];                /**< Reserved for future use                            */
  __IOM uint32_t LOCK_CLR;                      /**< Configuration Lock Register                        */
  uint32_t       RESERVED30[7U];                /**< Reserved for future use                            */
  uint32_t       RESERVED31[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED32[983U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP version ID                                      */
  uint32_t       RESERVED33[3U];                /**< Reserved for future use                            */
  __IOM uint32_t XTALCFG_TGL;                   /**< Crystal Configuration Register                     */
  uint32_t       RESERVED34[1U];                /**< Reserved for future use                            */
  __IOM uint32_t XTALCTRL_TGL;                  /**< Crystal Control Register                           */
  uint32_t       RESERVED35[1U];                /**< Reserved for future use                            */
  __IOM uint32_t CFG_TGL;                       /**< Configuration Register                             */
  uint32_t       RESERVED36[1U];                /**< Reserved for future use                            */
  __IOM uint32_t SLEEPYXTALCFG0_TGL;            /**< Sleepy Crystal Configuration 0                     */
  __IOM uint32_t SLEEPYXTALCFG1_TGL;            /**< Sleepy Crystal Configuration 1                     */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  uint32_t       RESERVED37[3U];                /**< Reserved for future use                            */
  __IOM uint32_t EXTCLKPKDETCTRL_TGL;           /**< Peak Detector Control Register                     */
  uint32_t       RESERVED38[4U];                /**< Reserved for future use                            */
  __IOM uint32_t CMD_TGL;                       /**< Command Register                                   */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  __IM uint32_t  AVGSTARTUPTIME_TGL;            /**< Average Startup Time Register                      */
  uint32_t       RESERVED39[4U];                /**< Reserved for future use                            */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED40[2U];                /**< Reserved for future use                            */
  __IOM uint32_t LOCK_TGL;                      /**< Configuration Lock Register                        */
  uint32_t       RESERVED41[7U];                /**< Reserved for future use                            */
  uint32_t       RESERVED42[1U];                /**< Reserved for future use                            */
} HFXO_TypeDef;
/** @} End of group SIBG301_HFXO */

/**************************************************************************//**
 * @addtogroup SIBG301_HFXO
 * @{
 * @defgroup SIBG301_HFXO_BitFields HFXO Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for HFXO IPVERSION */
#define _HFXO_IPVERSION_RESETVALUE                         0x00000005UL                             /**< Default value for HFXO_IPVERSION            */
#define _HFXO_IPVERSION_MASK                               0xFFFFFFFFUL                             /**< Mask for HFXO_IPVERSION                     */
#define _HFXO_IPVERSION_IPVERSION_SHIFT                    0                                        /**< Shift value for HFXO_IPVERSION              */
#define _HFXO_IPVERSION_IPVERSION_MASK                     0xFFFFFFFFUL                             /**< Bit mask for HFXO_IPVERSION                 */
#define _HFXO_IPVERSION_IPVERSION_DEFAULT                  0x00000005UL                             /**< Mode DEFAULT for HFXO_IPVERSION             */
#define HFXO_IPVERSION_IPVERSION_DEFAULT                   (_HFXO_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for HFXO_IPVERSION     */

/* Bit fields for HFXO XTALCFG */
#define _HFXO_XTALCFG_RESETVALUE                           0x2BC00208UL                                  /**< Default value for HFXO_XTALCFG              */
#define _HFXO_XTALCFG_MASK                                 0xFFF773CFUL                                  /**< Mask for HFXO_XTALCFG                       */
#define _HFXO_XTALCFG_COREBIASSTARTUPI_SHIFT               0                                             /**< Shift value for HFXO_COREBIASSTARTUPI       */
#define _HFXO_XTALCFG_COREBIASSTARTUPI_MASK                0xFUL                                         /**< Bit mask for HFXO_COREBIASSTARTUPI          */
#define _HFXO_XTALCFG_COREBIASSTARTUPI_DEFAULT             0x00000008UL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define HFXO_XTALCFG_COREBIASSTARTUPI_DEFAULT              (_HFXO_XTALCFG_COREBIASSTARTUPI_DEFAULT << 0) /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define _HFXO_XTALCFG_COREBIASSTARTUP_SHIFT                6                                             /**< Shift value for HFXO_COREBIASSTARTUP        */
#define _HFXO_XTALCFG_COREBIASSTARTUP_MASK                 0x3C0UL                                       /**< Bit mask for HFXO_COREBIASSTARTUP           */
#define _HFXO_XTALCFG_COREBIASSTARTUP_DEFAULT              0x00000008UL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define HFXO_XTALCFG_COREBIASSTARTUP_DEFAULT               (_HFXO_XTALCFG_COREBIASSTARTUP_DEFAULT << 6)  /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define _HFXO_XTALCFG_CTUNEXISTARTUP_SHIFT                 12                                            /**< Shift value for HFXO_CTUNEXISTARTUP         */
#define _HFXO_XTALCFG_CTUNEXISTARTUP_MASK                  0x7000UL                                      /**< Bit mask for HFXO_CTUNEXISTARTUP            */
#define _HFXO_XTALCFG_CTUNEXISTARTUP_DEFAULT               0x00000000UL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define HFXO_XTALCFG_CTUNEXISTARTUP_DEFAULT                (_HFXO_XTALCFG_CTUNEXISTARTUP_DEFAULT << 12)  /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define _HFXO_XTALCFG_CTUNEXOSTARTUP_SHIFT                 16                                            /**< Shift value for HFXO_CTUNEXOSTARTUP         */
#define _HFXO_XTALCFG_CTUNEXOSTARTUP_MASK                  0x70000UL                                     /**< Bit mask for HFXO_CTUNEXOSTARTUP            */
#define _HFXO_XTALCFG_CTUNEXOSTARTUP_DEFAULT               0x00000000UL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define HFXO_XTALCFG_CTUNEXOSTARTUP_DEFAULT                (_HFXO_XTALCFG_CTUNEXOSTARTUP_DEFAULT << 16)  /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_SHIFT                  20                                            /**< Shift value for HFXO_TIMEOUTSTEADY          */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_MASK                   0xF00000UL                                    /**< Bit mask for HFXO_TIMEOUTSTEADY             */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_DEFAULT                0x0000000CUL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T4US                   0x00000000UL                                  /**< Mode T4US for HFXO_XTALCFG                  */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T16US                  0x00000001UL                                  /**< Mode T16US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T41US                  0x00000002UL                                  /**< Mode T41US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T83US                  0x00000003UL                                  /**< Mode T83US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T125US                 0x00000004UL                                  /**< Mode T125US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T166US                 0x00000005UL                                  /**< Mode T166US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T208US                 0x00000006UL                                  /**< Mode T208US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T250US                 0x00000007UL                                  /**< Mode T250US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T333US                 0x00000008UL                                  /**< Mode T333US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T416US                 0x00000009UL                                  /**< Mode T416US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T500US                 0x0000000AUL                                  /**< Mode T500US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T666US                 0x0000000BUL                                  /**< Mode T666US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T833US                 0x0000000CUL                                  /**< Mode T833US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T1666US                0x0000000DUL                                  /**< Mode T1666US for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T2500US                0x0000000EUL                                  /**< Mode T2500US for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TIMEOUTSTEADY_T4166US                0x0000000FUL                                  /**< Mode T4166US for HFXO_XTALCFG               */
#define HFXO_XTALCFG_TIMEOUTSTEADY_DEFAULT                 (_HFXO_XTALCFG_TIMEOUTSTEADY_DEFAULT << 20)   /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T4US                    (_HFXO_XTALCFG_TIMEOUTSTEADY_T4US << 20)      /**< Shifted mode T4US for HFXO_XTALCFG          */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T16US                   (_HFXO_XTALCFG_TIMEOUTSTEADY_T16US << 20)     /**< Shifted mode T16US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T41US                   (_HFXO_XTALCFG_TIMEOUTSTEADY_T41US << 20)     /**< Shifted mode T41US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T83US                   (_HFXO_XTALCFG_TIMEOUTSTEADY_T83US << 20)     /**< Shifted mode T83US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T125US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T125US << 20)    /**< Shifted mode T125US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T166US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T166US << 20)    /**< Shifted mode T166US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T208US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T208US << 20)    /**< Shifted mode T208US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T250US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T250US << 20)    /**< Shifted mode T250US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T333US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T333US << 20)    /**< Shifted mode T333US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T416US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T416US << 20)    /**< Shifted mode T416US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T500US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T500US << 20)    /**< Shifted mode T500US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T666US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T666US << 20)    /**< Shifted mode T666US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T833US                  (_HFXO_XTALCFG_TIMEOUTSTEADY_T833US << 20)    /**< Shifted mode T833US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T1666US                 (_HFXO_XTALCFG_TIMEOUTSTEADY_T1666US << 20)   /**< Shifted mode T1666US for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T2500US                 (_HFXO_XTALCFG_TIMEOUTSTEADY_T2500US << 20)   /**< Shifted mode T2500US for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TIMEOUTSTEADY_T4166US                 (_HFXO_XTALCFG_TIMEOUTSTEADY_T4166US << 20)   /**< Shifted mode T4166US for HFXO_XTALCFG       */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_SHIFT                   24                                            /**< Shift value for HFXO_TIMEOUTCBLSB           */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_MASK                    0xF000000UL                                   /**< Bit mask for HFXO_TIMEOUTCBLSB              */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_DEFAULT                 0x0000000BUL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T8US                    0x00000000UL                                  /**< Mode T8US for HFXO_XTALCFG                  */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T20US                   0x00000001UL                                  /**< Mode T20US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T41US                   0x00000002UL                                  /**< Mode T41US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T62US                   0x00000003UL                                  /**< Mode T62US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T83US                   0x00000004UL                                  /**< Mode T83US for HFXO_XTALCFG                 */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T104US                  0x00000005UL                                  /**< Mode T104US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T125US                  0x00000006UL                                  /**< Mode T125US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T166US                  0x00000007UL                                  /**< Mode T166US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T208US                  0x00000008UL                                  /**< Mode T208US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T250US                  0x00000009UL                                  /**< Mode T250US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T333US                  0x0000000AUL                                  /**< Mode T333US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T416US                  0x0000000BUL                                  /**< Mode T416US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T833US                  0x0000000CUL                                  /**< Mode T833US for HFXO_XTALCFG                */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T1250US                 0x0000000DUL                                  /**< Mode T1250US for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T2083US                 0x0000000EUL                                  /**< Mode T2083US for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TIMEOUTCBLSB_T3750US                 0x0000000FUL                                  /**< Mode T3750US for HFXO_XTALCFG               */
#define HFXO_XTALCFG_TIMEOUTCBLSB_DEFAULT                  (_HFXO_XTALCFG_TIMEOUTCBLSB_DEFAULT << 24)    /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T8US                     (_HFXO_XTALCFG_TIMEOUTCBLSB_T8US << 24)       /**< Shifted mode T8US for HFXO_XTALCFG          */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T20US                    (_HFXO_XTALCFG_TIMEOUTCBLSB_T20US << 24)      /**< Shifted mode T20US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T41US                    (_HFXO_XTALCFG_TIMEOUTCBLSB_T41US << 24)      /**< Shifted mode T41US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T62US                    (_HFXO_XTALCFG_TIMEOUTCBLSB_T62US << 24)      /**< Shifted mode T62US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T83US                    (_HFXO_XTALCFG_TIMEOUTCBLSB_T83US << 24)      /**< Shifted mode T83US for HFXO_XTALCFG         */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T104US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T104US << 24)     /**< Shifted mode T104US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T125US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T125US << 24)     /**< Shifted mode T125US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T166US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T166US << 24)     /**< Shifted mode T166US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T208US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T208US << 24)     /**< Shifted mode T208US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T250US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T250US << 24)     /**< Shifted mode T250US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T333US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T333US << 24)     /**< Shifted mode T333US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T416US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T416US << 24)     /**< Shifted mode T416US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T833US                   (_HFXO_XTALCFG_TIMEOUTCBLSB_T833US << 24)     /**< Shifted mode T833US for HFXO_XTALCFG        */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T1250US                  (_HFXO_XTALCFG_TIMEOUTCBLSB_T1250US << 24)    /**< Shifted mode T1250US for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T2083US                  (_HFXO_XTALCFG_TIMEOUTCBLSB_T2083US << 24)    /**< Shifted mode T2083US for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TIMEOUTCBLSB_T3750US                  (_HFXO_XTALCFG_TIMEOUTCBLSB_T3750US << 24)    /**< Shifted mode T3750US for HFXO_XTALCFG       */
#define _HFXO_XTALCFG_TINJ_SHIFT                           28                                            /**< Shift value for HFXO_TINJ                   */
#define _HFXO_XTALCFG_TINJ_MASK                            0xF0000000UL                                  /**< Bit mask for HFXO_TINJ                      */
#define _HFXO_XTALCFG_TINJ_DEFAULT                         0x00000002UL                                  /**< Mode DEFAULT for HFXO_XTALCFG               */
#define _HFXO_XTALCFG_TINJ_T10CYCLES                       0x00000000UL                                  /**< Mode T10CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T20CYCLES                       0x00000001UL                                  /**< Mode T20CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T30CYCLES                       0x00000002UL                                  /**< Mode T30CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T40CYCLES                       0x00000003UL                                  /**< Mode T40CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T50CYCLES                       0x00000004UL                                  /**< Mode T50CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T60CYCLES                       0x00000005UL                                  /**< Mode T60CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T70CYCLES                       0x00000006UL                                  /**< Mode T70CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T80CYCLES                       0x00000007UL                                  /**< Mode T80CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T90CYCLES                       0x00000008UL                                  /**< Mode T90CYCLES for HFXO_XTALCFG             */
#define _HFXO_XTALCFG_TINJ_T100CYCLES                      0x00000009UL                                  /**< Mode T100CYCLES for HFXO_XTALCFG            */
#define _HFXO_XTALCFG_TINJ_T110CYCLES                      0x0000000AUL                                  /**< Mode T110CYCLES for HFXO_XTALCFG            */
#define _HFXO_XTALCFG_TINJ_T120CYCLES                      0x0000000BUL                                  /**< Mode T120CYCLES for HFXO_XTALCFG            */
#define _HFXO_XTALCFG_TINJ_T130CYCLES                      0x0000000CUL                                  /**< Mode T130CYCLES for HFXO_XTALCFG            */
#define _HFXO_XTALCFG_TINJ_T140CYCLES                      0x0000000DUL                                  /**< Mode T140CYCLES for HFXO_XTALCFG            */
#define _HFXO_XTALCFG_TINJ_T180CYCLES                      0x0000000EUL                                  /**< Mode T180CYCLES for HFXO_XTALCFG            */
#define _HFXO_XTALCFG_TINJ_T210CYCLES                      0x0000000FUL                                  /**< Mode T210CYCLES for HFXO_XTALCFG            */
#define HFXO_XTALCFG_TINJ_DEFAULT                          (_HFXO_XTALCFG_TINJ_DEFAULT << 28)            /**< Shifted mode DEFAULT for HFXO_XTALCFG       */
#define HFXO_XTALCFG_TINJ_T10CYCLES                        (_HFXO_XTALCFG_TINJ_T10CYCLES << 28)          /**< Shifted mode T10CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T20CYCLES                        (_HFXO_XTALCFG_TINJ_T20CYCLES << 28)          /**< Shifted mode T20CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T30CYCLES                        (_HFXO_XTALCFG_TINJ_T30CYCLES << 28)          /**< Shifted mode T30CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T40CYCLES                        (_HFXO_XTALCFG_TINJ_T40CYCLES << 28)          /**< Shifted mode T40CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T50CYCLES                        (_HFXO_XTALCFG_TINJ_T50CYCLES << 28)          /**< Shifted mode T50CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T60CYCLES                        (_HFXO_XTALCFG_TINJ_T60CYCLES << 28)          /**< Shifted mode T60CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T70CYCLES                        (_HFXO_XTALCFG_TINJ_T70CYCLES << 28)          /**< Shifted mode T70CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T80CYCLES                        (_HFXO_XTALCFG_TINJ_T80CYCLES << 28)          /**< Shifted mode T80CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T90CYCLES                        (_HFXO_XTALCFG_TINJ_T90CYCLES << 28)          /**< Shifted mode T90CYCLES for HFXO_XTALCFG     */
#define HFXO_XTALCFG_TINJ_T100CYCLES                       (_HFXO_XTALCFG_TINJ_T100CYCLES << 28)         /**< Shifted mode T100CYCLES for HFXO_XTALCFG    */
#define HFXO_XTALCFG_TINJ_T110CYCLES                       (_HFXO_XTALCFG_TINJ_T110CYCLES << 28)         /**< Shifted mode T110CYCLES for HFXO_XTALCFG    */
#define HFXO_XTALCFG_TINJ_T120CYCLES                       (_HFXO_XTALCFG_TINJ_T120CYCLES << 28)         /**< Shifted mode T120CYCLES for HFXO_XTALCFG    */
#define HFXO_XTALCFG_TINJ_T130CYCLES                       (_HFXO_XTALCFG_TINJ_T130CYCLES << 28)         /**< Shifted mode T130CYCLES for HFXO_XTALCFG    */
#define HFXO_XTALCFG_TINJ_T140CYCLES                       (_HFXO_XTALCFG_TINJ_T140CYCLES << 28)         /**< Shifted mode T140CYCLES for HFXO_XTALCFG    */
#define HFXO_XTALCFG_TINJ_T180CYCLES                       (_HFXO_XTALCFG_TINJ_T180CYCLES << 28)         /**< Shifted mode T180CYCLES for HFXO_XTALCFG    */
#define HFXO_XTALCFG_TINJ_T210CYCLES                       (_HFXO_XTALCFG_TINJ_T210CYCLES << 28)         /**< Shifted mode T210CYCLES for HFXO_XTALCFG    */

/* Bit fields for HFXO XTALCTRL */
#define _HFXO_XTALCTRL_RESETVALUE                          0x02404078UL                                   /**< Default value for HFXO_XTALCTRL             */
#define _HFXO_XTALCTRL_MASK                                0x87FFFFFFUL                                   /**< Mask for HFXO_XTALCTRL                      */
#define _HFXO_XTALCTRL_COREBIASANA_SHIFT                   0                                              /**< Shift value for HFXO_COREBIASANA            */
#define _HFXO_XTALCTRL_COREBIASANA_MASK                    0x1FFUL                                        /**< Bit mask for HFXO_COREBIASANA               */
#define _HFXO_XTALCTRL_COREBIASANA_DEFAULT                 0x00000078UL                                   /**< Mode DEFAULT for HFXO_XTALCTRL              */
#define HFXO_XTALCTRL_COREBIASANA_DEFAULT                  (_HFXO_XTALCTRL_COREBIASANA_DEFAULT << 0)      /**< Shifted mode DEFAULT for HFXO_XTALCTRL      */
#define _HFXO_XTALCTRL_CTUNEXIANA_SHIFT                    9                                              /**< Shift value for HFXO_CTUNEXIANA             */
#define _HFXO_XTALCTRL_CTUNEXIANA_MASK                     0x1FE00UL                                      /**< Bit mask for HFXO_CTUNEXIANA                */
#define _HFXO_XTALCTRL_CTUNEXIANA_DEFAULT                  0x00000020UL                                   /**< Mode DEFAULT for HFXO_XTALCTRL              */
#define HFXO_XTALCTRL_CTUNEXIANA_DEFAULT                   (_HFXO_XTALCTRL_CTUNEXIANA_DEFAULT << 9)       /**< Shifted mode DEFAULT for HFXO_XTALCTRL      */
#define _HFXO_XTALCTRL_CTUNEXOANA_SHIFT                    17                                             /**< Shift value for HFXO_CTUNEXOANA             */
#define _HFXO_XTALCTRL_CTUNEXOANA_MASK                     0x1FE0000UL                                    /**< Bit mask for HFXO_CTUNEXOANA                */
#define _HFXO_XTALCTRL_CTUNEXOANA_DEFAULT                  0x00000020UL                                   /**< Mode DEFAULT for HFXO_XTALCTRL              */
#define HFXO_XTALCTRL_CTUNEXOANA_DEFAULT                   (_HFXO_XTALCTRL_CTUNEXOANA_DEFAULT << 17)      /**< Shifted mode DEFAULT for HFXO_XTALCTRL      */
#define _HFXO_XTALCTRL_CTUNEFIXANA_SHIFT                   25                                             /**< Shift value for HFXO_CTUNEFIXANA            */
#define _HFXO_XTALCTRL_CTUNEFIXANA_MASK                    0x6000000UL                                    /**< Bit mask for HFXO_CTUNEFIXANA               */
#define _HFXO_XTALCTRL_CTUNEFIXANA_DEFAULT                 0x00000001UL                                   /**< Mode DEFAULT for HFXO_XTALCTRL              */
#define _HFXO_XTALCTRL_CTUNEFIXANA_NONE                    0x00000000UL                                   /**< Mode NONE for HFXO_XTALCTRL                 */
#define _HFXO_XTALCTRL_CTUNEFIXANA_SEL0                    0x00000001UL                                   /**< Mode SEL0 for HFXO_XTALCTRL                 */
#define _HFXO_XTALCTRL_CTUNEFIXANA_SEL1                    0x00000002UL                                   /**< Mode SEL1 for HFXO_XTALCTRL                 */
#define _HFXO_XTALCTRL_CTUNEFIXANA_SEL2                    0x00000003UL                                   /**< Mode SEL2 for HFXO_XTALCTRL                 */
#define HFXO_XTALCTRL_CTUNEFIXANA_DEFAULT                  (_HFXO_XTALCTRL_CTUNEFIXANA_DEFAULT << 25)     /**< Shifted mode DEFAULT for HFXO_XTALCTRL      */
#define HFXO_XTALCTRL_CTUNEFIXANA_NONE                     (_HFXO_XTALCTRL_CTUNEFIXANA_NONE << 25)        /**< Shifted mode NONE for HFXO_XTALCTRL         */
#define HFXO_XTALCTRL_CTUNEFIXANA_SEL0                     (_HFXO_XTALCTRL_CTUNEFIXANA_SEL0 << 25)        /**< Shifted mode SEL0 for HFXO_XTALCTRL         */
#define HFXO_XTALCTRL_CTUNEFIXANA_SEL1                     (_HFXO_XTALCTRL_CTUNEFIXANA_SEL1 << 25)        /**< Shifted mode SEL1 for HFXO_XTALCTRL         */
#define HFXO_XTALCTRL_CTUNEFIXANA_SEL2                     (_HFXO_XTALCTRL_CTUNEFIXANA_SEL2 << 25)        /**< Shifted mode SEL2 for HFXO_XTALCTRL         */
#define HFXO_XTALCTRL_SKIPCOREBIASOPT                      (0x1UL << 31)                                  /**< Skip Core Bias Optimization                 */
#define _HFXO_XTALCTRL_SKIPCOREBIASOPT_SHIFT               31                                             /**< Shift value for HFXO_SKIPCOREBIASOPT        */
#define _HFXO_XTALCTRL_SKIPCOREBIASOPT_MASK                0x80000000UL                                   /**< Bit mask for HFXO_SKIPCOREBIASOPT           */
#define _HFXO_XTALCTRL_SKIPCOREBIASOPT_DEFAULT             0x00000000UL                                   /**< Mode DEFAULT for HFXO_XTALCTRL              */
#define HFXO_XTALCTRL_SKIPCOREBIASOPT_DEFAULT              (_HFXO_XTALCTRL_SKIPCOREBIASOPT_DEFAULT << 31) /**< Shifted mode DEFAULT for HFXO_XTALCTRL      */

/* Bit fields for HFXO CFG */
#define _HFXO_CFG_RESETVALUE                               0x18000000UL                                   /**< Default value for HFXO_CFG                  */
#define _HFXO_CFG_MASK                                     0xF8000F07UL                                   /**< Mask for HFXO_CFG                           */
#define _HFXO_CFG_MODE_SHIFT                               0                                              /**< Shift value for HFXO_MODE                   */
#define _HFXO_CFG_MODE_MASK                                0x3UL                                          /**< Bit mask for HFXO_MODE                      */
#define _HFXO_CFG_MODE_DEFAULT                             0x00000000UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define _HFXO_CFG_MODE_XTAL                                0x00000000UL                                   /**< Mode XTAL for HFXO_CFG                      */
#define _HFXO_CFG_MODE_EXTCLK                              0x00000001UL                                   /**< Mode EXTCLK for HFXO_CFG                    */
#define _HFXO_CFG_MODE_EXTCLKPKDET                         0x00000002UL                                   /**< Mode EXTCLKPKDET for HFXO_CFG               */
#define HFXO_CFG_MODE_DEFAULT                              (_HFXO_CFG_MODE_DEFAULT << 0)                  /**< Shifted mode DEFAULT for HFXO_CFG           */
#define HFXO_CFG_MODE_XTAL                                 (_HFXO_CFG_MODE_XTAL << 0)                     /**< Shifted mode XTAL for HFXO_CFG              */
#define HFXO_CFG_MODE_EXTCLK                               (_HFXO_CFG_MODE_EXTCLK << 0)                   /**< Shifted mode EXTCLK for HFXO_CFG            */
#define HFXO_CFG_MODE_EXTCLKPKDET                          (_HFXO_CFG_MODE_EXTCLKPKDET << 0)              /**< Shifted mode EXTCLKPKDET for HFXO_CFG       */
#define HFXO_CFG_ENXIDCBIASANA                             (0x1UL << 2)                                   /**< Enable XI Internal DC Bias                  */
#define _HFXO_CFG_ENXIDCBIASANA_SHIFT                      2                                              /**< Shift value for HFXO_ENXIDCBIASANA          */
#define _HFXO_CFG_ENXIDCBIASANA_MASK                       0x4UL                                          /**< Bit mask for HFXO_ENXIDCBIASANA             */
#define _HFXO_CFG_ENXIDCBIASANA_DEFAULT                    0x00000000UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define HFXO_CFG_ENXIDCBIASANA_DEFAULT                     (_HFXO_CFG_ENXIDCBIASANA_DEFAULT << 2)         /**< Shifted mode DEFAULT for HFXO_CFG           */
#define HFXO_CFG_SLEEPYXTALSUPEN                           (0x1UL << 8)                                   /**< Enable sleepy crystal startup logic         */
#define _HFXO_CFG_SLEEPYXTALSUPEN_SHIFT                    8                                              /**< Shift value for HFXO_SLEEPYXTALSUPEN        */
#define _HFXO_CFG_SLEEPYXTALSUPEN_MASK                     0x100UL                                        /**< Bit mask for HFXO_SLEEPYXTALSUPEN           */
#define _HFXO_CFG_SLEEPYXTALSUPEN_DEFAULT                  0x00000000UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define HFXO_CFG_SLEEPYXTALSUPEN_DEFAULT                   (_HFXO_CFG_SLEEPYXTALSUPEN_DEFAULT << 8)       /**< Shifted mode DEFAULT for HFXO_CFG           */
#define HFXO_CFG_STUPMEASEN                                (0x1UL << 9)                                   /**< Enable startup time measurement logic       */
#define _HFXO_CFG_STUPMEASEN_SHIFT                         9                                              /**< Shift value for HFXO_STUPMEASEN             */
#define _HFXO_CFG_STUPMEASEN_MASK                          0x200UL                                        /**< Bit mask for HFXO_STUPMEASEN                */
#define _HFXO_CFG_STUPMEASEN_DEFAULT                       0x00000000UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define HFXO_CFG_STUPMEASEN_DEFAULT                        (_HFXO_CFG_STUPMEASEN_DEFAULT << 9)            /**< Shifted mode DEFAULT for HFXO_CFG           */
#define _HFXO_CFG_NUMSTUPMEAS_SHIFT                        10                                             /**< Shift value for HFXO_NUMSTUPMEAS            */
#define _HFXO_CFG_NUMSTUPMEAS_MASK                         0xC00UL                                        /**< Bit mask for HFXO_NUMSTUPMEAS               */
#define _HFXO_CFG_NUMSTUPMEAS_DEFAULT                      0x00000000UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define _HFXO_CFG_NUMSTUPMEAS_MEASURE1                     0x00000000UL                                   /**< Mode MEASURE1 for HFXO_CFG                  */
#define _HFXO_CFG_NUMSTUPMEAS_MEASURE8                     0x00000001UL                                   /**< Mode MEASURE8 for HFXO_CFG                  */
#define _HFXO_CFG_NUMSTUPMEAS_MEASURE16                    0x00000002UL                                   /**< Mode MEASURE16 for HFXO_CFG                 */
#define HFXO_CFG_NUMSTUPMEAS_DEFAULT                       (_HFXO_CFG_NUMSTUPMEAS_DEFAULT << 10)          /**< Shifted mode DEFAULT for HFXO_CFG           */
#define HFXO_CFG_NUMSTUPMEAS_MEASURE1                      (_HFXO_CFG_NUMSTUPMEAS_MEASURE1 << 10)         /**< Shifted mode MEASURE1 for HFXO_CFG          */
#define HFXO_CFG_NUMSTUPMEAS_MEASURE8                      (_HFXO_CFG_NUMSTUPMEAS_MEASURE8 << 10)         /**< Shifted mode MEASURE8 for HFXO_CFG          */
#define HFXO_CFG_NUMSTUPMEAS_MEASURE16                     (_HFXO_CFG_NUMSTUPMEAS_MEASURE16 << 10)        /**< Shifted mode MEASURE16 for HFXO_CFG         */
#define HFXO_CFG_FORCELFTIMEOUTSYSRTC                      (0x1UL << 27)                                  /**< Force SYSRTC Low Frequency Timeout          */
#define _HFXO_CFG_FORCELFTIMEOUTSYSRTC_SHIFT               27                                             /**< Shift value for HFXO_FORCELFTIMEOUTSYSRTC   */
#define _HFXO_CFG_FORCELFTIMEOUTSYSRTC_MASK                0x8000000UL                                    /**< Bit mask for HFXO_FORCELFTIMEOUTSYSRTC      */
#define _HFXO_CFG_FORCELFTIMEOUTSYSRTC_DEFAULT             0x00000001UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define HFXO_CFG_FORCELFTIMEOUTSYSRTC_DEFAULT              (_HFXO_CFG_FORCELFTIMEOUTSYSRTC_DEFAULT << 27) /**< Shifted mode DEFAULT for HFXO_CFG           */
#define HFXO_CFG_FORCELFTIMEOUTPRS                         (0x1UL << 28)                                  /**< Force PRS Low Frequency Timeout             */
#define _HFXO_CFG_FORCELFTIMEOUTPRS_SHIFT                  28                                             /**< Shift value for HFXO_FORCELFTIMEOUTPRS      */
#define _HFXO_CFG_FORCELFTIMEOUTPRS_MASK                   0x10000000UL                                   /**< Bit mask for HFXO_FORCELFTIMEOUTPRS         */
#define _HFXO_CFG_FORCELFTIMEOUTPRS_DEFAULT                0x00000001UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define HFXO_CFG_FORCELFTIMEOUTPRS_DEFAULT                 (_HFXO_CFG_FORCELFTIMEOUTPRS_DEFAULT << 28)    /**< Shifted mode DEFAULT for HFXO_CFG           */
#define HFXO_CFG_SQBUFENSTARTUPI                           (0x1UL << 30)                                  /**< Early enable for Squaring Buffer            */
#define _HFXO_CFG_SQBUFENSTARTUPI_SHIFT                    30                                             /**< Shift value for HFXO_SQBUFENSTARTUPI        */
#define _HFXO_CFG_SQBUFENSTARTUPI_MASK                     0x40000000UL                                   /**< Bit mask for HFXO_SQBUFENSTARTUPI           */
#define _HFXO_CFG_SQBUFENSTARTUPI_DEFAULT                  0x00000000UL                                   /**< Mode DEFAULT for HFXO_CFG                   */
#define HFXO_CFG_SQBUFENSTARTUPI_DEFAULT                   (_HFXO_CFG_SQBUFENSTARTUPI_DEFAULT << 30)      /**< Shifted mode DEFAULT for HFXO_CFG           */

/* Bit fields for HFXO SLEEPYXTALCFG0 */
#define _HFXO_SLEEPYXTALCFG0_RESETVALUE                    0x20200518UL                                          /**< Default value for HFXO_SLEEPYXTALCFG0       */
#define _HFXO_SLEEPYXTALCFG0_MASK                          0x77770FFFUL                                          /**< Mask for HFXO_SLEEPYXTALCFG0                */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHSUPSLEEPY_SHIFT        0                                                     /**< Shift value for HFXO_PKDETTHSUPSLEEPY       */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHSUPSLEEPY_MASK         0xFUL                                                 /**< Bit mask for HFXO_PKDETTHSUPSLEEPY          */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHSUPSLEEPY_DEFAULT      0x00000008UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_PKDETTHSUPSLEEPY_DEFAULT       (_HFXO_SLEEPYXTALCFG0_PKDETTHSUPSLEEPY_DEFAULT << 0)  /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/
#define _HFXO_SLEEPYXTALCFG0_PKDETTHSUPISLEEPY_SHIFT       4                                                     /**< Shift value for HFXO_PKDETTHSUPISLEEPY      */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHSUPISLEEPY_MASK        0xF0UL                                                /**< Bit mask for HFXO_PKDETTHSUPISLEEPY         */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHSUPISLEEPY_DEFAULT     0x00000001UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_PKDETTHSUPISLEEPY_DEFAULT      (_HFXO_SLEEPYXTALCFG0_PKDETTHSUPISLEEPY_DEFAULT << 4) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/
#define _HFXO_SLEEPYXTALCFG0_PKDETTHANASLEEPY_SHIFT        8                                                     /**< Shift value for HFXO_PKDETTHANASLEEPY       */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHANASLEEPY_MASK         0xF00UL                                               /**< Bit mask for HFXO_PKDETTHANASLEEPY          */
#define _HFXO_SLEEPYXTALCFG0_PKDETTHANASLEEPY_DEFAULT      0x00000005UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_PKDETTHANASLEEPY_DEFAULT       (_HFXO_SLEEPYXTALCFG0_PKDETTHANASLEEPY_DEFAULT << 8)  /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/
#define _HFXO_SLEEPYXTALCFG0_CTUNEXISUPSLEEPY_SHIFT        16                                                    /**< Shift value for HFXO_CTUNEXISUPSLEEPY       */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXISUPSLEEPY_MASK         0x70000UL                                             /**< Bit mask for HFXO_CTUNEXISUPSLEEPY          */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXISUPSLEEPY_DEFAULT      0x00000000UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_CTUNEXISUPSLEEPY_DEFAULT       (_HFXO_SLEEPYXTALCFG0_CTUNEXISUPSLEEPY_DEFAULT << 16) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/
#define _HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_SHIFT        20                                                    /**< Shift value for HFXO_CTUNEXIANASLEEPY       */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_MASK         0x700000UL                                            /**< Bit mask for HFXO_CTUNEXIANASLEEPY          */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_DEFAULT      0x00000002UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_DEFAULT       (_HFXO_SLEEPYXTALCFG0_CTUNEXIANASLEEPY_DEFAULT << 20) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/
#define _HFXO_SLEEPYXTALCFG0_CTUNEXOSUPSLEEPY_SHIFT        24                                                    /**< Shift value for HFXO_CTUNEXOSUPSLEEPY       */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXOSUPSLEEPY_MASK         0x7000000UL                                           /**< Bit mask for HFXO_CTUNEXOSUPSLEEPY          */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXOSUPSLEEPY_DEFAULT      0x00000000UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_CTUNEXOSUPSLEEPY_DEFAULT       (_HFXO_SLEEPYXTALCFG0_CTUNEXOSUPSLEEPY_DEFAULT << 24) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/
#define _HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_SHIFT        28                                                    /**< Shift value for HFXO_CTUNEXOANASLEEPY       */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_MASK         0x70000000UL                                          /**< Bit mask for HFXO_CTUNEXOANASLEEPY          */
#define _HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_DEFAULT      0x00000002UL                                          /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG0        */
#define HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_DEFAULT       (_HFXO_SLEEPYXTALCFG0_CTUNEXOANASLEEPY_DEFAULT << 28) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG0*/

/* Bit fields for HFXO SLEEPYXTALCFG1 */
#define _HFXO_SLEEPYXTALCFG1_RESETVALUE                    0x07FFFFFFUL                                           /**< Default value for HFXO_SLEEPYXTALCFG1       */
#define _HFXO_SLEEPYXTALCFG1_MASK                          0x07FFFFFFUL                                           /**< Mask for HFXO_SLEEPYXTALCFG1                */
#define _HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_SHIFT       0                                                      /**< Shift value for HFXO_COREBIASSUPSLEEPY      */
#define _HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_MASK        0x1FFUL                                                /**< Bit mask for HFXO_COREBIASSUPSLEEPY         */
#define _HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_DEFAULT     0x000001FFUL                                           /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG1        */
#define HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_DEFAULT      (_HFXO_SLEEPYXTALCFG1_COREBIASSUPSLEEPY_DEFAULT << 0)  /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG1*/
#define _HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_SHIFT      9                                                      /**< Shift value for HFXO_COREBIASSUPISLEEPY     */
#define _HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_MASK       0x3FE00UL                                              /**< Bit mask for HFXO_COREBIASSUPISLEEPY        */
#define _HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_DEFAULT    0x000001FFUL                                           /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG1        */
#define HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_DEFAULT     (_HFXO_SLEEPYXTALCFG1_COREBIASSUPISLEEPY_DEFAULT << 9) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG1*/
#define _HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_SHIFT       18                                                     /**< Shift value for HFXO_COREBIASANASLEEPY      */
#define _HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_MASK        0x7FC0000UL                                            /**< Bit mask for HFXO_COREBIASANASLEEPY         */
#define _HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_DEFAULT     0x000001FFUL                                           /**< Mode DEFAULT for HFXO_SLEEPYXTALCFG1        */
#define HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_DEFAULT      (_HFXO_SLEEPYXTALCFG1_COREBIASANASLEEPY_DEFAULT << 18) /**< Shifted mode DEFAULT for HFXO_SLEEPYXTALCFG1*/

/* Bit fields for HFXO CTRL */
#define _HFXO_CTRL_RESETVALUE                              0x0B000040UL                                    /**< Default value for HFXO_CTRL                 */
#define _HFXO_CTRL_MASK                                    0x8B0BFF7CUL                                    /**< Mask for HFXO_CTRL                          */
#define HFXO_CTRL_KEEPWARM                                 (0x1UL << 2)                                    /**< Keep Warm                                   */
#define _HFXO_CTRL_KEEPWARM_SHIFT                          2                                               /**< Shift value for HFXO_KEEPWARM               */
#define _HFXO_CTRL_KEEPWARM_MASK                           0x4UL                                           /**< Bit mask for HFXO_KEEPWARM                  */
#define _HFXO_CTRL_KEEPWARM_DEFAULT                        0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_KEEPWARM_DEFAULT                         (_HFXO_CTRL_KEEPWARM_DEFAULT << 2)              /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_FORCEXI2GNDANA                           (0x1UL << 4)                                    /**< Force XI Pin to Ground                      */
#define _HFXO_CTRL_FORCEXI2GNDANA_SHIFT                    4                                               /**< Shift value for HFXO_FORCEXI2GNDANA         */
#define _HFXO_CTRL_FORCEXI2GNDANA_MASK                     0x10UL                                          /**< Bit mask for HFXO_FORCEXI2GNDANA            */
#define _HFXO_CTRL_FORCEXI2GNDANA_DEFAULT                  0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define _HFXO_CTRL_FORCEXI2GNDANA_DISABLE                  0x00000000UL                                    /**< Mode DISABLE for HFXO_CTRL                  */
#define _HFXO_CTRL_FORCEXI2GNDANA_ENABLE                   0x00000001UL                                    /**< Mode ENABLE for HFXO_CTRL                   */
#define HFXO_CTRL_FORCEXI2GNDANA_DEFAULT                   (_HFXO_CTRL_FORCEXI2GNDANA_DEFAULT << 4)        /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_FORCEXI2GNDANA_DISABLE                   (_HFXO_CTRL_FORCEXI2GNDANA_DISABLE << 4)        /**< Shifted mode DISABLE for HFXO_CTRL          */
#define HFXO_CTRL_FORCEXI2GNDANA_ENABLE                    (_HFXO_CTRL_FORCEXI2GNDANA_ENABLE << 4)         /**< Shifted mode ENABLE for HFXO_CTRL           */
#define HFXO_CTRL_FORCEXO2GNDANA                           (0x1UL << 5)                                    /**< Force XO Pin to Ground                      */
#define _HFXO_CTRL_FORCEXO2GNDANA_SHIFT                    5                                               /**< Shift value for HFXO_FORCEXO2GNDANA         */
#define _HFXO_CTRL_FORCEXO2GNDANA_MASK                     0x20UL                                          /**< Bit mask for HFXO_FORCEXO2GNDANA            */
#define _HFXO_CTRL_FORCEXO2GNDANA_DEFAULT                  0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define _HFXO_CTRL_FORCEXO2GNDANA_DISABLE                  0x00000000UL                                    /**< Mode DISABLE for HFXO_CTRL                  */
#define _HFXO_CTRL_FORCEXO2GNDANA_ENABLE                   0x00000001UL                                    /**< Mode ENABLE for HFXO_CTRL                   */
#define HFXO_CTRL_FORCEXO2GNDANA_DEFAULT                   (_HFXO_CTRL_FORCEXO2GNDANA_DEFAULT << 5)        /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_FORCEXO2GNDANA_DISABLE                   (_HFXO_CTRL_FORCEXO2GNDANA_DISABLE << 5)        /**< Shifted mode DISABLE for HFXO_CTRL          */
#define HFXO_CTRL_FORCEXO2GNDANA_ENABLE                    (_HFXO_CTRL_FORCEXO2GNDANA_ENABLE << 5)         /**< Shifted mode ENABLE for HFXO_CTRL           */
#define HFXO_CTRL_FORCECTUNEMAX                            (0x1UL << 6)                                    /**< Force Tuning Cap to Max Value               */
#define _HFXO_CTRL_FORCECTUNEMAX_SHIFT                     6                                               /**< Shift value for HFXO_FORCECTUNEMAX          */
#define _HFXO_CTRL_FORCECTUNEMAX_MASK                      0x40UL                                          /**< Bit mask for HFXO_FORCECTUNEMAX             */
#define _HFXO_CTRL_FORCECTUNEMAX_DEFAULT                   0x00000001UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_FORCECTUNEMAX_DEFAULT                    (_HFXO_CTRL_FORCECTUNEMAX_DEFAULT << 6)         /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define _HFXO_CTRL_PRSSTATUSSEL0_SHIFT                     8                                               /**< Shift value for HFXO_PRSSTATUSSEL0          */
#define _HFXO_CTRL_PRSSTATUSSEL0_MASK                      0xF00UL                                         /**< Bit mask for HFXO_PRSSTATUSSEL0             */
#define _HFXO_CTRL_PRSSTATUSSEL0_DEFAULT                   0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define _HFXO_CTRL_PRSSTATUSSEL0_DISABLED                  0x00000000UL                                    /**< Mode DISABLED for HFXO_CTRL                 */
#define _HFXO_CTRL_PRSSTATUSSEL0_ENS                       0x00000001UL                                    /**< Mode ENS for HFXO_CTRL                      */
#define _HFXO_CTRL_PRSSTATUSSEL0_COREBIASOPTRDY            0x00000002UL                                    /**< Mode COREBIASOPTRDY for HFXO_CTRL           */
#define _HFXO_CTRL_PRSSTATUSSEL0_RDY                       0x00000003UL                                    /**< Mode RDY for HFXO_CTRL                      */
#define _HFXO_CTRL_PRSSTATUSSEL0_PRSRDY                    0x00000004UL                                    /**< Mode PRSRDY for HFXO_CTRL                   */
#define _HFXO_CTRL_PRSSTATUSSEL0_SYSRTCRDY                 0x00000005UL                                    /**< Mode SYSRTCRDY for HFXO_CTRL                */
#define _HFXO_CTRL_PRSSTATUSSEL0_HWREQ                     0x00000009UL                                    /**< Mode HWREQ for HFXO_CTRL                    */
#define _HFXO_CTRL_PRSSTATUSSEL0_PRSHWREQ                  0x0000000AUL                                    /**< Mode PRSHWREQ for HFXO_CTRL                 */
#define _HFXO_CTRL_PRSSTATUSSEL0_SYSRTCHWREQ               0x0000000BUL                                    /**< Mode SYSRTCHWREQ for HFXO_CTRL              */
#define HFXO_CTRL_PRSSTATUSSEL0_DEFAULT                    (_HFXO_CTRL_PRSSTATUSSEL0_DEFAULT << 8)         /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_PRSSTATUSSEL0_DISABLED                   (_HFXO_CTRL_PRSSTATUSSEL0_DISABLED << 8)        /**< Shifted mode DISABLED for HFXO_CTRL         */
#define HFXO_CTRL_PRSSTATUSSEL0_ENS                        (_HFXO_CTRL_PRSSTATUSSEL0_ENS << 8)             /**< Shifted mode ENS for HFXO_CTRL              */
#define HFXO_CTRL_PRSSTATUSSEL0_COREBIASOPTRDY             (_HFXO_CTRL_PRSSTATUSSEL0_COREBIASOPTRDY << 8)  /**< Shifted mode COREBIASOPTRDY for HFXO_CTRL   */
#define HFXO_CTRL_PRSSTATUSSEL0_RDY                        (_HFXO_CTRL_PRSSTATUSSEL0_RDY << 8)             /**< Shifted mode RDY for HFXO_CTRL              */
#define HFXO_CTRL_PRSSTATUSSEL0_PRSRDY                     (_HFXO_CTRL_PRSSTATUSSEL0_PRSRDY << 8)          /**< Shifted mode PRSRDY for HFXO_CTRL           */
#define HFXO_CTRL_PRSSTATUSSEL0_SYSRTCRDY                  (_HFXO_CTRL_PRSSTATUSSEL0_SYSRTCRDY << 8)       /**< Shifted mode SYSRTCRDY for HFXO_CTRL        */
#define HFXO_CTRL_PRSSTATUSSEL0_HWREQ                      (_HFXO_CTRL_PRSSTATUSSEL0_HWREQ << 8)           /**< Shifted mode HWREQ for HFXO_CTRL            */
#define HFXO_CTRL_PRSSTATUSSEL0_PRSHWREQ                   (_HFXO_CTRL_PRSSTATUSSEL0_PRSHWREQ << 8)        /**< Shifted mode PRSHWREQ for HFXO_CTRL         */
#define HFXO_CTRL_PRSSTATUSSEL0_SYSRTCHWREQ                (_HFXO_CTRL_PRSSTATUSSEL0_SYSRTCHWREQ << 8)     /**< Shifted mode SYSRTCHWREQ for HFXO_CTRL      */
#define _HFXO_CTRL_PRSSTATUSSEL1_SHIFT                     12                                              /**< Shift value for HFXO_PRSSTATUSSEL1          */
#define _HFXO_CTRL_PRSSTATUSSEL1_MASK                      0xF000UL                                        /**< Bit mask for HFXO_PRSSTATUSSEL1             */
#define _HFXO_CTRL_PRSSTATUSSEL1_DEFAULT                   0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define _HFXO_CTRL_PRSSTATUSSEL1_DISABLED                  0x00000000UL                                    /**< Mode DISABLED for HFXO_CTRL                 */
#define _HFXO_CTRL_PRSSTATUSSEL1_ENS                       0x00000001UL                                    /**< Mode ENS for HFXO_CTRL                      */
#define _HFXO_CTRL_PRSSTATUSSEL1_COREBIASOPTRDY            0x00000002UL                                    /**< Mode COREBIASOPTRDY for HFXO_CTRL           */
#define _HFXO_CTRL_PRSSTATUSSEL1_RDY                       0x00000003UL                                    /**< Mode RDY for HFXO_CTRL                      */
#define _HFXO_CTRL_PRSSTATUSSEL1_PRSRDY                    0x00000004UL                                    /**< Mode PRSRDY for HFXO_CTRL                   */
#define _HFXO_CTRL_PRSSTATUSSEL1_SYSRTCRDY                 0x00000005UL                                    /**< Mode SYSRTCRDY for HFXO_CTRL                */
#define _HFXO_CTRL_PRSSTATUSSEL1_HWREQ                     0x00000009UL                                    /**< Mode HWREQ for HFXO_CTRL                    */
#define _HFXO_CTRL_PRSSTATUSSEL1_PRSHWREQ                  0x0000000AUL                                    /**< Mode PRSHWREQ for HFXO_CTRL                 */
#define _HFXO_CTRL_PRSSTATUSSEL1_SYSRTCHWREQ               0x0000000BUL                                    /**< Mode SYSRTCHWREQ for HFXO_CTRL              */
#define HFXO_CTRL_PRSSTATUSSEL1_DEFAULT                    (_HFXO_CTRL_PRSSTATUSSEL1_DEFAULT << 12)        /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_PRSSTATUSSEL1_DISABLED                   (_HFXO_CTRL_PRSSTATUSSEL1_DISABLED << 12)       /**< Shifted mode DISABLED for HFXO_CTRL         */
#define HFXO_CTRL_PRSSTATUSSEL1_ENS                        (_HFXO_CTRL_PRSSTATUSSEL1_ENS << 12)            /**< Shifted mode ENS for HFXO_CTRL              */
#define HFXO_CTRL_PRSSTATUSSEL1_COREBIASOPTRDY             (_HFXO_CTRL_PRSSTATUSSEL1_COREBIASOPTRDY << 12) /**< Shifted mode COREBIASOPTRDY for HFXO_CTRL   */
#define HFXO_CTRL_PRSSTATUSSEL1_RDY                        (_HFXO_CTRL_PRSSTATUSSEL1_RDY << 12)            /**< Shifted mode RDY for HFXO_CTRL              */
#define HFXO_CTRL_PRSSTATUSSEL1_PRSRDY                     (_HFXO_CTRL_PRSSTATUSSEL1_PRSRDY << 12)         /**< Shifted mode PRSRDY for HFXO_CTRL           */
#define HFXO_CTRL_PRSSTATUSSEL1_SYSRTCRDY                  (_HFXO_CTRL_PRSSTATUSSEL1_SYSRTCRDY << 12)      /**< Shifted mode SYSRTCRDY for HFXO_CTRL        */
#define HFXO_CTRL_PRSSTATUSSEL1_HWREQ                      (_HFXO_CTRL_PRSSTATUSSEL1_HWREQ << 12)          /**< Shifted mode HWREQ for HFXO_CTRL            */
#define HFXO_CTRL_PRSSTATUSSEL1_PRSHWREQ                   (_HFXO_CTRL_PRSSTATUSSEL1_PRSHWREQ << 12)       /**< Shifted mode PRSHWREQ for HFXO_CTRL         */
#define HFXO_CTRL_PRSSTATUSSEL1_SYSRTCHWREQ                (_HFXO_CTRL_PRSSTATUSSEL1_SYSRTCHWREQ << 12)    /**< Shifted mode SYSRTCHWREQ for HFXO_CTRL      */
#define HFXO_CTRL_FORCEEN                                  (0x1UL << 16)                                   /**< Force Digital Clock Request                 */
#define _HFXO_CTRL_FORCEEN_SHIFT                           16                                              /**< Shift value for HFXO_FORCEEN                */
#define _HFXO_CTRL_FORCEEN_MASK                            0x10000UL                                       /**< Bit mask for HFXO_FORCEEN                   */
#define _HFXO_CTRL_FORCEEN_DEFAULT                         0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_FORCEEN_DEFAULT                          (_HFXO_CTRL_FORCEEN_DEFAULT << 16)              /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_FORCEENPRS                               (0x1UL << 17)                                   /**< Force PRS Oscillator Request                */
#define _HFXO_CTRL_FORCEENPRS_SHIFT                        17                                              /**< Shift value for HFXO_FORCEENPRS             */
#define _HFXO_CTRL_FORCEENPRS_MASK                         0x20000UL                                       /**< Bit mask for HFXO_FORCEENPRS                */
#define _HFXO_CTRL_FORCEENPRS_DEFAULT                      0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_FORCEENPRS_DEFAULT                       (_HFXO_CTRL_FORCEENPRS_DEFAULT << 17)           /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_FORCEENSYSRTC                            (0x1UL << 19)                                   /**< Force SYSRTC Oscillator Request             */
#define _HFXO_CTRL_FORCEENSYSRTC_SHIFT                     19                                              /**< Shift value for HFXO_FORCEENSYSRTC          */
#define _HFXO_CTRL_FORCEENSYSRTC_MASK                      0x80000UL                                       /**< Bit mask for HFXO_FORCEENSYSRTC             */
#define _HFXO_CTRL_FORCEENSYSRTC_DEFAULT                   0x00000000UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_FORCEENSYSRTC_DEFAULT                    (_HFXO_CTRL_FORCEENSYSRTC_DEFAULT << 19)        /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_DISONDEMAND                              (0x1UL << 24)                                   /**< Disable On-demand For Digital Clock         */
#define _HFXO_CTRL_DISONDEMAND_SHIFT                       24                                              /**< Shift value for HFXO_DISONDEMAND            */
#define _HFXO_CTRL_DISONDEMAND_MASK                        0x1000000UL                                     /**< Bit mask for HFXO_DISONDEMAND               */
#define _HFXO_CTRL_DISONDEMAND_DEFAULT                     0x00000001UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_DISONDEMAND_DEFAULT                      (_HFXO_CTRL_DISONDEMAND_DEFAULT << 24)          /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_DISONDEMANDPRS                           (0x1UL << 25)                                   /**< Disable On-demand For PRS                   */
#define _HFXO_CTRL_DISONDEMANDPRS_SHIFT                    25                                              /**< Shift value for HFXO_DISONDEMANDPRS         */
#define _HFXO_CTRL_DISONDEMANDPRS_MASK                     0x2000000UL                                     /**< Bit mask for HFXO_DISONDEMANDPRS            */
#define _HFXO_CTRL_DISONDEMANDPRS_DEFAULT                  0x00000001UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_DISONDEMANDPRS_DEFAULT                   (_HFXO_CTRL_DISONDEMANDPRS_DEFAULT << 25)       /**< Shifted mode DEFAULT for HFXO_CTRL          */
#define HFXO_CTRL_DISONDEMANDSYSRTC                        (0x1UL << 27)                                   /**< Disable On-demand For SYSRTC                */
#define _HFXO_CTRL_DISONDEMANDSYSRTC_SHIFT                 27                                              /**< Shift value for HFXO_DISONDEMANDSYSRTC      */
#define _HFXO_CTRL_DISONDEMANDSYSRTC_MASK                  0x8000000UL                                     /**< Bit mask for HFXO_DISONDEMANDSYSRTC         */
#define _HFXO_CTRL_DISONDEMANDSYSRTC_DEFAULT               0x00000001UL                                    /**< Mode DEFAULT for HFXO_CTRL                  */
#define HFXO_CTRL_DISONDEMANDSYSRTC_DEFAULT                (_HFXO_CTRL_DISONDEMANDSYSRTC_DEFAULT << 27)    /**< Shifted mode DEFAULT for HFXO_CTRL          */

/* Bit fields for HFXO EXTCLKPKDETCTRL */
#define _HFXO_EXTCLKPKDETCTRL_RESETVALUE                   0x00000055UL                                        /**< Default value for HFXO_EXTCLKPKDETCTRL      */
#define _HFXO_EXTCLKPKDETCTRL_MASK                         0x000000FFUL                                        /**< Mask for HFXO_EXTCLKPKDETCTRL               */
#define _HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK0_SHIFT         0                                                   /**< Shift value for HFXO_PKDETTHEXTCLK0         */
#define _HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK0_MASK          0xFUL                                               /**< Bit mask for HFXO_PKDETTHEXTCLK0            */
#define _HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK0_DEFAULT       0x00000005UL                                        /**< Mode DEFAULT for HFXO_EXTCLKPKDETCTRL       */
#define HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK0_DEFAULT        (_HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK0_DEFAULT << 0) /**< Shifted mode DEFAULT for HFXO_EXTCLKPKDETCTRL*/
#define _HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK1_SHIFT         4                                                   /**< Shift value for HFXO_PKDETTHEXTCLK1         */
#define _HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK1_MASK          0xF0UL                                              /**< Bit mask for HFXO_PKDETTHEXTCLK1            */
#define _HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK1_DEFAULT       0x00000005UL                                        /**< Mode DEFAULT for HFXO_EXTCLKPKDETCTRL       */
#define HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK1_DEFAULT        (_HFXO_EXTCLKPKDETCTRL_PKDETTHEXTCLK1_DEFAULT << 4) /**< Shifted mode DEFAULT for HFXO_EXTCLKPKDETCTRL*/

/* Bit fields for HFXO CMD */
#define _HFXO_CMD_RESETVALUE                               0x00000000UL                         /**< Default value for HFXO_CMD                  */
#define _HFXO_CMD_MASK                                     0x00000007UL                         /**< Mask for HFXO_CMD                           */
#define HFXO_CMD_COREBIASOPT                               (0x1UL << 0)                         /**< Core Bias Optimizaton                       */
#define _HFXO_CMD_COREBIASOPT_SHIFT                        0                                    /**< Shift value for HFXO_COREBIASOPT            */
#define _HFXO_CMD_COREBIASOPT_MASK                         0x1UL                                /**< Bit mask for HFXO_COREBIASOPT               */
#define _HFXO_CMD_COREBIASOPT_DEFAULT                      0x00000000UL                         /**< Mode DEFAULT for HFXO_CMD                   */
#define HFXO_CMD_COREBIASOPT_DEFAULT                       (_HFXO_CMD_COREBIASOPT_DEFAULT << 0) /**< Shifted mode DEFAULT for HFXO_CMD           */
#define HFXO_CMD_STARTMEAS                                 (0x1UL << 1)                         /**< Start measuring crystal startup time        */
#define _HFXO_CMD_STARTMEAS_SHIFT                          1                                    /**< Shift value for HFXO_STARTMEAS              */
#define _HFXO_CMD_STARTMEAS_MASK                           0x2UL                                /**< Bit mask for HFXO_STARTMEAS                 */
#define _HFXO_CMD_STARTMEAS_DEFAULT                        0x00000000UL                         /**< Mode DEFAULT for HFXO_CMD                   */
#define HFXO_CMD_STARTMEAS_DEFAULT                         (_HFXO_CMD_STARTMEAS_DEFAULT << 1)   /**< Shifted mode DEFAULT for HFXO_CMD           */
#define HFXO_CMD_STOPMEAS                                  (0x1UL << 2)                         /**< Stop measuring crystal startup time         */
#define _HFXO_CMD_STOPMEAS_SHIFT                           2                                    /**< Shift value for HFXO_STOPMEAS               */
#define _HFXO_CMD_STOPMEAS_MASK                            0x4UL                                /**< Bit mask for HFXO_STOPMEAS                  */
#define _HFXO_CMD_STOPMEAS_DEFAULT                         0x00000000UL                         /**< Mode DEFAULT for HFXO_CMD                   */
#define HFXO_CMD_STOPMEAS_DEFAULT                          (_HFXO_CMD_STOPMEAS_DEFAULT << 2)    /**< Shifted mode DEFAULT for HFXO_CMD           */

/* Bit fields for HFXO STATUS */
#define _HFXO_STATUS_RESETVALUE                            0x00000000UL                               /**< Default value for HFXO_STATUS               */
#define _HFXO_STATUS_MASK                                  0xF05F0317UL                               /**< Mask for HFXO_STATUS                        */
#define HFXO_STATUS_RDY                                    (0x1UL << 0)                               /**< Ready Status                                */
#define _HFXO_STATUS_RDY_SHIFT                             0                                          /**< Shift value for HFXO_RDY                    */
#define _HFXO_STATUS_RDY_MASK                              0x1UL                                      /**< Bit mask for HFXO_RDY                       */
#define _HFXO_STATUS_RDY_DEFAULT                           0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_RDY_DEFAULT                            (_HFXO_STATUS_RDY_DEFAULT << 0)            /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_COREBIASOPTRDY                         (0x1UL << 1)                               /**< Core Bias Optimization Ready                */
#define _HFXO_STATUS_COREBIASOPTRDY_SHIFT                  1                                          /**< Shift value for HFXO_COREBIASOPTRDY         */
#define _HFXO_STATUS_COREBIASOPTRDY_MASK                   0x2UL                                      /**< Bit mask for HFXO_COREBIASOPTRDY            */
#define _HFXO_STATUS_COREBIASOPTRDY_DEFAULT                0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_COREBIASOPTRDY_DEFAULT                 (_HFXO_STATUS_COREBIASOPTRDY_DEFAULT << 1) /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_PRSRDY                                 (0x1UL << 2)                               /**< PRS Ready Status                            */
#define _HFXO_STATUS_PRSRDY_SHIFT                          2                                          /**< Shift value for HFXO_PRSRDY                 */
#define _HFXO_STATUS_PRSRDY_MASK                           0x4UL                                      /**< Bit mask for HFXO_PRSRDY                    */
#define _HFXO_STATUS_PRSRDY_DEFAULT                        0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_PRSRDY_DEFAULT                         (_HFXO_STATUS_PRSRDY_DEFAULT << 2)         /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_SYSRTCRDY                              (0x1UL << 4)                               /**< SYSRTC Ready Status                         */
#define _HFXO_STATUS_SYSRTCRDY_SHIFT                       4                                          /**< Shift value for HFXO_SYSRTCRDY              */
#define _HFXO_STATUS_SYSRTCRDY_MASK                        0x10UL                                     /**< Bit mask for HFXO_SYSRTCRDY                 */
#define _HFXO_STATUS_SYSRTCRDY_DEFAULT                     0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_SYSRTCRDY_DEFAULT                      (_HFXO_STATUS_SYSRTCRDY_DEFAULT << 4)      /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_SLEEPYXTAL                             (0x1UL << 8)                               /**< Sleepy XTAL settings applied                */
#define _HFXO_STATUS_SLEEPYXTAL_SHIFT                      8                                          /**< Shift value for HFXO_SLEEPYXTAL             */
#define _HFXO_STATUS_SLEEPYXTAL_MASK                       0x100UL                                    /**< Bit mask for HFXO_SLEEPYXTAL                */
#define _HFXO_STATUS_SLEEPYXTAL_DEFAULT                    0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_SLEEPYXTAL_DEFAULT                     (_HFXO_STATUS_SLEEPYXTAL_DEFAULT << 8)     /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_SLEEPYXTALERR                          (0x1UL << 9)                               /**< XTAL Startup Failed with Sleepy Settings    */
#define _HFXO_STATUS_SLEEPYXTALERR_SHIFT                   9                                          /**< Shift value for HFXO_SLEEPYXTALERR          */
#define _HFXO_STATUS_SLEEPYXTALERR_MASK                    0x200UL                                    /**< Bit mask for HFXO_SLEEPYXTALERR             */
#define _HFXO_STATUS_SLEEPYXTALERR_DEFAULT                 0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_SLEEPYXTALERR_DEFAULT                  (_HFXO_STATUS_SLEEPYXTALERR_DEFAULT << 9)  /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_ENS                                    (0x1UL << 16)                              /**< Enabled Status                              */
#define _HFXO_STATUS_ENS_SHIFT                             16                                         /**< Shift value for HFXO_ENS                    */
#define _HFXO_STATUS_ENS_MASK                              0x10000UL                                  /**< Bit mask for HFXO_ENS                       */
#define _HFXO_STATUS_ENS_DEFAULT                           0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_ENS_DEFAULT                            (_HFXO_STATUS_ENS_DEFAULT << 16)           /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_HWREQ                                  (0x1UL << 17)                              /**< Oscillator Requested by Digital Clock       */
#define _HFXO_STATUS_HWREQ_SHIFT                           17                                         /**< Shift value for HFXO_HWREQ                  */
#define _HFXO_STATUS_HWREQ_MASK                            0x20000UL                                  /**< Bit mask for HFXO_HWREQ                     */
#define _HFXO_STATUS_HWREQ_DEFAULT                         0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_HWREQ_DEFAULT                          (_HFXO_STATUS_HWREQ_DEFAULT << 17)         /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_ISWARM                                 (0x1UL << 19)                              /**< Oscillator Is Kept Warm                     */
#define _HFXO_STATUS_ISWARM_SHIFT                          19                                         /**< Shift value for HFXO_ISWARM                 */
#define _HFXO_STATUS_ISWARM_MASK                           0x80000UL                                  /**< Bit mask for HFXO_ISWARM                    */
#define _HFXO_STATUS_ISWARM_DEFAULT                        0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_ISWARM_DEFAULT                         (_HFXO_STATUS_ISWARM_DEFAULT << 19)        /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_PRSHWREQ                               (0x1UL << 20)                              /**< Oscillator Requested by PRS Request         */
#define _HFXO_STATUS_PRSHWREQ_SHIFT                        20                                         /**< Shift value for HFXO_PRSHWREQ               */
#define _HFXO_STATUS_PRSHWREQ_MASK                         0x100000UL                                 /**< Bit mask for HFXO_PRSHWREQ                  */
#define _HFXO_STATUS_PRSHWREQ_DEFAULT                      0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_PRSHWREQ_DEFAULT                       (_HFXO_STATUS_PRSHWREQ_DEFAULT << 20)      /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_SYSRTCHWREQ                            (0x1UL << 22)                              /**< Oscillator Requested by SYSRTC Request      */
#define _HFXO_STATUS_SYSRTCHWREQ_SHIFT                     22                                         /**< Shift value for HFXO_SYSRTCHWREQ            */
#define _HFXO_STATUS_SYSRTCHWREQ_MASK                      0x400000UL                                 /**< Bit mask for HFXO_SYSRTCHWREQ               */
#define _HFXO_STATUS_SYSRTCHWREQ_DEFAULT                   0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_SYSRTCHWREQ_DEFAULT                    (_HFXO_STATUS_SYSRTCHWREQ_DEFAULT << 22)   /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_STUPMEASBSY                            (0x1UL << 28)                              /**< Startup Measurement Busy                    */
#define _HFXO_STATUS_STUPMEASBSY_SHIFT                     28                                         /**< Shift value for HFXO_STUPMEASBSY            */
#define _HFXO_STATUS_STUPMEASBSY_MASK                      0x10000000UL                               /**< Bit mask for HFXO_STUPMEASBSY               */
#define _HFXO_STATUS_STUPMEASBSY_DEFAULT                   0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_STUPMEASBSY_DEFAULT                    (_HFXO_STATUS_STUPMEASBSY_DEFAULT << 28)   /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_INJBSY                                 (0x1UL << 29)                              /**< Clock Injection Busy                        */
#define _HFXO_STATUS_INJBSY_SHIFT                          29                                         /**< Shift value for HFXO_INJBSY                 */
#define _HFXO_STATUS_INJBSY_MASK                           0x20000000UL                               /**< Bit mask for HFXO_INJBSY                    */
#define _HFXO_STATUS_INJBSY_DEFAULT                        0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_INJBSY_DEFAULT                         (_HFXO_STATUS_INJBSY_DEFAULT << 29)        /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_SYNCBUSY                               (0x1UL << 30)                              /**< Sync Busy                                   */
#define _HFXO_STATUS_SYNCBUSY_SHIFT                        30                                         /**< Shift value for HFXO_SYNCBUSY               */
#define _HFXO_STATUS_SYNCBUSY_MASK                         0x40000000UL                               /**< Bit mask for HFXO_SYNCBUSY                  */
#define _HFXO_STATUS_SYNCBUSY_DEFAULT                      0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define HFXO_STATUS_SYNCBUSY_DEFAULT                       (_HFXO_STATUS_SYNCBUSY_DEFAULT << 30)      /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_LOCK                                   (0x1UL << 31)                              /**< Configuration Lock Status                   */
#define _HFXO_STATUS_LOCK_SHIFT                            31                                         /**< Shift value for HFXO_LOCK                   */
#define _HFXO_STATUS_LOCK_MASK                             0x80000000UL                               /**< Bit mask for HFXO_LOCK                      */
#define _HFXO_STATUS_LOCK_DEFAULT                          0x00000000UL                               /**< Mode DEFAULT for HFXO_STATUS                */
#define _HFXO_STATUS_LOCK_UNLOCKED                         0x00000000UL                               /**< Mode UNLOCKED for HFXO_STATUS               */
#define _HFXO_STATUS_LOCK_LOCKED                           0x00000001UL                               /**< Mode LOCKED for HFXO_STATUS                 */
#define HFXO_STATUS_LOCK_DEFAULT                           (_HFXO_STATUS_LOCK_DEFAULT << 31)          /**< Shifted mode DEFAULT for HFXO_STATUS        */
#define HFXO_STATUS_LOCK_UNLOCKED                          (_HFXO_STATUS_LOCK_UNLOCKED << 31)         /**< Shifted mode UNLOCKED for HFXO_STATUS       */
#define HFXO_STATUS_LOCK_LOCKED                            (_HFXO_STATUS_LOCK_LOCKED << 31)           /**< Shifted mode LOCKED for HFXO_STATUS         */

/* Bit fields for HFXO AVGSTARTUPTIME */
#define _HFXO_AVGSTARTUPTIME_RESETVALUE                    0x00000000UL                                /**< Default value for HFXO_AVGSTARTUPTIME       */
#define _HFXO_AVGSTARTUPTIME_MASK                          0x000001FFUL                                /**< Mask for HFXO_AVGSTARTUPTIME                */
#define _HFXO_AVGSTARTUPTIME_AVGSTUP_SHIFT                 0                                           /**< Shift value for HFXO_AVGSTUP                */
#define _HFXO_AVGSTARTUPTIME_AVGSTUP_MASK                  0x1FFUL                                     /**< Bit mask for HFXO_AVGSTUP                   */
#define _HFXO_AVGSTARTUPTIME_AVGSTUP_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for HFXO_AVGSTARTUPTIME        */
#define HFXO_AVGSTARTUPTIME_AVGSTUP_DEFAULT                (_HFXO_AVGSTARTUPTIME_AVGSTUP_DEFAULT << 0) /**< Shifted mode DEFAULT for HFXO_AVGSTARTUPTIME*/

/* Bit fields for HFXO IF */
#define _HFXO_IF_RESETVALUE                                0x00000000UL                              /**< Default value for HFXO_IF                   */
#define _HFXO_IF_MASK                                      0xE6501317UL                              /**< Mask for HFXO_IF                            */
#define HFXO_IF_RDY                                        (0x1UL << 0)                              /**< Digital Clock Ready Interrupt               */
#define _HFXO_IF_RDY_SHIFT                                 0                                         /**< Shift value for HFXO_RDY                    */
#define _HFXO_IF_RDY_MASK                                  0x1UL                                     /**< Bit mask for HFXO_RDY                       */
#define _HFXO_IF_RDY_DEFAULT                               0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_RDY_DEFAULT                                (_HFXO_IF_RDY_DEFAULT << 0)               /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_COREBIASOPTRDY                             (0x1UL << 1)                              /**< Core Bias Optimization Ready Interrupt      */
#define _HFXO_IF_COREBIASOPTRDY_SHIFT                      1                                         /**< Shift value for HFXO_COREBIASOPTRDY         */
#define _HFXO_IF_COREBIASOPTRDY_MASK                       0x2UL                                     /**< Bit mask for HFXO_COREBIASOPTRDY            */
#define _HFXO_IF_COREBIASOPTRDY_DEFAULT                    0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_COREBIASOPTRDY_DEFAULT                     (_HFXO_IF_COREBIASOPTRDY_DEFAULT << 1)    /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_PRSRDY                                     (0x1UL << 2)                              /**< PRS Ready Interrupt                         */
#define _HFXO_IF_PRSRDY_SHIFT                              2                                         /**< Shift value for HFXO_PRSRDY                 */
#define _HFXO_IF_PRSRDY_MASK                               0x4UL                                     /**< Bit mask for HFXO_PRSRDY                    */
#define _HFXO_IF_PRSRDY_DEFAULT                            0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_PRSRDY_DEFAULT                             (_HFXO_IF_PRSRDY_DEFAULT << 2)            /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_SYSRTCRDY                                  (0x1UL << 4)                              /**< SYSRTC Ready Interrupt                      */
#define _HFXO_IF_SYSRTCRDY_SHIFT                           4                                         /**< Shift value for HFXO_SYSRTCRDY              */
#define _HFXO_IF_SYSRTCRDY_MASK                            0x10UL                                    /**< Bit mask for HFXO_SYSRTCRDY                 */
#define _HFXO_IF_SYSRTCRDY_DEFAULT                         0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_SYSRTCRDY_DEFAULT                          (_HFXO_IF_SYSRTCRDY_DEFAULT << 4)         /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_STUPMEASDONE                               (0x1UL << 8)                              /**< Startup Time Measurement Complete           */
#define _HFXO_IF_STUPMEASDONE_SHIFT                        8                                         /**< Shift value for HFXO_STUPMEASDONE           */
#define _HFXO_IF_STUPMEASDONE_MASK                         0x100UL                                   /**< Bit mask for HFXO_STUPMEASDONE              */
#define _HFXO_IF_STUPMEASDONE_DEFAULT                      0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_STUPMEASDONE_DEFAULT                       (_HFXO_IF_STUPMEASDONE_DEFAULT << 8)      /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_SLEEPYXTAL                                 (0x1UL << 9)                              /**< Sleepy crystal settings applied             */
#define _HFXO_IF_SLEEPYXTAL_SHIFT                          9                                         /**< Shift value for HFXO_SLEEPYXTAL             */
#define _HFXO_IF_SLEEPYXTAL_MASK                           0x200UL                                   /**< Bit mask for HFXO_SLEEPYXTAL                */
#define _HFXO_IF_SLEEPYXTAL_DEFAULT                        0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_SLEEPYXTAL_DEFAULT                         (_HFXO_IF_SLEEPYXTAL_DEFAULT << 9)        /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_INJSKIP                                    (0x1UL << 12)                             /**< Injection Skipped                           */
#define _HFXO_IF_INJSKIP_SHIFT                             12                                        /**< Shift value for HFXO_INJSKIP                */
#define _HFXO_IF_INJSKIP_MASK                              0x1000UL                                  /**< Bit mask for HFXO_INJSKIP                   */
#define _HFXO_IF_INJSKIP_DEFAULT                           0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_INJSKIP_DEFAULT                            (_HFXO_IF_INJSKIP_DEFAULT << 12)          /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_PRSERR                                     (0x1UL << 20)                             /**< PRS Requset Error Interrupt                 */
#define _HFXO_IF_PRSERR_SHIFT                              20                                        /**< Shift value for HFXO_PRSERR                 */
#define _HFXO_IF_PRSERR_MASK                               0x100000UL                                /**< Bit mask for HFXO_PRSERR                    */
#define _HFXO_IF_PRSERR_DEFAULT                            0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_PRSERR_DEFAULT                             (_HFXO_IF_PRSERR_DEFAULT << 20)           /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_SYSRTCERR                                  (0x1UL << 22)                             /**< SYSRTC Request Error Interrupt              */
#define _HFXO_IF_SYSRTCERR_SHIFT                           22                                        /**< Shift value for HFXO_SYSRTCERR              */
#define _HFXO_IF_SYSRTCERR_MASK                            0x400000UL                                /**< Bit mask for HFXO_SYSRTCERR                 */
#define _HFXO_IF_SYSRTCERR_DEFAULT                         0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_SYSRTCERR_DEFAULT                          (_HFXO_IF_SYSRTCERR_DEFAULT << 22)        /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_INJERR                                     (0x1UL << 25)                             /**< Injection Error Interrupt                   */
#define _HFXO_IF_INJERR_SHIFT                              25                                        /**< Shift value for HFXO_INJERR                 */
#define _HFXO_IF_INJERR_MASK                               0x2000000UL                               /**< Bit mask for HFXO_INJERR                    */
#define _HFXO_IF_INJERR_DEFAULT                            0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_INJERR_DEFAULT                             (_HFXO_IF_INJERR_DEFAULT << 25)           /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_DNSSLEEPYXTALERR                           (0x1UL << 26)                             /**< Did Not Start Sleepy Xtal Error IRQ         */
#define _HFXO_IF_DNSSLEEPYXTALERR_SHIFT                    26                                        /**< Shift value for HFXO_DNSSLEEPYXTALERR       */
#define _HFXO_IF_DNSSLEEPYXTALERR_MASK                     0x4000000UL                               /**< Bit mask for HFXO_DNSSLEEPYXTALERR          */
#define _HFXO_IF_DNSSLEEPYXTALERR_DEFAULT                  0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_DNSSLEEPYXTALERR_DEFAULT                   (_HFXO_IF_DNSSLEEPYXTALERR_DEFAULT << 26) /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_DNSERR                                     (0x1UL << 29)                             /**< Did Not Start Error Interrupt               */
#define _HFXO_IF_DNSERR_SHIFT                              29                                        /**< Shift value for HFXO_DNSERR                 */
#define _HFXO_IF_DNSERR_MASK                               0x20000000UL                              /**< Bit mask for HFXO_DNSERR                    */
#define _HFXO_IF_DNSERR_DEFAULT                            0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_DNSERR_DEFAULT                             (_HFXO_IF_DNSERR_DEFAULT << 29)           /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_LFTIMEOUTERR                               (0x1UL << 30)                             /**< Low Frequency Timeout Error Interrupt       */
#define _HFXO_IF_LFTIMEOUTERR_SHIFT                        30                                        /**< Shift value for HFXO_LFTIMEOUTERR           */
#define _HFXO_IF_LFTIMEOUTERR_MASK                         0x40000000UL                              /**< Bit mask for HFXO_LFTIMEOUTERR              */
#define _HFXO_IF_LFTIMEOUTERR_DEFAULT                      0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_LFTIMEOUTERR_DEFAULT                       (_HFXO_IF_LFTIMEOUTERR_DEFAULT << 30)     /**< Shifted mode DEFAULT for HFXO_IF            */
#define HFXO_IF_COREBIASOPTERR                             (0x1UL << 31)                             /**< Core Bias Optimization Error Interrupt      */
#define _HFXO_IF_COREBIASOPTERR_SHIFT                      31                                        /**< Shift value for HFXO_COREBIASOPTERR         */
#define _HFXO_IF_COREBIASOPTERR_MASK                       0x80000000UL                              /**< Bit mask for HFXO_COREBIASOPTERR            */
#define _HFXO_IF_COREBIASOPTERR_DEFAULT                    0x00000000UL                              /**< Mode DEFAULT for HFXO_IF                    */
#define HFXO_IF_COREBIASOPTERR_DEFAULT                     (_HFXO_IF_COREBIASOPTERR_DEFAULT << 31)   /**< Shifted mode DEFAULT for HFXO_IF            */

/* Bit fields for HFXO IEN */
#define _HFXO_IEN_RESETVALUE                               0x00000000UL                               /**< Default value for HFXO_IEN                  */
#define _HFXO_IEN_MASK                                     0xE6501317UL                               /**< Mask for HFXO_IEN                           */
#define HFXO_IEN_RDY                                       (0x1UL << 0)                               /**< Digital Clock Ready Interrupt               */
#define _HFXO_IEN_RDY_SHIFT                                0                                          /**< Shift value for HFXO_RDY                    */
#define _HFXO_IEN_RDY_MASK                                 0x1UL                                      /**< Bit mask for HFXO_RDY                       */
#define _HFXO_IEN_RDY_DEFAULT                              0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_RDY_DEFAULT                               (_HFXO_IEN_RDY_DEFAULT << 0)               /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_COREBIASOPTRDY                            (0x1UL << 1)                               /**< Core Bias Optimization Ready Interrupt      */
#define _HFXO_IEN_COREBIASOPTRDY_SHIFT                     1                                          /**< Shift value for HFXO_COREBIASOPTRDY         */
#define _HFXO_IEN_COREBIASOPTRDY_MASK                      0x2UL                                      /**< Bit mask for HFXO_COREBIASOPTRDY            */
#define _HFXO_IEN_COREBIASOPTRDY_DEFAULT                   0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_COREBIASOPTRDY_DEFAULT                    (_HFXO_IEN_COREBIASOPTRDY_DEFAULT << 1)    /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_PRSRDY                                    (0x1UL << 2)                               /**< PRS Ready Interrupt                         */
#define _HFXO_IEN_PRSRDY_SHIFT                             2                                          /**< Shift value for HFXO_PRSRDY                 */
#define _HFXO_IEN_PRSRDY_MASK                              0x4UL                                      /**< Bit mask for HFXO_PRSRDY                    */
#define _HFXO_IEN_PRSRDY_DEFAULT                           0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_PRSRDY_DEFAULT                            (_HFXO_IEN_PRSRDY_DEFAULT << 2)            /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_SYSRTCRDY                                 (0x1UL << 4)                               /**< SYSRTC Ready Interrupt                      */
#define _HFXO_IEN_SYSRTCRDY_SHIFT                          4                                          /**< Shift value for HFXO_SYSRTCRDY              */
#define _HFXO_IEN_SYSRTCRDY_MASK                           0x10UL                                     /**< Bit mask for HFXO_SYSRTCRDY                 */
#define _HFXO_IEN_SYSRTCRDY_DEFAULT                        0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_SYSRTCRDY_DEFAULT                         (_HFXO_IEN_SYSRTCRDY_DEFAULT << 4)         /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_STUPMEASDONE                              (0x1UL << 8)                               /**< Startup Time Measurement Complete           */
#define _HFXO_IEN_STUPMEASDONE_SHIFT                       8                                          /**< Shift value for HFXO_STUPMEASDONE           */
#define _HFXO_IEN_STUPMEASDONE_MASK                        0x100UL                                    /**< Bit mask for HFXO_STUPMEASDONE              */
#define _HFXO_IEN_STUPMEASDONE_DEFAULT                     0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_STUPMEASDONE_DEFAULT                      (_HFXO_IEN_STUPMEASDONE_DEFAULT << 8)      /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_SLEEPYXTAL                                (0x1UL << 9)                               /**< Sleepy crystal settings applied             */
#define _HFXO_IEN_SLEEPYXTAL_SHIFT                         9                                          /**< Shift value for HFXO_SLEEPYXTAL             */
#define _HFXO_IEN_SLEEPYXTAL_MASK                          0x200UL                                    /**< Bit mask for HFXO_SLEEPYXTAL                */
#define _HFXO_IEN_SLEEPYXTAL_DEFAULT                       0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_SLEEPYXTAL_DEFAULT                        (_HFXO_IEN_SLEEPYXTAL_DEFAULT << 9)        /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_INJSKIP                                   (0x1UL << 12)                              /**< Injection Skipped                           */
#define _HFXO_IEN_INJSKIP_SHIFT                            12                                         /**< Shift value for HFXO_INJSKIP                */
#define _HFXO_IEN_INJSKIP_MASK                             0x1000UL                                   /**< Bit mask for HFXO_INJSKIP                   */
#define _HFXO_IEN_INJSKIP_DEFAULT                          0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_INJSKIP_DEFAULT                           (_HFXO_IEN_INJSKIP_DEFAULT << 12)          /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_PRSERR                                    (0x1UL << 20)                              /**< PRS Requset Error Interrupt                 */
#define _HFXO_IEN_PRSERR_SHIFT                             20                                         /**< Shift value for HFXO_PRSERR                 */
#define _HFXO_IEN_PRSERR_MASK                              0x100000UL                                 /**< Bit mask for HFXO_PRSERR                    */
#define _HFXO_IEN_PRSERR_DEFAULT                           0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_PRSERR_DEFAULT                            (_HFXO_IEN_PRSERR_DEFAULT << 20)           /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_SYSRTCERR                                 (0x1UL << 22)                              /**< SYSRTC Request Error Interrupt              */
#define _HFXO_IEN_SYSRTCERR_SHIFT                          22                                         /**< Shift value for HFXO_SYSRTCERR              */
#define _HFXO_IEN_SYSRTCERR_MASK                           0x400000UL                                 /**< Bit mask for HFXO_SYSRTCERR                 */
#define _HFXO_IEN_SYSRTCERR_DEFAULT                        0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_SYSRTCERR_DEFAULT                         (_HFXO_IEN_SYSRTCERR_DEFAULT << 22)        /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_INJERR                                    (0x1UL << 25)                              /**< Injection Error Interrupt                   */
#define _HFXO_IEN_INJERR_SHIFT                             25                                         /**< Shift value for HFXO_INJERR                 */
#define _HFXO_IEN_INJERR_MASK                              0x2000000UL                                /**< Bit mask for HFXO_INJERR                    */
#define _HFXO_IEN_INJERR_DEFAULT                           0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_INJERR_DEFAULT                            (_HFXO_IEN_INJERR_DEFAULT << 25)           /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_DNSSLEEPYXTALERR                          (0x1UL << 26)                              /**< Did Not Start Sleepy Xtal Error IRQ         */
#define _HFXO_IEN_DNSSLEEPYXTALERR_SHIFT                   26                                         /**< Shift value for HFXO_DNSSLEEPYXTALERR       */
#define _HFXO_IEN_DNSSLEEPYXTALERR_MASK                    0x4000000UL                                /**< Bit mask for HFXO_DNSSLEEPYXTALERR          */
#define _HFXO_IEN_DNSSLEEPYXTALERR_DEFAULT                 0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_DNSSLEEPYXTALERR_DEFAULT                  (_HFXO_IEN_DNSSLEEPYXTALERR_DEFAULT << 26) /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_DNSERR                                    (0x1UL << 29)                              /**< Did Not Start Error Interrupt               */
#define _HFXO_IEN_DNSERR_SHIFT                             29                                         /**< Shift value for HFXO_DNSERR                 */
#define _HFXO_IEN_DNSERR_MASK                              0x20000000UL                               /**< Bit mask for HFXO_DNSERR                    */
#define _HFXO_IEN_DNSERR_DEFAULT                           0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_DNSERR_DEFAULT                            (_HFXO_IEN_DNSERR_DEFAULT << 29)           /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_LFTIMEOUTERR                              (0x1UL << 30)                              /**< Low Frequency Timeout Error Interrupt       */
#define _HFXO_IEN_LFTIMEOUTERR_SHIFT                       30                                         /**< Shift value for HFXO_LFTIMEOUTERR           */
#define _HFXO_IEN_LFTIMEOUTERR_MASK                        0x40000000UL                               /**< Bit mask for HFXO_LFTIMEOUTERR              */
#define _HFXO_IEN_LFTIMEOUTERR_DEFAULT                     0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_LFTIMEOUTERR_DEFAULT                      (_HFXO_IEN_LFTIMEOUTERR_DEFAULT << 30)     /**< Shifted mode DEFAULT for HFXO_IEN           */
#define HFXO_IEN_COREBIASOPTERR                            (0x1UL << 31)                              /**< Core Bias Optimization Error Interrupt      */
#define _HFXO_IEN_COREBIASOPTERR_SHIFT                     31                                         /**< Shift value for HFXO_COREBIASOPTERR         */
#define _HFXO_IEN_COREBIASOPTERR_MASK                      0x80000000UL                               /**< Bit mask for HFXO_COREBIASOPTERR            */
#define _HFXO_IEN_COREBIASOPTERR_DEFAULT                   0x00000000UL                               /**< Mode DEFAULT for HFXO_IEN                   */
#define HFXO_IEN_COREBIASOPTERR_DEFAULT                    (_HFXO_IEN_COREBIASOPTERR_DEFAULT << 31)   /**< Shifted mode DEFAULT for HFXO_IEN           */

/* Bit fields for HFXO LOCK */
#define _HFXO_LOCK_RESETVALUE                              0x0000580EUL                      /**< Default value for HFXO_LOCK                 */
#define _HFXO_LOCK_MASK                                    0x0000FFFFUL                      /**< Mask for HFXO_LOCK                          */
#define _HFXO_LOCK_LOCKKEY_SHIFT                           0                                 /**< Shift value for HFXO_LOCKKEY                */
#define _HFXO_LOCK_LOCKKEY_MASK                            0xFFFFUL                          /**< Bit mask for HFXO_LOCKKEY                   */
#define _HFXO_LOCK_LOCKKEY_DEFAULT                         0x0000580EUL                      /**< Mode DEFAULT for HFXO_LOCK                  */
#define _HFXO_LOCK_LOCKKEY_UNLOCK                          0x0000580EUL                      /**< Mode UNLOCK for HFXO_LOCK                   */
#define HFXO_LOCK_LOCKKEY_DEFAULT                          (_HFXO_LOCK_LOCKKEY_DEFAULT << 0) /**< Shifted mode DEFAULT for HFXO_LOCK          */
#define HFXO_LOCK_LOCKKEY_UNLOCK                           (_HFXO_LOCK_LOCKKEY_UNLOCK << 0)  /**< Shifted mode UNLOCK for HFXO_LOCK           */

/** @} End of group SIBG301_HFXO_BitFields */
/** @} End of group SIBG301_HFXO */
/** @} End of group Parts */

#endif // SIBG301_HFXO_H
