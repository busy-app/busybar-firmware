/**************************************************************************//**
 * @file
 * @brief SIMG301 I2C register and bit field definitions
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
#ifndef SIMG301_I2C_H
#define SIMG301_I2C_H
#define I2C_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_I2C I2C
 * @{
 * @brief SIMG301 I2C Register Declaration.
 *****************************************************************************/

/** I2C Register Declaration. */
typedef struct i2c_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP VERSION Register                                */
  __IOM uint32_t EN;                            /**< Enable Register                                    */
  __IOM uint32_t SWRST;                         /**< SWRST Register                                     */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  __IOM uint32_t CMD;                           /**< Command Register                                   */
  __IM uint32_t  STATE;                         /**< State Register                                     */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  __IOM uint32_t CLKDIV;                        /**< Clock Division Register                            */
  __IOM uint32_t SADDR;                         /**< Follower Address Register                          */
  __IOM uint32_t SADDRMASK;                     /**< Follower Address Mask Register                     */
  __IM uint32_t  RXDATA;                        /**< Receive FIFO Data Register                         */
  __IM uint32_t  RXDATAP;                       /**< Receive FIFO Data Peek Register                    */
  uint32_t       RESERVED0[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t TXDATA;                        /**< Transmit FIFO Data Register                        */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t IF;                            /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN;                           /**< Interrupt Enable Register                          */
  __IM uint32_t  SYNCBUSY;                      /**< Syncbusy Status                                    */
  uint32_t       RESERVED2[1006U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_SET;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_SET;                     /**< SWRST Register                                     */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  __IOM uint32_t CMD_SET;                       /**< Command Register                                   */
  __IM uint32_t  STATE_SET;                     /**< State Register                                     */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  __IOM uint32_t CLKDIV_SET;                    /**< Clock Division Register                            */
  __IOM uint32_t SADDR_SET;                     /**< Follower Address Register                          */
  __IOM uint32_t SADDRMASK_SET;                 /**< Follower Address Mask Register                     */
  __IM uint32_t  RXDATA_SET;                    /**< Receive FIFO Data Register                         */
  __IM uint32_t  RXDATAP_SET;                   /**< Receive FIFO Data Peek Register                    */
  uint32_t       RESERVED3[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t TXDATA_SET;                    /**< Transmit FIFO Data Register                        */
  uint32_t       RESERVED4[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable Register                          */
  __IM uint32_t  SYNCBUSY_SET;                  /**< Syncbusy Status                                    */
  uint32_t       RESERVED5[1006U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_CLR;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_CLR;                     /**< SWRST Register                                     */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  __IOM uint32_t CMD_CLR;                       /**< Command Register                                   */
  __IM uint32_t  STATE_CLR;                     /**< State Register                                     */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  __IOM uint32_t CLKDIV_CLR;                    /**< Clock Division Register                            */
  __IOM uint32_t SADDR_CLR;                     /**< Follower Address Register                          */
  __IOM uint32_t SADDRMASK_CLR;                 /**< Follower Address Mask Register                     */
  __IM uint32_t  RXDATA_CLR;                    /**< Receive FIFO Data Register                         */
  __IM uint32_t  RXDATAP_CLR;                   /**< Receive FIFO Data Peek Register                    */
  uint32_t       RESERVED6[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t TXDATA_CLR;                    /**< Transmit FIFO Data Register                        */
  uint32_t       RESERVED7[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable Register                          */
  __IM uint32_t  SYNCBUSY_CLR;                  /**< Syncbusy Status                                    */
  uint32_t       RESERVED8[1006U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP VERSION Register                                */
  __IOM uint32_t EN_TGL;                        /**< Enable Register                                    */
  __IOM uint32_t SWRST_TGL;                     /**< SWRST Register                                     */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  __IOM uint32_t CMD_TGL;                       /**< Command Register                                   */
  __IM uint32_t  STATE_TGL;                     /**< State Register                                     */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  __IOM uint32_t CLKDIV_TGL;                    /**< Clock Division Register                            */
  __IOM uint32_t SADDR_TGL;                     /**< Follower Address Register                          */
  __IOM uint32_t SADDRMASK_TGL;                 /**< Follower Address Mask Register                     */
  __IM uint32_t  RXDATA_TGL;                    /**< Receive FIFO Data Register                         */
  __IM uint32_t  RXDATAP_TGL;                   /**< Receive FIFO Data Peek Register                    */
  uint32_t       RESERVED9[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t TXDATA_TGL;                    /**< Transmit FIFO Data Register                        */
  uint32_t       RESERVED10[1U];                /**< Reserved for future use                            */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag Register                            */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable Register                          */
  __IM uint32_t  SYNCBUSY_TGL;                  /**< Syncbusy Status                                    */
} I2C_TypeDef;
/** @} End of group SIMG301_I2C */

/**************************************************************************//**
 * @addtogroup SIMG301_I2C
 * @{
 * @defgroup SIMG301_I2C_BitFields I2C Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for I2C IPVERSION */
#define _I2C_IPVERSION_RESETVALUE           0x00000001UL                                /**< Default value for I2C_IPVERSION             */
#define _I2C_IPVERSION_MASK                 0xFFFFFFFFUL                                /**< Mask for I2C_IPVERSION                      */
#define _I2C_IPVERSION_IPVERSION_SHIFT      0                                           /**< Shift value for I2C_IPVERSION               */
#define _I2C_IPVERSION_IPVERSION_MASK       0xFFFFFFFFUL                                /**< Bit mask for I2C_IPVERSION                  */
#define _I2C_IPVERSION_IPVERSION_DEFAULT    0x00000001UL                                /**< Mode DEFAULT for I2C_IPVERSION              */
#define I2C_IPVERSION_IPVERSION_DEFAULT     (_I2C_IPVERSION_IPVERSION_DEFAULT << 0)     /**< Shifted mode DEFAULT for I2C_IPVERSION      */

/* Bit fields for I2C EN */
#define _I2C_EN_RESETVALUE                  0x00000000UL                                /**< Default value for I2C_EN                    */
#define _I2C_EN_MASK                        0x00000003UL                                /**< Mask for I2C_EN                             */
#define I2C_EN_EN                           (0x1UL << 0)                                /**< module enable                               */
#define _I2C_EN_EN_SHIFT                    0                                           /**< Shift value for I2C_EN                      */
#define _I2C_EN_EN_MASK                     0x1UL                                       /**< Bit mask for I2C_EN                         */
#define _I2C_EN_EN_DEFAULT                  0x00000000UL                                /**< Mode DEFAULT for I2C_EN                     */
#define I2C_EN_EN_DEFAULT                   (_I2C_EN_EN_DEFAULT << 0)                   /**< Shifted mode DEFAULT for I2C_EN             */
#define I2C_EN_DISABLING                    (0x1UL << 1)                                /**< Disablement busy status                     */
#define _I2C_EN_DISABLING_SHIFT             1                                           /**< Shift value for I2C_DISABLING               */
#define _I2C_EN_DISABLING_MASK              0x2UL                                       /**< Bit mask for I2C_DISABLING                  */
#define _I2C_EN_DISABLING_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_EN                     */
#define I2C_EN_DISABLING_DEFAULT            (_I2C_EN_DISABLING_DEFAULT << 1)            /**< Shifted mode DEFAULT for I2C_EN             */

/* Bit fields for I2C SWRST */
#define _I2C_SWRST_RESETVALUE               0x00000000UL                                /**< Default value for I2C_SWRST                 */
#define _I2C_SWRST_MASK                     0x00000003UL                                /**< Mask for I2C_SWRST                          */
#define I2C_SWRST_SWRST                     (0x1UL << 0)                                /**< Software Reset bit                          */
#define _I2C_SWRST_SWRST_SHIFT              0                                           /**< Shift value for I2C_SWRST                   */
#define _I2C_SWRST_SWRST_MASK               0x1UL                                       /**< Bit mask for I2C_SWRST                      */
#define _I2C_SWRST_SWRST_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_SWRST                  */
#define I2C_SWRST_SWRST_DEFAULT             (_I2C_SWRST_SWRST_DEFAULT << 0)             /**< Shifted mode DEFAULT for I2C_SWRST          */
#define I2C_SWRST_RESETTING                 (0x1UL << 1)                                /**< Reset busy status                           */
#define _I2C_SWRST_RESETTING_SHIFT          1                                           /**< Shift value for I2C_RESETTING               */
#define _I2C_SWRST_RESETTING_MASK           0x2UL                                       /**< Bit mask for I2C_RESETTING                  */
#define _I2C_SWRST_RESETTING_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_SWRST                  */
#define I2C_SWRST_RESETTING_DEFAULT         (_I2C_SWRST_RESETTING_DEFAULT << 1)         /**< Shifted mode DEFAULT for I2C_SWRST          */

/* Bit fields for I2C CTRL */
#define _I2C_CTRL_RESETVALUE                0x00000000UL                                /**< Default value for I2C_CTRL                  */
#define _I2C_CTRL_MASK                      0x3FF7B37FUL                                /**< Mask for I2C_CTRL                           */
#define I2C_CTRL_CORERST                    (0x1UL << 0)                                /**< Soft Reset the internal state registers     */
#define _I2C_CTRL_CORERST_SHIFT             0                                           /**< Shift value for I2C_CORERST                 */
#define _I2C_CTRL_CORERST_MASK              0x1UL                                       /**< Bit mask for I2C_CORERST                    */
#define _I2C_CTRL_CORERST_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_CORERST_DISABLE           0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_CORERST_ENABLE            0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_CORERST_DEFAULT            (_I2C_CTRL_CORERST_DEFAULT << 0)            /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_CORERST_DISABLE            (_I2C_CTRL_CORERST_DISABLE << 0)            /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_CORERST_ENABLE             (_I2C_CTRL_CORERST_ENABLE << 0)             /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_SLAVE                      (0x1UL << 1)                                /**< Addressable as Follower                     */
#define _I2C_CTRL_SLAVE_SHIFT               1                                           /**< Shift value for I2C_SLAVE                   */
#define _I2C_CTRL_SLAVE_MASK                0x2UL                                       /**< Bit mask for I2C_SLAVE                      */
#define _I2C_CTRL_SLAVE_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_SLAVE_DISABLE             0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_SLAVE_ENABLE              0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_SLAVE_DEFAULT              (_I2C_CTRL_SLAVE_DEFAULT << 1)              /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_SLAVE_DISABLE              (_I2C_CTRL_SLAVE_DISABLE << 1)              /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_SLAVE_ENABLE               (_I2C_CTRL_SLAVE_ENABLE << 1)               /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_AUTOACK                    (0x1UL << 2)                                /**< Automatic Acknowledge                       */
#define _I2C_CTRL_AUTOACK_SHIFT             2                                           /**< Shift value for I2C_AUTOACK                 */
#define _I2C_CTRL_AUTOACK_MASK              0x4UL                                       /**< Bit mask for I2C_AUTOACK                    */
#define _I2C_CTRL_AUTOACK_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_AUTOACK_DISABLE           0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_AUTOACK_ENABLE            0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_AUTOACK_DEFAULT            (_I2C_CTRL_AUTOACK_DEFAULT << 2)            /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_AUTOACK_DISABLE            (_I2C_CTRL_AUTOACK_DISABLE << 2)            /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_AUTOACK_ENABLE             (_I2C_CTRL_AUTOACK_ENABLE << 2)             /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_AUTOSE                     (0x1UL << 3)                                /**< Automatic STOP when Empty                   */
#define _I2C_CTRL_AUTOSE_SHIFT              3                                           /**< Shift value for I2C_AUTOSE                  */
#define _I2C_CTRL_AUTOSE_MASK               0x8UL                                       /**< Bit mask for I2C_AUTOSE                     */
#define _I2C_CTRL_AUTOSE_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_AUTOSE_DISABLE            0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_AUTOSE_ENABLE             0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_AUTOSE_DEFAULT             (_I2C_CTRL_AUTOSE_DEFAULT << 3)             /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_AUTOSE_DISABLE             (_I2C_CTRL_AUTOSE_DISABLE << 3)             /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_AUTOSE_ENABLE              (_I2C_CTRL_AUTOSE_ENABLE << 3)              /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_AUTOSN                     (0x1UL << 4)                                /**< Automatic STOP on NACK                      */
#define _I2C_CTRL_AUTOSN_SHIFT              4                                           /**< Shift value for I2C_AUTOSN                  */
#define _I2C_CTRL_AUTOSN_MASK               0x10UL                                      /**< Bit mask for I2C_AUTOSN                     */
#define _I2C_CTRL_AUTOSN_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_AUTOSN_DISABLE            0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_AUTOSN_ENABLE             0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_AUTOSN_DEFAULT             (_I2C_CTRL_AUTOSN_DEFAULT << 4)             /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_AUTOSN_DISABLE             (_I2C_CTRL_AUTOSN_DISABLE << 4)             /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_AUTOSN_ENABLE              (_I2C_CTRL_AUTOSN_ENABLE << 4)              /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_ARBDIS                     (0x1UL << 5)                                /**< Arbitration Disable                         */
#define _I2C_CTRL_ARBDIS_SHIFT              5                                           /**< Shift value for I2C_ARBDIS                  */
#define _I2C_CTRL_ARBDIS_MASK               0x20UL                                      /**< Bit mask for I2C_ARBDIS                     */
#define _I2C_CTRL_ARBDIS_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_ARBDIS_DISABLE            0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_ARBDIS_ENABLE             0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_ARBDIS_DEFAULT             (_I2C_CTRL_ARBDIS_DEFAULT << 5)             /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_ARBDIS_DISABLE             (_I2C_CTRL_ARBDIS_DISABLE << 5)             /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_ARBDIS_ENABLE              (_I2C_CTRL_ARBDIS_ENABLE << 5)              /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_GCAMEN                     (0x1UL << 6)                                /**< General Call Address Match Enable           */
#define _I2C_CTRL_GCAMEN_SHIFT              6                                           /**< Shift value for I2C_GCAMEN                  */
#define _I2C_CTRL_GCAMEN_MASK               0x40UL                                      /**< Bit mask for I2C_GCAMEN                     */
#define _I2C_CTRL_GCAMEN_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_GCAMEN_DISABLE            0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_GCAMEN_ENABLE             0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_GCAMEN_DEFAULT             (_I2C_CTRL_GCAMEN_DEFAULT << 6)             /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_GCAMEN_DISABLE             (_I2C_CTRL_GCAMEN_DISABLE << 6)             /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_GCAMEN_ENABLE              (_I2C_CTRL_GCAMEN_ENABLE << 6)              /**< Shifted mode ENABLE for I2C_CTRL            */
#define _I2C_CTRL_CLHR_SHIFT                8                                           /**< Shift value for I2C_CLHR                    */
#define _I2C_CTRL_CLHR_MASK                 0x300UL                                     /**< Bit mask for I2C_CLHR                       */
#define _I2C_CTRL_CLHR_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_CLHR_STANDARD             0x00000000UL                                /**< Mode STANDARD for I2C_CTRL                  */
#define _I2C_CTRL_CLHR_ASYMMETRIC           0x00000001UL                                /**< Mode ASYMMETRIC for I2C_CTRL                */
#define _I2C_CTRL_CLHR_FAST                 0x00000002UL                                /**< Mode FAST for I2C_CTRL                      */
#define I2C_CTRL_CLHR_DEFAULT               (_I2C_CTRL_CLHR_DEFAULT << 8)               /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_CLHR_STANDARD              (_I2C_CTRL_CLHR_STANDARD << 8)              /**< Shifted mode STANDARD for I2C_CTRL          */
#define I2C_CTRL_CLHR_ASYMMETRIC            (_I2C_CTRL_CLHR_ASYMMETRIC << 8)            /**< Shifted mode ASYMMETRIC for I2C_CTRL        */
#define I2C_CTRL_CLHR_FAST                  (_I2C_CTRL_CLHR_FAST << 8)                  /**< Shifted mode FAST for I2C_CTRL              */
#define _I2C_CTRL_BITO_SHIFT                12                                          /**< Shift value for I2C_BITO                    */
#define _I2C_CTRL_BITO_MASK                 0x3000UL                                    /**< Bit mask for I2C_BITO                       */
#define _I2C_CTRL_BITO_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_BITO_OFF                  0x00000000UL                                /**< Mode OFF for I2C_CTRL                       */
#define _I2C_CTRL_BITO_I2C40PCC             0x00000001UL                                /**< Mode I2C40PCC for I2C_CTRL                  */
#define _I2C_CTRL_BITO_I2C80PCC             0x00000002UL                                /**< Mode I2C80PCC for I2C_CTRL                  */
#define _I2C_CTRL_BITO_I2C160PCC            0x00000003UL                                /**< Mode I2C160PCC for I2C_CTRL                 */
#define I2C_CTRL_BITO_DEFAULT               (_I2C_CTRL_BITO_DEFAULT << 12)              /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_BITO_OFF                   (_I2C_CTRL_BITO_OFF << 12)                  /**< Shifted mode OFF for I2C_CTRL               */
#define I2C_CTRL_BITO_I2C40PCC              (_I2C_CTRL_BITO_I2C40PCC << 12)             /**< Shifted mode I2C40PCC for I2C_CTRL          */
#define I2C_CTRL_BITO_I2C80PCC              (_I2C_CTRL_BITO_I2C80PCC << 12)             /**< Shifted mode I2C80PCC for I2C_CTRL          */
#define I2C_CTRL_BITO_I2C160PCC             (_I2C_CTRL_BITO_I2C160PCC << 12)            /**< Shifted mode I2C160PCC for I2C_CTRL         */
#define I2C_CTRL_GIBITO                     (0x1UL << 15)                               /**< Go Idle on Bus Idle Timeout                 */
#define _I2C_CTRL_GIBITO_SHIFT              15                                          /**< Shift value for I2C_GIBITO                  */
#define _I2C_CTRL_GIBITO_MASK               0x8000UL                                    /**< Bit mask for I2C_GIBITO                     */
#define _I2C_CTRL_GIBITO_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_GIBITO_DISABLE            0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_GIBITO_ENABLE             0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_GIBITO_DEFAULT             (_I2C_CTRL_GIBITO_DEFAULT << 15)            /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_GIBITO_DISABLE             (_I2C_CTRL_GIBITO_DISABLE << 15)            /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_GIBITO_ENABLE              (_I2C_CTRL_GIBITO_ENABLE << 15)             /**< Shifted mode ENABLE for I2C_CTRL            */
#define _I2C_CTRL_CLTO_SHIFT                16                                          /**< Shift value for I2C_CLTO                    */
#define _I2C_CTRL_CLTO_MASK                 0x70000UL                                   /**< Bit mask for I2C_CLTO                       */
#define _I2C_CTRL_CLTO_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_CLTO_OFF                  0x00000000UL                                /**< Mode OFF for I2C_CTRL                       */
#define _I2C_CTRL_CLTO_I2C40PCC             0x00000001UL                                /**< Mode I2C40PCC for I2C_CTRL                  */
#define _I2C_CTRL_CLTO_I2C80PCC             0x00000002UL                                /**< Mode I2C80PCC for I2C_CTRL                  */
#define _I2C_CTRL_CLTO_I2C160PCC            0x00000003UL                                /**< Mode I2C160PCC for I2C_CTRL                 */
#define _I2C_CTRL_CLTO_I2C320PCC            0x00000004UL                                /**< Mode I2C320PCC for I2C_CTRL                 */
#define _I2C_CTRL_CLTO_I2C1024PCC           0x00000005UL                                /**< Mode I2C1024PCC for I2C_CTRL                */
#define I2C_CTRL_CLTO_DEFAULT               (_I2C_CTRL_CLTO_DEFAULT << 16)              /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_CLTO_OFF                   (_I2C_CTRL_CLTO_OFF << 16)                  /**< Shifted mode OFF for I2C_CTRL               */
#define I2C_CTRL_CLTO_I2C40PCC              (_I2C_CTRL_CLTO_I2C40PCC << 16)             /**< Shifted mode I2C40PCC for I2C_CTRL          */
#define I2C_CTRL_CLTO_I2C80PCC              (_I2C_CTRL_CLTO_I2C80PCC << 16)             /**< Shifted mode I2C80PCC for I2C_CTRL          */
#define I2C_CTRL_CLTO_I2C160PCC             (_I2C_CTRL_CLTO_I2C160PCC << 16)            /**< Shifted mode I2C160PCC for I2C_CTRL         */
#define I2C_CTRL_CLTO_I2C320PCC             (_I2C_CTRL_CLTO_I2C320PCC << 16)            /**< Shifted mode I2C320PCC for I2C_CTRL         */
#define I2C_CTRL_CLTO_I2C1024PCC            (_I2C_CTRL_CLTO_I2C1024PCC << 16)           /**< Shifted mode I2C1024PCC for I2C_CTRL        */
#define I2C_CTRL_SCLMONEN                   (0x1UL << 20)                               /**< SCL Monitor Enable                          */
#define _I2C_CTRL_SCLMONEN_SHIFT            20                                          /**< Shift value for I2C_SCLMONEN                */
#define _I2C_CTRL_SCLMONEN_MASK             0x100000UL                                  /**< Bit mask for I2C_SCLMONEN                   */
#define _I2C_CTRL_SCLMONEN_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_SCLMONEN_DISABLE          0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_SCLMONEN_ENABLE           0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_SCLMONEN_DEFAULT           (_I2C_CTRL_SCLMONEN_DEFAULT << 20)          /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_SCLMONEN_DISABLE           (_I2C_CTRL_SCLMONEN_DISABLE << 20)          /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_SCLMONEN_ENABLE            (_I2C_CTRL_SCLMONEN_ENABLE << 20)           /**< Shifted mode ENABLE for I2C_CTRL            */
#define I2C_CTRL_SDAMONEN                   (0x1UL << 21)                               /**< SDA Monitor Enable                          */
#define _I2C_CTRL_SDAMONEN_SHIFT            21                                          /**< Shift value for I2C_SDAMONEN                */
#define _I2C_CTRL_SDAMONEN_MASK             0x200000UL                                  /**< Bit mask for I2C_SDAMONEN                   */
#define _I2C_CTRL_SDAMONEN_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define _I2C_CTRL_SDAMONEN_DISABLE          0x00000000UL                                /**< Mode DISABLE for I2C_CTRL                   */
#define _I2C_CTRL_SDAMONEN_ENABLE           0x00000001UL                                /**< Mode ENABLE for I2C_CTRL                    */
#define I2C_CTRL_SDAMONEN_DEFAULT           (_I2C_CTRL_SDAMONEN_DEFAULT << 21)          /**< Shifted mode DEFAULT for I2C_CTRL           */
#define I2C_CTRL_SDAMONEN_DISABLE           (_I2C_CTRL_SDAMONEN_DISABLE << 21)          /**< Shifted mode DISABLE for I2C_CTRL           */
#define I2C_CTRL_SDAMONEN_ENABLE            (_I2C_CTRL_SDAMONEN_ENABLE << 21)           /**< Shifted mode ENABLE for I2C_CTRL            */
#define _I2C_CTRL_TXFIL_SHIFT               22                                          /**< Shift value for I2C_TXFIL                   */
#define _I2C_CTRL_TXFIL_MASK                0x3C00000UL                                 /**< Bit mask for I2C_TXFIL                      */
#define _I2C_CTRL_TXFIL_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define I2C_CTRL_TXFIL_DEFAULT              (_I2C_CTRL_TXFIL_DEFAULT << 22)             /**< Shifted mode DEFAULT for I2C_CTRL           */
#define _I2C_CTRL_RXFIL_SHIFT               26                                          /**< Shift value for I2C_RXFIL                   */
#define _I2C_CTRL_RXFIL_MASK                0x3C000000UL                                /**< Bit mask for I2C_RXFIL                      */
#define _I2C_CTRL_RXFIL_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_CTRL                   */
#define I2C_CTRL_RXFIL_DEFAULT              (_I2C_CTRL_RXFIL_DEFAULT << 26)             /**< Shifted mode DEFAULT for I2C_CTRL           */

/* Bit fields for I2C CMD */
#define _I2C_CMD_RESETVALUE                 0x00000000UL                                /**< Default value for I2C_CMD                   */
#define _I2C_CMD_MASK                       0x000001FFUL                                /**< Mask for I2C_CMD                            */
#define I2C_CMD_START                       (0x1UL << 0)                                /**< Send start condition                        */
#define _I2C_CMD_START_SHIFT                0                                           /**< Shift value for I2C_START                   */
#define _I2C_CMD_START_MASK                 0x1UL                                       /**< Bit mask for I2C_START                      */
#define _I2C_CMD_START_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_START_DEFAULT               (_I2C_CMD_START_DEFAULT << 0)               /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_STOP                        (0x1UL << 1)                                /**< Send stop condition                         */
#define _I2C_CMD_STOP_SHIFT                 1                                           /**< Shift value for I2C_STOP                    */
#define _I2C_CMD_STOP_MASK                  0x2UL                                       /**< Bit mask for I2C_STOP                       */
#define _I2C_CMD_STOP_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_STOP_DEFAULT                (_I2C_CMD_STOP_DEFAULT << 1)                /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_ACK                         (0x1UL << 2)                                /**< Send ACK                                    */
#define _I2C_CMD_ACK_SHIFT                  2                                           /**< Shift value for I2C_ACK                     */
#define _I2C_CMD_ACK_MASK                   0x4UL                                       /**< Bit mask for I2C_ACK                        */
#define _I2C_CMD_ACK_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_ACK_DEFAULT                 (_I2C_CMD_ACK_DEFAULT << 2)                 /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_NACK                        (0x1UL << 3)                                /**< Send NACK                                   */
#define _I2C_CMD_NACK_SHIFT                 3                                           /**< Shift value for I2C_NACK                    */
#define _I2C_CMD_NACK_MASK                  0x8UL                                       /**< Bit mask for I2C_NACK                       */
#define _I2C_CMD_NACK_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_NACK_DEFAULT                (_I2C_CMD_NACK_DEFAULT << 3)                /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_CONT                        (0x1UL << 4)                                /**< Continue transmission                       */
#define _I2C_CMD_CONT_SHIFT                 4                                           /**< Shift value for I2C_CONT                    */
#define _I2C_CMD_CONT_MASK                  0x10UL                                      /**< Bit mask for I2C_CONT                       */
#define _I2C_CMD_CONT_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_CONT_DEFAULT                (_I2C_CMD_CONT_DEFAULT << 4)                /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_ABORT                       (0x1UL << 5)                                /**< Abort transmission                          */
#define _I2C_CMD_ABORT_SHIFT                5                                           /**< Shift value for I2C_ABORT                   */
#define _I2C_CMD_ABORT_MASK                 0x20UL                                      /**< Bit mask for I2C_ABORT                      */
#define _I2C_CMD_ABORT_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_ABORT_DEFAULT               (_I2C_CMD_ABORT_DEFAULT << 5)               /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_CLEARTX                     (0x1UL << 6)                                /**< Clear TX                                    */
#define _I2C_CMD_CLEARTX_SHIFT              6                                           /**< Shift value for I2C_CLEARTX                 */
#define _I2C_CMD_CLEARTX_MASK               0x40UL                                      /**< Bit mask for I2C_CLEARTX                    */
#define _I2C_CMD_CLEARTX_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_CLEARTX_DEFAULT             (_I2C_CMD_CLEARTX_DEFAULT << 6)             /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_CLEARPC                     (0x1UL << 7)                                /**< Clear Pending Commands                      */
#define _I2C_CMD_CLEARPC_SHIFT              7                                           /**< Shift value for I2C_CLEARPC                 */
#define _I2C_CMD_CLEARPC_MASK               0x80UL                                      /**< Bit mask for I2C_CLEARPC                    */
#define _I2C_CMD_CLEARPC_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_CLEARPC_DEFAULT             (_I2C_CMD_CLEARPC_DEFAULT << 7)             /**< Shifted mode DEFAULT for I2C_CMD            */
#define I2C_CMD_CLEARRX                     (0x1UL << 8)                                /**< Clear RX FIFO                               */
#define _I2C_CMD_CLEARRX_SHIFT              8                                           /**< Shift value for I2C_CLEARRX                 */
#define _I2C_CMD_CLEARRX_MASK               0x100UL                                     /**< Bit mask for I2C_CLEARRX                    */
#define _I2C_CMD_CLEARRX_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_CMD                    */
#define I2C_CMD_CLEARRX_DEFAULT             (_I2C_CMD_CLEARRX_DEFAULT << 8)             /**< Shifted mode DEFAULT for I2C_CMD            */

/* Bit fields for I2C STATE */
#define _I2C_STATE_RESETVALUE               0x00000001UL                                /**< Default value for I2C_STATE                 */
#define _I2C_STATE_MASK                     0x000000FFUL                                /**< Mask for I2C_STATE                          */
#define I2C_STATE_BUSY                      (0x1UL << 0)                                /**< Bus Busy                                    */
#define _I2C_STATE_BUSY_SHIFT               0                                           /**< Shift value for I2C_BUSY                    */
#define _I2C_STATE_BUSY_MASK                0x1UL                                       /**< Bit mask for I2C_BUSY                       */
#define _I2C_STATE_BUSY_DEFAULT             0x00000001UL                                /**< Mode DEFAULT for I2C_STATE                  */
#define I2C_STATE_BUSY_DEFAULT              (_I2C_STATE_BUSY_DEFAULT << 0)              /**< Shifted mode DEFAULT for I2C_STATE          */
#define I2C_STATE_MASTER                    (0x1UL << 1)                                /**< Leader                                      */
#define _I2C_STATE_MASTER_SHIFT             1                                           /**< Shift value for I2C_MASTER                  */
#define _I2C_STATE_MASTER_MASK              0x2UL                                       /**< Bit mask for I2C_MASTER                     */
#define _I2C_STATE_MASTER_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_STATE                  */
#define I2C_STATE_MASTER_DEFAULT            (_I2C_STATE_MASTER_DEFAULT << 1)            /**< Shifted mode DEFAULT for I2C_STATE          */
#define I2C_STATE_TRANSMITTER               (0x1UL << 2)                                /**< Transmitter                                 */
#define _I2C_STATE_TRANSMITTER_SHIFT        2                                           /**< Shift value for I2C_TRANSMITTER             */
#define _I2C_STATE_TRANSMITTER_MASK         0x4UL                                       /**< Bit mask for I2C_TRANSMITTER                */
#define _I2C_STATE_TRANSMITTER_DEFAULT      0x00000000UL                                /**< Mode DEFAULT for I2C_STATE                  */
#define I2C_STATE_TRANSMITTER_DEFAULT       (_I2C_STATE_TRANSMITTER_DEFAULT << 2)       /**< Shifted mode DEFAULT for I2C_STATE          */
#define I2C_STATE_NACKED                    (0x1UL << 3)                                /**< Nack Received                               */
#define _I2C_STATE_NACKED_SHIFT             3                                           /**< Shift value for I2C_NACKED                  */
#define _I2C_STATE_NACKED_MASK              0x8UL                                       /**< Bit mask for I2C_NACKED                     */
#define _I2C_STATE_NACKED_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_STATE                  */
#define I2C_STATE_NACKED_DEFAULT            (_I2C_STATE_NACKED_DEFAULT << 3)            /**< Shifted mode DEFAULT for I2C_STATE          */
#define I2C_STATE_BUSHOLD                   (0x1UL << 4)                                /**< Bus Held                                    */
#define _I2C_STATE_BUSHOLD_SHIFT            4                                           /**< Shift value for I2C_BUSHOLD                 */
#define _I2C_STATE_BUSHOLD_MASK             0x10UL                                      /**< Bit mask for I2C_BUSHOLD                    */
#define _I2C_STATE_BUSHOLD_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_STATE                  */
#define I2C_STATE_BUSHOLD_DEFAULT           (_I2C_STATE_BUSHOLD_DEFAULT << 4)           /**< Shifted mode DEFAULT for I2C_STATE          */
#define _I2C_STATE_STATE_SHIFT              5                                           /**< Shift value for I2C_STATE                   */
#define _I2C_STATE_STATE_MASK               0xE0UL                                      /**< Bit mask for I2C_STATE                      */
#define _I2C_STATE_STATE_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_STATE                  */
#define _I2C_STATE_STATE_IDLE               0x00000000UL                                /**< Mode IDLE for I2C_STATE                     */
#define _I2C_STATE_STATE_WAIT               0x00000001UL                                /**< Mode WAIT for I2C_STATE                     */
#define _I2C_STATE_STATE_START              0x00000002UL                                /**< Mode START for I2C_STATE                    */
#define _I2C_STATE_STATE_ADDR               0x00000003UL                                /**< Mode ADDR for I2C_STATE                     */
#define _I2C_STATE_STATE_ADDRACK            0x00000004UL                                /**< Mode ADDRACK for I2C_STATE                  */
#define _I2C_STATE_STATE_DATA               0x00000005UL                                /**< Mode DATA for I2C_STATE                     */
#define _I2C_STATE_STATE_DATAACK            0x00000006UL                                /**< Mode DATAACK for I2C_STATE                  */
#define I2C_STATE_STATE_DEFAULT             (_I2C_STATE_STATE_DEFAULT << 5)             /**< Shifted mode DEFAULT for I2C_STATE          */
#define I2C_STATE_STATE_IDLE                (_I2C_STATE_STATE_IDLE << 5)                /**< Shifted mode IDLE for I2C_STATE             */
#define I2C_STATE_STATE_WAIT                (_I2C_STATE_STATE_WAIT << 5)                /**< Shifted mode WAIT for I2C_STATE             */
#define I2C_STATE_STATE_START               (_I2C_STATE_STATE_START << 5)               /**< Shifted mode START for I2C_STATE            */
#define I2C_STATE_STATE_ADDR                (_I2C_STATE_STATE_ADDR << 5)                /**< Shifted mode ADDR for I2C_STATE             */
#define I2C_STATE_STATE_ADDRACK             (_I2C_STATE_STATE_ADDRACK << 5)             /**< Shifted mode ADDRACK for I2C_STATE          */
#define I2C_STATE_STATE_DATA                (_I2C_STATE_STATE_DATA << 5)                /**< Shifted mode DATA for I2C_STATE             */
#define I2C_STATE_STATE_DATAACK             (_I2C_STATE_STATE_DATAACK << 5)             /**< Shifted mode DATAACK for I2C_STATE          */

/* Bit fields for I2C STATUS */
#define _I2C_STATUS_RESETVALUE              0x00000080UL                                /**< Default value for I2C_STATUS                */
#define _I2C_STATUS_MASK                    0x001FFFFFUL                                /**< Mask for I2C_STATUS                         */
#define I2C_STATUS_PSTART                   (0x1UL << 0)                                /**< Pending START                               */
#define _I2C_STATUS_PSTART_SHIFT            0                                           /**< Shift value for I2C_PSTART                  */
#define _I2C_STATUS_PSTART_MASK             0x1UL                                       /**< Bit mask for I2C_PSTART                     */
#define _I2C_STATUS_PSTART_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_PSTART_DEFAULT           (_I2C_STATUS_PSTART_DEFAULT << 0)           /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_PSTOP                    (0x1UL << 1)                                /**< Pending STOP                                */
#define _I2C_STATUS_PSTOP_SHIFT             1                                           /**< Shift value for I2C_PSTOP                   */
#define _I2C_STATUS_PSTOP_MASK              0x2UL                                       /**< Bit mask for I2C_PSTOP                      */
#define _I2C_STATUS_PSTOP_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_PSTOP_DEFAULT            (_I2C_STATUS_PSTOP_DEFAULT << 1)            /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_PACK                     (0x1UL << 2)                                /**< Pending ACK                                 */
#define _I2C_STATUS_PACK_SHIFT              2                                           /**< Shift value for I2C_PACK                    */
#define _I2C_STATUS_PACK_MASK               0x4UL                                       /**< Bit mask for I2C_PACK                       */
#define _I2C_STATUS_PACK_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_PACK_DEFAULT             (_I2C_STATUS_PACK_DEFAULT << 2)             /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_PNACK                    (0x1UL << 3)                                /**< Pending NACK                                */
#define _I2C_STATUS_PNACK_SHIFT             3                                           /**< Shift value for I2C_PNACK                   */
#define _I2C_STATUS_PNACK_MASK              0x8UL                                       /**< Bit mask for I2C_PNACK                      */
#define _I2C_STATUS_PNACK_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_PNACK_DEFAULT            (_I2C_STATUS_PNACK_DEFAULT << 3)            /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_PCONT                    (0x1UL << 4)                                /**< Pending continue                            */
#define _I2C_STATUS_PCONT_SHIFT             4                                           /**< Shift value for I2C_PCONT                   */
#define _I2C_STATUS_PCONT_MASK              0x10UL                                      /**< Bit mask for I2C_PCONT                      */
#define _I2C_STATUS_PCONT_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_PCONT_DEFAULT            (_I2C_STATUS_PCONT_DEFAULT << 4)            /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_PABORT                   (0x1UL << 5)                                /**< Pending abort                               */
#define _I2C_STATUS_PABORT_SHIFT            5                                           /**< Shift value for I2C_PABORT                  */
#define _I2C_STATUS_PABORT_MASK             0x20UL                                      /**< Bit mask for I2C_PABORT                     */
#define _I2C_STATUS_PABORT_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_PABORT_DEFAULT           (_I2C_STATUS_PABORT_DEFAULT << 5)           /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_TXC                      (0x1UL << 6)                                /**< TX Complete                                 */
#define _I2C_STATUS_TXC_SHIFT               6                                           /**< Shift value for I2C_TXC                     */
#define _I2C_STATUS_TXC_MASK                0x40UL                                      /**< Bit mask for I2C_TXC                        */
#define _I2C_STATUS_TXC_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_TXC_DEFAULT              (_I2C_STATUS_TXC_DEFAULT << 6)              /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_TXFL                     (0x1UL << 7)                                /**< TX FIFO Level                               */
#define _I2C_STATUS_TXFL_SHIFT              7                                           /**< Shift value for I2C_TXFL                    */
#define _I2C_STATUS_TXFL_MASK               0x80UL                                      /**< Bit mask for I2C_TXFL                       */
#define _I2C_STATUS_TXFL_DEFAULT            0x00000001UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_TXFL_DEFAULT             (_I2C_STATUS_TXFL_DEFAULT << 7)             /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_RXDATAV                  (0x1UL << 8)                                /**< RX Data Valid                               */
#define _I2C_STATUS_RXDATAV_SHIFT           8                                           /**< Shift value for I2C_RXDATAV                 */
#define _I2C_STATUS_RXDATAV_MASK            0x100UL                                     /**< Bit mask for I2C_RXDATAV                    */
#define _I2C_STATUS_RXDATAV_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_RXDATAV_DEFAULT          (_I2C_STATUS_RXDATAV_DEFAULT << 8)          /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_RXFULL                   (0x1UL << 9)                                /**< RX FIFO Full                                */
#define _I2C_STATUS_RXFULL_SHIFT            9                                           /**< Shift value for I2C_RXFULL                  */
#define _I2C_STATUS_RXFULL_MASK             0x200UL                                     /**< Bit mask for I2C_RXFULL                     */
#define _I2C_STATUS_RXFULL_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_RXFULL_DEFAULT           (_I2C_STATUS_RXFULL_DEFAULT << 9)           /**< Shifted mode DEFAULT for I2C_STATUS         */
#define _I2C_STATUS_TXFCNT_SHIFT            10                                          /**< Shift value for I2C_TXFCNT                  */
#define _I2C_STATUS_TXFCNT_MASK             0x3C00UL                                    /**< Bit mask for I2C_TXFCNT                     */
#define _I2C_STATUS_TXFCNT_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_TXFCNT_DEFAULT           (_I2C_STATUS_TXFCNT_DEFAULT << 10)          /**< Shifted mode DEFAULT for I2C_STATUS         */
#define _I2C_STATUS_RXFCNT_SHIFT            14                                          /**< Shift value for I2C_RXFCNT                  */
#define _I2C_STATUS_RXFCNT_MASK             0x3C000UL                                   /**< Bit mask for I2C_RXFCNT                     */
#define _I2C_STATUS_RXFCNT_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_RXFCNT_DEFAULT           (_I2C_STATUS_RXFCNT_DEFAULT << 14)          /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_RXFL                     (0x1UL << 18)                               /**< RX FIFO level                               */
#define _I2C_STATUS_RXFL_SHIFT              18                                          /**< Shift value for I2C_RXFL                    */
#define _I2C_STATUS_RXFL_MASK               0x40000UL                                   /**< Bit mask for I2C_RXFL                       */
#define _I2C_STATUS_RXFL_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_RXFL_DEFAULT             (_I2C_STATUS_RXFL_DEFAULT << 18)            /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_CLEARTXBUSY              (0x1UL << 19)                               /**< Clear TX FIFO busy                          */
#define _I2C_STATUS_CLEARTXBUSY_SHIFT       19                                          /**< Shift value for I2C_CLEARTXBUSY             */
#define _I2C_STATUS_CLEARTXBUSY_MASK        0x80000UL                                   /**< Bit mask for I2C_CLEARTXBUSY                */
#define _I2C_STATUS_CLEARTXBUSY_DEFAULT     0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_CLEARTXBUSY_DEFAULT      (_I2C_STATUS_CLEARTXBUSY_DEFAULT << 19)     /**< Shifted mode DEFAULT for I2C_STATUS         */
#define I2C_STATUS_CLEARRXBUSY              (0x1UL << 20)                               /**< Clear RX FIFO busy                          */
#define _I2C_STATUS_CLEARRXBUSY_SHIFT       20                                          /**< Shift value for I2C_CLEARRXBUSY             */
#define _I2C_STATUS_CLEARRXBUSY_MASK        0x100000UL                                  /**< Bit mask for I2C_CLEARRXBUSY                */
#define _I2C_STATUS_CLEARRXBUSY_DEFAULT     0x00000000UL                                /**< Mode DEFAULT for I2C_STATUS                 */
#define I2C_STATUS_CLEARRXBUSY_DEFAULT      (_I2C_STATUS_CLEARRXBUSY_DEFAULT << 20)     /**< Shifted mode DEFAULT for I2C_STATUS         */

/* Bit fields for I2C CLKDIV */
#define _I2C_CLKDIV_RESETVALUE              0x00000000UL                                /**< Default value for I2C_CLKDIV                */
#define _I2C_CLKDIV_MASK                    0x000001FFUL                                /**< Mask for I2C_CLKDIV                         */
#define _I2C_CLKDIV_DIV_SHIFT               0                                           /**< Shift value for I2C_DIV                     */
#define _I2C_CLKDIV_DIV_MASK                0x1FFUL                                     /**< Bit mask for I2C_DIV                        */
#define _I2C_CLKDIV_DIV_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_CLKDIV                 */
#define I2C_CLKDIV_DIV_DEFAULT              (_I2C_CLKDIV_DIV_DEFAULT << 0)              /**< Shifted mode DEFAULT for I2C_CLKDIV         */

/* Bit fields for I2C SADDR */
#define _I2C_SADDR_RESETVALUE               0x00000000UL                                /**< Default value for I2C_SADDR                 */
#define _I2C_SADDR_MASK                     0x000000FEUL                                /**< Mask for I2C_SADDR                          */
#define _I2C_SADDR_ADDR_SHIFT               1                                           /**< Shift value for I2C_ADDR                    */
#define _I2C_SADDR_ADDR_MASK                0xFEUL                                      /**< Bit mask for I2C_ADDR                       */
#define _I2C_SADDR_ADDR_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_SADDR                  */
#define I2C_SADDR_ADDR_DEFAULT              (_I2C_SADDR_ADDR_DEFAULT << 1)              /**< Shifted mode DEFAULT for I2C_SADDR          */

/* Bit fields for I2C SADDRMASK */
#define _I2C_SADDRMASK_RESETVALUE           0x00000000UL                                /**< Default value for I2C_SADDRMASK             */
#define _I2C_SADDRMASK_MASK                 0x000000FEUL                                /**< Mask for I2C_SADDRMASK                      */
#define _I2C_SADDRMASK_SADDRMASK_SHIFT      1                                           /**< Shift value for I2C_SADDRMASK               */
#define _I2C_SADDRMASK_SADDRMASK_MASK       0xFEUL                                      /**< Bit mask for I2C_SADDRMASK                  */
#define _I2C_SADDRMASK_SADDRMASK_DEFAULT    0x00000000UL                                /**< Mode DEFAULT for I2C_SADDRMASK              */
#define I2C_SADDRMASK_SADDRMASK_DEFAULT     (_I2C_SADDRMASK_SADDRMASK_DEFAULT << 1)     /**< Shifted mode DEFAULT for I2C_SADDRMASK      */

/* Bit fields for I2C RXDATA */
#define _I2C_RXDATA_RESETVALUE              0x00000000UL                                /**< Default value for I2C_RXDATA                */
#define _I2C_RXDATA_MASK                    0x000000FFUL                                /**< Mask for I2C_RXDATA                         */
#define _I2C_RXDATA_RXDATA_SHIFT            0                                           /**< Shift value for I2C_RXDATA                  */
#define _I2C_RXDATA_RXDATA_MASK             0xFFUL                                      /**< Bit mask for I2C_RXDATA                     */
#define _I2C_RXDATA_RXDATA_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_RXDATA                 */
#define I2C_RXDATA_RXDATA_DEFAULT           (_I2C_RXDATA_RXDATA_DEFAULT << 0)           /**< Shifted mode DEFAULT for I2C_RXDATA         */

/* Bit fields for I2C RXDATAP */
#define _I2C_RXDATAP_RESETVALUE             0x00000000UL                                /**< Default value for I2C_RXDATAP               */
#define _I2C_RXDATAP_MASK                   0x000000FFUL                                /**< Mask for I2C_RXDATAP                        */
#define _I2C_RXDATAP_RXDATAP_SHIFT          0                                           /**< Shift value for I2C_RXDATAP                 */
#define _I2C_RXDATAP_RXDATAP_MASK           0xFFUL                                      /**< Bit mask for I2C_RXDATAP                    */
#define _I2C_RXDATAP_RXDATAP_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_RXDATAP                */
#define I2C_RXDATAP_RXDATAP_DEFAULT         (_I2C_RXDATAP_RXDATAP_DEFAULT << 0)         /**< Shifted mode DEFAULT for I2C_RXDATAP        */

/* Bit fields for I2C TXDATA */
#define _I2C_TXDATA_RESETVALUE              0x00000000UL                                /**< Default value for I2C_TXDATA                */
#define _I2C_TXDATA_MASK                    0x000000FFUL                                /**< Mask for I2C_TXDATA                         */
#define _I2C_TXDATA_TXDATA_SHIFT            0                                           /**< Shift value for I2C_TXDATA                  */
#define _I2C_TXDATA_TXDATA_MASK             0xFFUL                                      /**< Bit mask for I2C_TXDATA                     */
#define _I2C_TXDATA_TXDATA_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_TXDATA                 */
#define I2C_TXDATA_TXDATA_DEFAULT           (_I2C_TXDATA_TXDATA_DEFAULT << 0)           /**< Shifted mode DEFAULT for I2C_TXDATA         */

/* Bit fields for I2C IF */
#define _I2C_IF_RESETVALUE                  0x00000000UL                                /**< Default value for I2C_IF                    */
#define _I2C_IF_MASK                        0x003FFFFFUL                                /**< Mask for I2C_IF                             */
#define I2C_IF_START                        (0x1UL << 0)                                /**< START condition Interrupt Flag              */
#define _I2C_IF_START_SHIFT                 0                                           /**< Shift value for I2C_START                   */
#define _I2C_IF_START_MASK                  0x1UL                                       /**< Bit mask for I2C_START                      */
#define _I2C_IF_START_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_START_DEFAULT                (_I2C_IF_START_DEFAULT << 0)                /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_RSTART                       (0x1UL << 1)                                /**< Repeated START condition Interrupt Flag     */
#define _I2C_IF_RSTART_SHIFT                1                                           /**< Shift value for I2C_RSTART                  */
#define _I2C_IF_RSTART_MASK                 0x2UL                                       /**< Bit mask for I2C_RSTART                     */
#define _I2C_IF_RSTART_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_RSTART_DEFAULT               (_I2C_IF_RSTART_DEFAULT << 1)               /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_ADDR                         (0x1UL << 2)                                /**< Address Interrupt Flag                      */
#define _I2C_IF_ADDR_SHIFT                  2                                           /**< Shift value for I2C_ADDR                    */
#define _I2C_IF_ADDR_MASK                   0x4UL                                       /**< Bit mask for I2C_ADDR                       */
#define _I2C_IF_ADDR_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_ADDR_DEFAULT                 (_I2C_IF_ADDR_DEFAULT << 2)                 /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_TXC                          (0x1UL << 3)                                /**< Transfer Completed Interrupt Flag           */
#define _I2C_IF_TXC_SHIFT                   3                                           /**< Shift value for I2C_TXC                     */
#define _I2C_IF_TXC_MASK                    0x8UL                                       /**< Bit mask for I2C_TXC                        */
#define _I2C_IF_TXC_DEFAULT                 0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_TXC_DEFAULT                  (_I2C_IF_TXC_DEFAULT << 3)                  /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_TXBL                         (0x1UL << 4)                                /**< Transmit FIFO Level Interrupt Flag          */
#define _I2C_IF_TXBL_SHIFT                  4                                           /**< Shift value for I2C_TXBL                    */
#define _I2C_IF_TXBL_MASK                   0x10UL                                      /**< Bit mask for I2C_TXBL                       */
#define _I2C_IF_TXBL_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_TXBL_DEFAULT                 (_I2C_IF_TXBL_DEFAULT << 4)                 /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_RXDATAV                      (0x1UL << 5)                                /**< Receive Data Valid Interrupt Flag           */
#define _I2C_IF_RXDATAV_SHIFT               5                                           /**< Shift value for I2C_RXDATAV                 */
#define _I2C_IF_RXDATAV_MASK                0x20UL                                      /**< Bit mask for I2C_RXDATAV                    */
#define _I2C_IF_RXDATAV_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_RXDATAV_DEFAULT              (_I2C_IF_RXDATAV_DEFAULT << 5)              /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_ACK                          (0x1UL << 6)                                /**< Acknowledge Received Interrupt Flag         */
#define _I2C_IF_ACK_SHIFT                   6                                           /**< Shift value for I2C_ACK                     */
#define _I2C_IF_ACK_MASK                    0x40UL                                      /**< Bit mask for I2C_ACK                        */
#define _I2C_IF_ACK_DEFAULT                 0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_ACK_DEFAULT                  (_I2C_IF_ACK_DEFAULT << 6)                  /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_NACK                         (0x1UL << 7)                                /**< Not Acknowledge Received Interrupt Flag     */
#define _I2C_IF_NACK_SHIFT                  7                                           /**< Shift value for I2C_NACK                    */
#define _I2C_IF_NACK_MASK                   0x80UL                                      /**< Bit mask for I2C_NACK                       */
#define _I2C_IF_NACK_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_NACK_DEFAULT                 (_I2C_IF_NACK_DEFAULT << 7)                 /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_MSTOP                        (0x1UL << 8)                                /**< Leader STOP Condition Interrupt Flag        */
#define _I2C_IF_MSTOP_SHIFT                 8                                           /**< Shift value for I2C_MSTOP                   */
#define _I2C_IF_MSTOP_MASK                  0x100UL                                     /**< Bit mask for I2C_MSTOP                      */
#define _I2C_IF_MSTOP_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_MSTOP_DEFAULT                (_I2C_IF_MSTOP_DEFAULT << 8)                /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_ARBLOST                      (0x1UL << 9)                                /**< Arbitration Lost Interrupt Flag             */
#define _I2C_IF_ARBLOST_SHIFT               9                                           /**< Shift value for I2C_ARBLOST                 */
#define _I2C_IF_ARBLOST_MASK                0x200UL                                     /**< Bit mask for I2C_ARBLOST                    */
#define _I2C_IF_ARBLOST_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_ARBLOST_DEFAULT              (_I2C_IF_ARBLOST_DEFAULT << 9)              /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_BUSERR                       (0x1UL << 10)                               /**< Bus Error Interrupt Flag                    */
#define _I2C_IF_BUSERR_SHIFT                10                                          /**< Shift value for I2C_BUSERR                  */
#define _I2C_IF_BUSERR_MASK                 0x400UL                                     /**< Bit mask for I2C_BUSERR                     */
#define _I2C_IF_BUSERR_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_BUSERR_DEFAULT               (_I2C_IF_BUSERR_DEFAULT << 10)              /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_BUSHOLD                      (0x1UL << 11)                               /**< Bus Held Interrupt Flag                     */
#define _I2C_IF_BUSHOLD_SHIFT               11                                          /**< Shift value for I2C_BUSHOLD                 */
#define _I2C_IF_BUSHOLD_MASK                0x800UL                                     /**< Bit mask for I2C_BUSHOLD                    */
#define _I2C_IF_BUSHOLD_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_BUSHOLD_DEFAULT              (_I2C_IF_BUSHOLD_DEFAULT << 11)             /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_TXOF                         (0x1UL << 12)                               /**< Transmit FIFO Overflow Interrupt Flag       */
#define _I2C_IF_TXOF_SHIFT                  12                                          /**< Shift value for I2C_TXOF                    */
#define _I2C_IF_TXOF_MASK                   0x1000UL                                    /**< Bit mask for I2C_TXOF                       */
#define _I2C_IF_TXOF_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_TXOF_DEFAULT                 (_I2C_IF_TXOF_DEFAULT << 12)                /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_RXUF                         (0x1UL << 13)                               /**< Receive FIFO Underflow Interrupt Flag       */
#define _I2C_IF_RXUF_SHIFT                  13                                          /**< Shift value for I2C_RXUF                    */
#define _I2C_IF_RXUF_MASK                   0x2000UL                                    /**< Bit mask for I2C_RXUF                       */
#define _I2C_IF_RXUF_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_RXUF_DEFAULT                 (_I2C_IF_RXUF_DEFAULT << 13)                /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_BITO                         (0x1UL << 14)                               /**< Bus Idle Timeout Interrupt Flag             */
#define _I2C_IF_BITO_SHIFT                  14                                          /**< Shift value for I2C_BITO                    */
#define _I2C_IF_BITO_MASK                   0x4000UL                                    /**< Bit mask for I2C_BITO                       */
#define _I2C_IF_BITO_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_BITO_DEFAULT                 (_I2C_IF_BITO_DEFAULT << 14)                /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_CLTO                         (0x1UL << 15)                               /**< Clock Low Timeout Interrupt Flag            */
#define _I2C_IF_CLTO_SHIFT                  15                                          /**< Shift value for I2C_CLTO                    */
#define _I2C_IF_CLTO_MASK                   0x8000UL                                    /**< Bit mask for I2C_CLTO                       */
#define _I2C_IF_CLTO_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_CLTO_DEFAULT                 (_I2C_IF_CLTO_DEFAULT << 15)                /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_SSTOP                        (0x1UL << 16)                               /**< Follower STOP condition Interrupt Flag      */
#define _I2C_IF_SSTOP_SHIFT                 16                                          /**< Shift value for I2C_SSTOP                   */
#define _I2C_IF_SSTOP_MASK                  0x10000UL                                   /**< Bit mask for I2C_SSTOP                      */
#define _I2C_IF_SSTOP_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_SSTOP_DEFAULT                (_I2C_IF_SSTOP_DEFAULT << 16)               /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_RXFULL                       (0x1UL << 17)                               /**< Receive Fifo Full Interrupt Flag            */
#define _I2C_IF_RXFULL_SHIFT                17                                          /**< Shift value for I2C_RXFULL                  */
#define _I2C_IF_RXFULL_MASK                 0x20000UL                                   /**< Bit mask for I2C_RXFULL                     */
#define _I2C_IF_RXFULL_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_RXFULL_DEFAULT               (_I2C_IF_RXFULL_DEFAULT << 17)              /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_CLERR                        (0x1UL << 18)                               /**< Clock Low Error Interrupt Flag              */
#define _I2C_IF_CLERR_SHIFT                 18                                          /**< Shift value for I2C_CLERR                   */
#define _I2C_IF_CLERR_MASK                  0x40000UL                                   /**< Bit mask for I2C_CLERR                      */
#define _I2C_IF_CLERR_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_CLERR_DEFAULT                (_I2C_IF_CLERR_DEFAULT << 18)               /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_SCLERR                       (0x1UL << 19)                               /**< SCL Error Interrupt Flag                    */
#define _I2C_IF_SCLERR_SHIFT                19                                          /**< Shift value for I2C_SCLERR                  */
#define _I2C_IF_SCLERR_MASK                 0x80000UL                                   /**< Bit mask for I2C_SCLERR                     */
#define _I2C_IF_SCLERR_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_SCLERR_DEFAULT               (_I2C_IF_SCLERR_DEFAULT << 19)              /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_SDAERR                       (0x1UL << 20)                               /**< SDA Error Interrupt Flag                    */
#define _I2C_IF_SDAERR_SHIFT                20                                          /**< Shift value for I2C_SDAERR                  */
#define _I2C_IF_SDAERR_MASK                 0x100000UL                                  /**< Bit mask for I2C_SDAERR                     */
#define _I2C_IF_SDAERR_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_SDAERR_DEFAULT               (_I2C_IF_SDAERR_DEFAULT << 20)              /**< Shifted mode DEFAULT for I2C_IF             */
#define I2C_IF_RXFLIF                       (0x1UL << 21)                               /**< RX FIFO level Interrupt Flag                */
#define _I2C_IF_RXFLIF_SHIFT                21                                          /**< Shift value for I2C_RXFLIF                  */
#define _I2C_IF_RXFLIF_MASK                 0x200000UL                                  /**< Bit mask for I2C_RXFLIF                     */
#define _I2C_IF_RXFLIF_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IF                     */
#define I2C_IF_RXFLIF_DEFAULT               (_I2C_IF_RXFLIF_DEFAULT << 21)              /**< Shifted mode DEFAULT for I2C_IF             */

/* Bit fields for I2C IEN */
#define _I2C_IEN_RESETVALUE                 0x00000000UL                                /**< Default value for I2C_IEN                   */
#define _I2C_IEN_MASK                       0x003FFFFFUL                                /**< Mask for I2C_IEN                            */
#define I2C_IEN_START                       (0x1UL << 0)                                /**< START condition Interrupt Flag              */
#define _I2C_IEN_START_SHIFT                0                                           /**< Shift value for I2C_START                   */
#define _I2C_IEN_START_MASK                 0x1UL                                       /**< Bit mask for I2C_START                      */
#define _I2C_IEN_START_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_START_DEFAULT               (_I2C_IEN_START_DEFAULT << 0)               /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_RSTART                      (0x1UL << 1)                                /**< Repeated START condition Interrupt Flag     */
#define _I2C_IEN_RSTART_SHIFT               1                                           /**< Shift value for I2C_RSTART                  */
#define _I2C_IEN_RSTART_MASK                0x2UL                                       /**< Bit mask for I2C_RSTART                     */
#define _I2C_IEN_RSTART_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_RSTART_DEFAULT              (_I2C_IEN_RSTART_DEFAULT << 1)              /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_ADDR                        (0x1UL << 2)                                /**< Address Interrupt Flag                      */
#define _I2C_IEN_ADDR_SHIFT                 2                                           /**< Shift value for I2C_ADDR                    */
#define _I2C_IEN_ADDR_MASK                  0x4UL                                       /**< Bit mask for I2C_ADDR                       */
#define _I2C_IEN_ADDR_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_ADDR_DEFAULT                (_I2C_IEN_ADDR_DEFAULT << 2)                /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_TXC                         (0x1UL << 3)                                /**< Transfer Completed Interrupt Flag           */
#define _I2C_IEN_TXC_SHIFT                  3                                           /**< Shift value for I2C_TXC                     */
#define _I2C_IEN_TXC_MASK                   0x8UL                                       /**< Bit mask for I2C_TXC                        */
#define _I2C_IEN_TXC_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_TXC_DEFAULT                 (_I2C_IEN_TXC_DEFAULT << 3)                 /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_TXBL                        (0x1UL << 4)                                /**< Transmit FIFO Level Interrupt Flag          */
#define _I2C_IEN_TXBL_SHIFT                 4                                           /**< Shift value for I2C_TXBL                    */
#define _I2C_IEN_TXBL_MASK                  0x10UL                                      /**< Bit mask for I2C_TXBL                       */
#define _I2C_IEN_TXBL_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_TXBL_DEFAULT                (_I2C_IEN_TXBL_DEFAULT << 4)                /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_RXDATAV                     (0x1UL << 5)                                /**< Receive Data Valid Interrupt Flag           */
#define _I2C_IEN_RXDATAV_SHIFT              5                                           /**< Shift value for I2C_RXDATAV                 */
#define _I2C_IEN_RXDATAV_MASK               0x20UL                                      /**< Bit mask for I2C_RXDATAV                    */
#define _I2C_IEN_RXDATAV_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_RXDATAV_DEFAULT             (_I2C_IEN_RXDATAV_DEFAULT << 5)             /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_ACK                         (0x1UL << 6)                                /**< Acknowledge Received Interrupt Flag         */
#define _I2C_IEN_ACK_SHIFT                  6                                           /**< Shift value for I2C_ACK                     */
#define _I2C_IEN_ACK_MASK                   0x40UL                                      /**< Bit mask for I2C_ACK                        */
#define _I2C_IEN_ACK_DEFAULT                0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_ACK_DEFAULT                 (_I2C_IEN_ACK_DEFAULT << 6)                 /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_NACK                        (0x1UL << 7)                                /**< Not Acknowledge Received Interrupt Flag     */
#define _I2C_IEN_NACK_SHIFT                 7                                           /**< Shift value for I2C_NACK                    */
#define _I2C_IEN_NACK_MASK                  0x80UL                                      /**< Bit mask for I2C_NACK                       */
#define _I2C_IEN_NACK_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_NACK_DEFAULT                (_I2C_IEN_NACK_DEFAULT << 7)                /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_MSTOP                       (0x1UL << 8)                                /**< Leader STOP Condition Interrupt Flag        */
#define _I2C_IEN_MSTOP_SHIFT                8                                           /**< Shift value for I2C_MSTOP                   */
#define _I2C_IEN_MSTOP_MASK                 0x100UL                                     /**< Bit mask for I2C_MSTOP                      */
#define _I2C_IEN_MSTOP_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_MSTOP_DEFAULT               (_I2C_IEN_MSTOP_DEFAULT << 8)               /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_ARBLOST                     (0x1UL << 9)                                /**< Arbitration Lost Interrupt Flag             */
#define _I2C_IEN_ARBLOST_SHIFT              9                                           /**< Shift value for I2C_ARBLOST                 */
#define _I2C_IEN_ARBLOST_MASK               0x200UL                                     /**< Bit mask for I2C_ARBLOST                    */
#define _I2C_IEN_ARBLOST_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_ARBLOST_DEFAULT             (_I2C_IEN_ARBLOST_DEFAULT << 9)             /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_BUSERR                      (0x1UL << 10)                               /**< Bus Error Interrupt Flag                    */
#define _I2C_IEN_BUSERR_SHIFT               10                                          /**< Shift value for I2C_BUSERR                  */
#define _I2C_IEN_BUSERR_MASK                0x400UL                                     /**< Bit mask for I2C_BUSERR                     */
#define _I2C_IEN_BUSERR_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_BUSERR_DEFAULT              (_I2C_IEN_BUSERR_DEFAULT << 10)             /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_BUSHOLD                     (0x1UL << 11)                               /**< Bus Held Interrupt Flag                     */
#define _I2C_IEN_BUSHOLD_SHIFT              11                                          /**< Shift value for I2C_BUSHOLD                 */
#define _I2C_IEN_BUSHOLD_MASK               0x800UL                                     /**< Bit mask for I2C_BUSHOLD                    */
#define _I2C_IEN_BUSHOLD_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_BUSHOLD_DEFAULT             (_I2C_IEN_BUSHOLD_DEFAULT << 11)            /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_TXOF                        (0x1UL << 12)                               /**< Transmit FIFO Overflow Interrupt Flag       */
#define _I2C_IEN_TXOF_SHIFT                 12                                          /**< Shift value for I2C_TXOF                    */
#define _I2C_IEN_TXOF_MASK                  0x1000UL                                    /**< Bit mask for I2C_TXOF                       */
#define _I2C_IEN_TXOF_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_TXOF_DEFAULT                (_I2C_IEN_TXOF_DEFAULT << 12)               /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_RXUF                        (0x1UL << 13)                               /**< Receive FIFO Underflow Interrupt Flag       */
#define _I2C_IEN_RXUF_SHIFT                 13                                          /**< Shift value for I2C_RXUF                    */
#define _I2C_IEN_RXUF_MASK                  0x2000UL                                    /**< Bit mask for I2C_RXUF                       */
#define _I2C_IEN_RXUF_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_RXUF_DEFAULT                (_I2C_IEN_RXUF_DEFAULT << 13)               /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_BITO                        (0x1UL << 14)                               /**< Bus Idle Timeout Interrupt Flag             */
#define _I2C_IEN_BITO_SHIFT                 14                                          /**< Shift value for I2C_BITO                    */
#define _I2C_IEN_BITO_MASK                  0x4000UL                                    /**< Bit mask for I2C_BITO                       */
#define _I2C_IEN_BITO_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_BITO_DEFAULT                (_I2C_IEN_BITO_DEFAULT << 14)               /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_CLTO                        (0x1UL << 15)                               /**< Clock Low Timeout Interrupt Flag            */
#define _I2C_IEN_CLTO_SHIFT                 15                                          /**< Shift value for I2C_CLTO                    */
#define _I2C_IEN_CLTO_MASK                  0x8000UL                                    /**< Bit mask for I2C_CLTO                       */
#define _I2C_IEN_CLTO_DEFAULT               0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_CLTO_DEFAULT                (_I2C_IEN_CLTO_DEFAULT << 15)               /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_SSTOP                       (0x1UL << 16)                               /**< Follower STOP condition Interrupt Flag      */
#define _I2C_IEN_SSTOP_SHIFT                16                                          /**< Shift value for I2C_SSTOP                   */
#define _I2C_IEN_SSTOP_MASK                 0x10000UL                                   /**< Bit mask for I2C_SSTOP                      */
#define _I2C_IEN_SSTOP_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_SSTOP_DEFAULT               (_I2C_IEN_SSTOP_DEFAULT << 16)              /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_RXFULL                      (0x1UL << 17)                               /**< Receive Fifo Full Interrupt Flag            */
#define _I2C_IEN_RXFULL_SHIFT               17                                          /**< Shift value for I2C_RXFULL                  */
#define _I2C_IEN_RXFULL_MASK                0x20000UL                                   /**< Bit mask for I2C_RXFULL                     */
#define _I2C_IEN_RXFULL_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_RXFULL_DEFAULT              (_I2C_IEN_RXFULL_DEFAULT << 17)             /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_CLERR                       (0x1UL << 18)                               /**< Clock Low Error Interrupt Flag              */
#define _I2C_IEN_CLERR_SHIFT                18                                          /**< Shift value for I2C_CLERR                   */
#define _I2C_IEN_CLERR_MASK                 0x40000UL                                   /**< Bit mask for I2C_CLERR                      */
#define _I2C_IEN_CLERR_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_CLERR_DEFAULT               (_I2C_IEN_CLERR_DEFAULT << 18)              /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_SCLERR                      (0x1UL << 19)                               /**< SCL Error Interrupt Flag                    */
#define _I2C_IEN_SCLERR_SHIFT               19                                          /**< Shift value for I2C_SCLERR                  */
#define _I2C_IEN_SCLERR_MASK                0x80000UL                                   /**< Bit mask for I2C_SCLERR                     */
#define _I2C_IEN_SCLERR_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_SCLERR_DEFAULT              (_I2C_IEN_SCLERR_DEFAULT << 19)             /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_SDAERR                      (0x1UL << 20)                               /**< SDA Error Interrupt Flag                    */
#define _I2C_IEN_SDAERR_SHIFT               20                                          /**< Shift value for I2C_SDAERR                  */
#define _I2C_IEN_SDAERR_MASK                0x100000UL                                  /**< Bit mask for I2C_SDAERR                     */
#define _I2C_IEN_SDAERR_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_SDAERR_DEFAULT              (_I2C_IEN_SDAERR_DEFAULT << 20)             /**< Shifted mode DEFAULT for I2C_IEN            */
#define I2C_IEN_RXFLIEN                     (0x1UL << 21)                               /**< RX FIFO level interrupt enable              */
#define _I2C_IEN_RXFLIEN_SHIFT              21                                          /**< Shift value for I2C_RXFLIEN                 */
#define _I2C_IEN_RXFLIEN_MASK               0x200000UL                                  /**< Bit mask for I2C_RXFLIEN                    */
#define _I2C_IEN_RXFLIEN_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for I2C_IEN                    */
#define I2C_IEN_RXFLIEN_DEFAULT             (_I2C_IEN_RXFLIEN_DEFAULT << 21)            /**< Shifted mode DEFAULT for I2C_IEN            */

/* Bit fields for I2C SYNCBUSY */
#define _I2C_SYNCBUSY_RESETVALUE            0x00000000UL                                /**< Default value for I2C_SYNCBUSY              */
#define _I2C_SYNCBUSY_MASK                  0x007FFFFFUL                                /**< Mask for I2C_SYNCBUSY                       */
#define I2C_SYNCBUSY_DIV                    (0x1UL << 0)                                /**< Syncbusy of CLKDIV                          */
#define _I2C_SYNCBUSY_DIV_SHIFT             0                                           /**< Shift value for I2C_DIV                     */
#define _I2C_SYNCBUSY_DIV_MASK              0x1UL                                       /**< Bit mask for I2C_DIV                        */
#define _I2C_SYNCBUSY_DIV_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_DIV_DEFAULT            (_I2C_SYNCBUSY_DIV_DEFAULT << 0)            /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_ADDR                   (0x1UL << 1)                                /**< Sync busy of SADDR                          */
#define _I2C_SYNCBUSY_ADDR_SHIFT            1                                           /**< Shift value for I2C_ADDR                    */
#define _I2C_SYNCBUSY_ADDR_MASK             0x2UL                                       /**< Bit mask for I2C_ADDR                       */
#define _I2C_SYNCBUSY_ADDR_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_ADDR_DEFAULT           (_I2C_SYNCBUSY_ADDR_DEFAULT << 1)           /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_SADDRMASK              (0x1UL << 2)                                /**< Sync busy of SADDRMASK                      */
#define _I2C_SYNCBUSY_SADDRMASK_SHIFT       2                                           /**< Shift value for I2C_SADDRMASK               */
#define _I2C_SYNCBUSY_SADDRMASK_MASK        0x4UL                                       /**< Bit mask for I2C_SADDRMASK                  */
#define _I2C_SYNCBUSY_SADDRMASK_DEFAULT     0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_SADDRMASK_DEFAULT      (_I2C_SYNCBUSY_SADDRMASK_DEFAULT << 2)      /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_CORERST                (0x1UL << 3)                                /**< Sync busy of CORERST                        */
#define _I2C_SYNCBUSY_CORERST_SHIFT         3                                           /**< Shift value for I2C_CORERST                 */
#define _I2C_SYNCBUSY_CORERST_MASK          0x8UL                                       /**< Bit mask for I2C_CORERST                    */
#define _I2C_SYNCBUSY_CORERST_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_CORERST_DEFAULT        (_I2C_SYNCBUSY_CORERST_DEFAULT << 3)        /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_SLAVE                  (0x1UL << 4)                                /**< Sync busy of SLAVE                          */
#define _I2C_SYNCBUSY_SLAVE_SHIFT           4                                           /**< Shift value for I2C_SLAVE                   */
#define _I2C_SYNCBUSY_SLAVE_MASK            0x10UL                                      /**< Bit mask for I2C_SLAVE                      */
#define _I2C_SYNCBUSY_SLAVE_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_SLAVE_DEFAULT          (_I2C_SYNCBUSY_SLAVE_DEFAULT << 4)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_AUTOACK                (0x1UL << 5)                                /**< Sync busy of AUTOACK                        */
#define _I2C_SYNCBUSY_AUTOACK_SHIFT         5                                           /**< Shift value for I2C_AUTOACK                 */
#define _I2C_SYNCBUSY_AUTOACK_MASK          0x20UL                                      /**< Bit mask for I2C_AUTOACK                    */
#define _I2C_SYNCBUSY_AUTOACK_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_AUTOACK_DEFAULT        (_I2C_SYNCBUSY_AUTOACK_DEFAULT << 5)        /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_AUTOSE                 (0x1UL << 6)                                /**< Sync busy of AUTOSE                         */
#define _I2C_SYNCBUSY_AUTOSE_SHIFT          6                                           /**< Shift value for I2C_AUTOSE                  */
#define _I2C_SYNCBUSY_AUTOSE_MASK           0x40UL                                      /**< Bit mask for I2C_AUTOSE                     */
#define _I2C_SYNCBUSY_AUTOSE_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_AUTOSE_DEFAULT         (_I2C_SYNCBUSY_AUTOSE_DEFAULT << 6)         /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_AUTOSN                 (0x1UL << 7)                                /**< Sync busy of AUTOSN                         */
#define _I2C_SYNCBUSY_AUTOSN_SHIFT          7                                           /**< Shift value for I2C_AUTOSN                  */
#define _I2C_SYNCBUSY_AUTOSN_MASK           0x80UL                                      /**< Bit mask for I2C_AUTOSN                     */
#define _I2C_SYNCBUSY_AUTOSN_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_AUTOSN_DEFAULT         (_I2C_SYNCBUSY_AUTOSN_DEFAULT << 7)         /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_ARBDIS                 (0x1UL << 8)                                /**< Sync busy of ARBDIS                         */
#define _I2C_SYNCBUSY_ARBDIS_SHIFT          8                                           /**< Shift value for I2C_ARBDIS                  */
#define _I2C_SYNCBUSY_ARBDIS_MASK           0x100UL                                     /**< Bit mask for I2C_ARBDIS                     */
#define _I2C_SYNCBUSY_ARBDIS_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_ARBDIS_DEFAULT         (_I2C_SYNCBUSY_ARBDIS_DEFAULT << 8)         /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_GCAMEN                 (0x1UL << 9)                                /**< Sync busy of GCAMEN                         */
#define _I2C_SYNCBUSY_GCAMEN_SHIFT          9                                           /**< Shift value for I2C_GCAMEN                  */
#define _I2C_SYNCBUSY_GCAMEN_MASK           0x200UL                                     /**< Bit mask for I2C_GCAMEN                     */
#define _I2C_SYNCBUSY_GCAMEN_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_GCAMEN_DEFAULT         (_I2C_SYNCBUSY_GCAMEN_DEFAULT << 9)         /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_CLHR                   (0x1UL << 10)                               /**< Sync busy of CLHR                           */
#define _I2C_SYNCBUSY_CLHR_SHIFT            10                                          /**< Shift value for I2C_CLHR                    */
#define _I2C_SYNCBUSY_CLHR_MASK             0x400UL                                     /**< Bit mask for I2C_CLHR                       */
#define _I2C_SYNCBUSY_CLHR_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_CLHR_DEFAULT           (_I2C_SYNCBUSY_CLHR_DEFAULT << 10)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_BITO                   (0x1UL << 11)                               /**< Sync busy of BITO                           */
#define _I2C_SYNCBUSY_BITO_SHIFT            11                                          /**< Shift value for I2C_BITO                    */
#define _I2C_SYNCBUSY_BITO_MASK             0x800UL                                     /**< Bit mask for I2C_BITO                       */
#define _I2C_SYNCBUSY_BITO_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_BITO_DEFAULT           (_I2C_SYNCBUSY_BITO_DEFAULT << 11)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_GIBITO                 (0x1UL << 12)                               /**< Sync busy of GIBITO                         */
#define _I2C_SYNCBUSY_GIBITO_SHIFT          12                                          /**< Shift value for I2C_GIBITO                  */
#define _I2C_SYNCBUSY_GIBITO_MASK           0x1000UL                                    /**< Bit mask for I2C_GIBITO                     */
#define _I2C_SYNCBUSY_GIBITO_DEFAULT        0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_GIBITO_DEFAULT         (_I2C_SYNCBUSY_GIBITO_DEFAULT << 12)        /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_CLTO                   (0x1UL << 13)                               /**< Sync busy of CLTO                           */
#define _I2C_SYNCBUSY_CLTO_SHIFT            13                                          /**< Shift value for I2C_CLTO                    */
#define _I2C_SYNCBUSY_CLTO_MASK             0x2000UL                                    /**< Bit mask for I2C_CLTO                       */
#define _I2C_SYNCBUSY_CLTO_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_CLTO_DEFAULT           (_I2C_SYNCBUSY_CLTO_DEFAULT << 13)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_SCLMONEN               (0x1UL << 14)                               /**< Sync busy of SCLMONEN                       */
#define _I2C_SYNCBUSY_SCLMONEN_SHIFT        14                                          /**< Shift value for I2C_SCLMONEN                */
#define _I2C_SYNCBUSY_SCLMONEN_MASK         0x4000UL                                    /**< Bit mask for I2C_SCLMONEN                   */
#define _I2C_SYNCBUSY_SCLMONEN_DEFAULT      0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_SCLMONEN_DEFAULT       (_I2C_SYNCBUSY_SCLMONEN_DEFAULT << 14)      /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_SDAMONEN               (0x1UL << 15)                               /**< Sync busy of SDAMONEN                       */
#define _I2C_SYNCBUSY_SDAMONEN_SHIFT        15                                          /**< Shift value for I2C_SDAMONEN                */
#define _I2C_SYNCBUSY_SDAMONEN_MASK         0x8000UL                                    /**< Bit mask for I2C_SDAMONEN                   */
#define _I2C_SYNCBUSY_SDAMONEN_DEFAULT      0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_SDAMONEN_DEFAULT       (_I2C_SYNCBUSY_SDAMONEN_DEFAULT << 15)      /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_START                  (0x1UL << 16)                               /**< Sync busy of START                          */
#define _I2C_SYNCBUSY_START_SHIFT           16                                          /**< Shift value for I2C_START                   */
#define _I2C_SYNCBUSY_START_MASK            0x10000UL                                   /**< Bit mask for I2C_START                      */
#define _I2C_SYNCBUSY_START_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_START_DEFAULT          (_I2C_SYNCBUSY_START_DEFAULT << 16)         /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_STOP                   (0x1UL << 17)                               /**< Sync busy of STOP                           */
#define _I2C_SYNCBUSY_STOP_SHIFT            17                                          /**< Shift value for I2C_STOP                    */
#define _I2C_SYNCBUSY_STOP_MASK             0x20000UL                                   /**< Bit mask for I2C_STOP                       */
#define _I2C_SYNCBUSY_STOP_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_STOP_DEFAULT           (_I2C_SYNCBUSY_STOP_DEFAULT << 17)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_ACK                    (0x1UL << 18)                               /**< Sync busy of ACK                            */
#define _I2C_SYNCBUSY_ACK_SHIFT             18                                          /**< Shift value for I2C_ACK                     */
#define _I2C_SYNCBUSY_ACK_MASK              0x40000UL                                   /**< Bit mask for I2C_ACK                        */
#define _I2C_SYNCBUSY_ACK_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_ACK_DEFAULT            (_I2C_SYNCBUSY_ACK_DEFAULT << 18)           /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_NACK                   (0x1UL << 19)                               /**< Sync busy of NACK                           */
#define _I2C_SYNCBUSY_NACK_SHIFT            19                                          /**< Shift value for I2C_NACK                    */
#define _I2C_SYNCBUSY_NACK_MASK             0x80000UL                                   /**< Bit mask for I2C_NACK                       */
#define _I2C_SYNCBUSY_NACK_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_NACK_DEFAULT           (_I2C_SYNCBUSY_NACK_DEFAULT << 19)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_CONT                   (0x1UL << 20)                               /**< Sync busy of CONT                           */
#define _I2C_SYNCBUSY_CONT_SHIFT            20                                          /**< Shift value for I2C_CONT                    */
#define _I2C_SYNCBUSY_CONT_MASK             0x100000UL                                  /**< Bit mask for I2C_CONT                       */
#define _I2C_SYNCBUSY_CONT_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_CONT_DEFAULT           (_I2C_SYNCBUSY_CONT_DEFAULT << 20)          /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_ABORT                  (0x1UL << 21)                               /**< Sync busy of ABORT                          */
#define _I2C_SYNCBUSY_ABORT_SHIFT           21                                          /**< Shift value for I2C_ABORT                   */
#define _I2C_SYNCBUSY_ABORT_MASK            0x200000UL                                  /**< Bit mask for I2C_ABORT                      */
#define _I2C_SYNCBUSY_ABORT_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_ABORT_DEFAULT          (_I2C_SYNCBUSY_ABORT_DEFAULT << 21)         /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */
#define I2C_SYNCBUSY_CLEARPC                (0x1UL << 22)                               /**< Sync busy of CLEARPC                        */
#define _I2C_SYNCBUSY_CLEARPC_SHIFT         22                                          /**< Shift value for I2C_CLEARPC                 */
#define _I2C_SYNCBUSY_CLEARPC_MASK          0x400000UL                                  /**< Bit mask for I2C_CLEARPC                    */
#define _I2C_SYNCBUSY_CLEARPC_DEFAULT       0x00000000UL                                /**< Mode DEFAULT for I2C_SYNCBUSY               */
#define I2C_SYNCBUSY_CLEARPC_DEFAULT        (_I2C_SYNCBUSY_CLEARPC_DEFAULT << 22)       /**< Shifted mode DEFAULT for I2C_SYNCBUSY       */

/** @} End of group SIMG301_I2C_BitFields */
/** @} End of group SIMG301_I2C */
/** @} End of group Parts */

#endif // SIMG301_I2C_H
