/**************************************************************************//**
 * @file
 * @brief SIBG301 RPA register and bit field definitions
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
#ifndef SIBG301_RPA_H
#define SIBG301_RPA_H
#define RPA_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_RPA RPA
 * @{
 * @brief SIBG301 RPA Register Declaration.
 *****************************************************************************/

/** RPA Register Declaration. */
typedef struct rpa_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP version ID                                      */
  __IOM uint32_t EN;                            /**< Enable                                             */
  __IOM uint32_t SWRST;                         /**< Software Reset                                     */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  __IOM uint32_t PRAND;                         /**< Prand Register                                     */
  __IOM uint32_t HASH;                          /**< Hash Register                                      */
  __IOM uint32_t IRKADDR;                       /**< IRK Address Register                               */
  __IOM uint32_t IRKKEYSLOTID;                  /**< IRK Key Slot Register                              */
  __IOM uint32_t CMD;                           /**< Command Register                                   */
  __IOM uint32_t IF;                            /**< Interrupt Flags                                    */
  __IOM uint32_t IEN;                           /**< Interrupt Enables                                  */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  uint32_t       RESERVED0[20U];                /**< Reserved for future use                            */
  __IOM uint32_t IRKMASK;                       /**< IRK Masks                                          */
  uint32_t       RESERVED1[991U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP version ID                                      */
  __IOM uint32_t EN_SET;                        /**< Enable                                             */
  __IOM uint32_t SWRST_SET;                     /**< Software Reset                                     */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  __IOM uint32_t PRAND_SET;                     /**< Prand Register                                     */
  __IOM uint32_t HASH_SET;                      /**< Hash Register                                      */
  __IOM uint32_t IRKADDR_SET;                   /**< IRK Address Register                               */
  __IOM uint32_t IRKKEYSLOTID_SET;              /**< IRK Key Slot Register                              */
  __IOM uint32_t CMD_SET;                       /**< Command Register                                   */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enables                                  */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  uint32_t       RESERVED2[20U];                /**< Reserved for future use                            */
  __IOM uint32_t IRKMASK_SET;                   /**< IRK Masks                                          */
  uint32_t       RESERVED3[991U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP version ID                                      */
  __IOM uint32_t EN_CLR;                        /**< Enable                                             */
  __IOM uint32_t SWRST_CLR;                     /**< Software Reset                                     */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  __IOM uint32_t PRAND_CLR;                     /**< Prand Register                                     */
  __IOM uint32_t HASH_CLR;                      /**< Hash Register                                      */
  __IOM uint32_t IRKADDR_CLR;                   /**< IRK Address Register                               */
  __IOM uint32_t IRKKEYSLOTID_CLR;              /**< IRK Key Slot Register                              */
  __IOM uint32_t CMD_CLR;                       /**< Command Register                                   */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enables                                  */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  uint32_t       RESERVED4[20U];                /**< Reserved for future use                            */
  __IOM uint32_t IRKMASK_CLR;                   /**< IRK Masks                                          */
  uint32_t       RESERVED5[991U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP version ID                                      */
  __IOM uint32_t EN_TGL;                        /**< Enable                                             */
  __IOM uint32_t SWRST_TGL;                     /**< Software Reset                                     */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  __IOM uint32_t PRAND_TGL;                     /**< Prand Register                                     */
  __IOM uint32_t HASH_TGL;                      /**< Hash Register                                      */
  __IOM uint32_t IRKADDR_TGL;                   /**< IRK Address Register                               */
  __IOM uint32_t IRKKEYSLOTID_TGL;              /**< IRK Key Slot Register                              */
  __IOM uint32_t CMD_TGL;                       /**< Command Register                                   */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flags                                    */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enables                                  */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  uint32_t       RESERVED6[20U];                /**< Reserved for future use                            */
  __IOM uint32_t IRKMASK_TGL;                   /**< IRK Masks                                          */
} RPA_TypeDef;
/** @} End of group SIBG301_RPA */

/**************************************************************************//**
 * @addtogroup SIBG301_RPA
 * @{
 * @defgroup SIBG301_RPA_BitFields RPA Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for RPA IPVERSION */
#define _RPA_IPVERSION_RESETVALUE                 0x00000001UL                            /**< Default value for RPA_IPVERSION             */
#define _RPA_IPVERSION_MASK                       0xFFFFFFFFUL                            /**< Mask for RPA_IPVERSION                      */
#define _RPA_IPVERSION_IPVERSION_SHIFT            0                                       /**< Shift value for RPA_IPVERSION               */
#define _RPA_IPVERSION_IPVERSION_MASK             0xFFFFFFFFUL                            /**< Bit mask for RPA_IPVERSION                  */
#define _RPA_IPVERSION_IPVERSION_DEFAULT          0x00000001UL                            /**< Mode DEFAULT for RPA_IPVERSION              */
#define RPA_IPVERSION_IPVERSION_DEFAULT           (_RPA_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for RPA_IPVERSION      */

/* Bit fields for RPA EN */
#define _RPA_EN_RESETVALUE                        0x00000000UL                          /**< Default value for RPA_EN                    */
#define _RPA_EN_MASK                              0x00000003UL                          /**< Mask for RPA_EN                             */
#define RPA_EN_EN                                 (0x1UL << 0)                          /**< Module enable                               */
#define _RPA_EN_EN_SHIFT                          0                                     /**< Shift value for RPA_EN                      */
#define _RPA_EN_EN_MASK                           0x1UL                                 /**< Bit mask for RPA_EN                         */
#define _RPA_EN_EN_DEFAULT                        0x00000000UL                          /**< Mode DEFAULT for RPA_EN                     */
#define RPA_EN_EN_DEFAULT                         (_RPA_EN_EN_DEFAULT << 0)             /**< Shifted mode DEFAULT for RPA_EN             */
#define RPA_EN_DISABLING                          (0x1UL << 1)                          /**< Disablement busy status                     */
#define _RPA_EN_DISABLING_SHIFT                   1                                     /**< Shift value for RPA_DISABLING               */
#define _RPA_EN_DISABLING_MASK                    0x2UL                                 /**< Bit mask for RPA_DISABLING                  */
#define _RPA_EN_DISABLING_DEFAULT                 0x00000000UL                          /**< Mode DEFAULT for RPA_EN                     */
#define RPA_EN_DISABLING_DEFAULT                  (_RPA_EN_DISABLING_DEFAULT << 1)      /**< Shifted mode DEFAULT for RPA_EN             */

/* Bit fields for RPA SWRST */
#define _RPA_SWRST_RESETVALUE                     0x00000000UL                          /**< Default value for RPA_SWRST                 */
#define _RPA_SWRST_MASK                           0x00000003UL                          /**< Mask for RPA_SWRST                          */
#define RPA_SWRST_SWRST                           (0x1UL << 0)                          /**< Status of reset                             */
#define _RPA_SWRST_SWRST_SHIFT                    0                                     /**< Shift value for RPA_SWRST                   */
#define _RPA_SWRST_SWRST_MASK                     0x1UL                                 /**< Bit mask for RPA_SWRST                      */
#define _RPA_SWRST_SWRST_DEFAULT                  0x00000000UL                          /**< Mode DEFAULT for RPA_SWRST                  */
#define RPA_SWRST_SWRST_DEFAULT                   (_RPA_SWRST_SWRST_DEFAULT << 0)       /**< Shifted mode DEFAULT for RPA_SWRST          */
#define RPA_SWRST_RESETTING                       (0x1UL << 1)                          /**< Software reset command                      */
#define _RPA_SWRST_RESETTING_SHIFT                1                                     /**< Shift value for RPA_RESETTING               */
#define _RPA_SWRST_RESETTING_MASK                 0x2UL                                 /**< Bit mask for RPA_RESETTING                  */
#define _RPA_SWRST_RESETTING_DEFAULT              0x00000000UL                          /**< Mode DEFAULT for RPA_SWRST                  */
#define RPA_SWRST_RESETTING_DEFAULT               (_RPA_SWRST_RESETTING_DEFAULT << 1)   /**< Shifted mode DEFAULT for RPA_SWRST          */

/* Bit fields for RPA CTRL */
#define _RPA_CTRL_RESETVALUE                      0x00000000UL                           /**< Default value for RPA_CTRL                  */
#define _RPA_CTRL_MASK                            0xC00000FFUL                           /**< Mask for RPA_CTRL                           */
#define _RPA_CTRL_NUMIRK_SHIFT                    0                                      /**< Shift value for RPA_NUMIRK                  */
#define _RPA_CTRL_NUMIRK_MASK                     0xFFUL                                 /**< Bit mask for RPA_NUMIRK                     */
#define _RPA_CTRL_NUMIRK_DEFAULT                  0x00000000UL                           /**< Mode DEFAULT for RPA_CTRL                   */
#define RPA_CTRL_NUMIRK_DEFAULT                   (_RPA_CTRL_NUMIRK_DEFAULT << 0)        /**< Shifted mode DEFAULT for RPA_CTRL           */
#define RPA_CTRL_RPAAESSELREQ                     (0x1UL << 30)                          /**< Preemption request                          */
#define _RPA_CTRL_RPAAESSELREQ_SHIFT              30                                     /**< Shift value for RPA_RPAAESSELREQ            */
#define _RPA_CTRL_RPAAESSELREQ_MASK               0x40000000UL                           /**< Bit mask for RPA_RPAAESSELREQ               */
#define _RPA_CTRL_RPAAESSELREQ_DEFAULT            0x00000000UL                           /**< Mode DEFAULT for RPA_CTRL                   */
#define RPA_CTRL_RPAAESSELREQ_DEFAULT             (_RPA_CTRL_RPAAESSELREQ_DEFAULT << 30) /**< Shifted mode DEFAULT for RPA_CTRL           */
#define RPA_CTRL_IRKDMEMSEL                       (0x1UL << 31)                          /**< Selects DMEM IRKs                           */
#define _RPA_CTRL_IRKDMEMSEL_SHIFT                31                                     /**< Shift value for RPA_IRKDMEMSEL              */
#define _RPA_CTRL_IRKDMEMSEL_MASK                 0x80000000UL                           /**< Bit mask for RPA_IRKDMEMSEL                 */
#define _RPA_CTRL_IRKDMEMSEL_DEFAULT              0x00000000UL                           /**< Mode DEFAULT for RPA_CTRL                   */
#define RPA_CTRL_IRKDMEMSEL_DEFAULT               (_RPA_CTRL_IRKDMEMSEL_DEFAULT << 31)   /**< Shifted mode DEFAULT for RPA_CTRL           */

/* Bit fields for RPA PRAND */
#define _RPA_PRAND_RESETVALUE                     0x00000000UL                          /**< Default value for RPA_PRAND                 */
#define _RPA_PRAND_MASK                           0x00FFFFFFUL                          /**< Mask for RPA_PRAND                          */
#define _RPA_PRAND_PRAND_SHIFT                    0                                     /**< Shift value for RPA_PRAND                   */
#define _RPA_PRAND_PRAND_MASK                     0xFFFFFFUL                            /**< Bit mask for RPA_PRAND                      */
#define _RPA_PRAND_PRAND_DEFAULT                  0x00000000UL                          /**< Mode DEFAULT for RPA_PRAND                  */
#define RPA_PRAND_PRAND_DEFAULT                   (_RPA_PRAND_PRAND_DEFAULT << 0)       /**< Shifted mode DEFAULT for RPA_PRAND          */

/* Bit fields for RPA HASH */
#define _RPA_HASH_RESETVALUE                      0x00000000UL                          /**< Default value for RPA_HASH                  */
#define _RPA_HASH_MASK                            0x00FFFFFFUL                          /**< Mask for RPA_HASH                           */
#define _RPA_HASH_HASH_SHIFT                      0                                     /**< Shift value for RPA_HASH                    */
#define _RPA_HASH_HASH_MASK                       0xFFFFFFUL                            /**< Bit mask for RPA_HASH                       */
#define _RPA_HASH_HASH_DEFAULT                    0x00000000UL                          /**< Mode DEFAULT for RPA_HASH                   */
#define RPA_HASH_HASH_DEFAULT                     (_RPA_HASH_HASH_DEFAULT << 0)         /**< Shifted mode DEFAULT for RPA_HASH           */

/* Bit fields for RPA IRKADDR */
#define _RPA_IRKADDR_RESETVALUE                   0x00000000UL                          /**< Default value for RPA_IRKADDR               */
#define _RPA_IRKADDR_MASK                         0xFFFFFFFFUL                          /**< Mask for RPA_IRKADDR                        */
#define _RPA_IRKADDR_IRKADDR_SHIFT                0                                     /**< Shift value for RPA_IRKADDR                 */
#define _RPA_IRKADDR_IRKADDR_MASK                 0xFFFFFFFFUL                          /**< Bit mask for RPA_IRKADDR                    */
#define _RPA_IRKADDR_IRKADDR_DEFAULT              0x00000000UL                          /**< Mode DEFAULT for RPA_IRKADDR                */
#define RPA_IRKADDR_IRKADDR_DEFAULT               (_RPA_IRKADDR_IRKADDR_DEFAULT << 0)   /**< Shifted mode DEFAULT for RPA_IRKADDR        */

/* Bit fields for RPA IRKKEYSLOTID */
#define _RPA_IRKKEYSLOTID_RESETVALUE              0x00000000UL                                  /**< Default value for RPA_IRKKEYSLOTID          */
#define _RPA_IRKKEYSLOTID_MASK                    0x000000FFUL                                  /**< Mask for RPA_IRKKEYSLOTID                   */
#define _RPA_IRKKEYSLOTID_IRKKEYSLOTID_SHIFT      0                                             /**< Shift value for RPA_IRKKEYSLOTID            */
#define _RPA_IRKKEYSLOTID_IRKKEYSLOTID_MASK       0xFFUL                                        /**< Bit mask for RPA_IRKKEYSLOTID               */
#define _RPA_IRKKEYSLOTID_IRKKEYSLOTID_DEFAULT    0x00000000UL                                  /**< Mode DEFAULT for RPA_IRKKEYSLOTID           */
#define RPA_IRKKEYSLOTID_IRKKEYSLOTID_DEFAULT     (_RPA_IRKKEYSLOTID_IRKKEYSLOTID_DEFAULT << 0) /**< Shifted mode DEFAULT for RPA_IRKKEYSLOTID   */

/* Bit fields for RPA CMD */
#define _RPA_CMD_RESETVALUE                       0x00000000UL                          /**< Default value for RPA_CMD                   */
#define _RPA_CMD_MASK                             0x00000003UL                          /**< Mask for RPA_CMD                            */
#define RPA_CMD_START                             (0x1UL << 0)                          /**< Start RPA                                   */
#define _RPA_CMD_START_SHIFT                      0                                     /**< Shift value for RPA_START                   */
#define _RPA_CMD_START_MASK                       0x1UL                                 /**< Bit mask for RPA_START                      */
#define _RPA_CMD_START_DEFAULT                    0x00000000UL                          /**< Mode DEFAULT for RPA_CMD                    */
#define RPA_CMD_START_DEFAULT                     (_RPA_CMD_START_DEFAULT << 0)         /**< Shifted mode DEFAULT for RPA_CMD            */
#define RPA_CMD_STOP                              (0x1UL << 1)                          /**< Stop RPA                                    */
#define _RPA_CMD_STOP_SHIFT                       1                                     /**< Shift value for RPA_STOP                    */
#define _RPA_CMD_STOP_MASK                        0x2UL                                 /**< Bit mask for RPA_STOP                       */
#define _RPA_CMD_STOP_DEFAULT                     0x00000000UL                          /**< Mode DEFAULT for RPA_CMD                    */
#define RPA_CMD_STOP_DEFAULT                      (_RPA_CMD_STOP_DEFAULT << 1)          /**< Shifted mode DEFAULT for RPA_CMD            */

/* Bit fields for RPA IF */
#define _RPA_IF_RESETVALUE                        0x00000000UL                          /**< Default value for RPA_IF                    */
#define _RPA_IF_MASK                              0x00000007UL                          /**< Mask for RPA_IF                             */
#define RPA_IF_RPADONEIF                          (0x1UL << 0)                          /**< RPA done IF                                 */
#define _RPA_IF_RPADONEIF_SHIFT                   0                                     /**< Shift value for RPA_RPADONEIF               */
#define _RPA_IF_RPADONEIF_MASK                    0x1UL                                 /**< Bit mask for RPA_RPADONEIF                  */
#define _RPA_IF_RPADONEIF_DEFAULT                 0x00000000UL                          /**< Mode DEFAULT for RPA_IF                     */
#define RPA_IF_RPADONEIF_DEFAULT                  (_RPA_IF_RPADONEIF_DEFAULT << 0)      /**< Shifted mode DEFAULT for RPA_IF             */
#define RPA_IF_BUSERRORIF                         (0x1UL << 1)                          /**< Bus error IF                                */
#define _RPA_IF_BUSERRORIF_SHIFT                  1                                     /**< Shift value for RPA_BUSERRORIF              */
#define _RPA_IF_BUSERRORIF_MASK                   0x2UL                                 /**< Bit mask for RPA_BUSERRORIF                 */
#define _RPA_IF_BUSERRORIF_DEFAULT                0x00000000UL                          /**< Mode DEFAULT for RPA_IF                     */
#define RPA_IF_BUSERRORIF_DEFAULT                 (_RPA_IF_BUSERRORIF_DEFAULT << 1)     /**< Shifted mode DEFAULT for RPA_IF             */
#define RPA_IF_KEYREQERRORIF                      (0x1UL << 2)                          /**< Key request error IF                        */
#define _RPA_IF_KEYREQERRORIF_SHIFT               2                                     /**< Shift value for RPA_KEYREQERRORIF           */
#define _RPA_IF_KEYREQERRORIF_MASK                0x4UL                                 /**< Bit mask for RPA_KEYREQERRORIF              */
#define _RPA_IF_KEYREQERRORIF_DEFAULT             0x00000000UL                          /**< Mode DEFAULT for RPA_IF                     */
#define RPA_IF_KEYREQERRORIF_DEFAULT              (_RPA_IF_KEYREQERRORIF_DEFAULT << 2)  /**< Shifted mode DEFAULT for RPA_IF             */

/* Bit fields for RPA IEN */
#define _RPA_IEN_RESETVALUE                       0x00000000UL                           /**< Default value for RPA_IEN                   */
#define _RPA_IEN_MASK                             0x00000007UL                           /**< Mask for RPA_IEN                            */
#define RPA_IEN_RPADONEIEN                        (0x1UL << 0)                           /**< Enable RPA done IF                          */
#define _RPA_IEN_RPADONEIEN_SHIFT                 0                                      /**< Shift value for RPA_RPADONEIEN              */
#define _RPA_IEN_RPADONEIEN_MASK                  0x1UL                                  /**< Bit mask for RPA_RPADONEIEN                 */
#define _RPA_IEN_RPADONEIEN_DEFAULT               0x00000000UL                           /**< Mode DEFAULT for RPA_IEN                    */
#define RPA_IEN_RPADONEIEN_DEFAULT                (_RPA_IEN_RPADONEIEN_DEFAULT << 0)     /**< Shifted mode DEFAULT for RPA_IEN            */
#define RPA_IEN_BUSERRORIEN                       (0x1UL << 1)                           /**< Enable bus error IF                         */
#define _RPA_IEN_BUSERRORIEN_SHIFT                1                                      /**< Shift value for RPA_BUSERRORIEN             */
#define _RPA_IEN_BUSERRORIEN_MASK                 0x2UL                                  /**< Bit mask for RPA_BUSERRORIEN                */
#define _RPA_IEN_BUSERRORIEN_DEFAULT              0x00000000UL                           /**< Mode DEFAULT for RPA_IEN                    */
#define RPA_IEN_BUSERRORIEN_DEFAULT               (_RPA_IEN_BUSERRORIEN_DEFAULT << 1)    /**< Shifted mode DEFAULT for RPA_IEN            */
#define RPA_IEN_KEYREQERRORIEN                    (0x1UL << 2)                           /**< Enable key request error IF                 */
#define _RPA_IEN_KEYREQERRORIEN_SHIFT             2                                      /**< Shift value for RPA_KEYREQERRORIEN          */
#define _RPA_IEN_KEYREQERRORIEN_MASK              0x4UL                                  /**< Bit mask for RPA_KEYREQERRORIEN             */
#define _RPA_IEN_KEYREQERRORIEN_DEFAULT           0x00000000UL                           /**< Mode DEFAULT for RPA_IEN                    */
#define RPA_IEN_KEYREQERRORIEN_DEFAULT            (_RPA_IEN_KEYREQERRORIEN_DEFAULT << 2) /**< Shifted mode DEFAULT for RPA_IEN            */

/* Bit fields for RPA STATUS */
#define _RPA_STATUS_RESETVALUE                    0x00000000UL                            /**< Default value for RPA_STATUS                */
#define _RPA_STATUS_MASK                          0xE00000FFUL                            /**< Mask for RPA_STATUS                         */
#define _RPA_STATUS_IRKINDEX_SHIFT                0                                       /**< Shift value for RPA_IRKINDEX                */
#define _RPA_STATUS_IRKINDEX_MASK                 0xFFUL                                  /**< Bit mask for RPA_IRKINDEX                   */
#define _RPA_STATUS_IRKINDEX_DEFAULT              0x00000000UL                            /**< Mode DEFAULT for RPA_STATUS                 */
#define RPA_STATUS_IRKINDEX_DEFAULT               (_RPA_STATUS_IRKINDEX_DEFAULT << 0)     /**< Shifted mode DEFAULT for RPA_STATUS         */
#define RPA_STATUS_AESRDY                         (0x1UL << 29)                           /**< AES ready                                   */
#define _RPA_STATUS_AESRDY_SHIFT                  29                                      /**< Shift value for RPA_AESRDY                  */
#define _RPA_STATUS_AESRDY_MASK                   0x20000000UL                            /**< Bit mask for RPA_AESRDY                     */
#define _RPA_STATUS_AESRDY_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for RPA_STATUS                 */
#define RPA_STATUS_AESRDY_DEFAULT                 (_RPA_STATUS_AESRDY_DEFAULT << 29)      /**< Shifted mode DEFAULT for RPA_STATUS         */
#define RPA_STATUS_IRKNOTFOUND                    (0x1UL << 30)                           /**< IRK not found                               */
#define _RPA_STATUS_IRKNOTFOUND_SHIFT             30                                      /**< Shift value for RPA_IRKNOTFOUND             */
#define _RPA_STATUS_IRKNOTFOUND_MASK              0x40000000UL                            /**< Bit mask for RPA_IRKNOTFOUND                */
#define _RPA_STATUS_IRKNOTFOUND_DEFAULT           0x00000000UL                            /**< Mode DEFAULT for RPA_STATUS                 */
#define RPA_STATUS_IRKNOTFOUND_DEFAULT            (_RPA_STATUS_IRKNOTFOUND_DEFAULT << 30) /**< Shifted mode DEFAULT for RPA_STATUS         */
#define RPA_STATUS_BUSY                           (0x1UL << 31)                           /**< RPA busy                                    */
#define _RPA_STATUS_BUSY_SHIFT                    31                                      /**< Shift value for RPA_BUSY                    */
#define _RPA_STATUS_BUSY_MASK                     0x80000000UL                            /**< Bit mask for RPA_BUSY                       */
#define _RPA_STATUS_BUSY_DEFAULT                  0x00000000UL                            /**< Mode DEFAULT for RPA_STATUS                 */
#define RPA_STATUS_BUSY_DEFAULT                   (_RPA_STATUS_BUSY_DEFAULT << 31)        /**< Shifted mode DEFAULT for RPA_STATUS         */

/* Bit fields for RPA IRKMASK */
#define _RPA_IRKMASK_RESETVALUE                   0xFFFFFFFFUL                          /**< Default value for RPA_IRKMASK               */
#define _RPA_IRKMASK_MASK                         0xFFFFFFFFUL                          /**< Mask for RPA_IRKMASK                        */
#define _RPA_IRKMASK_IRKMASK_SHIFT                0                                     /**< Shift value for RPA_IRKMASK                 */
#define _RPA_IRKMASK_IRKMASK_MASK                 0xFFFFFFFFUL                          /**< Bit mask for RPA_IRKMASK                    */
#define _RPA_IRKMASK_IRKMASK_DEFAULT              0xFFFFFFFFUL                          /**< Mode DEFAULT for RPA_IRKMASK                */
#define RPA_IRKMASK_IRKMASK_DEFAULT               (_RPA_IRKMASK_IRKMASK_DEFAULT << 0)   /**< Shifted mode DEFAULT for RPA_IRKMASK        */

/** @} End of group SIBG301_RPA_BitFields */
/** @} End of group SIBG301_RPA */
/** @} End of group Parts */

#endif // SIBG301_RPA_H
