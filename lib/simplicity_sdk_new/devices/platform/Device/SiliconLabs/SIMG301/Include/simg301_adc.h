/**************************************************************************//**
 * @file
 * @brief SIMG301 ADC register and bit field definitions
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
#ifndef SIMG301_ADC_H
#define SIMG301_ADC_H
#define ADC_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_ADC ADC
 * @{
 * @brief SIMG301 ADC Register Declaration.
 *****************************************************************************/

/** ADC CFG Register Group Declaration. */
typedef struct adc_cfg_typedef{
  __IOM uint32_t CFG;                                /**< Configuration                                      */
  __IOM uint32_t OFFSETSE;                           /**< Offset trimming for SE mode                        */
  __IOM uint32_t OFFSETDIFF;                         /**< Offset trimming for diff mode                      */
  __IOM uint32_t GAINCAL;                            /**< gain calibration                                   */
} ADC_CFG_TypeDef;

/** ADC SCANTABLE Register Group Declaration. */
typedef struct adc_scantable_typedef{
  __IOM uint32_t SCAN;                               /**< SCAN ENTRY                                         */
} ADC_SCANTABLE_TypeDef;

/** ADC Register Declaration. */
typedef struct adc_typedef{
  __IM uint32_t         IPVERSION;              /**< IPVERSION                                          */
  __IOM uint32_t        EN;                     /**< Enable                                             */
  __IOM uint32_t        CTRL;                   /**< Control                                            */
  __IOM uint32_t        CMD;                    /**< Command                                            */
  __IOM uint32_t        TIMER;                  /**< Timer control                                      */
  __IM uint32_t         STATUS;                 /**< Status                                             */
  __IOM uint32_t        MASKREQ;                /**< Select channels to be converted                    */
  __IM uint32_t         STMASK;                 /**< Scan Table Mask                                    */
  __IOM uint32_t        CMPTHR;                 /**< Digital Window comparator Threshold                */
  __IOM uint32_t        IF;                     /**< Interrupt Flags                                    */
  __IOM uint32_t        IEN;                    /**< Interrupt Enable                                   */
  __IOM uint32_t        TRIGGER;                /**< Trigger                                            */
  uint32_t              RESERVED0[1U];          /**< Reserved for future use                            */
  uint32_t              RESERVED1[7U];          /**< Reserved for future use                            */
  ADC_CFG_TypeDef       CFG[2U];                /**< CFG                                                */
  __IOM uint32_t        SCANFIFOCFG;            /**< Scan FIFO Configuration                            */
  __IM uint32_t         SCANFIFODATA;           /**< Scan FIFO Read Data                                */
  __IM uint32_t         SCANFIFOSTAT;           /**< Scan FIFO Status                                   */
  __IM uint32_t         SCANDATA;               /**< Scan Data                                          */
  ADC_SCANTABLE_TypeDef SCANTABLE[16U];         /**< SCANTABLE                                          */
  uint32_t              RESERVED2[4U];          /**< Reserved for future use                            */
  uint32_t              RESERVED3[1U];          /**< Reserved for future use                            */
  uint32_t              RESERVED4[971U];        /**< Reserved for future use                            */
  __IM uint32_t         IPVERSION_SET;          /**< IPVERSION                                          */
  __IOM uint32_t        EN_SET;                 /**< Enable                                             */
  __IOM uint32_t        CTRL_SET;               /**< Control                                            */
  __IOM uint32_t        CMD_SET;                /**< Command                                            */
  __IOM uint32_t        TIMER_SET;              /**< Timer control                                      */
  __IM uint32_t         STATUS_SET;             /**< Status                                             */
  __IOM uint32_t        MASKREQ_SET;            /**< Select channels to be converted                    */
  __IM uint32_t         STMASK_SET;             /**< Scan Table Mask                                    */
  __IOM uint32_t        CMPTHR_SET;             /**< Digital Window comparator Threshold                */
  __IOM uint32_t        IF_SET;                 /**< Interrupt Flags                                    */
  __IOM uint32_t        IEN_SET;                /**< Interrupt Enable                                   */
  __IOM uint32_t        TRIGGER_SET;            /**< Trigger                                            */
  uint32_t              RESERVED5[1U];          /**< Reserved for future use                            */
  uint32_t              RESERVED6[7U];          /**< Reserved for future use                            */
  ADC_CFG_TypeDef       CFG_SET[2U];            /**< CFG                                                */
  __IOM uint32_t        SCANFIFOCFG_SET;        /**< Scan FIFO Configuration                            */
  __IM uint32_t         SCANFIFODATA_SET;       /**< Scan FIFO Read Data                                */
  __IM uint32_t         SCANFIFOSTAT_SET;       /**< Scan FIFO Status                                   */
  __IM uint32_t         SCANDATA_SET;           /**< Scan Data                                          */
  ADC_SCANTABLE_TypeDef SCANTABLE_SET[16U];     /**< SCANTABLE                                          */
  uint32_t              RESERVED7[4U];          /**< Reserved for future use                            */
  uint32_t              RESERVED8[1U];          /**< Reserved for future use                            */
  uint32_t              RESERVED9[971U];        /**< Reserved for future use                            */
  __IM uint32_t         IPVERSION_CLR;          /**< IPVERSION                                          */
  __IOM uint32_t        EN_CLR;                 /**< Enable                                             */
  __IOM uint32_t        CTRL_CLR;               /**< Control                                            */
  __IOM uint32_t        CMD_CLR;                /**< Command                                            */
  __IOM uint32_t        TIMER_CLR;              /**< Timer control                                      */
  __IM uint32_t         STATUS_CLR;             /**< Status                                             */
  __IOM uint32_t        MASKREQ_CLR;            /**< Select channels to be converted                    */
  __IM uint32_t         STMASK_CLR;             /**< Scan Table Mask                                    */
  __IOM uint32_t        CMPTHR_CLR;             /**< Digital Window comparator Threshold                */
  __IOM uint32_t        IF_CLR;                 /**< Interrupt Flags                                    */
  __IOM uint32_t        IEN_CLR;                /**< Interrupt Enable                                   */
  __IOM uint32_t        TRIGGER_CLR;            /**< Trigger                                            */
  uint32_t              RESERVED10[1U];         /**< Reserved for future use                            */
  uint32_t              RESERVED11[7U];         /**< Reserved for future use                            */
  ADC_CFG_TypeDef       CFG_CLR[2U];            /**< CFG                                                */
  __IOM uint32_t        SCANFIFOCFG_CLR;        /**< Scan FIFO Configuration                            */
  __IM uint32_t         SCANFIFODATA_CLR;       /**< Scan FIFO Read Data                                */
  __IM uint32_t         SCANFIFOSTAT_CLR;       /**< Scan FIFO Status                                   */
  __IM uint32_t         SCANDATA_CLR;           /**< Scan Data                                          */
  ADC_SCANTABLE_TypeDef SCANTABLE_CLR[16U];     /**< SCANTABLE                                          */
  uint32_t              RESERVED12[4U];         /**< Reserved for future use                            */
  uint32_t              RESERVED13[1U];         /**< Reserved for future use                            */
  uint32_t              RESERVED14[971U];       /**< Reserved for future use                            */
  __IM uint32_t         IPVERSION_TGL;          /**< IPVERSION                                          */
  __IOM uint32_t        EN_TGL;                 /**< Enable                                             */
  __IOM uint32_t        CTRL_TGL;               /**< Control                                            */
  __IOM uint32_t        CMD_TGL;                /**< Command                                            */
  __IOM uint32_t        TIMER_TGL;              /**< Timer control                                      */
  __IM uint32_t         STATUS_TGL;             /**< Status                                             */
  __IOM uint32_t        MASKREQ_TGL;            /**< Select channels to be converted                    */
  __IM uint32_t         STMASK_TGL;             /**< Scan Table Mask                                    */
  __IOM uint32_t        CMPTHR_TGL;             /**< Digital Window comparator Threshold                */
  __IOM uint32_t        IF_TGL;                 /**< Interrupt Flags                                    */
  __IOM uint32_t        IEN_TGL;                /**< Interrupt Enable                                   */
  __IOM uint32_t        TRIGGER_TGL;            /**< Trigger                                            */
  uint32_t              RESERVED15[1U];         /**< Reserved for future use                            */
  uint32_t              RESERVED16[7U];         /**< Reserved for future use                            */
  ADC_CFG_TypeDef       CFG_TGL[2U];            /**< CFG                                                */
  __IOM uint32_t        SCANFIFOCFG_TGL;        /**< Scan FIFO Configuration                            */
  __IM uint32_t         SCANFIFODATA_TGL;       /**< Scan FIFO Read Data                                */
  __IM uint32_t         SCANFIFOSTAT_TGL;       /**< Scan FIFO Status                                   */
  __IM uint32_t         SCANDATA_TGL;           /**< Scan Data                                          */
  ADC_SCANTABLE_TypeDef SCANTABLE_TGL[16U];     /**< SCANTABLE                                          */
  uint32_t              RESERVED17[4U];         /**< Reserved for future use                            */
  uint32_t              RESERVED18[1U];         /**< Reserved for future use                            */
} ADC_TypeDef;
/** @} End of group SIMG301_ADC */

/**************************************************************************//**
 * @addtogroup SIMG301_ADC
 * @{
 * @defgroup SIMG301_ADC_BitFields ADC Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for ADC IPVERSION */
#define _ADC_IPVERSION_RESETVALUE                  0x00000001UL                            /**< Default value for ADC_IPVERSION             */
#define _ADC_IPVERSION_MASK                        0xFFFFFFFFUL                            /**< Mask for ADC_IPVERSION                      */
#define _ADC_IPVERSION_IPVERSION_SHIFT             0                                       /**< Shift value for ADC_IPVERSION               */
#define _ADC_IPVERSION_IPVERSION_MASK              0xFFFFFFFFUL                            /**< Bit mask for ADC_IPVERSION                  */
#define _ADC_IPVERSION_IPVERSION_DEFAULT           0x00000001UL                            /**< Mode DEFAULT for ADC_IPVERSION              */
#define ADC_IPVERSION_IPVERSION_DEFAULT            (_ADC_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for ADC_IPVERSION      */

/* Bit fields for ADC EN */
#define _ADC_EN_RESETVALUE                         0x00000000UL                         /**< Default value for ADC_EN                    */
#define _ADC_EN_MASK                               0x00000003UL                         /**< Mask for ADC_EN                             */
#define ADC_EN_EN                                  (0x1UL << 0)                         /**< Enable SAR Module                           */
#define _ADC_EN_EN_SHIFT                           0                                    /**< Shift value for ADC_EN                      */
#define _ADC_EN_EN_MASK                            0x1UL                                /**< Bit mask for ADC_EN                         */
#define _ADC_EN_EN_DEFAULT                         0x00000000UL                         /**< Mode DEFAULT for ADC_EN                     */
#define _ADC_EN_EN_DISABLE                         0x00000000UL                         /**< Mode DISABLE for ADC_EN                     */
#define _ADC_EN_EN_ENABLE                          0x00000001UL                         /**< Mode ENABLE for ADC_EN                      */
#define ADC_EN_EN_DEFAULT                          (_ADC_EN_EN_DEFAULT << 0)            /**< Shifted mode DEFAULT for ADC_EN             */
#define ADC_EN_EN_DISABLE                          (_ADC_EN_EN_DISABLE << 0)            /**< Shifted mode DISABLE for ADC_EN             */
#define ADC_EN_EN_ENABLE                           (_ADC_EN_EN_ENABLE << 0)             /**< Shifted mode ENABLE for ADC_EN              */
#define ADC_EN_DISABLING                           (0x1UL << 1)                         /**< Disablement busy status                     */
#define _ADC_EN_DISABLING_SHIFT                    1                                    /**< Shift value for ADC_DISABLING               */
#define _ADC_EN_DISABLING_MASK                     0x2UL                                /**< Bit mask for ADC_DISABLING                  */
#define _ADC_EN_DISABLING_DEFAULT                  0x00000000UL                         /**< Mode DEFAULT for ADC_EN                     */
#define ADC_EN_DISABLING_DEFAULT                   (_ADC_EN_DISABLING_DEFAULT << 1)     /**< Shifted mode DEFAULT for ADC_EN             */

/* Bit fields for ADC CTRL */
#define _ADC_CTRL_RESETVALUE                       0x020F8000UL                            /**< Default value for ADC_CTRL                  */
#define _ADC_CTRL_MASK                             0xFFFF87FFUL                            /**< Mask for ADC_CTRL                           */
#define ADC_CTRL_DBGHALT                           (0x1UL << 2)                            /**< Debug Halt                                  */
#define _ADC_CTRL_DBGHALT_SHIFT                    2                                       /**< Shift value for ADC_DBGHALT                 */
#define _ADC_CTRL_DBGHALT_MASK                     0x4UL                                   /**< Bit mask for ADC_DBGHALT                    */
#define _ADC_CTRL_DBGHALT_DEFAULT                  0x00000000UL                            /**< Mode DEFAULT for ADC_CTRL                   */
#define _ADC_CTRL_DBGHALT_NORMAL                   0x00000000UL                            /**< Mode NORMAL for ADC_CTRL                    */
#define _ADC_CTRL_DBGHALT_HALT                     0x00000001UL                            /**< Mode HALT for ADC_CTRL                      */
#define ADC_CTRL_DBGHALT_DEFAULT                   (_ADC_CTRL_DBGHALT_DEFAULT << 2)        /**< Shifted mode DEFAULT for ADC_CTRL           */
#define ADC_CTRL_DBGHALT_NORMAL                    (_ADC_CTRL_DBGHALT_NORMAL << 2)         /**< Shifted mode NORMAL for ADC_CTRL            */
#define ADC_CTRL_DBGHALT_HALT                      (_ADC_CTRL_DBGHALT_HALT << 2)           /**< Shifted mode HALT for ADC_CTRL              */
#define _ADC_CTRL_WARMUPMODE_SHIFT                 3                                       /**< Shift value for ADC_WARMUPMODE              */
#define _ADC_CTRL_WARMUPMODE_MASK                  0x18UL                                  /**< Bit mask for ADC_WARMUPMODE                 */
#define _ADC_CTRL_WARMUPMODE_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for ADC_CTRL                   */
#define _ADC_CTRL_WARMUPMODE_NORMAL                0x00000000UL                            /**< Mode NORMAL for ADC_CTRL                    */
#define _ADC_CTRL_WARMUPMODE_KEEPWARM              0x00000001UL                            /**< Mode KEEPWARM for ADC_CTRL                  */
#define _ADC_CTRL_WARMUPMODE_KEEPSTANDBY           0x00000002UL                            /**< Mode KEEPSTANDBY for ADC_CTRL               */
#define ADC_CTRL_WARMUPMODE_DEFAULT                (_ADC_CTRL_WARMUPMODE_DEFAULT << 3)     /**< Shifted mode DEFAULT for ADC_CTRL           */
#define ADC_CTRL_WARMUPMODE_NORMAL                 (_ADC_CTRL_WARMUPMODE_NORMAL << 3)      /**< Shifted mode NORMAL for ADC_CTRL            */
#define ADC_CTRL_WARMUPMODE_KEEPWARM               (_ADC_CTRL_WARMUPMODE_KEEPWARM << 3)    /**< Shifted mode KEEPWARM for ADC_CTRL          */
#define ADC_CTRL_WARMUPMODE_KEEPSTANDBY            (_ADC_CTRL_WARMUPMODE_KEEPSTANDBY << 3) /**< Shifted mode KEEPSTANDBY for ADC_CTRL       */
#define _ADC_CTRL_ADCPRESCALE_SHIFT                5                                       /**< Shift value for ADC_ADCPRESCALE             */
#define _ADC_CTRL_ADCPRESCALE_MASK                 0x7E0UL                                 /**< Bit mask for ADC_ADCPRESCALE                */
#define _ADC_CTRL_ADCPRESCALE_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for ADC_CTRL                   */
#define ADC_CTRL_ADCPRESCALE_DEFAULT               (_ADC_CTRL_ADCPRESCALE_DEFAULT << 5)    /**< Shifted mode DEFAULT for ADC_CTRL           */
#define _ADC_CTRL_TIMEBASE_SHIFT                   15                                      /**< Shift value for ADC_TIMEBASE                */
#define _ADC_CTRL_TIMEBASE_MASK                    0x3F8000UL                              /**< Bit mask for ADC_TIMEBASE                   */
#define _ADC_CTRL_TIMEBASE_DEFAULT                 0x0000001FUL                            /**< Mode DEFAULT for ADC_CTRL                   */
#define ADC_CTRL_TIMEBASE_DEFAULT                  (_ADC_CTRL_TIMEBASE_DEFAULT << 15)      /**< Shifted mode DEFAULT for ADC_CTRL           */
#define _ADC_CTRL_REFSEL_SHIFT                     22                                      /**< Shift value for ADC_REFSEL                  */
#define _ADC_CTRL_REFSEL_MASK                      0x1C00000UL                             /**< Bit mask for ADC_REFSEL                     */
#define _ADC_CTRL_REFSEL_DEFAULT                   0x00000000UL                            /**< Mode DEFAULT for ADC_CTRL                   */
#define _ADC_CTRL_REFSEL_VREFINT                   0x00000000UL                            /**< Mode VREFINT for ADC_CTRL                   */
#define _ADC_CTRL_REFSEL_VREFPL                    0x00000001UL                            /**< Mode VREFPL for ADC_CTRL                    */
#define _ADC_CTRL_REFSEL_VREFPH                    0x00000002UL                            /**< Mode VREFPH for ADC_CTRL                    */
#define _ADC_CTRL_REFSEL_VDDA                      0x00000003UL                            /**< Mode VDDA for ADC_CTRL                      */
#define _ADC_CTRL_REFSEL_VREFPBUF                  0x00000004UL                            /**< Mode VREFPBUF for ADC_CTRL                  */
#define ADC_CTRL_REFSEL_DEFAULT                    (_ADC_CTRL_REFSEL_DEFAULT << 22)        /**< Shifted mode DEFAULT for ADC_CTRL           */
#define ADC_CTRL_REFSEL_VREFINT                    (_ADC_CTRL_REFSEL_VREFINT << 22)        /**< Shifted mode VREFINT for ADC_CTRL           */
#define ADC_CTRL_REFSEL_VREFPL                     (_ADC_CTRL_REFSEL_VREFPL << 22)         /**< Shifted mode VREFPL for ADC_CTRL            */
#define ADC_CTRL_REFSEL_VREFPH                     (_ADC_CTRL_REFSEL_VREFPH << 22)         /**< Shifted mode VREFPH for ADC_CTRL            */
#define ADC_CTRL_REFSEL_VDDA                       (_ADC_CTRL_REFSEL_VDDA << 22)           /**< Shifted mode VDDA for ADC_CTRL              */
#define ADC_CTRL_REFSEL_VREFPBUF                   (_ADC_CTRL_REFSEL_VREFPBUF << 22)       /**< Shifted mode VREFPBUF for ADC_CTRL          */
#define _ADC_CTRL_HSCLKRATE_SHIFT                  28                                      /**< Shift value for ADC_HSCLKRATE               */
#define _ADC_CTRL_HSCLKRATE_MASK                   0x70000000UL                            /**< Bit mask for ADC_HSCLKRATE                  */
#define _ADC_CTRL_HSCLKRATE_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for ADC_CTRL                   */
#define _ADC_CTRL_HSCLKRATE_DIV1                   0x00000000UL                            /**< Mode DIV1 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV2                   0x00000001UL                            /**< Mode DIV2 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV3                   0x00000002UL                            /**< Mode DIV3 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV4                   0x00000003UL                            /**< Mode DIV4 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV5                   0x00000004UL                            /**< Mode DIV5 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV6                   0x00000005UL                            /**< Mode DIV6 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV7                   0x00000006UL                            /**< Mode DIV7 for ADC_CTRL                      */
#define _ADC_CTRL_HSCLKRATE_DIV8                   0x00000007UL                            /**< Mode DIV8 for ADC_CTRL                      */
#define ADC_CTRL_HSCLKRATE_DEFAULT                 (_ADC_CTRL_HSCLKRATE_DEFAULT << 28)     /**< Shifted mode DEFAULT for ADC_CTRL           */
#define ADC_CTRL_HSCLKRATE_DIV1                    (_ADC_CTRL_HSCLKRATE_DIV1 << 28)        /**< Shifted mode DIV1 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV2                    (_ADC_CTRL_HSCLKRATE_DIV2 << 28)        /**< Shifted mode DIV2 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV3                    (_ADC_CTRL_HSCLKRATE_DIV3 << 28)        /**< Shifted mode DIV3 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV4                    (_ADC_CTRL_HSCLKRATE_DIV4 << 28)        /**< Shifted mode DIV4 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV5                    (_ADC_CTRL_HSCLKRATE_DIV5 << 28)        /**< Shifted mode DIV5 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV6                    (_ADC_CTRL_HSCLKRATE_DIV6 << 28)        /**< Shifted mode DIV6 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV7                    (_ADC_CTRL_HSCLKRATE_DIV7 << 28)        /**< Shifted mode DIV7 for ADC_CTRL              */
#define ADC_CTRL_HSCLKRATE_DIV8                    (_ADC_CTRL_HSCLKRATE_DIV8 << 28)        /**< Shifted mode DIV8 for ADC_CTRL              */

/* Bit fields for ADC CMD */
#define _ADC_CMD_RESETVALUE                        0x00000000UL                           /**< Default value for ADC_CMD                   */
#define _ADC_CMD_MASK                              0x02030018UL                           /**< Mask for ADC_CMD                            */
#define ADC_CMD_SCANSTART                          (0x1UL << 3)                           /**< Scan Queue Start                            */
#define _ADC_CMD_SCANSTART_SHIFT                   3                                      /**< Shift value for ADC_SCANSTART               */
#define _ADC_CMD_SCANSTART_MASK                    0x8UL                                  /**< Bit mask for ADC_SCANSTART                  */
#define _ADC_CMD_SCANSTART_DEFAULT                 0x00000000UL                           /**< Mode DEFAULT for ADC_CMD                    */
#define ADC_CMD_SCANSTART_DEFAULT                  (_ADC_CMD_SCANSTART_DEFAULT << 3)      /**< Shifted mode DEFAULT for ADC_CMD            */
#define ADC_CMD_SCANSTOP                           (0x1UL << 4)                           /**< Scan Queue Stop                             */
#define _ADC_CMD_SCANSTOP_SHIFT                    4                                      /**< Shift value for ADC_SCANSTOP                */
#define _ADC_CMD_SCANSTOP_MASK                     0x10UL                                 /**< Bit mask for ADC_SCANSTOP                   */
#define _ADC_CMD_SCANSTOP_DEFAULT                  0x00000000UL                           /**< Mode DEFAULT for ADC_CMD                    */
#define ADC_CMD_SCANSTOP_DEFAULT                   (_ADC_CMD_SCANSTOP_DEFAULT << 4)       /**< Shifted mode DEFAULT for ADC_CMD            */
#define ADC_CMD_TIMEREN                            (0x1UL << 16)                          /**< Timer Enable                                */
#define _ADC_CMD_TIMEREN_SHIFT                     16                                     /**< Shift value for ADC_TIMEREN                 */
#define _ADC_CMD_TIMEREN_MASK                      0x10000UL                              /**< Bit mask for ADC_TIMEREN                    */
#define _ADC_CMD_TIMEREN_DEFAULT                   0x00000000UL                           /**< Mode DEFAULT for ADC_CMD                    */
#define ADC_CMD_TIMEREN_DEFAULT                    (_ADC_CMD_TIMEREN_DEFAULT << 16)       /**< Shifted mode DEFAULT for ADC_CMD            */
#define ADC_CMD_TIMERDIS                           (0x1UL << 17)                          /**< Timer Disable                               */
#define _ADC_CMD_TIMERDIS_SHIFT                    17                                     /**< Shift value for ADC_TIMERDIS                */
#define _ADC_CMD_TIMERDIS_MASK                     0x20000UL                              /**< Bit mask for ADC_TIMERDIS                   */
#define _ADC_CMD_TIMERDIS_DEFAULT                  0x00000000UL                           /**< Mode DEFAULT for ADC_CMD                    */
#define ADC_CMD_TIMERDIS_DEFAULT                   (_ADC_CMD_TIMERDIS_DEFAULT << 17)      /**< Shifted mode DEFAULT for ADC_CMD            */
#define ADC_CMD_SCANFIFOFLUSH                      (0x1UL << 25)                          /**< Flush Scan FIFO                             */
#define _ADC_CMD_SCANFIFOFLUSH_SHIFT               25                                     /**< Shift value for ADC_SCANFIFOFLUSH           */
#define _ADC_CMD_SCANFIFOFLUSH_MASK                0x2000000UL                            /**< Bit mask for ADC_SCANFIFOFLUSH              */
#define _ADC_CMD_SCANFIFOFLUSH_DEFAULT             0x00000000UL                           /**< Mode DEFAULT for ADC_CMD                    */
#define ADC_CMD_SCANFIFOFLUSH_DEFAULT              (_ADC_CMD_SCANFIFOFLUSH_DEFAULT << 25) /**< Shifted mode DEFAULT for ADC_CMD            */

/* Bit fields for ADC TIMER */
#define _ADC_TIMER_RESETVALUE                      0x00000000UL                         /**< Default value for ADC_TIMER                 */
#define _ADC_TIMER_MASK                            0x0000FFFFUL                         /**< Mask for ADC_TIMER                          */
#define _ADC_TIMER_TIMER_SHIFT                     0                                    /**< Shift value for ADC_TIMER                   */
#define _ADC_TIMER_TIMER_MASK                      0xFFFFUL                             /**< Bit mask for ADC_TIMER                      */
#define _ADC_TIMER_TIMER_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for ADC_TIMER                  */
#define ADC_TIMER_TIMER_DEFAULT                    (_ADC_TIMER_TIMER_DEFAULT << 0)      /**< Shifted mode DEFAULT for ADC_TIMER          */

/* Bit fields for ADC STATUS */
#define _ADC_STATUS_RESETVALUE                     0x00000000UL                                    /**< Default value for ADC_STATUS                */
#define _ADC_STATUS_MASK                           0x41218658UL                                    /**< Mask for ADC_STATUS                         */
#define ADC_STATUS_SCANQEN                         (0x1UL << 3)                                    /**< Scan Queued Enabled                         */
#define _ADC_STATUS_SCANQEN_SHIFT                  3                                               /**< Shift value for ADC_SCANQEN                 */
#define _ADC_STATUS_SCANQEN_MASK                   0x8UL                                           /**< Bit mask for ADC_SCANQEN                    */
#define _ADC_STATUS_SCANQEN_DEFAULT                0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_SCANQEN_DEFAULT                 (_ADC_STATUS_SCANQEN_DEFAULT << 3)              /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_SCANQUEUEPENDING                (0x1UL << 4)                                    /**< Scan Queue Pending                          */
#define _ADC_STATUS_SCANQUEUEPENDING_SHIFT         4                                               /**< Shift value for ADC_SCANQUEUEPENDING        */
#define _ADC_STATUS_SCANQUEUEPENDING_MASK          0x10UL                                          /**< Bit mask for ADC_SCANQUEUEPENDING           */
#define _ADC_STATUS_SCANQUEUEPENDING_DEFAULT       0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_SCANQUEUEPENDING_DEFAULT        (_ADC_STATUS_SCANQUEUEPENDING_DEFAULT << 4)     /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_CONVERTING                      (0x1UL << 6)                                    /**< Ongoing Conversion                          */
#define _ADC_STATUS_CONVERTING_SHIFT               6                                               /**< Shift value for ADC_CONVERTING              */
#define _ADC_STATUS_CONVERTING_MASK                0x40UL                                          /**< Bit mask for ADC_CONVERTING                 */
#define _ADC_STATUS_CONVERTING_DEFAULT             0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_CONVERTING_DEFAULT              (_ADC_STATUS_CONVERTING_DEFAULT << 6)           /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_SCANFIFODV                      (0x1UL << 9)                                    /**< Scan FIFO Data Valid                        */
#define _ADC_STATUS_SCANFIFODV_SHIFT               9                                               /**< Shift value for ADC_SCANFIFODV              */
#define _ADC_STATUS_SCANFIFODV_MASK                0x200UL                                         /**< Bit mask for ADC_SCANFIFODV                 */
#define _ADC_STATUS_SCANFIFODV_DEFAULT             0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_SCANFIFODV_DEFAULT              (_ADC_STATUS_SCANFIFODV_DEFAULT << 9)           /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_SCANDATADV                      (0x1UL << 10)                                   /**< SCANDATA Data Valid                         */
#define _ADC_STATUS_SCANDATADV_SHIFT               10                                              /**< Shift value for ADC_SCANDATADV              */
#define _ADC_STATUS_SCANDATADV_MASK                0x400UL                                         /**< Bit mask for ADC_SCANDATADV                 */
#define _ADC_STATUS_SCANDATADV_DEFAULT             0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_SCANDATADV_DEFAULT              (_ADC_STATUS_SCANDATADV_DEFAULT << 10)          /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_SCANFIFOFLUSHING                (0x1UL << 15)                                   /**< Scan FIFO Flush in Progress                 */
#define _ADC_STATUS_SCANFIFOFLUSHING_SHIFT         15                                              /**< Shift value for ADC_SCANFIFOFLUSHING        */
#define _ADC_STATUS_SCANFIFOFLUSHING_MASK          0x8000UL                                        /**< Bit mask for ADC_SCANFIFOFLUSHING           */
#define _ADC_STATUS_SCANFIFOFLUSHING_DEFAULT       0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_SCANFIFOFLUSHING_DEFAULT        (_ADC_STATUS_SCANFIFOFLUSHING_DEFAULT << 15)    /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_TIMERACTIVE                     (0x1UL << 16)                                   /**< Timer Active                                */
#define _ADC_STATUS_TIMERACTIVE_SHIFT              16                                              /**< Shift value for ADC_TIMERACTIVE             */
#define _ADC_STATUS_TIMERACTIVE_MASK               0x10000UL                                       /**< Bit mask for ADC_TIMERACTIVE                */
#define _ADC_STATUS_TIMERACTIVE_DEFAULT            0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_TIMERACTIVE_DEFAULT             (_ADC_STATUS_TIMERACTIVE_DEFAULT << 16)         /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_MASKREQWRITEPENDING             (0x1UL << 21)                                   /**< MASKREQ Write pending                       */
#define _ADC_STATUS_MASKREQWRITEPENDING_SHIFT      21                                              /**< Shift value for ADC_MASKREQWRITEPENDING     */
#define _ADC_STATUS_MASKREQWRITEPENDING_MASK       0x200000UL                                      /**< Bit mask for ADC_MASKREQWRITEPENDING        */
#define _ADC_STATUS_MASKREQWRITEPENDING_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_MASKREQWRITEPENDING_DEFAULT     (_ADC_STATUS_MASKREQWRITEPENDING_DEFAULT << 21) /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_SYNCBUSY                        (0x1UL << 24)                                   /**< Synchronization in Progress                 */
#define _ADC_STATUS_SYNCBUSY_SHIFT                 24                                              /**< Shift value for ADC_SYNCBUSY                */
#define _ADC_STATUS_SYNCBUSY_MASK                  0x1000000UL                                     /**< Bit mask for ADC_SYNCBUSY                   */
#define _ADC_STATUS_SYNCBUSY_DEFAULT               0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_SYNCBUSY_DEFAULT                (_ADC_STATUS_SYNCBUSY_DEFAULT << 24)            /**< Shifted mode DEFAULT for ADC_STATUS         */
#define ADC_STATUS_ADCWARM                         (0x1UL << 30)                                   /**< ADC Warmed Up                               */
#define _ADC_STATUS_ADCWARM_SHIFT                  30                                              /**< Shift value for ADC_ADCWARM                 */
#define _ADC_STATUS_ADCWARM_MASK                   0x40000000UL                                    /**< Bit mask for ADC_ADCWARM                    */
#define _ADC_STATUS_ADCWARM_DEFAULT                0x00000000UL                                    /**< Mode DEFAULT for ADC_STATUS                 */
#define ADC_STATUS_ADCWARM_DEFAULT                 (_ADC_STATUS_ADCWARM_DEFAULT << 30)             /**< Shifted mode DEFAULT for ADC_STATUS         */

/* Bit fields for ADC MASKREQ */
#define _ADC_MASKREQ_RESETVALUE                    0x00000000UL                         /**< Default value for ADC_MASKREQ               */
#define _ADC_MASKREQ_MASK                          0x0000FFFFUL                         /**< Mask for ADC_MASKREQ                        */
#define _ADC_MASKREQ_MASKREQ_SHIFT                 0                                    /**< Shift value for ADC_MASKREQ                 */
#define _ADC_MASKREQ_MASKREQ_MASK                  0xFFFFUL                             /**< Bit mask for ADC_MASKREQ                    */
#define _ADC_MASKREQ_MASKREQ_DEFAULT               0x00000000UL                         /**< Mode DEFAULT for ADC_MASKREQ                */
#define ADC_MASKREQ_MASKREQ_DEFAULT                (_ADC_MASKREQ_MASKREQ_DEFAULT << 0)  /**< Shifted mode DEFAULT for ADC_MASKREQ        */

/* Bit fields for ADC STMASK */
#define _ADC_STMASK_RESETVALUE                     0x00000000UL                         /**< Default value for ADC_STMASK                */
#define _ADC_STMASK_MASK                           0x0000FFFFUL                         /**< Mask for ADC_STMASK                         */
#define _ADC_STMASK_STMASK_SHIFT                   0                                    /**< Shift value for ADC_STMASK                  */
#define _ADC_STMASK_STMASK_MASK                    0xFFFFUL                             /**< Bit mask for ADC_STMASK                     */
#define _ADC_STMASK_STMASK_DEFAULT                 0x00000000UL                         /**< Mode DEFAULT for ADC_STMASK                 */
#define ADC_STMASK_STMASK_DEFAULT                  (_ADC_STMASK_STMASK_DEFAULT << 0)    /**< Shifted mode DEFAULT for ADC_STMASK         */

/* Bit fields for ADC CMPTHR */
#define _ADC_CMPTHR_RESETVALUE                     0x00000000UL                         /**< Default value for ADC_CMPTHR                */
#define _ADC_CMPTHR_MASK                           0xFFFFFFFFUL                         /**< Mask for ADC_CMPTHR                         */
#define _ADC_CMPTHR_ADLT_SHIFT                     0                                    /**< Shift value for ADC_ADLT                    */
#define _ADC_CMPTHR_ADLT_MASK                      0xFFFFUL                             /**< Bit mask for ADC_ADLT                       */
#define _ADC_CMPTHR_ADLT_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for ADC_CMPTHR                 */
#define ADC_CMPTHR_ADLT_DEFAULT                    (_ADC_CMPTHR_ADLT_DEFAULT << 0)      /**< Shifted mode DEFAULT for ADC_CMPTHR         */
#define _ADC_CMPTHR_ADGT_SHIFT                     16                                   /**< Shift value for ADC_ADGT                    */
#define _ADC_CMPTHR_ADGT_MASK                      0xFFFF0000UL                         /**< Bit mask for ADC_ADGT                       */
#define _ADC_CMPTHR_ADGT_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for ADC_CMPTHR                 */
#define ADC_CMPTHR_ADGT_DEFAULT                    (_ADC_CMPTHR_ADGT_DEFAULT << 16)     /**< Shifted mode DEFAULT for ADC_CMPTHR         */

/* Bit fields for ADC IF */
#define _ADC_IF_RESETVALUE                         0x00000000UL                         /**< Default value for ADC_IF                    */
#define _ADC_IF_MASK                               0x800A318AUL                         /**< Mask for ADC_IF                             */
#define ADC_IF_SCANFIFODVL                         (0x1UL << 1)                         /**< Scan FIFO Data Valid Level                  */
#define _ADC_IF_SCANFIFODVL_SHIFT                  1                                    /**< Shift value for ADC_SCANFIFODVL             */
#define _ADC_IF_SCANFIFODVL_MASK                   0x2UL                                /**< Bit mask for ADC_SCANFIFODVL                */
#define _ADC_IF_SCANFIFODVL_DEFAULT                0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_SCANFIFODVL_DEFAULT                 (_ADC_IF_SCANFIFODVL_DEFAULT << 1)   /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_SCANCMP                             (0x1UL << 3)                         /**< Scan Result Window Compare                  */
#define _ADC_IF_SCANCMP_SHIFT                      3                                    /**< Shift value for ADC_SCANCMP                 */
#define _ADC_IF_SCANCMP_MASK                       0x8UL                                /**< Bit mask for ADC_SCANCMP                    */
#define _ADC_IF_SCANCMP_DEFAULT                    0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_SCANCMP_DEFAULT                     (_ADC_IF_SCANCMP_DEFAULT << 3)       /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_SCANENTRYDONE                       (0x1UL << 7)                         /**< Scan Entry Done                             */
#define _ADC_IF_SCANENTRYDONE_SHIFT                7                                    /**< Shift value for ADC_SCANENTRYDONE           */
#define _ADC_IF_SCANENTRYDONE_MASK                 0x80UL                               /**< Bit mask for ADC_SCANENTRYDONE              */
#define _ADC_IF_SCANENTRYDONE_DEFAULT              0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_SCANENTRYDONE_DEFAULT               (_ADC_IF_SCANENTRYDONE_DEFAULT << 7) /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_SCANTABLEDONE                       (0x1UL << 8)                         /**< Scan Sequence Done                          */
#define _ADC_IF_SCANTABLEDONE_SHIFT                8                                    /**< Shift value for ADC_SCANTABLEDONE           */
#define _ADC_IF_SCANTABLEDONE_MASK                 0x100UL                              /**< Bit mask for ADC_SCANTABLEDONE              */
#define _ADC_IF_SCANTABLEDONE_DEFAULT              0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_SCANTABLEDONE_DEFAULT               (_ADC_IF_SCANTABLEDONE_DEFAULT << 8) /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_POLARITYERR                         (0x1UL << 12)                        /**< Polarity Error                              */
#define _ADC_IF_POLARITYERR_SHIFT                  12                                   /**< Shift value for ADC_POLARITYERR             */
#define _ADC_IF_POLARITYERR_MASK                   0x1000UL                             /**< Bit mask for ADC_POLARITYERR                */
#define _ADC_IF_POLARITYERR_DEFAULT                0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_POLARITYERR_DEFAULT                 (_ADC_IF_POLARITYERR_DEFAULT << 12)  /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_PORTALLOCERR                        (0x1UL << 13)                        /**< Port Allocation Error                       */
#define _ADC_IF_PORTALLOCERR_SHIFT                 13                                   /**< Shift value for ADC_PORTALLOCERR            */
#define _ADC_IF_PORTALLOCERR_MASK                  0x2000UL                             /**< Bit mask for ADC_PORTALLOCERR               */
#define _ADC_IF_PORTALLOCERR_DEFAULT               0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_PORTALLOCERR_DEFAULT                (_ADC_IF_PORTALLOCERR_DEFAULT << 13) /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_SCANFIFOOF                          (0x1UL << 17)                        /**< Scan FIFO Overflow                          */
#define _ADC_IF_SCANFIFOOF_SHIFT                   17                                   /**< Shift value for ADC_SCANFIFOOF              */
#define _ADC_IF_SCANFIFOOF_MASK                    0x20000UL                            /**< Bit mask for ADC_SCANFIFOOF                 */
#define _ADC_IF_SCANFIFOOF_DEFAULT                 0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_SCANFIFOOF_DEFAULT                  (_ADC_IF_SCANFIFOOF_DEFAULT << 17)   /**< Shifted mode DEFAULT for ADC_IF             */
#define ADC_IF_SCANFIFOUF                          (0x1UL << 19)                        /**< Scan FIFO Underflow                         */
#define _ADC_IF_SCANFIFOUF_SHIFT                   19                                   /**< Shift value for ADC_SCANFIFOUF              */
#define _ADC_IF_SCANFIFOUF_MASK                    0x80000UL                            /**< Bit mask for ADC_SCANFIFOUF                 */
#define _ADC_IF_SCANFIFOUF_DEFAULT                 0x00000000UL                         /**< Mode DEFAULT for ADC_IF                     */
#define ADC_IF_SCANFIFOUF_DEFAULT                  (_ADC_IF_SCANFIFOUF_DEFAULT << 19)   /**< Shifted mode DEFAULT for ADC_IF             */

/* Bit fields for ADC IEN */
#define _ADC_IEN_RESETVALUE                        0x00000000UL                          /**< Default value for ADC_IEN                   */
#define _ADC_IEN_MASK                              0x800A318AUL                          /**< Mask for ADC_IEN                            */
#define ADC_IEN_SCANFIFODVL                        (0x1UL << 1)                          /**< Scan FIFO Data Valid Level Enable           */
#define _ADC_IEN_SCANFIFODVL_SHIFT                 1                                     /**< Shift value for ADC_SCANFIFODVL             */
#define _ADC_IEN_SCANFIFODVL_MASK                  0x2UL                                 /**< Bit mask for ADC_SCANFIFODVL                */
#define _ADC_IEN_SCANFIFODVL_DEFAULT               0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_SCANFIFODVL_DEFAULT                (_ADC_IEN_SCANFIFODVL_DEFAULT << 1)   /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_SCANCMP                            (0x1UL << 3)                          /**< Scan Result Window Compare Enable           */
#define _ADC_IEN_SCANCMP_SHIFT                     3                                     /**< Shift value for ADC_SCANCMP                 */
#define _ADC_IEN_SCANCMP_MASK                      0x8UL                                 /**< Bit mask for ADC_SCANCMP                    */
#define _ADC_IEN_SCANCMP_DEFAULT                   0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_SCANCMP_DEFAULT                    (_ADC_IEN_SCANCMP_DEFAULT << 3)       /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_SCANENTRYDONE                      (0x1UL << 7)                          /**< Scan Entry Done                             */
#define _ADC_IEN_SCANENTRYDONE_SHIFT               7                                     /**< Shift value for ADC_SCANENTRYDONE           */
#define _ADC_IEN_SCANENTRYDONE_MASK                0x80UL                                /**< Bit mask for ADC_SCANENTRYDONE              */
#define _ADC_IEN_SCANENTRYDONE_DEFAULT             0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_SCANENTRYDONE_DEFAULT              (_ADC_IEN_SCANENTRYDONE_DEFAULT << 7) /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_SCANTABLEDONE                      (0x1UL << 8)                          /**< Scan Sequence Done                          */
#define _ADC_IEN_SCANTABLEDONE_SHIFT               8                                     /**< Shift value for ADC_SCANTABLEDONE           */
#define _ADC_IEN_SCANTABLEDONE_MASK                0x100UL                               /**< Bit mask for ADC_SCANTABLEDONE              */
#define _ADC_IEN_SCANTABLEDONE_DEFAULT             0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_SCANTABLEDONE_DEFAULT              (_ADC_IEN_SCANTABLEDONE_DEFAULT << 8) /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_POLARITYERR                        (0x1UL << 12)                         /**< Polarity Error Enable                       */
#define _ADC_IEN_POLARITYERR_SHIFT                 12                                    /**< Shift value for ADC_POLARITYERR             */
#define _ADC_IEN_POLARITYERR_MASK                  0x1000UL                              /**< Bit mask for ADC_POLARITYERR                */
#define _ADC_IEN_POLARITYERR_DEFAULT               0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_POLARITYERR_DEFAULT                (_ADC_IEN_POLARITYERR_DEFAULT << 12)  /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_PORTALLOCERR                       (0x1UL << 13)                         /**< Port Allocation Error Enable                */
#define _ADC_IEN_PORTALLOCERR_SHIFT                13                                    /**< Shift value for ADC_PORTALLOCERR            */
#define _ADC_IEN_PORTALLOCERR_MASK                 0x2000UL                              /**< Bit mask for ADC_PORTALLOCERR               */
#define _ADC_IEN_PORTALLOCERR_DEFAULT              0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_PORTALLOCERR_DEFAULT               (_ADC_IEN_PORTALLOCERR_DEFAULT << 13) /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_SCANFIFOOF                         (0x1UL << 17)                         /**< Scan FIFO Overflow Enable                   */
#define _ADC_IEN_SCANFIFOOF_SHIFT                  17                                    /**< Shift value for ADC_SCANFIFOOF              */
#define _ADC_IEN_SCANFIFOOF_MASK                   0x20000UL                             /**< Bit mask for ADC_SCANFIFOOF                 */
#define _ADC_IEN_SCANFIFOOF_DEFAULT                0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_SCANFIFOOF_DEFAULT                 (_ADC_IEN_SCANFIFOOF_DEFAULT << 17)   /**< Shifted mode DEFAULT for ADC_IEN            */
#define ADC_IEN_SCANFIFOUF                         (0x1UL << 19)                         /**< Scan FIFO Underflow Enable                  */
#define _ADC_IEN_SCANFIFOUF_SHIFT                  19                                    /**< Shift value for ADC_SCANFIFOUF              */
#define _ADC_IEN_SCANFIFOUF_MASK                   0x80000UL                             /**< Bit mask for ADC_SCANFIFOUF                 */
#define _ADC_IEN_SCANFIFOUF_DEFAULT                0x00000000UL                          /**< Mode DEFAULT for ADC_IEN                    */
#define ADC_IEN_SCANFIFOUF_DEFAULT                 (_ADC_IEN_SCANFIFOUF_DEFAULT << 19)   /**< Shifted mode DEFAULT for ADC_IEN            */

/* Bit fields for ADC TRIGGER */
#define _ADC_TRIGGER_RESETVALUE                    0x00000000UL                                  /**< Default value for ADC_TRIGGER               */
#define _ADC_TRIGGER_MASK                          0x07800017UL                                  /**< Mask for ADC_TRIGGER                        */
#define _ADC_TRIGGER_SCANTRIGSEL_SHIFT             0                                             /**< Shift value for ADC_SCANTRIGSEL             */
#define _ADC_TRIGGER_SCANTRIGSEL_MASK              0x7UL                                         /**< Bit mask for ADC_SCANTRIGSEL                */
#define _ADC_TRIGGER_SCANTRIGSEL_DEFAULT           0x00000000UL                                  /**< Mode DEFAULT for ADC_TRIGGER                */
#define _ADC_TRIGGER_SCANTRIGSEL_IMMEDIATE         0x00000000UL                                  /**< Mode IMMEDIATE for ADC_TRIGGER              */
#define _ADC_TRIGGER_SCANTRIGSEL_TIMER             0x00000001UL                                  /**< Mode TIMER for ADC_TRIGGER                  */
#define _ADC_TRIGGER_SCANTRIGSEL_PRSCLKGRP         0x00000002UL                                  /**< Mode PRSCLKGRP for ADC_TRIGGER              */
#define _ADC_TRIGGER_SCANTRIGSEL_PRSPOS            0x00000003UL                                  /**< Mode PRSPOS for ADC_TRIGGER                 */
#define _ADC_TRIGGER_SCANTRIGSEL_PRSNEG            0x00000004UL                                  /**< Mode PRSNEG for ADC_TRIGGER                 */
#define ADC_TRIGGER_SCANTRIGSEL_DEFAULT            (_ADC_TRIGGER_SCANTRIGSEL_DEFAULT << 0)       /**< Shifted mode DEFAULT for ADC_TRIGGER        */
#define ADC_TRIGGER_SCANTRIGSEL_IMMEDIATE          (_ADC_TRIGGER_SCANTRIGSEL_IMMEDIATE << 0)     /**< Shifted mode IMMEDIATE for ADC_TRIGGER      */
#define ADC_TRIGGER_SCANTRIGSEL_TIMER              (_ADC_TRIGGER_SCANTRIGSEL_TIMER << 0)         /**< Shifted mode TIMER for ADC_TRIGGER          */
#define ADC_TRIGGER_SCANTRIGSEL_PRSCLKGRP          (_ADC_TRIGGER_SCANTRIGSEL_PRSCLKGRP << 0)     /**< Shifted mode PRSCLKGRP for ADC_TRIGGER      */
#define ADC_TRIGGER_SCANTRIGSEL_PRSPOS             (_ADC_TRIGGER_SCANTRIGSEL_PRSPOS << 0)        /**< Shifted mode PRSPOS for ADC_TRIGGER         */
#define ADC_TRIGGER_SCANTRIGSEL_PRSNEG             (_ADC_TRIGGER_SCANTRIGSEL_PRSNEG << 0)        /**< Shifted mode PRSNEG for ADC_TRIGGER         */
#define ADC_TRIGGER_SCANTRIGACTION                 (0x1UL << 4)                                  /**< Scan trigger action                         */
#define _ADC_TRIGGER_SCANTRIGACTION_SHIFT          4                                             /**< Shift value for ADC_SCANTRIGACTION          */
#define _ADC_TRIGGER_SCANTRIGACTION_MASK           0x10UL                                        /**< Bit mask for ADC_SCANTRIGACTION             */
#define _ADC_TRIGGER_SCANTRIGACTION_DEFAULT        0x00000000UL                                  /**< Mode DEFAULT for ADC_TRIGGER                */
#define _ADC_TRIGGER_SCANTRIGACTION_ONCE           0x00000000UL                                  /**< Mode ONCE for ADC_TRIGGER                   */
#define _ADC_TRIGGER_SCANTRIGACTION_CONTINUOUS     0x00000001UL                                  /**< Mode CONTINUOUS for ADC_TRIGGER             */
#define ADC_TRIGGER_SCANTRIGACTION_DEFAULT         (_ADC_TRIGGER_SCANTRIGACTION_DEFAULT << 4)    /**< Shifted mode DEFAULT for ADC_TRIGGER        */
#define ADC_TRIGGER_SCANTRIGACTION_ONCE            (_ADC_TRIGGER_SCANTRIGACTION_ONCE << 4)       /**< Shifted mode ONCE for ADC_TRIGGER           */
#define ADC_TRIGGER_SCANTRIGACTION_CONTINUOUS      (_ADC_TRIGGER_SCANTRIGACTION_CONTINUOUS << 4) /**< Shifted mode CONTINUOUS for ADC_TRIGGER     */
#define _ADC_TRIGGER_REPDELAY_SHIFT                23                                            /**< Shift value for ADC_REPDELAY                */
#define _ADC_TRIGGER_REPDELAY_MASK                 0x7800000UL                                   /**< Bit mask for ADC_REPDELAY                   */
#define _ADC_TRIGGER_REPDELAY_DEFAULT              0x00000000UL                                  /**< Mode DEFAULT for ADC_TRIGGER                */
#define _ADC_TRIGGER_REPDELAY_NODELAY              0x00000000UL                                  /**< Mode NODELAY for ADC_TRIGGER                */
#define _ADC_TRIGGER_REPDELAY_CYCLE2               0x00000001UL                                  /**< Mode CYCLE2 for ADC_TRIGGER                 */
#define _ADC_TRIGGER_REPDELAY_CYCLE4               0x00000002UL                                  /**< Mode CYCLE4 for ADC_TRIGGER                 */
#define _ADC_TRIGGER_REPDELAY_CYCLE8               0x00000003UL                                  /**< Mode CYCLE8 for ADC_TRIGGER                 */
#define _ADC_TRIGGER_REPDELAY_CYCLE16              0x00000004UL                                  /**< Mode CYCLE16 for ADC_TRIGGER                */
#define _ADC_TRIGGER_REPDELAY_CYCLE32              0x00000005UL                                  /**< Mode CYCLE32 for ADC_TRIGGER                */
#define _ADC_TRIGGER_REPDELAY_CYCLE64              0x00000006UL                                  /**< Mode CYCLE64 for ADC_TRIGGER                */
#define _ADC_TRIGGER_REPDELAY_CYCLE128             0x00000007UL                                  /**< Mode CYCLE128 for ADC_TRIGGER               */
#define _ADC_TRIGGER_REPDELAY_CYCLE256             0x00000008UL                                  /**< Mode CYCLE256 for ADC_TRIGGER               */
#define _ADC_TRIGGER_REPDELAY_CYCLE512             0x00000009UL                                  /**< Mode CYCLE512 for ADC_TRIGGER               */
#define _ADC_TRIGGER_REPDELAY_CYCLE1024            0x0000000AUL                                  /**< Mode CYCLE1024 for ADC_TRIGGER              */
#define ADC_TRIGGER_REPDELAY_DEFAULT               (_ADC_TRIGGER_REPDELAY_DEFAULT << 23)         /**< Shifted mode DEFAULT for ADC_TRIGGER        */
#define ADC_TRIGGER_REPDELAY_NODELAY               (_ADC_TRIGGER_REPDELAY_NODELAY << 23)         /**< Shifted mode NODELAY for ADC_TRIGGER        */
#define ADC_TRIGGER_REPDELAY_CYCLE2                (_ADC_TRIGGER_REPDELAY_CYCLE2 << 23)          /**< Shifted mode CYCLE2 for ADC_TRIGGER         */
#define ADC_TRIGGER_REPDELAY_CYCLE4                (_ADC_TRIGGER_REPDELAY_CYCLE4 << 23)          /**< Shifted mode CYCLE4 for ADC_TRIGGER         */
#define ADC_TRIGGER_REPDELAY_CYCLE8                (_ADC_TRIGGER_REPDELAY_CYCLE8 << 23)          /**< Shifted mode CYCLE8 for ADC_TRIGGER         */
#define ADC_TRIGGER_REPDELAY_CYCLE16               (_ADC_TRIGGER_REPDELAY_CYCLE16 << 23)         /**< Shifted mode CYCLE16 for ADC_TRIGGER        */
#define ADC_TRIGGER_REPDELAY_CYCLE32               (_ADC_TRIGGER_REPDELAY_CYCLE32 << 23)         /**< Shifted mode CYCLE32 for ADC_TRIGGER        */
#define ADC_TRIGGER_REPDELAY_CYCLE64               (_ADC_TRIGGER_REPDELAY_CYCLE64 << 23)         /**< Shifted mode CYCLE64 for ADC_TRIGGER        */
#define ADC_TRIGGER_REPDELAY_CYCLE128              (_ADC_TRIGGER_REPDELAY_CYCLE128 << 23)        /**< Shifted mode CYCLE128 for ADC_TRIGGER       */
#define ADC_TRIGGER_REPDELAY_CYCLE256              (_ADC_TRIGGER_REPDELAY_CYCLE256 << 23)        /**< Shifted mode CYCLE256 for ADC_TRIGGER       */
#define ADC_TRIGGER_REPDELAY_CYCLE512              (_ADC_TRIGGER_REPDELAY_CYCLE512 << 23)        /**< Shifted mode CYCLE512 for ADC_TRIGGER       */
#define ADC_TRIGGER_REPDELAY_CYCLE1024             (_ADC_TRIGGER_REPDELAY_CYCLE1024 << 23)       /**< Shifted mode CYCLE1024 for ADC_TRIGGER      */

/* Bit fields for ADC CFG */
#define _ADC_CFG_RESETVALUE                        0x00002000UL                            /**< Default value for ADC_CFG                   */
#define _ADC_CFG_MASK                              0x07807FFCUL                            /**< Mask for ADC_CFG                            */
#define _ADC_CFG_ATIME_SHIFT                       2                                       /**< Shift value for ADC_ATIME                   */
#define _ADC_CFG_ATIME_MASK                        0xFFCUL                                 /**< Bit mask for ADC_ATIME                      */
#define _ADC_CFG_ATIME_DEFAULT                     0x00000000UL                            /**< Mode DEFAULT for ADC_CFG                    */
#define ADC_CFG_ATIME_DEFAULT                      (_ADC_CFG_ATIME_DEFAULT << 2)           /**< Shifted mode DEFAULT for ADC_CFG            */
#define _ADC_CFG_ANALOGGAIN_SHIFT                  12                                      /**< Shift value for ADC_ANALOGGAIN              */
#define _ADC_CFG_ANALOGGAIN_MASK                   0x7000UL                                /**< Bit mask for ADC_ANALOGGAIN                 */
#define _ADC_CFG_ANALOGGAIN_DEFAULT                0x00000002UL                            /**< Mode DEFAULT for ADC_CFG                    */
#define _ADC_CFG_ANALOGGAIN_GAIN_0_3125            0x00000000UL                            /**< Mode GAIN_0_3125 for ADC_CFG                */
#define _ADC_CFG_ANALOGGAIN_GAIN_0_5               0x00000001UL                            /**< Mode GAIN_0_5 for ADC_CFG                   */
#define _ADC_CFG_ANALOGGAIN_GAIN_1                 0x00000002UL                            /**< Mode GAIN_1 for ADC_CFG                     */
#define _ADC_CFG_ANALOGGAIN_GAIN_2                 0x00000003UL                            /**< Mode GAIN_2 for ADC_CFG                     */
#define _ADC_CFG_ANALOGGAIN_GAIN_4                 0x00000004UL                            /**< Mode GAIN_4 for ADC_CFG                     */
#define ADC_CFG_ANALOGGAIN_DEFAULT                 (_ADC_CFG_ANALOGGAIN_DEFAULT << 12)     /**< Shifted mode DEFAULT for ADC_CFG            */
#define ADC_CFG_ANALOGGAIN_GAIN_0_3125             (_ADC_CFG_ANALOGGAIN_GAIN_0_3125 << 12) /**< Shifted mode GAIN_0_3125 for ADC_CFG        */
#define ADC_CFG_ANALOGGAIN_GAIN_0_5                (_ADC_CFG_ANALOGGAIN_GAIN_0_5 << 12)    /**< Shifted mode GAIN_0_5 for ADC_CFG           */
#define ADC_CFG_ANALOGGAIN_GAIN_1                  (_ADC_CFG_ANALOGGAIN_GAIN_1 << 12)      /**< Shifted mode GAIN_1 for ADC_CFG             */
#define ADC_CFG_ANALOGGAIN_GAIN_2                  (_ADC_CFG_ANALOGGAIN_GAIN_2 << 12)      /**< Shifted mode GAIN_2 for ADC_CFG             */
#define ADC_CFG_ANALOGGAIN_GAIN_4                  (_ADC_CFG_ANALOGGAIN_GAIN_4 << 12)      /**< Shifted mode GAIN_4 for ADC_CFG             */
#define _ADC_CFG_AVERAGESEL_SHIFT                  23                                      /**< Shift value for ADC_AVERAGESEL              */
#define _ADC_CFG_AVERAGESEL_MASK                   0x7800000UL                             /**< Bit mask for ADC_AVERAGESEL                 */
#define _ADC_CFG_AVERAGESEL_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for ADC_CFG                    */
#define _ADC_CFG_AVERAGESEL_X1                     0x00000000UL                            /**< Mode X1 for ADC_CFG                         */
#define _ADC_CFG_AVERAGESEL_X2                     0x00000001UL                            /**< Mode X2 for ADC_CFG                         */
#define _ADC_CFG_AVERAGESEL_X4                     0x00000002UL                            /**< Mode X4 for ADC_CFG                         */
#define _ADC_CFG_AVERAGESEL_X8                     0x00000003UL                            /**< Mode X8 for ADC_CFG                         */
#define _ADC_CFG_AVERAGESEL_X16                    0x00000004UL                            /**< Mode X16 for ADC_CFG                        */
#define _ADC_CFG_AVERAGESEL_X32                    0x00000005UL                            /**< Mode X32 for ADC_CFG                        */
#define _ADC_CFG_AVERAGESEL_X64                    0x00000006UL                            /**< Mode X64 for ADC_CFG                        */
#define _ADC_CFG_AVERAGESEL_X128                   0x00000007UL                            /**< Mode X128 for ADC_CFG                       */
#define _ADC_CFG_AVERAGESEL_X256                   0x00000008UL                            /**< Mode X256 for ADC_CFG                       */
#define _ADC_CFG_AVERAGESEL_X512                   0x00000009UL                            /**< Mode X512 for ADC_CFG                       */
#define _ADC_CFG_AVERAGESEL_X1024                  0x0000000AUL                            /**< Mode X1024 for ADC_CFG                      */
#define ADC_CFG_AVERAGESEL_DEFAULT                 (_ADC_CFG_AVERAGESEL_DEFAULT << 23)     /**< Shifted mode DEFAULT for ADC_CFG            */
#define ADC_CFG_AVERAGESEL_X1                      (_ADC_CFG_AVERAGESEL_X1 << 23)          /**< Shifted mode X1 for ADC_CFG                 */
#define ADC_CFG_AVERAGESEL_X2                      (_ADC_CFG_AVERAGESEL_X2 << 23)          /**< Shifted mode X2 for ADC_CFG                 */
#define ADC_CFG_AVERAGESEL_X4                      (_ADC_CFG_AVERAGESEL_X4 << 23)          /**< Shifted mode X4 for ADC_CFG                 */
#define ADC_CFG_AVERAGESEL_X8                      (_ADC_CFG_AVERAGESEL_X8 << 23)          /**< Shifted mode X8 for ADC_CFG                 */
#define ADC_CFG_AVERAGESEL_X16                     (_ADC_CFG_AVERAGESEL_X16 << 23)         /**< Shifted mode X16 for ADC_CFG                */
#define ADC_CFG_AVERAGESEL_X32                     (_ADC_CFG_AVERAGESEL_X32 << 23)         /**< Shifted mode X32 for ADC_CFG                */
#define ADC_CFG_AVERAGESEL_X64                     (_ADC_CFG_AVERAGESEL_X64 << 23)         /**< Shifted mode X64 for ADC_CFG                */
#define ADC_CFG_AVERAGESEL_X128                    (_ADC_CFG_AVERAGESEL_X128 << 23)        /**< Shifted mode X128 for ADC_CFG               */
#define ADC_CFG_AVERAGESEL_X256                    (_ADC_CFG_AVERAGESEL_X256 << 23)        /**< Shifted mode X256 for ADC_CFG               */
#define ADC_CFG_AVERAGESEL_X512                    (_ADC_CFG_AVERAGESEL_X512 << 23)        /**< Shifted mode X512 for ADC_CFG               */
#define ADC_CFG_AVERAGESEL_X1024                   (_ADC_CFG_AVERAGESEL_X1024 << 23)       /**< Shifted mode X1024 for ADC_CFG              */

/* Bit fields for ADC OFFSETSE */
#define _ADC_OFFSETSE_RESETVALUE                   0x08000800UL                         /**< Default value for ADC_OFFSETSE              */
#define _ADC_OFFSETSE_MASK                         0x0FFF0FFFUL                         /**< Mask for ADC_OFFSETSE                       */
#define _ADC_OFFSETSE_SEPOS_SHIFT                  0                                    /**< Shift value for ADC_SEPOS                   */
#define _ADC_OFFSETSE_SEPOS_MASK                   0xFFFUL                              /**< Bit mask for ADC_SEPOS                      */
#define _ADC_OFFSETSE_SEPOS_DEFAULT                0x00000800UL                         /**< Mode DEFAULT for ADC_OFFSETSE               */
#define ADC_OFFSETSE_SEPOS_DEFAULT                 (_ADC_OFFSETSE_SEPOS_DEFAULT << 0)   /**< Shifted mode DEFAULT for ADC_OFFSETSE       */
#define _ADC_OFFSETSE_SENEG_SHIFT                  16                                   /**< Shift value for ADC_SENEG                   */
#define _ADC_OFFSETSE_SENEG_MASK                   0xFFF0000UL                          /**< Bit mask for ADC_SENEG                      */
#define _ADC_OFFSETSE_SENEG_DEFAULT                0x00000800UL                         /**< Mode DEFAULT for ADC_OFFSETSE               */
#define ADC_OFFSETSE_SENEG_DEFAULT                 (_ADC_OFFSETSE_SENEG_DEFAULT << 16)  /**< Shifted mode DEFAULT for ADC_OFFSETSE       */

/* Bit fields for ADC OFFSETDIFF */
#define _ADC_OFFSETDIFF_RESETVALUE                 0x00000800UL                         /**< Default value for ADC_OFFSETDIFF            */
#define _ADC_OFFSETDIFF_MASK                       0x00000FFFUL                         /**< Mask for ADC_OFFSETDIFF                     */
#define _ADC_OFFSETDIFF_DIFF_SHIFT                 0                                    /**< Shift value for ADC_DIFF                    */
#define _ADC_OFFSETDIFF_DIFF_MASK                  0xFFFUL                              /**< Bit mask for ADC_DIFF                       */
#define _ADC_OFFSETDIFF_DIFF_DEFAULT               0x00000800UL                         /**< Mode DEFAULT for ADC_OFFSETDIFF             */
#define ADC_OFFSETDIFF_DIFF_DEFAULT                (_ADC_OFFSETDIFF_DIFF_DEFAULT << 0)  /**< Shifted mode DEFAULT for ADC_OFFSETDIFF     */

/* Bit fields for ADC GAINCAL */
#define _ADC_GAINCAL_RESETVALUE                    0x06018060UL                         /**< Default value for ADC_GAINCAL               */
#define _ADC_GAINCAL_MASK                          0x07F1FC7FUL                         /**< Mask for ADC_GAINCAL                        */
#define _ADC_GAINCAL_SEPOS_SHIFT                   0                                    /**< Shift value for ADC_SEPOS                   */
#define _ADC_GAINCAL_SEPOS_MASK                    0x7FUL                               /**< Bit mask for ADC_SEPOS                      */
#define _ADC_GAINCAL_SEPOS_DEFAULT                 0x00000060UL                         /**< Mode DEFAULT for ADC_GAINCAL                */
#define ADC_GAINCAL_SEPOS_DEFAULT                  (_ADC_GAINCAL_SEPOS_DEFAULT << 0)    /**< Shifted mode DEFAULT for ADC_GAINCAL        */
#define _ADC_GAINCAL_SENEG_SHIFT                   10                                   /**< Shift value for ADC_SENEG                   */
#define _ADC_GAINCAL_SENEG_MASK                    0x1FC00UL                            /**< Bit mask for ADC_SENEG                      */
#define _ADC_GAINCAL_SENEG_DEFAULT                 0x00000060UL                         /**< Mode DEFAULT for ADC_GAINCAL                */
#define ADC_GAINCAL_SENEG_DEFAULT                  (_ADC_GAINCAL_SENEG_DEFAULT << 10)   /**< Shifted mode DEFAULT for ADC_GAINCAL        */
#define _ADC_GAINCAL_DIFF_SHIFT                    20                                   /**< Shift value for ADC_DIFF                    */
#define _ADC_GAINCAL_DIFF_MASK                     0x7F00000UL                          /**< Bit mask for ADC_DIFF                       */
#define _ADC_GAINCAL_DIFF_DEFAULT                  0x00000060UL                         /**< Mode DEFAULT for ADC_GAINCAL                */
#define ADC_GAINCAL_DIFF_DEFAULT                   (_ADC_GAINCAL_DIFF_DEFAULT << 20)    /**< Shifted mode DEFAULT for ADC_GAINCAL        */

/* Bit fields for ADC SCANFIFOCFG */
#define _ADC_SCANFIFOCFG_RESETVALUE                0x00000030UL                              /**< Default value for ADC_SCANFIFOCFG           */
#define _ADC_SCANFIFOCFG_MASK                      0x000001FFUL                              /**< Mask for ADC_SCANFIFOCFG                    */
#define _ADC_SCANFIFOCFG_ALIGNMENT_SHIFT           0                                         /**< Shift value for ADC_ALIGNMENT               */
#define _ADC_SCANFIFOCFG_ALIGNMENT_MASK            0x7UL                                     /**< Bit mask for ADC_ALIGNMENT                  */
#define _ADC_SCANFIFOCFG_ALIGNMENT_DEFAULT         0x00000000UL                              /**< Mode DEFAULT for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT16         0x00000000UL                              /**< Mode RIGHT16 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT12         0x00000001UL                              /**< Mode RIGHT12 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_ALIGNMENT_RIGHT8          0x00000002UL                              /**< Mode RIGHT8 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_ALIGNMENT_LEFT16          0x00000003UL                              /**< Mode LEFT16 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_ALIGNMENT_LEFT12          0x00000004UL                              /**< Mode LEFT12 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_ALIGNMENT_LEFT8           0x00000005UL                              /**< Mode LEFT8 for ADC_SCANFIFOCFG              */
#define ADC_SCANFIFOCFG_ALIGNMENT_DEFAULT          (_ADC_SCANFIFOCFG_ALIGNMENT_DEFAULT << 0) /**< Shifted mode DEFAULT for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_ALIGNMENT_RIGHT16          (_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT16 << 0) /**< Shifted mode RIGHT16 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_ALIGNMENT_RIGHT12          (_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT12 << 0) /**< Shifted mode RIGHT12 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_ALIGNMENT_RIGHT8           (_ADC_SCANFIFOCFG_ALIGNMENT_RIGHT8 << 0)  /**< Shifted mode RIGHT8 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_ALIGNMENT_LEFT16           (_ADC_SCANFIFOCFG_ALIGNMENT_LEFT16 << 0)  /**< Shifted mode LEFT16 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_ALIGNMENT_LEFT12           (_ADC_SCANFIFOCFG_ALIGNMENT_LEFT12 << 0)  /**< Shifted mode LEFT12 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_ALIGNMENT_LEFT8            (_ADC_SCANFIFOCFG_ALIGNMENT_LEFT8 << 0)   /**< Shifted mode LEFT8 for ADC_SCANFIFOCFG      */
#define ADC_SCANFIFOCFG_SHOWID                     (0x1UL << 3)                              /**< Show ID                                     */
#define _ADC_SCANFIFOCFG_SHOWID_SHIFT              3                                         /**< Shift value for ADC_SHOWID                  */
#define _ADC_SCANFIFOCFG_SHOWID_MASK               0x8UL                                     /**< Bit mask for ADC_SHOWID                     */
#define _ADC_SCANFIFOCFG_SHOWID_DEFAULT            0x00000000UL                              /**< Mode DEFAULT for ADC_SCANFIFOCFG            */
#define ADC_SCANFIFOCFG_SHOWID_DEFAULT             (_ADC_SCANFIFOCFG_SHOWID_DEFAULT << 3)    /**< Shifted mode DEFAULT for ADC_SCANFIFOCFG    */
#define _ADC_SCANFIFOCFG_DVL_SHIFT                 4                                         /**< Shift value for ADC_DVL                     */
#define _ADC_SCANFIFOCFG_DVL_MASK                  0xF0UL                                    /**< Bit mask for ADC_DVL                        */
#define _ADC_SCANFIFOCFG_DVL_DEFAULT               0x00000003UL                              /**< Mode DEFAULT for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID1                0x00000000UL                              /**< Mode VALID1 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID2                0x00000001UL                              /**< Mode VALID2 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID3                0x00000002UL                              /**< Mode VALID3 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID4                0x00000003UL                              /**< Mode VALID4 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID5                0x00000004UL                              /**< Mode VALID5 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID6                0x00000005UL                              /**< Mode VALID6 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID7                0x00000006UL                              /**< Mode VALID7 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID8                0x00000007UL                              /**< Mode VALID8 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID9                0x00000008UL                              /**< Mode VALID9 for ADC_SCANFIFOCFG             */
#define _ADC_SCANFIFOCFG_DVL_VALID10               0x00000009UL                              /**< Mode VALID10 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID11               0x0000000AUL                              /**< Mode VALID11 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID12               0x0000000BUL                              /**< Mode VALID12 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID13               0x0000000CUL                              /**< Mode VALID13 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID14               0x0000000DUL                              /**< Mode VALID14 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID15               0x0000000EUL                              /**< Mode VALID15 for ADC_SCANFIFOCFG            */
#define _ADC_SCANFIFOCFG_DVL_VALID16               0x0000000FUL                              /**< Mode VALID16 for ADC_SCANFIFOCFG            */
#define ADC_SCANFIFOCFG_DVL_DEFAULT                (_ADC_SCANFIFOCFG_DVL_DEFAULT << 4)       /**< Shifted mode DEFAULT for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID1                 (_ADC_SCANFIFOCFG_DVL_VALID1 << 4)        /**< Shifted mode VALID1 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID2                 (_ADC_SCANFIFOCFG_DVL_VALID2 << 4)        /**< Shifted mode VALID2 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID3                 (_ADC_SCANFIFOCFG_DVL_VALID3 << 4)        /**< Shifted mode VALID3 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID4                 (_ADC_SCANFIFOCFG_DVL_VALID4 << 4)        /**< Shifted mode VALID4 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID5                 (_ADC_SCANFIFOCFG_DVL_VALID5 << 4)        /**< Shifted mode VALID5 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID6                 (_ADC_SCANFIFOCFG_DVL_VALID6 << 4)        /**< Shifted mode VALID6 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID7                 (_ADC_SCANFIFOCFG_DVL_VALID7 << 4)        /**< Shifted mode VALID7 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID8                 (_ADC_SCANFIFOCFG_DVL_VALID8 << 4)        /**< Shifted mode VALID8 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID9                 (_ADC_SCANFIFOCFG_DVL_VALID9 << 4)        /**< Shifted mode VALID9 for ADC_SCANFIFOCFG     */
#define ADC_SCANFIFOCFG_DVL_VALID10                (_ADC_SCANFIFOCFG_DVL_VALID10 << 4)       /**< Shifted mode VALID10 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID11                (_ADC_SCANFIFOCFG_DVL_VALID11 << 4)       /**< Shifted mode VALID11 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID12                (_ADC_SCANFIFOCFG_DVL_VALID12 << 4)       /**< Shifted mode VALID12 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID13                (_ADC_SCANFIFOCFG_DVL_VALID13 << 4)       /**< Shifted mode VALID13 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID14                (_ADC_SCANFIFOCFG_DVL_VALID14 << 4)       /**< Shifted mode VALID14 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID15                (_ADC_SCANFIFOCFG_DVL_VALID15 << 4)       /**< Shifted mode VALID15 for ADC_SCANFIFOCFG    */
#define ADC_SCANFIFOCFG_DVL_VALID16                (_ADC_SCANFIFOCFG_DVL_VALID16 << 4)       /**< Shifted mode VALID16 for ADC_SCANFIFOCFG    */

/* Bit fields for ADC SCANFIFODATA */
#define _ADC_SCANFIFODATA_RESETVALUE               0x00000000UL                          /**< Default value for ADC_SCANFIFODATA          */
#define _ADC_SCANFIFODATA_MASK                     0xFFFFFFFFUL                          /**< Mask for ADC_SCANFIFODATA                   */
#define _ADC_SCANFIFODATA_DATA_SHIFT               0                                     /**< Shift value for ADC_DATA                    */
#define _ADC_SCANFIFODATA_DATA_MASK                0xFFFFFFFFUL                          /**< Bit mask for ADC_DATA                       */
#define _ADC_SCANFIFODATA_DATA_DEFAULT             0x00000000UL                          /**< Mode DEFAULT for ADC_SCANFIFODATA           */
#define ADC_SCANFIFODATA_DATA_DEFAULT              (_ADC_SCANFIFODATA_DATA_DEFAULT << 0) /**< Shifted mode DEFAULT for ADC_SCANFIFODATA   */

/* Bit fields for ADC SCANFIFOSTAT */
#define _ADC_SCANFIFOSTAT_RESETVALUE               0x00000000UL                                 /**< Default value for ADC_SCANFIFOSTAT          */
#define _ADC_SCANFIFOSTAT_MASK                     0x0000001FUL                                 /**< Mask for ADC_SCANFIFOSTAT                   */
#define _ADC_SCANFIFOSTAT_FIFOREADCNT_SHIFT        0                                            /**< Shift value for ADC_FIFOREADCNT             */
#define _ADC_SCANFIFOSTAT_FIFOREADCNT_MASK         0x1FUL                                       /**< Bit mask for ADC_FIFOREADCNT                */
#define _ADC_SCANFIFOSTAT_FIFOREADCNT_DEFAULT      0x00000000UL                                 /**< Mode DEFAULT for ADC_SCANFIFOSTAT           */
#define ADC_SCANFIFOSTAT_FIFOREADCNT_DEFAULT       (_ADC_SCANFIFOSTAT_FIFOREADCNT_DEFAULT << 0) /**< Shifted mode DEFAULT for ADC_SCANFIFOSTAT   */

/* Bit fields for ADC SCANDATA */
#define _ADC_SCANDATA_RESETVALUE                   0x00000000UL                         /**< Default value for ADC_SCANDATA              */
#define _ADC_SCANDATA_MASK                         0xFFFFFFFFUL                         /**< Mask for ADC_SCANDATA                       */
#define _ADC_SCANDATA_DATA_SHIFT                   0                                    /**< Shift value for ADC_DATA                    */
#define _ADC_SCANDATA_DATA_MASK                    0xFFFFFFFFUL                         /**< Bit mask for ADC_DATA                       */
#define _ADC_SCANDATA_DATA_DEFAULT                 0x00000000UL                         /**< Mode DEFAULT for ADC_SCANDATA               */
#define ADC_SCANDATA_DATA_DEFAULT                  (_ADC_SCANDATA_DATA_DEFAULT << 0)    /**< Shifted mode DEFAULT for ADC_SCANDATA       */

/* Bit fields for ADC SCAN */
#define _ADC_SCAN_RESETVALUE                       0x00000000UL                         /**< Default value for ADC_SCAN                  */
#define _ADC_SCAN_MASK                             0x0003FFFFUL                         /**< Mask for ADC_SCAN                           */
#define _ADC_SCAN_PINNEG_SHIFT                     0                                    /**< Shift value for ADC_PINNEG                  */
#define _ADC_SCAN_PINNEG_MASK                      0xFUL                                /**< Bit mask for ADC_PINNEG                     */
#define _ADC_SCAN_PINNEG_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for ADC_SCAN                   */
#define ADC_SCAN_PINNEG_DEFAULT                    (_ADC_SCAN_PINNEG_DEFAULT << 0)      /**< Shifted mode DEFAULT for ADC_SCAN           */
#define _ADC_SCAN_PORTNEG_SHIFT                    4                                    /**< Shift value for ADC_PORTNEG                 */
#define _ADC_SCAN_PORTNEG_MASK                     0xF0UL                               /**< Bit mask for ADC_PORTNEG                    */
#define _ADC_SCAN_PORTNEG_DEFAULT                  0x00000000UL                         /**< Mode DEFAULT for ADC_SCAN                   */
#define _ADC_SCAN_PORTNEG_GND                      0x00000000UL                         /**< Mode GND for ADC_SCAN                       */
#define _ADC_SCAN_PORTNEG_PORTA                    0x00000008UL                         /**< Mode PORTA for ADC_SCAN                     */
#define _ADC_SCAN_PORTNEG_PORTB                    0x00000009UL                         /**< Mode PORTB for ADC_SCAN                     */
#define _ADC_SCAN_PORTNEG_PORTC                    0x0000000AUL                         /**< Mode PORTC for ADC_SCAN                     */
#define _ADC_SCAN_PORTNEG_PORTD                    0x0000000BUL                         /**< Mode PORTD for ADC_SCAN                     */
#define ADC_SCAN_PORTNEG_DEFAULT                   (_ADC_SCAN_PORTNEG_DEFAULT << 4)     /**< Shifted mode DEFAULT for ADC_SCAN           */
#define ADC_SCAN_PORTNEG_GND                       (_ADC_SCAN_PORTNEG_GND << 4)         /**< Shifted mode GND for ADC_SCAN               */
#define ADC_SCAN_PORTNEG_PORTA                     (_ADC_SCAN_PORTNEG_PORTA << 4)       /**< Shifted mode PORTA for ADC_SCAN             */
#define ADC_SCAN_PORTNEG_PORTB                     (_ADC_SCAN_PORTNEG_PORTB << 4)       /**< Shifted mode PORTB for ADC_SCAN             */
#define ADC_SCAN_PORTNEG_PORTC                     (_ADC_SCAN_PORTNEG_PORTC << 4)       /**< Shifted mode PORTC for ADC_SCAN             */
#define ADC_SCAN_PORTNEG_PORTD                     (_ADC_SCAN_PORTNEG_PORTD << 4)       /**< Shifted mode PORTD for ADC_SCAN             */
#define _ADC_SCAN_PINPOS_SHIFT                     8                                    /**< Shift value for ADC_PINPOS                  */
#define _ADC_SCAN_PINPOS_MASK                      0xF00UL                              /**< Bit mask for ADC_PINPOS                     */
#define _ADC_SCAN_PINPOS_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for ADC_SCAN                   */
#define ADC_SCAN_PINPOS_DEFAULT                    (_ADC_SCAN_PINPOS_DEFAULT << 8)      /**< Shifted mode DEFAULT for ADC_SCAN           */
#define _ADC_SCAN_PORTPOS_SHIFT                    12                                   /**< Shift value for ADC_PORTPOS                 */
#define _ADC_SCAN_PORTPOS_MASK                     0xF000UL                             /**< Bit mask for ADC_PORTPOS                    */
#define _ADC_SCAN_PORTPOS_DEFAULT                  0x00000000UL                         /**< Mode DEFAULT for ADC_SCAN                   */
#define _ADC_SCAN_PORTPOS_GND                      0x00000000UL                         /**< Mode GND for ADC_SCAN                       */
#define _ADC_SCAN_PORTPOS_SUPPLY                   0x00000001UL                         /**< Mode SUPPLY for ADC_SCAN                    */
#define _ADC_SCAN_PORTPOS_PORTA                    0x00000008UL                         /**< Mode PORTA for ADC_SCAN                     */
#define _ADC_SCAN_PORTPOS_PORTB                    0x00000009UL                         /**< Mode PORTB for ADC_SCAN                     */
#define _ADC_SCAN_PORTPOS_PORTC                    0x0000000AUL                         /**< Mode PORTC for ADC_SCAN                     */
#define _ADC_SCAN_PORTPOS_PORTD                    0x0000000BUL                         /**< Mode PORTD for ADC_SCAN                     */
#define ADC_SCAN_PORTPOS_DEFAULT                   (_ADC_SCAN_PORTPOS_DEFAULT << 12)    /**< Shifted mode DEFAULT for ADC_SCAN           */
#define ADC_SCAN_PORTPOS_GND                       (_ADC_SCAN_PORTPOS_GND << 12)        /**< Shifted mode GND for ADC_SCAN               */
#define ADC_SCAN_PORTPOS_SUPPLY                    (_ADC_SCAN_PORTPOS_SUPPLY << 12)     /**< Shifted mode SUPPLY for ADC_SCAN            */
#define ADC_SCAN_PORTPOS_PORTA                     (_ADC_SCAN_PORTPOS_PORTA << 12)      /**< Shifted mode PORTA for ADC_SCAN             */
#define ADC_SCAN_PORTPOS_PORTB                     (_ADC_SCAN_PORTPOS_PORTB << 12)      /**< Shifted mode PORTB for ADC_SCAN             */
#define ADC_SCAN_PORTPOS_PORTC                     (_ADC_SCAN_PORTPOS_PORTC << 12)      /**< Shifted mode PORTC for ADC_SCAN             */
#define ADC_SCAN_PORTPOS_PORTD                     (_ADC_SCAN_PORTPOS_PORTD << 12)      /**< Shifted mode PORTD for ADC_SCAN             */
#define ADC_SCAN_CFG                               (0x1UL << 16)                        /**< Configuration Group Select                  */
#define _ADC_SCAN_CFG_SHIFT                        16                                   /**< Shift value for ADC_CFG                     */
#define _ADC_SCAN_CFG_MASK                         0x10000UL                            /**< Bit mask for ADC_CFG                        */
#define _ADC_SCAN_CFG_DEFAULT                      0x00000000UL                         /**< Mode DEFAULT for ADC_SCAN                   */
#define _ADC_SCAN_CFG_CONFIG0                      0x00000000UL                         /**< Mode CONFIG0 for ADC_SCAN                   */
#define _ADC_SCAN_CFG_CONFIG1                      0x00000001UL                         /**< Mode CONFIG1 for ADC_SCAN                   */
#define ADC_SCAN_CFG_DEFAULT                       (_ADC_SCAN_CFG_DEFAULT << 16)        /**< Shifted mode DEFAULT for ADC_SCAN           */
#define ADC_SCAN_CFG_CONFIG0                       (_ADC_SCAN_CFG_CONFIG0 << 16)        /**< Shifted mode CONFIG0 for ADC_SCAN           */
#define ADC_SCAN_CFG_CONFIG1                       (_ADC_SCAN_CFG_CONFIG1 << 16)        /**< Shifted mode CONFIG1 for ADC_SCAN           */
#define ADC_SCAN_CMP                               (0x1UL << 17)                        /**< Comparison Enable                           */
#define _ADC_SCAN_CMP_SHIFT                        17                                   /**< Shift value for ADC_CMP                     */
#define _ADC_SCAN_CMP_MASK                         0x20000UL                            /**< Bit mask for ADC_CMP                        */
#define _ADC_SCAN_CMP_DEFAULT                      0x00000000UL                         /**< Mode DEFAULT for ADC_SCAN                   */
#define ADC_SCAN_CMP_DEFAULT                       (_ADC_SCAN_CMP_DEFAULT << 17)        /**< Shifted mode DEFAULT for ADC_SCAN           */

/** @} End of group SIMG301_ADC_BitFields */
/** @} End of group SIMG301_ADC */
/** @} End of group Parts */

#endif // SIMG301_ADC_H
