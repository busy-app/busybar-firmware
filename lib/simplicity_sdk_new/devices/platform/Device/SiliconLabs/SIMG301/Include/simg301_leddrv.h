/**************************************************************************//**
 * @file
 * @brief SIMG301 LEDDRV register and bit field definitions
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
#ifndef SIMG301_LEDDRV_H
#define SIMG301_LEDDRV_H
#define LEDDRV_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_LEDDRV LEDDRV
 * @{
 * @brief SIMG301 LEDDRV Register Declaration.
 *****************************************************************************/

/** LEDDRV Register Declaration. */
typedef struct leddrv_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP VERSION Register                                */
  __IOM uint32_t EN;                            /**< Enable Register                                    */
  __IOM uint32_t SWRST;                         /**< SWRST Register                                     */
  uint32_t       RESERVED0[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFG;                           /**< Configuration Register                             */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  __IOM uint32_t STATUS;                        /**< Status Register                                    */
  __IOM uint32_t ABUSCTRL0;                     /**< LEDDRV ABUS Control 0                              */
  __IOM uint32_t ABUSCTRL1;                     /**< LEDDRV ABUS Control 1                              */
  __IOM uint32_t IF;                            /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN;                           /**< Interrupt Enable Register                          */
  uint32_t       RESERVED1[52U];                /**< Reserved for future use                            */
  uint32_t       RESERVED2[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED3[959U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_SET;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_SET;                     /**< SWRST Register                                     */
  uint32_t       RESERVED4[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFG_SET;                       /**< Configuration Register                             */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  __IOM uint32_t STATUS_SET;                    /**< Status Register                                    */
  __IOM uint32_t ABUSCTRL0_SET;                 /**< LEDDRV ABUS Control 0                              */
  __IOM uint32_t ABUSCTRL1_SET;                 /**< LEDDRV ABUS Control 1                              */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED5[52U];                /**< Reserved for future use                            */
  uint32_t       RESERVED6[1U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED7[959U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_CLR;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_CLR;                     /**< SWRST Register                                     */
  uint32_t       RESERVED8[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t CFG_CLR;                       /**< Configuration Register                             */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  __IOM uint32_t STATUS_CLR;                    /**< Status Register                                    */
  __IOM uint32_t ABUSCTRL0_CLR;                 /**< LEDDRV ABUS Control 0                              */
  __IOM uint32_t ABUSCTRL1_CLR;                 /**< LEDDRV ABUS Control 1                              */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED9[52U];                /**< Reserved for future use                            */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
  uint32_t       RESERVED11[959U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_TGL;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_TGL;                     /**< SWRST Register                                     */
  uint32_t       RESERVED12[2U];                /**< Reserved for future use                            */
  __IOM uint32_t CFG_TGL;                       /**< Configuration Register                             */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  __IOM uint32_t STATUS_TGL;                    /**< Status Register                                    */
  __IOM uint32_t ABUSCTRL0_TGL;                 /**< LEDDRV ABUS Control 0                              */
  __IOM uint32_t ABUSCTRL1_TGL;                 /**< LEDDRV ABUS Control 1                              */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED13[52U];               /**< Reserved for future use                            */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
} LEDDRV_TypeDef;
/** @} End of group SIMG301_LEDDRV */

/**************************************************************************//**
 * @addtogroup SIMG301_LEDDRV
 * @{
 * @defgroup SIMG301_LEDDRV_BitFields LEDDRV Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for LEDDRV IPVERSION */
#define _LEDDRV_IPVERSION_RESETVALUE                0x00000001UL                               /**< Default value for LEDDRV_IPVERSION          */
#define _LEDDRV_IPVERSION_MASK                      0xFFFFFFFFUL                               /**< Mask for LEDDRV_IPVERSION                   */
#define _LEDDRV_IPVERSION_IPVERSION_SHIFT           0                                          /**< Shift value for LEDDRV_IPVERSION            */
#define _LEDDRV_IPVERSION_IPVERSION_MASK            0xFFFFFFFFUL                               /**< Bit mask for LEDDRV_IPVERSION               */
#define _LEDDRV_IPVERSION_IPVERSION_DEFAULT         0x00000001UL                               /**< Mode DEFAULT for LEDDRV_IPVERSION           */
#define LEDDRV_IPVERSION_IPVERSION_DEFAULT          (_LEDDRV_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for LEDDRV_IPVERSION   */

/* Bit fields for LEDDRV EN */
#define _LEDDRV_EN_RESETVALUE                       0x00000000UL                        /**< Default value for LEDDRV_EN                 */
#define _LEDDRV_EN_MASK                             0x00000003UL                        /**< Mask for LEDDRV_EN                          */
#define LEDDRV_EN_EN                                (0x1UL << 0)                        /**< Module enable                               */
#define _LEDDRV_EN_EN_SHIFT                         0                                   /**< Shift value for LEDDRV_EN                   */
#define _LEDDRV_EN_EN_MASK                          0x1UL                               /**< Bit mask for LEDDRV_EN                      */
#define _LEDDRV_EN_EN_DEFAULT                       0x00000000UL                        /**< Mode DEFAULT for LEDDRV_EN                  */
#define LEDDRV_EN_EN_DEFAULT                        (_LEDDRV_EN_EN_DEFAULT << 0)        /**< Shifted mode DEFAULT for LEDDRV_EN          */
#define LEDDRV_EN_DISABLING                         (0x1UL << 1)                        /**< Disablement busy status                     */
#define _LEDDRV_EN_DISABLING_SHIFT                  1                                   /**< Shift value for LEDDRV_DISABLING            */
#define _LEDDRV_EN_DISABLING_MASK                   0x2UL                               /**< Bit mask for LEDDRV_DISABLING               */
#define _LEDDRV_EN_DISABLING_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for LEDDRV_EN                  */
#define LEDDRV_EN_DISABLING_DEFAULT                 (_LEDDRV_EN_DISABLING_DEFAULT << 1) /**< Shifted mode DEFAULT for LEDDRV_EN          */

/* Bit fields for LEDDRV SWRST */
#define _LEDDRV_SWRST_RESETVALUE                    0x00000000UL                           /**< Default value for LEDDRV_SWRST              */
#define _LEDDRV_SWRST_MASK                          0x00000003UL                           /**< Mask for LEDDRV_SWRST                       */
#define LEDDRV_SWRST_SWRST                          (0x1UL << 0)                           /**< Software Reset bit                          */
#define _LEDDRV_SWRST_SWRST_SHIFT                   0                                      /**< Shift value for LEDDRV_SWRST                */
#define _LEDDRV_SWRST_SWRST_MASK                    0x1UL                                  /**< Bit mask for LEDDRV_SWRST                   */
#define _LEDDRV_SWRST_SWRST_DEFAULT                 0x00000000UL                           /**< Mode DEFAULT for LEDDRV_SWRST               */
#define LEDDRV_SWRST_SWRST_DEFAULT                  (_LEDDRV_SWRST_SWRST_DEFAULT << 0)     /**< Shifted mode DEFAULT for LEDDRV_SWRST       */
#define LEDDRV_SWRST_RESETTING                      (0x1UL << 1)                           /**< Reset busy status                           */
#define _LEDDRV_SWRST_RESETTING_SHIFT               1                                      /**< Shift value for LEDDRV_RESETTING            */
#define _LEDDRV_SWRST_RESETTING_MASK                0x2UL                                  /**< Bit mask for LEDDRV_RESETTING               */
#define _LEDDRV_SWRST_RESETTING_DEFAULT             0x00000000UL                           /**< Mode DEFAULT for LEDDRV_SWRST               */
#define LEDDRV_SWRST_RESETTING_DEFAULT              (_LEDDRV_SWRST_RESETTING_DEFAULT << 1) /**< Shifted mode DEFAULT for LEDDRV_SWRST       */

/* Bit fields for LEDDRV CFG */
#define _LEDDRV_CFG_RESETVALUE                      0x0000077FUL                            /**< Default value for LEDDRV_CFG                */
#define _LEDDRV_CFG_MASK                            0x0000FFFFUL                            /**< Mask for LEDDRV_CFG                         */
#define _LEDDRV_CFG_TONMAXVAL_SHIFT                 0                                       /**< Shift value for LEDDRV_TONMAXVAL            */
#define _LEDDRV_CFG_TONMAXVAL_MASK                  0x7UL                                   /**< Bit mask for LEDDRV_TONMAXVAL               */
#define _LEDDRV_CFG_TONMAXVAL_DEFAULT               0x00000007UL                            /**< Mode DEFAULT for LEDDRV_CFG                 */
#define LEDDRV_CFG_TONMAXVAL_DEFAULT                (_LEDDRV_CFG_TONMAXVAL_DEFAULT << 0)    /**< Shifted mode DEFAULT for LEDDRV_CFG         */
#define _LEDDRV_CFG_TOFFMAXVAL_SHIFT                3                                       /**< Shift value for LEDDRV_TOFFMAXVAL           */
#define _LEDDRV_CFG_TOFFMAXVAL_MASK                 0x38UL                                  /**< Bit mask for LEDDRV_TOFFMAXVAL              */
#define _LEDDRV_CFG_TOFFMAXVAL_DEFAULT              0x00000007UL                            /**< Mode DEFAULT for LEDDRV_CFG                 */
#define LEDDRV_CFG_TOFFMAXVAL_DEFAULT               (_LEDDRV_CFG_TOFFMAXVAL_DEFAULT << 3)   /**< Shifted mode DEFAULT for LEDDRV_CFG         */
#define _LEDDRV_CFG_IPKDETREFADJ_SHIFT              6                                       /**< Shift value for LEDDRV_IPKDETREFADJ         */
#define _LEDDRV_CFG_IPKDETREFADJ_MASK               0xC0UL                                  /**< Bit mask for LEDDRV_IPKDETREFADJ            */
#define _LEDDRV_CFG_IPKDETREFADJ_DEFAULT            0x00000001UL                            /**< Mode DEFAULT for LEDDRV_CFG                 */
#define LEDDRV_CFG_IPKDETREFADJ_DEFAULT             (_LEDDRV_CFG_IPKDETREFADJ_DEFAULT << 6) /**< Shifted mode DEFAULT for LEDDRV_CFG         */
#define _LEDDRV_CFG_IOCREFADJ_SHIFT                 8                                       /**< Shift value for LEDDRV_IOCREFADJ            */
#define _LEDDRV_CFG_IOCREFADJ_MASK                  0x300UL                                 /**< Bit mask for LEDDRV_IOCREFADJ               */
#define _LEDDRV_CFG_IOCREFADJ_DEFAULT               0x00000003UL                            /**< Mode DEFAULT for LEDDRV_CFG                 */
#define LEDDRV_CFG_IOCREFADJ_DEFAULT                (_LEDDRV_CFG_IOCREFADJ_DEFAULT << 8)    /**< Shifted mode DEFAULT for LEDDRV_CFG         */
#define _LEDDRV_CFG_TONCALMODE_SHIFT                10                                      /**< Shift value for LEDDRV_TONCALMODE           */
#define _LEDDRV_CFG_TONCALMODE_MASK                 0xC00UL                                 /**< Bit mask for LEDDRV_TONCALMODE              */
#define _LEDDRV_CFG_TONCALMODE_DEFAULT              0x00000001UL                            /**< Mode DEFAULT for LEDDRV_CFG                 */
#define _LEDDRV_CFG_TONCALMODE_ACTIVE               0x00000000UL                            /**< Mode ACTIVE for LEDDRV_CFG                  */
#define _LEDDRV_CFG_TONCALMODE_BOTH                 0x00000001UL                            /**< Mode BOTH for LEDDRV_CFG                    */
#define _LEDDRV_CFG_TONCALMODE_FORCECAL             0x00000002UL                            /**< Mode FORCECAL for LEDDRV_CFG                */
#define _LEDDRV_CFG_TONCALMODE_FIXED                0x00000003UL                            /**< Mode FIXED for LEDDRV_CFG                   */
#define LEDDRV_CFG_TONCALMODE_DEFAULT               (_LEDDRV_CFG_TONCALMODE_DEFAULT << 10)  /**< Shifted mode DEFAULT for LEDDRV_CFG         */
#define LEDDRV_CFG_TONCALMODE_ACTIVE                (_LEDDRV_CFG_TONCALMODE_ACTIVE << 10)   /**< Shifted mode ACTIVE for LEDDRV_CFG          */
#define LEDDRV_CFG_TONCALMODE_BOTH                  (_LEDDRV_CFG_TONCALMODE_BOTH << 10)     /**< Shifted mode BOTH for LEDDRV_CFG            */
#define LEDDRV_CFG_TONCALMODE_FORCECAL              (_LEDDRV_CFG_TONCALMODE_FORCECAL << 10) /**< Shifted mode FORCECAL for LEDDRV_CFG        */
#define LEDDRV_CFG_TONCALMODE_FIXED                 (_LEDDRV_CFG_TONCALMODE_FIXED << 10)    /**< Shifted mode FIXED for LEDDRV_CFG           */
#define _LEDDRV_CFG_FIXEDTONSEL_SHIFT               12                                      /**< Shift value for LEDDRV_FIXEDTONSEL          */
#define _LEDDRV_CFG_FIXEDTONSEL_MASK                0xF000UL                                /**< Bit mask for LEDDRV_FIXEDTONSEL             */
#define _LEDDRV_CFG_FIXEDTONSEL_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for LEDDRV_CFG                 */
#define LEDDRV_CFG_FIXEDTONSEL_DEFAULT              (_LEDDRV_CFG_FIXEDTONSEL_DEFAULT << 12) /**< Shifted mode DEFAULT for LEDDRV_CFG         */

/* Bit fields for LEDDRV CTRL */
#define _LEDDRV_CTRL_RESETVALUE                     0x00000000UL                              /**< Default value for LEDDRV_CTRL               */
#define _LEDDRV_CTRL_MASK                           0x00001C1FUL                              /**< Mask for LEDDRV_CTRL                        */
#define LEDDRV_CTRL_ENADUALMODE                     (0x1UL << 0)                              /**< Dual driver mode enable                     */
#define _LEDDRV_CTRL_ENADUALMODE_SHIFT              0                                         /**< Shift value for LEDDRV_ENADUALMODE          */
#define _LEDDRV_CTRL_ENADUALMODE_MASK               0x1UL                                     /**< Bit mask for LEDDRV_ENADUALMODE             */
#define _LEDDRV_CTRL_ENADUALMODE_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_ENADUALMODE_DEFAULT             (_LEDDRV_CTRL_ENADUALMODE_DEFAULT << 0)   /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_IPKFOLDBACK                     (0x1UL << 1)                              /**< Peak current foldback enable                */
#define _LEDDRV_CTRL_IPKFOLDBACK_SHIFT              1                                         /**< Shift value for LEDDRV_IPKFOLDBACK          */
#define _LEDDRV_CTRL_IPKFOLDBACK_MASK               0x2UL                                     /**< Bit mask for LEDDRV_IPKFOLDBACK             */
#define _LEDDRV_CTRL_IPKFOLDBACK_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_IPKFOLDBACK_DEFAULT             (_LEDDRV_CTRL_IPKFOLDBACK_DEFAULT << 1)   /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_ENDIRDRV                        (0x1UL << 2)                              /**< Direct Drive Enable                         */
#define _LEDDRV_CTRL_ENDIRDRV_SHIFT                 2                                         /**< Shift value for LEDDRV_ENDIRDRV             */
#define _LEDDRV_CTRL_ENDIRDRV_MASK                  0x4UL                                     /**< Bit mask for LEDDRV_ENDIRDRV                */
#define _LEDDRV_CTRL_ENDIRDRV_DEFAULT               0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_ENDIRDRV_DEFAULT                (_LEDDRV_CTRL_ENDIRDRV_DEFAULT << 2)      /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_TONFRCCAL                       (0x1UL << 3)                              /**< Ton Force calibration                       */
#define _LEDDRV_CTRL_TONFRCCAL_SHIFT                3                                         /**< Shift value for LEDDRV_TONFRCCAL            */
#define _LEDDRV_CTRL_TONFRCCAL_MASK                 0x8UL                                     /**< Bit mask for LEDDRV_TONFRCCAL               */
#define _LEDDRV_CTRL_TONFRCCAL_DEFAULT              0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_TONFRCCAL_DEFAULT               (_LEDDRV_CTRL_TONFRCCAL_DEFAULT << 3)     /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_ENFRC                           (0x1UL << 4)                              /**< Enable Force calibration                    */
#define _LEDDRV_CTRL_ENFRC_SHIFT                    4                                         /**< Shift value for LEDDRV_ENFRC                */
#define _LEDDRV_CTRL_ENFRC_MASK                     0x10UL                                    /**< Bit mask for LEDDRV_ENFRC                   */
#define _LEDDRV_CTRL_ENFRC_DEFAULT                  0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_ENFRC_DEFAULT                   (_LEDDRV_CTRL_ENFRC_DEFAULT << 4)         /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_HWOCFLAGDIS                     (0x1UL << 10)                             /**< Disable Hardware Over current handling      */
#define _LEDDRV_CTRL_HWOCFLAGDIS_SHIFT              10                                        /**< Shift value for LEDDRV_HWOCFLAGDIS          */
#define _LEDDRV_CTRL_HWOCFLAGDIS_MASK               0x400UL                                   /**< Bit mask for LEDDRV_HWOCFLAGDIS             */
#define _LEDDRV_CTRL_HWOCFLAGDIS_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_HWOCFLAGDIS_DEFAULT             (_LEDDRV_CTRL_HWOCFLAGDIS_DEFAULT << 10)  /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_SWCLROCFLAG0                    (0x1UL << 11)                             /**< Software Clear OCFLAG channel 0             */
#define _LEDDRV_CTRL_SWCLROCFLAG0_SHIFT             11                                        /**< Shift value for LEDDRV_SWCLROCFLAG0         */
#define _LEDDRV_CTRL_SWCLROCFLAG0_MASK              0x800UL                                   /**< Bit mask for LEDDRV_SWCLROCFLAG0            */
#define _LEDDRV_CTRL_SWCLROCFLAG0_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_SWCLROCFLAG0_DEFAULT            (_LEDDRV_CTRL_SWCLROCFLAG0_DEFAULT << 11) /**< Shifted mode DEFAULT for LEDDRV_CTRL        */
#define LEDDRV_CTRL_SWCLROCFLAG1                    (0x1UL << 12)                             /**< Software Clear OCFLAG channel 1             */
#define _LEDDRV_CTRL_SWCLROCFLAG1_SHIFT             12                                        /**< Shift value for LEDDRV_SWCLROCFLAG1         */
#define _LEDDRV_CTRL_SWCLROCFLAG1_MASK              0x1000UL                                  /**< Bit mask for LEDDRV_SWCLROCFLAG1            */
#define _LEDDRV_CTRL_SWCLROCFLAG1_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for LEDDRV_CTRL                */
#define LEDDRV_CTRL_SWCLROCFLAG1_DEFAULT            (_LEDDRV_CTRL_SWCLROCFLAG1_DEFAULT << 12) /**< Shifted mode DEFAULT for LEDDRV_CTRL        */

/* Bit fields for LEDDRV STATUS */
#define _LEDDRV_STATUS_RESETVALUE                   0x00000000UL                                     /**< Default value for LEDDRV_STATUS             */
#define _LEDDRV_STATUS_MASK                         0x00007FFFUL                                     /**< Mask for LEDDRV_STATUS                      */
#define _LEDDRV_STATUS_ACDETCNT_SHIFT               0                                                /**< Shift value for LEDDRV_ACDETCNT             */
#define _LEDDRV_STATUS_ACDETCNT_MASK                0x1FUL                                           /**< Bit mask for LEDDRV_ACDETCNT                */
#define _LEDDRV_STATUS_ACDETCNT_DEFAULT             0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_ACDETCNT_DEFAULT              (_LEDDRV_STATUS_ACDETCNT_DEFAULT << 0)           /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define _LEDDRV_STATUS_VALDETCNT_SHIFT              5                                                /**< Shift value for LEDDRV_VALDETCNT            */
#define _LEDDRV_STATUS_VALDETCNT_MASK               0x1E0UL                                          /**< Bit mask for LEDDRV_VALDETCNT               */
#define _LEDDRV_STATUS_VALDETCNT_DEFAULT            0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_VALDETCNT_DEFAULT             (_LEDDRV_STATUS_VALDETCNT_DEFAULT << 5)          /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define LEDDRV_STATUS_OCFLAG0                       (0x1UL << 9)                                     /**< Over Current Channel 0                      */
#define _LEDDRV_STATUS_OCFLAG0_SHIFT                9                                                /**< Shift value for LEDDRV_OCFLAG0              */
#define _LEDDRV_STATUS_OCFLAG0_MASK                 0x200UL                                          /**< Bit mask for LEDDRV_OCFLAG0                 */
#define _LEDDRV_STATUS_OCFLAG0_DEFAULT              0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_OCFLAG0_DEFAULT               (_LEDDRV_STATUS_OCFLAG0_DEFAULT << 9)            /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define LEDDRV_STATUS_OCFLAG1                       (0x1UL << 10)                                    /**< Over Current Channel 1                      */
#define _LEDDRV_STATUS_OCFLAG1_SHIFT                10                                               /**< Shift value for LEDDRV_OCFLAG1              */
#define _LEDDRV_STATUS_OCFLAG1_MASK                 0x400UL                                          /**< Bit mask for LEDDRV_OCFLAG1                 */
#define _LEDDRV_STATUS_OCFLAG1_DEFAULT              0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_OCFLAG1_DEFAULT               (_LEDDRV_STATUS_OCFLAG1_DEFAULT << 10)           /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define LEDDRV_STATUS_ABUSALLOCERR                  (0x1UL << 11)                                    /**< ABUS Allocation Error Status                */
#define _LEDDRV_STATUS_ABUSALLOCERR_SHIFT           11                                               /**< Shift value for LEDDRV_ABUSALLOCERR         */
#define _LEDDRV_STATUS_ABUSALLOCERR_MASK            0x800UL                                          /**< Bit mask for LEDDRV_ABUSALLOCERR            */
#define _LEDDRV_STATUS_ABUSALLOCERR_DEFAULT         0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_ABUSALLOCERR_DEFAULT          (_LEDDRV_STATUS_ABUSALLOCERR_DEFAULT << 11)      /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define LEDDRV_STATUS_ABUSINPUTCONFLICT             (0x1UL << 12)                                    /**< ABUS Input Conflict Status                  */
#define _LEDDRV_STATUS_ABUSINPUTCONFLICT_SHIFT      12                                               /**< Shift value for LEDDRV_ABUSINPUTCONFLICT    */
#define _LEDDRV_STATUS_ABUSINPUTCONFLICT_MASK       0x1000UL                                         /**< Bit mask for LEDDRV_ABUSINPUTCONFLICT       */
#define _LEDDRV_STATUS_ABUSINPUTCONFLICT_DEFAULT    0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_ABUSINPUTCONFLICT_DEFAULT     (_LEDDRV_STATUS_ABUSINPUTCONFLICT_DEFAULT << 12) /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define LEDDRV_STATUS_SWCLROCFLAGSTS0               (0x1UL << 13)                                    /**< Software OCFLAG0 Clear Pending              */
#define _LEDDRV_STATUS_SWCLROCFLAGSTS0_SHIFT        13                                               /**< Shift value for LEDDRV_SWCLROCFLAGSTS0      */
#define _LEDDRV_STATUS_SWCLROCFLAGSTS0_MASK         0x2000UL                                         /**< Bit mask for LEDDRV_SWCLROCFLAGSTS0         */
#define _LEDDRV_STATUS_SWCLROCFLAGSTS0_DEFAULT      0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_SWCLROCFLAGSTS0_DEFAULT       (_LEDDRV_STATUS_SWCLROCFLAGSTS0_DEFAULT << 13)   /**< Shifted mode DEFAULT for LEDDRV_STATUS      */
#define LEDDRV_STATUS_SWCLROCFLAGSTS1               (0x1UL << 14)                                    /**< Software OCFLAG1 Clear Pending              */
#define _LEDDRV_STATUS_SWCLROCFLAGSTS1_SHIFT        14                                               /**< Shift value for LEDDRV_SWCLROCFLAGSTS1      */
#define _LEDDRV_STATUS_SWCLROCFLAGSTS1_MASK         0x4000UL                                         /**< Bit mask for LEDDRV_SWCLROCFLAGSTS1         */
#define _LEDDRV_STATUS_SWCLROCFLAGSTS1_DEFAULT      0x00000000UL                                     /**< Mode DEFAULT for LEDDRV_STATUS              */
#define LEDDRV_STATUS_SWCLROCFLAGSTS1_DEFAULT       (_LEDDRV_STATUS_SWCLROCFLAGSTS1_DEFAULT << 14)   /**< Shifted mode DEFAULT for LEDDRV_STATUS      */

/* Bit fields for LEDDRV ABUSCTRL0 */
#define _LEDDRV_ABUSCTRL0_RESETVALUE                0x00000000UL                                /**< Default value for LEDDRV_ABUSCTRL0          */
#define _LEDDRV_ABUSCTRL0_MASK                      0x000000F3UL                                /**< Mask for LEDDRV_ABUSCTRL0                   */
#define _LEDDRV_ABUSCTRL0_VACPORTSEL_SHIFT          0                                           /**< Shift value for LEDDRV_VACPORTSEL           */
#define _LEDDRV_ABUSCTRL0_VACPORTSEL_MASK           0x3UL                                       /**< Bit mask for LEDDRV_VACPORTSEL              */
#define _LEDDRV_ABUSCTRL0_VACPORTSEL_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LEDDRV_ABUSCTRL0           */
#define _LEDDRV_ABUSCTRL0_VACPORTSEL_NONE           0x00000000UL                                /**< Mode NONE for LEDDRV_ABUSCTRL0              */
#define _LEDDRV_ABUSCTRL0_VACPORTSEL_PORTC          0x00000001UL                                /**< Mode PORTC for LEDDRV_ABUSCTRL0             */
#define _LEDDRV_ABUSCTRL0_VACPORTSEL_PORTD          0x00000002UL                                /**< Mode PORTD for LEDDRV_ABUSCTRL0             */
#define LEDDRV_ABUSCTRL0_VACPORTSEL_DEFAULT         (_LEDDRV_ABUSCTRL0_VACPORTSEL_DEFAULT << 0) /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL0   */
#define LEDDRV_ABUSCTRL0_VACPORTSEL_NONE            (_LEDDRV_ABUSCTRL0_VACPORTSEL_NONE << 0)    /**< Shifted mode NONE for LEDDRV_ABUSCTRL0      */
#define LEDDRV_ABUSCTRL0_VACPORTSEL_PORTC           (_LEDDRV_ABUSCTRL0_VACPORTSEL_PORTC << 0)   /**< Shifted mode PORTC for LEDDRV_ABUSCTRL0     */
#define LEDDRV_ABUSCTRL0_VACPORTSEL_PORTD           (_LEDDRV_ABUSCTRL0_VACPORTSEL_PORTD << 0)   /**< Shifted mode PORTD for LEDDRV_ABUSCTRL0     */
#define _LEDDRV_ABUSCTRL0_VACPINSEL_SHIFT           4                                           /**< Shift value for LEDDRV_VACPINSEL            */
#define _LEDDRV_ABUSCTRL0_VACPINSEL_MASK            0xF0UL                                      /**< Bit mask for LEDDRV_VACPINSEL               */
#define _LEDDRV_ABUSCTRL0_VACPINSEL_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for LEDDRV_ABUSCTRL0           */
#define LEDDRV_ABUSCTRL0_VACPINSEL_DEFAULT          (_LEDDRV_ABUSCTRL0_VACPINSEL_DEFAULT << 4)  /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL0   */

/* Bit fields for LEDDRV ABUSCTRL1 */
#define _LEDDRV_ABUSCTRL1_RESETVALUE                0x00000000UL                                   /**< Default value for LEDDRV_ABUSCTRL1          */
#define _LEDDRV_ABUSCTRL1_MASK                      0xF3F3F3F3UL                                   /**< Mask for LEDDRV_ABUSCTRL1                   */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_SHIFT        0                                              /**< Shift value for LEDDRV_ISNSPORTSEL0         */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_MASK         0x3UL                                          /**< Bit mask for LEDDRV_ISNSPORTSEL0            */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_NONE         0x00000000UL                                   /**< Mode NONE for LEDDRV_ABUSCTRL1              */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_PORTC        0x00000001UL                                   /**< Mode PORTC for LEDDRV_ABUSCTRL1             */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL0_PORTD        0x00000002UL                                   /**< Mode PORTD for LEDDRV_ABUSCTRL1             */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL0_DEFAULT       (_LEDDRV_ABUSCTRL1_ISNSPORTSEL0_DEFAULT << 0)  /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL0_NONE          (_LEDDRV_ABUSCTRL1_ISNSPORTSEL0_NONE << 0)     /**< Shifted mode NONE for LEDDRV_ABUSCTRL1      */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL0_PORTC         (_LEDDRV_ABUSCTRL1_ISNSPORTSEL0_PORTC << 0)    /**< Shifted mode PORTC for LEDDRV_ABUSCTRL1     */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL0_PORTD         (_LEDDRV_ABUSCTRL1_ISNSPORTSEL0_PORTD << 0)    /**< Shifted mode PORTD for LEDDRV_ABUSCTRL1     */
#define _LEDDRV_ABUSCTRL1_ISNSPINSEL0_SHIFT         4                                              /**< Shift value for LEDDRV_ISNSPINSEL0          */
#define _LEDDRV_ABUSCTRL1_ISNSPINSEL0_MASK          0xF0UL                                         /**< Bit mask for LEDDRV_ISNSPINSEL0             */
#define _LEDDRV_ABUSCTRL1_ISNSPINSEL0_DEFAULT       0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define LEDDRV_ABUSCTRL1_ISNSPINSEL0_DEFAULT        (_LEDDRV_ABUSCTRL1_ISNSPINSEL0_DEFAULT << 4)   /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_SHIFT        8                                              /**< Shift value for LEDDRV_DSNSPORTSEL0         */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_MASK         0x300UL                                        /**< Bit mask for LEDDRV_DSNSPORTSEL0            */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_NONE         0x00000000UL                                   /**< Mode NONE for LEDDRV_ABUSCTRL1              */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_PORTC        0x00000001UL                                   /**< Mode PORTC for LEDDRV_ABUSCTRL1             */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL0_PORTD        0x00000002UL                                   /**< Mode PORTD for LEDDRV_ABUSCTRL1             */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL0_DEFAULT       (_LEDDRV_ABUSCTRL1_DSNSPORTSEL0_DEFAULT << 8)  /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL0_NONE          (_LEDDRV_ABUSCTRL1_DSNSPORTSEL0_NONE << 8)     /**< Shifted mode NONE for LEDDRV_ABUSCTRL1      */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL0_PORTC         (_LEDDRV_ABUSCTRL1_DSNSPORTSEL0_PORTC << 8)    /**< Shifted mode PORTC for LEDDRV_ABUSCTRL1     */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL0_PORTD         (_LEDDRV_ABUSCTRL1_DSNSPORTSEL0_PORTD << 8)    /**< Shifted mode PORTD for LEDDRV_ABUSCTRL1     */
#define _LEDDRV_ABUSCTRL1_DSNSPINSEL0_SHIFT         12                                             /**< Shift value for LEDDRV_DSNSPINSEL0          */
#define _LEDDRV_ABUSCTRL1_DSNSPINSEL0_MASK          0xF000UL                                       /**< Bit mask for LEDDRV_DSNSPINSEL0             */
#define _LEDDRV_ABUSCTRL1_DSNSPINSEL0_DEFAULT       0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define LEDDRV_ABUSCTRL1_DSNSPINSEL0_DEFAULT        (_LEDDRV_ABUSCTRL1_DSNSPINSEL0_DEFAULT << 12)  /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_SHIFT        16                                             /**< Shift value for LEDDRV_ISNSPORTSEL1         */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_MASK         0x30000UL                                      /**< Bit mask for LEDDRV_ISNSPORTSEL1            */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_NONE         0x00000000UL                                   /**< Mode NONE for LEDDRV_ABUSCTRL1              */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_PORTC        0x00000001UL                                   /**< Mode PORTC for LEDDRV_ABUSCTRL1             */
#define _LEDDRV_ABUSCTRL1_ISNSPORTSEL1_PORTD        0x00000002UL                                   /**< Mode PORTD for LEDDRV_ABUSCTRL1             */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL1_DEFAULT       (_LEDDRV_ABUSCTRL1_ISNSPORTSEL1_DEFAULT << 16) /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL1_NONE          (_LEDDRV_ABUSCTRL1_ISNSPORTSEL1_NONE << 16)    /**< Shifted mode NONE for LEDDRV_ABUSCTRL1      */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL1_PORTC         (_LEDDRV_ABUSCTRL1_ISNSPORTSEL1_PORTC << 16)   /**< Shifted mode PORTC for LEDDRV_ABUSCTRL1     */
#define LEDDRV_ABUSCTRL1_ISNSPORTSEL1_PORTD         (_LEDDRV_ABUSCTRL1_ISNSPORTSEL1_PORTD << 16)   /**< Shifted mode PORTD for LEDDRV_ABUSCTRL1     */
#define _LEDDRV_ABUSCTRL1_ISNSPINSEL1_SHIFT         20                                             /**< Shift value for LEDDRV_ISNSPINSEL1          */
#define _LEDDRV_ABUSCTRL1_ISNSPINSEL1_MASK          0xF00000UL                                     /**< Bit mask for LEDDRV_ISNSPINSEL1             */
#define _LEDDRV_ABUSCTRL1_ISNSPINSEL1_DEFAULT       0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define LEDDRV_ABUSCTRL1_ISNSPINSEL1_DEFAULT        (_LEDDRV_ABUSCTRL1_ISNSPINSEL1_DEFAULT << 20)  /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_SHIFT        24                                             /**< Shift value for LEDDRV_DSNSPORTSEL1         */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_MASK         0x3000000UL                                    /**< Bit mask for LEDDRV_DSNSPORTSEL1            */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_DEFAULT      0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_NONE         0x00000000UL                                   /**< Mode NONE for LEDDRV_ABUSCTRL1              */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_PORTC        0x00000001UL                                   /**< Mode PORTC for LEDDRV_ABUSCTRL1             */
#define _LEDDRV_ABUSCTRL1_DSNSPORTSEL1_PORTD        0x00000002UL                                   /**< Mode PORTD for LEDDRV_ABUSCTRL1             */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL1_DEFAULT       (_LEDDRV_ABUSCTRL1_DSNSPORTSEL1_DEFAULT << 24) /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL1_NONE          (_LEDDRV_ABUSCTRL1_DSNSPORTSEL1_NONE << 24)    /**< Shifted mode NONE for LEDDRV_ABUSCTRL1      */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL1_PORTC         (_LEDDRV_ABUSCTRL1_DSNSPORTSEL1_PORTC << 24)   /**< Shifted mode PORTC for LEDDRV_ABUSCTRL1     */
#define LEDDRV_ABUSCTRL1_DSNSPORTSEL1_PORTD         (_LEDDRV_ABUSCTRL1_DSNSPORTSEL1_PORTD << 24)   /**< Shifted mode PORTD for LEDDRV_ABUSCTRL1     */
#define _LEDDRV_ABUSCTRL1_DSNSPINSEL1_SHIFT         28                                             /**< Shift value for LEDDRV_DSNSPINSEL1          */
#define _LEDDRV_ABUSCTRL1_DSNSPINSEL1_MASK          0xF0000000UL                                   /**< Bit mask for LEDDRV_DSNSPINSEL1             */
#define _LEDDRV_ABUSCTRL1_DSNSPINSEL1_DEFAULT       0x00000000UL                                   /**< Mode DEFAULT for LEDDRV_ABUSCTRL1           */
#define LEDDRV_ABUSCTRL1_DSNSPINSEL1_DEFAULT        (_LEDDRV_ABUSCTRL1_DSNSPINSEL1_DEFAULT << 28)  /**< Shifted mode DEFAULT for LEDDRV_ABUSCTRL1   */

/* Bit fields for LEDDRV IF */
#define _LEDDRV_IF_RESETVALUE                       0x00000000UL                                /**< Default value for LEDDRV_IF                 */
#define _LEDDRV_IF_MASK                             0x00000007UL                                /**< Mask for LEDDRV_IF                          */
#define LEDDRV_IF_ABUSALLOCERR                      (0x1UL << 0)                                /**< ABUS Port Allocation Error Flag             */
#define _LEDDRV_IF_ABUSALLOCERR_SHIFT               0                                           /**< Shift value for LEDDRV_ABUSALLOCERR         */
#define _LEDDRV_IF_ABUSALLOCERR_MASK                0x1UL                                       /**< Bit mask for LEDDRV_ABUSALLOCERR            */
#define _LEDDRV_IF_ABUSALLOCERR_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for LEDDRV_IF                  */
#define LEDDRV_IF_ABUSALLOCERR_DEFAULT              (_LEDDRV_IF_ABUSALLOCERR_DEFAULT << 0)      /**< Shifted mode DEFAULT for LEDDRV_IF          */
#define LEDDRV_IF_ABUSINPUTCONFLICT                 (0x1UL << 1)                                /**< ABUS Input Conflict Error Flag              */
#define _LEDDRV_IF_ABUSINPUTCONFLICT_SHIFT          1                                           /**< Shift value for LEDDRV_ABUSINPUTCONFLICT    */
#define _LEDDRV_IF_ABUSINPUTCONFLICT_MASK           0x2UL                                       /**< Bit mask for LEDDRV_ABUSINPUTCONFLICT       */
#define _LEDDRV_IF_ABUSINPUTCONFLICT_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for LEDDRV_IF                  */
#define LEDDRV_IF_ABUSINPUTCONFLICT_DEFAULT         (_LEDDRV_IF_ABUSINPUTCONFLICT_DEFAULT << 1) /**< Shifted mode DEFAULT for LEDDRV_IF          */
#define LEDDRV_IF_OCFLAG                            (0x1UL << 2)                                /**< OCFLAG Interrupt Flag                       */
#define _LEDDRV_IF_OCFLAG_SHIFT                     2                                           /**< Shift value for LEDDRV_OCFLAG               */
#define _LEDDRV_IF_OCFLAG_MASK                      0x4UL                                       /**< Bit mask for LEDDRV_OCFLAG                  */
#define _LEDDRV_IF_OCFLAG_DEFAULT                   0x00000000UL                                /**< Mode DEFAULT for LEDDRV_IF                  */
#define LEDDRV_IF_OCFLAG_DEFAULT                    (_LEDDRV_IF_OCFLAG_DEFAULT << 2)            /**< Shifted mode DEFAULT for LEDDRV_IF          */

/* Bit fields for LEDDRV IEN */
#define _LEDDRV_IEN_RESETVALUE                      0x00000000UL                                 /**< Default value for LEDDRV_IEN                */
#define _LEDDRV_IEN_MASK                            0x00000007UL                                 /**< Mask for LEDDRV_IEN                         */
#define LEDDRV_IEN_ABUSALLOCERR                     (0x1UL << 0)                                 /**< ABUS Allocation Error Interrupt Enable      */
#define _LEDDRV_IEN_ABUSALLOCERR_SHIFT              0                                            /**< Shift value for LEDDRV_ABUSALLOCERR         */
#define _LEDDRV_IEN_ABUSALLOCERR_MASK               0x1UL                                        /**< Bit mask for LEDDRV_ABUSALLOCERR            */
#define _LEDDRV_IEN_ABUSALLOCERR_DEFAULT            0x00000000UL                                 /**< Mode DEFAULT for LEDDRV_IEN                 */
#define LEDDRV_IEN_ABUSALLOCERR_DEFAULT             (_LEDDRV_IEN_ABUSALLOCERR_DEFAULT << 0)      /**< Shifted mode DEFAULT for LEDDRV_IEN         */
#define LEDDRV_IEN_ABUSINPUTCONFLICT                (0x1UL << 1)                                 /**< ABUS Input Conflict Interrupt Enable        */
#define _LEDDRV_IEN_ABUSINPUTCONFLICT_SHIFT         1                                            /**< Shift value for LEDDRV_ABUSINPUTCONFLICT    */
#define _LEDDRV_IEN_ABUSINPUTCONFLICT_MASK          0x2UL                                        /**< Bit mask for LEDDRV_ABUSINPUTCONFLICT       */
#define _LEDDRV_IEN_ABUSINPUTCONFLICT_DEFAULT       0x00000000UL                                 /**< Mode DEFAULT for LEDDRV_IEN                 */
#define LEDDRV_IEN_ABUSINPUTCONFLICT_DEFAULT        (_LEDDRV_IEN_ABUSINPUTCONFLICT_DEFAULT << 1) /**< Shifted mode DEFAULT for LEDDRV_IEN         */
#define LEDDRV_IEN_OCFLAG                           (0x1UL << 2)                                 /**< OCFLAG Interrupt Enable                     */
#define _LEDDRV_IEN_OCFLAG_SHIFT                    2                                            /**< Shift value for LEDDRV_OCFLAG               */
#define _LEDDRV_IEN_OCFLAG_MASK                     0x4UL                                        /**< Bit mask for LEDDRV_OCFLAG                  */
#define _LEDDRV_IEN_OCFLAG_DEFAULT                  0x00000000UL                                 /**< Mode DEFAULT for LEDDRV_IEN                 */
#define LEDDRV_IEN_OCFLAG_DEFAULT                   (_LEDDRV_IEN_OCFLAG_DEFAULT << 2)            /**< Shifted mode DEFAULT for LEDDRV_IEN         */

/** @} End of group SIMG301_LEDDRV_BitFields */
/** @} End of group SIMG301_LEDDRV */
/** @} End of group Parts */

#endif // SIMG301_LEDDRV_H
