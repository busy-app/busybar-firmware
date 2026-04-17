/**************************************************************************//**
 * @file
 * @brief SIMG301 SMU register and bit field definitions
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
#ifndef SIMG301_SMU_H
#define SIMG301_SMU_H
#define SMU_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_SMU SMU
 * @{
 * @brief SIMG301 SMU Register Declaration.
 *****************************************************************************/

/** SMU Register Declaration. */
typedef struct smu_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP Version                                         */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  __IOM uint32_t LOCK;                          /**< Lock Register                                      */
  __IOM uint32_t IF;                            /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN;                           /**< Interrupt Enable Register                          */
  uint32_t       RESERVED0[3U];                 /**< Reserved for future use                            */
  __IOM uint32_t M33CTRL;                       /**< M33 Control Settings                               */
  uint32_t       RESERVED1[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t PPUPATD0;                      /**< PPU Privileged Access                              */
  __IOM uint32_t PPUPATD1;                      /**< PPU Privileged Access                              */
  uint32_t       RESERVED2[6U];                 /**< Reserved for future use                            */
  __IOM uint32_t PPUSATD0;                      /**< PPU Secure Access                                  */
  __IOM uint32_t PPUSATD1;                      /**< PPU Secure Access                                  */
  uint32_t       RESERVED3[50U];                /**< Reserved for future use                            */
  __IOM uint32_t PPURSPERRD;                    /**< PPU Respond with Error                             */
  uint32_t       RESERVED4[3U];                 /**< Reserved for future use                            */
  __IM uint32_t  PPUFS;                         /**< PPU Fault Status                                   */
  __IM uint32_t  PPUFSADDR;                     /**< PPU Fault Status Address                           */
  uint32_t       RESERVED5[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t BMPUPATD0;                     /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED6[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t BMPUSATD0;                     /**< BMPU Secure Attribute                              */
  uint32_t       RESERVED7[7U];                 /**< Reserved for future use                            */
  __IOM uint32_t BMPURSPERRD0;                  /**< BMPU Respond with Error                            */
  uint32_t       RESERVED8[47U];                /**< Reserved for future use                            */
  __IM uint32_t  BMPUFS;                        /**< BMPU Fault Status                                  */
  __IM uint32_t  BMPUFSADDR;                    /**< BMPU Fault Status Address                          */
  uint32_t       RESERVED9[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t MSPUDMEMREGIONSIZE;            /**< DMEM MSPU region size                              */
  __IOM uint32_t MSPUNVMREGIONSIZE;             /**< NVM MSPU region size                               */
  __IOM uint32_t MSPUDMEMRSPERRD;               /**< DMEM MSPU Respond with Error                       */
  __IOM uint32_t MSPUNVMRSPERRD;                /**< NVM MSPU Respond with Error                        */
  __IOM uint32_t MSPUDMEMNSREGIONFLAG0;         /**< DMEM MSPU non-secure region flags                  */
  __IOM uint32_t MSPUNVMNSREGIONFLAG0;          /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG1;          /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG2;          /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG3;          /**< NVM MSPU non-secure region flags                   */
  uint32_t       RESERVED10[55U];               /**< Reserved for future use                            */
  __IM uint32_t  MSPUDMEMFSADDR;                /**< DMEM MSPU Fault Status Address                     */
  __IM uint32_t  MSPUNVMFSADDR;                 /**< NVM MSPU Fault Status Address                      */
  uint32_t       RESERVED11[2U];                /**< Reserved for future use                            */
  __IOM uint32_t ESAURTYPES;                    /**< ESAU Region Types                                  */
  uint32_t       RESERVED12[4U];                /**< Reserved for future use                            */
  uint32_t       RESERVED13[799U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP Version                                         */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  __IOM uint32_t LOCK_SET;                      /**< Lock Register                                      */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED14[3U];                /**< Reserved for future use                            */
  __IOM uint32_t M33CTRL_SET;                   /**< M33 Control Settings                               */
  uint32_t       RESERVED15[7U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUPATD0_SET;                  /**< PPU Privileged Access                              */
  __IOM uint32_t PPUPATD1_SET;                  /**< PPU Privileged Access                              */
  uint32_t       RESERVED16[6U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUSATD0_SET;                  /**< PPU Secure Access                                  */
  __IOM uint32_t PPUSATD1_SET;                  /**< PPU Secure Access                                  */
  uint32_t       RESERVED17[50U];               /**< Reserved for future use                            */
  __IOM uint32_t PPURSPERRD_SET;                /**< PPU Respond with Error                             */
  uint32_t       RESERVED18[3U];                /**< Reserved for future use                            */
  __IM uint32_t  PPUFS_SET;                     /**< PPU Fault Status                                   */
  __IM uint32_t  PPUFSADDR_SET;                 /**< PPU Fault Status Address                           */
  uint32_t       RESERVED19[2U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUPATD0_SET;                 /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED20[7U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUSATD0_SET;                 /**< BMPU Secure Attribute                              */
  uint32_t       RESERVED21[7U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPURSPERRD0_SET;              /**< BMPU Respond with Error                            */
  uint32_t       RESERVED22[47U];               /**< Reserved for future use                            */
  __IM uint32_t  BMPUFS_SET;                    /**< BMPU Fault Status                                  */
  __IM uint32_t  BMPUFSADDR_SET;                /**< BMPU Fault Status Address                          */
  uint32_t       RESERVED23[2U];                /**< Reserved for future use                            */
  __IOM uint32_t MSPUDMEMREGIONSIZE_SET;        /**< DMEM MSPU region size                              */
  __IOM uint32_t MSPUNVMREGIONSIZE_SET;         /**< NVM MSPU region size                               */
  __IOM uint32_t MSPUDMEMRSPERRD_SET;           /**< DMEM MSPU Respond with Error                       */
  __IOM uint32_t MSPUNVMRSPERRD_SET;            /**< NVM MSPU Respond with Error                        */
  __IOM uint32_t MSPUDMEMNSREGIONFLAG0_SET;     /**< DMEM MSPU non-secure region flags                  */
  __IOM uint32_t MSPUNVMNSREGIONFLAG0_SET;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG1_SET;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG2_SET;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG3_SET;      /**< NVM MSPU non-secure region flags                   */
  uint32_t       RESERVED24[55U];               /**< Reserved for future use                            */
  __IM uint32_t  MSPUDMEMFSADDR_SET;            /**< DMEM MSPU Fault Status Address                     */
  __IM uint32_t  MSPUNVMFSADDR_SET;             /**< NVM MSPU Fault Status Address                      */
  uint32_t       RESERVED25[2U];                /**< Reserved for future use                            */
  __IOM uint32_t ESAURTYPES_SET;                /**< ESAU Region Types                                  */
  uint32_t       RESERVED26[4U];                /**< Reserved for future use                            */
  uint32_t       RESERVED27[799U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP Version                                         */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  __IOM uint32_t LOCK_CLR;                      /**< Lock Register                                      */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED28[3U];                /**< Reserved for future use                            */
  __IOM uint32_t M33CTRL_CLR;                   /**< M33 Control Settings                               */
  uint32_t       RESERVED29[7U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUPATD0_CLR;                  /**< PPU Privileged Access                              */
  __IOM uint32_t PPUPATD1_CLR;                  /**< PPU Privileged Access                              */
  uint32_t       RESERVED30[6U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUSATD0_CLR;                  /**< PPU Secure Access                                  */
  __IOM uint32_t PPUSATD1_CLR;                  /**< PPU Secure Access                                  */
  uint32_t       RESERVED31[50U];               /**< Reserved for future use                            */
  __IOM uint32_t PPURSPERRD_CLR;                /**< PPU Respond with Error                             */
  uint32_t       RESERVED32[3U];                /**< Reserved for future use                            */
  __IM uint32_t  PPUFS_CLR;                     /**< PPU Fault Status                                   */
  __IM uint32_t  PPUFSADDR_CLR;                 /**< PPU Fault Status Address                           */
  uint32_t       RESERVED33[2U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUPATD0_CLR;                 /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED34[7U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUSATD0_CLR;                 /**< BMPU Secure Attribute                              */
  uint32_t       RESERVED35[7U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPURSPERRD0_CLR;              /**< BMPU Respond with Error                            */
  uint32_t       RESERVED36[47U];               /**< Reserved for future use                            */
  __IM uint32_t  BMPUFS_CLR;                    /**< BMPU Fault Status                                  */
  __IM uint32_t  BMPUFSADDR_CLR;                /**< BMPU Fault Status Address                          */
  uint32_t       RESERVED37[2U];                /**< Reserved for future use                            */
  __IOM uint32_t MSPUDMEMREGIONSIZE_CLR;        /**< DMEM MSPU region size                              */
  __IOM uint32_t MSPUNVMREGIONSIZE_CLR;         /**< NVM MSPU region size                               */
  __IOM uint32_t MSPUDMEMRSPERRD_CLR;           /**< DMEM MSPU Respond with Error                       */
  __IOM uint32_t MSPUNVMRSPERRD_CLR;            /**< NVM MSPU Respond with Error                        */
  __IOM uint32_t MSPUDMEMNSREGIONFLAG0_CLR;     /**< DMEM MSPU non-secure region flags                  */
  __IOM uint32_t MSPUNVMNSREGIONFLAG0_CLR;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG1_CLR;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG2_CLR;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG3_CLR;      /**< NVM MSPU non-secure region flags                   */
  uint32_t       RESERVED38[55U];               /**< Reserved for future use                            */
  __IM uint32_t  MSPUDMEMFSADDR_CLR;            /**< DMEM MSPU Fault Status Address                     */
  __IM uint32_t  MSPUNVMFSADDR_CLR;             /**< NVM MSPU Fault Status Address                      */
  uint32_t       RESERVED39[2U];                /**< Reserved for future use                            */
  __IOM uint32_t ESAURTYPES_CLR;                /**< ESAU Region Types                                  */
  uint32_t       RESERVED40[4U];                /**< Reserved for future use                            */
  uint32_t       RESERVED41[799U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP Version                                         */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  __IOM uint32_t LOCK_TGL;                      /**< Lock Register                                      */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable Register                          */
  uint32_t       RESERVED42[3U];                /**< Reserved for future use                            */
  __IOM uint32_t M33CTRL_TGL;                   /**< M33 Control Settings                               */
  uint32_t       RESERVED43[7U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUPATD0_TGL;                  /**< PPU Privileged Access                              */
  __IOM uint32_t PPUPATD1_TGL;                  /**< PPU Privileged Access                              */
  uint32_t       RESERVED44[6U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUSATD0_TGL;                  /**< PPU Secure Access                                  */
  __IOM uint32_t PPUSATD1_TGL;                  /**< PPU Secure Access                                  */
  uint32_t       RESERVED45[50U];               /**< Reserved for future use                            */
  __IOM uint32_t PPURSPERRD_TGL;                /**< PPU Respond with Error                             */
  uint32_t       RESERVED46[3U];                /**< Reserved for future use                            */
  __IM uint32_t  PPUFS_TGL;                     /**< PPU Fault Status                                   */
  __IM uint32_t  PPUFSADDR_TGL;                 /**< PPU Fault Status Address                           */
  uint32_t       RESERVED47[2U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUPATD0_TGL;                 /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED48[7U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUSATD0_TGL;                 /**< BMPU Secure Attribute                              */
  uint32_t       RESERVED49[7U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPURSPERRD0_TGL;              /**< BMPU Respond with Error                            */
  uint32_t       RESERVED50[47U];               /**< Reserved for future use                            */
  __IM uint32_t  BMPUFS_TGL;                    /**< BMPU Fault Status                                  */
  __IM uint32_t  BMPUFSADDR_TGL;                /**< BMPU Fault Status Address                          */
  uint32_t       RESERVED51[2U];                /**< Reserved for future use                            */
  __IOM uint32_t MSPUDMEMREGIONSIZE_TGL;        /**< DMEM MSPU region size                              */
  __IOM uint32_t MSPUNVMREGIONSIZE_TGL;         /**< NVM MSPU region size                               */
  __IOM uint32_t MSPUDMEMRSPERRD_TGL;           /**< DMEM MSPU Respond with Error                       */
  __IOM uint32_t MSPUNVMRSPERRD_TGL;            /**< NVM MSPU Respond with Error                        */
  __IOM uint32_t MSPUDMEMNSREGIONFLAG0_TGL;     /**< DMEM MSPU non-secure region flags                  */
  __IOM uint32_t MSPUNVMNSREGIONFLAG0_TGL;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG1_TGL;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG2_TGL;      /**< NVM MSPU non-secure region flags                   */
  __IOM uint32_t MSPUNVMNSREGIONFLAG3_TGL;      /**< NVM MSPU non-secure region flags                   */
  uint32_t       RESERVED52[55U];               /**< Reserved for future use                            */
  __IM uint32_t  MSPUDMEMFSADDR_TGL;            /**< DMEM MSPU Fault Status Address                     */
  __IM uint32_t  MSPUNVMFSADDR_TGL;             /**< NVM MSPU Fault Status Address                      */
  uint32_t       RESERVED53[2U];                /**< Reserved for future use                            */
  __IOM uint32_t ESAURTYPES_TGL;                /**< ESAU Region Types                                  */
  uint32_t       RESERVED54[4U];                /**< Reserved for future use                            */
} SMU_TypeDef;
/** @} End of group SIMG301_SMU */

/**************************************************************************//**
 * @addtogroup SIMG301_SMU
 * @{
 * @defgroup SIMG301_SMU_BitFields SMU Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SMU IPVERSION */
#define _SMU_IPVERSION_RESETVALUE                          0x00000008UL                            /**< Default value for SMU_IPVERSION             */
#define _SMU_IPVERSION_MASK                                0xFFFFFFFFUL                            /**< Mask for SMU_IPVERSION                      */
#define _SMU_IPVERSION_IPVERSION_SHIFT                     0                                       /**< Shift value for SMU_IPVERSION               */
#define _SMU_IPVERSION_IPVERSION_MASK                      0xFFFFFFFFUL                            /**< Bit mask for SMU_IPVERSION                  */
#define _SMU_IPVERSION_IPVERSION_DEFAULT                   0x00000008UL                            /**< Mode DEFAULT for SMU_IPVERSION              */
#define SMU_IPVERSION_IPVERSION_DEFAULT                    (_SMU_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_IPVERSION      */

/* Bit fields for SMU STATUS */
#define _SMU_STATUS_RESETVALUE                             0x00000000UL                        /**< Default value for SMU_STATUS                */
#define _SMU_STATUS_MASK                                   0x00000003UL                        /**< Mask for SMU_STATUS                         */
#define SMU_STATUS_SMULOCK                                 (0x1UL << 0)                        /**< SMU Lock                                    */
#define _SMU_STATUS_SMULOCK_SHIFT                          0                                   /**< Shift value for SMU_SMULOCK                 */
#define _SMU_STATUS_SMULOCK_MASK                           0x1UL                               /**< Bit mask for SMU_SMULOCK                    */
#define _SMU_STATUS_SMULOCK_DEFAULT                        0x00000000UL                        /**< Mode DEFAULT for SMU_STATUS                 */
#define _SMU_STATUS_SMULOCK_UNLOCKED                       0x00000000UL                        /**< Mode UNLOCKED for SMU_STATUS                */
#define _SMU_STATUS_SMULOCK_LOCKED                         0x00000001UL                        /**< Mode LOCKED for SMU_STATUS                  */
#define SMU_STATUS_SMULOCK_DEFAULT                         (_SMU_STATUS_SMULOCK_DEFAULT << 0)  /**< Shifted mode DEFAULT for SMU_STATUS         */
#define SMU_STATUS_SMULOCK_UNLOCKED                        (_SMU_STATUS_SMULOCK_UNLOCKED << 0) /**< Shifted mode UNLOCKED for SMU_STATUS        */
#define SMU_STATUS_SMULOCK_LOCKED                          (_SMU_STATUS_SMULOCK_LOCKED << 0)   /**< Shifted mode LOCKED for SMU_STATUS          */

/* Bit fields for SMU LOCK */
#define _SMU_LOCK_RESETVALUE                               0x00000000UL                        /**< Default value for SMU_LOCK                  */
#define _SMU_LOCK_MASK                                     0x00FFFFFFUL                        /**< Mask for SMU_LOCK                           */
#define _SMU_LOCK_SMULOCKKEY_SHIFT                         0                                   /**< Shift value for SMU_SMULOCKKEY              */
#define _SMU_LOCK_SMULOCKKEY_MASK                          0xFFFFFFUL                          /**< Bit mask for SMU_SMULOCKKEY                 */
#define _SMU_LOCK_SMULOCKKEY_DEFAULT                       0x00000000UL                        /**< Mode DEFAULT for SMU_LOCK                   */
#define _SMU_LOCK_SMULOCKKEY_UNLOCK                        0x00ACCE55UL                        /**< Mode UNLOCK for SMU_LOCK                    */
#define SMU_LOCK_SMULOCKKEY_DEFAULT                        (_SMU_LOCK_SMULOCKKEY_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_LOCK           */
#define SMU_LOCK_SMULOCKKEY_UNLOCK                         (_SMU_LOCK_SMULOCKKEY_UNLOCK << 0)  /**< Shifted mode UNLOCK for SMU_LOCK            */

/* Bit fields for SMU IF */
#define _SMU_IF_RESETVALUE                                 0x00000000UL                    /**< Default value for SMU_IF                    */
#define _SMU_IF_MASK                                       0x00030055UL                    /**< Mask for SMU_IF                             */
#define SMU_IF_PPUPRIV                                     (0x1UL << 0)                    /**< PPU Privilege Interrupt Flag                */
#define _SMU_IF_PPUPRIV_SHIFT                              0                               /**< Shift value for SMU_PPUPRIV                 */
#define _SMU_IF_PPUPRIV_MASK                               0x1UL                           /**< Bit mask for SMU_PPUPRIV                    */
#define _SMU_IF_PPUPRIV_DEFAULT                            0x00000000UL                    /**< Mode DEFAULT for SMU_IF                     */
#define SMU_IF_PPUPRIV_DEFAULT                             (_SMU_IF_PPUPRIV_DEFAULT << 0)  /**< Shifted mode DEFAULT for SMU_IF             */
#define SMU_IF_PPUINST                                     (0x1UL << 2)                    /**< PPU Instruction Interrupt Flag              */
#define _SMU_IF_PPUINST_SHIFT                              2                               /**< Shift value for SMU_PPUINST                 */
#define _SMU_IF_PPUINST_MASK                               0x4UL                           /**< Bit mask for SMU_PPUINST                    */
#define _SMU_IF_PPUINST_DEFAULT                            0x00000000UL                    /**< Mode DEFAULT for SMU_IF                     */
#define SMU_IF_PPUINST_DEFAULT                             (_SMU_IF_PPUINST_DEFAULT << 2)  /**< Shifted mode DEFAULT for SMU_IF             */
#define SMU_IF_MSPU0                                       (0x1UL << 4)                    /**< DMEM MSPU Interrupt Flag                    */
#define _SMU_IF_MSPU0_SHIFT                                4                               /**< Shift value for SMU_MSPU0                   */
#define _SMU_IF_MSPU0_MASK                                 0x10UL                          /**< Bit mask for SMU_MSPU0                      */
#define _SMU_IF_MSPU0_DEFAULT                              0x00000000UL                    /**< Mode DEFAULT for SMU_IF                     */
#define SMU_IF_MSPU0_DEFAULT                               (_SMU_IF_MSPU0_DEFAULT << 4)    /**< Shifted mode DEFAULT for SMU_IF             */
#define SMU_IF_MSPU1                                       (0x1UL << 6)                    /**< NVM MSPU Interrupt Flag                     */
#define _SMU_IF_MSPU1_SHIFT                                6                               /**< Shift value for SMU_MSPU1                   */
#define _SMU_IF_MSPU1_MASK                                 0x40UL                          /**< Bit mask for SMU_MSPU1                      */
#define _SMU_IF_MSPU1_DEFAULT                              0x00000000UL                    /**< Mode DEFAULT for SMU_IF                     */
#define SMU_IF_MSPU1_DEFAULT                               (_SMU_IF_MSPU1_DEFAULT << 6)    /**< Shifted mode DEFAULT for SMU_IF             */
#define SMU_IF_PPUSEC                                      (0x1UL << 16)                   /**< PPU Security Interrupt Flag                 */
#define _SMU_IF_PPUSEC_SHIFT                               16                              /**< Shift value for SMU_PPUSEC                  */
#define _SMU_IF_PPUSEC_MASK                                0x10000UL                       /**< Bit mask for SMU_PPUSEC                     */
#define _SMU_IF_PPUSEC_DEFAULT                             0x00000000UL                    /**< Mode DEFAULT for SMU_IF                     */
#define SMU_IF_PPUSEC_DEFAULT                              (_SMU_IF_PPUSEC_DEFAULT << 16)  /**< Shifted mode DEFAULT for SMU_IF             */
#define SMU_IF_BMPUSEC                                     (0x1UL << 17)                   /**< BMPU Security Interrupt Flag                */
#define _SMU_IF_BMPUSEC_SHIFT                              17                              /**< Shift value for SMU_BMPUSEC                 */
#define _SMU_IF_BMPUSEC_MASK                               0x20000UL                       /**< Bit mask for SMU_BMPUSEC                    */
#define _SMU_IF_BMPUSEC_DEFAULT                            0x00000000UL                    /**< Mode DEFAULT for SMU_IF                     */
#define SMU_IF_BMPUSEC_DEFAULT                             (_SMU_IF_BMPUSEC_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_IF             */

/* Bit fields for SMU IEN */
#define _SMU_IEN_RESETVALUE                                0x00000000UL                     /**< Default value for SMU_IEN                   */
#define _SMU_IEN_MASK                                      0x00030055UL                     /**< Mask for SMU_IEN                            */
#define SMU_IEN_PPUPRIV                                    (0x1UL << 0)                     /**< PPU Privilege Interrupt Enable              */
#define _SMU_IEN_PPUPRIV_SHIFT                             0                                /**< Shift value for SMU_PPUPRIV                 */
#define _SMU_IEN_PPUPRIV_MASK                              0x1UL                            /**< Bit mask for SMU_PPUPRIV                    */
#define _SMU_IEN_PPUPRIV_DEFAULT                           0x00000000UL                     /**< Mode DEFAULT for SMU_IEN                    */
#define SMU_IEN_PPUPRIV_DEFAULT                            (_SMU_IEN_PPUPRIV_DEFAULT << 0)  /**< Shifted mode DEFAULT for SMU_IEN            */
#define SMU_IEN_PPUINST                                    (0x1UL << 2)                     /**< PPU Instruction Interrupt Enable            */
#define _SMU_IEN_PPUINST_SHIFT                             2                                /**< Shift value for SMU_PPUINST                 */
#define _SMU_IEN_PPUINST_MASK                              0x4UL                            /**< Bit mask for SMU_PPUINST                    */
#define _SMU_IEN_PPUINST_DEFAULT                           0x00000000UL                     /**< Mode DEFAULT for SMU_IEN                    */
#define SMU_IEN_PPUINST_DEFAULT                            (_SMU_IEN_PPUINST_DEFAULT << 2)  /**< Shifted mode DEFAULT for SMU_IEN            */
#define SMU_IEN_MSPU0                                      (0x1UL << 4)                     /**< DMEM MSPU Interrupt Enable                  */
#define _SMU_IEN_MSPU0_SHIFT                               4                                /**< Shift value for SMU_MSPU0                   */
#define _SMU_IEN_MSPU0_MASK                                0x10UL                           /**< Bit mask for SMU_MSPU0                      */
#define _SMU_IEN_MSPU0_DEFAULT                             0x00000000UL                     /**< Mode DEFAULT for SMU_IEN                    */
#define SMU_IEN_MSPU0_DEFAULT                              (_SMU_IEN_MSPU0_DEFAULT << 4)    /**< Shifted mode DEFAULT for SMU_IEN            */
#define SMU_IEN_MSPU1                                      (0x1UL << 6)                     /**< NVM MSPU Interrupt Enable                   */
#define _SMU_IEN_MSPU1_SHIFT                               6                                /**< Shift value for SMU_MSPU1                   */
#define _SMU_IEN_MSPU1_MASK                                0x40UL                           /**< Bit mask for SMU_MSPU1                      */
#define _SMU_IEN_MSPU1_DEFAULT                             0x00000000UL                     /**< Mode DEFAULT for SMU_IEN                    */
#define SMU_IEN_MSPU1_DEFAULT                              (_SMU_IEN_MSPU1_DEFAULT << 6)    /**< Shifted mode DEFAULT for SMU_IEN            */
#define SMU_IEN_PPUSEC                                     (0x1UL << 16)                    /**< PPU Security Interrupt Enable               */
#define _SMU_IEN_PPUSEC_SHIFT                              16                               /**< Shift value for SMU_PPUSEC                  */
#define _SMU_IEN_PPUSEC_MASK                               0x10000UL                        /**< Bit mask for SMU_PPUSEC                     */
#define _SMU_IEN_PPUSEC_DEFAULT                            0x00000000UL                     /**< Mode DEFAULT for SMU_IEN                    */
#define SMU_IEN_PPUSEC_DEFAULT                             (_SMU_IEN_PPUSEC_DEFAULT << 16)  /**< Shifted mode DEFAULT for SMU_IEN            */
#define SMU_IEN_BMPUSEC                                    (0x1UL << 17)                    /**< BMPU Security Interrupt Enable              */
#define _SMU_IEN_BMPUSEC_SHIFT                             17                               /**< Shift value for SMU_BMPUSEC                 */
#define _SMU_IEN_BMPUSEC_MASK                              0x20000UL                        /**< Bit mask for SMU_BMPUSEC                    */
#define _SMU_IEN_BMPUSEC_DEFAULT                           0x00000000UL                     /**< Mode DEFAULT for SMU_IEN                    */
#define SMU_IEN_BMPUSEC_DEFAULT                            (_SMU_IEN_BMPUSEC_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_IEN            */

/* Bit fields for SMU M33CTRL */
#define _SMU_M33CTRL_RESETVALUE                            0x00000000UL                             /**< Default value for SMU_M33CTRL               */
#define _SMU_M33CTRL_MASK                                  0x0000001FUL                             /**< Mask for SMU_M33CTRL                        */
#define SMU_M33CTRL_LOCKSVTAIRCR                           (0x1UL << 0)                             /**< Lock Secure VTAIRCR                         */
#define _SMU_M33CTRL_LOCKSVTAIRCR_SHIFT                    0                                        /**< Shift value for SMU_LOCKSVTAIRCR            */
#define _SMU_M33CTRL_LOCKSVTAIRCR_MASK                     0x1UL                                    /**< Bit mask for SMU_LOCKSVTAIRCR               */
#define _SMU_M33CTRL_LOCKSVTAIRCR_DEFAULT                  0x00000000UL                             /**< Mode DEFAULT for SMU_M33CTRL                */
#define SMU_M33CTRL_LOCKSVTAIRCR_DEFAULT                   (_SMU_M33CTRL_LOCKSVTAIRCR_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_M33CTRL        */
#define SMU_M33CTRL_LOCKNSVTOR                             (0x1UL << 1)                             /**< Lock Non-secure VTOR                        */
#define _SMU_M33CTRL_LOCKNSVTOR_SHIFT                      1                                        /**< Shift value for SMU_LOCKNSVTOR              */
#define _SMU_M33CTRL_LOCKNSVTOR_MASK                       0x2UL                                    /**< Bit mask for SMU_LOCKNSVTOR                 */
#define _SMU_M33CTRL_LOCKNSVTOR_DEFAULT                    0x00000000UL                             /**< Mode DEFAULT for SMU_M33CTRL                */
#define SMU_M33CTRL_LOCKNSVTOR_DEFAULT                     (_SMU_M33CTRL_LOCKNSVTOR_DEFAULT << 1)   /**< Shifted mode DEFAULT for SMU_M33CTRL        */
#define SMU_M33CTRL_LOCKSMPU                               (0x1UL << 2)                             /**< Lock Secure MPU configuration               */
#define _SMU_M33CTRL_LOCKSMPU_SHIFT                        2                                        /**< Shift value for SMU_LOCKSMPU                */
#define _SMU_M33CTRL_LOCKSMPU_MASK                         0x4UL                                    /**< Bit mask for SMU_LOCKSMPU                   */
#define _SMU_M33CTRL_LOCKSMPU_DEFAULT                      0x00000000UL                             /**< Mode DEFAULT for SMU_M33CTRL                */
#define SMU_M33CTRL_LOCKSMPU_DEFAULT                       (_SMU_M33CTRL_LOCKSMPU_DEFAULT << 2)     /**< Shifted mode DEFAULT for SMU_M33CTRL        */
#define SMU_M33CTRL_LOCKNSMPU                              (0x1UL << 3)                             /**< Lock Non-secure MPU configuration           */
#define _SMU_M33CTRL_LOCKNSMPU_SHIFT                       3                                        /**< Shift value for SMU_LOCKNSMPU               */
#define _SMU_M33CTRL_LOCKNSMPU_MASK                        0x8UL                                    /**< Bit mask for SMU_LOCKNSMPU                  */
#define _SMU_M33CTRL_LOCKNSMPU_DEFAULT                     0x00000000UL                             /**< Mode DEFAULT for SMU_M33CTRL                */
#define SMU_M33CTRL_LOCKNSMPU_DEFAULT                      (_SMU_M33CTRL_LOCKNSMPU_DEFAULT << 3)    /**< Shifted mode DEFAULT for SMU_M33CTRL        */
#define SMU_M33CTRL_LOCKSAU                                (0x1UL << 4)                             /**< Lock SAU                                    */
#define _SMU_M33CTRL_LOCKSAU_SHIFT                         4                                        /**< Shift value for SMU_LOCKSAU                 */
#define _SMU_M33CTRL_LOCKSAU_MASK                          0x10UL                                   /**< Bit mask for SMU_LOCKSAU                    */
#define _SMU_M33CTRL_LOCKSAU_DEFAULT                       0x00000000UL                             /**< Mode DEFAULT for SMU_M33CTRL                */
#define SMU_M33CTRL_LOCKSAU_DEFAULT                        (_SMU_M33CTRL_LOCKSAU_DEFAULT << 4)      /**< Shifted mode DEFAULT for SMU_M33CTRL        */

/* Bit fields for SMU PPUPATD0 */
#define _SMU_PPUPATD0_RESETVALUE                           0xFFFFFFFFUL                               /**< Default value for SMU_PPUPATD0              */
#define _SMU_PPUPATD0_MASK                                 0xFFFFFFFFUL                               /**< Mask for SMU_PPUPATD0                       */
#define SMU_PPUPATD0_SOCPLL0                               (0x1UL << 0)                               /**< SOCPLL0 Privileged Access                   */
#define _SMU_PPUPATD0_SOCPLL0_SHIFT                        0                                          /**< Shift value for SMU_SOCPLL0                 */
#define _SMU_PPUPATD0_SOCPLL0_MASK                         0x1UL                                      /**< Bit mask for SMU_SOCPLL0                    */
#define _SMU_PPUPATD0_SOCPLL0_DEFAULT                      0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_SOCPLL0_DEFAULT                       (_SMU_PPUPATD0_SOCPLL0_DEFAULT << 0)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_CMU                                   (0x1UL << 1)                               /**< CMU Privileged Access                       */
#define _SMU_PPUPATD0_CMU_SHIFT                            1                                          /**< Shift value for SMU_CMU                     */
#define _SMU_PPUPATD0_CMU_MASK                             0x2UL                                      /**< Bit mask for SMU_CMU                        */
#define _SMU_PPUPATD0_CMU_DEFAULT                          0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_CMU_DEFAULT                           (_SMU_PPUPATD0_CMU_DEFAULT << 1)           /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_DMEM                                  (0x1UL << 2)                               /**< DMEM Privileged Access                      */
#define _SMU_PPUPATD0_DMEM_SHIFT                           2                                          /**< Shift value for SMU_DMEM                    */
#define _SMU_PPUPATD0_DMEM_MASK                            0x4UL                                      /**< Bit mask for SMU_DMEM                       */
#define _SMU_PPUPATD0_DMEM_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_DMEM_DEFAULT                          (_SMU_PPUPATD0_DMEM_DEFAULT << 2)          /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_HFRCO0                                (0x1UL << 3)                               /**< HFRCO0 Privileged Access                    */
#define _SMU_PPUPATD0_HFRCO0_SHIFT                         3                                          /**< Shift value for SMU_HFRCO0                  */
#define _SMU_PPUPATD0_HFRCO0_MASK                          0x8UL                                      /**< Bit mask for SMU_HFRCO0                     */
#define _SMU_PPUPATD0_HFRCO0_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_HFRCO0_DEFAULT                        (_SMU_PPUPATD0_HFRCO0_DEFAULT << 3)        /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_LDMA0                                 (0x1UL << 4)                               /**< LDMA0 Privileged Access                     */
#define _SMU_PPUPATD0_LDMA0_SHIFT                          4                                          /**< Shift value for SMU_LDMA0                   */
#define _SMU_PPUPATD0_LDMA0_MASK                           0x10UL                                     /**< Bit mask for SMU_LDMA0                      */
#define _SMU_PPUPATD0_LDMA0_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_LDMA0_DEFAULT                         (_SMU_PPUPATD0_LDMA0_DEFAULT << 4)         /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_FSRCO                                 (0x1UL << 5)                               /**< FSRCO Privileged Access                     */
#define _SMU_PPUPATD0_FSRCO_SHIFT                          5                                          /**< Shift value for SMU_FSRCO                   */
#define _SMU_PPUPATD0_FSRCO_MASK                           0x20UL                                     /**< Bit mask for SMU_FSRCO                      */
#define _SMU_PPUPATD0_FSRCO_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_FSRCO_DEFAULT                         (_SMU_PPUPATD0_FSRCO_DEFAULT << 5)         /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_DPLL0                                 (0x1UL << 6)                               /**< DPLL0 Privileged Access                     */
#define _SMU_PPUPATD0_DPLL0_SHIFT                          6                                          /**< Shift value for SMU_DPLL0                   */
#define _SMU_PPUPATD0_DPLL0_MASK                           0x40UL                                     /**< Bit mask for SMU_DPLL0                      */
#define _SMU_PPUPATD0_DPLL0_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_DPLL0_DEFAULT                         (_SMU_PPUPATD0_DPLL0_DEFAULT << 6)         /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_LFXO                                  (0x1UL << 7)                               /**< LFXO Privileged Access                      */
#define _SMU_PPUPATD0_LFXO_SHIFT                           7                                          /**< Shift value for SMU_LFXO                    */
#define _SMU_PPUPATD0_LFXO_MASK                            0x80UL                                     /**< Bit mask for SMU_LFXO                       */
#define _SMU_PPUPATD0_LFXO_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_LFXO_DEFAULT                          (_SMU_PPUPATD0_LFXO_DEFAULT << 7)          /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_LFRCO                                 (0x1UL << 8)                               /**< LFRCO Privileged Access                     */
#define _SMU_PPUPATD0_LFRCO_SHIFT                          8                                          /**< Shift value for SMU_LFRCO                   */
#define _SMU_PPUPATD0_LFRCO_MASK                           0x100UL                                    /**< Bit mask for SMU_LFRCO                      */
#define _SMU_PPUPATD0_LFRCO_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_LFRCO_DEFAULT                         (_SMU_PPUPATD0_LFRCO_DEFAULT << 8)         /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_ULFRCO                                (0x1UL << 9)                               /**< ULFRCO Privileged Access                    */
#define _SMU_PPUPATD0_ULFRCO_SHIFT                         9                                          /**< Shift value for SMU_ULFRCO                  */
#define _SMU_PPUPATD0_ULFRCO_MASK                          0x200UL                                    /**< Bit mask for SMU_ULFRCO                     */
#define _SMU_PPUPATD0_ULFRCO_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_ULFRCO_DEFAULT                        (_SMU_PPUPATD0_ULFRCO_DEFAULT << 9)        /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_SEMAPHORE0                            (0x1UL << 10)                              /**< SEMAPHORE0 Privileged Access                */
#define _SMU_PPUPATD0_SEMAPHORE0_SHIFT                     10                                         /**< Shift value for SMU_SEMAPHORE0              */
#define _SMU_PPUPATD0_SEMAPHORE0_MASK                      0x400UL                                    /**< Bit mask for SMU_SEMAPHORE0                 */
#define _SMU_PPUPATD0_SEMAPHORE0_DEFAULT                   0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_SEMAPHORE0_DEFAULT                    (_SMU_PPUPATD0_SEMAPHORE0_DEFAULT << 10)   /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_SEMAPHORE1                            (0x1UL << 11)                              /**< SEMAPHORE1 Privileged Access                */
#define _SMU_PPUPATD0_SEMAPHORE1_SHIFT                     11                                         /**< Shift value for SMU_SEMAPHORE1              */
#define _SMU_PPUPATD0_SEMAPHORE1_MASK                      0x800UL                                    /**< Bit mask for SMU_SEMAPHORE1                 */
#define _SMU_PPUPATD0_SEMAPHORE1_DEFAULT                   0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_SEMAPHORE1_DEFAULT                    (_SMU_PPUPATD0_SEMAPHORE1_DEFAULT << 11)   /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_L1ICACHE0                             (0x1UL << 12)                              /**< L1ICACHE0 Privileged Access                 */
#define _SMU_PPUPATD0_L1ICACHE0_SHIFT                      12                                         /**< Shift value for SMU_L1ICACHE0               */
#define _SMU_PPUPATD0_L1ICACHE0_MASK                       0x1000UL                                   /**< Bit mask for SMU_L1ICACHE0                  */
#define _SMU_PPUPATD0_L1ICACHE0_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_L1ICACHE0_DEFAULT                     (_SMU_PPUPATD0_L1ICACHE0_DEFAULT << 12)    /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_PRS                                   (0x1UL << 13)                              /**< PRS Privileged Access                       */
#define _SMU_PPUPATD0_PRS_SHIFT                            13                                         /**< Shift value for SMU_PRS                     */
#define _SMU_PPUPATD0_PRS_MASK                             0x2000UL                                   /**< Bit mask for SMU_PRS                        */
#define _SMU_PPUPATD0_PRS_DEFAULT                          0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_PRS_DEFAULT                           (_SMU_PPUPATD0_PRS_DEFAULT << 13)          /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_GPIO                                  (0x1UL << 14)                              /**< GPIO Privileged Access                      */
#define _SMU_PPUPATD0_GPIO_SHIFT                           14                                         /**< Shift value for SMU_GPIO                    */
#define _SMU_PPUPATD0_GPIO_MASK                            0x4000UL                                   /**< Bit mask for SMU_GPIO                       */
#define _SMU_PPUPATD0_GPIO_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_GPIO_DEFAULT                          (_SMU_PPUPATD0_GPIO_DEFAULT << 14)         /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_LDMAXBAR0                             (0x1UL << 15)                              /**< LDMAXBAR0 Privileged Access                 */
#define _SMU_PPUPATD0_LDMAXBAR0_SHIFT                      15                                         /**< Shift value for SMU_LDMAXBAR0               */
#define _SMU_PPUPATD0_LDMAXBAR0_MASK                       0x8000UL                                   /**< Bit mask for SMU_LDMAXBAR0                  */
#define _SMU_PPUPATD0_LDMAXBAR0_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_LDMAXBAR0_DEFAULT                     (_SMU_PPUPATD0_LDMAXBAR0_DEFAULT << 15)    /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_TIMER0                                (0x1UL << 16)                              /**< TIMER0 Privileged Access                    */
#define _SMU_PPUPATD0_TIMER0_SHIFT                         16                                         /**< Shift value for SMU_TIMER0                  */
#define _SMU_PPUPATD0_TIMER0_MASK                          0x10000UL                                  /**< Bit mask for SMU_TIMER0                     */
#define _SMU_PPUPATD0_TIMER0_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_TIMER0_DEFAULT                        (_SMU_PPUPATD0_TIMER0_DEFAULT << 16)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_TIMER1                                (0x1UL << 17)                              /**< TIMER1 Privileged Access                    */
#define _SMU_PPUPATD0_TIMER1_SHIFT                         17                                         /**< Shift value for SMU_TIMER1                  */
#define _SMU_PPUPATD0_TIMER1_MASK                          0x20000UL                                  /**< Bit mask for SMU_TIMER1                     */
#define _SMU_PPUPATD0_TIMER1_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_TIMER1_DEFAULT                        (_SMU_PPUPATD0_TIMER1_DEFAULT << 17)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_TIMER2                                (0x1UL << 18)                              /**< TIMER2 Privileged Access                    */
#define _SMU_PPUPATD0_TIMER2_SHIFT                         18                                         /**< Shift value for SMU_TIMER2                  */
#define _SMU_PPUPATD0_TIMER2_MASK                          0x40000UL                                  /**< Bit mask for SMU_TIMER2                     */
#define _SMU_PPUPATD0_TIMER2_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_TIMER2_DEFAULT                        (_SMU_PPUPATD0_TIMER2_DEFAULT << 18)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_TIMER3                                (0x1UL << 19)                              /**< TIMER3 Privileged Access                    */
#define _SMU_PPUPATD0_TIMER3_SHIFT                         19                                         /**< Shift value for SMU_TIMER3                  */
#define _SMU_PPUPATD0_TIMER3_MASK                          0x80000UL                                  /**< Bit mask for SMU_TIMER3                     */
#define _SMU_PPUPATD0_TIMER3_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_TIMER3_DEFAULT                        (_SMU_PPUPATD0_TIMER3_DEFAULT << 19)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_ETAMPDET                              (0x1UL << 20)                              /**< ETAMPDET Privileged Access                  */
#define _SMU_PPUPATD0_ETAMPDET_SHIFT                       20                                         /**< Shift value for SMU_ETAMPDET                */
#define _SMU_PPUPATD0_ETAMPDET_MASK                        0x100000UL                                 /**< Bit mask for SMU_ETAMPDET                   */
#define _SMU_PPUPATD0_ETAMPDET_DEFAULT                     0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_ETAMPDET_DEFAULT                      (_SMU_PPUPATD0_ETAMPDET_DEFAULT << 20)     /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_L2ICACHE0                             (0x1UL << 21)                              /**< L2ICACHE0 Privileged Access                 */
#define _SMU_PPUPATD0_L2ICACHE0_SHIFT                      21                                         /**< Shift value for SMU_L2ICACHE0               */
#define _SMU_PPUPATD0_L2ICACHE0_MASK                       0x200000UL                                 /**< Bit mask for SMU_L2ICACHE0                  */
#define _SMU_PPUPATD0_L2ICACHE0_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_L2ICACHE0_DEFAULT                     (_SMU_PPUPATD0_L2ICACHE0_DEFAULT << 21)    /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_PIXELRZ0                              (0x1UL << 22)                              /**< PIXELRZ0 Privileged Access                  */
#define _SMU_PPUPATD0_PIXELRZ0_SHIFT                       22                                         /**< Shift value for SMU_PIXELRZ0                */
#define _SMU_PPUPATD0_PIXELRZ0_MASK                        0x400000UL                                 /**< Bit mask for SMU_PIXELRZ0                   */
#define _SMU_PPUPATD0_PIXELRZ0_DEFAULT                     0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_PIXELRZ0_DEFAULT                      (_SMU_PPUPATD0_PIXELRZ0_DEFAULT << 22)     /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_BURTC                                 (0x1UL << 23)                              /**< BURTC Privileged Access                     */
#define _SMU_PPUPATD0_BURTC_SHIFT                          23                                         /**< Shift value for SMU_BURTC                   */
#define _SMU_PPUPATD0_BURTC_MASK                           0x800000UL                                 /**< Bit mask for SMU_BURTC                      */
#define _SMU_PPUPATD0_BURTC_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_BURTC_DEFAULT                         (_SMU_PPUPATD0_BURTC_DEFAULT << 23)        /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_I2C1                                  (0x1UL << 24)                              /**< I2C1 Privileged Access                      */
#define _SMU_PPUPATD0_I2C1_SHIFT                           24                                         /**< Shift value for SMU_I2C1                    */
#define _SMU_PPUPATD0_I2C1_MASK                            0x1000000UL                                /**< Bit mask for SMU_I2C1                       */
#define _SMU_PPUPATD0_I2C1_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_I2C1_DEFAULT                          (_SMU_PPUPATD0_I2C1_DEFAULT << 24)         /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_CHIPTESTCTRL                          (0x1UL << 25)                              /**< CHIPTESTCTRL Privileged Access              */
#define _SMU_PPUPATD0_CHIPTESTCTRL_SHIFT                   25                                         /**< Shift value for SMU_CHIPTESTCTRL            */
#define _SMU_PPUPATD0_CHIPTESTCTRL_MASK                    0x2000000UL                                /**< Bit mask for SMU_CHIPTESTCTRL               */
#define _SMU_PPUPATD0_CHIPTESTCTRL_DEFAULT                 0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_CHIPTESTCTRL_DEFAULT                  (_SMU_PPUPATD0_CHIPTESTCTRL_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_SYSCFGCFGNS                           (0x1UL << 26)                              /**< SYSCFGCFGNS Privileged Access               */
#define _SMU_PPUPATD0_SYSCFGCFGNS_SHIFT                    26                                         /**< Shift value for SMU_SYSCFGCFGNS             */
#define _SMU_PPUPATD0_SYSCFGCFGNS_MASK                     0x4000000UL                                /**< Bit mask for SMU_SYSCFGCFGNS                */
#define _SMU_PPUPATD0_SYSCFGCFGNS_DEFAULT                  0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_SYSCFGCFGNS_DEFAULT                   (_SMU_PPUPATD0_SYSCFGCFGNS_DEFAULT << 26)  /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_SYSCFG                                (0x1UL << 27)                              /**< SYSCFG Privileged Access                    */
#define _SMU_PPUPATD0_SYSCFG_SHIFT                         27                                         /**< Shift value for SMU_SYSCFG                  */
#define _SMU_PPUPATD0_SYSCFG_MASK                          0x8000000UL                                /**< Bit mask for SMU_SYSCFG                     */
#define _SMU_PPUPATD0_SYSCFG_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_SYSCFG_DEFAULT                        (_SMU_PPUPATD0_SYSCFG_DEFAULT << 27)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_BURAM                                 (0x1UL << 28)                              /**< BURAM Privileged Access                     */
#define _SMU_PPUPATD0_BURAM_SHIFT                          28                                         /**< Shift value for SMU_BURAM                   */
#define _SMU_PPUPATD0_BURAM_MASK                           0x10000000UL                               /**< Bit mask for SMU_BURAM                      */
#define _SMU_PPUPATD0_BURAM_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_BURAM_DEFAULT                         (_SMU_PPUPATD0_BURAM_DEFAULT << 28)        /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_GPCRC0                                (0x1UL << 29)                              /**< GPCRC0 Privileged Access                    */
#define _SMU_PPUPATD0_GPCRC0_SHIFT                         29                                         /**< Shift value for SMU_GPCRC0                  */
#define _SMU_PPUPATD0_GPCRC0_MASK                          0x20000000UL                               /**< Bit mask for SMU_GPCRC0                     */
#define _SMU_PPUPATD0_GPCRC0_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_GPCRC0_DEFAULT                        (_SMU_PPUPATD0_GPCRC0_DEFAULT << 29)       /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_LEDDRV0                               (0x1UL << 30)                              /**< LEDDRV0 Privileged Access                   */
#define _SMU_PPUPATD0_LEDDRV0_SHIFT                        30                                         /**< Shift value for SMU_LEDDRV0                 */
#define _SMU_PPUPATD0_LEDDRV0_MASK                         0x40000000UL                               /**< Bit mask for SMU_LEDDRV0                    */
#define _SMU_PPUPATD0_LEDDRV0_DEFAULT                      0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_LEDDRV0_DEFAULT                       (_SMU_PPUPATD0_LEDDRV0_DEFAULT << 30)      /**< Shifted mode DEFAULT for SMU_PPUPATD0       */
#define SMU_PPUPATD0_EUSART1                               (0x1UL << 31)                              /**< EUSART1 Privileged Access                   */
#define _SMU_PPUPATD0_EUSART1_SHIFT                        31                                         /**< Shift value for SMU_EUSART1                 */
#define _SMU_PPUPATD0_EUSART1_MASK                         0x80000000UL                               /**< Bit mask for SMU_EUSART1                    */
#define _SMU_PPUPATD0_EUSART1_DEFAULT                      0x00000001UL                               /**< Mode DEFAULT for SMU_PPUPATD0               */
#define SMU_PPUPATD0_EUSART1_DEFAULT                       (_SMU_PPUPATD0_EUSART1_DEFAULT << 31)      /**< Shifted mode DEFAULT for SMU_PPUPATD0       */

/* Bit fields for SMU PPUPATD1 */
#define _SMU_PPUPATD1_RESETVALUE                           0xFFFFFFFFUL                                   /**< Default value for SMU_PPUPATD1              */
#define _SMU_PPUPATD1_MASK                                 0xFFFFFFFFUL                                   /**< Mask for SMU_PPUPATD1                       */
#define SMU_PPUPATD1_EUSART2                               (0x1UL << 0)                                   /**< EUSART2 Privileged Access                   */
#define _SMU_PPUPATD1_EUSART2_SHIFT                        0                                              /**< Shift value for SMU_EUSART2                 */
#define _SMU_PPUPATD1_EUSART2_MASK                         0x1UL                                          /**< Bit mask for SMU_EUSART2                    */
#define _SMU_PPUPATD1_EUSART2_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_EUSART2_DEFAULT                       (_SMU_PPUPATD1_EUSART2_DEFAULT << 0)           /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_PIXELRZ1                              (0x1UL << 1)                                   /**< PIXELRZ1 Privileged Access                  */
#define _SMU_PPUPATD1_PIXELRZ1_SHIFT                       1                                              /**< Shift value for SMU_PIXELRZ1                */
#define _SMU_PPUPATD1_PIXELRZ1_MASK                        0x2UL                                          /**< Bit mask for SMU_PIXELRZ1                   */
#define _SMU_PPUPATD1_PIXELRZ1_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_PIXELRZ1_DEFAULT                      (_SMU_PPUPATD1_PIXELRZ1_DEFAULT << 1)          /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_I2C2                                  (0x1UL << 2)                                   /**< I2C2 Privileged Access                      */
#define _SMU_PPUPATD1_I2C2_SHIFT                           2                                              /**< Shift value for SMU_I2C2                    */
#define _SMU_PPUPATD1_I2C2_MASK                            0x4UL                                          /**< Bit mask for SMU_I2C2                       */
#define _SMU_PPUPATD1_I2C2_DEFAULT                         0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_I2C2_DEFAULT                          (_SMU_PPUPATD1_I2C2_DEFAULT << 2)              /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_WDOG1                                 (0x1UL << 3)                                   /**< WDOG1 Privileged Access                     */
#define _SMU_PPUPATD1_WDOG1_SHIFT                          3                                              /**< Shift value for SMU_WDOG1                   */
#define _SMU_PPUPATD1_WDOG1_MASK                           0x8UL                                          /**< Bit mask for SMU_WDOG1                      */
#define _SMU_PPUPATD1_WDOG1_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_WDOG1_DEFAULT                         (_SMU_PPUPATD1_WDOG1_DEFAULT << 3)             /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_RPA                                   (0x1UL << 4)                                   /**< RPA Privileged Access                       */
#define _SMU_PPUPATD1_RPA_SHIFT                            4                                              /**< Shift value for SMU_RPA                     */
#define _SMU_PPUPATD1_RPA_MASK                             0x10UL                                         /**< Bit mask for SMU_RPA                        */
#define _SMU_PPUPATD1_RPA_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_RPA_DEFAULT                           (_SMU_PPUPATD1_RPA_DEFAULT << 4)               /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_KSU                                   (0x1UL << 5)                                   /**< KSU Privileged Access                       */
#define _SMU_PPUPATD1_KSU_SHIFT                            5                                              /**< Shift value for SMU_KSU                     */
#define _SMU_PPUPATD1_KSU_MASK                             0x20UL                                         /**< Bit mask for SMU_KSU                        */
#define _SMU_PPUPATD1_KSU_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_KSU_DEFAULT                           (_SMU_PPUPATD1_KSU_DEFAULT << 5)               /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_DEVINFO                               (0x1UL << 6)                                   /**< DEVINFO Privileged Access                   */
#define _SMU_PPUPATD1_DEVINFO_SHIFT                        6                                              /**< Shift value for SMU_DEVINFO                 */
#define _SMU_PPUPATD1_DEVINFO_MASK                         0x40UL                                         /**< Bit mask for SMU_DEVINFO                    */
#define _SMU_PPUPATD1_DEVINFO_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_DEVINFO_DEFAULT                       (_SMU_PPUPATD1_DEVINFO_DEFAULT << 6)           /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_LETIMER0                              (0x1UL << 7)                                   /**< LETIMER0 Privileged Access                  */
#define _SMU_PPUPATD1_LETIMER0_SHIFT                       7                                              /**< Shift value for SMU_LETIMER0                */
#define _SMU_PPUPATD1_LETIMER0_MASK                        0x80UL                                         /**< Bit mask for SMU_LETIMER0                   */
#define _SMU_PPUPATD1_LETIMER0_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_LETIMER0_DEFAULT                      (_SMU_PPUPATD1_LETIMER0_DEFAULT << 7)          /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_ADC0                                  (0x1UL << 8)                                   /**< ADC0 Privileged Access                      */
#define _SMU_PPUPATD1_ADC0_SHIFT                           8                                              /**< Shift value for SMU_ADC0                    */
#define _SMU_PPUPATD1_ADC0_MASK                            0x100UL                                        /**< Bit mask for SMU_ADC0                       */
#define _SMU_PPUPATD1_ADC0_DEFAULT                         0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_ADC0_DEFAULT                          (_SMU_PPUPATD1_ADC0_DEFAULT << 8)              /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_ACMP0                                 (0x1UL << 9)                                   /**< ACMP0 Privileged Access                     */
#define _SMU_PPUPATD1_ACMP0_SHIFT                          9                                              /**< Shift value for SMU_ACMP0                   */
#define _SMU_PPUPATD1_ACMP0_MASK                           0x200UL                                        /**< Bit mask for SMU_ACMP0                      */
#define _SMU_PPUPATD1_ACMP0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_ACMP0_DEFAULT                         (_SMU_PPUPATD1_ACMP0_DEFAULT << 9)             /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_ACMP1                                 (0x1UL << 10)                                  /**< ACMP1 Privileged Access                     */
#define _SMU_PPUPATD1_ACMP1_SHIFT                          10                                             /**< Shift value for SMU_ACMP1                   */
#define _SMU_PPUPATD1_ACMP1_MASK                           0x400UL                                        /**< Bit mask for SMU_ACMP1                      */
#define _SMU_PPUPATD1_ACMP1_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_ACMP1_DEFAULT                         (_SMU_PPUPATD1_ACMP1_DEFAULT << 10)            /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_AMUXCP0                               (0x1UL << 11)                                  /**< AMUXCP0 Privileged Access                   */
#define _SMU_PPUPATD1_AMUXCP0_SHIFT                        11                                             /**< Shift value for SMU_AMUXCP0                 */
#define _SMU_PPUPATD1_AMUXCP0_MASK                         0x800UL                                        /**< Bit mask for SMU_AMUXCP0                    */
#define _SMU_PPUPATD1_AMUXCP0_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_AMUXCP0_DEFAULT                       (_SMU_PPUPATD1_AMUXCP0_DEFAULT << 11)          /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_PCNT0                                 (0x1UL << 12)                                  /**< PCNT0 Privileged Access                     */
#define _SMU_PPUPATD1_PCNT0_SHIFT                          12                                             /**< Shift value for SMU_PCNT0                   */
#define _SMU_PPUPATD1_PCNT0_MASK                           0x1000UL                                       /**< Bit mask for SMU_PCNT0                      */
#define _SMU_PPUPATD1_PCNT0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_PCNT0_DEFAULT                         (_SMU_PPUPATD1_PCNT0_DEFAULT << 12)            /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_HFRCOEM23                             (0x1UL << 13)                                  /**< HFRCOEM23 Privileged Access                 */
#define _SMU_PPUPATD1_HFRCOEM23_SHIFT                      13                                             /**< Shift value for SMU_HFRCOEM23               */
#define _SMU_PPUPATD1_HFRCOEM23_MASK                       0x2000UL                                       /**< Bit mask for SMU_HFRCOEM23                  */
#define _SMU_PPUPATD1_HFRCOEM23_DEFAULT                    0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_HFRCOEM23_DEFAULT                     (_SMU_PPUPATD1_HFRCOEM23_DEFAULT << 13)        /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_HFXO0                                 (0x1UL << 14)                                  /**< HFXO0 Privileged Access                     */
#define _SMU_PPUPATD1_HFXO0_SHIFT                          14                                             /**< Shift value for SMU_HFXO0                   */
#define _SMU_PPUPATD1_HFXO0_MASK                           0x4000UL                                       /**< Bit mask for SMU_HFXO0                      */
#define _SMU_PPUPATD1_HFXO0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_HFXO0_DEFAULT                         (_SMU_PPUPATD1_HFXO0_DEFAULT << 14)            /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_I2C0                                  (0x1UL << 15)                                  /**< I2C0 Privileged Access                      */
#define _SMU_PPUPATD1_I2C0_SHIFT                           15                                             /**< Shift value for SMU_I2C0                    */
#define _SMU_PPUPATD1_I2C0_MASK                            0x8000UL                                       /**< Bit mask for SMU_I2C0                       */
#define _SMU_PPUPATD1_I2C0_DEFAULT                         0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_I2C0_DEFAULT                          (_SMU_PPUPATD1_I2C0_DEFAULT << 15)             /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_WDOG0                                 (0x1UL << 16)                                  /**< WDOG0 Privileged Access                     */
#define _SMU_PPUPATD1_WDOG0_SHIFT                          16                                             /**< Shift value for SMU_WDOG0                   */
#define _SMU_PPUPATD1_WDOG0_MASK                           0x10000UL                                      /**< Bit mask for SMU_WDOG0                      */
#define _SMU_PPUPATD1_WDOG0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_WDOG0_DEFAULT                         (_SMU_PPUPATD1_WDOG0_DEFAULT << 16)            /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_EUSART0                               (0x1UL << 17)                                  /**< EUSART0 Privileged Access                   */
#define _SMU_PPUPATD1_EUSART0_SHIFT                        17                                             /**< Shift value for SMU_EUSART0                 */
#define _SMU_PPUPATD1_EUSART0_MASK                         0x20000UL                                      /**< Bit mask for SMU_EUSART0                    */
#define _SMU_PPUPATD1_EUSART0_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_EUSART0_DEFAULT                       (_SMU_PPUPATD1_EUSART0_DEFAULT << 17)          /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SYMCRYPTODMACTRL                      (0x1UL << 18)                                  /**< SYMCRYPTODMACTRL Privileged Access          */
#define _SMU_PPUPATD1_SYMCRYPTODMACTRL_SHIFT               18                                             /**< Shift value for SMU_SYMCRYPTODMACTRL        */
#define _SMU_PPUPATD1_SYMCRYPTODMACTRL_MASK                0x40000UL                                      /**< Bit mask for SMU_SYMCRYPTODMACTRL           */
#define _SMU_PPUPATD1_SYMCRYPTODMACTRL_DEFAULT             0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SYMCRYPTODMACTRL_DEFAULT              (_SMU_PPUPATD1_SYMCRYPTODMACTRL_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_LPWAESDMACTRL                         (0x1UL << 19)                                  /**< LPWAESDMACTRL Privileged Access             */
#define _SMU_PPUPATD1_LPWAESDMACTRL_SHIFT                  19                                             /**< Shift value for SMU_LPWAESDMACTRL           */
#define _SMU_PPUPATD1_LPWAESDMACTRL_MASK                   0x80000UL                                      /**< Bit mask for SMU_LPWAESDMACTRL              */
#define _SMU_PPUPATD1_LPWAESDMACTRL_DEFAULT                0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_LPWAESDMACTRL_DEFAULT                 (_SMU_PPUPATD1_LPWAESDMACTRL_DEFAULT << 19)    /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SMU                                   (0x1UL << 20)                                  /**< SMU Privileged Access                       */
#define _SMU_PPUPATD1_SMU_SHIFT                            20                                             /**< Shift value for SMU_SMU                     */
#define _SMU_PPUPATD1_SMU_MASK                             0x100000UL                                     /**< Bit mask for SMU_SMU                        */
#define _SMU_PPUPATD1_SMU_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SMU_DEFAULT                           (_SMU_PPUPATD1_SMU_DEFAULT << 20)              /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SMUCFGNS                              (0x1UL << 21)                                  /**< SMUCFGNS Privileged Access                  */
#define _SMU_PPUPATD1_SMUCFGNS_SHIFT                       21                                             /**< Shift value for SMU_SMUCFGNS                */
#define _SMU_PPUPATD1_SMUCFGNS_MASK                        0x200000UL                                     /**< Bit mask for SMU_SMUCFGNS                   */
#define _SMU_PPUPATD1_SMUCFGNS_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SMUCFGNS_DEFAULT                      (_SMU_PPUPATD1_SMUCFGNS_DEFAULT << 21)         /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SEMAILBOX                             (0x1UL << 22)                                  /**< SEMAILBOX Privileged Access                 */
#define _SMU_PPUPATD1_SEMAILBOX_SHIFT                      22                                             /**< Shift value for SMU_SEMAILBOX               */
#define _SMU_PPUPATD1_SEMAILBOX_MASK                       0x400000UL                                     /**< Bit mask for SMU_SEMAILBOX                  */
#define _SMU_PPUPATD1_SEMAILBOX_DEFAULT                    0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SEMAILBOX_DEFAULT                     (_SMU_PPUPATD1_SEMAILBOX_DEFAULT << 22)        /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_QSPI0MANUAL                           (0x1UL << 23)                                  /**< QSPI0MANUAL Privileged Access               */
#define _SMU_PPUPATD1_QSPI0MANUAL_SHIFT                    23                                             /**< Shift value for SMU_QSPI0MANUAL             */
#define _SMU_PPUPATD1_QSPI0MANUAL_MASK                     0x800000UL                                     /**< Bit mask for SMU_QSPI0MANUAL                */
#define _SMU_PPUPATD1_QSPI0MANUAL_DEFAULT                  0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_QSPI0MANUAL_DEFAULT                   (_SMU_PPUPATD1_QSPI0MANUAL_DEFAULT << 23)      /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_AHBRADIO                              (0x1UL << 24)                                  /**< AHBRADIO Privileged Access                  */
#define _SMU_PPUPATD1_AHBRADIO_SHIFT                       24                                             /**< Shift value for SMU_AHBRADIO                */
#define _SMU_PPUPATD1_AHBRADIO_MASK                        0x1000000UL                                    /**< Bit mask for SMU_AHBRADIO                   */
#define _SMU_PPUPATD1_AHBRADIO_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_AHBRADIO_DEFAULT                      (_SMU_PPUPATD1_AHBRADIO_DEFAULT << 24)         /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SYSRTC0                               (0x1UL << 25)                                  /**< SYSRTC0 Privileged Access                   */
#define _SMU_PPUPATD1_SYSRTC0_SHIFT                        25                                             /**< Shift value for SMU_SYSRTC0                 */
#define _SMU_PPUPATD1_SYSRTC0_MASK                         0x2000000UL                                    /**< Bit mask for SMU_SYSRTC0                    */
#define _SMU_PPUPATD1_SYSRTC0_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SYSRTC0_DEFAULT                       (_SMU_PPUPATD1_SYSRTC0_DEFAULT << 25)          /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SCRATCHPAD                            (0x1UL << 26)                                  /**< SCRATCHPAD Privileged Access                */
#define _SMU_PPUPATD1_SCRATCHPAD_SHIFT                     26                                             /**< Shift value for SMU_SCRATCHPAD              */
#define _SMU_PPUPATD1_SCRATCHPAD_MASK                      0x4000000UL                                    /**< Bit mask for SMU_SCRATCHPAD                 */
#define _SMU_PPUPATD1_SCRATCHPAD_DEFAULT                   0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SCRATCHPAD_DEFAULT                    (_SMU_PPUPATD1_SCRATCHPAD_DEFAULT << 26)       /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_EMU                                   (0x1UL << 27)                                  /**< EMU Privileged Access                       */
#define _SMU_PPUPATD1_EMU_SHIFT                            27                                             /**< Shift value for SMU_EMU                     */
#define _SMU_PPUPATD1_EMU_MASK                             0x8000000UL                                    /**< Bit mask for SMU_EMU                        */
#define _SMU_PPUPATD1_EMU_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_EMU_DEFAULT                           (_SMU_PPUPATD1_EMU_DEFAULT << 27)              /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_EMUCFGNS                              (0x1UL << 28)                                  /**< EMUCFGNS Privileged Access                  */
#define _SMU_PPUPATD1_EMUCFGNS_SHIFT                       28                                             /**< Shift value for SMU_EMUCFGNS                */
#define _SMU_PPUPATD1_EMUCFGNS_MASK                        0x10000000UL                                   /**< Bit mask for SMU_EMUCFGNS                   */
#define _SMU_PPUPATD1_EMUCFGNS_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_EMUCFGNS_DEFAULT                      (_SMU_PPUPATD1_EMUCFGNS_DEFAULT << 28)         /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_SEPORTAL                              (0x1UL << 29)                                  /**< SEPORTAL Privileged Access                  */
#define _SMU_PPUPATD1_SEPORTAL_SHIFT                       29                                             /**< Shift value for SMU_SEPORTAL                */
#define _SMU_PPUPATD1_SEPORTAL_MASK                        0x20000000UL                                   /**< Bit mask for SMU_SEPORTAL                   */
#define _SMU_PPUPATD1_SEPORTAL_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_SEPORTAL_DEFAULT                      (_SMU_PPUPATD1_SEPORTAL_DEFAULT << 29)         /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_LPW0PORTAL                            (0x1UL << 30)                                  /**< LPW0PORTAL Privileged Access                */
#define _SMU_PPUPATD1_LPW0PORTAL_SHIFT                     30                                             /**< Shift value for SMU_LPW0PORTAL              */
#define _SMU_PPUPATD1_LPW0PORTAL_MASK                      0x40000000UL                                   /**< Bit mask for SMU_LPW0PORTAL                 */
#define _SMU_PPUPATD1_LPW0PORTAL_DEFAULT                   0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_LPW0PORTAL_DEFAULT                    (_SMU_PPUPATD1_LPW0PORTAL_DEFAULT << 30)       /**< Shifted mode DEFAULT for SMU_PPUPATD1       */
#define SMU_PPUPATD1_HOSTPORTAL                            (0x1UL << 31)                                  /**< HOSTPORTAL Privileged Access                */
#define _SMU_PPUPATD1_HOSTPORTAL_SHIFT                     31                                             /**< Shift value for SMU_HOSTPORTAL              */
#define _SMU_PPUPATD1_HOSTPORTAL_MASK                      0x80000000UL                                   /**< Bit mask for SMU_HOSTPORTAL                 */
#define _SMU_PPUPATD1_HOSTPORTAL_DEFAULT                   0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUPATD1               */
#define SMU_PPUPATD1_HOSTPORTAL_DEFAULT                    (_SMU_PPUPATD1_HOSTPORTAL_DEFAULT << 31)       /**< Shifted mode DEFAULT for SMU_PPUPATD1       */

/* Bit fields for SMU PPUSATD0 */
#define _SMU_PPUSATD0_RESETVALUE                           0xFFFFFFFFUL                               /**< Default value for SMU_PPUSATD0              */
#define _SMU_PPUSATD0_MASK                                 0xFFFFFFFFUL                               /**< Mask for SMU_PPUSATD0                       */
#define SMU_PPUSATD0_SOCPLL0                               (0x1UL << 0)                               /**< SOCPLL0 Secure Access                       */
#define _SMU_PPUSATD0_SOCPLL0_SHIFT                        0                                          /**< Shift value for SMU_SOCPLL0                 */
#define _SMU_PPUSATD0_SOCPLL0_MASK                         0x1UL                                      /**< Bit mask for SMU_SOCPLL0                    */
#define _SMU_PPUSATD0_SOCPLL0_DEFAULT                      0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_SOCPLL0_DEFAULT                       (_SMU_PPUSATD0_SOCPLL0_DEFAULT << 0)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_CMU                                   (0x1UL << 1)                               /**< CMU Secure Access                           */
#define _SMU_PPUSATD0_CMU_SHIFT                            1                                          /**< Shift value for SMU_CMU                     */
#define _SMU_PPUSATD0_CMU_MASK                             0x2UL                                      /**< Bit mask for SMU_CMU                        */
#define _SMU_PPUSATD0_CMU_DEFAULT                          0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_CMU_DEFAULT                           (_SMU_PPUSATD0_CMU_DEFAULT << 1)           /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_DMEM                                  (0x1UL << 2)                               /**< DMEM Secure Access                          */
#define _SMU_PPUSATD0_DMEM_SHIFT                           2                                          /**< Shift value for SMU_DMEM                    */
#define _SMU_PPUSATD0_DMEM_MASK                            0x4UL                                      /**< Bit mask for SMU_DMEM                       */
#define _SMU_PPUSATD0_DMEM_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_DMEM_DEFAULT                          (_SMU_PPUSATD0_DMEM_DEFAULT << 2)          /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_HFRCO0                                (0x1UL << 3)                               /**< HFRCO0 Secure Access                        */
#define _SMU_PPUSATD0_HFRCO0_SHIFT                         3                                          /**< Shift value for SMU_HFRCO0                  */
#define _SMU_PPUSATD0_HFRCO0_MASK                          0x8UL                                      /**< Bit mask for SMU_HFRCO0                     */
#define _SMU_PPUSATD0_HFRCO0_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_HFRCO0_DEFAULT                        (_SMU_PPUSATD0_HFRCO0_DEFAULT << 3)        /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_LDMA0                                 (0x1UL << 4)                               /**< LDMA0 Secure Access                         */
#define _SMU_PPUSATD0_LDMA0_SHIFT                          4                                          /**< Shift value for SMU_LDMA0                   */
#define _SMU_PPUSATD0_LDMA0_MASK                           0x10UL                                     /**< Bit mask for SMU_LDMA0                      */
#define _SMU_PPUSATD0_LDMA0_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_LDMA0_DEFAULT                         (_SMU_PPUSATD0_LDMA0_DEFAULT << 4)         /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_FSRCO                                 (0x1UL << 5)                               /**< FSRCO Secure Access                         */
#define _SMU_PPUSATD0_FSRCO_SHIFT                          5                                          /**< Shift value for SMU_FSRCO                   */
#define _SMU_PPUSATD0_FSRCO_MASK                           0x20UL                                     /**< Bit mask for SMU_FSRCO                      */
#define _SMU_PPUSATD0_FSRCO_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_FSRCO_DEFAULT                         (_SMU_PPUSATD0_FSRCO_DEFAULT << 5)         /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_DPLL0                                 (0x1UL << 6)                               /**< DPLL0 Secure Access                         */
#define _SMU_PPUSATD0_DPLL0_SHIFT                          6                                          /**< Shift value for SMU_DPLL0                   */
#define _SMU_PPUSATD0_DPLL0_MASK                           0x40UL                                     /**< Bit mask for SMU_DPLL0                      */
#define _SMU_PPUSATD0_DPLL0_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_DPLL0_DEFAULT                         (_SMU_PPUSATD0_DPLL0_DEFAULT << 6)         /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_LFXO                                  (0x1UL << 7)                               /**< LFXO Secure Access                          */
#define _SMU_PPUSATD0_LFXO_SHIFT                           7                                          /**< Shift value for SMU_LFXO                    */
#define _SMU_PPUSATD0_LFXO_MASK                            0x80UL                                     /**< Bit mask for SMU_LFXO                       */
#define _SMU_PPUSATD0_LFXO_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_LFXO_DEFAULT                          (_SMU_PPUSATD0_LFXO_DEFAULT << 7)          /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_LFRCO                                 (0x1UL << 8)                               /**< LFRCO Secure Access                         */
#define _SMU_PPUSATD0_LFRCO_SHIFT                          8                                          /**< Shift value for SMU_LFRCO                   */
#define _SMU_PPUSATD0_LFRCO_MASK                           0x100UL                                    /**< Bit mask for SMU_LFRCO                      */
#define _SMU_PPUSATD0_LFRCO_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_LFRCO_DEFAULT                         (_SMU_PPUSATD0_LFRCO_DEFAULT << 8)         /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_ULFRCO                                (0x1UL << 9)                               /**< ULFRCO Secure Access                        */
#define _SMU_PPUSATD0_ULFRCO_SHIFT                         9                                          /**< Shift value for SMU_ULFRCO                  */
#define _SMU_PPUSATD0_ULFRCO_MASK                          0x200UL                                    /**< Bit mask for SMU_ULFRCO                     */
#define _SMU_PPUSATD0_ULFRCO_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_ULFRCO_DEFAULT                        (_SMU_PPUSATD0_ULFRCO_DEFAULT << 9)        /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_SEMAPHORE0                            (0x1UL << 10)                              /**< SEMAPHORE0 Secure Access                    */
#define _SMU_PPUSATD0_SEMAPHORE0_SHIFT                     10                                         /**< Shift value for SMU_SEMAPHORE0              */
#define _SMU_PPUSATD0_SEMAPHORE0_MASK                      0x400UL                                    /**< Bit mask for SMU_SEMAPHORE0                 */
#define _SMU_PPUSATD0_SEMAPHORE0_DEFAULT                   0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_SEMAPHORE0_DEFAULT                    (_SMU_PPUSATD0_SEMAPHORE0_DEFAULT << 10)   /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_SEMAPHORE1                            (0x1UL << 11)                              /**< SEMAPHORE1 Secure Access                    */
#define _SMU_PPUSATD0_SEMAPHORE1_SHIFT                     11                                         /**< Shift value for SMU_SEMAPHORE1              */
#define _SMU_PPUSATD0_SEMAPHORE1_MASK                      0x800UL                                    /**< Bit mask for SMU_SEMAPHORE1                 */
#define _SMU_PPUSATD0_SEMAPHORE1_DEFAULT                   0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_SEMAPHORE1_DEFAULT                    (_SMU_PPUSATD0_SEMAPHORE1_DEFAULT << 11)   /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_L1ICACHE0                             (0x1UL << 12)                              /**< L1ICACHE0 Secure Access                     */
#define _SMU_PPUSATD0_L1ICACHE0_SHIFT                      12                                         /**< Shift value for SMU_L1ICACHE0               */
#define _SMU_PPUSATD0_L1ICACHE0_MASK                       0x1000UL                                   /**< Bit mask for SMU_L1ICACHE0                  */
#define _SMU_PPUSATD0_L1ICACHE0_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_L1ICACHE0_DEFAULT                     (_SMU_PPUSATD0_L1ICACHE0_DEFAULT << 12)    /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_PRS                                   (0x1UL << 13)                              /**< PRS Secure Access                           */
#define _SMU_PPUSATD0_PRS_SHIFT                            13                                         /**< Shift value for SMU_PRS                     */
#define _SMU_PPUSATD0_PRS_MASK                             0x2000UL                                   /**< Bit mask for SMU_PRS                        */
#define _SMU_PPUSATD0_PRS_DEFAULT                          0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_PRS_DEFAULT                           (_SMU_PPUSATD0_PRS_DEFAULT << 13)          /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_GPIO                                  (0x1UL << 14)                              /**< GPIO Secure Access                          */
#define _SMU_PPUSATD0_GPIO_SHIFT                           14                                         /**< Shift value for SMU_GPIO                    */
#define _SMU_PPUSATD0_GPIO_MASK                            0x4000UL                                   /**< Bit mask for SMU_GPIO                       */
#define _SMU_PPUSATD0_GPIO_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_GPIO_DEFAULT                          (_SMU_PPUSATD0_GPIO_DEFAULT << 14)         /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_LDMAXBAR0                             (0x1UL << 15)                              /**< LDMAXBAR0 Secure Access                     */
#define _SMU_PPUSATD0_LDMAXBAR0_SHIFT                      15                                         /**< Shift value for SMU_LDMAXBAR0               */
#define _SMU_PPUSATD0_LDMAXBAR0_MASK                       0x8000UL                                   /**< Bit mask for SMU_LDMAXBAR0                  */
#define _SMU_PPUSATD0_LDMAXBAR0_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_LDMAXBAR0_DEFAULT                     (_SMU_PPUSATD0_LDMAXBAR0_DEFAULT << 15)    /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_TIMER0                                (0x1UL << 16)                              /**< TIMER0 Secure Access                        */
#define _SMU_PPUSATD0_TIMER0_SHIFT                         16                                         /**< Shift value for SMU_TIMER0                  */
#define _SMU_PPUSATD0_TIMER0_MASK                          0x10000UL                                  /**< Bit mask for SMU_TIMER0                     */
#define _SMU_PPUSATD0_TIMER0_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_TIMER0_DEFAULT                        (_SMU_PPUSATD0_TIMER0_DEFAULT << 16)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_TIMER1                                (0x1UL << 17)                              /**< TIMER1 Secure Access                        */
#define _SMU_PPUSATD0_TIMER1_SHIFT                         17                                         /**< Shift value for SMU_TIMER1                  */
#define _SMU_PPUSATD0_TIMER1_MASK                          0x20000UL                                  /**< Bit mask for SMU_TIMER1                     */
#define _SMU_PPUSATD0_TIMER1_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_TIMER1_DEFAULT                        (_SMU_PPUSATD0_TIMER1_DEFAULT << 17)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_TIMER2                                (0x1UL << 18)                              /**< TIMER2 Secure Access                        */
#define _SMU_PPUSATD0_TIMER2_SHIFT                         18                                         /**< Shift value for SMU_TIMER2                  */
#define _SMU_PPUSATD0_TIMER2_MASK                          0x40000UL                                  /**< Bit mask for SMU_TIMER2                     */
#define _SMU_PPUSATD0_TIMER2_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_TIMER2_DEFAULT                        (_SMU_PPUSATD0_TIMER2_DEFAULT << 18)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_TIMER3                                (0x1UL << 19)                              /**< TIMER3 Secure Access                        */
#define _SMU_PPUSATD0_TIMER3_SHIFT                         19                                         /**< Shift value for SMU_TIMER3                  */
#define _SMU_PPUSATD0_TIMER3_MASK                          0x80000UL                                  /**< Bit mask for SMU_TIMER3                     */
#define _SMU_PPUSATD0_TIMER3_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_TIMER3_DEFAULT                        (_SMU_PPUSATD0_TIMER3_DEFAULT << 19)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_ETAMPDET                              (0x1UL << 20)                              /**< ETAMPDET Secure Access                      */
#define _SMU_PPUSATD0_ETAMPDET_SHIFT                       20                                         /**< Shift value for SMU_ETAMPDET                */
#define _SMU_PPUSATD0_ETAMPDET_MASK                        0x100000UL                                 /**< Bit mask for SMU_ETAMPDET                   */
#define _SMU_PPUSATD0_ETAMPDET_DEFAULT                     0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_ETAMPDET_DEFAULT                      (_SMU_PPUSATD0_ETAMPDET_DEFAULT << 20)     /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_L2ICACHE0                             (0x1UL << 21)                              /**< L2ICACHE0 Secure Access                     */
#define _SMU_PPUSATD0_L2ICACHE0_SHIFT                      21                                         /**< Shift value for SMU_L2ICACHE0               */
#define _SMU_PPUSATD0_L2ICACHE0_MASK                       0x200000UL                                 /**< Bit mask for SMU_L2ICACHE0                  */
#define _SMU_PPUSATD0_L2ICACHE0_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_L2ICACHE0_DEFAULT                     (_SMU_PPUSATD0_L2ICACHE0_DEFAULT << 21)    /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_PIXELRZ0                              (0x1UL << 22)                              /**< PIXELRZ0 Secure Access                      */
#define _SMU_PPUSATD0_PIXELRZ0_SHIFT                       22                                         /**< Shift value for SMU_PIXELRZ0                */
#define _SMU_PPUSATD0_PIXELRZ0_MASK                        0x400000UL                                 /**< Bit mask for SMU_PIXELRZ0                   */
#define _SMU_PPUSATD0_PIXELRZ0_DEFAULT                     0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_PIXELRZ0_DEFAULT                      (_SMU_PPUSATD0_PIXELRZ0_DEFAULT << 22)     /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_BURTC                                 (0x1UL << 23)                              /**< BURTC Secure Access                         */
#define _SMU_PPUSATD0_BURTC_SHIFT                          23                                         /**< Shift value for SMU_BURTC                   */
#define _SMU_PPUSATD0_BURTC_MASK                           0x800000UL                                 /**< Bit mask for SMU_BURTC                      */
#define _SMU_PPUSATD0_BURTC_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_BURTC_DEFAULT                         (_SMU_PPUSATD0_BURTC_DEFAULT << 23)        /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_I2C1                                  (0x1UL << 24)                              /**< I2C1 Secure Access                          */
#define _SMU_PPUSATD0_I2C1_SHIFT                           24                                         /**< Shift value for SMU_I2C1                    */
#define _SMU_PPUSATD0_I2C1_MASK                            0x1000000UL                                /**< Bit mask for SMU_I2C1                       */
#define _SMU_PPUSATD0_I2C1_DEFAULT                         0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_I2C1_DEFAULT                          (_SMU_PPUSATD0_I2C1_DEFAULT << 24)         /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_CHIPTESTCTRL                          (0x1UL << 25)                              /**< CHIPTESTCTRL Secure Access                  */
#define _SMU_PPUSATD0_CHIPTESTCTRL_SHIFT                   25                                         /**< Shift value for SMU_CHIPTESTCTRL            */
#define _SMU_PPUSATD0_CHIPTESTCTRL_MASK                    0x2000000UL                                /**< Bit mask for SMU_CHIPTESTCTRL               */
#define _SMU_PPUSATD0_CHIPTESTCTRL_DEFAULT                 0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_CHIPTESTCTRL_DEFAULT                  (_SMU_PPUSATD0_CHIPTESTCTRL_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_SYSCFGCFGNS                           (0x1UL << 26)                              /**< SYSCFGCFGNS Secure Access                   */
#define _SMU_PPUSATD0_SYSCFGCFGNS_SHIFT                    26                                         /**< Shift value for SMU_SYSCFGCFGNS             */
#define _SMU_PPUSATD0_SYSCFGCFGNS_MASK                     0x4000000UL                                /**< Bit mask for SMU_SYSCFGCFGNS                */
#define _SMU_PPUSATD0_SYSCFGCFGNS_DEFAULT                  0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_SYSCFGCFGNS_DEFAULT                   (_SMU_PPUSATD0_SYSCFGCFGNS_DEFAULT << 26)  /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_SYSCFG                                (0x1UL << 27)                              /**< SYSCFG Secure Access                        */
#define _SMU_PPUSATD0_SYSCFG_SHIFT                         27                                         /**< Shift value for SMU_SYSCFG                  */
#define _SMU_PPUSATD0_SYSCFG_MASK                          0x8000000UL                                /**< Bit mask for SMU_SYSCFG                     */
#define _SMU_PPUSATD0_SYSCFG_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_SYSCFG_DEFAULT                        (_SMU_PPUSATD0_SYSCFG_DEFAULT << 27)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_BURAM                                 (0x1UL << 28)                              /**< BURAM Secure Access                         */
#define _SMU_PPUSATD0_BURAM_SHIFT                          28                                         /**< Shift value for SMU_BURAM                   */
#define _SMU_PPUSATD0_BURAM_MASK                           0x10000000UL                               /**< Bit mask for SMU_BURAM                      */
#define _SMU_PPUSATD0_BURAM_DEFAULT                        0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_BURAM_DEFAULT                         (_SMU_PPUSATD0_BURAM_DEFAULT << 28)        /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_GPCRC0                                (0x1UL << 29)                              /**< GPCRC0 Secure Access                        */
#define _SMU_PPUSATD0_GPCRC0_SHIFT                         29                                         /**< Shift value for SMU_GPCRC0                  */
#define _SMU_PPUSATD0_GPCRC0_MASK                          0x20000000UL                               /**< Bit mask for SMU_GPCRC0                     */
#define _SMU_PPUSATD0_GPCRC0_DEFAULT                       0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_GPCRC0_DEFAULT                        (_SMU_PPUSATD0_GPCRC0_DEFAULT << 29)       /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_LEDDRV0                               (0x1UL << 30)                              /**< LEDDRV0 Secure Access                       */
#define _SMU_PPUSATD0_LEDDRV0_SHIFT                        30                                         /**< Shift value for SMU_LEDDRV0                 */
#define _SMU_PPUSATD0_LEDDRV0_MASK                         0x40000000UL                               /**< Bit mask for SMU_LEDDRV0                    */
#define _SMU_PPUSATD0_LEDDRV0_DEFAULT                      0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_LEDDRV0_DEFAULT                       (_SMU_PPUSATD0_LEDDRV0_DEFAULT << 30)      /**< Shifted mode DEFAULT for SMU_PPUSATD0       */
#define SMU_PPUSATD0_EUSART1                               (0x1UL << 31)                              /**< EUSART1 Secure Access                       */
#define _SMU_PPUSATD0_EUSART1_SHIFT                        31                                         /**< Shift value for SMU_EUSART1                 */
#define _SMU_PPUSATD0_EUSART1_MASK                         0x80000000UL                               /**< Bit mask for SMU_EUSART1                    */
#define _SMU_PPUSATD0_EUSART1_DEFAULT                      0x00000001UL                               /**< Mode DEFAULT for SMU_PPUSATD0               */
#define SMU_PPUSATD0_EUSART1_DEFAULT                       (_SMU_PPUSATD0_EUSART1_DEFAULT << 31)      /**< Shifted mode DEFAULT for SMU_PPUSATD0       */

/* Bit fields for SMU PPUSATD1 */
#define _SMU_PPUSATD1_RESETVALUE                           0xFFFFFFFFUL                                   /**< Default value for SMU_PPUSATD1              */
#define _SMU_PPUSATD1_MASK                                 0xFFFFFFFFUL                                   /**< Mask for SMU_PPUSATD1                       */
#define SMU_PPUSATD1_EUSART2                               (0x1UL << 0)                                   /**< EUSART2 Secure Access                       */
#define _SMU_PPUSATD1_EUSART2_SHIFT                        0                                              /**< Shift value for SMU_EUSART2                 */
#define _SMU_PPUSATD1_EUSART2_MASK                         0x1UL                                          /**< Bit mask for SMU_EUSART2                    */
#define _SMU_PPUSATD1_EUSART2_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_EUSART2_DEFAULT                       (_SMU_PPUSATD1_EUSART2_DEFAULT << 0)           /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_PIXELRZ1                              (0x1UL << 1)                                   /**< PIXELRZ1 Secure Access                      */
#define _SMU_PPUSATD1_PIXELRZ1_SHIFT                       1                                              /**< Shift value for SMU_PIXELRZ1                */
#define _SMU_PPUSATD1_PIXELRZ1_MASK                        0x2UL                                          /**< Bit mask for SMU_PIXELRZ1                   */
#define _SMU_PPUSATD1_PIXELRZ1_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_PIXELRZ1_DEFAULT                      (_SMU_PPUSATD1_PIXELRZ1_DEFAULT << 1)          /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_I2C2                                  (0x1UL << 2)                                   /**< I2C2 Secure Access                          */
#define _SMU_PPUSATD1_I2C2_SHIFT                           2                                              /**< Shift value for SMU_I2C2                    */
#define _SMU_PPUSATD1_I2C2_MASK                            0x4UL                                          /**< Bit mask for SMU_I2C2                       */
#define _SMU_PPUSATD1_I2C2_DEFAULT                         0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_I2C2_DEFAULT                          (_SMU_PPUSATD1_I2C2_DEFAULT << 2)              /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_WDOG1                                 (0x1UL << 3)                                   /**< WDOG1 Secure Access                         */
#define _SMU_PPUSATD1_WDOG1_SHIFT                          3                                              /**< Shift value for SMU_WDOG1                   */
#define _SMU_PPUSATD1_WDOG1_MASK                           0x8UL                                          /**< Bit mask for SMU_WDOG1                      */
#define _SMU_PPUSATD1_WDOG1_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_WDOG1_DEFAULT                         (_SMU_PPUSATD1_WDOG1_DEFAULT << 3)             /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_RPA                                   (0x1UL << 4)                                   /**< RPA Secure Access                           */
#define _SMU_PPUSATD1_RPA_SHIFT                            4                                              /**< Shift value for SMU_RPA                     */
#define _SMU_PPUSATD1_RPA_MASK                             0x10UL                                         /**< Bit mask for SMU_RPA                        */
#define _SMU_PPUSATD1_RPA_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_RPA_DEFAULT                           (_SMU_PPUSATD1_RPA_DEFAULT << 4)               /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_KSU                                   (0x1UL << 5)                                   /**< KSU Secure Access                           */
#define _SMU_PPUSATD1_KSU_SHIFT                            5                                              /**< Shift value for SMU_KSU                     */
#define _SMU_PPUSATD1_KSU_MASK                             0x20UL                                         /**< Bit mask for SMU_KSU                        */
#define _SMU_PPUSATD1_KSU_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_KSU_DEFAULT                           (_SMU_PPUSATD1_KSU_DEFAULT << 5)               /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_DEVINFO                               (0x1UL << 6)                                   /**< DEVINFO Secure Access                       */
#define _SMU_PPUSATD1_DEVINFO_SHIFT                        6                                              /**< Shift value for SMU_DEVINFO                 */
#define _SMU_PPUSATD1_DEVINFO_MASK                         0x40UL                                         /**< Bit mask for SMU_DEVINFO                    */
#define _SMU_PPUSATD1_DEVINFO_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_DEVINFO_DEFAULT                       (_SMU_PPUSATD1_DEVINFO_DEFAULT << 6)           /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_LETIMER0                              (0x1UL << 7)                                   /**< LETIMER0 Secure Access                      */
#define _SMU_PPUSATD1_LETIMER0_SHIFT                       7                                              /**< Shift value for SMU_LETIMER0                */
#define _SMU_PPUSATD1_LETIMER0_MASK                        0x80UL                                         /**< Bit mask for SMU_LETIMER0                   */
#define _SMU_PPUSATD1_LETIMER0_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_LETIMER0_DEFAULT                      (_SMU_PPUSATD1_LETIMER0_DEFAULT << 7)          /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_ADC0                                  (0x1UL << 8)                                   /**< ADC0 Secure Access                          */
#define _SMU_PPUSATD1_ADC0_SHIFT                           8                                              /**< Shift value for SMU_ADC0                    */
#define _SMU_PPUSATD1_ADC0_MASK                            0x100UL                                        /**< Bit mask for SMU_ADC0                       */
#define _SMU_PPUSATD1_ADC0_DEFAULT                         0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_ADC0_DEFAULT                          (_SMU_PPUSATD1_ADC0_DEFAULT << 8)              /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_ACMP0                                 (0x1UL << 9)                                   /**< ACMP0 Secure Access                         */
#define _SMU_PPUSATD1_ACMP0_SHIFT                          9                                              /**< Shift value for SMU_ACMP0                   */
#define _SMU_PPUSATD1_ACMP0_MASK                           0x200UL                                        /**< Bit mask for SMU_ACMP0                      */
#define _SMU_PPUSATD1_ACMP0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_ACMP0_DEFAULT                         (_SMU_PPUSATD1_ACMP0_DEFAULT << 9)             /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_ACMP1                                 (0x1UL << 10)                                  /**< ACMP1 Secure Access                         */
#define _SMU_PPUSATD1_ACMP1_SHIFT                          10                                             /**< Shift value for SMU_ACMP1                   */
#define _SMU_PPUSATD1_ACMP1_MASK                           0x400UL                                        /**< Bit mask for SMU_ACMP1                      */
#define _SMU_PPUSATD1_ACMP1_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_ACMP1_DEFAULT                         (_SMU_PPUSATD1_ACMP1_DEFAULT << 10)            /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_AMUXCP0                               (0x1UL << 11)                                  /**< AMUXCP0 Secure Access                       */
#define _SMU_PPUSATD1_AMUXCP0_SHIFT                        11                                             /**< Shift value for SMU_AMUXCP0                 */
#define _SMU_PPUSATD1_AMUXCP0_MASK                         0x800UL                                        /**< Bit mask for SMU_AMUXCP0                    */
#define _SMU_PPUSATD1_AMUXCP0_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_AMUXCP0_DEFAULT                       (_SMU_PPUSATD1_AMUXCP0_DEFAULT << 11)          /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_PCNT0                                 (0x1UL << 12)                                  /**< PCNT0 Secure Access                         */
#define _SMU_PPUSATD1_PCNT0_SHIFT                          12                                             /**< Shift value for SMU_PCNT0                   */
#define _SMU_PPUSATD1_PCNT0_MASK                           0x1000UL                                       /**< Bit mask for SMU_PCNT0                      */
#define _SMU_PPUSATD1_PCNT0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_PCNT0_DEFAULT                         (_SMU_PPUSATD1_PCNT0_DEFAULT << 12)            /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_HFRCOEM23                             (0x1UL << 13)                                  /**< HFRCOEM23 Secure Access                     */
#define _SMU_PPUSATD1_HFRCOEM23_SHIFT                      13                                             /**< Shift value for SMU_HFRCOEM23               */
#define _SMU_PPUSATD1_HFRCOEM23_MASK                       0x2000UL                                       /**< Bit mask for SMU_HFRCOEM23                  */
#define _SMU_PPUSATD1_HFRCOEM23_DEFAULT                    0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_HFRCOEM23_DEFAULT                     (_SMU_PPUSATD1_HFRCOEM23_DEFAULT << 13)        /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_HFXO0                                 (0x1UL << 14)                                  /**< HFXO0 Secure Access                         */
#define _SMU_PPUSATD1_HFXO0_SHIFT                          14                                             /**< Shift value for SMU_HFXO0                   */
#define _SMU_PPUSATD1_HFXO0_MASK                           0x4000UL                                       /**< Bit mask for SMU_HFXO0                      */
#define _SMU_PPUSATD1_HFXO0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_HFXO0_DEFAULT                         (_SMU_PPUSATD1_HFXO0_DEFAULT << 14)            /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_I2C0                                  (0x1UL << 15)                                  /**< I2C0 Secure Access                          */
#define _SMU_PPUSATD1_I2C0_SHIFT                           15                                             /**< Shift value for SMU_I2C0                    */
#define _SMU_PPUSATD1_I2C0_MASK                            0x8000UL                                       /**< Bit mask for SMU_I2C0                       */
#define _SMU_PPUSATD1_I2C0_DEFAULT                         0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_I2C0_DEFAULT                          (_SMU_PPUSATD1_I2C0_DEFAULT << 15)             /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_WDOG0                                 (0x1UL << 16)                                  /**< WDOG0 Secure Access                         */
#define _SMU_PPUSATD1_WDOG0_SHIFT                          16                                             /**< Shift value for SMU_WDOG0                   */
#define _SMU_PPUSATD1_WDOG0_MASK                           0x10000UL                                      /**< Bit mask for SMU_WDOG0                      */
#define _SMU_PPUSATD1_WDOG0_DEFAULT                        0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_WDOG0_DEFAULT                         (_SMU_PPUSATD1_WDOG0_DEFAULT << 16)            /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_EUSART0                               (0x1UL << 17)                                  /**< EUSART0 Secure Access                       */
#define _SMU_PPUSATD1_EUSART0_SHIFT                        17                                             /**< Shift value for SMU_EUSART0                 */
#define _SMU_PPUSATD1_EUSART0_MASK                         0x20000UL                                      /**< Bit mask for SMU_EUSART0                    */
#define _SMU_PPUSATD1_EUSART0_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_EUSART0_DEFAULT                       (_SMU_PPUSATD1_EUSART0_DEFAULT << 17)          /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SYMCRYPTODMACTRL                      (0x1UL << 18)                                  /**< SYMCRYPTODMACTRL Secure Access              */
#define _SMU_PPUSATD1_SYMCRYPTODMACTRL_SHIFT               18                                             /**< Shift value for SMU_SYMCRYPTODMACTRL        */
#define _SMU_PPUSATD1_SYMCRYPTODMACTRL_MASK                0x40000UL                                      /**< Bit mask for SMU_SYMCRYPTODMACTRL           */
#define _SMU_PPUSATD1_SYMCRYPTODMACTRL_DEFAULT             0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SYMCRYPTODMACTRL_DEFAULT              (_SMU_PPUSATD1_SYMCRYPTODMACTRL_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_LPWAESDMACTRL                         (0x1UL << 19)                                  /**< LPWAESDMACTRL Secure Access                 */
#define _SMU_PPUSATD1_LPWAESDMACTRL_SHIFT                  19                                             /**< Shift value for SMU_LPWAESDMACTRL           */
#define _SMU_PPUSATD1_LPWAESDMACTRL_MASK                   0x80000UL                                      /**< Bit mask for SMU_LPWAESDMACTRL              */
#define _SMU_PPUSATD1_LPWAESDMACTRL_DEFAULT                0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_LPWAESDMACTRL_DEFAULT                 (_SMU_PPUSATD1_LPWAESDMACTRL_DEFAULT << 19)    /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SMU                                   (0x1UL << 20)                                  /**< SMU Secure Access                           */
#define _SMU_PPUSATD1_SMU_SHIFT                            20                                             /**< Shift value for SMU_SMU                     */
#define _SMU_PPUSATD1_SMU_MASK                             0x100000UL                                     /**< Bit mask for SMU_SMU                        */
#define _SMU_PPUSATD1_SMU_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SMU_DEFAULT                           (_SMU_PPUSATD1_SMU_DEFAULT << 20)              /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SMUCFGNS                              (0x1UL << 21)                                  /**< SMUCFGNS Secure Access                      */
#define _SMU_PPUSATD1_SMUCFGNS_SHIFT                       21                                             /**< Shift value for SMU_SMUCFGNS                */
#define _SMU_PPUSATD1_SMUCFGNS_MASK                        0x200000UL                                     /**< Bit mask for SMU_SMUCFGNS                   */
#define _SMU_PPUSATD1_SMUCFGNS_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SMUCFGNS_DEFAULT                      (_SMU_PPUSATD1_SMUCFGNS_DEFAULT << 21)         /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SEMAILBOX                             (0x1UL << 22)                                  /**< SEMAILBOX Secure Access                     */
#define _SMU_PPUSATD1_SEMAILBOX_SHIFT                      22                                             /**< Shift value for SMU_SEMAILBOX               */
#define _SMU_PPUSATD1_SEMAILBOX_MASK                       0x400000UL                                     /**< Bit mask for SMU_SEMAILBOX                  */
#define _SMU_PPUSATD1_SEMAILBOX_DEFAULT                    0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SEMAILBOX_DEFAULT                     (_SMU_PPUSATD1_SEMAILBOX_DEFAULT << 22)        /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_QSPI0MANUAL                           (0x1UL << 23)                                  /**< QSPI0MANUAL Secure Access                   */
#define _SMU_PPUSATD1_QSPI0MANUAL_SHIFT                    23                                             /**< Shift value for SMU_QSPI0MANUAL             */
#define _SMU_PPUSATD1_QSPI0MANUAL_MASK                     0x800000UL                                     /**< Bit mask for SMU_QSPI0MANUAL                */
#define _SMU_PPUSATD1_QSPI0MANUAL_DEFAULT                  0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_QSPI0MANUAL_DEFAULT                   (_SMU_PPUSATD1_QSPI0MANUAL_DEFAULT << 23)      /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_AHBRADIO                              (0x1UL << 24)                                  /**< AHBRADIO Secure Access                      */
#define _SMU_PPUSATD1_AHBRADIO_SHIFT                       24                                             /**< Shift value for SMU_AHBRADIO                */
#define _SMU_PPUSATD1_AHBRADIO_MASK                        0x1000000UL                                    /**< Bit mask for SMU_AHBRADIO                   */
#define _SMU_PPUSATD1_AHBRADIO_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_AHBRADIO_DEFAULT                      (_SMU_PPUSATD1_AHBRADIO_DEFAULT << 24)         /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SYSRTC0                               (0x1UL << 25)                                  /**< SYSRTC0 Secure Access                       */
#define _SMU_PPUSATD1_SYSRTC0_SHIFT                        25                                             /**< Shift value for SMU_SYSRTC0                 */
#define _SMU_PPUSATD1_SYSRTC0_MASK                         0x2000000UL                                    /**< Bit mask for SMU_SYSRTC0                    */
#define _SMU_PPUSATD1_SYSRTC0_DEFAULT                      0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SYSRTC0_DEFAULT                       (_SMU_PPUSATD1_SYSRTC0_DEFAULT << 25)          /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SCRATCHPAD                            (0x1UL << 26)                                  /**< SCRATCHPAD Secure Access                    */
#define _SMU_PPUSATD1_SCRATCHPAD_SHIFT                     26                                             /**< Shift value for SMU_SCRATCHPAD              */
#define _SMU_PPUSATD1_SCRATCHPAD_MASK                      0x4000000UL                                    /**< Bit mask for SMU_SCRATCHPAD                 */
#define _SMU_PPUSATD1_SCRATCHPAD_DEFAULT                   0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SCRATCHPAD_DEFAULT                    (_SMU_PPUSATD1_SCRATCHPAD_DEFAULT << 26)       /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_EMU                                   (0x1UL << 27)                                  /**< EMU Secure Access                           */
#define _SMU_PPUSATD1_EMU_SHIFT                            27                                             /**< Shift value for SMU_EMU                     */
#define _SMU_PPUSATD1_EMU_MASK                             0x8000000UL                                    /**< Bit mask for SMU_EMU                        */
#define _SMU_PPUSATD1_EMU_DEFAULT                          0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_EMU_DEFAULT                           (_SMU_PPUSATD1_EMU_DEFAULT << 27)              /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_EMUCFGNS                              (0x1UL << 28)                                  /**< EMUCFGNS Secure Access                      */
#define _SMU_PPUSATD1_EMUCFGNS_SHIFT                       28                                             /**< Shift value for SMU_EMUCFGNS                */
#define _SMU_PPUSATD1_EMUCFGNS_MASK                        0x10000000UL                                   /**< Bit mask for SMU_EMUCFGNS                   */
#define _SMU_PPUSATD1_EMUCFGNS_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_EMUCFGNS_DEFAULT                      (_SMU_PPUSATD1_EMUCFGNS_DEFAULT << 28)         /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_SEPORTAL                              (0x1UL << 29)                                  /**< SEPORTAL Secure Access                      */
#define _SMU_PPUSATD1_SEPORTAL_SHIFT                       29                                             /**< Shift value for SMU_SEPORTAL                */
#define _SMU_PPUSATD1_SEPORTAL_MASK                        0x20000000UL                                   /**< Bit mask for SMU_SEPORTAL                   */
#define _SMU_PPUSATD1_SEPORTAL_DEFAULT                     0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_SEPORTAL_DEFAULT                      (_SMU_PPUSATD1_SEPORTAL_DEFAULT << 29)         /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_LPW0PORTAL                            (0x1UL << 30)                                  /**< LPW0PORTAL Secure Access                    */
#define _SMU_PPUSATD1_LPW0PORTAL_SHIFT                     30                                             /**< Shift value for SMU_LPW0PORTAL              */
#define _SMU_PPUSATD1_LPW0PORTAL_MASK                      0x40000000UL                                   /**< Bit mask for SMU_LPW0PORTAL                 */
#define _SMU_PPUSATD1_LPW0PORTAL_DEFAULT                   0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_LPW0PORTAL_DEFAULT                    (_SMU_PPUSATD1_LPW0PORTAL_DEFAULT << 30)       /**< Shifted mode DEFAULT for SMU_PPUSATD1       */
#define SMU_PPUSATD1_HOSTPORTAL                            (0x1UL << 31)                                  /**< HOSTPORTAL Secure Access                    */
#define _SMU_PPUSATD1_HOSTPORTAL_SHIFT                     31                                             /**< Shift value for SMU_HOSTPORTAL              */
#define _SMU_PPUSATD1_HOSTPORTAL_MASK                      0x80000000UL                                   /**< Bit mask for SMU_HOSTPORTAL                 */
#define _SMU_PPUSATD1_HOSTPORTAL_DEFAULT                   0x00000001UL                                   /**< Mode DEFAULT for SMU_PPUSATD1               */
#define SMU_PPUSATD1_HOSTPORTAL_DEFAULT                    (_SMU_PPUSATD1_HOSTPORTAL_DEFAULT << 31)       /**< Shifted mode DEFAULT for SMU_PPUSATD1       */

/* Bit fields for SMU PPURSPERRD */
#define _SMU_PPURSPERRD_RESETVALUE                         0x00000000UL                              /**< Default value for SMU_PPURSPERRD            */
#define _SMU_PPURSPERRD_MASK                               0x00000001UL                              /**< Mask for SMU_PPURSPERRD                     */
#define SMU_PPURSPERRD_PPURSPERRD                          (0x1UL << 0)                              /**< PPU Respond with Error                      */
#define _SMU_PPURSPERRD_PPURSPERRD_SHIFT                   0                                         /**< Shift value for SMU_PPURSPERRD              */
#define _SMU_PPURSPERRD_PPURSPERRD_MASK                    0x1UL                                     /**< Bit mask for SMU_PPURSPERRD                 */
#define _SMU_PPURSPERRD_PPURSPERRD_DEFAULT                 0x00000000UL                              /**< Mode DEFAULT for SMU_PPURSPERRD             */
#define SMU_PPURSPERRD_PPURSPERRD_DEFAULT                  (_SMU_PPURSPERRD_PPURSPERRD_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_PPURSPERRD     */

/* Bit fields for SMU PPUFS */
#define _SMU_PPUFS_RESETVALUE                              0x00000000UL                            /**< Default value for SMU_PPUFS                 */
#define _SMU_PPUFS_MASK                                    0x000000FFUL                            /**< Mask for SMU_PPUFS                          */
#define _SMU_PPUFS_PPUFSPERIPHID_SHIFT                     0                                       /**< Shift value for SMU_PPUFSPERIPHID           */
#define _SMU_PPUFS_PPUFSPERIPHID_MASK                      0xFFUL                                  /**< Bit mask for SMU_PPUFSPERIPHID              */
#define _SMU_PPUFS_PPUFSPERIPHID_DEFAULT                   0x00000000UL                            /**< Mode DEFAULT for SMU_PPUFS                  */
#define SMU_PPUFS_PPUFSPERIPHID_DEFAULT                    (_SMU_PPUFS_PPUFSPERIPHID_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_PPUFS          */

/* Bit fields for SMU PPUFSADDR */
#define _SMU_PPUFSADDR_RESETVALUE                          0x00000000UL                            /**< Default value for SMU_PPUFSADDR             */
#define _SMU_PPUFSADDR_MASK                                0xFFFFFFFFUL                            /**< Mask for SMU_PPUFSADDR                      */
#define _SMU_PPUFSADDR_PPUFSADDR_SHIFT                     0                                       /**< Shift value for SMU_PPUFSADDR               */
#define _SMU_PPUFSADDR_PPUFSADDR_MASK                      0xFFFFFFFFUL                            /**< Bit mask for SMU_PPUFSADDR                  */
#define _SMU_PPUFSADDR_PPUFSADDR_DEFAULT                   0x00000000UL                            /**< Mode DEFAULT for SMU_PPUFSADDR              */
#define SMU_PPUFSADDR_PPUFSADDR_DEFAULT                    (_SMU_PPUFSADDR_PPUFSADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_PPUFSADDR      */

/* Bit fields for SMU BMPUPATD0 */
#define _SMU_BMPUPATD0_RESETVALUE                          0x000000FFUL                               /**< Default value for SMU_BMPUPATD0             */
#define _SMU_BMPUPATD0_MASK                                0x000000FFUL                               /**< Mask for SMU_BMPUPATD0                      */
#define SMU_BMPUPATD0_LPWAESDMA                            (0x1UL << 0)                               /**< LPWAESDMA Privileged Mode                   */
#define _SMU_BMPUPATD0_LPWAESDMA_SHIFT                     0                                          /**< Shift value for SMU_LPWAESDMA               */
#define _SMU_BMPUPATD0_LPWAESDMA_MASK                      0x1UL                                      /**< Bit mask for SMU_LPWAESDMA                  */
#define _SMU_BMPUPATD0_LPWAESDMA_DEFAULT                   0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_LPWAESDMA_DEFAULT                    (_SMU_BMPUPATD0_LPWAESDMA_DEFAULT << 0)    /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_AHBRADIOBUS                          (0x1UL << 1)                               /**< AHBRADIOBUS Privileged Mode                 */
#define _SMU_BMPUPATD0_AHBRADIOBUS_SHIFT                   1                                          /**< Shift value for SMU_AHBRADIOBUS             */
#define _SMU_BMPUPATD0_AHBRADIOBUS_MASK                    0x2UL                                      /**< Bit mask for SMU_AHBRADIOBUS                */
#define _SMU_BMPUPATD0_AHBRADIOBUS_DEFAULT                 0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_AHBRADIOBUS_DEFAULT                  (_SMU_BMPUPATD0_AHBRADIOBUS_DEFAULT << 1)  /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_LDMA0PORT0                           (0x1UL << 2)                               /**< LDMA0PORT0 Privileged Mode                  */
#define _SMU_BMPUPATD0_LDMA0PORT0_SHIFT                    2                                          /**< Shift value for SMU_LDMA0PORT0              */
#define _SMU_BMPUPATD0_LDMA0PORT0_MASK                     0x4UL                                      /**< Bit mask for SMU_LDMA0PORT0                 */
#define _SMU_BMPUPATD0_LDMA0PORT0_DEFAULT                  0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_LDMA0PORT0_DEFAULT                   (_SMU_BMPUPATD0_LDMA0PORT0_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_LDMA0PORT1                           (0x1UL << 3)                               /**< LDMA0PORT1 Privileged Mode                  */
#define _SMU_BMPUPATD0_LDMA0PORT1_SHIFT                    3                                          /**< Shift value for SMU_LDMA0PORT1              */
#define _SMU_BMPUPATD0_LDMA0PORT1_MASK                     0x8UL                                      /**< Bit mask for SMU_LDMA0PORT1                 */
#define _SMU_BMPUPATD0_LDMA0PORT1_DEFAULT                  0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_LDMA0PORT1_DEFAULT                   (_SMU_BMPUPATD0_LDMA0PORT1_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_RPASLAXI                             (0x1UL << 4)                               /**< RPASLAXI Privileged Mode                    */
#define _SMU_BMPUPATD0_RPASLAXI_SHIFT                      4                                          /**< Shift value for SMU_RPASLAXI                */
#define _SMU_BMPUPATD0_RPASLAXI_MASK                       0x10UL                                     /**< Bit mask for SMU_RPASLAXI                   */
#define _SMU_BMPUPATD0_RPASLAXI_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_RPASLAXI_DEFAULT                     (_SMU_BMPUPATD0_RPASLAXI_DEFAULT << 4)     /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_SEEXTDMAPORT                         (0x1UL << 5)                               /**< SEEXTDMAPORT Privileged Mode                */
#define _SMU_BMPUPATD0_SEEXTDMAPORT_SHIFT                  5                                          /**< Shift value for SMU_SEEXTDMAPORT            */
#define _SMU_BMPUPATD0_SEEXTDMAPORT_MASK                   0x20UL                                     /**< Bit mask for SMU_SEEXTDMAPORT               */
#define _SMU_BMPUPATD0_SEEXTDMAPORT_DEFAULT                0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_SEEXTDMAPORT_DEFAULT                 (_SMU_BMPUPATD0_SEEXTDMAPORT_DEFAULT << 5) /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_SYMCRYPTODMA                         (0x1UL << 6)                               /**< SYMCRYPTODMA Privileged Mode                */
#define _SMU_BMPUPATD0_SYMCRYPTODMA_SHIFT                  6                                          /**< Shift value for SMU_SYMCRYPTODMA            */
#define _SMU_BMPUPATD0_SYMCRYPTODMA_MASK                   0x40UL                                     /**< Bit mask for SMU_SYMCRYPTODMA               */
#define _SMU_BMPUPATD0_SYMCRYPTODMA_DEFAULT                0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_SYMCRYPTODMA_DEFAULT                 (_SMU_BMPUPATD0_SYMCRYPTODMA_DEFAULT << 6) /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */
#define SMU_BMPUPATD0_SLAXICOREBUS                         (0x1UL << 7)                               /**< SLAXICOREBUS Privileged Mode                */
#define _SMU_BMPUPATD0_SLAXICOREBUS_SHIFT                  7                                          /**< Shift value for SMU_SLAXICOREBUS            */
#define _SMU_BMPUPATD0_SLAXICOREBUS_MASK                   0x80UL                                     /**< Bit mask for SMU_SLAXICOREBUS               */
#define _SMU_BMPUPATD0_SLAXICOREBUS_DEFAULT                0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUPATD0              */
#define SMU_BMPUPATD0_SLAXICOREBUS_DEFAULT                 (_SMU_BMPUPATD0_SLAXICOREBUS_DEFAULT << 7) /**< Shifted mode DEFAULT for SMU_BMPUPATD0      */

/* Bit fields for SMU BMPUSATD0 */
#define _SMU_BMPUSATD0_RESETVALUE                          0x000000FFUL                               /**< Default value for SMU_BMPUSATD0             */
#define _SMU_BMPUSATD0_MASK                                0x000000FFUL                               /**< Mask for SMU_BMPUSATD0                      */
#define SMU_BMPUSATD0_LPWAESDMA                            (0x1UL << 0)                               /**< LPWAESDMA Secure Mode                       */
#define _SMU_BMPUSATD0_LPWAESDMA_SHIFT                     0                                          /**< Shift value for SMU_LPWAESDMA               */
#define _SMU_BMPUSATD0_LPWAESDMA_MASK                      0x1UL                                      /**< Bit mask for SMU_LPWAESDMA                  */
#define _SMU_BMPUSATD0_LPWAESDMA_DEFAULT                   0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_LPWAESDMA_DEFAULT                    (_SMU_BMPUSATD0_LPWAESDMA_DEFAULT << 0)    /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_AHBRADIOBUS                          (0x1UL << 1)                               /**< AHBRADIOBUS Secure Mode                     */
#define _SMU_BMPUSATD0_AHBRADIOBUS_SHIFT                   1                                          /**< Shift value for SMU_AHBRADIOBUS             */
#define _SMU_BMPUSATD0_AHBRADIOBUS_MASK                    0x2UL                                      /**< Bit mask for SMU_AHBRADIOBUS                */
#define _SMU_BMPUSATD0_AHBRADIOBUS_DEFAULT                 0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_AHBRADIOBUS_DEFAULT                  (_SMU_BMPUSATD0_AHBRADIOBUS_DEFAULT << 1)  /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_LDMA0PORT0                           (0x1UL << 2)                               /**< LDMA0PORT0 Secure Mode                      */
#define _SMU_BMPUSATD0_LDMA0PORT0_SHIFT                    2                                          /**< Shift value for SMU_LDMA0PORT0              */
#define _SMU_BMPUSATD0_LDMA0PORT0_MASK                     0x4UL                                      /**< Bit mask for SMU_LDMA0PORT0                 */
#define _SMU_BMPUSATD0_LDMA0PORT0_DEFAULT                  0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_LDMA0PORT0_DEFAULT                   (_SMU_BMPUSATD0_LDMA0PORT0_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_LDMA0PORT1                           (0x1UL << 3)                               /**< LDMA0PORT1 Secure Mode                      */
#define _SMU_BMPUSATD0_LDMA0PORT1_SHIFT                    3                                          /**< Shift value for SMU_LDMA0PORT1              */
#define _SMU_BMPUSATD0_LDMA0PORT1_MASK                     0x8UL                                      /**< Bit mask for SMU_LDMA0PORT1                 */
#define _SMU_BMPUSATD0_LDMA0PORT1_DEFAULT                  0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_LDMA0PORT1_DEFAULT                   (_SMU_BMPUSATD0_LDMA0PORT1_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_RPASLAXI                             (0x1UL << 4)                               /**< RPASLAXI Secure Mode                        */
#define _SMU_BMPUSATD0_RPASLAXI_SHIFT                      4                                          /**< Shift value for SMU_RPASLAXI                */
#define _SMU_BMPUSATD0_RPASLAXI_MASK                       0x10UL                                     /**< Bit mask for SMU_RPASLAXI                   */
#define _SMU_BMPUSATD0_RPASLAXI_DEFAULT                    0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_RPASLAXI_DEFAULT                     (_SMU_BMPUSATD0_RPASLAXI_DEFAULT << 4)     /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_SEEXTDMAPORT                         (0x1UL << 5)                               /**< SEEXTDMAPORT Secure Mode                    */
#define _SMU_BMPUSATD0_SEEXTDMAPORT_SHIFT                  5                                          /**< Shift value for SMU_SEEXTDMAPORT            */
#define _SMU_BMPUSATD0_SEEXTDMAPORT_MASK                   0x20UL                                     /**< Bit mask for SMU_SEEXTDMAPORT               */
#define _SMU_BMPUSATD0_SEEXTDMAPORT_DEFAULT                0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_SEEXTDMAPORT_DEFAULT                 (_SMU_BMPUSATD0_SEEXTDMAPORT_DEFAULT << 5) /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_SYMCRYPTODMA                         (0x1UL << 6)                               /**< SYMCRYPTODMA Secure Mode                    */
#define _SMU_BMPUSATD0_SYMCRYPTODMA_SHIFT                  6                                          /**< Shift value for SMU_SYMCRYPTODMA            */
#define _SMU_BMPUSATD0_SYMCRYPTODMA_MASK                   0x40UL                                     /**< Bit mask for SMU_SYMCRYPTODMA               */
#define _SMU_BMPUSATD0_SYMCRYPTODMA_DEFAULT                0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_SYMCRYPTODMA_DEFAULT                 (_SMU_BMPUSATD0_SYMCRYPTODMA_DEFAULT << 6) /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */
#define SMU_BMPUSATD0_SLAXICOREBUS                         (0x1UL << 7)                               /**< SLAXICOREBUS Secure Mode                    */
#define _SMU_BMPUSATD0_SLAXICOREBUS_SHIFT                  7                                          /**< Shift value for SMU_SLAXICOREBUS            */
#define _SMU_BMPUSATD0_SLAXICOREBUS_MASK                   0x80UL                                     /**< Bit mask for SMU_SLAXICOREBUS               */
#define _SMU_BMPUSATD0_SLAXICOREBUS_DEFAULT                0x00000001UL                               /**< Mode DEFAULT for SMU_BMPUSATD0              */
#define SMU_BMPUSATD0_SLAXICOREBUS_DEFAULT                 (_SMU_BMPUSATD0_SLAXICOREBUS_DEFAULT << 7) /**< Shifted mode DEFAULT for SMU_BMPUSATD0      */

/* Bit fields for SMU BMPURSPERRD0 */
#define _SMU_BMPURSPERRD0_RESETVALUE                       0x00000000UL                                  /**< Default value for SMU_BMPURSPERRD0          */
#define _SMU_BMPURSPERRD0_MASK                             0x000000FFUL                                  /**< Mask for SMU_BMPURSPERRD0                   */
#define SMU_BMPURSPERRD0_LPWAESDMA                         (0x1UL << 0)                                  /**< LPWAESDMA Respond with Error                */
#define _SMU_BMPURSPERRD0_LPWAESDMA_SHIFT                  0                                             /**< Shift value for SMU_LPWAESDMA               */
#define _SMU_BMPURSPERRD0_LPWAESDMA_MASK                   0x1UL                                         /**< Bit mask for SMU_LPWAESDMA                  */
#define _SMU_BMPURSPERRD0_LPWAESDMA_DEFAULT                0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_LPWAESDMA_DEFAULT                 (_SMU_BMPURSPERRD0_LPWAESDMA_DEFAULT << 0)    /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_AHBRADIOBUS                       (0x1UL << 1)                                  /**< AHBRADIOBUS Respond with Error              */
#define _SMU_BMPURSPERRD0_AHBRADIOBUS_SHIFT                1                                             /**< Shift value for SMU_AHBRADIOBUS             */
#define _SMU_BMPURSPERRD0_AHBRADIOBUS_MASK                 0x2UL                                         /**< Bit mask for SMU_AHBRADIOBUS                */
#define _SMU_BMPURSPERRD0_AHBRADIOBUS_DEFAULT              0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_AHBRADIOBUS_DEFAULT               (_SMU_BMPURSPERRD0_AHBRADIOBUS_DEFAULT << 1)  /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_LDMA0PORT0                        (0x1UL << 2)                                  /**< LDMA0PORT0 Respond with Error               */
#define _SMU_BMPURSPERRD0_LDMA0PORT0_SHIFT                 2                                             /**< Shift value for SMU_LDMA0PORT0              */
#define _SMU_BMPURSPERRD0_LDMA0PORT0_MASK                  0x4UL                                         /**< Bit mask for SMU_LDMA0PORT0                 */
#define _SMU_BMPURSPERRD0_LDMA0PORT0_DEFAULT               0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_LDMA0PORT0_DEFAULT                (_SMU_BMPURSPERRD0_LDMA0PORT0_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_LDMA0PORT1                        (0x1UL << 3)                                  /**< LDMA0PORT1 Respond with Error               */
#define _SMU_BMPURSPERRD0_LDMA0PORT1_SHIFT                 3                                             /**< Shift value for SMU_LDMA0PORT1              */
#define _SMU_BMPURSPERRD0_LDMA0PORT1_MASK                  0x8UL                                         /**< Bit mask for SMU_LDMA0PORT1                 */
#define _SMU_BMPURSPERRD0_LDMA0PORT1_DEFAULT               0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_LDMA0PORT1_DEFAULT                (_SMU_BMPURSPERRD0_LDMA0PORT1_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_RPASLAXI                          (0x1UL << 4)                                  /**< RPASLAXI Respond with Error                 */
#define _SMU_BMPURSPERRD0_RPASLAXI_SHIFT                   4                                             /**< Shift value for SMU_RPASLAXI                */
#define _SMU_BMPURSPERRD0_RPASLAXI_MASK                    0x10UL                                        /**< Bit mask for SMU_RPASLAXI                   */
#define _SMU_BMPURSPERRD0_RPASLAXI_DEFAULT                 0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_RPASLAXI_DEFAULT                  (_SMU_BMPURSPERRD0_RPASLAXI_DEFAULT << 4)     /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_SEEXTDMAPORT                      (0x1UL << 5)                                  /**< SEEXTDMAPORT Respond with Error             */
#define _SMU_BMPURSPERRD0_SEEXTDMAPORT_SHIFT               5                                             /**< Shift value for SMU_SEEXTDMAPORT            */
#define _SMU_BMPURSPERRD0_SEEXTDMAPORT_MASK                0x20UL                                        /**< Bit mask for SMU_SEEXTDMAPORT               */
#define _SMU_BMPURSPERRD0_SEEXTDMAPORT_DEFAULT             0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_SEEXTDMAPORT_DEFAULT              (_SMU_BMPURSPERRD0_SEEXTDMAPORT_DEFAULT << 5) /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_SYMCRYPTODMA                      (0x1UL << 6)                                  /**< SYMCRYPTODMA Respond with Error             */
#define _SMU_BMPURSPERRD0_SYMCRYPTODMA_SHIFT               6                                             /**< Shift value for SMU_SYMCRYPTODMA            */
#define _SMU_BMPURSPERRD0_SYMCRYPTODMA_MASK                0x40UL                                        /**< Bit mask for SMU_SYMCRYPTODMA               */
#define _SMU_BMPURSPERRD0_SYMCRYPTODMA_DEFAULT             0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_SYMCRYPTODMA_DEFAULT              (_SMU_BMPURSPERRD0_SYMCRYPTODMA_DEFAULT << 6) /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */
#define SMU_BMPURSPERRD0_SLAXICOREBUS                      (0x1UL << 7)                                  /**< SLAXICOREBUS Respond with Error             */
#define _SMU_BMPURSPERRD0_SLAXICOREBUS_SHIFT               7                                             /**< Shift value for SMU_SLAXICOREBUS            */
#define _SMU_BMPURSPERRD0_SLAXICOREBUS_MASK                0x80UL                                        /**< Bit mask for SMU_SLAXICOREBUS               */
#define _SMU_BMPURSPERRD0_SLAXICOREBUS_DEFAULT             0x00000000UL                                  /**< Mode DEFAULT for SMU_BMPURSPERRD0           */
#define SMU_BMPURSPERRD0_SLAXICOREBUS_DEFAULT              (_SMU_BMPURSPERRD0_SLAXICOREBUS_DEFAULT << 7) /**< Shifted mode DEFAULT for SMU_BMPURSPERRD0   */

/* Bit fields for SMU BMPUFS */
#define _SMU_BMPUFS_RESETVALUE                             0x00000000UL                              /**< Default value for SMU_BMPUFS                */
#define _SMU_BMPUFS_MASK                                   0x000000FFUL                              /**< Mask for SMU_BMPUFS                         */
#define _SMU_BMPUFS_BMPUFSMASTERID_SHIFT                   0                                         /**< Shift value for SMU_BMPUFSMASTERID          */
#define _SMU_BMPUFS_BMPUFSMASTERID_MASK                    0xFFUL                                    /**< Bit mask for SMU_BMPUFSMASTERID             */
#define _SMU_BMPUFS_BMPUFSMASTERID_DEFAULT                 0x00000000UL                              /**< Mode DEFAULT for SMU_BMPUFS                 */
#define SMU_BMPUFS_BMPUFSMASTERID_DEFAULT                  (_SMU_BMPUFS_BMPUFSMASTERID_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_BMPUFS         */

/* Bit fields for SMU BMPUFSADDR */
#define _SMU_BMPUFSADDR_RESETVALUE                         0x00000000UL                              /**< Default value for SMU_BMPUFSADDR            */
#define _SMU_BMPUFSADDR_MASK                               0xFFFFFFFFUL                              /**< Mask for SMU_BMPUFSADDR                     */
#define _SMU_BMPUFSADDR_BMPUFSADDR_SHIFT                   0                                         /**< Shift value for SMU_BMPUFSADDR              */
#define _SMU_BMPUFSADDR_BMPUFSADDR_MASK                    0xFFFFFFFFUL                              /**< Bit mask for SMU_BMPUFSADDR                 */
#define _SMU_BMPUFSADDR_BMPUFSADDR_DEFAULT                 0x00000000UL                              /**< Mode DEFAULT for SMU_BMPUFSADDR             */
#define SMU_BMPUFSADDR_BMPUFSADDR_DEFAULT                  (_SMU_BMPUFSADDR_BMPUFSADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_BMPUFSADDR     */

/* Bit fields for SMU MSPUDMEMREGIONSIZE */
#define _SMU_MSPUDMEMREGIONSIZE_RESETVALUE                 0x00000000UL                                        /**< Default value for SMU_MSPUDMEMREGIONSIZE    */
#define _SMU_MSPUDMEMREGIONSIZE_MASK                       0x00000007UL                                        /**< Mask for SMU_MSPUDMEMREGIONSIZE             */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_SHIFT            0                                                   /**< Shift value for SMU_MSPURSIZE               */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MASK             0x7UL                                               /**< Bit mask for SMU_MSPURSIZE                  */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_DEFAULT          0x00000000UL                                        /**< Mode DEFAULT for SMU_MSPUDMEMREGIONSIZE     */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_16KB        0x00000000UL                                        /**< Mode MSPU_16KB for SMU_MSPUDMEMREGIONSIZE   */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_32KB        0x00000001UL                                        /**< Mode MSPU_32KB for SMU_MSPUDMEMREGIONSIZE   */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_64KB        0x00000002UL                                        /**< Mode MSPU_64KB for SMU_MSPUDMEMREGIONSIZE   */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_128KB       0x00000003UL                                        /**< Mode MSPU_128KB for SMU_MSPUDMEMREGIONSIZE  */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_256KB       0x00000004UL                                        /**< Mode MSPU_256KB for SMU_MSPUDMEMREGIONSIZE  */
#define _SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_512KB       0x00000005UL                                        /**< Mode MSPU_512KB for SMU_MSPUDMEMREGIONSIZE  */
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_DEFAULT           (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_DEFAULT << 0)    /**< Shifted mode DEFAULT for SMU_MSPUDMEMREGIONSIZE*/
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_16KB         (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_16KB << 0)  /**< Shifted mode MSPU_16KB for SMU_MSPUDMEMREGIONSIZE*/
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_32KB         (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_32KB << 0)  /**< Shifted mode MSPU_32KB for SMU_MSPUDMEMREGIONSIZE*/
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_64KB         (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_64KB << 0)  /**< Shifted mode MSPU_64KB for SMU_MSPUDMEMREGIONSIZE*/
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_128KB        (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_128KB << 0) /**< Shifted mode MSPU_128KB for SMU_MSPUDMEMREGIONSIZE*/
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_256KB        (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_256KB << 0) /**< Shifted mode MSPU_256KB for SMU_MSPUDMEMREGIONSIZE*/
#define SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_512KB        (_SMU_MSPUDMEMREGIONSIZE_MSPURSIZE_MSPU_512KB << 0) /**< Shifted mode MSPU_512KB for SMU_MSPUDMEMREGIONSIZE*/

/* Bit fields for SMU MSPUNVMREGIONSIZE */
#define _SMU_MSPUNVMREGIONSIZE_RESETVALUE                  0x00000000UL                                       /**< Default value for SMU_MSPUNVMREGIONSIZE     */
#define _SMU_MSPUNVMREGIONSIZE_MASK                        0x00000007UL                                       /**< Mask for SMU_MSPUNVMREGIONSIZE              */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_SHIFT             0                                                  /**< Shift value for SMU_MSPURSIZE               */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MASK              0x7UL                                              /**< Bit mask for SMU_MSPURSIZE                  */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_DEFAULT           0x00000000UL                                       /**< Mode DEFAULT for SMU_MSPUNVMREGIONSIZE      */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB         0x00000000UL                                       /**< Mode MSPU_16KB for SMU_MSPUNVMREGIONSIZE    */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_32KB         0x00000001UL                                       /**< Mode MSPU_32KB for SMU_MSPUNVMREGIONSIZE    */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_64KB         0x00000002UL                                       /**< Mode MSPU_64KB for SMU_MSPUNVMREGIONSIZE    */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_128KB        0x00000003UL                                       /**< Mode MSPU_128KB for SMU_MSPUNVMREGIONSIZE   */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_256KB        0x00000004UL                                       /**< Mode MSPU_256KB for SMU_MSPUNVMREGIONSIZE   */
#define _SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_512KB        0x00000005UL                                       /**< Mode MSPU_512KB for SMU_MSPUNVMREGIONSIZE   */
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_DEFAULT            (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_DEFAULT << 0)    /**< Shifted mode DEFAULT for SMU_MSPUNVMREGIONSIZE*/
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB          (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_16KB << 0)  /**< Shifted mode MSPU_16KB for SMU_MSPUNVMREGIONSIZE*/
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_32KB          (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_32KB << 0)  /**< Shifted mode MSPU_32KB for SMU_MSPUNVMREGIONSIZE*/
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_64KB          (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_64KB << 0)  /**< Shifted mode MSPU_64KB for SMU_MSPUNVMREGIONSIZE*/
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_128KB         (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_128KB << 0) /**< Shifted mode MSPU_128KB for SMU_MSPUNVMREGIONSIZE*/
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_256KB         (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_256KB << 0) /**< Shifted mode MSPU_256KB for SMU_MSPUNVMREGIONSIZE*/
#define SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_512KB         (_SMU_MSPUNVMREGIONSIZE_MSPURSIZE_MSPU_512KB << 0) /**< Shifted mode MSPU_512KB for SMU_MSPUNVMREGIONSIZE*/

/* Bit fields for SMU MSPUDMEMRSPERRD */
#define _SMU_MSPUDMEMRSPERRD_RESETVALUE                    0x00000001UL                                     /**< Default value for SMU_MSPUDMEMRSPERRD       */
#define _SMU_MSPUDMEMRSPERRD_MASK                          0x00000001UL                                     /**< Mask for SMU_MSPUDMEMRSPERRD                */
#define SMU_MSPUDMEMRSPERRD_MSPURSPERRD0                   (0x1UL << 0)                                     /**< DMEM MSPU Respond with Error                */
#define _SMU_MSPUDMEMRSPERRD_MSPURSPERRD0_SHIFT            0                                                /**< Shift value for SMU_MSPURSPERRD0            */
#define _SMU_MSPUDMEMRSPERRD_MSPURSPERRD0_MASK             0x1UL                                            /**< Bit mask for SMU_MSPURSPERRD0               */
#define _SMU_MSPUDMEMRSPERRD_MSPURSPERRD0_DEFAULT          0x00000001UL                                     /**< Mode DEFAULT for SMU_MSPUDMEMRSPERRD        */
#define SMU_MSPUDMEMRSPERRD_MSPURSPERRD0_DEFAULT           (_SMU_MSPUDMEMRSPERRD_MSPURSPERRD0_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_MSPUDMEMRSPERRD*/

/* Bit fields for SMU MSPUNVMRSPERRD */
#define _SMU_MSPUNVMRSPERRD_RESETVALUE                     0x00000001UL                                    /**< Default value for SMU_MSPUNVMRSPERRD        */
#define _SMU_MSPUNVMRSPERRD_MASK                           0x00000001UL                                    /**< Mask for SMU_MSPUNVMRSPERRD                 */
#define SMU_MSPUNVMRSPERRD_MSPURSPERRD0                    (0x1UL << 0)                                    /**< NVM MSPU Respond with Error                 */
#define _SMU_MSPUNVMRSPERRD_MSPURSPERRD0_SHIFT             0                                               /**< Shift value for SMU_MSPURSPERRD0            */
#define _SMU_MSPUNVMRSPERRD_MSPURSPERRD0_MASK              0x1UL                                           /**< Bit mask for SMU_MSPURSPERRD0               */
#define _SMU_MSPUNVMRSPERRD_MSPURSPERRD0_DEFAULT           0x00000001UL                                    /**< Mode DEFAULT for SMU_MSPUNVMRSPERRD         */
#define SMU_MSPUNVMRSPERRD_MSPURSPERRD0_DEFAULT            (_SMU_MSPUNVMRSPERRD_MSPURSPERRD0_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_MSPUNVMRSPERRD */

/* Bit fields for SMU MSPUDMEMNSREGIONFLAG0 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_RESETVALUE              0x00000000UL                                           /**< Default value for SMU_MSPUDMEMNSREGIONFLAG0 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MASK                    0xFFFFFFFFUL                                           /**< Mask for SMU_MSPUDMEMNSREGIONFLAG0          */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL0               (0x1UL << 0)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL0_SHIFT        0                                                      /**< Shift value for SMU_MSPUNSRFL0              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL0_MASK         0x1UL                                                  /**< Bit mask for SMU_MSPUNSRFL0                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL0_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL0_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL0_DEFAULT << 0)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL1               (0x1UL << 1)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL1_SHIFT        1                                                      /**< Shift value for SMU_MSPUNSRFL1              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL1_MASK         0x2UL                                                  /**< Bit mask for SMU_MSPUNSRFL1                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL1_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL1_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL1_DEFAULT << 1)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL2               (0x1UL << 2)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL2_SHIFT        2                                                      /**< Shift value for SMU_MSPUNSRFL2              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL2_MASK         0x4UL                                                  /**< Bit mask for SMU_MSPUNSRFL2                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL2_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL2_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL2_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL3               (0x1UL << 3)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL3_SHIFT        3                                                      /**< Shift value for SMU_MSPUNSRFL3              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL3_MASK         0x8UL                                                  /**< Bit mask for SMU_MSPUNSRFL3                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL3_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL3_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL3_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL4               (0x1UL << 4)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL4_SHIFT        4                                                      /**< Shift value for SMU_MSPUNSRFL4              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL4_MASK         0x10UL                                                 /**< Bit mask for SMU_MSPUNSRFL4                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL4_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL4_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL4_DEFAULT << 4)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL5               (0x1UL << 5)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL5_SHIFT        5                                                      /**< Shift value for SMU_MSPUNSRFL5              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL5_MASK         0x20UL                                                 /**< Bit mask for SMU_MSPUNSRFL5                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL5_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL5_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL5_DEFAULT << 5)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL6               (0x1UL << 6)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL6_SHIFT        6                                                      /**< Shift value for SMU_MSPUNSRFL6              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL6_MASK         0x40UL                                                 /**< Bit mask for SMU_MSPUNSRFL6                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL6_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL6_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL6_DEFAULT << 6)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL7               (0x1UL << 7)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL7_SHIFT        7                                                      /**< Shift value for SMU_MSPUNSRFL7              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL7_MASK         0x80UL                                                 /**< Bit mask for SMU_MSPUNSRFL7                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL7_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL7_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL7_DEFAULT << 7)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL8               (0x1UL << 8)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL8_SHIFT        8                                                      /**< Shift value for SMU_MSPUNSRFL8              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL8_MASK         0x100UL                                                /**< Bit mask for SMU_MSPUNSRFL8                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL8_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL8_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL8_DEFAULT << 8)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL9               (0x1UL << 9)                                           /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL9_SHIFT        9                                                      /**< Shift value for SMU_MSPUNSRFL9              */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL9_MASK         0x200UL                                                /**< Bit mask for SMU_MSPUNSRFL9                 */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL9_DEFAULT      0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL9_DEFAULT       (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL9_DEFAULT << 9)   /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL10              (0x1UL << 10)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL10_SHIFT       10                                                     /**< Shift value for SMU_MSPUNSRFL10             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL10_MASK        0x400UL                                                /**< Bit mask for SMU_MSPUNSRFL10                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL10_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL10_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL10_DEFAULT << 10) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL11              (0x1UL << 11)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL11_SHIFT       11                                                     /**< Shift value for SMU_MSPUNSRFL11             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL11_MASK        0x800UL                                                /**< Bit mask for SMU_MSPUNSRFL11                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL11_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL11_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL11_DEFAULT << 11) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL12              (0x1UL << 12)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL12_SHIFT       12                                                     /**< Shift value for SMU_MSPUNSRFL12             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL12_MASK        0x1000UL                                               /**< Bit mask for SMU_MSPUNSRFL12                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL12_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL12_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL12_DEFAULT << 12) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL13              (0x1UL << 13)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL13_SHIFT       13                                                     /**< Shift value for SMU_MSPUNSRFL13             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL13_MASK        0x2000UL                                               /**< Bit mask for SMU_MSPUNSRFL13                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL13_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL13_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL13_DEFAULT << 13) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL14              (0x1UL << 14)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL14_SHIFT       14                                                     /**< Shift value for SMU_MSPUNSRFL14             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL14_MASK        0x4000UL                                               /**< Bit mask for SMU_MSPUNSRFL14                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL14_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL14_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL14_DEFAULT << 14) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL15              (0x1UL << 15)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL15_SHIFT       15                                                     /**< Shift value for SMU_MSPUNSRFL15             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL15_MASK        0x8000UL                                               /**< Bit mask for SMU_MSPUNSRFL15                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL15_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL15_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL15_DEFAULT << 15) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL16              (0x1UL << 16)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL16_SHIFT       16                                                     /**< Shift value for SMU_MSPUNSRFL16             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL16_MASK        0x10000UL                                              /**< Bit mask for SMU_MSPUNSRFL16                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL16_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL16_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL16_DEFAULT << 16) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL17              (0x1UL << 17)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL17_SHIFT       17                                                     /**< Shift value for SMU_MSPUNSRFL17             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL17_MASK        0x20000UL                                              /**< Bit mask for SMU_MSPUNSRFL17                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL17_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL17_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL17_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL18              (0x1UL << 18)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL18_SHIFT       18                                                     /**< Shift value for SMU_MSPUNSRFL18             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL18_MASK        0x40000UL                                              /**< Bit mask for SMU_MSPUNSRFL18                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL18_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL18_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL18_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL19              (0x1UL << 19)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL19_SHIFT       19                                                     /**< Shift value for SMU_MSPUNSRFL19             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL19_MASK        0x80000UL                                              /**< Bit mask for SMU_MSPUNSRFL19                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL19_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL19_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL19_DEFAULT << 19) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL20              (0x1UL << 20)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL20_SHIFT       20                                                     /**< Shift value for SMU_MSPUNSRFL20             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL20_MASK        0x100000UL                                             /**< Bit mask for SMU_MSPUNSRFL20                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL20_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL20_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL20_DEFAULT << 20) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL21              (0x1UL << 21)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL21_SHIFT       21                                                     /**< Shift value for SMU_MSPUNSRFL21             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL21_MASK        0x200000UL                                             /**< Bit mask for SMU_MSPUNSRFL21                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL21_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL21_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL21_DEFAULT << 21) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL22              (0x1UL << 22)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL22_SHIFT       22                                                     /**< Shift value for SMU_MSPUNSRFL22             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL22_MASK        0x400000UL                                             /**< Bit mask for SMU_MSPUNSRFL22                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL22_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL22_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL22_DEFAULT << 22) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL23              (0x1UL << 23)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL23_SHIFT       23                                                     /**< Shift value for SMU_MSPUNSRFL23             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL23_MASK        0x800000UL                                             /**< Bit mask for SMU_MSPUNSRFL23                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL23_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL23_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL23_DEFAULT << 23) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL24              (0x1UL << 24)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL24_SHIFT       24                                                     /**< Shift value for SMU_MSPUNSRFL24             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL24_MASK        0x1000000UL                                            /**< Bit mask for SMU_MSPUNSRFL24                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL24_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL24_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL24_DEFAULT << 24) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL25              (0x1UL << 25)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL25_SHIFT       25                                                     /**< Shift value for SMU_MSPUNSRFL25             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL25_MASK        0x2000000UL                                            /**< Bit mask for SMU_MSPUNSRFL25                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL25_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL25_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL25_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL26              (0x1UL << 26)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL26_SHIFT       26                                                     /**< Shift value for SMU_MSPUNSRFL26             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL26_MASK        0x4000000UL                                            /**< Bit mask for SMU_MSPUNSRFL26                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL26_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL26_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL26_DEFAULT << 26) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL27              (0x1UL << 27)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL27_SHIFT       27                                                     /**< Shift value for SMU_MSPUNSRFL27             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL27_MASK        0x8000000UL                                            /**< Bit mask for SMU_MSPUNSRFL27                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL27_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL27_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL27_DEFAULT << 27) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL28              (0x1UL << 28)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL28_SHIFT       28                                                     /**< Shift value for SMU_MSPUNSRFL28             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL28_MASK        0x10000000UL                                           /**< Bit mask for SMU_MSPUNSRFL28                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL28_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL28_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL28_DEFAULT << 28) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL29              (0x1UL << 29)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL29_SHIFT       29                                                     /**< Shift value for SMU_MSPUNSRFL29             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL29_MASK        0x20000000UL                                           /**< Bit mask for SMU_MSPUNSRFL29                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL29_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL29_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL29_DEFAULT << 29) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL30              (0x1UL << 30)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL30_SHIFT       30                                                     /**< Shift value for SMU_MSPUNSRFL30             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL30_MASK        0x40000000UL                                           /**< Bit mask for SMU_MSPUNSRFL30                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL30_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL30_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL30_DEFAULT << 30) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL31              (0x1UL << 31)                                          /**< DMEM non-secure region flags                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL31_SHIFT       31                                                     /**< Shift value for SMU_MSPUNSRFL31             */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL31_MASK        0x80000000UL                                           /**< Bit mask for SMU_MSPUNSRFL31                */
#define _SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL31_DEFAULT     0x00000000UL                                           /**< Mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0  */
#define SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL31_DEFAULT      (_SMU_MSPUDMEMNSREGIONFLAG0_MSPUNSRFL31_DEFAULT << 31) /**< Shifted mode DEFAULT for SMU_MSPUDMEMNSREGIONFLAG0*/

/* Bit fields for SMU MSPUNVMNSREGIONFLAG0 */
#define _SMU_MSPUNVMNSREGIONFLAG0_RESETVALUE               0x00000000UL                                            /**< Default value for SMU_MSPUNVMNSREGIONFLAG0  */
#define _SMU_MSPUNVMNSREGIONFLAG0_MASK                     0xFFFFFFFFUL                                            /**< Mask for SMU_MSPUNVMNSREGIONFLAG0           */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL0_0              (0x1UL << 0)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL0_0_SHIFT       0                                                       /**< Shift value for SMU_MSPUNSRFL0_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL0_0_MASK        0x1UL                                                   /**< Bit mask for SMU_MSPUNSRFL0_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL0_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL0_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL0_0_DEFAULT << 0)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL1_0              (0x1UL << 1)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL1_0_SHIFT       1                                                       /**< Shift value for SMU_MSPUNSRFL1_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL1_0_MASK        0x2UL                                                   /**< Bit mask for SMU_MSPUNSRFL1_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL1_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL1_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL1_0_DEFAULT << 1)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL2_0              (0x1UL << 2)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL2_0_SHIFT       2                                                       /**< Shift value for SMU_MSPUNSRFL2_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL2_0_MASK        0x4UL                                                   /**< Bit mask for SMU_MSPUNSRFL2_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL2_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL2_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL2_0_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL3_0              (0x1UL << 3)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL3_0_SHIFT       3                                                       /**< Shift value for SMU_MSPUNSRFL3_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL3_0_MASK        0x8UL                                                   /**< Bit mask for SMU_MSPUNSRFL3_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL3_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL3_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL3_0_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL4_0              (0x1UL << 4)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL4_0_SHIFT       4                                                       /**< Shift value for SMU_MSPUNSRFL4_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL4_0_MASK        0x10UL                                                  /**< Bit mask for SMU_MSPUNSRFL4_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL4_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL4_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL4_0_DEFAULT << 4)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL5_0              (0x1UL << 5)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL5_0_SHIFT       5                                                       /**< Shift value for SMU_MSPUNSRFL5_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL5_0_MASK        0x20UL                                                  /**< Bit mask for SMU_MSPUNSRFL5_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL5_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL5_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL5_0_DEFAULT << 5)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL6_0              (0x1UL << 6)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL6_0_SHIFT       6                                                       /**< Shift value for SMU_MSPUNSRFL6_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL6_0_MASK        0x40UL                                                  /**< Bit mask for SMU_MSPUNSRFL6_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL6_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL6_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL6_0_DEFAULT << 6)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL7_0              (0x1UL << 7)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL7_0_SHIFT       7                                                       /**< Shift value for SMU_MSPUNSRFL7_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL7_0_MASK        0x80UL                                                  /**< Bit mask for SMU_MSPUNSRFL7_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL7_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL7_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL7_0_DEFAULT << 7)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL8_0              (0x1UL << 8)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL8_0_SHIFT       8                                                       /**< Shift value for SMU_MSPUNSRFL8_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL8_0_MASK        0x100UL                                                 /**< Bit mask for SMU_MSPUNSRFL8_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL8_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL8_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL8_0_DEFAULT << 8)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL9_0              (0x1UL << 9)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL9_0_SHIFT       9                                                       /**< Shift value for SMU_MSPUNSRFL9_0            */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL9_0_MASK        0x200UL                                                 /**< Bit mask for SMU_MSPUNSRFL9_0               */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL9_0_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL9_0_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL9_0_DEFAULT << 9)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL10_0             (0x1UL << 10)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL10_0_SHIFT      10                                                      /**< Shift value for SMU_MSPUNSRFL10_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL10_0_MASK       0x400UL                                                 /**< Bit mask for SMU_MSPUNSRFL10_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL10_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL10_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL10_0_DEFAULT << 10) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL11_0             (0x1UL << 11)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL11_0_SHIFT      11                                                      /**< Shift value for SMU_MSPUNSRFL11_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL11_0_MASK       0x800UL                                                 /**< Bit mask for SMU_MSPUNSRFL11_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL11_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL11_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL11_0_DEFAULT << 11) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL12_0             (0x1UL << 12)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL12_0_SHIFT      12                                                      /**< Shift value for SMU_MSPUNSRFL12_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL12_0_MASK       0x1000UL                                                /**< Bit mask for SMU_MSPUNSRFL12_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL12_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL12_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL12_0_DEFAULT << 12) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL13_0             (0x1UL << 13)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL13_0_SHIFT      13                                                      /**< Shift value for SMU_MSPUNSRFL13_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL13_0_MASK       0x2000UL                                                /**< Bit mask for SMU_MSPUNSRFL13_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL13_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL13_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL13_0_DEFAULT << 13) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL14_0             (0x1UL << 14)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL14_0_SHIFT      14                                                      /**< Shift value for SMU_MSPUNSRFL14_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL14_0_MASK       0x4000UL                                                /**< Bit mask for SMU_MSPUNSRFL14_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL14_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL14_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL14_0_DEFAULT << 14) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL15_0             (0x1UL << 15)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL15_0_SHIFT      15                                                      /**< Shift value for SMU_MSPUNSRFL15_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL15_0_MASK       0x8000UL                                                /**< Bit mask for SMU_MSPUNSRFL15_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL15_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL15_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL15_0_DEFAULT << 15) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL16_0             (0x1UL << 16)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL16_0_SHIFT      16                                                      /**< Shift value for SMU_MSPUNSRFL16_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL16_0_MASK       0x10000UL                                               /**< Bit mask for SMU_MSPUNSRFL16_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL16_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL16_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL16_0_DEFAULT << 16) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL17_0             (0x1UL << 17)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL17_0_SHIFT      17                                                      /**< Shift value for SMU_MSPUNSRFL17_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL17_0_MASK       0x20000UL                                               /**< Bit mask for SMU_MSPUNSRFL17_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL17_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL17_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL17_0_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL18_0             (0x1UL << 18)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL18_0_SHIFT      18                                                      /**< Shift value for SMU_MSPUNSRFL18_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL18_0_MASK       0x40000UL                                               /**< Bit mask for SMU_MSPUNSRFL18_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL18_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL18_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL18_0_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL19_0             (0x1UL << 19)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL19_0_SHIFT      19                                                      /**< Shift value for SMU_MSPUNSRFL19_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL19_0_MASK       0x80000UL                                               /**< Bit mask for SMU_MSPUNSRFL19_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL19_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL19_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL19_0_DEFAULT << 19) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL20_0             (0x1UL << 20)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL20_0_SHIFT      20                                                      /**< Shift value for SMU_MSPUNSRFL20_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL20_0_MASK       0x100000UL                                              /**< Bit mask for SMU_MSPUNSRFL20_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL20_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL20_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL20_0_DEFAULT << 20) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL21_0             (0x1UL << 21)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL21_0_SHIFT      21                                                      /**< Shift value for SMU_MSPUNSRFL21_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL21_0_MASK       0x200000UL                                              /**< Bit mask for SMU_MSPUNSRFL21_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL21_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL21_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL21_0_DEFAULT << 21) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL22_0             (0x1UL << 22)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL22_0_SHIFT      22                                                      /**< Shift value for SMU_MSPUNSRFL22_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL22_0_MASK       0x400000UL                                              /**< Bit mask for SMU_MSPUNSRFL22_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL22_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL22_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL22_0_DEFAULT << 22) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL23_0             (0x1UL << 23)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL23_0_SHIFT      23                                                      /**< Shift value for SMU_MSPUNSRFL23_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL23_0_MASK       0x800000UL                                              /**< Bit mask for SMU_MSPUNSRFL23_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL23_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL23_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL23_0_DEFAULT << 23) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL24_0             (0x1UL << 24)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL24_0_SHIFT      24                                                      /**< Shift value for SMU_MSPUNSRFL24_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL24_0_MASK       0x1000000UL                                             /**< Bit mask for SMU_MSPUNSRFL24_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL24_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL24_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL24_0_DEFAULT << 24) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL25_0             (0x1UL << 25)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL25_0_SHIFT      25                                                      /**< Shift value for SMU_MSPUNSRFL25_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL25_0_MASK       0x2000000UL                                             /**< Bit mask for SMU_MSPUNSRFL25_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL25_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL25_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL25_0_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL26_0             (0x1UL << 26)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL26_0_SHIFT      26                                                      /**< Shift value for SMU_MSPUNSRFL26_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL26_0_MASK       0x4000000UL                                             /**< Bit mask for SMU_MSPUNSRFL26_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL26_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL26_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL26_0_DEFAULT << 26) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL27_0             (0x1UL << 27)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL27_0_SHIFT      27                                                      /**< Shift value for SMU_MSPUNSRFL27_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL27_0_MASK       0x8000000UL                                             /**< Bit mask for SMU_MSPUNSRFL27_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL27_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL27_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL27_0_DEFAULT << 27) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL28_0             (0x1UL << 28)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL28_0_SHIFT      28                                                      /**< Shift value for SMU_MSPUNSRFL28_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL28_0_MASK       0x10000000UL                                            /**< Bit mask for SMU_MSPUNSRFL28_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL28_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL28_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL28_0_DEFAULT << 28) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL29_0             (0x1UL << 29)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL29_0_SHIFT      29                                                      /**< Shift value for SMU_MSPUNSRFL29_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL29_0_MASK       0x20000000UL                                            /**< Bit mask for SMU_MSPUNSRFL29_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL29_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL29_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL29_0_DEFAULT << 29) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL30_0             (0x1UL << 30)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL30_0_SHIFT      30                                                      /**< Shift value for SMU_MSPUNSRFL30_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL30_0_MASK       0x40000000UL                                            /**< Bit mask for SMU_MSPUNSRFL30_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL30_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL30_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL30_0_DEFAULT << 30) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL31_0             (0x1UL << 31)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL31_0_SHIFT      31                                                      /**< Shift value for SMU_MSPUNSRFL31_0           */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL31_0_MASK       0x80000000UL                                            /**< Bit mask for SMU_MSPUNSRFL31_0              */
#define _SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL31_0_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0   */
#define SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL31_0_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG0_MSPUNSRFL31_0_DEFAULT << 31) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG0*/

/* Bit fields for SMU MSPUNVMNSREGIONFLAG1 */
#define _SMU_MSPUNVMNSREGIONFLAG1_RESETVALUE               0x00000000UL                                            /**< Default value for SMU_MSPUNVMNSREGIONFLAG1  */
#define _SMU_MSPUNVMNSREGIONFLAG1_MASK                     0xFFFFFFFFUL                                            /**< Mask for SMU_MSPUNVMNSREGIONFLAG1           */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL0_1              (0x1UL << 0)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL0_1_SHIFT       0                                                       /**< Shift value for SMU_MSPUNSRFL0_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL0_1_MASK        0x1UL                                                   /**< Bit mask for SMU_MSPUNSRFL0_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL0_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL0_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL0_1_DEFAULT << 0)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL1_1              (0x1UL << 1)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL1_1_SHIFT       1                                                       /**< Shift value for SMU_MSPUNSRFL1_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL1_1_MASK        0x2UL                                                   /**< Bit mask for SMU_MSPUNSRFL1_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL1_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL1_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL1_1_DEFAULT << 1)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL2_1              (0x1UL << 2)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL2_1_SHIFT       2                                                       /**< Shift value for SMU_MSPUNSRFL2_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL2_1_MASK        0x4UL                                                   /**< Bit mask for SMU_MSPUNSRFL2_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL2_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL2_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL2_1_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL3_1              (0x1UL << 3)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL3_1_SHIFT       3                                                       /**< Shift value for SMU_MSPUNSRFL3_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL3_1_MASK        0x8UL                                                   /**< Bit mask for SMU_MSPUNSRFL3_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL3_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL3_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL3_1_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL4_1              (0x1UL << 4)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL4_1_SHIFT       4                                                       /**< Shift value for SMU_MSPUNSRFL4_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL4_1_MASK        0x10UL                                                  /**< Bit mask for SMU_MSPUNSRFL4_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL4_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL4_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL4_1_DEFAULT << 4)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL5_1              (0x1UL << 5)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL5_1_SHIFT       5                                                       /**< Shift value for SMU_MSPUNSRFL5_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL5_1_MASK        0x20UL                                                  /**< Bit mask for SMU_MSPUNSRFL5_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL5_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL5_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL5_1_DEFAULT << 5)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL6_1              (0x1UL << 6)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL6_1_SHIFT       6                                                       /**< Shift value for SMU_MSPUNSRFL6_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL6_1_MASK        0x40UL                                                  /**< Bit mask for SMU_MSPUNSRFL6_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL6_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL6_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL6_1_DEFAULT << 6)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL7_1              (0x1UL << 7)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL7_1_SHIFT       7                                                       /**< Shift value for SMU_MSPUNSRFL7_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL7_1_MASK        0x80UL                                                  /**< Bit mask for SMU_MSPUNSRFL7_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL7_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL7_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL7_1_DEFAULT << 7)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL8_1              (0x1UL << 8)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL8_1_SHIFT       8                                                       /**< Shift value for SMU_MSPUNSRFL8_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL8_1_MASK        0x100UL                                                 /**< Bit mask for SMU_MSPUNSRFL8_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL8_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL8_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL8_1_DEFAULT << 8)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL9_1              (0x1UL << 9)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL9_1_SHIFT       9                                                       /**< Shift value for SMU_MSPUNSRFL9_1            */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL9_1_MASK        0x200UL                                                 /**< Bit mask for SMU_MSPUNSRFL9_1               */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL9_1_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL9_1_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL9_1_DEFAULT << 9)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL10_1             (0x1UL << 10)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL10_1_SHIFT      10                                                      /**< Shift value for SMU_MSPUNSRFL10_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL10_1_MASK       0x400UL                                                 /**< Bit mask for SMU_MSPUNSRFL10_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL10_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL10_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL10_1_DEFAULT << 10) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL11_1             (0x1UL << 11)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL11_1_SHIFT      11                                                      /**< Shift value for SMU_MSPUNSRFL11_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL11_1_MASK       0x800UL                                                 /**< Bit mask for SMU_MSPUNSRFL11_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL11_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL11_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL11_1_DEFAULT << 11) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL12_1             (0x1UL << 12)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL12_1_SHIFT      12                                                      /**< Shift value for SMU_MSPUNSRFL12_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL12_1_MASK       0x1000UL                                                /**< Bit mask for SMU_MSPUNSRFL12_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL12_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL12_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL12_1_DEFAULT << 12) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL13_1             (0x1UL << 13)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL13_1_SHIFT      13                                                      /**< Shift value for SMU_MSPUNSRFL13_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL13_1_MASK       0x2000UL                                                /**< Bit mask for SMU_MSPUNSRFL13_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL13_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL13_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL13_1_DEFAULT << 13) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL14_1             (0x1UL << 14)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL14_1_SHIFT      14                                                      /**< Shift value for SMU_MSPUNSRFL14_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL14_1_MASK       0x4000UL                                                /**< Bit mask for SMU_MSPUNSRFL14_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL14_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL14_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL14_1_DEFAULT << 14) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL15_1             (0x1UL << 15)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL15_1_SHIFT      15                                                      /**< Shift value for SMU_MSPUNSRFL15_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL15_1_MASK       0x8000UL                                                /**< Bit mask for SMU_MSPUNSRFL15_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL15_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL15_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL15_1_DEFAULT << 15) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL16_1             (0x1UL << 16)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL16_1_SHIFT      16                                                      /**< Shift value for SMU_MSPUNSRFL16_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL16_1_MASK       0x10000UL                                               /**< Bit mask for SMU_MSPUNSRFL16_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL16_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL16_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL16_1_DEFAULT << 16) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL17_1             (0x1UL << 17)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL17_1_SHIFT      17                                                      /**< Shift value for SMU_MSPUNSRFL17_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL17_1_MASK       0x20000UL                                               /**< Bit mask for SMU_MSPUNSRFL17_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL17_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL17_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL17_1_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL18_1             (0x1UL << 18)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL18_1_SHIFT      18                                                      /**< Shift value for SMU_MSPUNSRFL18_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL18_1_MASK       0x40000UL                                               /**< Bit mask for SMU_MSPUNSRFL18_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL18_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL18_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL18_1_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL19_1             (0x1UL << 19)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL19_1_SHIFT      19                                                      /**< Shift value for SMU_MSPUNSRFL19_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL19_1_MASK       0x80000UL                                               /**< Bit mask for SMU_MSPUNSRFL19_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL19_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL19_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL19_1_DEFAULT << 19) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL20_1             (0x1UL << 20)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL20_1_SHIFT      20                                                      /**< Shift value for SMU_MSPUNSRFL20_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL20_1_MASK       0x100000UL                                              /**< Bit mask for SMU_MSPUNSRFL20_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL20_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL20_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL20_1_DEFAULT << 20) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL21_1             (0x1UL << 21)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL21_1_SHIFT      21                                                      /**< Shift value for SMU_MSPUNSRFL21_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL21_1_MASK       0x200000UL                                              /**< Bit mask for SMU_MSPUNSRFL21_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL21_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL21_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL21_1_DEFAULT << 21) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL22_1             (0x1UL << 22)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL22_1_SHIFT      22                                                      /**< Shift value for SMU_MSPUNSRFL22_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL22_1_MASK       0x400000UL                                              /**< Bit mask for SMU_MSPUNSRFL22_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL22_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL22_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL22_1_DEFAULT << 22) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL23_1             (0x1UL << 23)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL23_1_SHIFT      23                                                      /**< Shift value for SMU_MSPUNSRFL23_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL23_1_MASK       0x800000UL                                              /**< Bit mask for SMU_MSPUNSRFL23_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL23_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL23_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL23_1_DEFAULT << 23) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL24_1             (0x1UL << 24)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL24_1_SHIFT      24                                                      /**< Shift value for SMU_MSPUNSRFL24_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL24_1_MASK       0x1000000UL                                             /**< Bit mask for SMU_MSPUNSRFL24_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL24_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL24_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL24_1_DEFAULT << 24) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL25_1             (0x1UL << 25)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL25_1_SHIFT      25                                                      /**< Shift value for SMU_MSPUNSRFL25_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL25_1_MASK       0x2000000UL                                             /**< Bit mask for SMU_MSPUNSRFL25_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL25_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL25_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL25_1_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL26_1             (0x1UL << 26)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL26_1_SHIFT      26                                                      /**< Shift value for SMU_MSPUNSRFL26_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL26_1_MASK       0x4000000UL                                             /**< Bit mask for SMU_MSPUNSRFL26_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL26_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL26_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL26_1_DEFAULT << 26) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL27_1             (0x1UL << 27)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL27_1_SHIFT      27                                                      /**< Shift value for SMU_MSPUNSRFL27_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL27_1_MASK       0x8000000UL                                             /**< Bit mask for SMU_MSPUNSRFL27_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL27_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL27_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL27_1_DEFAULT << 27) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL28_1             (0x1UL << 28)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL28_1_SHIFT      28                                                      /**< Shift value for SMU_MSPUNSRFL28_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL28_1_MASK       0x10000000UL                                            /**< Bit mask for SMU_MSPUNSRFL28_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL28_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL28_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL28_1_DEFAULT << 28) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL29_1             (0x1UL << 29)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL29_1_SHIFT      29                                                      /**< Shift value for SMU_MSPUNSRFL29_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL29_1_MASK       0x20000000UL                                            /**< Bit mask for SMU_MSPUNSRFL29_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL29_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL29_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL29_1_DEFAULT << 29) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL30_1             (0x1UL << 30)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL30_1_SHIFT      30                                                      /**< Shift value for SMU_MSPUNSRFL30_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL30_1_MASK       0x40000000UL                                            /**< Bit mask for SMU_MSPUNSRFL30_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL30_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL30_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL30_1_DEFAULT << 30) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL31_1             (0x1UL << 31)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL31_1_SHIFT      31                                                      /**< Shift value for SMU_MSPUNSRFL31_1           */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL31_1_MASK       0x80000000UL                                            /**< Bit mask for SMU_MSPUNSRFL31_1              */
#define _SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL31_1_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1   */
#define SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL31_1_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG1_MSPUNSRFL31_1_DEFAULT << 31) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG1*/

/* Bit fields for SMU MSPUNVMNSREGIONFLAG2 */
#define _SMU_MSPUNVMNSREGIONFLAG2_RESETVALUE               0x00000000UL                                            /**< Default value for SMU_MSPUNVMNSREGIONFLAG2  */
#define _SMU_MSPUNVMNSREGIONFLAG2_MASK                     0xFFFFFFFFUL                                            /**< Mask for SMU_MSPUNVMNSREGIONFLAG2           */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL0_2              (0x1UL << 0)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL0_2_SHIFT       0                                                       /**< Shift value for SMU_MSPUNSRFL0_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL0_2_MASK        0x1UL                                                   /**< Bit mask for SMU_MSPUNSRFL0_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL0_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL0_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL0_2_DEFAULT << 0)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL1_2              (0x1UL << 1)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL1_2_SHIFT       1                                                       /**< Shift value for SMU_MSPUNSRFL1_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL1_2_MASK        0x2UL                                                   /**< Bit mask for SMU_MSPUNSRFL1_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL1_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL1_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL1_2_DEFAULT << 1)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL2_2              (0x1UL << 2)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL2_2_SHIFT       2                                                       /**< Shift value for SMU_MSPUNSRFL2_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL2_2_MASK        0x4UL                                                   /**< Bit mask for SMU_MSPUNSRFL2_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL2_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL2_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL2_2_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL3_2              (0x1UL << 3)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL3_2_SHIFT       3                                                       /**< Shift value for SMU_MSPUNSRFL3_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL3_2_MASK        0x8UL                                                   /**< Bit mask for SMU_MSPUNSRFL3_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL3_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL3_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL3_2_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL4_2              (0x1UL << 4)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL4_2_SHIFT       4                                                       /**< Shift value for SMU_MSPUNSRFL4_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL4_2_MASK        0x10UL                                                  /**< Bit mask for SMU_MSPUNSRFL4_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL4_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL4_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL4_2_DEFAULT << 4)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL5_2              (0x1UL << 5)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL5_2_SHIFT       5                                                       /**< Shift value for SMU_MSPUNSRFL5_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL5_2_MASK        0x20UL                                                  /**< Bit mask for SMU_MSPUNSRFL5_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL5_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL5_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL5_2_DEFAULT << 5)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL6_2              (0x1UL << 6)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL6_2_SHIFT       6                                                       /**< Shift value for SMU_MSPUNSRFL6_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL6_2_MASK        0x40UL                                                  /**< Bit mask for SMU_MSPUNSRFL6_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL6_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL6_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL6_2_DEFAULT << 6)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL7_2              (0x1UL << 7)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL7_2_SHIFT       7                                                       /**< Shift value for SMU_MSPUNSRFL7_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL7_2_MASK        0x80UL                                                  /**< Bit mask for SMU_MSPUNSRFL7_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL7_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL7_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL7_2_DEFAULT << 7)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL8_2              (0x1UL << 8)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL8_2_SHIFT       8                                                       /**< Shift value for SMU_MSPUNSRFL8_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL8_2_MASK        0x100UL                                                 /**< Bit mask for SMU_MSPUNSRFL8_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL8_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL8_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL8_2_DEFAULT << 8)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL9_2              (0x1UL << 9)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL9_2_SHIFT       9                                                       /**< Shift value for SMU_MSPUNSRFL9_2            */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL9_2_MASK        0x200UL                                                 /**< Bit mask for SMU_MSPUNSRFL9_2               */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL9_2_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL9_2_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL9_2_DEFAULT << 9)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL10_2             (0x1UL << 10)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL10_2_SHIFT      10                                                      /**< Shift value for SMU_MSPUNSRFL10_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL10_2_MASK       0x400UL                                                 /**< Bit mask for SMU_MSPUNSRFL10_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL10_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL10_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL10_2_DEFAULT << 10) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL11_2             (0x1UL << 11)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL11_2_SHIFT      11                                                      /**< Shift value for SMU_MSPUNSRFL11_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL11_2_MASK       0x800UL                                                 /**< Bit mask for SMU_MSPUNSRFL11_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL11_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL11_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL11_2_DEFAULT << 11) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL12_2             (0x1UL << 12)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL12_2_SHIFT      12                                                      /**< Shift value for SMU_MSPUNSRFL12_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL12_2_MASK       0x1000UL                                                /**< Bit mask for SMU_MSPUNSRFL12_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL12_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL12_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL12_2_DEFAULT << 12) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL13_2             (0x1UL << 13)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL13_2_SHIFT      13                                                      /**< Shift value for SMU_MSPUNSRFL13_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL13_2_MASK       0x2000UL                                                /**< Bit mask for SMU_MSPUNSRFL13_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL13_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL13_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL13_2_DEFAULT << 13) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL14_2             (0x1UL << 14)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL14_2_SHIFT      14                                                      /**< Shift value for SMU_MSPUNSRFL14_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL14_2_MASK       0x4000UL                                                /**< Bit mask for SMU_MSPUNSRFL14_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL14_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL14_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL14_2_DEFAULT << 14) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL15_2             (0x1UL << 15)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL15_2_SHIFT      15                                                      /**< Shift value for SMU_MSPUNSRFL15_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL15_2_MASK       0x8000UL                                                /**< Bit mask for SMU_MSPUNSRFL15_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL15_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL15_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL15_2_DEFAULT << 15) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL16_2             (0x1UL << 16)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL16_2_SHIFT      16                                                      /**< Shift value for SMU_MSPUNSRFL16_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL16_2_MASK       0x10000UL                                               /**< Bit mask for SMU_MSPUNSRFL16_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL16_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL16_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL16_2_DEFAULT << 16) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL17_2             (0x1UL << 17)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL17_2_SHIFT      17                                                      /**< Shift value for SMU_MSPUNSRFL17_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL17_2_MASK       0x20000UL                                               /**< Bit mask for SMU_MSPUNSRFL17_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL17_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL17_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL17_2_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL18_2             (0x1UL << 18)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL18_2_SHIFT      18                                                      /**< Shift value for SMU_MSPUNSRFL18_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL18_2_MASK       0x40000UL                                               /**< Bit mask for SMU_MSPUNSRFL18_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL18_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL18_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL18_2_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL19_2             (0x1UL << 19)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL19_2_SHIFT      19                                                      /**< Shift value for SMU_MSPUNSRFL19_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL19_2_MASK       0x80000UL                                               /**< Bit mask for SMU_MSPUNSRFL19_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL19_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL19_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL19_2_DEFAULT << 19) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL20_2             (0x1UL << 20)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL20_2_SHIFT      20                                                      /**< Shift value for SMU_MSPUNSRFL20_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL20_2_MASK       0x100000UL                                              /**< Bit mask for SMU_MSPUNSRFL20_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL20_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL20_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL20_2_DEFAULT << 20) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL21_2             (0x1UL << 21)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL21_2_SHIFT      21                                                      /**< Shift value for SMU_MSPUNSRFL21_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL21_2_MASK       0x200000UL                                              /**< Bit mask for SMU_MSPUNSRFL21_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL21_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL21_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL21_2_DEFAULT << 21) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL22_2             (0x1UL << 22)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL22_2_SHIFT      22                                                      /**< Shift value for SMU_MSPUNSRFL22_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL22_2_MASK       0x400000UL                                              /**< Bit mask for SMU_MSPUNSRFL22_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL22_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL22_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL22_2_DEFAULT << 22) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL23_2             (0x1UL << 23)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL23_2_SHIFT      23                                                      /**< Shift value for SMU_MSPUNSRFL23_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL23_2_MASK       0x800000UL                                              /**< Bit mask for SMU_MSPUNSRFL23_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL23_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL23_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL23_2_DEFAULT << 23) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL24_2             (0x1UL << 24)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL24_2_SHIFT      24                                                      /**< Shift value for SMU_MSPUNSRFL24_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL24_2_MASK       0x1000000UL                                             /**< Bit mask for SMU_MSPUNSRFL24_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL24_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL24_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL24_2_DEFAULT << 24) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL25_2             (0x1UL << 25)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL25_2_SHIFT      25                                                      /**< Shift value for SMU_MSPUNSRFL25_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL25_2_MASK       0x2000000UL                                             /**< Bit mask for SMU_MSPUNSRFL25_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL25_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL25_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL25_2_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL26_2             (0x1UL << 26)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL26_2_SHIFT      26                                                      /**< Shift value for SMU_MSPUNSRFL26_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL26_2_MASK       0x4000000UL                                             /**< Bit mask for SMU_MSPUNSRFL26_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL26_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL26_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL26_2_DEFAULT << 26) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL27_2             (0x1UL << 27)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL27_2_SHIFT      27                                                      /**< Shift value for SMU_MSPUNSRFL27_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL27_2_MASK       0x8000000UL                                             /**< Bit mask for SMU_MSPUNSRFL27_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL27_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL27_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL27_2_DEFAULT << 27) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL28_2             (0x1UL << 28)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL28_2_SHIFT      28                                                      /**< Shift value for SMU_MSPUNSRFL28_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL28_2_MASK       0x10000000UL                                            /**< Bit mask for SMU_MSPUNSRFL28_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL28_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL28_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL28_2_DEFAULT << 28) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL29_2             (0x1UL << 29)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL29_2_SHIFT      29                                                      /**< Shift value for SMU_MSPUNSRFL29_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL29_2_MASK       0x20000000UL                                            /**< Bit mask for SMU_MSPUNSRFL29_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL29_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL29_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL29_2_DEFAULT << 29) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL30_2             (0x1UL << 30)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL30_2_SHIFT      30                                                      /**< Shift value for SMU_MSPUNSRFL30_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL30_2_MASK       0x40000000UL                                            /**< Bit mask for SMU_MSPUNSRFL30_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL30_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL30_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL30_2_DEFAULT << 30) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL31_2             (0x1UL << 31)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL31_2_SHIFT      31                                                      /**< Shift value for SMU_MSPUNSRFL31_2           */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL31_2_MASK       0x80000000UL                                            /**< Bit mask for SMU_MSPUNSRFL31_2              */
#define _SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL31_2_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2   */
#define SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL31_2_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG2_MSPUNSRFL31_2_DEFAULT << 31) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG2*/

/* Bit fields for SMU MSPUNVMNSREGIONFLAG3 */
#define _SMU_MSPUNVMNSREGIONFLAG3_RESETVALUE               0x00000000UL                                            /**< Default value for SMU_MSPUNVMNSREGIONFLAG3  */
#define _SMU_MSPUNVMNSREGIONFLAG3_MASK                     0xFFFFFFFFUL                                            /**< Mask for SMU_MSPUNVMNSREGIONFLAG3           */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL0_3              (0x1UL << 0)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL0_3_SHIFT       0                                                       /**< Shift value for SMU_MSPUNSRFL0_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL0_3_MASK        0x1UL                                                   /**< Bit mask for SMU_MSPUNSRFL0_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL0_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL0_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL0_3_DEFAULT << 0)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL1_3              (0x1UL << 1)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL1_3_SHIFT       1                                                       /**< Shift value for SMU_MSPUNSRFL1_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL1_3_MASK        0x2UL                                                   /**< Bit mask for SMU_MSPUNSRFL1_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL1_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL1_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL1_3_DEFAULT << 1)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL2_3              (0x1UL << 2)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL2_3_SHIFT       2                                                       /**< Shift value for SMU_MSPUNSRFL2_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL2_3_MASK        0x4UL                                                   /**< Bit mask for SMU_MSPUNSRFL2_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL2_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL2_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL2_3_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL3_3              (0x1UL << 3)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL3_3_SHIFT       3                                                       /**< Shift value for SMU_MSPUNSRFL3_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL3_3_MASK        0x8UL                                                   /**< Bit mask for SMU_MSPUNSRFL3_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL3_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL3_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL3_3_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL4_3              (0x1UL << 4)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL4_3_SHIFT       4                                                       /**< Shift value for SMU_MSPUNSRFL4_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL4_3_MASK        0x10UL                                                  /**< Bit mask for SMU_MSPUNSRFL4_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL4_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL4_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL4_3_DEFAULT << 4)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL5_3              (0x1UL << 5)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL5_3_SHIFT       5                                                       /**< Shift value for SMU_MSPUNSRFL5_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL5_3_MASK        0x20UL                                                  /**< Bit mask for SMU_MSPUNSRFL5_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL5_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL5_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL5_3_DEFAULT << 5)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL6_3              (0x1UL << 6)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL6_3_SHIFT       6                                                       /**< Shift value for SMU_MSPUNSRFL6_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL6_3_MASK        0x40UL                                                  /**< Bit mask for SMU_MSPUNSRFL6_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL6_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL6_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL6_3_DEFAULT << 6)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL7_3              (0x1UL << 7)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL7_3_SHIFT       7                                                       /**< Shift value for SMU_MSPUNSRFL7_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL7_3_MASK        0x80UL                                                  /**< Bit mask for SMU_MSPUNSRFL7_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL7_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL7_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL7_3_DEFAULT << 7)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL8_3              (0x1UL << 8)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL8_3_SHIFT       8                                                       /**< Shift value for SMU_MSPUNSRFL8_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL8_3_MASK        0x100UL                                                 /**< Bit mask for SMU_MSPUNSRFL8_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL8_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL8_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL8_3_DEFAULT << 8)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL9_3              (0x1UL << 9)                                            /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL9_3_SHIFT       9                                                       /**< Shift value for SMU_MSPUNSRFL9_3            */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL9_3_MASK        0x200UL                                                 /**< Bit mask for SMU_MSPUNSRFL9_3               */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL9_3_DEFAULT     0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL9_3_DEFAULT      (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL9_3_DEFAULT << 9)   /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL10_3             (0x1UL << 10)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL10_3_SHIFT      10                                                      /**< Shift value for SMU_MSPUNSRFL10_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL10_3_MASK       0x400UL                                                 /**< Bit mask for SMU_MSPUNSRFL10_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL10_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL10_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL10_3_DEFAULT << 10) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL11_3             (0x1UL << 11)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL11_3_SHIFT      11                                                      /**< Shift value for SMU_MSPUNSRFL11_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL11_3_MASK       0x800UL                                                 /**< Bit mask for SMU_MSPUNSRFL11_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL11_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL11_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL11_3_DEFAULT << 11) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL12_3             (0x1UL << 12)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL12_3_SHIFT      12                                                      /**< Shift value for SMU_MSPUNSRFL12_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL12_3_MASK       0x1000UL                                                /**< Bit mask for SMU_MSPUNSRFL12_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL12_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL12_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL12_3_DEFAULT << 12) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL13_3             (0x1UL << 13)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL13_3_SHIFT      13                                                      /**< Shift value for SMU_MSPUNSRFL13_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL13_3_MASK       0x2000UL                                                /**< Bit mask for SMU_MSPUNSRFL13_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL13_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL13_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL13_3_DEFAULT << 13) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL14_3             (0x1UL << 14)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL14_3_SHIFT      14                                                      /**< Shift value for SMU_MSPUNSRFL14_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL14_3_MASK       0x4000UL                                                /**< Bit mask for SMU_MSPUNSRFL14_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL14_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL14_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL14_3_DEFAULT << 14) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL15_3             (0x1UL << 15)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL15_3_SHIFT      15                                                      /**< Shift value for SMU_MSPUNSRFL15_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL15_3_MASK       0x8000UL                                                /**< Bit mask for SMU_MSPUNSRFL15_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL15_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL15_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL15_3_DEFAULT << 15) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL16_3             (0x1UL << 16)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL16_3_SHIFT      16                                                      /**< Shift value for SMU_MSPUNSRFL16_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL16_3_MASK       0x10000UL                                               /**< Bit mask for SMU_MSPUNSRFL16_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL16_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL16_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL16_3_DEFAULT << 16) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL17_3             (0x1UL << 17)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL17_3_SHIFT      17                                                      /**< Shift value for SMU_MSPUNSRFL17_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL17_3_MASK       0x20000UL                                               /**< Bit mask for SMU_MSPUNSRFL17_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL17_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL17_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL17_3_DEFAULT << 17) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL18_3             (0x1UL << 18)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL18_3_SHIFT      18                                                      /**< Shift value for SMU_MSPUNSRFL18_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL18_3_MASK       0x40000UL                                               /**< Bit mask for SMU_MSPUNSRFL18_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL18_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL18_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL18_3_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL19_3             (0x1UL << 19)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL19_3_SHIFT      19                                                      /**< Shift value for SMU_MSPUNSRFL19_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL19_3_MASK       0x80000UL                                               /**< Bit mask for SMU_MSPUNSRFL19_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL19_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL19_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL19_3_DEFAULT << 19) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL20_3             (0x1UL << 20)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL20_3_SHIFT      20                                                      /**< Shift value for SMU_MSPUNSRFL20_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL20_3_MASK       0x100000UL                                              /**< Bit mask for SMU_MSPUNSRFL20_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL20_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL20_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL20_3_DEFAULT << 20) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL21_3             (0x1UL << 21)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL21_3_SHIFT      21                                                      /**< Shift value for SMU_MSPUNSRFL21_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL21_3_MASK       0x200000UL                                              /**< Bit mask for SMU_MSPUNSRFL21_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL21_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL21_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL21_3_DEFAULT << 21) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL22_3             (0x1UL << 22)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL22_3_SHIFT      22                                                      /**< Shift value for SMU_MSPUNSRFL22_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL22_3_MASK       0x400000UL                                              /**< Bit mask for SMU_MSPUNSRFL22_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL22_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL22_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL22_3_DEFAULT << 22) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL23_3             (0x1UL << 23)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL23_3_SHIFT      23                                                      /**< Shift value for SMU_MSPUNSRFL23_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL23_3_MASK       0x800000UL                                              /**< Bit mask for SMU_MSPUNSRFL23_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL23_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL23_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL23_3_DEFAULT << 23) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL24_3             (0x1UL << 24)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL24_3_SHIFT      24                                                      /**< Shift value for SMU_MSPUNSRFL24_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL24_3_MASK       0x1000000UL                                             /**< Bit mask for SMU_MSPUNSRFL24_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL24_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL24_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL24_3_DEFAULT << 24) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL25_3             (0x1UL << 25)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL25_3_SHIFT      25                                                      /**< Shift value for SMU_MSPUNSRFL25_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL25_3_MASK       0x2000000UL                                             /**< Bit mask for SMU_MSPUNSRFL25_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL25_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL25_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL25_3_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL26_3             (0x1UL << 26)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL26_3_SHIFT      26                                                      /**< Shift value for SMU_MSPUNSRFL26_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL26_3_MASK       0x4000000UL                                             /**< Bit mask for SMU_MSPUNSRFL26_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL26_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL26_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL26_3_DEFAULT << 26) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL27_3             (0x1UL << 27)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL27_3_SHIFT      27                                                      /**< Shift value for SMU_MSPUNSRFL27_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL27_3_MASK       0x8000000UL                                             /**< Bit mask for SMU_MSPUNSRFL27_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL27_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL27_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL27_3_DEFAULT << 27) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL28_3             (0x1UL << 28)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL28_3_SHIFT      28                                                      /**< Shift value for SMU_MSPUNSRFL28_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL28_3_MASK       0x10000000UL                                            /**< Bit mask for SMU_MSPUNSRFL28_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL28_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL28_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL28_3_DEFAULT << 28) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL29_3             (0x1UL << 29)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL29_3_SHIFT      29                                                      /**< Shift value for SMU_MSPUNSRFL29_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL29_3_MASK       0x20000000UL                                            /**< Bit mask for SMU_MSPUNSRFL29_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL29_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL29_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL29_3_DEFAULT << 29) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL30_3             (0x1UL << 30)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL30_3_SHIFT      30                                                      /**< Shift value for SMU_MSPUNSRFL30_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL30_3_MASK       0x40000000UL                                            /**< Bit mask for SMU_MSPUNSRFL30_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL30_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL30_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL30_3_DEFAULT << 30) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL31_3             (0x1UL << 31)                                           /**< NVM non-secure region flags                 */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL31_3_SHIFT      31                                                      /**< Shift value for SMU_MSPUNSRFL31_3           */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL31_3_MASK       0x80000000UL                                            /**< Bit mask for SMU_MSPUNSRFL31_3              */
#define _SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL31_3_DEFAULT    0x00000000UL                                            /**< Mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3   */
#define SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL31_3_DEFAULT     (_SMU_MSPUNVMNSREGIONFLAG3_MSPUNSRFL31_3_DEFAULT << 31) /**< Shifted mode DEFAULT for SMU_MSPUNVMNSREGIONFLAG3*/

/* Bit fields for SMU MSPUDMEMFSADDR */
#define _SMU_MSPUDMEMFSADDR_RESETVALUE                     0x00000000UL                                  /**< Default value for SMU_MSPUDMEMFSADDR        */
#define _SMU_MSPUDMEMFSADDR_MASK                           0xFFFFFFFFUL                                  /**< Mask for SMU_MSPUDMEMFSADDR                 */
#define _SMU_MSPUDMEMFSADDR_MSPUFSADDR_SHIFT               0                                             /**< Shift value for SMU_MSPUFSADDR              */
#define _SMU_MSPUDMEMFSADDR_MSPUFSADDR_MASK                0xFFFFFFFFUL                                  /**< Bit mask for SMU_MSPUFSADDR                 */
#define _SMU_MSPUDMEMFSADDR_MSPUFSADDR_DEFAULT             0x00000000UL                                  /**< Mode DEFAULT for SMU_MSPUDMEMFSADDR         */
#define SMU_MSPUDMEMFSADDR_MSPUFSADDR_DEFAULT              (_SMU_MSPUDMEMFSADDR_MSPUFSADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_MSPUDMEMFSADDR */

/* Bit fields for SMU MSPUNVMFSADDR */
#define _SMU_MSPUNVMFSADDR_RESETVALUE                      0x00000000UL                                 /**< Default value for SMU_MSPUNVMFSADDR         */
#define _SMU_MSPUNVMFSADDR_MASK                            0xFFFFFFFFUL                                 /**< Mask for SMU_MSPUNVMFSADDR                  */
#define _SMU_MSPUNVMFSADDR_MSPUFSADDR_SHIFT                0                                            /**< Shift value for SMU_MSPUFSADDR              */
#define _SMU_MSPUNVMFSADDR_MSPUFSADDR_MASK                 0xFFFFFFFFUL                                 /**< Bit mask for SMU_MSPUFSADDR                 */
#define _SMU_MSPUNVMFSADDR_MSPUFSADDR_DEFAULT              0x00000000UL                                 /**< Mode DEFAULT for SMU_MSPUNVMFSADDR          */
#define SMU_MSPUNVMFSADDR_MSPUFSADDR_DEFAULT               (_SMU_MSPUNVMFSADDR_MSPUFSADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_MSPUNVMFSADDR  */

/* Bit fields for SMU ESAURTYPES */
#define _SMU_ESAURTYPES_RESETVALUE                         0x01000000UL                               /**< Default value for SMU_ESAURTYPES            */
#define _SMU_ESAURTYPES_MASK                               0x01005555UL                               /**< Mask for SMU_ESAURTYPES                     */
#define SMU_ESAURTYPES_ESAUEN                              (0x1UL << 0)                               /**< ESAU enable                                 */
#define _SMU_ESAURTYPES_ESAUEN_SHIFT                       0                                          /**< Shift value for SMU_ESAUEN                  */
#define _SMU_ESAURTYPES_ESAUEN_MASK                        0x1UL                                      /**< Bit mask for SMU_ESAUEN                     */
#define _SMU_ESAURTYPES_ESAUEN_DEFAULT                     0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUEN_DEFAULT                      (_SMU_ESAURTYPES_ESAUEN_DEFAULT << 0)      /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR0SNS                           (0x1UL << 2)                               /**< Region 0/1 Non-Secure-Callable              */
#define _SMU_ESAURTYPES_ESAUR0SNS_SHIFT                    2                                          /**< Shift value for SMU_ESAUR0SNS               */
#define _SMU_ESAURTYPES_ESAUR0SNS_MASK                     0x4UL                                      /**< Bit mask for SMU_ESAUR0SNS                  */
#define _SMU_ESAURTYPES_ESAUR0SNS_DEFAULT                  0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR0SNS_DEFAULT                   (_SMU_ESAURTYPES_ESAUR0SNS_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR2SNS                           (0x1UL << 4)                               /**< Region 2/3 Non-Secure-Callable              */
#define _SMU_ESAURTYPES_ESAUR2SNS_SHIFT                    4                                          /**< Shift value for SMU_ESAUR2SNS               */
#define _SMU_ESAURTYPES_ESAUR2SNS_MASK                     0x10UL                                     /**< Bit mask for SMU_ESAUR2SNS                  */
#define _SMU_ESAURTYPES_ESAUR2SNS_DEFAULT                  0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR2SNS_DEFAULT                   (_SMU_ESAURTYPES_ESAUR2SNS_DEFAULT << 4)   /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR4SNS                           (0x1UL << 6)                               /**< Region 4/5 Non-Secure-Callable              */
#define _SMU_ESAURTYPES_ESAUR4SNS_SHIFT                    6                                          /**< Shift value for SMU_ESAUR4SNS               */
#define _SMU_ESAURTYPES_ESAUR4SNS_MASK                     0x40UL                                     /**< Bit mask for SMU_ESAUR4SNS                  */
#define _SMU_ESAURTYPES_ESAUR4SNS_DEFAULT                  0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR4SNS_DEFAULT                   (_SMU_ESAURTYPES_ESAUR4SNS_DEFAULT << 6)   /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR6SNS                           (0x1UL << 8)                               /**< Region 6/7 Non-Secure-Callable              */
#define _SMU_ESAURTYPES_ESAUR6SNS_SHIFT                    8                                          /**< Shift value for SMU_ESAUR6SNS               */
#define _SMU_ESAURTYPES_ESAUR6SNS_MASK                     0x100UL                                    /**< Bit mask for SMU_ESAUR6SNS                  */
#define _SMU_ESAURTYPES_ESAUR6SNS_DEFAULT                  0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR6SNS_DEFAULT                   (_SMU_ESAURTYPES_ESAUR6SNS_DEFAULT << 8)   /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR8SNS                           (0x1UL << 10)                              /**< Region 8/9 Non-Secure-Callable              */
#define _SMU_ESAURTYPES_ESAUR8SNS_SHIFT                    10                                         /**< Shift value for SMU_ESAUR8SNS               */
#define _SMU_ESAURTYPES_ESAUR8SNS_MASK                     0x400UL                                    /**< Bit mask for SMU_ESAUR8SNS                  */
#define _SMU_ESAURTYPES_ESAUR8SNS_DEFAULT                  0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR8SNS_DEFAULT                   (_SMU_ESAURTYPES_ESAUR8SNS_DEFAULT << 10)  /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR10SNS                          (0x1UL << 12)                              /**< Region 10/11 Non-Secure-Callable            */
#define _SMU_ESAURTYPES_ESAUR10SNS_SHIFT                   12                                         /**< Shift value for SMU_ESAUR10SNS              */
#define _SMU_ESAURTYPES_ESAUR10SNS_MASK                    0x1000UL                                   /**< Bit mask for SMU_ESAUR10SNS                 */
#define _SMU_ESAURTYPES_ESAUR10SNS_DEFAULT                 0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR10SNS_DEFAULT                  (_SMU_ESAURTYPES_ESAUR10SNS_DEFAULT << 12) /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */
#define SMU_ESAURTYPES_ESAUR12SNS                          (0x1UL << 14)                              /**< Region 12/13 Non-Secure-Callable            */
#define _SMU_ESAURTYPES_ESAUR12SNS_SHIFT                   14                                         /**< Shift value for SMU_ESAUR12SNS              */
#define _SMU_ESAURTYPES_ESAUR12SNS_MASK                    0x4000UL                                   /**< Bit mask for SMU_ESAUR12SNS                 */
#define _SMU_ESAURTYPES_ESAUR12SNS_DEFAULT                 0x00000000UL                               /**< Mode DEFAULT for SMU_ESAURTYPES             */
#define SMU_ESAURTYPES_ESAUR12SNS_DEFAULT                  (_SMU_ESAURTYPES_ESAUR12SNS_DEFAULT << 14) /**< Shifted mode DEFAULT for SMU_ESAURTYPES     */

/** @} End of group SIMG301_SMU_BitFields */
/** @} End of group SIMG301_SMU */
/**************************************************************************//**
 * @defgroup SIMG301_SMU_CFGNS SMU_CFGNS
 * @{
 * @brief SIMG301 SMU_CFGNS Register Declaration.
 *****************************************************************************/

/** SMU_CFGNS Register Declaration. */
typedef struct smu_cfgns_typedef{
  uint32_t       RESERVED0[1U];                 /**< Reserved for future use                            */
  __IM uint32_t  NSSTATUS;                      /**< Status Register                                    */
  __IOM uint32_t NSLOCK;                        /**< Lock Register                                      */
  __IOM uint32_t NSIF;                          /**< Interrupt Flag Register                            */
  __IOM uint32_t NSIEN;                         /**< Interrupt Enable Register                          */
  uint32_t       RESERVED1[3U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED2[8U];                 /**< Reserved for future use                            */
  __IOM uint32_t PPUNSPATD0;                    /**< PPU Privileged Access                              */
  __IOM uint32_t PPUNSPATD1;                    /**< PPU Privileged Access                              */
  uint32_t       RESERVED3[62U];                /**< Reserved for future use                            */
  __IM uint32_t  PPUNSFS;                       /**< PPU Fault Status                                   */
  __IM uint32_t  PPUNSFSADDR;                   /**< PPU Fault Status Address                           */
  uint32_t       RESERVED4[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t BMPUNSPATD0;                   /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED5[63U];                /**< Reserved for future use                            */
  uint32_t       RESERVED6[876U];               /**< Reserved for future use                            */
  uint32_t       RESERVED7[1U];                 /**< Reserved for future use                            */
  __IM uint32_t  NSSTATUS_SET;                  /**< Status Register                                    */
  __IOM uint32_t NSLOCK_SET;                    /**< Lock Register                                      */
  __IOM uint32_t NSIF_SET;                      /**< Interrupt Flag Register                            */
  __IOM uint32_t NSIEN_SET;                     /**< Interrupt Enable Register                          */
  uint32_t       RESERVED8[3U];                 /**< Reserved for future use                            */
  uint32_t       RESERVED9[8U];                 /**< Reserved for future use                            */
  __IOM uint32_t PPUNSPATD0_SET;                /**< PPU Privileged Access                              */
  __IOM uint32_t PPUNSPATD1_SET;                /**< PPU Privileged Access                              */
  uint32_t       RESERVED10[62U];               /**< Reserved for future use                            */
  __IM uint32_t  PPUNSFS_SET;                   /**< PPU Fault Status                                   */
  __IM uint32_t  PPUNSFSADDR_SET;               /**< PPU Fault Status Address                           */
  uint32_t       RESERVED11[2U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUNSPATD0_SET;               /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED12[63U];               /**< Reserved for future use                            */
  uint32_t       RESERVED13[876U];              /**< Reserved for future use                            */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
  __IM uint32_t  NSSTATUS_CLR;                  /**< Status Register                                    */
  __IOM uint32_t NSLOCK_CLR;                    /**< Lock Register                                      */
  __IOM uint32_t NSIF_CLR;                      /**< Interrupt Flag Register                            */
  __IOM uint32_t NSIEN_CLR;                     /**< Interrupt Enable Register                          */
  uint32_t       RESERVED15[3U];                /**< Reserved for future use                            */
  uint32_t       RESERVED16[8U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUNSPATD0_CLR;                /**< PPU Privileged Access                              */
  __IOM uint32_t PPUNSPATD1_CLR;                /**< PPU Privileged Access                              */
  uint32_t       RESERVED17[62U];               /**< Reserved for future use                            */
  __IM uint32_t  PPUNSFS_CLR;                   /**< PPU Fault Status                                   */
  __IM uint32_t  PPUNSFSADDR_CLR;               /**< PPU Fault Status Address                           */
  uint32_t       RESERVED18[2U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUNSPATD0_CLR;               /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED19[63U];               /**< Reserved for future use                            */
  uint32_t       RESERVED20[876U];              /**< Reserved for future use                            */
  uint32_t       RESERVED21[1U];                /**< Reserved for future use                            */
  __IM uint32_t  NSSTATUS_TGL;                  /**< Status Register                                    */
  __IOM uint32_t NSLOCK_TGL;                    /**< Lock Register                                      */
  __IOM uint32_t NSIF_TGL;                      /**< Interrupt Flag Register                            */
  __IOM uint32_t NSIEN_TGL;                     /**< Interrupt Enable Register                          */
  uint32_t       RESERVED22[3U];                /**< Reserved for future use                            */
  uint32_t       RESERVED23[8U];                /**< Reserved for future use                            */
  __IOM uint32_t PPUNSPATD0_TGL;                /**< PPU Privileged Access                              */
  __IOM uint32_t PPUNSPATD1_TGL;                /**< PPU Privileged Access                              */
  uint32_t       RESERVED24[62U];               /**< Reserved for future use                            */
  __IM uint32_t  PPUNSFS_TGL;                   /**< PPU Fault Status                                   */
  __IM uint32_t  PPUNSFSADDR_TGL;               /**< PPU Fault Status Address                           */
  uint32_t       RESERVED25[2U];                /**< Reserved for future use                            */
  __IOM uint32_t BMPUNSPATD0_TGL;               /**< BMPU Privileged Attribute                          */
  uint32_t       RESERVED26[63U];               /**< Reserved for future use                            */
} SMU_CFGNS_TypeDef;
/** @} End of group SIMG301_SMU_CFGNS */

/**************************************************************************//**
 * @addtogroup SIMG301_SMU_CFGNS
 * @{
 * @defgroup SIMG301_SMU_CFGNS_BitFields SMU_CFGNS Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SMU NSSTATUS */
#define _SMU_NSSTATUS_RESETVALUE                    0x00000000UL                            /**< Default value for SMU_NSSTATUS              */
#define _SMU_NSSTATUS_MASK                          0x00000001UL                            /**< Mask for SMU_NSSTATUS                       */
#define SMU_NSSTATUS_SMUNSLOCK                      (0x1UL << 0)                            /**< SMUNS Lock                                  */
#define _SMU_NSSTATUS_SMUNSLOCK_SHIFT               0                                       /**< Shift value for SMU_SMUNSLOCK               */
#define _SMU_NSSTATUS_SMUNSLOCK_MASK                0x1UL                                   /**< Bit mask for SMU_SMUNSLOCK                  */
#define _SMU_NSSTATUS_SMUNSLOCK_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for SMU_NSSTATUS               */
#define _SMU_NSSTATUS_SMUNSLOCK_UNLOCKED            0x00000000UL                            /**< Mode UNLOCKED for SMU_NSSTATUS              */
#define _SMU_NSSTATUS_SMUNSLOCK_LOCKED              0x00000001UL                            /**< Mode LOCKED for SMU_NSSTATUS                */
#define SMU_NSSTATUS_SMUNSLOCK_DEFAULT              (_SMU_NSSTATUS_SMUNSLOCK_DEFAULT << 0)  /**< Shifted mode DEFAULT for SMU_NSSTATUS       */
#define SMU_NSSTATUS_SMUNSLOCK_UNLOCKED             (_SMU_NSSTATUS_SMUNSLOCK_UNLOCKED << 0) /**< Shifted mode UNLOCKED for SMU_NSSTATUS      */
#define SMU_NSSTATUS_SMUNSLOCK_LOCKED               (_SMU_NSSTATUS_SMUNSLOCK_LOCKED << 0)   /**< Shifted mode LOCKED for SMU_NSSTATUS        */

/* Bit fields for SMU NSLOCK */
#define _SMU_NSLOCK_RESETVALUE                      0x00000000UL                            /**< Default value for SMU_NSLOCK                */
#define _SMU_NSLOCK_MASK                            0x00FFFFFFUL                            /**< Mask for SMU_NSLOCK                         */
#define _SMU_NSLOCK_SMUNSLOCKKEY_SHIFT              0                                       /**< Shift value for SMU_SMUNSLOCKKEY            */
#define _SMU_NSLOCK_SMUNSLOCKKEY_MASK               0xFFFFFFUL                              /**< Bit mask for SMU_SMUNSLOCKKEY               */
#define _SMU_NSLOCK_SMUNSLOCKKEY_DEFAULT            0x00000000UL                            /**< Mode DEFAULT for SMU_NSLOCK                 */
#define _SMU_NSLOCK_SMUNSLOCKKEY_UNLOCK             0x00ACCE55UL                            /**< Mode UNLOCK for SMU_NSLOCK                  */
#define SMU_NSLOCK_SMUNSLOCKKEY_DEFAULT             (_SMU_NSLOCK_SMUNSLOCKKEY_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_NSLOCK         */
#define SMU_NSLOCK_SMUNSLOCKKEY_UNLOCK              (_SMU_NSLOCK_SMUNSLOCKKEY_UNLOCK << 0)  /**< Shifted mode UNLOCK for SMU_NSLOCK          */

/* Bit fields for SMU NSIF */
#define _SMU_NSIF_RESETVALUE                        0x00000000UL                        /**< Default value for SMU_NSIF                  */
#define _SMU_NSIF_MASK                              0x00000005UL                        /**< Mask for SMU_NSIF                           */
#define SMU_NSIF_PPUNSPRIV                          (0x1UL << 0)                        /**< PPUNS Privilege Interrupt Flag              */
#define _SMU_NSIF_PPUNSPRIV_SHIFT                   0                                   /**< Shift value for SMU_PPUNSPRIV               */
#define _SMU_NSIF_PPUNSPRIV_MASK                    0x1UL                               /**< Bit mask for SMU_PPUNSPRIV                  */
#define _SMU_NSIF_PPUNSPRIV_DEFAULT                 0x00000000UL                        /**< Mode DEFAULT for SMU_NSIF                   */
#define SMU_NSIF_PPUNSPRIV_DEFAULT                  (_SMU_NSIF_PPUNSPRIV_DEFAULT << 0)  /**< Shifted mode DEFAULT for SMU_NSIF           */
#define SMU_NSIF_PPUNSINST                          (0x1UL << 2)                        /**< PPUNS Instruction Interrupt Flag            */
#define _SMU_NSIF_PPUNSINST_SHIFT                   2                                   /**< Shift value for SMU_PPUNSINST               */
#define _SMU_NSIF_PPUNSINST_MASK                    0x4UL                               /**< Bit mask for SMU_PPUNSINST                  */
#define _SMU_NSIF_PPUNSINST_DEFAULT                 0x00000000UL                        /**< Mode DEFAULT for SMU_NSIF                   */
#define SMU_NSIF_PPUNSINST_DEFAULT                  (_SMU_NSIF_PPUNSINST_DEFAULT << 2)  /**< Shifted mode DEFAULT for SMU_NSIF           */

/* Bit fields for SMU NSIEN */
#define _SMU_NSIEN_RESETVALUE                       0x00000000UL                        /**< Default value for SMU_NSIEN                 */
#define _SMU_NSIEN_MASK                             0x00000005UL                        /**< Mask for SMU_NSIEN                          */
#define SMU_NSIEN_PPUNSPRIV                         (0x1UL << 0)                        /**< PPUNS Privilege Interrupt Enable            */
#define _SMU_NSIEN_PPUNSPRIV_SHIFT                  0                                   /**< Shift value for SMU_PPUNSPRIV               */
#define _SMU_NSIEN_PPUNSPRIV_MASK                   0x1UL                               /**< Bit mask for SMU_PPUNSPRIV                  */
#define _SMU_NSIEN_PPUNSPRIV_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SMU_NSIEN                  */
#define SMU_NSIEN_PPUNSPRIV_DEFAULT                 (_SMU_NSIEN_PPUNSPRIV_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_NSIEN          */
#define SMU_NSIEN_PPUNSINST                         (0x1UL << 2)                        /**< PPUNS Instruction Interrupt Enable          */
#define _SMU_NSIEN_PPUNSINST_SHIFT                  2                                   /**< Shift value for SMU_PPUNSINST               */
#define _SMU_NSIEN_PPUNSINST_MASK                   0x4UL                               /**< Bit mask for SMU_PPUNSINST                  */
#define _SMU_NSIEN_PPUNSINST_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SMU_NSIEN                  */
#define SMU_NSIEN_PPUNSINST_DEFAULT                 (_SMU_NSIEN_PPUNSINST_DEFAULT << 2) /**< Shifted mode DEFAULT for SMU_NSIEN          */

/* Bit fields for SMU PPUNSPATD0 */
#define _SMU_PPUNSPATD0_RESETVALUE                  0xFFFFFFFFUL                                 /**< Default value for SMU_PPUNSPATD0            */
#define _SMU_PPUNSPATD0_MASK                        0xFFFFFFFFUL                                 /**< Mask for SMU_PPUNSPATD0                     */
#define SMU_PPUNSPATD0_SOCPLL0                      (0x1UL << 0)                                 /**< SOCPLL0 Privileged Access                   */
#define _SMU_PPUNSPATD0_SOCPLL0_SHIFT               0                                            /**< Shift value for SMU_SOCPLL0                 */
#define _SMU_PPUNSPATD0_SOCPLL0_MASK                0x1UL                                        /**< Bit mask for SMU_SOCPLL0                    */
#define _SMU_PPUNSPATD0_SOCPLL0_DEFAULT             0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_SOCPLL0_DEFAULT              (_SMU_PPUNSPATD0_SOCPLL0_DEFAULT << 0)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_CMU                          (0x1UL << 1)                                 /**< CMU Privileged Access                       */
#define _SMU_PPUNSPATD0_CMU_SHIFT                   1                                            /**< Shift value for SMU_CMU                     */
#define _SMU_PPUNSPATD0_CMU_MASK                    0x2UL                                        /**< Bit mask for SMU_CMU                        */
#define _SMU_PPUNSPATD0_CMU_DEFAULT                 0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_CMU_DEFAULT                  (_SMU_PPUNSPATD0_CMU_DEFAULT << 1)           /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_DMEM                         (0x1UL << 2)                                 /**< DMEM Privileged Access                      */
#define _SMU_PPUNSPATD0_DMEM_SHIFT                  2                                            /**< Shift value for SMU_DMEM                    */
#define _SMU_PPUNSPATD0_DMEM_MASK                   0x4UL                                        /**< Bit mask for SMU_DMEM                       */
#define _SMU_PPUNSPATD0_DMEM_DEFAULT                0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_DMEM_DEFAULT                 (_SMU_PPUNSPATD0_DMEM_DEFAULT << 2)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_HFRCO0                       (0x1UL << 3)                                 /**< HFRCO0 Privileged Access                    */
#define _SMU_PPUNSPATD0_HFRCO0_SHIFT                3                                            /**< Shift value for SMU_HFRCO0                  */
#define _SMU_PPUNSPATD0_HFRCO0_MASK                 0x8UL                                        /**< Bit mask for SMU_HFRCO0                     */
#define _SMU_PPUNSPATD0_HFRCO0_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_HFRCO0_DEFAULT               (_SMU_PPUNSPATD0_HFRCO0_DEFAULT << 3)        /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_LDMA0                        (0x1UL << 4)                                 /**< LDMA0 Privileged Access                     */
#define _SMU_PPUNSPATD0_LDMA0_SHIFT                 4                                            /**< Shift value for SMU_LDMA0                   */
#define _SMU_PPUNSPATD0_LDMA0_MASK                  0x10UL                                       /**< Bit mask for SMU_LDMA0                      */
#define _SMU_PPUNSPATD0_LDMA0_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_LDMA0_DEFAULT                (_SMU_PPUNSPATD0_LDMA0_DEFAULT << 4)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_FSRCO                        (0x1UL << 5)                                 /**< FSRCO Privileged Access                     */
#define _SMU_PPUNSPATD0_FSRCO_SHIFT                 5                                            /**< Shift value for SMU_FSRCO                   */
#define _SMU_PPUNSPATD0_FSRCO_MASK                  0x20UL                                       /**< Bit mask for SMU_FSRCO                      */
#define _SMU_PPUNSPATD0_FSRCO_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_FSRCO_DEFAULT                (_SMU_PPUNSPATD0_FSRCO_DEFAULT << 5)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_DPLL0                        (0x1UL << 6)                                 /**< DPLL0 Privileged Access                     */
#define _SMU_PPUNSPATD0_DPLL0_SHIFT                 6                                            /**< Shift value for SMU_DPLL0                   */
#define _SMU_PPUNSPATD0_DPLL0_MASK                  0x40UL                                       /**< Bit mask for SMU_DPLL0                      */
#define _SMU_PPUNSPATD0_DPLL0_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_DPLL0_DEFAULT                (_SMU_PPUNSPATD0_DPLL0_DEFAULT << 6)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_LFXO                         (0x1UL << 7)                                 /**< LFXO Privileged Access                      */
#define _SMU_PPUNSPATD0_LFXO_SHIFT                  7                                            /**< Shift value for SMU_LFXO                    */
#define _SMU_PPUNSPATD0_LFXO_MASK                   0x80UL                                       /**< Bit mask for SMU_LFXO                       */
#define _SMU_PPUNSPATD0_LFXO_DEFAULT                0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_LFXO_DEFAULT                 (_SMU_PPUNSPATD0_LFXO_DEFAULT << 7)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_LFRCO                        (0x1UL << 8)                                 /**< LFRCO Privileged Access                     */
#define _SMU_PPUNSPATD0_LFRCO_SHIFT                 8                                            /**< Shift value for SMU_LFRCO                   */
#define _SMU_PPUNSPATD0_LFRCO_MASK                  0x100UL                                      /**< Bit mask for SMU_LFRCO                      */
#define _SMU_PPUNSPATD0_LFRCO_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_LFRCO_DEFAULT                (_SMU_PPUNSPATD0_LFRCO_DEFAULT << 8)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_ULFRCO                       (0x1UL << 9)                                 /**< ULFRCO Privileged Access                    */
#define _SMU_PPUNSPATD0_ULFRCO_SHIFT                9                                            /**< Shift value for SMU_ULFRCO                  */
#define _SMU_PPUNSPATD0_ULFRCO_MASK                 0x200UL                                      /**< Bit mask for SMU_ULFRCO                     */
#define _SMU_PPUNSPATD0_ULFRCO_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_ULFRCO_DEFAULT               (_SMU_PPUNSPATD0_ULFRCO_DEFAULT << 9)        /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_SEMAPHORE0                   (0x1UL << 10)                                /**< SEMAPHORE0 Privileged Access                */
#define _SMU_PPUNSPATD0_SEMAPHORE0_SHIFT            10                                           /**< Shift value for SMU_SEMAPHORE0              */
#define _SMU_PPUNSPATD0_SEMAPHORE0_MASK             0x400UL                                      /**< Bit mask for SMU_SEMAPHORE0                 */
#define _SMU_PPUNSPATD0_SEMAPHORE0_DEFAULT          0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_SEMAPHORE0_DEFAULT           (_SMU_PPUNSPATD0_SEMAPHORE0_DEFAULT << 10)   /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_SEMAPHORE1                   (0x1UL << 11)                                /**< SEMAPHORE1 Privileged Access                */
#define _SMU_PPUNSPATD0_SEMAPHORE1_SHIFT            11                                           /**< Shift value for SMU_SEMAPHORE1              */
#define _SMU_PPUNSPATD0_SEMAPHORE1_MASK             0x800UL                                      /**< Bit mask for SMU_SEMAPHORE1                 */
#define _SMU_PPUNSPATD0_SEMAPHORE1_DEFAULT          0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_SEMAPHORE1_DEFAULT           (_SMU_PPUNSPATD0_SEMAPHORE1_DEFAULT << 11)   /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_L1ICACHE0                    (0x1UL << 12)                                /**< L1ICACHE0 Privileged Access                 */
#define _SMU_PPUNSPATD0_L1ICACHE0_SHIFT             12                                           /**< Shift value for SMU_L1ICACHE0               */
#define _SMU_PPUNSPATD0_L1ICACHE0_MASK              0x1000UL                                     /**< Bit mask for SMU_L1ICACHE0                  */
#define _SMU_PPUNSPATD0_L1ICACHE0_DEFAULT           0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_L1ICACHE0_DEFAULT            (_SMU_PPUNSPATD0_L1ICACHE0_DEFAULT << 12)    /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_PRS                          (0x1UL << 13)                                /**< PRS Privileged Access                       */
#define _SMU_PPUNSPATD0_PRS_SHIFT                   13                                           /**< Shift value for SMU_PRS                     */
#define _SMU_PPUNSPATD0_PRS_MASK                    0x2000UL                                     /**< Bit mask for SMU_PRS                        */
#define _SMU_PPUNSPATD0_PRS_DEFAULT                 0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_PRS_DEFAULT                  (_SMU_PPUNSPATD0_PRS_DEFAULT << 13)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_GPIO                         (0x1UL << 14)                                /**< GPIO Privileged Access                      */
#define _SMU_PPUNSPATD0_GPIO_SHIFT                  14                                           /**< Shift value for SMU_GPIO                    */
#define _SMU_PPUNSPATD0_GPIO_MASK                   0x4000UL                                     /**< Bit mask for SMU_GPIO                       */
#define _SMU_PPUNSPATD0_GPIO_DEFAULT                0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_GPIO_DEFAULT                 (_SMU_PPUNSPATD0_GPIO_DEFAULT << 14)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_LDMAXBAR0                    (0x1UL << 15)                                /**< LDMAXBAR0 Privileged Access                 */
#define _SMU_PPUNSPATD0_LDMAXBAR0_SHIFT             15                                           /**< Shift value for SMU_LDMAXBAR0               */
#define _SMU_PPUNSPATD0_LDMAXBAR0_MASK              0x8000UL                                     /**< Bit mask for SMU_LDMAXBAR0                  */
#define _SMU_PPUNSPATD0_LDMAXBAR0_DEFAULT           0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_LDMAXBAR0_DEFAULT            (_SMU_PPUNSPATD0_LDMAXBAR0_DEFAULT << 15)    /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_TIMER0                       (0x1UL << 16)                                /**< TIMER0 Privileged Access                    */
#define _SMU_PPUNSPATD0_TIMER0_SHIFT                16                                           /**< Shift value for SMU_TIMER0                  */
#define _SMU_PPUNSPATD0_TIMER0_MASK                 0x10000UL                                    /**< Bit mask for SMU_TIMER0                     */
#define _SMU_PPUNSPATD0_TIMER0_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_TIMER0_DEFAULT               (_SMU_PPUNSPATD0_TIMER0_DEFAULT << 16)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_TIMER1                       (0x1UL << 17)                                /**< TIMER1 Privileged Access                    */
#define _SMU_PPUNSPATD0_TIMER1_SHIFT                17                                           /**< Shift value for SMU_TIMER1                  */
#define _SMU_PPUNSPATD0_TIMER1_MASK                 0x20000UL                                    /**< Bit mask for SMU_TIMER1                     */
#define _SMU_PPUNSPATD0_TIMER1_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_TIMER1_DEFAULT               (_SMU_PPUNSPATD0_TIMER1_DEFAULT << 17)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_TIMER2                       (0x1UL << 18)                                /**< TIMER2 Privileged Access                    */
#define _SMU_PPUNSPATD0_TIMER2_SHIFT                18                                           /**< Shift value for SMU_TIMER2                  */
#define _SMU_PPUNSPATD0_TIMER2_MASK                 0x40000UL                                    /**< Bit mask for SMU_TIMER2                     */
#define _SMU_PPUNSPATD0_TIMER2_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_TIMER2_DEFAULT               (_SMU_PPUNSPATD0_TIMER2_DEFAULT << 18)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_TIMER3                       (0x1UL << 19)                                /**< TIMER3 Privileged Access                    */
#define _SMU_PPUNSPATD0_TIMER3_SHIFT                19                                           /**< Shift value for SMU_TIMER3                  */
#define _SMU_PPUNSPATD0_TIMER3_MASK                 0x80000UL                                    /**< Bit mask for SMU_TIMER3                     */
#define _SMU_PPUNSPATD0_TIMER3_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_TIMER3_DEFAULT               (_SMU_PPUNSPATD0_TIMER3_DEFAULT << 19)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_ETAMPDET                     (0x1UL << 20)                                /**< ETAMPDET Privileged Access                  */
#define _SMU_PPUNSPATD0_ETAMPDET_SHIFT              20                                           /**< Shift value for SMU_ETAMPDET                */
#define _SMU_PPUNSPATD0_ETAMPDET_MASK               0x100000UL                                   /**< Bit mask for SMU_ETAMPDET                   */
#define _SMU_PPUNSPATD0_ETAMPDET_DEFAULT            0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_ETAMPDET_DEFAULT             (_SMU_PPUNSPATD0_ETAMPDET_DEFAULT << 20)     /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_L2ICACHE0                    (0x1UL << 21)                                /**< L2ICACHE0 Privileged Access                 */
#define _SMU_PPUNSPATD0_L2ICACHE0_SHIFT             21                                           /**< Shift value for SMU_L2ICACHE0               */
#define _SMU_PPUNSPATD0_L2ICACHE0_MASK              0x200000UL                                   /**< Bit mask for SMU_L2ICACHE0                  */
#define _SMU_PPUNSPATD0_L2ICACHE0_DEFAULT           0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_L2ICACHE0_DEFAULT            (_SMU_PPUNSPATD0_L2ICACHE0_DEFAULT << 21)    /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_PIXELRZ0                     (0x1UL << 22)                                /**< PIXELRZ0 Privileged Access                  */
#define _SMU_PPUNSPATD0_PIXELRZ0_SHIFT              22                                           /**< Shift value for SMU_PIXELRZ0                */
#define _SMU_PPUNSPATD0_PIXELRZ0_MASK               0x400000UL                                   /**< Bit mask for SMU_PIXELRZ0                   */
#define _SMU_PPUNSPATD0_PIXELRZ0_DEFAULT            0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_PIXELRZ0_DEFAULT             (_SMU_PPUNSPATD0_PIXELRZ0_DEFAULT << 22)     /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_BURTC                        (0x1UL << 23)                                /**< BURTC Privileged Access                     */
#define _SMU_PPUNSPATD0_BURTC_SHIFT                 23                                           /**< Shift value for SMU_BURTC                   */
#define _SMU_PPUNSPATD0_BURTC_MASK                  0x800000UL                                   /**< Bit mask for SMU_BURTC                      */
#define _SMU_PPUNSPATD0_BURTC_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_BURTC_DEFAULT                (_SMU_PPUNSPATD0_BURTC_DEFAULT << 23)        /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_I2C1                         (0x1UL << 24)                                /**< I2C1 Privileged Access                      */
#define _SMU_PPUNSPATD0_I2C1_SHIFT                  24                                           /**< Shift value for SMU_I2C1                    */
#define _SMU_PPUNSPATD0_I2C1_MASK                   0x1000000UL                                  /**< Bit mask for SMU_I2C1                       */
#define _SMU_PPUNSPATD0_I2C1_DEFAULT                0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_I2C1_DEFAULT                 (_SMU_PPUNSPATD0_I2C1_DEFAULT << 24)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_CHIPTESTCTRL                 (0x1UL << 25)                                /**< CHIPTESTCTRL Privileged Access              */
#define _SMU_PPUNSPATD0_CHIPTESTCTRL_SHIFT          25                                           /**< Shift value for SMU_CHIPTESTCTRL            */
#define _SMU_PPUNSPATD0_CHIPTESTCTRL_MASK           0x2000000UL                                  /**< Bit mask for SMU_CHIPTESTCTRL               */
#define _SMU_PPUNSPATD0_CHIPTESTCTRL_DEFAULT        0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_CHIPTESTCTRL_DEFAULT         (_SMU_PPUNSPATD0_CHIPTESTCTRL_DEFAULT << 25) /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_SYSCFGCFGNS                  (0x1UL << 26)                                /**< SYSCFGCFGNS Privileged Access               */
#define _SMU_PPUNSPATD0_SYSCFGCFGNS_SHIFT           26                                           /**< Shift value for SMU_SYSCFGCFGNS             */
#define _SMU_PPUNSPATD0_SYSCFGCFGNS_MASK            0x4000000UL                                  /**< Bit mask for SMU_SYSCFGCFGNS                */
#define _SMU_PPUNSPATD0_SYSCFGCFGNS_DEFAULT         0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_SYSCFGCFGNS_DEFAULT          (_SMU_PPUNSPATD0_SYSCFGCFGNS_DEFAULT << 26)  /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_SYSCFG                       (0x1UL << 27)                                /**< SYSCFG Privileged Access                    */
#define _SMU_PPUNSPATD0_SYSCFG_SHIFT                27                                           /**< Shift value for SMU_SYSCFG                  */
#define _SMU_PPUNSPATD0_SYSCFG_MASK                 0x8000000UL                                  /**< Bit mask for SMU_SYSCFG                     */
#define _SMU_PPUNSPATD0_SYSCFG_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_SYSCFG_DEFAULT               (_SMU_PPUNSPATD0_SYSCFG_DEFAULT << 27)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_BURAM                        (0x1UL << 28)                                /**< BURAM Privileged Access                     */
#define _SMU_PPUNSPATD0_BURAM_SHIFT                 28                                           /**< Shift value for SMU_BURAM                   */
#define _SMU_PPUNSPATD0_BURAM_MASK                  0x10000000UL                                 /**< Bit mask for SMU_BURAM                      */
#define _SMU_PPUNSPATD0_BURAM_DEFAULT               0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_BURAM_DEFAULT                (_SMU_PPUNSPATD0_BURAM_DEFAULT << 28)        /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_GPCRC0                       (0x1UL << 29)                                /**< GPCRC0 Privileged Access                    */
#define _SMU_PPUNSPATD0_GPCRC0_SHIFT                29                                           /**< Shift value for SMU_GPCRC0                  */
#define _SMU_PPUNSPATD0_GPCRC0_MASK                 0x20000000UL                                 /**< Bit mask for SMU_GPCRC0                     */
#define _SMU_PPUNSPATD0_GPCRC0_DEFAULT              0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_GPCRC0_DEFAULT               (_SMU_PPUNSPATD0_GPCRC0_DEFAULT << 29)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_LEDDRV0                      (0x1UL << 30)                                /**< LEDDRV0 Privileged Access                   */
#define _SMU_PPUNSPATD0_LEDDRV0_SHIFT               30                                           /**< Shift value for SMU_LEDDRV0                 */
#define _SMU_PPUNSPATD0_LEDDRV0_MASK                0x40000000UL                                 /**< Bit mask for SMU_LEDDRV0                    */
#define _SMU_PPUNSPATD0_LEDDRV0_DEFAULT             0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_LEDDRV0_DEFAULT              (_SMU_PPUNSPATD0_LEDDRV0_DEFAULT << 30)      /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */
#define SMU_PPUNSPATD0_EUSART1                      (0x1UL << 31)                                /**< EUSART1 Privileged Access                   */
#define _SMU_PPUNSPATD0_EUSART1_SHIFT               31                                           /**< Shift value for SMU_EUSART1                 */
#define _SMU_PPUNSPATD0_EUSART1_MASK                0x80000000UL                                 /**< Bit mask for SMU_EUSART1                    */
#define _SMU_PPUNSPATD0_EUSART1_DEFAULT             0x00000001UL                                 /**< Mode DEFAULT for SMU_PPUNSPATD0             */
#define SMU_PPUNSPATD0_EUSART1_DEFAULT              (_SMU_PPUNSPATD0_EUSART1_DEFAULT << 31)      /**< Shifted mode DEFAULT for SMU_PPUNSPATD0     */

/* Bit fields for SMU PPUNSPATD1 */
#define _SMU_PPUNSPATD1_RESETVALUE                  0xFFFFFFFFUL                                     /**< Default value for SMU_PPUNSPATD1            */
#define _SMU_PPUNSPATD1_MASK                        0xFFFFFFFFUL                                     /**< Mask for SMU_PPUNSPATD1                     */
#define SMU_PPUNSPATD1_EUSART2                      (0x1UL << 0)                                     /**< EUSART2 Privileged Access                   */
#define _SMU_PPUNSPATD1_EUSART2_SHIFT               0                                                /**< Shift value for SMU_EUSART2                 */
#define _SMU_PPUNSPATD1_EUSART2_MASK                0x1UL                                            /**< Bit mask for SMU_EUSART2                    */
#define _SMU_PPUNSPATD1_EUSART2_DEFAULT             0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_EUSART2_DEFAULT              (_SMU_PPUNSPATD1_EUSART2_DEFAULT << 0)           /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_PIXELRZ1                     (0x1UL << 1)                                     /**< PIXELRZ1 Privileged Access                  */
#define _SMU_PPUNSPATD1_PIXELRZ1_SHIFT              1                                                /**< Shift value for SMU_PIXELRZ1                */
#define _SMU_PPUNSPATD1_PIXELRZ1_MASK               0x2UL                                            /**< Bit mask for SMU_PIXELRZ1                   */
#define _SMU_PPUNSPATD1_PIXELRZ1_DEFAULT            0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_PIXELRZ1_DEFAULT             (_SMU_PPUNSPATD1_PIXELRZ1_DEFAULT << 1)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_I2C2                         (0x1UL << 2)                                     /**< I2C2 Privileged Access                      */
#define _SMU_PPUNSPATD1_I2C2_SHIFT                  2                                                /**< Shift value for SMU_I2C2                    */
#define _SMU_PPUNSPATD1_I2C2_MASK                   0x4UL                                            /**< Bit mask for SMU_I2C2                       */
#define _SMU_PPUNSPATD1_I2C2_DEFAULT                0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_I2C2_DEFAULT                 (_SMU_PPUNSPATD1_I2C2_DEFAULT << 2)              /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_WDOG1                        (0x1UL << 3)                                     /**< WDOG1 Privileged Access                     */
#define _SMU_PPUNSPATD1_WDOG1_SHIFT                 3                                                /**< Shift value for SMU_WDOG1                   */
#define _SMU_PPUNSPATD1_WDOG1_MASK                  0x8UL                                            /**< Bit mask for SMU_WDOG1                      */
#define _SMU_PPUNSPATD1_WDOG1_DEFAULT               0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_WDOG1_DEFAULT                (_SMU_PPUNSPATD1_WDOG1_DEFAULT << 3)             /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_RPA                          (0x1UL << 4)                                     /**< RPA Privileged Access                       */
#define _SMU_PPUNSPATD1_RPA_SHIFT                   4                                                /**< Shift value for SMU_RPA                     */
#define _SMU_PPUNSPATD1_RPA_MASK                    0x10UL                                           /**< Bit mask for SMU_RPA                        */
#define _SMU_PPUNSPATD1_RPA_DEFAULT                 0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_RPA_DEFAULT                  (_SMU_PPUNSPATD1_RPA_DEFAULT << 4)               /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_KSU                          (0x1UL << 5)                                     /**< KSU Privileged Access                       */
#define _SMU_PPUNSPATD1_KSU_SHIFT                   5                                                /**< Shift value for SMU_KSU                     */
#define _SMU_PPUNSPATD1_KSU_MASK                    0x20UL                                           /**< Bit mask for SMU_KSU                        */
#define _SMU_PPUNSPATD1_KSU_DEFAULT                 0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_KSU_DEFAULT                  (_SMU_PPUNSPATD1_KSU_DEFAULT << 5)               /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_DEVINFO                      (0x1UL << 6)                                     /**< DEVINFO Privileged Access                   */
#define _SMU_PPUNSPATD1_DEVINFO_SHIFT               6                                                /**< Shift value for SMU_DEVINFO                 */
#define _SMU_PPUNSPATD1_DEVINFO_MASK                0x40UL                                           /**< Bit mask for SMU_DEVINFO                    */
#define _SMU_PPUNSPATD1_DEVINFO_DEFAULT             0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_DEVINFO_DEFAULT              (_SMU_PPUNSPATD1_DEVINFO_DEFAULT << 6)           /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_LETIMER0                     (0x1UL << 7)                                     /**< LETIMER0 Privileged Access                  */
#define _SMU_PPUNSPATD1_LETIMER0_SHIFT              7                                                /**< Shift value for SMU_LETIMER0                */
#define _SMU_PPUNSPATD1_LETIMER0_MASK               0x80UL                                           /**< Bit mask for SMU_LETIMER0                   */
#define _SMU_PPUNSPATD1_LETIMER0_DEFAULT            0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_LETIMER0_DEFAULT             (_SMU_PPUNSPATD1_LETIMER0_DEFAULT << 7)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_ADC0                         (0x1UL << 8)                                     /**< ADC0 Privileged Access                      */
#define _SMU_PPUNSPATD1_ADC0_SHIFT                  8                                                /**< Shift value for SMU_ADC0                    */
#define _SMU_PPUNSPATD1_ADC0_MASK                   0x100UL                                          /**< Bit mask for SMU_ADC0                       */
#define _SMU_PPUNSPATD1_ADC0_DEFAULT                0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_ADC0_DEFAULT                 (_SMU_PPUNSPATD1_ADC0_DEFAULT << 8)              /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_ACMP0                        (0x1UL << 9)                                     /**< ACMP0 Privileged Access                     */
#define _SMU_PPUNSPATD1_ACMP0_SHIFT                 9                                                /**< Shift value for SMU_ACMP0                   */
#define _SMU_PPUNSPATD1_ACMP0_MASK                  0x200UL                                          /**< Bit mask for SMU_ACMP0                      */
#define _SMU_PPUNSPATD1_ACMP0_DEFAULT               0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_ACMP0_DEFAULT                (_SMU_PPUNSPATD1_ACMP0_DEFAULT << 9)             /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_ACMP1                        (0x1UL << 10)                                    /**< ACMP1 Privileged Access                     */
#define _SMU_PPUNSPATD1_ACMP1_SHIFT                 10                                               /**< Shift value for SMU_ACMP1                   */
#define _SMU_PPUNSPATD1_ACMP1_MASK                  0x400UL                                          /**< Bit mask for SMU_ACMP1                      */
#define _SMU_PPUNSPATD1_ACMP1_DEFAULT               0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_ACMP1_DEFAULT                (_SMU_PPUNSPATD1_ACMP1_DEFAULT << 10)            /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_AMUXCP0                      (0x1UL << 11)                                    /**< AMUXCP0 Privileged Access                   */
#define _SMU_PPUNSPATD1_AMUXCP0_SHIFT               11                                               /**< Shift value for SMU_AMUXCP0                 */
#define _SMU_PPUNSPATD1_AMUXCP0_MASK                0x800UL                                          /**< Bit mask for SMU_AMUXCP0                    */
#define _SMU_PPUNSPATD1_AMUXCP0_DEFAULT             0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_AMUXCP0_DEFAULT              (_SMU_PPUNSPATD1_AMUXCP0_DEFAULT << 11)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_PCNT0                        (0x1UL << 12)                                    /**< PCNT0 Privileged Access                     */
#define _SMU_PPUNSPATD1_PCNT0_SHIFT                 12                                               /**< Shift value for SMU_PCNT0                   */
#define _SMU_PPUNSPATD1_PCNT0_MASK                  0x1000UL                                         /**< Bit mask for SMU_PCNT0                      */
#define _SMU_PPUNSPATD1_PCNT0_DEFAULT               0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_PCNT0_DEFAULT                (_SMU_PPUNSPATD1_PCNT0_DEFAULT << 12)            /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_HFRCOEM23                    (0x1UL << 13)                                    /**< HFRCOEM23 Privileged Access                 */
#define _SMU_PPUNSPATD1_HFRCOEM23_SHIFT             13                                               /**< Shift value for SMU_HFRCOEM23               */
#define _SMU_PPUNSPATD1_HFRCOEM23_MASK              0x2000UL                                         /**< Bit mask for SMU_HFRCOEM23                  */
#define _SMU_PPUNSPATD1_HFRCOEM23_DEFAULT           0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_HFRCOEM23_DEFAULT            (_SMU_PPUNSPATD1_HFRCOEM23_DEFAULT << 13)        /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_HFXO0                        (0x1UL << 14)                                    /**< HFXO0 Privileged Access                     */
#define _SMU_PPUNSPATD1_HFXO0_SHIFT                 14                                               /**< Shift value for SMU_HFXO0                   */
#define _SMU_PPUNSPATD1_HFXO0_MASK                  0x4000UL                                         /**< Bit mask for SMU_HFXO0                      */
#define _SMU_PPUNSPATD1_HFXO0_DEFAULT               0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_HFXO0_DEFAULT                (_SMU_PPUNSPATD1_HFXO0_DEFAULT << 14)            /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_I2C0                         (0x1UL << 15)                                    /**< I2C0 Privileged Access                      */
#define _SMU_PPUNSPATD1_I2C0_SHIFT                  15                                               /**< Shift value for SMU_I2C0                    */
#define _SMU_PPUNSPATD1_I2C0_MASK                   0x8000UL                                         /**< Bit mask for SMU_I2C0                       */
#define _SMU_PPUNSPATD1_I2C0_DEFAULT                0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_I2C0_DEFAULT                 (_SMU_PPUNSPATD1_I2C0_DEFAULT << 15)             /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_WDOG0                        (0x1UL << 16)                                    /**< WDOG0 Privileged Access                     */
#define _SMU_PPUNSPATD1_WDOG0_SHIFT                 16                                               /**< Shift value for SMU_WDOG0                   */
#define _SMU_PPUNSPATD1_WDOG0_MASK                  0x10000UL                                        /**< Bit mask for SMU_WDOG0                      */
#define _SMU_PPUNSPATD1_WDOG0_DEFAULT               0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_WDOG0_DEFAULT                (_SMU_PPUNSPATD1_WDOG0_DEFAULT << 16)            /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_EUSART0                      (0x1UL << 17)                                    /**< EUSART0 Privileged Access                   */
#define _SMU_PPUNSPATD1_EUSART0_SHIFT               17                                               /**< Shift value for SMU_EUSART0                 */
#define _SMU_PPUNSPATD1_EUSART0_MASK                0x20000UL                                        /**< Bit mask for SMU_EUSART0                    */
#define _SMU_PPUNSPATD1_EUSART0_DEFAULT             0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_EUSART0_DEFAULT              (_SMU_PPUNSPATD1_EUSART0_DEFAULT << 17)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SYMCRYPTODMACTRL             (0x1UL << 18)                                    /**< SYMCRYPTODMACTRL Privileged Access          */
#define _SMU_PPUNSPATD1_SYMCRYPTODMACTRL_SHIFT      18                                               /**< Shift value for SMU_SYMCRYPTODMACTRL        */
#define _SMU_PPUNSPATD1_SYMCRYPTODMACTRL_MASK       0x40000UL                                        /**< Bit mask for SMU_SYMCRYPTODMACTRL           */
#define _SMU_PPUNSPATD1_SYMCRYPTODMACTRL_DEFAULT    0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SYMCRYPTODMACTRL_DEFAULT     (_SMU_PPUNSPATD1_SYMCRYPTODMACTRL_DEFAULT << 18) /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_LPWAESDMACTRL                (0x1UL << 19)                                    /**< LPWAESDMACTRL Privileged Access             */
#define _SMU_PPUNSPATD1_LPWAESDMACTRL_SHIFT         19                                               /**< Shift value for SMU_LPWAESDMACTRL           */
#define _SMU_PPUNSPATD1_LPWAESDMACTRL_MASK          0x80000UL                                        /**< Bit mask for SMU_LPWAESDMACTRL              */
#define _SMU_PPUNSPATD1_LPWAESDMACTRL_DEFAULT       0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_LPWAESDMACTRL_DEFAULT        (_SMU_PPUNSPATD1_LPWAESDMACTRL_DEFAULT << 19)    /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SMU                          (0x1UL << 20)                                    /**< SMU Privileged Access                       */
#define _SMU_PPUNSPATD1_SMU_SHIFT                   20                                               /**< Shift value for SMU_SMU                     */
#define _SMU_PPUNSPATD1_SMU_MASK                    0x100000UL                                       /**< Bit mask for SMU_SMU                        */
#define _SMU_PPUNSPATD1_SMU_DEFAULT                 0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SMU_DEFAULT                  (_SMU_PPUNSPATD1_SMU_DEFAULT << 20)              /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SMUCFGNS                     (0x1UL << 21)                                    /**< SMUCFGNS Privileged Access                  */
#define _SMU_PPUNSPATD1_SMUCFGNS_SHIFT              21                                               /**< Shift value for SMU_SMUCFGNS                */
#define _SMU_PPUNSPATD1_SMUCFGNS_MASK               0x200000UL                                       /**< Bit mask for SMU_SMUCFGNS                   */
#define _SMU_PPUNSPATD1_SMUCFGNS_DEFAULT            0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SMUCFGNS_DEFAULT             (_SMU_PPUNSPATD1_SMUCFGNS_DEFAULT << 21)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SEMAILBOX                    (0x1UL << 22)                                    /**< SEMAILBOX Privileged Access                 */
#define _SMU_PPUNSPATD1_SEMAILBOX_SHIFT             22                                               /**< Shift value for SMU_SEMAILBOX               */
#define _SMU_PPUNSPATD1_SEMAILBOX_MASK              0x400000UL                                       /**< Bit mask for SMU_SEMAILBOX                  */
#define _SMU_PPUNSPATD1_SEMAILBOX_DEFAULT           0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SEMAILBOX_DEFAULT            (_SMU_PPUNSPATD1_SEMAILBOX_DEFAULT << 22)        /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_QSPI0MANUAL                  (0x1UL << 23)                                    /**< QSPI0MANUAL Privileged Access               */
#define _SMU_PPUNSPATD1_QSPI0MANUAL_SHIFT           23                                               /**< Shift value for SMU_QSPI0MANUAL             */
#define _SMU_PPUNSPATD1_QSPI0MANUAL_MASK            0x800000UL                                       /**< Bit mask for SMU_QSPI0MANUAL                */
#define _SMU_PPUNSPATD1_QSPI0MANUAL_DEFAULT         0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_QSPI0MANUAL_DEFAULT          (_SMU_PPUNSPATD1_QSPI0MANUAL_DEFAULT << 23)      /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_AHBRADIO                     (0x1UL << 24)                                    /**< AHBRADIO Privileged Access                  */
#define _SMU_PPUNSPATD1_AHBRADIO_SHIFT              24                                               /**< Shift value for SMU_AHBRADIO                */
#define _SMU_PPUNSPATD1_AHBRADIO_MASK               0x1000000UL                                      /**< Bit mask for SMU_AHBRADIO                   */
#define _SMU_PPUNSPATD1_AHBRADIO_DEFAULT            0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_AHBRADIO_DEFAULT             (_SMU_PPUNSPATD1_AHBRADIO_DEFAULT << 24)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SYSRTC0                      (0x1UL << 25)                                    /**< SYSRTC0 Privileged Access                   */
#define _SMU_PPUNSPATD1_SYSRTC0_SHIFT               25                                               /**< Shift value for SMU_SYSRTC0                 */
#define _SMU_PPUNSPATD1_SYSRTC0_MASK                0x2000000UL                                      /**< Bit mask for SMU_SYSRTC0                    */
#define _SMU_PPUNSPATD1_SYSRTC0_DEFAULT             0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SYSRTC0_DEFAULT              (_SMU_PPUNSPATD1_SYSRTC0_DEFAULT << 25)          /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SCRATCHPAD                   (0x1UL << 26)                                    /**< SCRATCHPAD Privileged Access                */
#define _SMU_PPUNSPATD1_SCRATCHPAD_SHIFT            26                                               /**< Shift value for SMU_SCRATCHPAD              */
#define _SMU_PPUNSPATD1_SCRATCHPAD_MASK             0x4000000UL                                      /**< Bit mask for SMU_SCRATCHPAD                 */
#define _SMU_PPUNSPATD1_SCRATCHPAD_DEFAULT          0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SCRATCHPAD_DEFAULT           (_SMU_PPUNSPATD1_SCRATCHPAD_DEFAULT << 26)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_EMU                          (0x1UL << 27)                                    /**< EMU Privileged Access                       */
#define _SMU_PPUNSPATD1_EMU_SHIFT                   27                                               /**< Shift value for SMU_EMU                     */
#define _SMU_PPUNSPATD1_EMU_MASK                    0x8000000UL                                      /**< Bit mask for SMU_EMU                        */
#define _SMU_PPUNSPATD1_EMU_DEFAULT                 0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_EMU_DEFAULT                  (_SMU_PPUNSPATD1_EMU_DEFAULT << 27)              /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_EMUCFGNS                     (0x1UL << 28)                                    /**< EMUCFGNS Privileged Access                  */
#define _SMU_PPUNSPATD1_EMUCFGNS_SHIFT              28                                               /**< Shift value for SMU_EMUCFGNS                */
#define _SMU_PPUNSPATD1_EMUCFGNS_MASK               0x10000000UL                                     /**< Bit mask for SMU_EMUCFGNS                   */
#define _SMU_PPUNSPATD1_EMUCFGNS_DEFAULT            0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_EMUCFGNS_DEFAULT             (_SMU_PPUNSPATD1_EMUCFGNS_DEFAULT << 28)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_SEPORTAL                     (0x1UL << 29)                                    /**< SEPORTAL Privileged Access                  */
#define _SMU_PPUNSPATD1_SEPORTAL_SHIFT              29                                               /**< Shift value for SMU_SEPORTAL                */
#define _SMU_PPUNSPATD1_SEPORTAL_MASK               0x20000000UL                                     /**< Bit mask for SMU_SEPORTAL                   */
#define _SMU_PPUNSPATD1_SEPORTAL_DEFAULT            0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_SEPORTAL_DEFAULT             (_SMU_PPUNSPATD1_SEPORTAL_DEFAULT << 29)         /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_LPW0PORTAL                   (0x1UL << 30)                                    /**< LPW0PORTAL Privileged Access                */
#define _SMU_PPUNSPATD1_LPW0PORTAL_SHIFT            30                                               /**< Shift value for SMU_LPW0PORTAL              */
#define _SMU_PPUNSPATD1_LPW0PORTAL_MASK             0x40000000UL                                     /**< Bit mask for SMU_LPW0PORTAL                 */
#define _SMU_PPUNSPATD1_LPW0PORTAL_DEFAULT          0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_LPW0PORTAL_DEFAULT           (_SMU_PPUNSPATD1_LPW0PORTAL_DEFAULT << 30)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */
#define SMU_PPUNSPATD1_HOSTPORTAL                   (0x1UL << 31)                                    /**< HOSTPORTAL Privileged Access                */
#define _SMU_PPUNSPATD1_HOSTPORTAL_SHIFT            31                                               /**< Shift value for SMU_HOSTPORTAL              */
#define _SMU_PPUNSPATD1_HOSTPORTAL_MASK             0x80000000UL                                     /**< Bit mask for SMU_HOSTPORTAL                 */
#define _SMU_PPUNSPATD1_HOSTPORTAL_DEFAULT          0x00000001UL                                     /**< Mode DEFAULT for SMU_PPUNSPATD1             */
#define SMU_PPUNSPATD1_HOSTPORTAL_DEFAULT           (_SMU_PPUNSPATD1_HOSTPORTAL_DEFAULT << 31)       /**< Shifted mode DEFAULT for SMU_PPUNSPATD1     */

/* Bit fields for SMU PPUNSFS */
#define _SMU_PPUNSFS_RESETVALUE                     0x00000000UL                              /**< Default value for SMU_PPUNSFS               */
#define _SMU_PPUNSFS_MASK                           0x000000FFUL                              /**< Mask for SMU_PPUNSFS                        */
#define _SMU_PPUNSFS_PPUFSPERIPHID_SHIFT            0                                         /**< Shift value for SMU_PPUFSPERIPHID           */
#define _SMU_PPUNSFS_PPUFSPERIPHID_MASK             0xFFUL                                    /**< Bit mask for SMU_PPUFSPERIPHID              */
#define _SMU_PPUNSFS_PPUFSPERIPHID_DEFAULT          0x00000000UL                              /**< Mode DEFAULT for SMU_PPUNSFS                */
#define SMU_PPUNSFS_PPUFSPERIPHID_DEFAULT           (_SMU_PPUNSFS_PPUFSPERIPHID_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_PPUNSFS        */

/* Bit fields for SMU PPUNSFSADDR */
#define _SMU_PPUNSFSADDR_RESETVALUE                 0x00000000UL                              /**< Default value for SMU_PPUNSFSADDR           */
#define _SMU_PPUNSFSADDR_MASK                       0xFFFFFFFFUL                              /**< Mask for SMU_PPUNSFSADDR                    */
#define _SMU_PPUNSFSADDR_PPUFSADDR_SHIFT            0                                         /**< Shift value for SMU_PPUFSADDR               */
#define _SMU_PPUNSFSADDR_PPUFSADDR_MASK             0xFFFFFFFFUL                              /**< Bit mask for SMU_PPUFSADDR                  */
#define _SMU_PPUNSFSADDR_PPUFSADDR_DEFAULT          0x00000000UL                              /**< Mode DEFAULT for SMU_PPUNSFSADDR            */
#define SMU_PPUNSFSADDR_PPUFSADDR_DEFAULT           (_SMU_PPUNSFSADDR_PPUFSADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for SMU_PPUNSFSADDR    */

/* Bit fields for SMU BMPUNSPATD0 */
#define _SMU_BMPUNSPATD0_RESETVALUE                 0x000000FFUL                                 /**< Default value for SMU_BMPUNSPATD0           */
#define _SMU_BMPUNSPATD0_MASK                       0x000000FFUL                                 /**< Mask for SMU_BMPUNSPATD0                    */
#define SMU_BMPUNSPATD0_LPWAESDMA                   (0x1UL << 0)                                 /**< LPWAESDMA Privileged Mode                   */
#define _SMU_BMPUNSPATD0_LPWAESDMA_SHIFT            0                                            /**< Shift value for SMU_LPWAESDMA               */
#define _SMU_BMPUNSPATD0_LPWAESDMA_MASK             0x1UL                                        /**< Bit mask for SMU_LPWAESDMA                  */
#define _SMU_BMPUNSPATD0_LPWAESDMA_DEFAULT          0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_LPWAESDMA_DEFAULT           (_SMU_BMPUNSPATD0_LPWAESDMA_DEFAULT << 0)    /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_AHBRADIOBUS                 (0x1UL << 1)                                 /**< AHBRADIOBUS Privileged Mode                 */
#define _SMU_BMPUNSPATD0_AHBRADIOBUS_SHIFT          1                                            /**< Shift value for SMU_AHBRADIOBUS             */
#define _SMU_BMPUNSPATD0_AHBRADIOBUS_MASK           0x2UL                                        /**< Bit mask for SMU_AHBRADIOBUS                */
#define _SMU_BMPUNSPATD0_AHBRADIOBUS_DEFAULT        0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_AHBRADIOBUS_DEFAULT         (_SMU_BMPUNSPATD0_AHBRADIOBUS_DEFAULT << 1)  /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_LDMA0PORT0                  (0x1UL << 2)                                 /**< LDMA0PORT0 Privileged Mode                  */
#define _SMU_BMPUNSPATD0_LDMA0PORT0_SHIFT           2                                            /**< Shift value for SMU_LDMA0PORT0              */
#define _SMU_BMPUNSPATD0_LDMA0PORT0_MASK            0x4UL                                        /**< Bit mask for SMU_LDMA0PORT0                 */
#define _SMU_BMPUNSPATD0_LDMA0PORT0_DEFAULT         0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_LDMA0PORT0_DEFAULT          (_SMU_BMPUNSPATD0_LDMA0PORT0_DEFAULT << 2)   /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_LDMA0PORT1                  (0x1UL << 3)                                 /**< LDMA0PORT1 Privileged Mode                  */
#define _SMU_BMPUNSPATD0_LDMA0PORT1_SHIFT           3                                            /**< Shift value for SMU_LDMA0PORT1              */
#define _SMU_BMPUNSPATD0_LDMA0PORT1_MASK            0x8UL                                        /**< Bit mask for SMU_LDMA0PORT1                 */
#define _SMU_BMPUNSPATD0_LDMA0PORT1_DEFAULT         0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_LDMA0PORT1_DEFAULT          (_SMU_BMPUNSPATD0_LDMA0PORT1_DEFAULT << 3)   /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_RPASLAXI                    (0x1UL << 4)                                 /**< RPASLAXI Privileged Mode                    */
#define _SMU_BMPUNSPATD0_RPASLAXI_SHIFT             4                                            /**< Shift value for SMU_RPASLAXI                */
#define _SMU_BMPUNSPATD0_RPASLAXI_MASK              0x10UL                                       /**< Bit mask for SMU_RPASLAXI                   */
#define _SMU_BMPUNSPATD0_RPASLAXI_DEFAULT           0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_RPASLAXI_DEFAULT            (_SMU_BMPUNSPATD0_RPASLAXI_DEFAULT << 4)     /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_SEEXTDMAPORT                (0x1UL << 5)                                 /**< SEEXTDMAPORT Privileged Mode                */
#define _SMU_BMPUNSPATD0_SEEXTDMAPORT_SHIFT         5                                            /**< Shift value for SMU_SEEXTDMAPORT            */
#define _SMU_BMPUNSPATD0_SEEXTDMAPORT_MASK          0x20UL                                       /**< Bit mask for SMU_SEEXTDMAPORT               */
#define _SMU_BMPUNSPATD0_SEEXTDMAPORT_DEFAULT       0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_SEEXTDMAPORT_DEFAULT        (_SMU_BMPUNSPATD0_SEEXTDMAPORT_DEFAULT << 5) /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_SYMCRYPTODMA                (0x1UL << 6)                                 /**< SYMCRYPTODMA Privileged Mode                */
#define _SMU_BMPUNSPATD0_SYMCRYPTODMA_SHIFT         6                                            /**< Shift value for SMU_SYMCRYPTODMA            */
#define _SMU_BMPUNSPATD0_SYMCRYPTODMA_MASK          0x40UL                                       /**< Bit mask for SMU_SYMCRYPTODMA               */
#define _SMU_BMPUNSPATD0_SYMCRYPTODMA_DEFAULT       0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_SYMCRYPTODMA_DEFAULT        (_SMU_BMPUNSPATD0_SYMCRYPTODMA_DEFAULT << 6) /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */
#define SMU_BMPUNSPATD0_SLAXICOREBUS                (0x1UL << 7)                                 /**< SLAXICOREBUS Privileged Mode                */
#define _SMU_BMPUNSPATD0_SLAXICOREBUS_SHIFT         7                                            /**< Shift value for SMU_SLAXICOREBUS            */
#define _SMU_BMPUNSPATD0_SLAXICOREBUS_MASK          0x80UL                                       /**< Bit mask for SMU_SLAXICOREBUS               */
#define _SMU_BMPUNSPATD0_SLAXICOREBUS_DEFAULT       0x00000001UL                                 /**< Mode DEFAULT for SMU_BMPUNSPATD0            */
#define SMU_BMPUNSPATD0_SLAXICOREBUS_DEFAULT        (_SMU_BMPUNSPATD0_SLAXICOREBUS_DEFAULT << 7) /**< Shifted mode DEFAULT for SMU_BMPUNSPATD0    */

/** @} End of group SIMG301_SMU_CFGNS_BitFields */
/** @} End of group SIMG301_SMU_CFGNS */
/** @} End of group Parts */

#endif // SIMG301_SMU_H
