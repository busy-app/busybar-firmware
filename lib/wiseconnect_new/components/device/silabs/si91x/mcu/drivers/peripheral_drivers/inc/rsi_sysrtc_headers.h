/******************************************************************************
* @file  rsi_sysrtc_header.h
* @brief SYSRTC register and bit field definitions
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
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
******************************************************************************/
#ifndef SYSRTC_DEVICE_9117_H
#define SYSRTC_DEVICE_9117_H
#define SYSRTC_HAS_SET_CLEAR

#define SYSRTC0_GROUP0_ALTIRQDIS 0x1UL /**> Group 0 Alternate IRQ disable */
#define SYSRTC0_GROUP0_CAPDIS    0x0UL /**> Group 0 Capture disable */
#define SYSRTC0_GROUP0_CMP1DIS   0x0UL /**> Group 0 Compare1 disable */
#define SYSRTC0_GROUP0_DIS       0x0UL /**> Group 0 Disable */
#define SYSRTC0_GROUP0_ROOTDIS   0x1UL /**> Group 0 ROOT disable */
#define SYSRTC0_GROUP1_ALTIRQDIS 0x1UL /**> Group 1 Alternate IRQ disable */
#define SYSRTC0_GROUP1_CAPDIS    0x0UL /**> Group 1 Capture disable */
#define SYSRTC0_GROUP1_CMP1DIS   0x0UL /**> Group 1 Compare1 disable */
#define SYSRTC0_GROUP1_DIS       0x0UL /**> Group 1 Disable */
#define SYSRTC0_GROUP1_ROOTDIS   0x1UL /**> Group 1 ROOT disable */
#define SYSRTC0_GROUP2_ALTIRQDIS 0x1UL /**> Group 2 Alternate IRQ disable */
#define SYSRTC0_GROUP2_CAPDIS    0x0UL /**> Group 2 Capture disable */
#define SYSRTC0_GROUP2_CMP1DIS   0x0UL /**> Group 2 Compare1 disable */
#define SYSRTC0_GROUP2_DIS       0x1UL /**> Group 2 Disable */
#define SYSRTC0_GROUP2_ROOTDIS   0x1UL /**> Group 2 ROOT disable */
#define SYSRTC0_GROUP3_ALTIRQDIS 0x1UL /**> Group 3 Alternate IRQ disable */
#define SYSRTC0_GROUP3_CAPDIS    0x1UL /**> Group 3 Capture disable */
#define SYSRTC0_GROUP3_CMP1DIS   0x1UL /**> Group 3 Compare1 disable */
#define SYSRTC0_GROUP3_DIS       0x1UL /**> Group 3 Disable */
#define SYSRTC0_GROUP3_ROOTDIS   0x1UL /**> Group 3 ROOT disable */
#define SYSRTC0_GROUP4_ALTIRQDIS 0x1UL /**> Group 4 Alternate IRQ disable */
#define SYSRTC0_GROUP4_CAPDIS    0x1UL /**> Group 4 Capture disable */
#define SYSRTC0_GROUP4_CMP1DIS   0x1UL /**> Group 4 Compare1 disable */
#define SYSRTC0_GROUP4_DIS       0x1UL /**> Group 4 Disable */
#define SYSRTC0_GROUP4_ROOTDIS   0x1UL /**> Group 4 ROOT disable */
#define SYSRTC0_GROUP5_ALTIRQDIS 0x1UL /**> Group 5 Alternate IRQ disable */
#define SYSRTC0_GROUP5_CAPDIS    0x1UL /**> Group 5 Capture disable */
#define SYSRTC0_GROUP5_CMP1DIS   0x1UL /**> Group 5 Compare1 disable */
#define SYSRTC0_GROUP5_DIS       0x1UL /**> Group 5 Disable */
#define SYSRTC0_GROUP5_ROOTDIS   0x1UL /**> Group 5 ROOT disable */
#define SYSRTC0_GROUP6_ALTIRQDIS 0x1UL /**> Group 6 Alternate IRQ disable */
#define SYSRTC0_GROUP6_CAPDIS    0x1UL /**> Group 6 Capture disable */
#define SYSRTC0_GROUP6_CMP1DIS   0x1UL /**> Group 6 Compare1 disable */
#define SYSRTC0_GROUP6_DIS       0x1UL /**> Group 6 Disable */
#define SYSRTC0_GROUP6_ROOTDIS   0x1UL /**> Group 6 ROOT disable */
#define SYSRTC0_GROUP7_ALTIRQDIS 0x1UL /**> Group 7 Alternate IRQ disable */
#define SYSRTC0_GROUP7_CAPDIS    0x1UL /**> Group 7 Capture disable */
#define SYSRTC0_GROUP7_CMP1DIS   0x1UL /**> Group 7 Compare1 disable */
#define SYSRTC0_GROUP7_DIS       0x1UL /**> Group 7 Disable */
#define SYSRTC0_GROUP7_ROOTDIS   0x1UL /**> Group 7 ROOT disable */
#define SYSRTC0_ROOTDIS          0x1UL /**> ROOT disable */

/**************************************************************************/ /**
* @addtogroup Parts
* @{
******************************************************************************/

/**************************************************************************/ /**
 * @addtogroup RSI_DEVICE_SYSRTC
 * @{
 * @defgroup RSI_DEVICE_SYSRTC_BitFields SYSRTC Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SYSRTC IPVERSION */
#define _SYSRTC_IPVERSION_RESETVALUE        0x00000001UL /**< Default value for SYSRTC_IPVERSION          */
#define _SYSRTC_IPVERSION_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_IPVERSION                   */
#define _SYSRTC_IPVERSION_IPVERSION_SHIFT   0            /**< Shift value for SYSRTC_IPVERSION            */
#define _SYSRTC_IPVERSION_IPVERSION_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_IPVERSION               */
#define _SYSRTC_IPVERSION_IPVERSION_DEFAULT 0x00000001UL /**< Mode DEFAULT for SYSRTC_IPVERSION           */
#define SYSRTC_IPVERSION_IPVERSION_DEFAULT \
  (_SYSRTC_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_IPVERSION   */

/* Bit fields for SYSRTC EN */
#define _SYSRTC_EN_RESETVALUE        0x00000000UL                 /**< Default value for SYSRTC_EN                 */
#define _SYSRTC_EN_MASK              0x00000003UL                 /**< Mask for SYSRTC_EN                          */
#define SYSRTC_EN_EN                 (0x1UL << 0)                 /**< SYSRTC Enable                               */
#define _SYSRTC_EN_EN_SHIFT          0                            /**< Shift value for SYSRTC_EN                   */
#define _SYSRTC_EN_EN_MASK           0x1UL                        /**< Bit mask for SYSRTC_EN                      */
#define _SYSRTC_EN_EN_DEFAULT        0x00000000UL                 /**< Mode DEFAULT for SYSRTC_EN                  */
#define SYSRTC_EN_EN_DEFAULT         (_SYSRTC_EN_EN_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_EN          */
#define SYSRTC_EN_DISABLING          (0x1UL << 1)                 /**< Disablement busy status                     */
#define _SYSRTC_EN_DISABLING_SHIFT   1                            /**< Shift value for SYSRTC_DISABLING            */
#define _SYSRTC_EN_DISABLING_MASK    0x2UL                        /**< Bit mask for SYSRTC_DISABLING               */
#define _SYSRTC_EN_DISABLING_DEFAULT 0x00000000UL                 /**< Mode DEFAULT for SYSRTC_EN                  */
#define SYSRTC_EN_DISABLING_DEFAULT \
  (_SYSRTC_EN_DISABLING_DEFAULT << 1)     /**< Shifted mode DEFAULT for SYSRTC_EN          */
#define _SYSRTC_EN_SYNC_MASK 0x2UL        /**< Bit mask for SYSRTC_EN Sync                     */
#define _SYSRTC_EN_SYNC      (0x1UL << 1) /**< Value  for SYSRTC_EN Sync                    */

/* Bit fields for SYSRTC SWRST */
#define _SYSRTC_SWRST_RESETVALUE    0x00000000UL /**< Default value for SYSRTC_SWRST              */
#define _SYSRTC_SWRST_MASK          0x00000003UL /**< Mask for SYSRTC_SWRST                       */
#define SYSRTC_SWRST_SWRST          (0x1UL << 0) /**< Software reset command                      */
#define _SYSRTC_SWRST_SWRST_SHIFT   0            /**< Shift value for SYSRTC_SWRST                */
#define _SYSRTC_SWRST_SWRST_MASK    0x1UL        /**< Bit mask for SYSRTC_SWRST                   */
#define _SYSRTC_SWRST_SWRST_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_SWRST               */
#define SYSRTC_SWRST_SWRST_DEFAULT \
  (_SYSRTC_SWRST_SWRST_DEFAULT << 0)                 /**< Shifted mode DEFAULT for SYSRTC_SWRST       */
#define SYSRTC_SWRST_RESETTING          (0x1UL << 1) /**< Software reset busy status                  */
#define _SYSRTC_SWRST_RESETTING_SHIFT   1            /**< Shift value for SYSRTC_RESETTING            */
#define _SYSRTC_SWRST_RESETTING_MASK    0x2UL        /**< Bit mask for SYSRTC_RESETTING               */
#define _SYSRTC_SWRST_RESETTING_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_SWRST               */
#define SYSRTC_SWRST_RESETTING_DEFAULT \
  (_SYSRTC_SWRST_RESETTING_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_SWRST       */

/* Bit fields for SYSRTC CFG */
#define _SYSRTC_CFG_RESETVALUE       0x00000000UL /**< Default value for SYSRTC_CFG                */
#define _SYSRTC_CFG_MASK             0x00000000UL /**< Mask for SYSRTC_CFG                         */
#define SYSRTC_CFG_DEBUGRUN          (0x1UL << 0) /**< Debug Mode Run Enable                       */
#define _SYSRTC_CFG_DEBUGRUN_SHIFT   0            /**< Shift value for SYSRTC_DEBUGRUN             */
#define _SYSRTC_CFG_DEBUGRUN_MASK    0x1UL        /**< Bit mask for SYSRTC_DEBUGRUN                */
#define _SYSRTC_CFG_DEBUGRUN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_CFG                 */
#define _SYSRTC_CFG_DEBUGRUN_DISABLE 0x00000000UL /**< Mode DISABLE for SYSRTC_CFG                 */
#define _SYSRTC_CFG_DEBUGRUN_ENABLE  0x00000001UL /**< Mode ENABLE for SYSRTC_CFG                  */
#define SYSRTC_CFG_DEBUGRUN_DEFAULT \
  (_SYSRTC_CFG_DEBUGRUN_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_CFG         */
#define SYSRTC_CFG_DEBUGRUN_DISABLE \
  (_SYSRTC_CFG_DEBUGRUN_DISABLE << 0) /**< Shifted mode DISABLE for SYSRTC_CFG         */
#define SYSRTC_CFG_DEBUGRUN_ENABLE \
  (_SYSRTC_CFG_DEBUGRUN_ENABLE << 0) /**< Shifted mode ENABLE for SYSRTC_CFG          */

/* Bit fields for SYSRTC CMD */
#define _SYSRTC_CMD_RESETVALUE    0x00000000UL                     /**< Default value for SYSRTC_CMD                */
#define _SYSRTC_CMD_MASK          0x00000003UL                     /**< Mask for SYSRTC_CMD                         */
#define SYSRTC_CMD_START          (0x1UL << 0)                     /**< Start SYSRTC                                */
#define _SYSRTC_CMD_START_SHIFT   0                                /**< Shift value for SYSRTC_START                */
#define _SYSRTC_CMD_START_MASK    0x1UL                            /**< Bit mask for SYSRTC_START                   */
#define _SYSRTC_CMD_START_DEFAULT 0x00000000UL                     /**< Mode DEFAULT for SYSRTC_CMD                 */
#define SYSRTC_CMD_START_DEFAULT  (_SYSRTC_CMD_START_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_CMD         */
#define SYSRTC_CMD_STOP           (0x1UL << 1)                     /**< Stop SYSRTC                                 */
#define _SYSRTC_CMD_STOP_SHIFT    1                                /**< Shift value for SYSRTC_STOP                 */
#define _SYSRTC_CMD_STOP_MASK     0x2UL                            /**< Bit mask for SYSRTC_STOP                    */
#define _SYSRTC_CMD_STOP_DEFAULT  0x00000000UL                     /**< Mode DEFAULT for SYSRTC_CMD                 */
#define SYSRTC_CMD_STOP_DEFAULT   (_SYSRTC_CMD_STOP_DEFAULT << 1)  /**< Shifted mode DEFAULT for SYSRTC_CMD         */

/* Bit fields for SYSRTC STATUS */
#define _SYSRTC_STATUS_RESETVALUE      0x00000000UL /**< Default value for SYSRTC_STATUS             */
#define _SYSRTC_STATUS_MASK            0x00000007UL /**< Mask for SYSRTC_STATUS                      */
#define SYSRTC_STATUS_RUNNING          (0x1UL << 0) /**< SYSRTC running status                       */
#define _SYSRTC_STATUS_RUNNING_SHIFT   0            /**< Shift value for SYSRTC_RUNNING              */
#define _SYSRTC_STATUS_RUNNING_MASK    0x1UL        /**< Bit mask for SYSRTC_RUNNING                 */
#define _SYSRTC_STATUS_RUNNING_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_STATUS              */
#define SYSRTC_STATUS_RUNNING_DEFAULT \
  (_SYSRTC_STATUS_RUNNING_DEFAULT << 0)                 /**< Shifted mode DEFAULT for SYSRTC_STATUS      */
#define SYSRTC_STATUS_LOCKSTATUS           (0x1UL << 1) /**< Lock Status                                 */
#define _SYSRTC_STATUS_LOCKSTATUS_SHIFT    1            /**< Shift value for SYSRTC_LOCKSTATUS           */
#define _SYSRTC_STATUS_LOCKSTATUS_MASK     0x2UL        /**< Bit mask for SYSRTC_LOCKSTATUS              */
#define _SYSRTC_STATUS_LOCKSTATUS_DEFAULT  0x00000000UL /**< Mode DEFAULT for SYSRTC_STATUS              */
#define _SYSRTC_STATUS_LOCKSTATUS_UNLOCKED 0x00000000UL /**< Mode UNLOCKED for SYSRTC_STATUS             */
#define _SYSRTC_STATUS_LOCKSTATUS_LOCKED   0x00000001UL /**< Mode LOCKED for SYSRTC_STATUS               */
#define SYSRTC_STATUS_LOCKSTATUS_DEFAULT \
  (_SYSRTC_STATUS_LOCKSTATUS_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_STATUS      */
#define SYSRTC_STATUS_LOCKSTATUS_UNLOCKED \
  (_SYSRTC_STATUS_LOCKSTATUS_UNLOCKED << 1) /**< Shifted mode UNLOCKED for SYSRTC_STATUS     */
#define SYSRTC_STATUS_LOCKSTATUS_LOCKED \
  (_SYSRTC_STATUS_LOCKSTATUS_LOCKED << 1)                      /**< Shifted mode LOCKED for SYSRTC_STATUS       */
#define SYSRTC_STATUS_FAILDETLOCKSTATUS           (0x1UL << 2) /**< FAILDET Lock Status                         */
#define _SYSRTC_STATUS_FAILDETLOCKSTATUS_SHIFT    2            /**< Shift value for SYSRTC_FAILDETLOCKSTATUS    */
#define _SYSRTC_STATUS_FAILDETLOCKSTATUS_MASK     0x4UL        /**< Bit mask for SYSRTC_FAILDETLOCKSTATUS       */
#define _SYSRTC_STATUS_FAILDETLOCKSTATUS_DEFAULT  0x00000000UL /**< Mode DEFAULT for SYSRTC_STATUS              */
#define _SYSRTC_STATUS_FAILDETLOCKSTATUS_UNLOCKED 0x00000000UL /**< Mode UNLOCKED for SYSRTC_STATUS             */
#define _SYSRTC_STATUS_FAILDETLOCKSTATUS_LOCKED   0x00000001UL /**< Mode LOCKED for SYSRTC_STATUS               */
#define SYSRTC_STATUS_FAILDETLOCKSTATUS_DEFAULT \
  (_SYSRTC_STATUS_FAILDETLOCKSTATUS_DEFAULT << 2) /**< Shifted mode DEFAULT for SYSRTC_STATUS      */
#define SYSRTC_STATUS_FAILDETLOCKSTATUS_UNLOCKED \
  (_SYSRTC_STATUS_FAILDETLOCKSTATUS_UNLOCKED << 2) /**< Shifted mode UNLOCKED for SYSRTC_STATUS     */
#define SYSRTC_STATUS_FAILDETLOCKSTATUS_LOCKED \
  (_SYSRTC_STATUS_FAILDETLOCKSTATUS_LOCKED << 2) /**< Shifted mode LOCKED for SYSRTC_STATUS       */

/* Bit fields for SYSRTC CNT */
#define _SYSRTC_CNT_RESETVALUE  0x00000000UL                   /**< Default value for SYSRTC_CNT                */
#define _SYSRTC_CNT_MASK        0xFFFFFFFFUL                   /**< Mask for SYSRTC_CNT                         */
#define _SYSRTC_CNT_CNT_SHIFT   0                              /**< Shift value for SYSRTC_CNT                  */
#define _SYSRTC_CNT_CNT_MASK    0xFFFFFFFFUL                   /**< Bit mask for SYSRTC_CNT                     */
#define _SYSRTC_CNT_CNT_DEFAULT 0x00000000UL                   /**< Mode DEFAULT for SYSRTC_CNT                 */
#define SYSRTC_CNT_CNT_DEFAULT  (_SYSRTC_CNT_CNT_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_CNT         */

/* Bit fields for SYSRTC SYNCBUSY */
#define _SYSRTC_SYNCBUSY_RESETVALUE    0x00000000UL /**< Default value for SYSRTC_SYNCBUSY           */
#define _SYSRTC_SYNCBUSY_MASK          0x0000000FUL /**< Mask for SYSRTC_SYNCBUSY                    */
#define SYSRTC_SYNCBUSY_START          (0x1UL << 0) /**< Sync busy for START bitfield                */
#define _SYSRTC_SYNCBUSY_START_SHIFT   0            /**< Shift value for SYSRTC_START                */
#define _SYSRTC_SYNCBUSY_START_MASK    0x1UL        /**< Bit mask for SYSRTC_START                   */
#define _SYSRTC_SYNCBUSY_START_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_START_DEFAULT \
  (_SYSRTC_SYNCBUSY_START_DEFAULT << 0)            /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */
#define SYSRTC_SYNCBUSY_STOP          (0x1UL << 1) /**< Sync busy for STOP bitfield                 */
#define _SYSRTC_SYNCBUSY_STOP_SHIFT   1            /**< Shift value for SYSRTC_STOP                 */
#define _SYSRTC_SYNCBUSY_STOP_MASK    0x2UL        /**< Bit mask for SYSRTC_STOP                    */
#define _SYSRTC_SYNCBUSY_STOP_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_STOP_DEFAULT \
  (_SYSRTC_SYNCBUSY_STOP_DEFAULT << 1)            /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */
#define SYSRTC_SYNCBUSY_CNT          (0x1UL << 2) /**< Sync busy for CNT bitfield                  */
#define _SYSRTC_SYNCBUSY_CNT_SHIFT   2            /**< Shift value for SYSRTC_CNT                  */
#define _SYSRTC_SYNCBUSY_CNT_MASK    0x4UL        /**< Bit mask for SYSRTC_CNT                     */
#define _SYSRTC_SYNCBUSY_CNT_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_CNT_DEFAULT \
  (_SYSRTC_SYNCBUSY_CNT_DEFAULT << 2)                     /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */
#define SYSRTC_SYNCBUSY_FAILDETCTRL          (0x1UL << 3) /**< Sync busy for FAILDETCTRL register          */
#define _SYSRTC_SYNCBUSY_FAILDETCTRL_SHIFT   3            /**< Shift value for SYSRTC_FAILDETCTRL          */
#define _SYSRTC_SYNCBUSY_FAILDETCTRL_MASK    0x8UL        /**< Bit mask for SYSRTC_FAILDETCTRL             */
#define _SYSRTC_SYNCBUSY_FAILDETCTRL_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_FAILDETCTRL_DEFAULT \
  (_SYSRTC_SYNCBUSY_FAILDETCTRL_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */

/* Bit fields for SYSRTC LOCK */
#define _SYSRTC_LOCK_RESETVALUE      0x00000000UL /**< Default value for SYSRTC_LOCK               */
#define _SYSRTC_LOCK_MASK            0x0000FFFFUL /**< Mask for SYSRTC_LOCK                        */
#define _SYSRTC_LOCK_LOCKKEY_SHIFT   0            /**< Shift value for SYSRTC_LOCKKEY              */
#define _SYSRTC_LOCK_LOCKKEY_MASK    0xFFFFUL     /**< Bit mask for SYSRTC_LOCKKEY                 */
#define _SYSRTC_LOCK_LOCKKEY_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_LOCK                */
#define _SYSRTC_LOCK_LOCKKEY_UNLOCK  0x00004776UL /**< Mode UNLOCK for SYSRTC_LOCK                 */
#define SYSRTC_LOCK_LOCKKEY_DEFAULT \
  (_SYSRTC_LOCK_LOCKKEY_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_LOCK        */
#define SYSRTC_LOCK_LOCKKEY_UNLOCK \
  (_SYSRTC_LOCK_LOCKKEY_UNLOCK << 0) /**< Shifted mode UNLOCK for SYSRTC_LOCK         */

/* Bit fields for SYSRTC FAILDETCTRL */
#define _SYSRTC_FAILDETCTRL_RESETVALUE        0x00001030UL /**< Default value for SYSRTC_FAILDETCTRL        */
#define _SYSRTC_FAILDETCTRL_MASK              0x00011F3FUL /**< Mask for SYSRTC_FAILDETCTRL                 */
#define _SYSRTC_FAILDETCTRL_FAILCNTHI_SHIFT   0            /**< Shift value for SYSRTC_FAILCNTHI            */
#define _SYSRTC_FAILDETCTRL_FAILCNTHI_MASK    0x3FUL       /**< Bit mask for SYSRTC_FAILCNTHI               */
#define _SYSRTC_FAILDETCTRL_FAILCNTHI_DEFAULT 0x00000030UL /**< Mode DEFAULT for SYSRTC_FAILDETCTRL         */
#define SYSRTC_FAILDETCTRL_FAILCNTHI_DEFAULT \
  (_SYSRTC_FAILDETCTRL_FAILCNTHI_DEFAULT << 0)             /**< Shifted mode DEFAULT for SYSRTC_FAILDETCTRL */
#define _SYSRTC_FAILDETCTRL_FAILCNTLO_SHIFT   8            /**< Shift value for SYSRTC_FAILCNTLO            */
#define _SYSRTC_FAILDETCTRL_FAILCNTLO_MASK    0x1F00UL     /**< Bit mask for SYSRTC_FAILCNTLO               */
#define _SYSRTC_FAILDETCTRL_FAILCNTLO_DEFAULT 0x00000010UL /**< Mode DEFAULT for SYSRTC_FAILDETCTRL         */
#define SYSRTC_FAILDETCTRL_FAILCNTLO_DEFAULT \
  (_SYSRTC_FAILDETCTRL_FAILCNTLO_DEFAULT << 8)              /**< Shifted mode DEFAULT for SYSRTC_FAILDETCTRL */
#define SYSRTC_FAILDETCTRL_FAILDETEN          (0x1UL << 16) /**< Failure Detection Enable                    */
#define _SYSRTC_FAILDETCTRL_FAILDETEN_SHIFT   16            /**< Shift value for SYSRTC_FAILDETEN            */
#define _SYSRTC_FAILDETCTRL_FAILDETEN_MASK    0x10000UL     /**< Bit mask for SYSRTC_FAILDETEN               */
#define _SYSRTC_FAILDETCTRL_FAILDETEN_DEFAULT 0x00000000UL  /**< Mode DEFAULT for SYSRTC_FAILDETCTRL         */
#define SYSRTC_FAILDETCTRL_FAILDETEN_DEFAULT \
  (_SYSRTC_FAILDETCTRL_FAILDETEN_DEFAULT << 16) /**< Shifted mode DEFAULT for SYSRTC_FAILDETCTRL */

/* Bit fields for SYSRTC FAILDETLOCK */
#define _SYSRTC_FAILDETLOCK_RESETVALUE      0x00000000UL /**< Default value for SYSRTC_FAILDETLOCK        */
#define _SYSRTC_FAILDETLOCK_MASK            0x0000FFFFUL /**< Mask for SYSRTC_FAILDETLOCK                 */
#define _SYSRTC_FAILDETLOCK_LOCKKEY_SHIFT   0            /**< Shift value for SYSRTC_LOCKKEY              */
#define _SYSRTC_FAILDETLOCK_LOCKKEY_MASK    0xFFFFUL     /**< Bit mask for SYSRTC_LOCKKEY                 */
#define _SYSRTC_FAILDETLOCK_LOCKKEY_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_FAILDETLOCK         */
#define _SYSRTC_FAILDETLOCK_LOCKKEY_UNLOCK  0x000037A2UL /**< Mode UNLOCK for SYSRTC_FAILDETLOCK          */
#define SYSRTC_FAILDETLOCK_LOCKKEY_DEFAULT \
  (_SYSRTC_FAILDETLOCK_LOCKKEY_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_FAILDETLOCK */
#define SYSRTC_FAILDETLOCK_LOCKKEY_UNLOCK \
  (_SYSRTC_FAILDETLOCK_LOCKKEY_UNLOCK << 0) /**< Shifted mode UNLOCK for SYSRTC_FAILDETLOCK  */

/* Bit fields for SYSRTC GRP0_IF */
#define _SYSRTC_GRP0_IF_RESETVALUE  0x00000000UL /**< Default value for SYSRTC_GRP0_IF            */
#define _SYSRTC_GRP0_IF_MASK        0x0000000FUL /**< Mask for SYSRTC_GRP0_IF                     */
#define SYSRTC_GRP0_IF_OVF          (0x1UL << 0) /**< Overflow Interrupt Flag                     */
#define _SYSRTC_GRP0_IF_OVF_SHIFT   0            /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP0_IF_OVF_MASK    0x1UL        /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP0_IF_OVF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_OVF_DEFAULT \
  (_SYSRTC_GRP0_IF_OVF_DEFAULT << 0)              /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */
#define SYSRTC_GRP0_IF_CMP0          (0x1UL << 1) /**< Compare 0 Interrupt Flag                    */
#define _SYSRTC_GRP0_IF_CMP0_SHIFT   1            /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP0_IF_CMP0_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP0_IF_CMP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_CMP0_DEFAULT \
  (_SYSRTC_GRP0_IF_CMP0_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */
#define SYSRTC_GRP0_IF_CMP1          (0x1UL << 2) /**< Compare 1 Interrupt Flag                    */
#define _SYSRTC_GRP0_IF_CMP1_SHIFT   2            /**< Shift value for SYSRTC_CMP1                 */
#define _SYSRTC_GRP0_IF_CMP1_MASK    0x4UL        /**< Bit mask for SYSRTC_CMP1                    */
#define _SYSRTC_GRP0_IF_CMP1_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_CMP1_DEFAULT \
  (_SYSRTC_GRP0_IF_CMP1_DEFAULT << 2)             /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */
#define SYSRTC_GRP0_IF_CAP0          (0x1UL << 3) /**< Capture 0 Interrupt Flag                    */
#define _SYSRTC_GRP0_IF_CAP0_SHIFT   3            /**< Shift value for SYSRTC_CAP0                 */
#define _SYSRTC_GRP0_IF_CAP0_MASK    0x8UL        /**< Bit mask for SYSRTC_CAP0                    */
#define _SYSRTC_GRP0_IF_CAP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_CAP0_DEFAULT \
  (_SYSRTC_GRP0_IF_CAP0_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */

/* Bit fields for SYSRTC GRP0_IEN */
#define _SYSRTC_GRP0_IEN_RESETVALUE  0x00000000UL /**< Default value for SYSRTC_GRP0_IEN           */
#define _SYSRTC_GRP0_IEN_MASK        0x0000000FUL /**< Mask for SYSRTC_GRP0_IEN                    */
#define SYSRTC_GRP0_IEN_OVF          (0x1UL << 0) /**< Overflow Interrupt Enable                   */
#define _SYSRTC_GRP0_IEN_OVF_SHIFT   0            /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP0_IEN_OVF_MASK    0x1UL        /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP0_IEN_OVF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_OVF_DEFAULT \
  (_SYSRTC_GRP0_IEN_OVF_DEFAULT << 0)              /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */
#define SYSRTC_GRP0_IEN_CMP0          (0x1UL << 1) /**< Compare 0 Interrupt Enable                  */
#define _SYSRTC_GRP0_IEN_CMP0_SHIFT   1            /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP0_IEN_CMP0_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP0_IEN_CMP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_CMP0_DEFAULT \
  (_SYSRTC_GRP0_IEN_CMP0_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */
#define SYSRTC_GRP0_IEN_CMP1          (0x1UL << 2) /**< Compare 1 Interrupt Enable                  */
#define _SYSRTC_GRP0_IEN_CMP1_SHIFT   2            /**< Shift value for SYSRTC_CMP1                 */
#define _SYSRTC_GRP0_IEN_CMP1_MASK    0x4UL        /**< Bit mask for SYSRTC_CMP1                    */
#define _SYSRTC_GRP0_IEN_CMP1_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_CMP1_DEFAULT \
  (_SYSRTC_GRP0_IEN_CMP1_DEFAULT << 2)             /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */
#define SYSRTC_GRP0_IEN_CAP0          (0x1UL << 3) /**< Capture 0 Interrupt Enable                  */
#define _SYSRTC_GRP0_IEN_CAP0_SHIFT   3            /**< Shift value for SYSRTC_CAP0                 */
#define _SYSRTC_GRP0_IEN_CAP0_MASK    0x8UL        /**< Bit mask for SYSRTC_CAP0                    */
#define _SYSRTC_GRP0_IEN_CAP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_CAP0_DEFAULT \
  (_SYSRTC_GRP0_IEN_CAP0_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */

/* Bit fields for SYSRTC GRP0_CTRL */
#define _SYSRTC_GRP0_CTRL_RESETVALUE     0x00000000UL /**< Default value for SYSRTC_GRP0_CTRL          */
#define _SYSRTC_GRP0_CTRL_MASK           0x000007FFUL /**< Mask for SYSRTC_GRP0_CTRL                   */
#define SYSRTC_GRP0_CTRL_CMP0EN          (0x1UL << 0) /**< Compare 0 Enable                            */
#define _SYSRTC_GRP0_CTRL_CMP0EN_SHIFT   0            /**< Shift value for SYSRTC_CMP0EN               */
#define _SYSRTC_GRP0_CTRL_CMP0EN_MASK    0x1UL        /**< Bit mask for SYSRTC_CMP0EN                  */
#define _SYSRTC_GRP0_CTRL_CMP0EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define SYSRTC_GRP0_CTRL_CMP0EN_DEFAULT \
  (_SYSRTC_GRP0_CTRL_CMP0EN_DEFAULT << 0)             /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CMP1EN          (0x1UL << 1) /**< Compare 1 Enable                            */
#define _SYSRTC_GRP0_CTRL_CMP1EN_SHIFT   1            /**< Shift value for SYSRTC_CMP1EN               */
#define _SYSRTC_GRP0_CTRL_CMP1EN_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP1EN                  */
#define _SYSRTC_GRP0_CTRL_CMP1EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define SYSRTC_GRP0_CTRL_CMP1EN_DEFAULT \
  (_SYSRTC_GRP0_CTRL_CMP1EN_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CAP0EN          (0x1UL << 2) /**< Capture 0 Enable                            */
#define _SYSRTC_GRP0_CTRL_CAP0EN_SHIFT   2            /**< Shift value for SYSRTC_CAP0EN               */
#define _SYSRTC_GRP0_CTRL_CAP0EN_MASK    0x4UL        /**< Bit mask for SYSRTC_CAP0EN                  */
#define _SYSRTC_GRP0_CTRL_CAP0EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define SYSRTC_GRP0_CTRL_CAP0EN_DEFAULT \
  (_SYSRTC_GRP0_CTRL_CAP0EN_DEFAULT << 2)               /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_SHIFT   3            /**< Shift value for SYSRTC_CMP0CMOA             */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_MASK    0x38UL       /**< Bit mask for SYSRTC_CMP0CMOA                */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_CLEAR   0x00000000UL /**< Mode CLEAR for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_SET     0x00000001UL /**< Mode SET for SYSRTC_GRP0_CTRL               */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_PULSE   0x00000002UL /**< Mode PULSE for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_TOGGLE  0x00000003UL /**< Mode TOGGLE for SYSRTC_GRP0_CTRL            */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_CMPIF   0x00000004UL /**< Mode CMPIF for SYSRTC_GRP0_CTRL             */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_DEFAULT \
  (_SYSRTC_GRP0_CTRL_CMP0CMOA_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_CLEAR \
  (_SYSRTC_GRP0_CTRL_CMP0CMOA_CLEAR << 3) /**< Shifted mode CLEAR for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_SET \
  (_SYSRTC_GRP0_CTRL_CMP0CMOA_SET << 3) /**< Shifted mode SET for SYSRTC_GRP0_CTRL       */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_PULSE \
  (_SYSRTC_GRP0_CTRL_CMP0CMOA_PULSE << 3) /**< Shifted mode PULSE for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_TOGGLE \
  (_SYSRTC_GRP0_CTRL_CMP0CMOA_TOGGLE << 3) /**< Shifted mode TOGGLE for SYSRTC_GRP0_CTRL    */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_CMPIF \
  (_SYSRTC_GRP0_CTRL_CMP0CMOA_CMPIF << 3)               /**< Shifted mode CMPIF for SYSRTC_GRP0_CTRL     */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_SHIFT   6            /**< Shift value for SYSRTC_CMP1CMOA             */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_MASK    0x1C0UL      /**< Bit mask for SYSRTC_CMP1CMOA                */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_CLEAR   0x00000000UL /**< Mode CLEAR for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_SET     0x00000001UL /**< Mode SET for SYSRTC_GRP0_CTRL               */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_PULSE   0x00000002UL /**< Mode PULSE for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_TOGGLE  0x00000003UL /**< Mode TOGGLE for SYSRTC_GRP0_CTRL            */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_CMPIF   0x00000004UL /**< Mode CMPIF for SYSRTC_GRP0_CTRL             */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_DEFAULT \
  (_SYSRTC_GRP0_CTRL_CMP1CMOA_DEFAULT << 6) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_CLEAR \
  (_SYSRTC_GRP0_CTRL_CMP1CMOA_CLEAR << 6) /**< Shifted mode CLEAR for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_SET \
  (_SYSRTC_GRP0_CTRL_CMP1CMOA_SET << 6) /**< Shifted mode SET for SYSRTC_GRP0_CTRL       */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_PULSE \
  (_SYSRTC_GRP0_CTRL_CMP1CMOA_PULSE << 6) /**< Shifted mode PULSE for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_TOGGLE \
  (_SYSRTC_GRP0_CTRL_CMP1CMOA_TOGGLE << 6) /**< Shifted mode TOGGLE for SYSRTC_GRP0_CTRL    */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_CMPIF \
  (_SYSRTC_GRP0_CTRL_CMP1CMOA_CMPIF << 6)               /**< Shifted mode CMPIF for SYSRTC_GRP0_CTRL     */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_SHIFT   9            /**< Shift value for SYSRTC_CAP0EDGE             */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_MASK    0x600UL      /**< Bit mask for SYSRTC_CAP0EDGE                */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_RISING  0x00000000UL /**< Mode RISING for SYSRTC_GRP0_CTRL            */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_FALLING 0x00000001UL /**< Mode FALLING for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_BOTH    0x00000002UL /**< Mode BOTH for SYSRTC_GRP0_CTRL              */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_DEFAULT \
  (_SYSRTC_GRP0_CTRL_CAP0EDGE_DEFAULT << 9) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_RISING \
  (_SYSRTC_GRP0_CTRL_CAP0EDGE_RISING << 9) /**< Shifted mode RISING for SYSRTC_GRP0_CTRL    */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_FALLING \
  (_SYSRTC_GRP0_CTRL_CAP0EDGE_FALLING << 9) /**< Shifted mode FALLING for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_BOTH \
  (_SYSRTC_GRP0_CTRL_CAP0EDGE_BOTH << 9) /**< Shifted mode BOTH for SYSRTC_GRP0_CTRL      */

/* Bit fields for SYSRTC GRP0_CMP0VALUE */
#define _SYSRTC_GRP0_CMP0VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP0_CMP0VALUE     */
#define _SYSRTC_GRP0_CMP0VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP0_CMP0VALUE              */
#define _SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_SHIFT   0            /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CMP0VALUE      */
#define SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_DEFAULT \
  (_SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CMP0VALUE*/

/* Bit fields for SYSRTC GRP0_CMP1VALUE */
#define _SYSRTC_GRP0_CMP1VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP0_CMP1VALUE     */
#define _SYSRTC_GRP0_CMP1VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP0_CMP1VALUE              */
#define _SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_SHIFT   0            /**< Shift value for SYSRTC_CMP1VALUE            */
#define _SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CMP1VALUE               */
#define _SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CMP1VALUE      */
#define SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_DEFAULT \
  (_SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CMP1VALUE*/

/* Bit fields for SYSRTC GRP0_CAP0VALUE */
#define _SYSRTC_GRP0_CAP0VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP0_CAP0VALUE     */
#define _SYSRTC_GRP0_CAP0VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP0_CAP0VALUE              */
#define _SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_SHIFT   0            /**< Shift value for SYSRTC_CAP0VALUE            */
#define _SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CAP0VALUE               */
#define _SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP0_CAP0VALUE      */
#define SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_DEFAULT \
  (_SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CAP0VALUE*/

/* Bit fields for SYSRTC GRPx_SYNCBUSY */
#define _SYSRTC_GRPx_SYNCBUSY_RESETVALUE   0x00000000UL /**< Default value for SYSRTC_GRP0_SYNCBUSY      */
#define _SYSRTC_GRPx_SYNCBUSY_MASK         0x00000007UL /**< Mask for SYSRTC_GRPx_SYNCBUSY               */
#define SYSRTC_GRPx_SYNCBUSY_CTRL          (0x1UL << 0) /**< Sync busy for CTRL register                 */
#define _SYSRTC_GRPx_SYNCBUSY_CTRL_SHIFT   0            /**< Shift value for SYSRTC_CTRL                 */
#define _SYSRTC_GRPx_SYNCBUSY_CTRL_MASK    0x1UL        /**< Bit mask for SYSRTC_CTRL                    */
#define _SYSRTC_GRPx_SYNCBUSY_CTRL_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRPx_SYNCBUSY       */
#define SYSRTC_GRPx_SYNCBUSY_CTRL_DEFAULT \
  (_SYSRTC_GRPx_SYNCBUSY_CTRL_DEFAULT << 0)                  /**< Shifted mode DEFAULT for SYSRTC_GRPx_SYNCBUSY*/
#define SYSRTC_GRPx_SYNCBUSY_CMP0VALUE          (0x1UL << 1) /**< Sync busy for CMP0VALUE register            */
#define _SYSRTC_GRPx_SYNCBUSY_CMP0VALUE_SHIFT   1            /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRPx_SYNCBUSY_CMP0VALUE_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRPx_SYNCBUSY_CMP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRPx_SYNCBUSY       */
#define SYSRTC_GRPx_SYNCBUSY_CMP0VALUE_DEFAULT \
  (_SYSRTC_GRPx_SYNCBUSY_CMP0VALUE_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRPx_SYNCBUSY*/
#define SYSRTC_GRPx_SYNCBUSY_CMP1VALUE          (0x1UL << 2) /**< Sync busy for CMP1VALUE register            */
#define _SYSRTC_GRPx_SYNCBUSY_CMP1VALUE_SHIFT   2            /**< Shift value for SYSRTC_CMP1VALUE            */
#define _SYSRTC_GRPx_SYNCBUSY_CMP1VALUE_MASK    0x4UL        /**< Bit mask for SYSRTC_CMP1VALUE               */
#define _SYSRTC_GRPx_SYNCBUSY_CMP1VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRPx_SYNCBUSY       */
#define SYSRTC_GRPx_SYNCBUSY_CMP1VALUE_DEFAULT \
  (_SYSRTC_GRPx_SYNCBUSY_CMP1VALUE_DEFAULT << 2) /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/

/* Bit fields for SYSRTC GRP1_IF */
#define _SYSRTC_GRP1_IF_RESETVALUE    0x00000000UL /**< Default value for SYSRTC_GRP1_IF            */
#define _SYSRTC_GRP1_IF_MASK          0x0000000FUL /**< Mask for SYSRTC_GRP1_IF                     */
#define SYSRTC_GRP1_IF_OVFIF          (0x1UL << 0) /**< Overflow Interrupt Flag                     */
#define _SYSRTC_GRP1_IF_OVFIF_SHIFT   0            /**< Shift value for SYSRTC_OVFIF                */
#define _SYSRTC_GRP1_IF_OVFIF_MASK    0x1UL        /**< Bit mask for SYSRTC_OVFIF                   */
#define _SYSRTC_GRP1_IF_OVFIF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_OVFIF_DEFAULT \
  (_SYSRTC_GRP1_IF_OVFIF_DEFAULT << 0)              /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_CMP0IF          (0x1UL << 1) /**< Compare 0 Interrupt Flag                    */
#define _SYSRTC_GRP1_IF_CMP0IF_SHIFT   1            /**< Shift value for SYSRTC_CMP0IF               */
#define _SYSRTC_GRP1_IF_CMP0IF_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0IF                  */
#define _SYSRTC_GRP1_IF_CMP0IF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_CMP0IF_DEFAULT \
  (_SYSRTC_GRP1_IF_CMP0IF_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_CMP1IF          (0x1UL << 2) /**< Compare 1 Interrupt Flag                    */
#define _SYSRTC_GRP1_IF_CMP1IF_SHIFT   2            /**< Shift value for SYSRTC_CMP1IF               */
#define _SYSRTC_GRP1_IF_CMP1IF_MASK    0x4UL        /**< Bit mask for SYSRTC_CMP1IF                  */
#define _SYSRTC_GRP1_IF_CMP1IF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_CMP1IF_DEFAULT \
  (_SYSRTC_GRP1_IF_CMP1IF_DEFAULT << 2)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_CAP0IF          (0x1UL << 3) /**< Capture 0 Interrupt Flag                    */
#define _SYSRTC_GRP1_IF_CAP0IF_SHIFT   3            /**< Shift value for SYSRTC_CAP0IF               */
#define _SYSRTC_GRP1_IF_CAP0IF_MASK    0x8UL        /**< Bit mask for SYSRTC_CAP0IF                  */
#define _SYSRTC_GRP1_IF_CAP0IF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_CAP0IF_DEFAULT \
  (_SYSRTC_GRP1_IF_CAP0IF_DEFAULT << 3)               /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_ALTOVFIF          (0x1UL << 4) /**< Alternate Overflow Interrupt Flag           */
#define _SYSRTC_GRP1_IF_ALTOVFIF_SHIFT   4            /**< Shift value for SYSRTC_ALTOVFIF             */
#define _SYSRTC_GRP1_IF_ALTOVFIF_MASK    0x10UL       /**< Bit mask for SYSRTC_ALTOVFIF                */
#define _SYSRTC_GRP1_IF_ALTOVFIF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_ALTOVFIF_DEFAULT \
  (_SYSRTC_GRP1_IF_ALTOVFIF_DEFAULT << 4)              /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_ALTCMP0IF          (0x1UL << 5) /**< Alternate Compare 0 Interrupt Flag          */
#define _SYSRTC_GRP1_IF_ALTCMP0IF_SHIFT   5            /**< Shift value for SYSRTC_ALTCMP0IF            */
#define _SYSRTC_GRP1_IF_ALTCMP0IF_MASK    0x20UL       /**< Bit mask for SYSRTC_ALTCMP0IF               */
#define _SYSRTC_GRP1_IF_ALTCMP0IF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_ALTCMP0IF_DEFAULT \
  (_SYSRTC_GRP1_IF_ALTCMP0IF_DEFAULT << 5)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_ALTCMP1IF          (0x1UL << 6) /**< Alternate Compare 1 Interrupt Flag          */
#define _SYSRTC_GRP1_IF_ALTCMP1IF_SHIFT   6            /**< Shift value for SYSRTC_ALTCMP1IF            */
#define _SYSRTC_GRP1_IF_ALTCMP1IF_MASK    0x40UL       /**< Bit mask for SYSRTC_ALTCMP1IF               */
#define _SYSRTC_GRP1_IF_ALTCMP1IF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_ALTCMP1IF_DEFAULT \
  (_SYSRTC_GRP1_IF_ALTCMP1IF_DEFAULT << 6)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */
#define SYSRTC_GRP1_IF_ALTCAP0IF          (0x1UL << 7) /**< Alternate Capture 0 Interrupt Flag          */
#define _SYSRTC_GRP1_IF_ALTCAP0IF_SHIFT   7            /**< Shift value for SYSRTC_ALTCAP0IF            */
#define _SYSRTC_GRP1_IF_ALTCAP0IF_MASK    0x80UL       /**< Bit mask for SYSRTC_ALTCAP0IF               */
#define _SYSRTC_GRP1_IF_ALTCAP0IF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IF             */
#define SYSRTC_GRP1_IF_ALTCAP0IF_DEFAULT \
  (_SYSRTC_GRP1_IF_ALTCAP0IF_DEFAULT << 7) /**< Shifted mode DEFAULT for SYSRTC_GRP1_IF     */

/* Bit fields for SYSRTC GRP1_IEN */
#define _SYSRTC_GRP1_IEN_RESETVALUE  0x00000000UL /**< Default value for SYSRTC_GRP1_IEN           */
#define _SYSRTC_GRP1_IEN_MASK        0x0000000FUL /**< Mask for SYSRTC_GRP1_IEN                    */
#define SYSRTC_GRP1_IEN_OVF          (0x1UL << 0) /**< Overflow Interrupt Enable                   */
#define _SYSRTC_GRP1_IEN_OVF_SHIFT   0            /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP1_IEN_OVF_MASK    0x1UL        /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP1_IEN_OVF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_OVF_DEFAULT \
  (_SYSRTC_GRP1_IEN_OVF_DEFAULT << 0)              /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_CMP0          (0x1UL << 1) /**< Compare 0 Interrupt Enable                  */
#define _SYSRTC_GRP1_IEN_CMP0_SHIFT   1            /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP1_IEN_CMP0_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP1_IEN_CMP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_CMP0_DEFAULT \
  (_SYSRTC_GRP1_IEN_CMP0_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_CMP1          (0x1UL << 2) /**< Compare 1 Interrupt Enable                  */
#define _SYSRTC_GRP1_IEN_CMP1_SHIFT   2            /**< Shift value for SYSRTC_CMP1                 */
#define _SYSRTC_GRP1_IEN_CMP1_MASK    0x4UL        /**< Bit mask for SYSRTC_CMP1                    */
#define _SYSRTC_GRP1_IEN_CMP1_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_CMP1_DEFAULT \
  (_SYSRTC_GRP1_IEN_CMP1_DEFAULT << 2)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_CAP0          (0x1UL << 3) /**< Capture 0 Interrupt Enable                  */
#define _SYSRTC_GRP1_IEN_CAP0_SHIFT   3            /**< Shift value for SYSRTC_CAP0                 */
#define _SYSRTC_GRP1_IEN_CAP0_MASK    0x8UL        /**< Bit mask for SYSRTC_CAP0                    */
#define _SYSRTC_GRP1_IEN_CAP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_CAP0_DEFAULT \
  (_SYSRTC_GRP1_IEN_CAP0_DEFAULT << 3)               /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_ALTOVF          (0x1UL << 4) /**< Alternate Overflow Interrupt Enable         */
#define _SYSRTC_GRP1_IEN_ALTOVF_SHIFT   4            /**< Shift value for SYSRTC_ALTOVF               */
#define _SYSRTC_GRP1_IEN_ALTOVF_MASK    0x10UL       /**< Bit mask for SYSRTC_ALTOVF                  */
#define _SYSRTC_GRP1_IEN_ALTOVF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_ALTOVF_DEFAULT \
  (_SYSRTC_GRP1_IEN_ALTOVF_DEFAULT << 4)              /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_ALTCMP0          (0x1UL << 5) /**< Alternate Compare 0 Interrupt Enable        */
#define _SYSRTC_GRP1_IEN_ALTCMP0_SHIFT   5            /**< Shift value for SYSRTC_ALTCMP0              */
#define _SYSRTC_GRP1_IEN_ALTCMP0_MASK    0x20UL       /**< Bit mask for SYSRTC_ALTCMP0                 */
#define _SYSRTC_GRP1_IEN_ALTCMP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_ALTCMP0_DEFAULT \
  (_SYSRTC_GRP1_IEN_ALTCMP0_DEFAULT << 5)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_ALTCMP1          (0x1UL << 6) /**< Alternate Compare 1 Interrupt Enable        */
#define _SYSRTC_GRP1_IEN_ALTCMP1_SHIFT   6            /**< Shift value for SYSRTC_ALTCMP1              */
#define _SYSRTC_GRP1_IEN_ALTCMP1_MASK    0x40UL       /**< Bit mask for SYSRTC_ALTCMP1                 */
#define _SYSRTC_GRP1_IEN_ALTCMP1_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_ALTCMP1_DEFAULT \
  (_SYSRTC_GRP1_IEN_ALTCMP1_DEFAULT << 6)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */
#define SYSRTC_GRP1_IEN_ALTCAP0          (0x1UL << 7) /**< Alternate Capture 0 Interrupt Enable        */
#define _SYSRTC_GRP1_IEN_ALTCAP0_SHIFT   7            /**< Shift value for SYSRTC_ALTCAP0              */
#define _SYSRTC_GRP1_IEN_ALTCAP0_MASK    0x80UL       /**< Bit mask for SYSRTC_ALTCAP0                 */
#define _SYSRTC_GRP1_IEN_ALTCAP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_IEN            */
#define SYSRTC_GRP1_IEN_ALTCAP0_DEFAULT \
  (_SYSRTC_GRP1_IEN_ALTCAP0_DEFAULT << 7) /**< Shifted mode DEFAULT for SYSRTC_GRP1_IEN    */

/* Bit fields for SYSRTC GRP1_CTRL */
#define _SYSRTC_GRP1_CTRL_RESETVALUE     0x00000000UL /**< Default value for SYSRTC_GRP1_CTRL          */
#define _SYSRTC_GRP1_CTRL_MASK           0x000007FFUL /**< Mask for SYSRTC_GRP1_CTRL                   */
#define SYSRTC_GRP1_CTRL_CMP0EN          (0x1UL << 0) /**< Compare 0 Enable                            */
#define _SYSRTC_GRP1_CTRL_CMP0EN_SHIFT   0            /**< Shift value for SYSRTC_CMP0EN               */
#define _SYSRTC_GRP1_CTRL_CMP0EN_MASK    0x1UL        /**< Bit mask for SYSRTC_CMP0EN                  */
#define _SYSRTC_GRP1_CTRL_CMP0EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CTRL           */
#define SYSRTC_GRP1_CTRL_CMP0EN_DEFAULT \
  (_SYSRTC_GRP1_CTRL_CMP0EN_DEFAULT << 0)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_CTRL   */
#define SYSRTC_GRP1_CTRL_CMP1EN          (0x1UL << 1) /**< Compare 1 Enable                            */
#define _SYSRTC_GRP1_CTRL_CMP1EN_SHIFT   1            /**< Shift value for SYSRTC_CMP1EN               */
#define _SYSRTC_GRP1_CTRL_CMP1EN_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP1EN                  */
#define _SYSRTC_GRP1_CTRL_CMP1EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CTRL           */
#define SYSRTC_GRP1_CTRL_CMP1EN_DEFAULT \
  (_SYSRTC_GRP1_CTRL_CMP1EN_DEFAULT << 1)             /**< Shifted mode DEFAULT for SYSRTC_GRP1_CTRL   */
#define SYSRTC_GRP1_CTRL_CAP0EN          (0x1UL << 2) /**< Capture 0 Enable                            */
#define _SYSRTC_GRP1_CTRL_CAP0EN_SHIFT   2            /**< Shift value for SYSRTC_CAP0EN               */
#define _SYSRTC_GRP1_CTRL_CAP0EN_MASK    0x4UL        /**< Bit mask for SYSRTC_CAP0EN                  */
#define _SYSRTC_GRP1_CTRL_CAP0EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CTRL           */
#define SYSRTC_GRP1_CTRL_CAP0EN_DEFAULT \
  (_SYSRTC_GRP1_CTRL_CAP0EN_DEFAULT << 2)               /**< Shifted mode DEFAULT for SYSRTC_GRP1_CTRL   */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_SHIFT   3            /**< Shift value for SYSRTC_CMP0CMOA             */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_MASK    0x38UL       /**< Bit mask for SYSRTC_CMP0CMOA                */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CTRL           */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_CLEAR   0x00000000UL /**< Mode CLEAR for SYSRTC_GRP1_CTRL             */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_SET     0x00000001UL /**< Mode SET for SYSRTC_GRP1_CTRL               */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_PULSE   0x00000002UL /**< Mode PULSE for SYSRTC_GRP1_CTRL             */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_TOGGLE  0x00000003UL /**< Mode TOGGLE for SYSRTC_GRP1_CTRL            */
#define _SYSRTC_GRP1_CTRL_CMP0CMOA_CMPIF   0x00000004UL /**< Mode CMPIF for SYSRTC_GRP1_CTRL             */
#define SYSRTC_GRP1_CTRL_CMP0CMOA_DEFAULT \
  (_SYSRTC_GRP1_CTRL_CMP0CMOA_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP1_CTRL   */
#define SYSRTC_GRP1_CTRL_CMP0CMOA_CLEAR \
  (_SYSRTC_GRP1_CTRL_CMP0CMOA_CLEAR << 3) /**< Shifted mode CLEAR for SYSRTC_GRP1_CTRL     */
#define SYSRTC_GRP1_CTRL_CMP0CMOA_SET \
  (_SYSRTC_GRP1_CTRL_CMP0CMOA_SET << 3) /**< Shifted mode SET for SYSRTC_GRP1_CTRL       */
#define SYSRTC_GRP1_CTRL_CMP0CMOA_PULSE \
  (_SYSRTC_GRP1_CTRL_CMP0CMOA_PULSE << 3) /**< Shifted mode PULSE for SYSRTC_GRP1_CTRL     */
#define SYSRTC_GRP1_CTRL_CMP0CMOA_TOGGLE \
  (_SYSRTC_GRP1_CTRL_CMP0CMOA_TOGGLE << 3) /**< Shifted mode TOGGLE for SYSRTC_GRP1_CTRL    */
#define SYSRTC_GRP1_CTRL_CMP0CMOA_CMPIF \
  (_SYSRTC_GRP1_CTRL_CMP0CMOA_CMPIF << 3)               /**< Shifted mode CMPIF for SYSRTC_GRP1_CTRL     */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_SHIFT   6            /**< Shift value for SYSRTC_CMP1CMOA             */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_MASK    0x1C0UL      /**< Bit mask for SYSRTC_CMP1CMOA                */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CTRL           */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_CLEAR   0x00000000UL /**< Mode CLEAR for SYSRTC_GRP1_CTRL             */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_SET     0x00000001UL /**< Mode SET for SYSRTC_GRP1_CTRL               */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_PULSE   0x00000002UL /**< Mode PULSE for SYSRTC_GRP1_CTRL             */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_TOGGLE  0x00000003UL /**< Mode TOGGLE for SYSRTC_GRP1_CTRL            */
#define _SYSRTC_GRP1_CTRL_CMP1CMOA_CMPIF   0x00000004UL /**< Mode CMPIF for SYSRTC_GRP1_CTRL             */
#define SYSRTC_GRP1_CTRL_CMP1CMOA_DEFAULT \
  (_SYSRTC_GRP1_CTRL_CMP1CMOA_DEFAULT << 6) /**< Shifted mode DEFAULT for SYSRTC_GRP1_CTRL   */
#define SYSRTC_GRP1_CTRL_CMP1CMOA_CLEAR \
  (_SYSRTC_GRP1_CTRL_CMP1CMOA_CLEAR << 6) /**< Shifted mode CLEAR for SYSRTC_GRP1_CTRL     */
#define SYSRTC_GRP1_CTRL_CMP1CMOA_SET \
  (_SYSRTC_GRP1_CTRL_CMP1CMOA_SET << 6) /**< Shifted mode SET for SYSRTC_GRP1_CTRL       */
#define SYSRTC_GRP1_CTRL_CMP1CMOA_PULSE \
  (_SYSRTC_GRP1_CTRL_CMP1CMOA_PULSE << 6) /**< Shifted mode PULSE for SYSRTC_GRP1_CTRL     */
#define SYSRTC_GRP1_CTRL_CMP1CMOA_TOGGLE \
  (_SYSRTC_GRP1_CTRL_CMP1CMOA_TOGGLE << 6) /**< Shifted mode TOGGLE for SYSRTC_GRP1_CTRL    */
#define SYSRTC_GRP1_CTRL_CMP1CMOA_CMPIF \
  (_SYSRTC_GRP1_CTRL_CMP1CMOA_CMPIF << 6)               /**< Shifted mode CMPIF for SYSRTC_GRP1_CTRL     */
#define _SYSRTC_GRP1_CTRL_CAP0EDGE_SHIFT   9            /**< Shift value for SYSRTC_CAP0EDGE             */
#define _SYSRTC_GRP1_CTRL_CAP0EDGE_MASK    0x600UL      /**< Bit mask for SYSRTC_CAP0EDGE                */
#define _SYSRTC_GRP1_CTRL_CAP0EDGE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CTRL           */
#define _SYSRTC_GRP1_CTRL_CAP0EDGE_RISING  0x00000000UL /**< Mode RISING for SYSRTC_GRP1_CTRL            */
#define _SYSRTC_GRP1_CTRL_CAP0EDGE_FALLING 0x00000001UL /**< Mode FALLING for SYSRTC_GRP1_CTRL           */
#define _SYSRTC_GRP1_CTRL_CAP0EDGE_BOTH    0x00000002UL /**< Mode BOTH for SYSRTC_GRP1_CTRL              */
#define SYSRTC_GRP1_CTRL_CAP0EDGE_DEFAULT \
  (_SYSRTC_GRP1_CTRL_CAP0EDGE_DEFAULT << 9) /**< Shifted mode DEFAULT for SYSRTC_GRP1_CTRL   */
#define SYSRTC_GRP1_CTRL_CAP0EDGE_RISING \
  (_SYSRTC_GRP1_CTRL_CAP0EDGE_RISING << 9) /**< Shifted mode RISING for SYSRTC_GRP1_CTRL    */
#define SYSRTC_GRP1_CTRL_CAP0EDGE_FALLING \
  (_SYSRTC_GRP1_CTRL_CAP0EDGE_FALLING << 9) /**< Shifted mode FALLING for SYSRTC_GRP1_CTRL   */
#define SYSRTC_GRP1_CTRL_CAP0EDGE_BOTH \
  (_SYSRTC_GRP1_CTRL_CAP0EDGE_BOTH << 9) /**< Shifted mode BOTH for SYSRTC_GRP1_CTRL      */

/* Bit fields for SYSRTC GRP1_CMP0VALUE */
#define _SYSRTC_GRP1_CMP0VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP1_CMP0VALUE     */
#define _SYSRTC_GRP1_CMP0VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP1_CMP0VALUE              */
#define _SYSRTC_GRP1_CMP0VALUE_CMP0VALUE_SHIFT   0            /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRP1_CMP0VALUE_CMP0VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRP1_CMP0VALUE_CMP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CMP0VALUE      */
#define SYSRTC_GRP1_CMP0VALUE_CMP0VALUE_DEFAULT \
  (_SYSRTC_GRP1_CMP0VALUE_CMP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP1_CMP0VALUE*/

/* Bit fields for SYSRTC GRP1_CMP1VALUE */
#define _SYSRTC_GRP1_CMP1VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP1_CMP1VALUE     */
#define _SYSRTC_GRP1_CMP1VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP1_CMP1VALUE              */
#define _SYSRTC_GRP1_CMP1VALUE_CMP1VALUE_SHIFT   0            /**< Shift value for SYSRTC_CMP1VALUE            */
#define _SYSRTC_GRP1_CMP1VALUE_CMP1VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CMP1VALUE               */
#define _SYSRTC_GRP1_CMP1VALUE_CMP1VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CMP1VALUE      */
#define SYSRTC_GRP1_CMP1VALUE_CMP1VALUE_DEFAULT \
  (_SYSRTC_GRP1_CMP1VALUE_CMP1VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP1_CMP1VALUE*/

/* Bit fields for SYSRTC GRP1_CAP0VALUE */
#define _SYSRTC_GRP1_CAP0VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP1_CAP0VALUE     */
#define _SYSRTC_GRP1_CAP0VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP1_CAP0VALUE              */
#define _SYSRTC_GRP1_CAP0VALUE_CAP0VALUE_SHIFT   0            /**< Shift value for SYSRTC_CAP0VALUE            */
#define _SYSRTC_GRP1_CAP0VALUE_CAP0VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CAP0VALUE               */
#define _SYSRTC_GRP1_CAP0VALUE_CAP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP1_CAP0VALUE      */
#define SYSRTC_GRP1_CAP0VALUE_CAP0VALUE_DEFAULT \
  (_SYSRTC_GRP1_CAP0VALUE_CAP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP1_CAP0VALUE*/

///* Bit fields for SYSRTC GRP1_SYNCBUSY */
//#define _SYSRTC_GRP1_SYNCBUSY_RESETVALUE             0x00000000UL                                   /**< Default value for SYSRTC_GRP1_SYNCBUSY      */
//#define _SYSRTC_GRP1_SYNCBUSY_MASK                   0x00000007UL                                   /**< Mask for SYSRTC_GRP1_SYNCBUSY               */
//#define SYSRTC_GRP1_SYNCBUSY_CTRL                    (0x1UL << 0)                                   /**< Sync busy for CTRL register                 */
//#define _SYSRTC_GRP1_SYNCBUSY_CTRL_SHIFT             0                                              /**< Shift value for SYSRTC_CTRL                 */
//#define _SYSRTC_GRP1_SYNCBUSY_CTRL_MASK              0x1UL                                          /**< Bit mask for SYSRTC_CTRL                    */
//#define _SYSRTC_GRP1_SYNCBUSY_CTRL_DEFAULT           0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP1_SYNCBUSY       */
//#define SYSRTC_GRP1_SYNCBUSY_CTRL_DEFAULT            (_SYSRTC_GRP1_SYNCBUSY_CTRL_DEFAULT << 0)      /**< Shifted mode DEFAULT for SYSRTC_GRP1_SYNCBUSY*/
//#define SYSRTC_GRP1_SYNCBUSY_CMP0VALUE               (0x1UL << 1)                                   /**< Sync busy for CMP0VALUE register            */
//#define _SYSRTC_GRP1_SYNCBUSY_CMP0VALUE_SHIFT        1                                              /**< Shift value for SYSRTC_CMP0VALUE            */
//#define _SYSRTC_GRP1_SYNCBUSY_CMP0VALUE_MASK         0x2UL                                          /**< Bit mask for SYSRTC_CMP0VALUE               */
//#define _SYSRTC_GRP1_SYNCBUSY_CMP0VALUE_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP1_SYNCBUSY       */
//#define SYSRTC_GRP1_SYNCBUSY_CMP0VALUE_DEFAULT       (_SYSRTC_GRP1_SYNCBUSY_CMP0VALUE_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_GRP1_SYNCBUSY*/
//#define SYSRTC_GRP1_SYNCBUSY_CMP1VALUE               (0x1UL << 2)                                   /**< Sync busy for CMP1VALUE register            */
//#define _SYSRTC_GRP1_SYNCBUSY_CMP1VALUE_SHIFT        2                                              /**< Shift value for SYSRTC_CMP1VALUE            */
//#define _SYSRTC_GRP1_SYNCBUSY_CMP1VALUE_MASK         0x4UL                                          /**< Bit mask for SYSRTC_CMP1VALUE               */
//#define _SYSRTC_GRP1_SYNCBUSY_CMP1VALUE_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP1_SYNCBUSY       */
//#define SYSRTC_GRP1_SYNCBUSY_CMP1VALUE_DEFAULT       (_SYSRTC_GRP1_SYNCBUSY_CMP1VALUE_DEFAULT << 2) /**< Shifted mode DEFAULT for SYSRTC_GRP1_SYNCBUSY*/

/* Bit fields for SYSRTC GRP2_IF */
#define _SYSRTC_GRP2_IF_RESETVALUE  0x00000000UL /**< Default value for SYSRTC_GRP2_IF            */
#define _SYSRTC_GRP2_IF_MASK        0x00000303UL /**< Mask for SYSRTC_GRP2_IF                     */
#define SYSRTC_GRP2_IF_OVF          (0x1UL << 0) /**< Overflow Interrupt Flag                     */
#define _SYSRTC_GRP2_IF_OVF_SHIFT   0            /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP2_IF_OVF_MASK    0x1UL        /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP2_IF_OVF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IF             */
#define SYSRTC_GRP2_IF_OVF_DEFAULT \
  (_SYSRTC_GRP2_IF_OVF_DEFAULT << 0)              /**< Shifted mode DEFAULT for SYSRTC_GRP2_IF     */
#define SYSRTC_GRP2_IF_CMP0          (0x1UL << 1) /**< Compare 0 Interrupt Flag                    */
#define _SYSRTC_GRP2_IF_CMP0_SHIFT   1            /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP2_IF_CMP0_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP2_IF_CMP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IF             */
#define SYSRTC_GRP2_IF_CMP0_DEFAULT \
  (_SYSRTC_GRP2_IF_CMP0_DEFAULT << 1)                /**< Shifted mode DEFAULT for SYSRTC_GRP2_IF     */
#define SYSRTC_GRP2_IF_FAILDET          (0x1UL << 8) /**< Failure Detection Interrupt Flag            */
#define _SYSRTC_GRP2_IF_FAILDET_SHIFT   8            /**< Shift value for SYSRTC_FAILDET              */
#define _SYSRTC_GRP2_IF_FAILDET_MASK    0x100UL      /**< Bit mask for SYSRTC_FAILDET                 */
#define _SYSRTC_GRP2_IF_FAILDET_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IF             */
#define SYSRTC_GRP2_IF_FAILDET_DEFAULT \
  (_SYSRTC_GRP2_IF_FAILDET_DEFAULT << 8)            /**< Shifted mode DEFAULT for SYSRTC_GRP2_IF     */
#define SYSRTC_GRP2_IF_TAMPER          (0x1UL << 9) /**< Tamper Detection Interrupt Flag             */
#define _SYSRTC_GRP2_IF_TAMPER_SHIFT   9            /**< Shift value for SYSRTC_TAMPER               */
#define _SYSRTC_GRP2_IF_TAMPER_MASK    0x200UL      /**< Bit mask for SYSRTC_TAMPER                  */
#define _SYSRTC_GRP2_IF_TAMPER_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IF             */
#define SYSRTC_GRP2_IF_TAMPER_DEFAULT \
  (_SYSRTC_GRP2_IF_TAMPER_DEFAULT << 9) /**< Shifted mode DEFAULT for SYSRTC_GRP2_IF     */

/* Bit fields for SYSRTC GRP2_IEN */
#define _SYSRTC_GRP2_IEN_RESETVALUE  0x00000000UL /**< Default value for SYSRTC_GRP2_IEN           */
#define _SYSRTC_GRP2_IEN_MASK        0x00000303UL /**< Mask for SYSRTC_GRP2_IEN                    */
#define SYSRTC_GRP2_IEN_OVF          (0x1UL << 0) /**< Overflow Interrupt Enable                   */
#define _SYSRTC_GRP2_IEN_OVF_SHIFT   0            /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP2_IEN_OVF_MASK    0x1UL        /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP2_IEN_OVF_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IEN            */
#define SYSRTC_GRP2_IEN_OVF_DEFAULT \
  (_SYSRTC_GRP2_IEN_OVF_DEFAULT << 0)              /**< Shifted mode DEFAULT for SYSRTC_GRP2_IEN    */
#define SYSRTC_GRP2_IEN_CMP0          (0x1UL << 1) /**< Compare 0 Interrupt Enable                  */
#define _SYSRTC_GRP2_IEN_CMP0_SHIFT   1            /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP2_IEN_CMP0_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP2_IEN_CMP0_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IEN            */
#define SYSRTC_GRP2_IEN_CMP0_DEFAULT \
  (_SYSRTC_GRP2_IEN_CMP0_DEFAULT << 1)                /**< Shifted mode DEFAULT for SYSRTC_GRP2_IEN    */
#define SYSRTC_GRP2_IEN_FAILDET          (0x1UL << 8) /**< Failure Detection Interrupt Enable          */
#define _SYSRTC_GRP2_IEN_FAILDET_SHIFT   8            /**< Shift value for SYSRTC_FAILDET              */
#define _SYSRTC_GRP2_IEN_FAILDET_MASK    0x100UL      /**< Bit mask for SYSRTC_FAILDET                 */
#define _SYSRTC_GRP2_IEN_FAILDET_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IEN            */
#define SYSRTC_GRP2_IEN_FAILDET_DEFAULT \
  (_SYSRTC_GRP2_IEN_FAILDET_DEFAULT << 8)            /**< Shifted mode DEFAULT for SYSRTC_GRP2_IEN    */
#define SYSRTC_GRP2_IEN_TAMPER          (0x1UL << 9) /**< Tamper Detection Interrupt Enable           */
#define _SYSRTC_GRP2_IEN_TAMPER_SHIFT   9            /**< Shift value for SYSRTC_TAMPER               */
#define _SYSRTC_GRP2_IEN_TAMPER_MASK    0x200UL      /**< Bit mask for SYSRTC_TAMPER                  */
#define _SYSRTC_GRP2_IEN_TAMPER_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_IEN            */
#define SYSRTC_GRP2_IEN_TAMPER_DEFAULT \
  (_SYSRTC_GRP2_IEN_TAMPER_DEFAULT << 9) /**< Shifted mode DEFAULT for SYSRTC_GRP2_IEN    */

/* Bit fields for SYSRTC GRP2_CTRL */
#define _SYSRTC_GRP2_CTRL_RESETVALUE     0x00000000UL /**< Default value for SYSRTC_GRP2_CTRL          */
#define _SYSRTC_GRP2_CTRL_MASK           0x00000039UL /**< Mask for SYSRTC_GRP2_CTRL                   */
#define SYSRTC_GRP2_CTRL_CMP0EN          (0x1UL << 0) /**< Compare 0 Enable                            */
#define _SYSRTC_GRP2_CTRL_CMP0EN_SHIFT   0            /**< Shift value for SYSRTC_CMP0EN               */
#define _SYSRTC_GRP2_CTRL_CMP0EN_MASK    0x1UL        /**< Bit mask for SYSRTC_CMP0EN                  */
#define _SYSRTC_GRP2_CTRL_CMP0EN_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_CTRL           */
#define SYSRTC_GRP2_CTRL_CMP0EN_DEFAULT \
  (_SYSRTC_GRP2_CTRL_CMP0EN_DEFAULT << 0)               /**< Shifted mode DEFAULT for SYSRTC_GRP2_CTRL   */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_SHIFT   3            /**< Shift value for SYSRTC_CMP0CMOA             */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_MASK    0x38UL       /**< Bit mask for SYSRTC_CMP0CMOA                */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_CTRL           */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_CLEAR   0x00000000UL /**< Mode CLEAR for SYSRTC_GRP2_CTRL             */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_SET     0x00000001UL /**< Mode SET for SYSRTC_GRP2_CTRL               */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_PULSE   0x00000002UL /**< Mode PULSE for SYSRTC_GRP2_CTRL             */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_TOGGLE  0x00000003UL /**< Mode TOGGLE for SYSRTC_GRP2_CTRL            */
#define _SYSRTC_GRP2_CTRL_CMP0CMOA_CMPIF   0x00000004UL /**< Mode CMPIF for SYSRTC_GRP2_CTRL             */
#define SYSRTC_GRP2_CTRL_CMP0CMOA_DEFAULT \
  (_SYSRTC_GRP2_CTRL_CMP0CMOA_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP2_CTRL   */
#define SYSRTC_GRP2_CTRL_CMP0CMOA_CLEAR \
  (_SYSRTC_GRP2_CTRL_CMP0CMOA_CLEAR << 3) /**< Shifted mode CLEAR for SYSRTC_GRP2_CTRL     */
#define SYSRTC_GRP2_CTRL_CMP0CMOA_SET \
  (_SYSRTC_GRP2_CTRL_CMP0CMOA_SET << 3) /**< Shifted mode SET for SYSRTC_GRP2_CTRL       */
#define SYSRTC_GRP2_CTRL_CMP0CMOA_PULSE \
  (_SYSRTC_GRP2_CTRL_CMP0CMOA_PULSE << 3) /**< Shifted mode PULSE for SYSRTC_GRP2_CTRL     */
#define SYSRTC_GRP2_CTRL_CMP0CMOA_TOGGLE \
  (_SYSRTC_GRP2_CTRL_CMP0CMOA_TOGGLE << 3) /**< Shifted mode TOGGLE for SYSRTC_GRP2_CTRL    */
#define SYSRTC_GRP2_CTRL_CMP0CMOA_CMPIF \
  (_SYSRTC_GRP2_CTRL_CMP0CMOA_CMPIF << 3) /**< Shifted mode CMPIF for SYSRTC_GRP2_CTRL     */

/* Bit fields for SYSRTC GRP2_CMP0VALUE */
#define _SYSRTC_GRP2_CMP0VALUE_RESETVALUE        0x00000000UL /**< Default value for SYSRTC_GRP2_CMP0VALUE     */
#define _SYSRTC_GRP2_CMP0VALUE_MASK              0xFFFFFFFFUL /**< Mask for SYSRTC_GRP2_CMP0VALUE              */
#define _SYSRTC_GRP2_CMP0VALUE_CMP0VALUE_SHIFT   0            /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRP2_CMP0VALUE_CMP0VALUE_MASK    0xFFFFFFFFUL /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRP2_CMP0VALUE_CMP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_CMP0VALUE      */
#define SYSRTC_GRP2_CMP0VALUE_CMP0VALUE_DEFAULT \
  (_SYSRTC_GRP2_CMP0VALUE_CMP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP2_CMP0VALUE*/

/* Bit fields for SYSRTC GRP2_SYNCBUSY */
#define _SYSRTC_GRP2_SYNCBUSY_RESETVALUE   0x00000000UL /**< Default value for SYSRTC_GRP2_SYNCBUSY      */
#define _SYSRTC_GRP2_SYNCBUSY_MASK         0x00000003UL /**< Mask for SYSRTC_GRP2_SYNCBUSY               */
#define SYSRTC_GRP2_SYNCBUSY_CTRL          (0x1UL << 0) /**< Sync busy for CTRL register                 */
#define _SYSRTC_GRP2_SYNCBUSY_CTRL_SHIFT   0            /**< Shift value for SYSRTC_CTRL                 */
#define _SYSRTC_GRP2_SYNCBUSY_CTRL_MASK    0x1UL        /**< Bit mask for SYSRTC_CTRL                    */
#define _SYSRTC_GRP2_SYNCBUSY_CTRL_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_SYNCBUSY       */
#define SYSRTC_GRP2_SYNCBUSY_CTRL_DEFAULT \
  (_SYSRTC_GRP2_SYNCBUSY_CTRL_DEFAULT << 0)                  /**< Shifted mode DEFAULT for SYSRTC_GRP2_SYNCBUSY*/
#define SYSRTC_GRP2_SYNCBUSY_CMP0VALUE          (0x1UL << 1) /**< Sync busy for CMP0VALUE register            */
#define _SYSRTC_GRP2_SYNCBUSY_CMP0VALUE_SHIFT   1            /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRP2_SYNCBUSY_CMP0VALUE_MASK    0x2UL        /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRP2_SYNCBUSY_CMP0VALUE_DEFAULT 0x00000000UL /**< Mode DEFAULT for SYSRTC_GRP2_SYNCBUSY       */
#define SYSRTC_GRP2_SYNCBUSY_CMP0VALUE_DEFAULT \
  (_SYSRTC_GRP2_SYNCBUSY_CMP0VALUE_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_GRP2_SYNCBUSY*/

/*Bit feilds for MCUSYSRTC_REG1 Register */
#define _SYSRTC_IO_FROM_GPIO      0x00000000UL /**< Value to select for GPIO mode for inputs and outputs >*/
#define _STSRTC_IO_FROM_REG       0x00000001UL /**< Value to select for register mode for inputs and outputs >*/
#define MASK_GPIO_SELECT          0x00000001UL /**< Mask for selecting GPIO or register for inputs and outputs>*/
#define SYSRTC_GRP0_CAP_INPUT_SET (0x1UL << 1) /**< Mask for group 0 - capture input register bit >*/
#define SYSRTC_GRP1_CAP_INPUT_SET (0x1UL << 2) /**< Mask for group 1 - capture input register bit >*/
#define SYSRTC_GRP0_CMP_SHIFT     (0x03UL)     /**< Mask for group 0 - compare 0 -  output register bit >*/
#define SYSRTC_GRP1_CMP_SHIFT     (0x05UL)     /**< Mask for group 0 - compare 1 -  output register bit >*/
#define SYSRTC_GRPx_CMP0_OUT_BIT  (0x01UL)     /**< Mask for group 1 - compare 0 -  output register bit >*/
#define SYSRTC_GRPx_CMP1_OUT_BIT  (0x02UL)     /**< Mask for group 1 - compare 1 -  output register bit >*/
/** @} End of group RSI_DEVICE_SYSRTC_BitFields */
/** @} End of group RSI_DEVICE_SYSRTC */
/** @} End of group Parts */

#endif /* RSI_DEVICE_SYSRTC_H */
