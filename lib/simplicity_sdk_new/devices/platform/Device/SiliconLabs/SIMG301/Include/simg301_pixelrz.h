/**************************************************************************//**
 * @file
 * @brief SIMG301 PIXELRZ register and bit field definitions
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
#ifndef SIMG301_PIXELRZ_H
#define SIMG301_PIXELRZ_H
#define PIXELRZ_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_PIXELRZ PIXELRZ
 * @{
 * @brief SIMG301 PIXELRZ Register Declaration.
 *****************************************************************************/

/** PIXELRZ Register Declaration. */
typedef struct pixelrz_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP VERSION Register                                */
  __IOM uint32_t EN;                            /**< Enable Register                                    */
  __IOM uint32_t SWRST;                         /**< Software Reset Register                            */
  __IOM uint32_t CFG;                           /**< Configuration Register                             */
  __IOM uint32_t FRAMECFG;                      /**< Frame Configuration Register                       */
  __IOM uint32_t T0CFG;                         /**< Zero symbol configuration register                 */
  __IOM uint32_t T1CFG;                         /**< One symbol configuration register                  */
  __IOM uint32_t TSTOPCFG;                      /**< Stop sympbol configuration register                */
  __IOM uint32_t TRSTCFG;                       /**< Reset symbol configuration register                */
  __IOM uint32_t CMD;                           /**< Command Register                                   */
  __IOM uint32_t TXDATA;                        /**< Transmit Buffer Data Register                      */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  __IOM uint32_t IF;                            /**<  Interrupt Flag register                           */
  __IOM uint32_t IEN;                           /**< Interrupt Enable register                          */
  uint32_t       RESERVED0[1010U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_SET;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_SET;                     /**< Software Reset Register                            */
  __IOM uint32_t CFG_SET;                       /**< Configuration Register                             */
  __IOM uint32_t FRAMECFG_SET;                  /**< Frame Configuration Register                       */
  __IOM uint32_t T0CFG_SET;                     /**< Zero symbol configuration register                 */
  __IOM uint32_t T1CFG_SET;                     /**< One symbol configuration register                  */
  __IOM uint32_t TSTOPCFG_SET;                  /**< Stop sympbol configuration register                */
  __IOM uint32_t TRSTCFG_SET;                   /**< Reset symbol configuration register                */
  __IOM uint32_t CMD_SET;                       /**< Command Register                                   */
  __IOM uint32_t TXDATA_SET;                    /**< Transmit Buffer Data Register                      */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  __IOM uint32_t IF_SET;                        /**<  Interrupt Flag register                           */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable register                          */
  uint32_t       RESERVED1[1010U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_CLR;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_CLR;                     /**< Software Reset Register                            */
  __IOM uint32_t CFG_CLR;                       /**< Configuration Register                             */
  __IOM uint32_t FRAMECFG_CLR;                  /**< Frame Configuration Register                       */
  __IOM uint32_t T0CFG_CLR;                     /**< Zero symbol configuration register                 */
  __IOM uint32_t T1CFG_CLR;                     /**< One symbol configuration register                  */
  __IOM uint32_t TSTOPCFG_CLR;                  /**< Stop sympbol configuration register                */
  __IOM uint32_t TRSTCFG_CLR;                   /**< Reset symbol configuration register                */
  __IOM uint32_t CMD_CLR;                       /**< Command Register                                   */
  __IOM uint32_t TXDATA_CLR;                    /**< Transmit Buffer Data Register                      */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  __IOM uint32_t IF_CLR;                        /**<  Interrupt Flag register                           */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable register                          */
  uint32_t       RESERVED2[1010U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_TGL;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_TGL;                     /**< Software Reset Register                            */
  __IOM uint32_t CFG_TGL;                       /**< Configuration Register                             */
  __IOM uint32_t FRAMECFG_TGL;                  /**< Frame Configuration Register                       */
  __IOM uint32_t T0CFG_TGL;                     /**< Zero symbol configuration register                 */
  __IOM uint32_t T1CFG_TGL;                     /**< One symbol configuration register                  */
  __IOM uint32_t TSTOPCFG_TGL;                  /**< Stop sympbol configuration register                */
  __IOM uint32_t TRSTCFG_TGL;                   /**< Reset symbol configuration register                */
  __IOM uint32_t CMD_TGL;                       /**< Command Register                                   */
  __IOM uint32_t TXDATA_TGL;                    /**< Transmit Buffer Data Register                      */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  __IOM uint32_t IF_TGL;                        /**<  Interrupt Flag register                           */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable register                          */
} PIXELRZ_TypeDef;
/** @} End of group SIMG301_PIXELRZ */

/**************************************************************************//**
 * @addtogroup SIMG301_PIXELRZ
 * @{
 * @defgroup SIMG301_PIXELRZ_BitFields PIXELRZ Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for PIXELRZ IPVERSION */
#define _PIXELRZ_IPVERSION_RESETVALUE           0x00000001UL                                /**< Default value for PIXELRZ_IPVERSION         */
#define _PIXELRZ_IPVERSION_MASK                 0xFFFFFFFFUL                                /**< Mask for PIXELRZ_IPVERSION                  */
#define _PIXELRZ_IPVERSION_IPVERSION_SHIFT      0                                           /**< Shift value for PIXELRZ_IPVERSION           */
#define _PIXELRZ_IPVERSION_IPVERSION_MASK       0xFFFFFFFFUL                                /**< Bit mask for PIXELRZ_IPVERSION              */
#define _PIXELRZ_IPVERSION_IPVERSION_DEFAULT    0x00000001UL                                /**< Mode DEFAULT for PIXELRZ_IPVERSION          */
#define PIXELRZ_IPVERSION_IPVERSION_DEFAULT     (_PIXELRZ_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for PIXELRZ_IPVERSION  */

/* Bit fields for PIXELRZ EN */
#define _PIXELRZ_EN_RESETVALUE                  0x00000000UL                            /**< Default value for PIXELRZ_EN                */
#define _PIXELRZ_EN_MASK                        0x00000003UL                            /**< Mask for PIXELRZ_EN                         */
#define PIXELRZ_EN_EN                           (0x1UL << 0)                            /**< module enable                               */
#define _PIXELRZ_EN_EN_SHIFT                    0                                       /**< Shift value for PIXELRZ_EN                  */
#define _PIXELRZ_EN_EN_MASK                     0x1UL                                   /**< Bit mask for PIXELRZ_EN                     */
#define _PIXELRZ_EN_EN_DEFAULT                  0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_EN                 */
#define _PIXELRZ_EN_EN_DISABLE                  0x00000000UL                            /**< Mode DISABLE for PIXELRZ_EN                 */
#define _PIXELRZ_EN_EN_ENABLE                   0x00000001UL                            /**< Mode ENABLE for PIXELRZ_EN                  */
#define PIXELRZ_EN_EN_DEFAULT                   (_PIXELRZ_EN_EN_DEFAULT << 0)           /**< Shifted mode DEFAULT for PIXELRZ_EN         */
#define PIXELRZ_EN_EN_DISABLE                   (_PIXELRZ_EN_EN_DISABLE << 0)           /**< Shifted mode DISABLE for PIXELRZ_EN         */
#define PIXELRZ_EN_EN_ENABLE                    (_PIXELRZ_EN_EN_ENABLE << 0)            /**< Shifted mode ENABLE for PIXELRZ_EN          */
#define PIXELRZ_EN_DISABLING                    (0x1UL << 1)                            /**< Disablment busy status                      */
#define _PIXELRZ_EN_DISABLING_SHIFT             1                                       /**< Shift value for PIXELRZ_DISABLING           */
#define _PIXELRZ_EN_DISABLING_MASK              0x2UL                                   /**< Bit mask for PIXELRZ_DISABLING              */
#define _PIXELRZ_EN_DISABLING_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_EN                 */
#define PIXELRZ_EN_DISABLING_DEFAULT            (_PIXELRZ_EN_DISABLING_DEFAULT << 1)    /**< Shifted mode DEFAULT for PIXELRZ_EN         */

/* Bit fields for PIXELRZ SWRST */
#define _PIXELRZ_SWRST_RESETVALUE               0x00000000UL                            /**< Default value for PIXELRZ_SWRST             */
#define _PIXELRZ_SWRST_MASK                     0x00000003UL                            /**< Mask for PIXELRZ_SWRST                      */
#define PIXELRZ_SWRST_SWRST                     (0x1UL << 0)                            /**< Software Reset bit                          */
#define _PIXELRZ_SWRST_SWRST_SHIFT              0                                       /**< Shift value for PIXELRZ_SWRST               */
#define _PIXELRZ_SWRST_SWRST_MASK               0x1UL                                   /**< Bit mask for PIXELRZ_SWRST                  */
#define _PIXELRZ_SWRST_SWRST_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_SWRST              */
#define PIXELRZ_SWRST_SWRST_DEFAULT             (_PIXELRZ_SWRST_SWRST_DEFAULT << 0)     /**< Shifted mode DEFAULT for PIXELRZ_SWRST      */
#define PIXELRZ_SWRST_RESETTING                 (0x1UL << 1)                            /**< Reset busy status                           */
#define _PIXELRZ_SWRST_RESETTING_SHIFT          1                                       /**< Shift value for PIXELRZ_RESETTING           */
#define _PIXELRZ_SWRST_RESETTING_MASK           0x2UL                                   /**< Bit mask for PIXELRZ_RESETTING              */
#define _PIXELRZ_SWRST_RESETTING_DEFAULT        0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_SWRST              */
#define PIXELRZ_SWRST_RESETTING_DEFAULT         (_PIXELRZ_SWRST_RESETTING_DEFAULT << 1) /**< Shifted mode DEFAULT for PIXELRZ_SWRST      */

/* Bit fields for PIXELRZ CFG */
#define _PIXELRZ_CFG_RESETVALUE                 0x00000000UL                            /**< Default value for PIXELRZ_CFG               */
#define _PIXELRZ_CFG_MASK                       0x007FFFFFUL                            /**< Mask for PIXELRZ_CFG                        */
#define PIXELRZ_CFG_OUTINV                      (0x1UL << 0)                            /**< Invert Output                               */
#define _PIXELRZ_CFG_OUTINV_SHIFT               0                                       /**< Shift value for PIXELRZ_OUTINV              */
#define _PIXELRZ_CFG_OUTINV_MASK                0x1UL                                   /**< Bit mask for PIXELRZ_OUTINV                 */
#define _PIXELRZ_CFG_OUTINV_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_OUTINV_NORMAL              0x00000000UL                            /**< Mode NORMAL for PIXELRZ_CFG                 */
#define _PIXELRZ_CFG_OUTINV_INVERT              0x00000001UL                            /**< Mode INVERT for PIXELRZ_CFG                 */
#define PIXELRZ_CFG_OUTINV_DEFAULT              (_PIXELRZ_CFG_OUTINV_DEFAULT << 0)      /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_OUTINV_NORMAL               (_PIXELRZ_CFG_OUTINV_NORMAL << 0)       /**< Shifted mode NORMAL for PIXELRZ_CFG         */
#define PIXELRZ_CFG_OUTINV_INVERT               (_PIXELRZ_CFG_OUTINV_INVERT << 0)       /**< Shifted mode INVERT for PIXELRZ_CFG         */
#define PIXELRZ_CFG_JITTEREN                    (0x1UL << 1)                            /**< Jitter Enable                               */
#define _PIXELRZ_CFG_JITTEREN_SHIFT             1                                       /**< Shift value for PIXELRZ_JITTEREN            */
#define _PIXELRZ_CFG_JITTEREN_MASK              0x2UL                                   /**< Bit mask for PIXELRZ_JITTEREN               */
#define _PIXELRZ_CFG_JITTEREN_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_JITTEREN_DISABLE           0x00000000UL                            /**< Mode DISABLE for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_JITTEREN_ENABLE            0x00000001UL                            /**< Mode ENABLE for PIXELRZ_CFG                 */
#define PIXELRZ_CFG_JITTEREN_DEFAULT            (_PIXELRZ_CFG_JITTEREN_DEFAULT << 1)    /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_JITTEREN_DISABLE            (_PIXELRZ_CFG_JITTEREN_DISABLE << 1)    /**< Shifted mode DISABLE for PIXELRZ_CFG        */
#define PIXELRZ_CFG_JITTEREN_ENABLE             (_PIXELRZ_CFG_JITTEREN_ENABLE << 1)     /**< Shifted mode ENABLE for PIXELRZ_CFG         */
#define _PIXELRZ_CFG_TXFWL_SHIFT                2                                       /**< Shift value for PIXELRZ_TXFWL               */
#define _PIXELRZ_CFG_TXFWL_MASK                 0x3CUL                                  /**< Bit mask for PIXELRZ_TXFWL                  */
#define _PIXELRZ_CFG_TXFWL_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define PIXELRZ_CFG_TXFWL_DEFAULT               (_PIXELRZ_CFG_TXFWL_DEFAULT << 2)       /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define _PIXELRZ_CFG_DIV_SHIFT                  6                                       /**< Shift value for PIXELRZ_DIV                 */
#define _PIXELRZ_CFG_DIV_MASK                   0xFFC0UL                                /**< Bit mask for PIXELRZ_DIV                    */
#define _PIXELRZ_CFG_DIV_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define PIXELRZ_CFG_DIV_DEFAULT                 (_PIXELRZ_CFG_DIV_DEFAULT << 6)         /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_MEMALIGN                    (0x1UL << 16)                           /**< Memory Alignment                            */
#define _PIXELRZ_CFG_MEMALIGN_SHIFT             16                                      /**< Shift value for PIXELRZ_MEMALIGN            */
#define _PIXELRZ_CFG_MEMALIGN_MASK              0x10000UL                               /**< Bit mask for PIXELRZ_MEMALIGN               */
#define _PIXELRZ_CFG_MEMALIGN_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_MEMALIGN_ALIGN8            0x00000000UL                            /**< Mode ALIGN8 for PIXELRZ_CFG                 */
#define _PIXELRZ_CFG_MEMALIGN_ALIGN32           0x00000001UL                            /**< Mode ALIGN32 for PIXELRZ_CFG                */
#define PIXELRZ_CFG_MEMALIGN_DEFAULT            (_PIXELRZ_CFG_MEMALIGN_DEFAULT << 16)   /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_MEMALIGN_ALIGN8             (_PIXELRZ_CFG_MEMALIGN_ALIGN8 << 16)    /**< Shifted mode ALIGN8 for PIXELRZ_CFG         */
#define PIXELRZ_CFG_MEMALIGN_ALIGN32            (_PIXELRZ_CFG_MEMALIGN_ALIGN32 << 16)   /**< Shifted mode ALIGN32 for PIXELRZ_CFG        */
#define PIXELRZ_CFG_MSBF                        (0x1UL << 17)                           /**< MSB First Enable                            */
#define _PIXELRZ_CFG_MSBF_SHIFT                 17                                      /**< Shift value for PIXELRZ_MSBF                */
#define _PIXELRZ_CFG_MSBF_MASK                  0x20000UL                               /**< Bit mask for PIXELRZ_MSBF                   */
#define _PIXELRZ_CFG_MSBF_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_MSBF_LSB                   0x00000000UL                            /**< Mode LSB for PIXELRZ_CFG                    */
#define _PIXELRZ_CFG_MSBF_MSB                   0x00000001UL                            /**< Mode MSB for PIXELRZ_CFG                    */
#define PIXELRZ_CFG_MSBF_DEFAULT                (_PIXELRZ_CFG_MSBF_DEFAULT << 17)       /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_MSBF_LSB                    (_PIXELRZ_CFG_MSBF_LSB << 17)           /**< Shifted mode LSB for PIXELRZ_CFG            */
#define PIXELRZ_CFG_MSBF_MSB                    (_PIXELRZ_CFG_MSBF_MSB << 17)           /**< Shifted mode MSB for PIXELRZ_CFG            */
#define PIXELRZ_CFG_MODE                        (0x1UL << 18)                           /**< Bus Mode                                    */
#define _PIXELRZ_CFG_MODE_SHIFT                 18                                      /**< Shift value for PIXELRZ_MODE                */
#define _PIXELRZ_CFG_MODE_MASK                  0x40000UL                               /**< Bit mask for PIXELRZ_MODE                   */
#define _PIXELRZ_CFG_MODE_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_MODE_SEQUENTIAL            0x00000000UL                            /**< Mode SEQUENTIAL for PIXELRZ_CFG             */
#define _PIXELRZ_CFG_MODE_PARALLEL              0x00000001UL                            /**< Mode PARALLEL for PIXELRZ_CFG               */
#define PIXELRZ_CFG_MODE_DEFAULT                (_PIXELRZ_CFG_MODE_DEFAULT << 18)       /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_MODE_SEQUENTIAL             (_PIXELRZ_CFG_MODE_SEQUENTIAL << 18)    /**< Shifted mode SEQUENTIAL for PIXELRZ_CFG     */
#define PIXELRZ_CFG_MODE_PARALLEL               (_PIXELRZ_CFG_MODE_PARALLEL << 18)      /**< Shifted mode PARALLEL for PIXELRZ_CFG       */
#define _PIXELRZ_CFG_TRIGSEL_SHIFT              19                                      /**< Shift value for PIXELRZ_TRIGSEL             */
#define _PIXELRZ_CFG_TRIGSEL_MASK               0x180000UL                              /**< Bit mask for PIXELRZ_TRIGSEL                */
#define _PIXELRZ_CFG_TRIGSEL_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_TRIGSEL_SW                 0x00000000UL                            /**< Mode SW for PIXELRZ_CFG                     */
#define _PIXELRZ_CFG_TRIGSEL_AUTO               0x00000001UL                            /**< Mode AUTO for PIXELRZ_CFG                   */
#define _PIXELRZ_CFG_TRIGSEL_PRS                0x00000002UL                            /**< Mode PRS for PIXELRZ_CFG                    */
#define PIXELRZ_CFG_TRIGSEL_DEFAULT             (_PIXELRZ_CFG_TRIGSEL_DEFAULT << 19)    /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_TRIGSEL_SW                  (_PIXELRZ_CFG_TRIGSEL_SW << 19)         /**< Shifted mode SW for PIXELRZ_CFG             */
#define PIXELRZ_CFG_TRIGSEL_AUTO                (_PIXELRZ_CFG_TRIGSEL_AUTO << 19)       /**< Shifted mode AUTO for PIXELRZ_CFG           */
#define PIXELRZ_CFG_TRIGSEL_PRS                 (_PIXELRZ_CFG_TRIGSEL_PRS << 19)        /**< Shifted mode PRS for PIXELRZ_CFG            */
#define PIXELRZ_CFG_STOPEN                      (0x1UL << 21)                           /**< STOP Symbol Enable                          */
#define _PIXELRZ_CFG_STOPEN_SHIFT               21                                      /**< Shift value for PIXELRZ_STOPEN              */
#define _PIXELRZ_CFG_STOPEN_MASK                0x200000UL                              /**< Bit mask for PIXELRZ_STOPEN                 */
#define _PIXELRZ_CFG_STOPEN_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_STOPEN_DISABLE             0x00000000UL                            /**< Mode DISABLE for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_STOPEN_ENABLE              0x00000001UL                            /**< Mode ENABLE for PIXELRZ_CFG                 */
#define PIXELRZ_CFG_STOPEN_DEFAULT              (_PIXELRZ_CFG_STOPEN_DEFAULT << 21)     /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_STOPEN_DISABLE              (_PIXELRZ_CFG_STOPEN_DISABLE << 21)     /**< Shifted mode DISABLE for PIXELRZ_CFG        */
#define PIXELRZ_CFG_STOPEN_ENABLE               (_PIXELRZ_CFG_STOPEN_ENABLE << 21)      /**< Shifted mode ENABLE for PIXELRZ_CFG         */
#define PIXELRZ_CFG_RESETEN                     (0x1UL << 22)                           /**< RESET Symbol Enable                         */
#define _PIXELRZ_CFG_RESETEN_SHIFT              22                                      /**< Shift value for PIXELRZ_RESETEN             */
#define _PIXELRZ_CFG_RESETEN_MASK               0x400000UL                              /**< Bit mask for PIXELRZ_RESETEN                */
#define _PIXELRZ_CFG_RESETEN_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_RESETEN_DISABLE            0x00000000UL                            /**< Mode DISABLE for PIXELRZ_CFG                */
#define _PIXELRZ_CFG_RESETEN_ENABLE             0x00000001UL                            /**< Mode ENABLE for PIXELRZ_CFG                 */
#define PIXELRZ_CFG_RESETEN_DEFAULT             (_PIXELRZ_CFG_RESETEN_DEFAULT << 22)    /**< Shifted mode DEFAULT for PIXELRZ_CFG        */
#define PIXELRZ_CFG_RESETEN_DISABLE             (_PIXELRZ_CFG_RESETEN_DISABLE << 22)    /**< Shifted mode DISABLE for PIXELRZ_CFG        */
#define PIXELRZ_CFG_RESETEN_ENABLE              (_PIXELRZ_CFG_RESETEN_ENABLE << 22)     /**< Shifted mode ENABLE for PIXELRZ_CFG         */

/* Bit fields for PIXELRZ FRAMECFG */
#define _PIXELRZ_FRAMECFG_RESETVALUE            0x00000000UL                                 /**< Default value for PIXELRZ_FRAMECFG          */
#define _PIXELRZ_FRAMECFG_MASK                  0x1FFFFFFFUL                                 /**< Mask for PIXELRZ_FRAMECFG                   */
#define _PIXELRZ_FRAMECFG_PIXELNUM_SHIFT        0                                            /**< Shift value for PIXELRZ_PIXELNUM            */
#define _PIXELRZ_FRAMECFG_PIXELNUM_MASK         0x3FFUL                                      /**< Bit mask for PIXELRZ_PIXELNUM               */
#define _PIXELRZ_FRAMECFG_PIXELNUM_DEFAULT      0x00000000UL                                 /**< Mode DEFAULT for PIXELRZ_FRAMECFG           */
#define PIXELRZ_FRAMECFG_PIXELNUM_DEFAULT       (_PIXELRZ_FRAMECFG_PIXELNUM_DEFAULT << 0)    /**< Shifted mode DEFAULT for PIXELRZ_FRAMECFG   */
#define _PIXELRZ_FRAMECFG_PIXELWIDTH_SHIFT      10                                           /**< Shift value for PIXELRZ_PIXELWIDTH          */
#define _PIXELRZ_FRAMECFG_PIXELWIDTH_MASK       0xFFC00UL                                    /**< Bit mask for PIXELRZ_PIXELWIDTH             */
#define _PIXELRZ_FRAMECFG_PIXELWIDTH_DEFAULT    0x00000000UL                                 /**< Mode DEFAULT for PIXELRZ_FRAMECFG           */
#define PIXELRZ_FRAMECFG_PIXELWIDTH_DEFAULT     (_PIXELRZ_FRAMECFG_PIXELWIDTH_DEFAULT << 10) /**< Shifted mode DEFAULT for PIXELRZ_FRAMECFG   */
#define _PIXELRZ_FRAMECFG_TRAILWIDTH_SHIFT      20                                           /**< Shift value for PIXELRZ_TRAILWIDTH          */
#define _PIXELRZ_FRAMECFG_TRAILWIDTH_MASK       0x1FF00000UL                                 /**< Bit mask for PIXELRZ_TRAILWIDTH             */
#define _PIXELRZ_FRAMECFG_TRAILWIDTH_DEFAULT    0x00000000UL                                 /**< Mode DEFAULT for PIXELRZ_FRAMECFG           */
#define PIXELRZ_FRAMECFG_TRAILWIDTH_DEFAULT     (_PIXELRZ_FRAMECFG_TRAILWIDTH_DEFAULT << 20) /**< Shifted mode DEFAULT for PIXELRZ_FRAMECFG   */

/* Bit fields for PIXELRZ T0CFG */
#define _PIXELRZ_T0CFG_RESETVALUE               0x00000000UL                            /**< Default value for PIXELRZ_T0CFG             */
#define _PIXELRZ_T0CFG_MASK                     0x003FFFFFUL                            /**< Mask for PIXELRZ_T0CFG                      */
#define _PIXELRZ_T0CFG_LOW_SHIFT                0                                       /**< Shift value for PIXELRZ_LOW                 */
#define _PIXELRZ_T0CFG_LOW_MASK                 0x7FFUL                                 /**< Bit mask for PIXELRZ_LOW                    */
#define _PIXELRZ_T0CFG_LOW_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_T0CFG              */
#define PIXELRZ_T0CFG_LOW_DEFAULT               (_PIXELRZ_T0CFG_LOW_DEFAULT << 0)       /**< Shifted mode DEFAULT for PIXELRZ_T0CFG      */
#define _PIXELRZ_T0CFG_HIGH_SHIFT               11                                      /**< Shift value for PIXELRZ_HIGH                */
#define _PIXELRZ_T0CFG_HIGH_MASK                0x3FF800UL                              /**< Bit mask for PIXELRZ_HIGH                   */
#define _PIXELRZ_T0CFG_HIGH_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_T0CFG              */
#define PIXELRZ_T0CFG_HIGH_DEFAULT              (_PIXELRZ_T0CFG_HIGH_DEFAULT << 11)     /**< Shifted mode DEFAULT for PIXELRZ_T0CFG      */

/* Bit fields for PIXELRZ T1CFG */
#define _PIXELRZ_T1CFG_RESETVALUE               0x00000000UL                            /**< Default value for PIXELRZ_T1CFG             */
#define _PIXELRZ_T1CFG_MASK                     0x003FFFFFUL                            /**< Mask for PIXELRZ_T1CFG                      */
#define _PIXELRZ_T1CFG_LOW_SHIFT                0                                       /**< Shift value for PIXELRZ_LOW                 */
#define _PIXELRZ_T1CFG_LOW_MASK                 0x7FFUL                                 /**< Bit mask for PIXELRZ_LOW                    */
#define _PIXELRZ_T1CFG_LOW_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_T1CFG              */
#define PIXELRZ_T1CFG_LOW_DEFAULT               (_PIXELRZ_T1CFG_LOW_DEFAULT << 0)       /**< Shifted mode DEFAULT for PIXELRZ_T1CFG      */
#define _PIXELRZ_T1CFG_HIGH_SHIFT               11                                      /**< Shift value for PIXELRZ_HIGH                */
#define _PIXELRZ_T1CFG_HIGH_MASK                0x3FF800UL                              /**< Bit mask for PIXELRZ_HIGH                   */
#define _PIXELRZ_T1CFG_HIGH_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_T1CFG              */
#define PIXELRZ_T1CFG_HIGH_DEFAULT              (_PIXELRZ_T1CFG_HIGH_DEFAULT << 11)     /**< Shifted mode DEFAULT for PIXELRZ_T1CFG      */

/* Bit fields for PIXELRZ TSTOPCFG */
#define _PIXELRZ_TSTOPCFG_RESETVALUE            0x00000000UL                            /**< Default value for PIXELRZ_TSTOPCFG          */
#define _PIXELRZ_TSTOPCFG_MASK                  0x003FFFFFUL                            /**< Mask for PIXELRZ_TSTOPCFG                   */
#define _PIXELRZ_TSTOPCFG_LOW_SHIFT             0                                       /**< Shift value for PIXELRZ_LOW                 */
#define _PIXELRZ_TSTOPCFG_LOW_MASK              0x7FFUL                                 /**< Bit mask for PIXELRZ_LOW                    */
#define _PIXELRZ_TSTOPCFG_LOW_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_TSTOPCFG           */
#define PIXELRZ_TSTOPCFG_LOW_DEFAULT            (_PIXELRZ_TSTOPCFG_LOW_DEFAULT << 0)    /**< Shifted mode DEFAULT for PIXELRZ_TSTOPCFG   */
#define _PIXELRZ_TSTOPCFG_HIGH_SHIFT            11                                      /**< Shift value for PIXELRZ_HIGH                */
#define _PIXELRZ_TSTOPCFG_HIGH_MASK             0x3FF800UL                              /**< Bit mask for PIXELRZ_HIGH                   */
#define _PIXELRZ_TSTOPCFG_HIGH_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_TSTOPCFG           */
#define PIXELRZ_TSTOPCFG_HIGH_DEFAULT           (_PIXELRZ_TSTOPCFG_HIGH_DEFAULT << 11)  /**< Shifted mode DEFAULT for PIXELRZ_TSTOPCFG   */

/* Bit fields for PIXELRZ TRSTCFG */
#define _PIXELRZ_TRSTCFG_RESETVALUE             0x00000000UL                            /**< Default value for PIXELRZ_TRSTCFG           */
#define _PIXELRZ_TRSTCFG_MASK                   0xFFFFFFFFUL                            /**< Mask for PIXELRZ_TRSTCFG                    */
#define _PIXELRZ_TRSTCFG_LOW_SHIFT              0                                       /**< Shift value for PIXELRZ_LOW                 */
#define _PIXELRZ_TRSTCFG_LOW_MASK               0xFFFFUL                                /**< Bit mask for PIXELRZ_LOW                    */
#define _PIXELRZ_TRSTCFG_LOW_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_TRSTCFG            */
#define PIXELRZ_TRSTCFG_LOW_DEFAULT             (_PIXELRZ_TRSTCFG_LOW_DEFAULT << 0)     /**< Shifted mode DEFAULT for PIXELRZ_TRSTCFG    */
#define _PIXELRZ_TRSTCFG_HIGH_SHIFT             16                                      /**< Shift value for PIXELRZ_HIGH                */
#define _PIXELRZ_TRSTCFG_HIGH_MASK              0xFFFF0000UL                            /**< Bit mask for PIXELRZ_HIGH                   */
#define _PIXELRZ_TRSTCFG_HIGH_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_TRSTCFG            */
#define PIXELRZ_TRSTCFG_HIGH_DEFAULT            (_PIXELRZ_TRSTCFG_HIGH_DEFAULT << 16)   /**< Shifted mode DEFAULT for PIXELRZ_TRSTCFG    */

/* Bit fields for PIXELRZ CMD */
#define _PIXELRZ_CMD_RESETVALUE                 0x00000000UL                            /**< Default value for PIXELRZ_CMD               */
#define _PIXELRZ_CMD_MASK                       0x00000007UL                            /**< Mask for PIXELRZ_CMD                        */
#define PIXELRZ_CMD_TXEN                        (0x1UL << 0)                            /**< Send start condition                        */
#define _PIXELRZ_CMD_TXEN_SHIFT                 0                                       /**< Shift value for PIXELRZ_TXEN                */
#define _PIXELRZ_CMD_TXEN_MASK                  0x1UL                                   /**< Bit mask for PIXELRZ_TXEN                   */
#define _PIXELRZ_CMD_TXEN_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CMD                */
#define PIXELRZ_CMD_TXEN_DEFAULT                (_PIXELRZ_CMD_TXEN_DEFAULT << 0)        /**< Shifted mode DEFAULT for PIXELRZ_CMD        */
#define PIXELRZ_CMD_TXDIS                       (0x1UL << 1)                            /**< Send stop condition                         */
#define _PIXELRZ_CMD_TXDIS_SHIFT                1                                       /**< Shift value for PIXELRZ_TXDIS               */
#define _PIXELRZ_CMD_TXDIS_MASK                 0x2UL                                   /**< Bit mask for PIXELRZ_TXDIS                  */
#define _PIXELRZ_CMD_TXDIS_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CMD                */
#define PIXELRZ_CMD_TXDIS_DEFAULT               (_PIXELRZ_CMD_TXDIS_DEFAULT << 1)       /**< Shifted mode DEFAULT for PIXELRZ_CMD        */
#define PIXELRZ_CMD_CLEARTX                     (0x1UL << 2)                            /**< Clear TX fifo                               */
#define _PIXELRZ_CMD_CLEARTX_SHIFT              2                                       /**< Shift value for PIXELRZ_CLEARTX             */
#define _PIXELRZ_CMD_CLEARTX_MASK               0x4UL                                   /**< Bit mask for PIXELRZ_CLEARTX                */
#define _PIXELRZ_CMD_CLEARTX_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_CMD                */
#define PIXELRZ_CMD_CLEARTX_DEFAULT             (_PIXELRZ_CMD_CLEARTX_DEFAULT << 2)     /**< Shifted mode DEFAULT for PIXELRZ_CMD        */

/* Bit fields for PIXELRZ TXDATA */
#define _PIXELRZ_TXDATA_RESETVALUE              0x00000000UL                            /**< Default value for PIXELRZ_TXDATA            */
#define _PIXELRZ_TXDATA_MASK                    0xFFFFFFFFUL                            /**< Mask for PIXELRZ_TXDATA                     */
#define _PIXELRZ_TXDATA_TXDATA_SHIFT            0                                       /**< Shift value for PIXELRZ_TXDATA              */
#define _PIXELRZ_TXDATA_TXDATA_MASK             0xFFFFFFFFUL                            /**< Bit mask for PIXELRZ_TXDATA                 */
#define _PIXELRZ_TXDATA_TXDATA_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_TXDATA             */
#define PIXELRZ_TXDATA_TXDATA_DEFAULT           (_PIXELRZ_TXDATA_TXDATA_DEFAULT << 0)   /**< Shifted mode DEFAULT for PIXELRZ_TXDATA     */

/* Bit fields for PIXELRZ STATUS */
#define _PIXELRZ_STATUS_RESETVALUE              0x00000000UL                                /**< Default value for PIXELRZ_STATUS            */
#define _PIXELRZ_STATUS_MASK                    0x003FFFFFUL                                /**< Mask for PIXELRZ_STATUS                     */
#define PIXELRZ_STATUS_TXC                      (0x1UL << 0)                                /**< TX Complete                                 */
#define _PIXELRZ_STATUS_TXC_SHIFT               0                                           /**< Shift value for PIXELRZ_TXC                 */
#define _PIXELRZ_STATUS_TXC_MASK                0x1UL                                       /**< Bit mask for PIXELRZ_TXC                    */
#define _PIXELRZ_STATUS_TXC_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_TXC_DEFAULT              (_PIXELRZ_STATUS_TXC_DEFAULT << 0)          /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_TXFL                     (0x1UL << 1)                                /**< TX FIFO Watermark                           */
#define _PIXELRZ_STATUS_TXFL_SHIFT              1                                           /**< Shift value for PIXELRZ_TXFL                */
#define _PIXELRZ_STATUS_TXFL_MASK               0x2UL                                       /**< Bit mask for PIXELRZ_TXFL                   */
#define _PIXELRZ_STATUS_TXFL_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_TXFL_DEFAULT             (_PIXELRZ_STATUS_TXFL_DEFAULT << 1)         /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_TXRUNNING                (0x1UL << 2)                                /**< TX in Progress                              */
#define _PIXELRZ_STATUS_TXRUNNING_SHIFT         2                                           /**< Shift value for PIXELRZ_TXRUNNING           */
#define _PIXELRZ_STATUS_TXRUNNING_MASK          0x4UL                                       /**< Bit mask for PIXELRZ_TXRUNNING              */
#define _PIXELRZ_STATUS_TXRUNNING_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_TXRUNNING_DEFAULT        (_PIXELRZ_STATUS_TXRUNNING_DEFAULT << 2)    /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_TXFULL                   (0x1UL << 3)                                /**< TX FIFO full                                */
#define _PIXELRZ_STATUS_TXFULL_SHIFT            3                                           /**< Shift value for PIXELRZ_TXFULL              */
#define _PIXELRZ_STATUS_TXFULL_MASK             0x8UL                                       /**< Bit mask for PIXELRZ_TXFULL                 */
#define _PIXELRZ_STATUS_TXFULL_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_TXFULL_DEFAULT           (_PIXELRZ_STATUS_TXFULL_DEFAULT << 3)       /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_TXIDLE                   (0x1UL << 4)                                /**< Transmitter Idle                            */
#define _PIXELRZ_STATUS_TXIDLE_SHIFT            4                                           /**< Shift value for PIXELRZ_TXIDLE              */
#define _PIXELRZ_STATUS_TXIDLE_MASK             0x10UL                                      /**< Bit mask for PIXELRZ_TXIDLE                 */
#define _PIXELRZ_STATUS_TXIDLE_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_TXIDLE_DEFAULT           (_PIXELRZ_STATUS_TXIDLE_DEFAULT << 4)       /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define _PIXELRZ_STATUS_TXFCNT_SHIFT            5                                           /**< Shift value for PIXELRZ_TXFCNT              */
#define _PIXELRZ_STATUS_TXFCNT_MASK             0x1E0UL                                     /**< Bit mask for PIXELRZ_TXFCNT                 */
#define _PIXELRZ_STATUS_TXFCNT_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_TXFCNT_DEFAULT           (_PIXELRZ_STATUS_TXFCNT_DEFAULT << 5)       /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_SYNCBUSY                 (0x1UL << 9)                                /**< Sync Busy                                   */
#define _PIXELRZ_STATUS_SYNCBUSY_SHIFT          9                                           /**< Shift value for PIXELRZ_SYNCBUSY            */
#define _PIXELRZ_STATUS_SYNCBUSY_MASK           0x200UL                                     /**< Bit mask for PIXELRZ_SYNCBUSY               */
#define _PIXELRZ_STATUS_SYNCBUSY_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_SYNCBUSY_DEFAULT         (_PIXELRZ_STATUS_SYNCBUSY_DEFAULT << 9)     /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_CLEARTXBUSY              (0x1UL << 10)                               /**< Clear TX Command Busy                       */
#define _PIXELRZ_STATUS_CLEARTXBUSY_SHIFT       10                                          /**< Shift value for PIXELRZ_CLEARTXBUSY         */
#define _PIXELRZ_STATUS_CLEARTXBUSY_MASK        0x400UL                                     /**< Bit mask for PIXELRZ_CLEARTXBUSY            */
#define _PIXELRZ_STATUS_CLEARTXBUSY_DEFAULT     0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_CLEARTXBUSY_DEFAULT      (_PIXELRZ_STATUS_CLEARTXBUSY_DEFAULT << 10) /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define _PIXELRZ_STATUS_PIXELCNT_SHIFT          11                                          /**< Shift value for PIXELRZ_PIXELCNT            */
#define _PIXELRZ_STATUS_PIXELCNT_MASK           0x1FF800UL                                  /**< Bit mask for PIXELRZ_PIXELCNT               */
#define _PIXELRZ_STATUS_PIXELCNT_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_PIXELCNT_DEFAULT         (_PIXELRZ_STATUS_PIXELCNT_DEFAULT << 11)    /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */
#define PIXELRZ_STATUS_FRAMEDONE                (0x1UL << 21)                               /**< Frame Done                                  */
#define _PIXELRZ_STATUS_FRAMEDONE_SHIFT         21                                          /**< Shift value for PIXELRZ_FRAMEDONE           */
#define _PIXELRZ_STATUS_FRAMEDONE_MASK          0x200000UL                                  /**< Bit mask for PIXELRZ_FRAMEDONE              */
#define _PIXELRZ_STATUS_FRAMEDONE_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for PIXELRZ_STATUS             */
#define PIXELRZ_STATUS_FRAMEDONE_DEFAULT        (_PIXELRZ_STATUS_FRAMEDONE_DEFAULT << 21)   /**< Shifted mode DEFAULT for PIXELRZ_STATUS     */

/* Bit fields for PIXELRZ IF */
#define _PIXELRZ_IF_RESETVALUE                  0x00000000UL                            /**< Default value for PIXELRZ_IF                */
#define _PIXELRZ_IF_MASK                        0x0000000FUL                            /**< Mask for PIXELRZ_IF                         */
#define PIXELRZ_IF_TXC                          (0x1UL << 0)                            /**< Transmission complete                       */
#define _PIXELRZ_IF_TXC_SHIFT                   0                                       /**< Shift value for PIXELRZ_TXC                 */
#define _PIXELRZ_IF_TXC_MASK                    0x1UL                                   /**< Bit mask for PIXELRZ_TXC                    */
#define _PIXELRZ_IF_TXC_DEFAULT                 0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IF                 */
#define PIXELRZ_IF_TXC_DEFAULT                  (_PIXELRZ_IF_TXC_DEFAULT << 0)          /**< Shifted mode DEFAULT for PIXELRZ_IF         */
#define PIXELRZ_IF_TXFL                         (0x1UL << 1)                            /**< Reached watermark                           */
#define _PIXELRZ_IF_TXFL_SHIFT                  1                                       /**< Shift value for PIXELRZ_TXFL                */
#define _PIXELRZ_IF_TXFL_MASK                   0x2UL                                   /**< Bit mask for PIXELRZ_TXFL                   */
#define _PIXELRZ_IF_TXFL_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IF                 */
#define PIXELRZ_IF_TXFL_DEFAULT                 (_PIXELRZ_IF_TXFL_DEFAULT << 1)         /**< Shifted mode DEFAULT for PIXELRZ_IF         */
#define PIXELRZ_IF_TXFOF                        (0x1UL << 2)                            /**< Overflow                                    */
#define _PIXELRZ_IF_TXFOF_SHIFT                 2                                       /**< Shift value for PIXELRZ_TXFOF               */
#define _PIXELRZ_IF_TXFOF_MASK                  0x4UL                                   /**< Bit mask for PIXELRZ_TXFOF                  */
#define _PIXELRZ_IF_TXFOF_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IF                 */
#define PIXELRZ_IF_TXFOF_DEFAULT                (_PIXELRZ_IF_TXFOF_DEFAULT << 2)        /**< Shifted mode DEFAULT for PIXELRZ_IF         */
#define PIXELRZ_IF_TXFUF                        (0x1UL << 3)                            /**< Underflow                                   */
#define _PIXELRZ_IF_TXFUF_SHIFT                 3                                       /**< Shift value for PIXELRZ_TXFUF               */
#define _PIXELRZ_IF_TXFUF_MASK                  0x8UL                                   /**< Bit mask for PIXELRZ_TXFUF                  */
#define _PIXELRZ_IF_TXFUF_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IF                 */
#define PIXELRZ_IF_TXFUF_DEFAULT                (_PIXELRZ_IF_TXFUF_DEFAULT << 3)        /**< Shifted mode DEFAULT for PIXELRZ_IF         */

/* Bit fields for PIXELRZ IEN */
#define _PIXELRZ_IEN_RESETVALUE                 0x00000000UL                            /**< Default value for PIXELRZ_IEN               */
#define _PIXELRZ_IEN_MASK                       0x0000000FUL                            /**< Mask for PIXELRZ_IEN                        */
#define PIXELRZ_IEN_TXC                         (0x1UL << 0)                            /**< Transmission complete interrupt enable      */
#define _PIXELRZ_IEN_TXC_SHIFT                  0                                       /**< Shift value for PIXELRZ_TXC                 */
#define _PIXELRZ_IEN_TXC_MASK                   0x1UL                                   /**< Bit mask for PIXELRZ_TXC                    */
#define _PIXELRZ_IEN_TXC_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IEN                */
#define PIXELRZ_IEN_TXC_DEFAULT                 (_PIXELRZ_IEN_TXC_DEFAULT << 0)         /**< Shifted mode DEFAULT for PIXELRZ_IEN        */
#define PIXELRZ_IEN_TXFL                        (0x1UL << 1)                            /**< Reached watermark interrupt enable          */
#define _PIXELRZ_IEN_TXFL_SHIFT                 1                                       /**< Shift value for PIXELRZ_TXFL                */
#define _PIXELRZ_IEN_TXFL_MASK                  0x2UL                                   /**< Bit mask for PIXELRZ_TXFL                   */
#define _PIXELRZ_IEN_TXFL_DEFAULT               0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IEN                */
#define PIXELRZ_IEN_TXFL_DEFAULT                (_PIXELRZ_IEN_TXFL_DEFAULT << 1)        /**< Shifted mode DEFAULT for PIXELRZ_IEN        */
#define PIXELRZ_IEN_TXFOF                       (0x1UL << 2)                            /**< Overflow interrupt enable                   */
#define _PIXELRZ_IEN_TXFOF_SHIFT                2                                       /**< Shift value for PIXELRZ_TXFOF               */
#define _PIXELRZ_IEN_TXFOF_MASK                 0x4UL                                   /**< Bit mask for PIXELRZ_TXFOF                  */
#define _PIXELRZ_IEN_TXFOF_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IEN                */
#define PIXELRZ_IEN_TXFOF_DEFAULT               (_PIXELRZ_IEN_TXFOF_DEFAULT << 2)       /**< Shifted mode DEFAULT for PIXELRZ_IEN        */
#define PIXELRZ_IEN_TXFUF                       (0x1UL << 3)                            /**< Underflow interrupt enable                  */
#define _PIXELRZ_IEN_TXFUF_SHIFT                3                                       /**< Shift value for PIXELRZ_TXFUF               */
#define _PIXELRZ_IEN_TXFUF_MASK                 0x8UL                                   /**< Bit mask for PIXELRZ_TXFUF                  */
#define _PIXELRZ_IEN_TXFUF_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for PIXELRZ_IEN                */
#define PIXELRZ_IEN_TXFUF_DEFAULT               (_PIXELRZ_IEN_TXFUF_DEFAULT << 3)       /**< Shifted mode DEFAULT for PIXELRZ_IEN        */

/** @} End of group SIMG301_PIXELRZ_BitFields */
/** @} End of group SIMG301_PIXELRZ */
/** @} End of group Parts */

#endif // SIMG301_PIXELRZ_H
