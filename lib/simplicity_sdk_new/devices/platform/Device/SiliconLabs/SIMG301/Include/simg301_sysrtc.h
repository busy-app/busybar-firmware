/**************************************************************************//**
 * @file
 * @brief SIMG301 SYSRTC register and bit field definitions
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
#ifndef SIMG301_SYSRTC_H
#define SIMG301_SYSRTC_H
#define SYSRTC_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_SYSRTC SYSRTC
 * @{
 * @brief SIMG301 SYSRTC Register Declaration.
 *****************************************************************************/

/** SYSRTC Register Declaration. */
typedef struct sysrtc_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP VERSION                                         */
  __IOM uint32_t EN;                            /**< Module Enable Register                             */
  __IOM uint32_t SWRST;                         /**< Software Reset Register                            */
  __IOM uint32_t CFG;                           /**< Configuration Register                             */
  __IOM uint32_t CMD;                           /**< Command Register                                   */
  __IM uint32_t  STATUS;                        /**< Status register                                    */
  __IOM uint32_t CNT;                           /**< Counter Value Register                             */
  __IM uint32_t  SYNCBUSY;                      /**< Synchronization Busy Register                      */
  __IOM uint32_t LOCK;                          /**< Configuration Lock Register                        */
  __IM uint32_t  MSCNT;                         /**< Millisecond Counter Register                       */
  __IOM uint32_t MSCMPVAL;                      /**< Millisecond Compare Register                       */
  __IOM uint32_t MSCMPBUF;                      /**< Millisecond Compare Buffer Register                */
  __IOM uint32_t IF;                            /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN;                           /**< Interrupt Enable Register                          */
  uint32_t       RESERVED0[2U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED2[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t GRP0_IF;                       /**< Group Interrupt Flags                              */
  __IOM uint32_t GRP0_IEN;                      /**< Group Interrupt Enables                            */
  __IOM uint32_t GRP0_CTRL;                     /**< Group Control Register                             */
  __IOM uint32_t GRP0_CMP0VALUE;                /**< Compare 0 Value Register                           */
  __IOM uint32_t GRP0_CMP1VALUE;                /**< Compare 1 Value Register                           */
  __IM uint32_t  GRP0_CAP0VALUE;                /**< Capture 0 Value Register                           */
  __IM uint32_t  GRP0_SYNCBUSY;                 /**< Synchronization busy Register                      */
  __IOM uint32_t GRP0_PRETRIG;                  /**< Pretrigger Register                                */
  uint32_t       RESERVED3[4U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED4[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED5[11U];                /**< Reserved for future use                            */
  uint32_t       RESERVED6[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED7[979U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP VERSION                                         */
  __IOM uint32_t EN_SET;                        /**< Module Enable Register                             */
  __IOM uint32_t SWRST_SET;                     /**< Software Reset Register                            */
  __IOM uint32_t CFG_SET;                       /**< Configuration Register                             */
  __IOM uint32_t CMD_SET;                       /**< Command Register                                   */
  __IM uint32_t  STATUS_SET;                    /**< Status register                                    */
  __IOM uint32_t CNT_SET;                       /**< Counter Value Register                             */
  __IM uint32_t  SYNCBUSY_SET;                  /**< Synchronization Busy Register                      */
  __IOM uint32_t LOCK_SET;                      /**< Configuration Lock Register                        */
  __IM uint32_t  MSCNT_SET;                     /**< Millisecond Counter Register                       */
  __IOM uint32_t MSCMPVAL_SET;                  /**< Millisecond Compare Register                       */
  __IOM uint32_t MSCMPBUF_SET;                  /**< Millisecond Compare Buffer Register                */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED8[2U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED9[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED10[3U];                /**< Reserved for future use                            */
  __IOM uint32_t GRP0_IF_SET;                   /**< Group Interrupt Flags                              */
  __IOM uint32_t GRP0_IEN_SET;                  /**< Group Interrupt Enables                            */
  __IOM uint32_t GRP0_CTRL_SET;                 /**< Group Control Register                             */
  __IOM uint32_t GRP0_CMP0VALUE_SET;            /**< Compare 0 Value Register                           */
  __IOM uint32_t GRP0_CMP1VALUE_SET;            /**< Compare 1 Value Register                           */
  __IM uint32_t  GRP0_CAP0VALUE_SET;            /**< Capture 0 Value Register                           */
  __IM uint32_t  GRP0_SYNCBUSY_SET;             /**< Synchronization busy Register                      */
  __IOM uint32_t GRP0_PRETRIG_SET;              /**< Pretrigger Register                                */
  uint32_t       RESERVED11[4U];                /**< Reserved for future use                            */
  uint32_t       RESERVED12[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED13[11U];               /**< Reserved for future use                            */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED15[979U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP VERSION                                         */
  __IOM uint32_t EN_CLR;                        /**< Module Enable Register                             */
  __IOM uint32_t SWRST_CLR;                     /**< Software Reset Register                            */
  __IOM uint32_t CFG_CLR;                       /**< Configuration Register                             */
  __IOM uint32_t CMD_CLR;                       /**< Command Register                                   */
  __IM uint32_t  STATUS_CLR;                    /**< Status register                                    */
  __IOM uint32_t CNT_CLR;                       /**< Counter Value Register                             */
  __IM uint32_t  SYNCBUSY_CLR;                  /**< Synchronization Busy Register                      */
  __IOM uint32_t LOCK_CLR;                      /**< Configuration Lock Register                        */
  __IM uint32_t  MSCNT_CLR;                     /**< Millisecond Counter Register                       */
  __IOM uint32_t MSCMPVAL_CLR;                  /**< Millisecond Compare Register                       */
  __IOM uint32_t MSCMPBUF_CLR;                  /**< Millisecond Compare Buffer Register                */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED16[2U];                /**< Reserved for future use                            */
  uint32_t       RESERVED17[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED18[3U];                /**< Reserved for future use                            */
  __IOM uint32_t GRP0_IF_CLR;                   /**< Group Interrupt Flags                              */
  __IOM uint32_t GRP0_IEN_CLR;                  /**< Group Interrupt Enables                            */
  __IOM uint32_t GRP0_CTRL_CLR;                 /**< Group Control Register                             */
  __IOM uint32_t GRP0_CMP0VALUE_CLR;            /**< Compare 0 Value Register                           */
  __IOM uint32_t GRP0_CMP1VALUE_CLR;            /**< Compare 1 Value Register                           */
  __IM uint32_t  GRP0_CAP0VALUE_CLR;            /**< Capture 0 Value Register                           */
  __IM uint32_t  GRP0_SYNCBUSY_CLR;             /**< Synchronization busy Register                      */
  __IOM uint32_t GRP0_PRETRIG_CLR;              /**< Pretrigger Register                                */
  uint32_t       RESERVED19[4U];                /**< Reserved for future use                            */
  uint32_t       RESERVED20[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED21[11U];               /**< Reserved for future use                            */
  uint32_t       RESERVED22[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED23[979U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP VERSION                                         */
  __IOM uint32_t EN_TGL;                        /**< Module Enable Register                             */
  __IOM uint32_t SWRST_TGL;                     /**< Software Reset Register                            */
  __IOM uint32_t CFG_TGL;                       /**< Configuration Register                             */
  __IOM uint32_t CMD_TGL;                       /**< Command Register                                   */
  __IM uint32_t  STATUS_TGL;                    /**< Status register                                    */
  __IOM uint32_t CNT_TGL;                       /**< Counter Value Register                             */
  __IM uint32_t  SYNCBUSY_TGL;                  /**< Synchronization Busy Register                      */
  __IOM uint32_t LOCK_TGL;                      /**< Configuration Lock Register                        */
  __IM uint32_t  MSCNT_TGL;                     /**< Millisecond Counter Register                       */
  __IOM uint32_t MSCMPVAL_TGL;                  /**< Millisecond Compare Register                       */
  __IOM uint32_t MSCMPBUF_TGL;                  /**< Millisecond Compare Buffer Register                */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED24[2U];                /**< Reserved for future use                            */
  uint32_t       RESERVED25[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED26[3U];                /**< Reserved for future use                            */
  __IOM uint32_t GRP0_IF_TGL;                   /**< Group Interrupt Flags                              */
  __IOM uint32_t GRP0_IEN_TGL;                  /**< Group Interrupt Enables                            */
  __IOM uint32_t GRP0_CTRL_TGL;                 /**< Group Control Register                             */
  __IOM uint32_t GRP0_CMP0VALUE_TGL;            /**< Compare 0 Value Register                           */
  __IOM uint32_t GRP0_CMP1VALUE_TGL;            /**< Compare 1 Value Register                           */
  __IM uint32_t  GRP0_CAP0VALUE_TGL;            /**< Capture 0 Value Register                           */
  __IM uint32_t  GRP0_SYNCBUSY_TGL;             /**< Synchronization busy Register                      */
  __IOM uint32_t GRP0_PRETRIG_TGL;              /**< Pretrigger Register                                */
  uint32_t       RESERVED27[4U];                /**< Reserved for future use                            */
  uint32_t       RESERVED28[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED29[11U];               /**< Reserved for future use                            */
  uint32_t       RESERVED30[1U];                /**< Reserved for future use                            */
} SYSRTC_TypeDef;
/** @} End of group SIMG301_SYSRTC */

/**************************************************************************//**
 * @addtogroup SIMG301_SYSRTC
 * @{
 * @defgroup SIMG301_SYSRTC_BitFields SYSRTC Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SYSRTC IPVERSION */
#define _SYSRTC_IPVERSION_RESETVALUE                0x00000002UL                               /**< Default value for SYSRTC_IPVERSION          */
#define _SYSRTC_IPVERSION_MASK                      0xFFFFFFFFUL                               /**< Mask for SYSRTC_IPVERSION                   */
#define _SYSRTC_IPVERSION_IPVERSION_SHIFT           0                                          /**< Shift value for SYSRTC_IPVERSION            */
#define _SYSRTC_IPVERSION_IPVERSION_MASK            0xFFFFFFFFUL                               /**< Bit mask for SYSRTC_IPVERSION               */
#define _SYSRTC_IPVERSION_IPVERSION_DEFAULT         0x00000002UL                               /**< Mode DEFAULT for SYSRTC_IPVERSION           */
#define SYSRTC_IPVERSION_IPVERSION_DEFAULT          (_SYSRTC_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_IPVERSION   */

/* Bit fields for SYSRTC EN */
#define _SYSRTC_EN_RESETVALUE                       0x00000000UL                        /**< Default value for SYSRTC_EN                 */
#define _SYSRTC_EN_MASK                             0x00000003UL                        /**< Mask for SYSRTC_EN                          */
#define SYSRTC_EN_EN                                (0x1UL << 0)                        /**< SYSRTC Enable                               */
#define _SYSRTC_EN_EN_SHIFT                         0                                   /**< Shift value for SYSRTC_EN                   */
#define _SYSRTC_EN_EN_MASK                          0x1UL                               /**< Bit mask for SYSRTC_EN                      */
#define _SYSRTC_EN_EN_DEFAULT                       0x00000000UL                        /**< Mode DEFAULT for SYSRTC_EN                  */
#define SYSRTC_EN_EN_DEFAULT                        (_SYSRTC_EN_EN_DEFAULT << 0)        /**< Shifted mode DEFAULT for SYSRTC_EN          */
#define SYSRTC_EN_DISABLING                         (0x1UL << 1)                        /**< Disablement busy status                     */
#define _SYSRTC_EN_DISABLING_SHIFT                  1                                   /**< Shift value for SYSRTC_DISABLING            */
#define _SYSRTC_EN_DISABLING_MASK                   0x2UL                               /**< Bit mask for SYSRTC_DISABLING               */
#define _SYSRTC_EN_DISABLING_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SYSRTC_EN                  */
#define SYSRTC_EN_DISABLING_DEFAULT                 (_SYSRTC_EN_DISABLING_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_EN          */

/* Bit fields for SYSRTC SWRST */
#define _SYSRTC_SWRST_RESETVALUE                    0x00000000UL                           /**< Default value for SYSRTC_SWRST              */
#define _SYSRTC_SWRST_MASK                          0x00000003UL                           /**< Mask for SYSRTC_SWRST                       */
#define SYSRTC_SWRST_SWRST                          (0x1UL << 0)                           /**< Software reset command                      */
#define _SYSRTC_SWRST_SWRST_SHIFT                   0                                      /**< Shift value for SYSRTC_SWRST                */
#define _SYSRTC_SWRST_SWRST_MASK                    0x1UL                                  /**< Bit mask for SYSRTC_SWRST                   */
#define _SYSRTC_SWRST_SWRST_DEFAULT                 0x00000000UL                           /**< Mode DEFAULT for SYSRTC_SWRST               */
#define SYSRTC_SWRST_SWRST_DEFAULT                  (_SYSRTC_SWRST_SWRST_DEFAULT << 0)     /**< Shifted mode DEFAULT for SYSRTC_SWRST       */
#define SYSRTC_SWRST_RESETTING                      (0x1UL << 1)                           /**< Software reset busy status                  */
#define _SYSRTC_SWRST_RESETTING_SHIFT               1                                      /**< Shift value for SYSRTC_RESETTING            */
#define _SYSRTC_SWRST_RESETTING_MASK                0x2UL                                  /**< Bit mask for SYSRTC_RESETTING               */
#define _SYSRTC_SWRST_RESETTING_DEFAULT             0x00000000UL                           /**< Mode DEFAULT for SYSRTC_SWRST               */
#define SYSRTC_SWRST_RESETTING_DEFAULT              (_SYSRTC_SWRST_RESETTING_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_SWRST       */

/* Bit fields for SYSRTC CFG */
#define _SYSRTC_CFG_RESETVALUE                      0x00000000UL                        /**< Default value for SYSRTC_CFG                */
#define _SYSRTC_CFG_MASK                            0x00000001UL                        /**< Mask for SYSRTC_CFG                         */
#define SYSRTC_CFG_DEBUGRUN                         (0x1UL << 0)                        /**< Debug Mode Run Enable                       */
#define _SYSRTC_CFG_DEBUGRUN_SHIFT                  0                                   /**< Shift value for SYSRTC_DEBUGRUN             */
#define _SYSRTC_CFG_DEBUGRUN_MASK                   0x1UL                               /**< Bit mask for SYSRTC_DEBUGRUN                */
#define _SYSRTC_CFG_DEBUGRUN_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SYSRTC_CFG                 */
#define _SYSRTC_CFG_DEBUGRUN_DISABLE                0x00000000UL                        /**< Mode DISABLE for SYSRTC_CFG                 */
#define _SYSRTC_CFG_DEBUGRUN_ENABLE                 0x00000001UL                        /**< Mode ENABLE for SYSRTC_CFG                  */
#define SYSRTC_CFG_DEBUGRUN_DEFAULT                 (_SYSRTC_CFG_DEBUGRUN_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_CFG         */
#define SYSRTC_CFG_DEBUGRUN_DISABLE                 (_SYSRTC_CFG_DEBUGRUN_DISABLE << 0) /**< Shifted mode DISABLE for SYSRTC_CFG         */
#define SYSRTC_CFG_DEBUGRUN_ENABLE                  (_SYSRTC_CFG_DEBUGRUN_ENABLE << 0)  /**< Shifted mode ENABLE for SYSRTC_CFG          */

/* Bit fields for SYSRTC CMD */
#define _SYSRTC_CMD_RESETVALUE                      0x00000000UL                        /**< Default value for SYSRTC_CMD                */
#define _SYSRTC_CMD_MASK                            0x0000000FUL                        /**< Mask for SYSRTC_CMD                         */
#define SYSRTC_CMD_START                            (0x1UL << 0)                        /**< Start SYSRTC                                */
#define _SYSRTC_CMD_START_SHIFT                     0                                   /**< Shift value for SYSRTC_START                */
#define _SYSRTC_CMD_START_MASK                      0x1UL                               /**< Bit mask for SYSRTC_START                   */
#define _SYSRTC_CMD_START_DEFAULT                   0x00000000UL                        /**< Mode DEFAULT for SYSRTC_CMD                 */
#define SYSRTC_CMD_START_DEFAULT                    (_SYSRTC_CMD_START_DEFAULT << 0)    /**< Shifted mode DEFAULT for SYSRTC_CMD         */
#define SYSRTC_CMD_STOP                             (0x1UL << 1)                        /**< Stop SYSRTC                                 */
#define _SYSRTC_CMD_STOP_SHIFT                      1                                   /**< Shift value for SYSRTC_STOP                 */
#define _SYSRTC_CMD_STOP_MASK                       0x2UL                               /**< Bit mask for SYSRTC_STOP                    */
#define _SYSRTC_CMD_STOP_DEFAULT                    0x00000000UL                        /**< Mode DEFAULT for SYSRTC_CMD                 */
#define SYSRTC_CMD_STOP_DEFAULT                     (_SYSRTC_CMD_STOP_DEFAULT << 1)     /**< Shifted mode DEFAULT for SYSRTC_CMD         */
#define SYSRTC_CMD_MSSTART                          (0x1UL << 2)                        /**< Start ms counter                            */
#define _SYSRTC_CMD_MSSTART_SHIFT                   2                                   /**< Shift value for SYSRTC_MSSTART              */
#define _SYSRTC_CMD_MSSTART_MASK                    0x4UL                               /**< Bit mask for SYSRTC_MSSTART                 */
#define _SYSRTC_CMD_MSSTART_DEFAULT                 0x00000000UL                        /**< Mode DEFAULT for SYSRTC_CMD                 */
#define SYSRTC_CMD_MSSTART_DEFAULT                  (_SYSRTC_CMD_MSSTART_DEFAULT << 2)  /**< Shifted mode DEFAULT for SYSRTC_CMD         */
#define SYSRTC_CMD_MSSTOP                           (0x1UL << 3)                        /**< stop ms counter                             */
#define _SYSRTC_CMD_MSSTOP_SHIFT                    3                                   /**< Shift value for SYSRTC_MSSTOP               */
#define _SYSRTC_CMD_MSSTOP_MASK                     0x8UL                               /**< Bit mask for SYSRTC_MSSTOP                  */
#define _SYSRTC_CMD_MSSTOP_DEFAULT                  0x00000000UL                        /**< Mode DEFAULT for SYSRTC_CMD                 */
#define SYSRTC_CMD_MSSTOP_DEFAULT                   (_SYSRTC_CMD_MSSTOP_DEFAULT << 3)   /**< Shifted mode DEFAULT for SYSRTC_CMD         */

/* Bit fields for SYSRTC STATUS */
#define _SYSRTC_STATUS_RESETVALUE                   0x00000000UL                              /**< Default value for SYSRTC_STATUS             */
#define _SYSRTC_STATUS_MASK                         0x0000000FUL                              /**< Mask for SYSRTC_STATUS                      */
#define SYSRTC_STATUS_RUNNING                       (0x1UL << 0)                              /**< SYSRTC running status                       */
#define _SYSRTC_STATUS_RUNNING_SHIFT                0                                         /**< Shift value for SYSRTC_RUNNING              */
#define _SYSRTC_STATUS_RUNNING_MASK                 0x1UL                                     /**< Bit mask for SYSRTC_RUNNING                 */
#define _SYSRTC_STATUS_RUNNING_DEFAULT              0x00000000UL                              /**< Mode DEFAULT for SYSRTC_STATUS              */
#define SYSRTC_STATUS_RUNNING_DEFAULT               (_SYSRTC_STATUS_RUNNING_DEFAULT << 0)     /**< Shifted mode DEFAULT for SYSRTC_STATUS      */
#define SYSRTC_STATUS_LOCKSTATUS                    (0x1UL << 1)                              /**< Lock Status                                 */
#define _SYSRTC_STATUS_LOCKSTATUS_SHIFT             1                                         /**< Shift value for SYSRTC_LOCKSTATUS           */
#define _SYSRTC_STATUS_LOCKSTATUS_MASK              0x2UL                                     /**< Bit mask for SYSRTC_LOCKSTATUS              */
#define _SYSRTC_STATUS_LOCKSTATUS_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for SYSRTC_STATUS              */
#define _SYSRTC_STATUS_LOCKSTATUS_UNLOCKED          0x00000000UL                              /**< Mode UNLOCKED for SYSRTC_STATUS             */
#define _SYSRTC_STATUS_LOCKSTATUS_LOCKED            0x00000001UL                              /**< Mode LOCKED for SYSRTC_STATUS               */
#define SYSRTC_STATUS_LOCKSTATUS_DEFAULT            (_SYSRTC_STATUS_LOCKSTATUS_DEFAULT << 1)  /**< Shifted mode DEFAULT for SYSRTC_STATUS      */
#define SYSRTC_STATUS_LOCKSTATUS_UNLOCKED           (_SYSRTC_STATUS_LOCKSTATUS_UNLOCKED << 1) /**< Shifted mode UNLOCKED for SYSRTC_STATUS     */
#define SYSRTC_STATUS_LOCKSTATUS_LOCKED             (_SYSRTC_STATUS_LOCKSTATUS_LOCKED << 1)   /**< Shifted mode LOCKED for SYSRTC_STATUS       */
#define SYSRTC_STATUS_MSRUNNING                     (0x1UL << 3)                              /**< MS-Counter running status                   */
#define _SYSRTC_STATUS_MSRUNNING_SHIFT              3                                         /**< Shift value for SYSRTC_MSRUNNING            */
#define _SYSRTC_STATUS_MSRUNNING_MASK               0x8UL                                     /**< Bit mask for SYSRTC_MSRUNNING               */
#define _SYSRTC_STATUS_MSRUNNING_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for SYSRTC_STATUS              */
#define SYSRTC_STATUS_MSRUNNING_DEFAULT             (_SYSRTC_STATUS_MSRUNNING_DEFAULT << 3)   /**< Shifted mode DEFAULT for SYSRTC_STATUS      */

/* Bit fields for SYSRTC CNT */
#define _SYSRTC_CNT_RESETVALUE                      0x00000000UL                        /**< Default value for SYSRTC_CNT                */
#define _SYSRTC_CNT_MASK                            0xFFFFFFFFUL                        /**< Mask for SYSRTC_CNT                         */
#define _SYSRTC_CNT_CNT_SHIFT                       0                                   /**< Shift value for SYSRTC_CNT                  */
#define _SYSRTC_CNT_CNT_MASK                        0xFFFFFFFFUL                        /**< Bit mask for SYSRTC_CNT                     */
#define _SYSRTC_CNT_CNT_DEFAULT                     0x00000000UL                        /**< Mode DEFAULT for SYSRTC_CNT                 */
#define SYSRTC_CNT_CNT_DEFAULT                      (_SYSRTC_CNT_CNT_DEFAULT << 0)      /**< Shifted mode DEFAULT for SYSRTC_CNT         */

/* Bit fields for SYSRTC SYNCBUSY */
#define _SYSRTC_SYNCBUSY_RESETVALUE                 0x00000000UL                          /**< Default value for SYSRTC_SYNCBUSY           */
#define _SYSRTC_SYNCBUSY_MASK                       0x0000000FUL                          /**< Mask for SYSRTC_SYNCBUSY                    */
#define SYSRTC_SYNCBUSY_START                       (0x1UL << 0)                          /**< Sync busy for START bitfield                */
#define _SYSRTC_SYNCBUSY_START_SHIFT                0                                     /**< Shift value for SYSRTC_START                */
#define _SYSRTC_SYNCBUSY_START_MASK                 0x1UL                                 /**< Bit mask for SYSRTC_START                   */
#define _SYSRTC_SYNCBUSY_START_DEFAULT              0x00000000UL                          /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_START_DEFAULT               (_SYSRTC_SYNCBUSY_START_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */
#define SYSRTC_SYNCBUSY_STOP                        (0x1UL << 1)                          /**< Sync busy for STOP bitfield                 */
#define _SYSRTC_SYNCBUSY_STOP_SHIFT                 1                                     /**< Shift value for SYSRTC_STOP                 */
#define _SYSRTC_SYNCBUSY_STOP_MASK                  0x2UL                                 /**< Bit mask for SYSRTC_STOP                    */
#define _SYSRTC_SYNCBUSY_STOP_DEFAULT               0x00000000UL                          /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_STOP_DEFAULT                (_SYSRTC_SYNCBUSY_STOP_DEFAULT << 1)  /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */
#define SYSRTC_SYNCBUSY_CNT                         (0x1UL << 2)                          /**< Sync busy for CNT bitfield                  */
#define _SYSRTC_SYNCBUSY_CNT_SHIFT                  2                                     /**< Shift value for SYSRTC_CNT                  */
#define _SYSRTC_SYNCBUSY_CNT_MASK                   0x4UL                                 /**< Bit mask for SYSRTC_CNT                     */
#define _SYSRTC_SYNCBUSY_CNT_DEFAULT                0x00000000UL                          /**< Mode DEFAULT for SYSRTC_SYNCBUSY            */
#define SYSRTC_SYNCBUSY_CNT_DEFAULT                 (_SYSRTC_SYNCBUSY_CNT_DEFAULT << 2)   /**< Shifted mode DEFAULT for SYSRTC_SYNCBUSY    */

/* Bit fields for SYSRTC LOCK */
#define _SYSRTC_LOCK_RESETVALUE                     0x00000000UL                        /**< Default value for SYSRTC_LOCK               */
#define _SYSRTC_LOCK_MASK                           0x0000FFFFUL                        /**< Mask for SYSRTC_LOCK                        */
#define _SYSRTC_LOCK_LOCKKEY_SHIFT                  0                                   /**< Shift value for SYSRTC_LOCKKEY              */
#define _SYSRTC_LOCK_LOCKKEY_MASK                   0xFFFFUL                            /**< Bit mask for SYSRTC_LOCKKEY                 */
#define _SYSRTC_LOCK_LOCKKEY_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SYSRTC_LOCK                */
#define _SYSRTC_LOCK_LOCKKEY_UNLOCK                 0x00004776UL                        /**< Mode UNLOCK for SYSRTC_LOCK                 */
#define SYSRTC_LOCK_LOCKKEY_DEFAULT                 (_SYSRTC_LOCK_LOCKKEY_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_LOCK        */
#define SYSRTC_LOCK_LOCKKEY_UNLOCK                  (_SYSRTC_LOCK_LOCKKEY_UNLOCK << 0)  /**< Shifted mode UNLOCK for SYSRTC_LOCK         */

/* Bit fields for SYSRTC MSCNT */
#define _SYSRTC_MSCNT_RESETVALUE                    0x00000000UL                        /**< Default value for SYSRTC_MSCNT              */
#define _SYSRTC_MSCNT_MASK                          0xFFFFFFFFUL                        /**< Mask for SYSRTC_MSCNT                       */
#define _SYSRTC_MSCNT_MSCNT_SHIFT                   0                                   /**< Shift value for SYSRTC_MSCNT                */
#define _SYSRTC_MSCNT_MSCNT_MASK                    0xFFFFFFFFUL                        /**< Bit mask for SYSRTC_MSCNT                   */
#define _SYSRTC_MSCNT_MSCNT_DEFAULT                 0x00000000UL                        /**< Mode DEFAULT for SYSRTC_MSCNT               */
#define SYSRTC_MSCNT_MSCNT_DEFAULT                  (_SYSRTC_MSCNT_MSCNT_DEFAULT << 0)  /**< Shifted mode DEFAULT for SYSRTC_MSCNT       */

/* Bit fields for SYSRTC MSCMPVAL */
#define _SYSRTC_MSCMPVAL_RESETVALUE                 0x00000000UL                             /**< Default value for SYSRTC_MSCMPVAL           */
#define _SYSRTC_MSCMPVAL_MASK                       0xFFFFFFFFUL                             /**< Mask for SYSRTC_MSCMPVAL                    */
#define _SYSRTC_MSCMPVAL_MSCMPVAL_SHIFT             0                                        /**< Shift value for SYSRTC_MSCMPVAL             */
#define _SYSRTC_MSCMPVAL_MSCMPVAL_MASK              0xFFFFFFFFUL                             /**< Bit mask for SYSRTC_MSCMPVAL                */
#define _SYSRTC_MSCMPVAL_MSCMPVAL_DEFAULT           0x00000000UL                             /**< Mode DEFAULT for SYSRTC_MSCMPVAL            */
#define SYSRTC_MSCMPVAL_MSCMPVAL_DEFAULT            (_SYSRTC_MSCMPVAL_MSCMPVAL_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_MSCMPVAL    */

/* Bit fields for SYSRTC MSCMPBUF */
#define _SYSRTC_MSCMPBUF_RESETVALUE                 0x00000000UL                             /**< Default value for SYSRTC_MSCMPBUF           */
#define _SYSRTC_MSCMPBUF_MASK                       0xFFFFFFFFUL                             /**< Mask for SYSRTC_MSCMPBUF                    */
#define _SYSRTC_MSCMPBUF_MSCMPBUF_SHIFT             0                                        /**< Shift value for SYSRTC_MSCMPBUF             */
#define _SYSRTC_MSCMPBUF_MSCMPBUF_MASK              0xFFFFFFFFUL                             /**< Bit mask for SYSRTC_MSCMPBUF                */
#define _SYSRTC_MSCMPBUF_MSCMPBUF_DEFAULT           0x00000000UL                             /**< Mode DEFAULT for SYSRTC_MSCMPBUF            */
#define SYSRTC_MSCMPBUF_MSCMPBUF_DEFAULT            (_SYSRTC_MSCMPBUF_MSCMPBUF_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_MSCMPBUF    */

/* Bit fields for SYSRTC IF */
#define _SYSRTC_IF_RESETVALUE                       0x00000000UL                        /**< Default value for SYSRTC_IF                 */
#define _SYSRTC_IF_MASK                             0x00000003UL                        /**< Mask for SYSRTC_IF                          */
#define SYSRTC_IF_MSOVF                             (0x1UL << 0)                        /**< MS Counter Overflow                         */
#define _SYSRTC_IF_MSOVF_SHIFT                      0                                   /**< Shift value for SYSRTC_MSOVF                */
#define _SYSRTC_IF_MSOVF_MASK                       0x1UL                               /**< Bit mask for SYSRTC_MSOVF                   */
#define _SYSRTC_IF_MSOVF_DEFAULT                    0x00000000UL                        /**< Mode DEFAULT for SYSRTC_IF                  */
#define SYSRTC_IF_MSOVF_DEFAULT                     (_SYSRTC_IF_MSOVF_DEFAULT << 0)     /**< Shifted mode DEFAULT for SYSRTC_IF          */
#define SYSRTC_IF_MSCMP                             (0x1UL << 1)                        /**< MS Counter Compare                          */
#define _SYSRTC_IF_MSCMP_SHIFT                      1                                   /**< Shift value for SYSRTC_MSCMP                */
#define _SYSRTC_IF_MSCMP_MASK                       0x2UL                               /**< Bit mask for SYSRTC_MSCMP                   */
#define _SYSRTC_IF_MSCMP_DEFAULT                    0x00000000UL                        /**< Mode DEFAULT for SYSRTC_IF                  */
#define SYSRTC_IF_MSCMP_DEFAULT                     (_SYSRTC_IF_MSCMP_DEFAULT << 1)     /**< Shifted mode DEFAULT for SYSRTC_IF          */

/* Bit fields for SYSRTC IEN */
#define _SYSRTC_IEN_RESETVALUE                      0x00000000UL                        /**< Default value for SYSRTC_IEN                */
#define _SYSRTC_IEN_MASK                            0x00000003UL                        /**< Mask for SYSRTC_IEN                         */
#define SYSRTC_IEN_MSOVF                            (0x1UL << 0)                        /**< MS Counter Overflow                         */
#define _SYSRTC_IEN_MSOVF_SHIFT                     0                                   /**< Shift value for SYSRTC_MSOVF                */
#define _SYSRTC_IEN_MSOVF_MASK                      0x1UL                               /**< Bit mask for SYSRTC_MSOVF                   */
#define _SYSRTC_IEN_MSOVF_DEFAULT                   0x00000000UL                        /**< Mode DEFAULT for SYSRTC_IEN                 */
#define SYSRTC_IEN_MSOVF_DEFAULT                    (_SYSRTC_IEN_MSOVF_DEFAULT << 0)    /**< Shifted mode DEFAULT for SYSRTC_IEN         */
#define SYSRTC_IEN_MSCMP                            (0x1UL << 1)                        /**< MS Counter Compare                          */
#define _SYSRTC_IEN_MSCMP_SHIFT                     1                                   /**< Shift value for SYSRTC_MSCMP                */
#define _SYSRTC_IEN_MSCMP_MASK                      0x2UL                               /**< Bit mask for SYSRTC_MSCMP                   */
#define _SYSRTC_IEN_MSCMP_DEFAULT                   0x00000000UL                        /**< Mode DEFAULT for SYSRTC_IEN                 */
#define SYSRTC_IEN_MSCMP_DEFAULT                    (_SYSRTC_IEN_MSCMP_DEFAULT << 1)    /**< Shifted mode DEFAULT for SYSRTC_IEN         */

/* Bit fields for SYSRTC GRP0_IF */
#define _SYSRTC_GRP0_IF_RESETVALUE                  0x00000000UL                        /**< Default value for SYSRTC_GRP0_IF            */
#define _SYSRTC_GRP0_IF_MASK                        0x0000000FUL                        /**< Mask for SYSRTC_GRP0_IF                     */
#define SYSRTC_GRP0_IF_OVF                          (0x1UL << 0)                        /**< Overflow Interrupt Flag                     */
#define _SYSRTC_GRP0_IF_OVF_SHIFT                   0                                   /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP0_IF_OVF_MASK                    0x1UL                               /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP0_IF_OVF_DEFAULT                 0x00000000UL                        /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_OVF_DEFAULT                  (_SYSRTC_GRP0_IF_OVF_DEFAULT << 0)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */
#define SYSRTC_GRP0_IF_CMP0                         (0x1UL << 1)                        /**< Compare 0 Interrupt Flag                    */
#define _SYSRTC_GRP0_IF_CMP0_SHIFT                  1                                   /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP0_IF_CMP0_MASK                   0x2UL                               /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP0_IF_CMP0_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_CMP0_DEFAULT                 (_SYSRTC_GRP0_IF_CMP0_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */
#define SYSRTC_GRP0_IF_CMP1                         (0x1UL << 2)                        /**< Compare 1 Interrupt Flag                    */
#define _SYSRTC_GRP0_IF_CMP1_SHIFT                  2                                   /**< Shift value for SYSRTC_CMP1                 */
#define _SYSRTC_GRP0_IF_CMP1_MASK                   0x4UL                               /**< Bit mask for SYSRTC_CMP1                    */
#define _SYSRTC_GRP0_IF_CMP1_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_CMP1_DEFAULT                 (_SYSRTC_GRP0_IF_CMP1_DEFAULT << 2) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */
#define SYSRTC_GRP0_IF_CAP0                         (0x1UL << 3)                        /**< Capture 0 Interrupt Flag                    */
#define _SYSRTC_GRP0_IF_CAP0_SHIFT                  3                                   /**< Shift value for SYSRTC_CAP0                 */
#define _SYSRTC_GRP0_IF_CAP0_MASK                   0x8UL                               /**< Bit mask for SYSRTC_CAP0                    */
#define _SYSRTC_GRP0_IF_CAP0_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SYSRTC_GRP0_IF             */
#define SYSRTC_GRP0_IF_CAP0_DEFAULT                 (_SYSRTC_GRP0_IF_CAP0_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IF     */

/* Bit fields for SYSRTC GRP0_IEN */
#define _SYSRTC_GRP0_IEN_RESETVALUE                 0x00000000UL                         /**< Default value for SYSRTC_GRP0_IEN           */
#define _SYSRTC_GRP0_IEN_MASK                       0x0000000FUL                         /**< Mask for SYSRTC_GRP0_IEN                    */
#define SYSRTC_GRP0_IEN_OVF                         (0x1UL << 0)                         /**< Overflow Interrupt Enable                   */
#define _SYSRTC_GRP0_IEN_OVF_SHIFT                  0                                    /**< Shift value for SYSRTC_OVF                  */
#define _SYSRTC_GRP0_IEN_OVF_MASK                   0x1UL                                /**< Bit mask for SYSRTC_OVF                     */
#define _SYSRTC_GRP0_IEN_OVF_DEFAULT                0x00000000UL                         /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_OVF_DEFAULT                 (_SYSRTC_GRP0_IEN_OVF_DEFAULT << 0)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */
#define SYSRTC_GRP0_IEN_CMP0                        (0x1UL << 1)                         /**< Compare 0 Interrupt Enable                  */
#define _SYSRTC_GRP0_IEN_CMP0_SHIFT                 1                                    /**< Shift value for SYSRTC_CMP0                 */
#define _SYSRTC_GRP0_IEN_CMP0_MASK                  0x2UL                                /**< Bit mask for SYSRTC_CMP0                    */
#define _SYSRTC_GRP0_IEN_CMP0_DEFAULT               0x00000000UL                         /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_CMP0_DEFAULT                (_SYSRTC_GRP0_IEN_CMP0_DEFAULT << 1) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */
#define SYSRTC_GRP0_IEN_CMP1                        (0x1UL << 2)                         /**< Compare 1 Interrupt Enable                  */
#define _SYSRTC_GRP0_IEN_CMP1_SHIFT                 2                                    /**< Shift value for SYSRTC_CMP1                 */
#define _SYSRTC_GRP0_IEN_CMP1_MASK                  0x4UL                                /**< Bit mask for SYSRTC_CMP1                    */
#define _SYSRTC_GRP0_IEN_CMP1_DEFAULT               0x00000000UL                         /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_CMP1_DEFAULT                (_SYSRTC_GRP0_IEN_CMP1_DEFAULT << 2) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */
#define SYSRTC_GRP0_IEN_CAP0                        (0x1UL << 3)                         /**< Capture 0 Interrupt Enable                  */
#define _SYSRTC_GRP0_IEN_CAP0_SHIFT                 3                                    /**< Shift value for SYSRTC_CAP0                 */
#define _SYSRTC_GRP0_IEN_CAP0_MASK                  0x8UL                                /**< Bit mask for SYSRTC_CAP0                    */
#define _SYSRTC_GRP0_IEN_CAP0_DEFAULT               0x00000000UL                         /**< Mode DEFAULT for SYSRTC_GRP0_IEN            */
#define SYSRTC_GRP0_IEN_CAP0_DEFAULT                (_SYSRTC_GRP0_IEN_CAP0_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP0_IEN    */

/* Bit fields for SYSRTC GRP0_CTRL */
#define _SYSRTC_GRP0_CTRL_RESETVALUE                0x00000000UL                              /**< Default value for SYSRTC_GRP0_CTRL          */
#define _SYSRTC_GRP0_CTRL_MASK                      0x000007FFUL                              /**< Mask for SYSRTC_GRP0_CTRL                   */
#define SYSRTC_GRP0_CTRL_CMP0EN                     (0x1UL << 0)                              /**< Compare 0 Enable                            */
#define _SYSRTC_GRP0_CTRL_CMP0EN_SHIFT              0                                         /**< Shift value for SYSRTC_CMP0EN               */
#define _SYSRTC_GRP0_CTRL_CMP0EN_MASK               0x1UL                                     /**< Bit mask for SYSRTC_CMP0EN                  */
#define _SYSRTC_GRP0_CTRL_CMP0EN_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define SYSRTC_GRP0_CTRL_CMP0EN_DEFAULT             (_SYSRTC_GRP0_CTRL_CMP0EN_DEFAULT << 0)   /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CMP1EN                     (0x1UL << 1)                              /**< Compare 1 Enable                            */
#define _SYSRTC_GRP0_CTRL_CMP1EN_SHIFT              1                                         /**< Shift value for SYSRTC_CMP1EN               */
#define _SYSRTC_GRP0_CTRL_CMP1EN_MASK               0x2UL                                     /**< Bit mask for SYSRTC_CMP1EN                  */
#define _SYSRTC_GRP0_CTRL_CMP1EN_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define SYSRTC_GRP0_CTRL_CMP1EN_DEFAULT             (_SYSRTC_GRP0_CTRL_CMP1EN_DEFAULT << 1)   /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CAP0EN                     (0x1UL << 2)                              /**< Capture 0 Enable                            */
#define _SYSRTC_GRP0_CTRL_CAP0EN_SHIFT              2                                         /**< Shift value for SYSRTC_CAP0EN               */
#define _SYSRTC_GRP0_CTRL_CAP0EN_MASK               0x4UL                                     /**< Bit mask for SYSRTC_CAP0EN                  */
#define _SYSRTC_GRP0_CTRL_CAP0EN_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define SYSRTC_GRP0_CTRL_CAP0EN_DEFAULT             (_SYSRTC_GRP0_CTRL_CAP0EN_DEFAULT << 2)   /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_SHIFT            3                                         /**< Shift value for SYSRTC_CMP0CMOA             */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_MASK             0x38UL                                    /**< Bit mask for SYSRTC_CMP0CMOA                */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_DEFAULT          0x00000000UL                              /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_CLEAR            0x00000000UL                              /**< Mode CLEAR for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_SET              0x00000001UL                              /**< Mode SET for SYSRTC_GRP0_CTRL               */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_PULSE            0x00000002UL                              /**< Mode PULSE for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_TOGGLE           0x00000003UL                              /**< Mode TOGGLE for SYSRTC_GRP0_CTRL            */
#define _SYSRTC_GRP0_CTRL_CMP0CMOA_CMPIF            0x00000004UL                              /**< Mode CMPIF for SYSRTC_GRP0_CTRL             */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_DEFAULT           (_SYSRTC_GRP0_CTRL_CMP0CMOA_DEFAULT << 3) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_CLEAR             (_SYSRTC_GRP0_CTRL_CMP0CMOA_CLEAR << 3)   /**< Shifted mode CLEAR for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_SET               (_SYSRTC_GRP0_CTRL_CMP0CMOA_SET << 3)     /**< Shifted mode SET for SYSRTC_GRP0_CTRL       */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_PULSE             (_SYSRTC_GRP0_CTRL_CMP0CMOA_PULSE << 3)   /**< Shifted mode PULSE for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_TOGGLE            (_SYSRTC_GRP0_CTRL_CMP0CMOA_TOGGLE << 3)  /**< Shifted mode TOGGLE for SYSRTC_GRP0_CTRL    */
#define SYSRTC_GRP0_CTRL_CMP0CMOA_CMPIF             (_SYSRTC_GRP0_CTRL_CMP0CMOA_CMPIF << 3)   /**< Shifted mode CMPIF for SYSRTC_GRP0_CTRL     */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_SHIFT            6                                         /**< Shift value for SYSRTC_CMP1CMOA             */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_MASK             0x1C0UL                                   /**< Bit mask for SYSRTC_CMP1CMOA                */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_DEFAULT          0x00000000UL                              /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_CLEAR            0x00000000UL                              /**< Mode CLEAR for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_SET              0x00000001UL                              /**< Mode SET for SYSRTC_GRP0_CTRL               */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_PULSE            0x00000002UL                              /**< Mode PULSE for SYSRTC_GRP0_CTRL             */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_TOGGLE           0x00000003UL                              /**< Mode TOGGLE for SYSRTC_GRP0_CTRL            */
#define _SYSRTC_GRP0_CTRL_CMP1CMOA_CMPIF            0x00000004UL                              /**< Mode CMPIF for SYSRTC_GRP0_CTRL             */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_DEFAULT           (_SYSRTC_GRP0_CTRL_CMP1CMOA_DEFAULT << 6) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_CLEAR             (_SYSRTC_GRP0_CTRL_CMP1CMOA_CLEAR << 6)   /**< Shifted mode CLEAR for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_SET               (_SYSRTC_GRP0_CTRL_CMP1CMOA_SET << 6)     /**< Shifted mode SET for SYSRTC_GRP0_CTRL       */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_PULSE             (_SYSRTC_GRP0_CTRL_CMP1CMOA_PULSE << 6)   /**< Shifted mode PULSE for SYSRTC_GRP0_CTRL     */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_TOGGLE            (_SYSRTC_GRP0_CTRL_CMP1CMOA_TOGGLE << 6)  /**< Shifted mode TOGGLE for SYSRTC_GRP0_CTRL    */
#define SYSRTC_GRP0_CTRL_CMP1CMOA_CMPIF             (_SYSRTC_GRP0_CTRL_CMP1CMOA_CMPIF << 6)   /**< Shifted mode CMPIF for SYSRTC_GRP0_CTRL     */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_SHIFT            9                                         /**< Shift value for SYSRTC_CAP0EDGE             */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_MASK             0x600UL                                   /**< Bit mask for SYSRTC_CAP0EDGE                */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_DEFAULT          0x00000000UL                              /**< Mode DEFAULT for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_RISING           0x00000000UL                              /**< Mode RISING for SYSRTC_GRP0_CTRL            */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_FALLING          0x00000001UL                              /**< Mode FALLING for SYSRTC_GRP0_CTRL           */
#define _SYSRTC_GRP0_CTRL_CAP0EDGE_BOTH             0x00000002UL                              /**< Mode BOTH for SYSRTC_GRP0_CTRL              */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_DEFAULT           (_SYSRTC_GRP0_CTRL_CAP0EDGE_DEFAULT << 9) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_RISING            (_SYSRTC_GRP0_CTRL_CAP0EDGE_RISING << 9)  /**< Shifted mode RISING for SYSRTC_GRP0_CTRL    */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_FALLING           (_SYSRTC_GRP0_CTRL_CAP0EDGE_FALLING << 9) /**< Shifted mode FALLING for SYSRTC_GRP0_CTRL   */
#define SYSRTC_GRP0_CTRL_CAP0EDGE_BOTH              (_SYSRTC_GRP0_CTRL_CAP0EDGE_BOTH << 9)    /**< Shifted mode BOTH for SYSRTC_GRP0_CTRL      */

/* Bit fields for SYSRTC GRP0_CMP0VALUE */
#define _SYSRTC_GRP0_CMP0VALUE_RESETVALUE           0x00000000UL                                    /**< Default value for SYSRTC_GRP0_CMP0VALUE     */
#define _SYSRTC_GRP0_CMP0VALUE_MASK                 0xFFFFFFFFUL                                    /**< Mask for SYSRTC_GRP0_CMP0VALUE              */
#define _SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_SHIFT      0                                               /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_MASK       0xFFFFFFFFUL                                    /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_CMP0VALUE      */
#define SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_DEFAULT     (_SYSRTC_GRP0_CMP0VALUE_CMP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CMP0VALUE*/

/* Bit fields for SYSRTC GRP0_CMP1VALUE */
#define _SYSRTC_GRP0_CMP1VALUE_RESETVALUE           0x00000000UL                                    /**< Default value for SYSRTC_GRP0_CMP1VALUE     */
#define _SYSRTC_GRP0_CMP1VALUE_MASK                 0xFFFFFFFFUL                                    /**< Mask for SYSRTC_GRP0_CMP1VALUE              */
#define _SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_SHIFT      0                                               /**< Shift value for SYSRTC_CMP1VALUE            */
#define _SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_MASK       0xFFFFFFFFUL                                    /**< Bit mask for SYSRTC_CMP1VALUE               */
#define _SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_CMP1VALUE      */
#define SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_DEFAULT     (_SYSRTC_GRP0_CMP1VALUE_CMP1VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CMP1VALUE*/

/* Bit fields for SYSRTC GRP0_CAP0VALUE */
#define _SYSRTC_GRP0_CAP0VALUE_RESETVALUE           0x00000000UL                                    /**< Default value for SYSRTC_GRP0_CAP0VALUE     */
#define _SYSRTC_GRP0_CAP0VALUE_MASK                 0xFFFFFFFFUL                                    /**< Mask for SYSRTC_GRP0_CAP0VALUE              */
#define _SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_SHIFT      0                                               /**< Shift value for SYSRTC_CAP0VALUE            */
#define _SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_MASK       0xFFFFFFFFUL                                    /**< Bit mask for SYSRTC_CAP0VALUE               */
#define _SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_CAP0VALUE      */
#define SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_DEFAULT     (_SYSRTC_GRP0_CAP0VALUE_CAP0VALUE_DEFAULT << 0) /**< Shifted mode DEFAULT for SYSRTC_GRP0_CAP0VALUE*/

/* Bit fields for SYSRTC GRP0_SYNCBUSY */
#define _SYSRTC_GRP0_SYNCBUSY_RESETVALUE            0x00000000UL                                    /**< Default value for SYSRTC_GRP0_SYNCBUSY      */
#define _SYSRTC_GRP0_SYNCBUSY_MASK                  0x0000007FUL                                    /**< Mask for SYSRTC_GRP0_SYNCBUSY               */
#define SYSRTC_GRP0_SYNCBUSY_CTRL                   (0x1UL << 0)                                    /**< Sync busy for CTRL register                 */
#define _SYSRTC_GRP0_SYNCBUSY_CTRL_SHIFT            0                                               /**< Shift value for SYSRTC_CTRL                 */
#define _SYSRTC_GRP0_SYNCBUSY_CTRL_MASK             0x1UL                                           /**< Bit mask for SYSRTC_CTRL                    */
#define _SYSRTC_GRP0_SYNCBUSY_CTRL_DEFAULT          0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_CTRL_DEFAULT           (_SYSRTC_GRP0_SYNCBUSY_CTRL_DEFAULT << 0)       /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/
#define SYSRTC_GRP0_SYNCBUSY_CMP0VALUE              (0x1UL << 1)                                    /**< Sync busy for CMP0VALUE register            */
#define _SYSRTC_GRP0_SYNCBUSY_CMP0VALUE_SHIFT       1                                               /**< Shift value for SYSRTC_CMP0VALUE            */
#define _SYSRTC_GRP0_SYNCBUSY_CMP0VALUE_MASK        0x2UL                                           /**< Bit mask for SYSRTC_CMP0VALUE               */
#define _SYSRTC_GRP0_SYNCBUSY_CMP0VALUE_DEFAULT     0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_CMP0VALUE_DEFAULT      (_SYSRTC_GRP0_SYNCBUSY_CMP0VALUE_DEFAULT << 1)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/
#define SYSRTC_GRP0_SYNCBUSY_CMP1VALUE              (0x1UL << 2)                                    /**< Sync busy for CMP1VALUE register            */
#define _SYSRTC_GRP0_SYNCBUSY_CMP1VALUE_SHIFT       2                                               /**< Shift value for SYSRTC_CMP1VALUE            */
#define _SYSRTC_GRP0_SYNCBUSY_CMP1VALUE_MASK        0x4UL                                           /**< Bit mask for SYSRTC_CMP1VALUE               */
#define _SYSRTC_GRP0_SYNCBUSY_CMP1VALUE_DEFAULT     0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_CMP1VALUE_DEFAULT      (_SYSRTC_GRP0_SYNCBUSY_CMP1VALUE_DEFAULT << 2)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/
#define SYSRTC_GRP0_SYNCBUSY_HFXOSTART              (0x1UL << 3)                                    /**< Sync busy for HFXOSTART bit                 */
#define _SYSRTC_GRP0_SYNCBUSY_HFXOSTART_SHIFT       3                                               /**< Shift value for SYSRTC_HFXOSTART            */
#define _SYSRTC_GRP0_SYNCBUSY_HFXOSTART_MASK        0x8UL                                           /**< Bit mask for SYSRTC_HFXOSTART               */
#define _SYSRTC_GRP0_SYNCBUSY_HFXOSTART_DEFAULT     0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_HFXOSTART_DEFAULT      (_SYSRTC_GRP0_SYNCBUSY_HFXOSTART_DEFAULT << 3)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/
#define SYSRTC_GRP0_SYNCBUSY_EMUWAKEUP              (0x1UL << 4)                                    /**< Sync busy for EMUWAKEUP bit                 */
#define _SYSRTC_GRP0_SYNCBUSY_EMUWAKEUP_SHIFT       4                                               /**< Shift value for SYSRTC_EMUWAKEUP            */
#define _SYSRTC_GRP0_SYNCBUSY_EMUWAKEUP_MASK        0x10UL                                          /**< Bit mask for SYSRTC_EMUWAKEUP               */
#define _SYSRTC_GRP0_SYNCBUSY_EMUWAKEUP_DEFAULT     0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_EMUWAKEUP_DEFAULT      (_SYSRTC_GRP0_SYNCBUSY_EMUWAKEUP_DEFAULT << 4)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/
#define SYSRTC_GRP0_SYNCBUSY_HFXOACTIVE             (0x1UL << 5)                                    /**< Sync busy for HFXOACTIVE bit                */
#define _SYSRTC_GRP0_SYNCBUSY_HFXOACTIVE_SHIFT      5                                               /**< Shift value for SYSRTC_HFXOACTIVE           */
#define _SYSRTC_GRP0_SYNCBUSY_HFXOACTIVE_MASK       0x20UL                                          /**< Bit mask for SYSRTC_HFXOACTIVE              */
#define _SYSRTC_GRP0_SYNCBUSY_HFXOACTIVE_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_HFXOACTIVE_DEFAULT     (_SYSRTC_GRP0_SYNCBUSY_HFXOACTIVE_DEFAULT << 5) /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/
#define SYSRTC_GRP0_SYNCBUSY_EMUACTIVE              (0x1UL << 6)                                    /**< Sync busy for EMUACTIVE bit                 */
#define _SYSRTC_GRP0_SYNCBUSY_EMUACTIVE_SHIFT       6                                               /**< Shift value for SYSRTC_EMUACTIVE            */
#define _SYSRTC_GRP0_SYNCBUSY_EMUACTIVE_MASK        0x40UL                                          /**< Bit mask for SYSRTC_EMUACTIVE               */
#define _SYSRTC_GRP0_SYNCBUSY_EMUACTIVE_DEFAULT     0x00000000UL                                    /**< Mode DEFAULT for SYSRTC_GRP0_SYNCBUSY       */
#define SYSRTC_GRP0_SYNCBUSY_EMUACTIVE_DEFAULT      (_SYSRTC_GRP0_SYNCBUSY_EMUACTIVE_DEFAULT << 6)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_SYNCBUSY*/

/* Bit fields for SYSRTC GRP0_PRETRIG */
#define _SYSRTC_GRP0_PRETRIG_RESETVALUE             0x00000000UL                                   /**< Default value for SYSRTC_GRP0_PRETRIG       */
#define _SYSRTC_GRP0_PRETRIG_MASK                   0x000003FFUL                                   /**< Mask for SYSRTC_GRP0_PRETRIG                */
#define _SYSRTC_GRP0_PRETRIG_HFXOSTART_SHIFT        0                                              /**< Shift value for SYSRTC_HFXOSTART            */
#define _SYSRTC_GRP0_PRETRIG_HFXOSTART_MASK         0xFUL                                          /**< Bit mask for SYSRTC_HFXOSTART               */
#define _SYSRTC_GRP0_PRETRIG_HFXOSTART_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP0_PRETRIG        */
#define SYSRTC_GRP0_PRETRIG_HFXOSTART_DEFAULT       (_SYSRTC_GRP0_PRETRIG_HFXOSTART_DEFAULT << 0)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_PRETRIG*/
#define _SYSRTC_GRP0_PRETRIG_EMUWAKEUP_SHIFT        4                                              /**< Shift value for SYSRTC_EMUWAKEUP            */
#define _SYSRTC_GRP0_PRETRIG_EMUWAKEUP_MASK         0xF0UL                                         /**< Bit mask for SYSRTC_EMUWAKEUP               */
#define _SYSRTC_GRP0_PRETRIG_EMUWAKEUP_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP0_PRETRIG        */
#define SYSRTC_GRP0_PRETRIG_EMUWAKEUP_DEFAULT       (_SYSRTC_GRP0_PRETRIG_EMUWAKEUP_DEFAULT << 4)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_PRETRIG*/
#define SYSRTC_GRP0_PRETRIG_HFXOACTIVE              (0x1UL << 8)                                   /**< Status bit for HFXO pretrigger              */
#define _SYSRTC_GRP0_PRETRIG_HFXOACTIVE_SHIFT       8                                              /**< Shift value for SYSRTC_HFXOACTIVE           */
#define _SYSRTC_GRP0_PRETRIG_HFXOACTIVE_MASK        0x100UL                                        /**< Bit mask for SYSRTC_HFXOACTIVE              */
#define _SYSRTC_GRP0_PRETRIG_HFXOACTIVE_DEFAULT     0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP0_PRETRIG        */
#define SYSRTC_GRP0_PRETRIG_HFXOACTIVE_DEFAULT      (_SYSRTC_GRP0_PRETRIG_HFXOACTIVE_DEFAULT << 8) /**< Shifted mode DEFAULT for SYSRTC_GRP0_PRETRIG*/
#define SYSRTC_GRP0_PRETRIG_EMUACTIVE               (0x1UL << 9)                                   /**< Status bit for EMU pretrigger               */
#define _SYSRTC_GRP0_PRETRIG_EMUACTIVE_SHIFT        9                                              /**< Shift value for SYSRTC_EMUACTIVE            */
#define _SYSRTC_GRP0_PRETRIG_EMUACTIVE_MASK         0x200UL                                        /**< Bit mask for SYSRTC_EMUACTIVE               */
#define _SYSRTC_GRP0_PRETRIG_EMUACTIVE_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for SYSRTC_GRP0_PRETRIG        */
#define SYSRTC_GRP0_PRETRIG_EMUACTIVE_DEFAULT       (_SYSRTC_GRP0_PRETRIG_EMUACTIVE_DEFAULT << 9)  /**< Shifted mode DEFAULT for SYSRTC_GRP0_PRETRIG*/

/** @} End of group SIMG301_SYSRTC_BitFields */
/** @} End of group SIMG301_SYSRTC */
/** @} End of group Parts */

#endif // SIMG301_SYSRTC_H
