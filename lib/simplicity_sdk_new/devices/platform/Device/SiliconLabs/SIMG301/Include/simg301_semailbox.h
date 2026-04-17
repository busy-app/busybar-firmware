/**************************************************************************//**
 * @file
 * @brief SIMG301 SEMAILBOX register and bit field definitions
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
#ifndef SIMG301_SEMAILBOX_H
#define SIMG301_SEMAILBOX_H

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIMG301_SEMAILBOX_HOST SEMAILBOX_HOST
 * @{
 * @brief SIMG301 SEMAILBOX_HOST Register Declaration.
 *****************************************************************************/

/** SEMAILBOX_HOST Register Declaration. */
typedef struct semailbox_host_typedef{
  __IOM uint32_t FIFO;                               /**< FIFO                                               */
  uint32_t       RESERVED0[15U];                     /**< Reserved for future use                            */
  __IM uint32_t  TX_STATUS;                          /**< TX_STATUS                                          */
  __IM uint32_t  RX_STATUS;                          /**< RX_STATUS                                          */
  __IM uint32_t  TX_PROT;                            /**< TX_PROT                                            */
  __IM uint32_t  RX_PROT;                            /**< RX_PROT                                            */
  __IOM uint32_t TX_HEADER;                          /**< TX_HEADER                                          */
  __IM uint32_t  RX_HEADER;                          /**< RX_HEADER                                          */
  __IOM uint32_t CONFIGURATION;                      /**< CONFIGURATION                                      */
} SEMAILBOX_HOST_TypeDef;
/** @} End of group SIMG301_SEMAILBOX_HOST */

/**************************************************************************//**
 * @addtogroup SIMG301_SEMAILBOX_HOST
 * @{
 * @defgroup SIMG301_SEMAILBOX_HOST_BitFields SEMAILBOX_HOST Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SEMAILBOX FIFO */
#define _SEMAILBOX_FIFO_RESETVALUE                  0x00000000UL                        /**< Default value for SEMAILBOX_FIFO            */
#define _SEMAILBOX_FIFO_MASK                        0xFFFFFFFFUL                        /**< Mask for SEMAILBOX_FIFO                     */
#define _SEMAILBOX_FIFO_FIFO_SHIFT                  0                                   /**< Shift value for SEMAILBOX_FIFO              */
#define _SEMAILBOX_FIFO_FIFO_MASK                   0xFFFFFFFFUL                        /**< Bit mask for SEMAILBOX_FIFO                 */
#define _SEMAILBOX_FIFO_FIFO_DEFAULT                0x00000000UL                        /**< Mode DEFAULT for SEMAILBOX_FIFO             */
#define SEMAILBOX_FIFO_FIFO_DEFAULT                 (_SEMAILBOX_FIFO_FIFO_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAILBOX_FIFO     */

/* Bit fields for SEMAILBOX TX_STATUS */
#define _SEMAILBOX_TX_STATUS_RESETVALUE             0x00000000UL                                     /**< Default value for SEMAILBOX_TX_STATUS       */
#define _SEMAILBOX_TX_STATUS_MASK                   0x01BFFFFFUL                                     /**< Mask for SEMAILBOX_TX_STATUS                */
#define _SEMAILBOX_TX_STATUS_BYTEREM_SHIFT          0                                                /**< Shift value for SEMAILBOX_BYTEREM           */
#define _SEMAILBOX_TX_STATUS_BYTEREM_MASK           0xFFFFUL                                         /**< Bit mask for SEMAILBOX_BYTEREM              */
#define _SEMAILBOX_TX_STATUS_BYTEREM_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_TX_STATUS        */
#define SEMAILBOX_TX_STATUS_BYTEREM_DEFAULT         (_SEMAILBOX_TX_STATUS_BYTEREM_DEFAULT << 0)      /**< Shifted mode DEFAULT for SEMAILBOX_TX_STATUS*/
#define _SEMAILBOX_TX_STATUS_MSGINFO_SHIFT          16                                               /**< Shift value for SEMAILBOX_MSGINFO           */
#define _SEMAILBOX_TX_STATUS_MSGINFO_MASK           0xF0000UL                                        /**< Bit mask for SEMAILBOX_MSGINFO              */
#define _SEMAILBOX_TX_STATUS_MSGINFO_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_TX_STATUS        */
#define SEMAILBOX_TX_STATUS_MSGINFO_DEFAULT         (_SEMAILBOX_TX_STATUS_MSGINFO_DEFAULT << 16)     /**< Shifted mode DEFAULT for SEMAILBOX_TX_STATUS*/
#define SEMAILBOX_TX_STATUS_TXINT                   (0x1UL << 20)                                    /**< TXINT                                       */
#define _SEMAILBOX_TX_STATUS_TXINT_SHIFT            20                                               /**< Shift value for SEMAILBOX_TXINT             */
#define _SEMAILBOX_TX_STATUS_TXINT_MASK             0x100000UL                                       /**< Bit mask for SEMAILBOX_TXINT                */
#define _SEMAILBOX_TX_STATUS_TXINT_DEFAULT          0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_TX_STATUS        */
#define SEMAILBOX_TX_STATUS_TXINT_DEFAULT           (_SEMAILBOX_TX_STATUS_TXINT_DEFAULT << 20)       /**< Shifted mode DEFAULT for SEMAILBOX_TX_STATUS*/
#define SEMAILBOX_TX_STATUS_TXFULL                  (0x1UL << 21)                                    /**< TXFULL                                      */
#define _SEMAILBOX_TX_STATUS_TXFULL_SHIFT           21                                               /**< Shift value for SEMAILBOX_TXFULL            */
#define _SEMAILBOX_TX_STATUS_TXFULL_MASK            0x200000UL                                       /**< Bit mask for SEMAILBOX_TXFULL               */
#define _SEMAILBOX_TX_STATUS_TXFULL_DEFAULT         0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_TX_STATUS        */
#define SEMAILBOX_TX_STATUS_TXFULL_DEFAULT          (_SEMAILBOX_TX_STATUS_TXFULL_DEFAULT << 21)      /**< Shifted mode DEFAULT for SEMAILBOX_TX_STATUS*/
#define SEMAILBOX_TX_STATUS_TXERROR                 (0x1UL << 23)                                    /**< TXERROR                                     */
#define _SEMAILBOX_TX_STATUS_TXERROR_SHIFT          23                                               /**< Shift value for SEMAILBOX_TXERROR           */
#define _SEMAILBOX_TX_STATUS_TXERROR_MASK           0x800000UL                                       /**< Bit mask for SEMAILBOX_TXERROR              */
#define _SEMAILBOX_TX_STATUS_TXERROR_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_TX_STATUS        */
#define SEMAILBOX_TX_STATUS_TXERROR_DEFAULT         (_SEMAILBOX_TX_STATUS_TXERROR_DEFAULT << 23)     /**< Shifted mode DEFAULT for SEMAILBOX_TX_STATUS*/
#define SEMAILBOX_TX_STATUS_UNPROTECTED             (0x1UL << 24)                                    /**< UNPROTECTED                                 */
#define _SEMAILBOX_TX_STATUS_UNPROTECTED_SHIFT      24                                               /**< Shift value for SEMAILBOX_UNPROTECTED       */
#define _SEMAILBOX_TX_STATUS_UNPROTECTED_MASK       0x1000000UL                                      /**< Bit mask for SEMAILBOX_UNPROTECTED          */
#define _SEMAILBOX_TX_STATUS_UNPROTECTED_DEFAULT    0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_TX_STATUS        */
#define SEMAILBOX_TX_STATUS_UNPROTECTED_DEFAULT     (_SEMAILBOX_TX_STATUS_UNPROTECTED_DEFAULT << 24) /**< Shifted mode DEFAULT for SEMAILBOX_TX_STATUS*/

/* Bit fields for SEMAILBOX RX_STATUS */
#define _SEMAILBOX_RX_STATUS_RESETVALUE             0x00000000UL                                     /**< Default value for SEMAILBOX_RX_STATUS       */
#define _SEMAILBOX_RX_STATUS_MASK                   0x01FFFFFFUL                                     /**< Mask for SEMAILBOX_RX_STATUS                */
#define _SEMAILBOX_RX_STATUS_BYTEREM_SHIFT          0                                                /**< Shift value for SEMAILBOX_BYTEREM           */
#define _SEMAILBOX_RX_STATUS_BYTEREM_MASK           0xFFFFUL                                         /**< Bit mask for SEMAILBOX_BYTEREM              */
#define _SEMAILBOX_RX_STATUS_BYTEREM_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_BYTEREM_DEFAULT         (_SEMAILBOX_RX_STATUS_BYTEREM_DEFAULT << 0)      /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/
#define _SEMAILBOX_RX_STATUS_MSGINFO_SHIFT          16                                               /**< Shift value for SEMAILBOX_MSGINFO           */
#define _SEMAILBOX_RX_STATUS_MSGINFO_MASK           0xF0000UL                                        /**< Bit mask for SEMAILBOX_MSGINFO              */
#define _SEMAILBOX_RX_STATUS_MSGINFO_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_MSGINFO_DEFAULT         (_SEMAILBOX_RX_STATUS_MSGINFO_DEFAULT << 16)     /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/
#define SEMAILBOX_RX_STATUS_RXINT                   (0x1UL << 20)                                    /**< RXINT                                       */
#define _SEMAILBOX_RX_STATUS_RXINT_SHIFT            20                                               /**< Shift value for SEMAILBOX_RXINT             */
#define _SEMAILBOX_RX_STATUS_RXINT_MASK             0x100000UL                                       /**< Bit mask for SEMAILBOX_RXINT                */
#define _SEMAILBOX_RX_STATUS_RXINT_DEFAULT          0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_RXINT_DEFAULT           (_SEMAILBOX_RX_STATUS_RXINT_DEFAULT << 20)       /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/
#define SEMAILBOX_RX_STATUS_RXEMPTY                 (0x1UL << 21)                                    /**< RXEMPTY                                     */
#define _SEMAILBOX_RX_STATUS_RXEMPTY_SHIFT          21                                               /**< Shift value for SEMAILBOX_RXEMPTY           */
#define _SEMAILBOX_RX_STATUS_RXEMPTY_MASK           0x200000UL                                       /**< Bit mask for SEMAILBOX_RXEMPTY              */
#define _SEMAILBOX_RX_STATUS_RXEMPTY_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_RXEMPTY_DEFAULT         (_SEMAILBOX_RX_STATUS_RXEMPTY_DEFAULT << 21)     /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/
#define SEMAILBOX_RX_STATUS_RXHEADER                (0x1UL << 22)                                    /**< RXHEADER                                    */
#define _SEMAILBOX_RX_STATUS_RXHEADER_SHIFT         22                                               /**< Shift value for SEMAILBOX_RXHEADER          */
#define _SEMAILBOX_RX_STATUS_RXHEADER_MASK          0x400000UL                                       /**< Bit mask for SEMAILBOX_RXHEADER             */
#define _SEMAILBOX_RX_STATUS_RXHEADER_DEFAULT       0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_RXHEADER_DEFAULT        (_SEMAILBOX_RX_STATUS_RXHEADER_DEFAULT << 22)    /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/
#define SEMAILBOX_RX_STATUS_RXERROR                 (0x1UL << 23)                                    /**< RXERROR                                     */
#define _SEMAILBOX_RX_STATUS_RXERROR_SHIFT          23                                               /**< Shift value for SEMAILBOX_RXERROR           */
#define _SEMAILBOX_RX_STATUS_RXERROR_MASK           0x800000UL                                       /**< Bit mask for SEMAILBOX_RXERROR              */
#define _SEMAILBOX_RX_STATUS_RXERROR_DEFAULT        0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_RXERROR_DEFAULT         (_SEMAILBOX_RX_STATUS_RXERROR_DEFAULT << 23)     /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/
#define SEMAILBOX_RX_STATUS_UNPROTECTED             (0x1UL << 24)                                    /**< UNPROTECTED                                 */
#define _SEMAILBOX_RX_STATUS_UNPROTECTED_SHIFT      24                                               /**< Shift value for SEMAILBOX_UNPROTECTED       */
#define _SEMAILBOX_RX_STATUS_UNPROTECTED_MASK       0x1000000UL                                      /**< Bit mask for SEMAILBOX_UNPROTECTED          */
#define _SEMAILBOX_RX_STATUS_UNPROTECTED_DEFAULT    0x00000000UL                                     /**< Mode DEFAULT for SEMAILBOX_RX_STATUS        */
#define SEMAILBOX_RX_STATUS_UNPROTECTED_DEFAULT     (_SEMAILBOX_RX_STATUS_UNPROTECTED_DEFAULT << 24) /**< Shifted mode DEFAULT for SEMAILBOX_RX_STATUS*/

/* Bit fields for SEMAILBOX TX_PROT */
#define _SEMAILBOX_TX_PROT_RESETVALUE               0x00000000UL                              /**< Default value for SEMAILBOX_TX_PROT         */
#define _SEMAILBOX_TX_PROT_MASK                     0xFFFFFFFFUL                              /**< Mask for SEMAILBOX_TX_PROT                  */
#define _SEMAILBOX_TX_PROT_USER_SHIFT               0                                         /**< Shift value for SEMAILBOX_USER              */
#define _SEMAILBOX_TX_PROT_USER_MASK                0x3FFFFFFFUL                              /**< Bit mask for SEMAILBOX_USER                 */
#define _SEMAILBOX_TX_PROT_USER_DEFAULT             0x00000000UL                              /**< Mode DEFAULT for SEMAILBOX_TX_PROT          */
#define SEMAILBOX_TX_PROT_USER_DEFAULT              (_SEMAILBOX_TX_PROT_USER_DEFAULT << 0)    /**< Shifted mode DEFAULT for SEMAILBOX_TX_PROT  */
#define SEMAILBOX_TX_PROT_PRIV                      (0x1UL << 30)                             /**< PRIV                                        */
#define _SEMAILBOX_TX_PROT_PRIV_SHIFT               30                                        /**< Shift value for SEMAILBOX_PRIV              */
#define _SEMAILBOX_TX_PROT_PRIV_MASK                0x40000000UL                              /**< Bit mask for SEMAILBOX_PRIV                 */
#define _SEMAILBOX_TX_PROT_PRIV_DEFAULT             0x00000000UL                              /**< Mode DEFAULT for SEMAILBOX_TX_PROT          */
#define SEMAILBOX_TX_PROT_PRIV_DEFAULT              (_SEMAILBOX_TX_PROT_PRIV_DEFAULT << 30)   /**< Shifted mode DEFAULT for SEMAILBOX_TX_PROT  */
#define SEMAILBOX_TX_PROT_NONSEC                    (0x1UL << 31)                             /**< NONSEC                                      */
#define _SEMAILBOX_TX_PROT_NONSEC_SHIFT             31                                        /**< Shift value for SEMAILBOX_NONSEC            */
#define _SEMAILBOX_TX_PROT_NONSEC_MASK              0x80000000UL                              /**< Bit mask for SEMAILBOX_NONSEC               */
#define _SEMAILBOX_TX_PROT_NONSEC_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for SEMAILBOX_TX_PROT          */
#define SEMAILBOX_TX_PROT_NONSEC_DEFAULT            (_SEMAILBOX_TX_PROT_NONSEC_DEFAULT << 31) /**< Shifted mode DEFAULT for SEMAILBOX_TX_PROT  */

/* Bit fields for SEMAILBOX RX_PROT */
#define _SEMAILBOX_RX_PROT_RESETVALUE               0x00000000UL                              /**< Default value for SEMAILBOX_RX_PROT         */
#define _SEMAILBOX_RX_PROT_MASK                     0xFFFFFFFFUL                              /**< Mask for SEMAILBOX_RX_PROT                  */
#define _SEMAILBOX_RX_PROT_USER_SHIFT               0                                         /**< Shift value for SEMAILBOX_USER              */
#define _SEMAILBOX_RX_PROT_USER_MASK                0x3FFFFFFFUL                              /**< Bit mask for SEMAILBOX_USER                 */
#define _SEMAILBOX_RX_PROT_USER_DEFAULT             0x00000000UL                              /**< Mode DEFAULT for SEMAILBOX_RX_PROT          */
#define SEMAILBOX_RX_PROT_USER_DEFAULT              (_SEMAILBOX_RX_PROT_USER_DEFAULT << 0)    /**< Shifted mode DEFAULT for SEMAILBOX_RX_PROT  */
#define SEMAILBOX_RX_PROT_PRIV                      (0x1UL << 30)                             /**< PRIV                                        */
#define _SEMAILBOX_RX_PROT_PRIV_SHIFT               30                                        /**< Shift value for SEMAILBOX_PRIV              */
#define _SEMAILBOX_RX_PROT_PRIV_MASK                0x40000000UL                              /**< Bit mask for SEMAILBOX_PRIV                 */
#define _SEMAILBOX_RX_PROT_PRIV_DEFAULT             0x00000000UL                              /**< Mode DEFAULT for SEMAILBOX_RX_PROT          */
#define SEMAILBOX_RX_PROT_PRIV_DEFAULT              (_SEMAILBOX_RX_PROT_PRIV_DEFAULT << 30)   /**< Shifted mode DEFAULT for SEMAILBOX_RX_PROT  */
#define SEMAILBOX_RX_PROT_NONSEC                    (0x1UL << 31)                             /**< NONSEC                                      */
#define _SEMAILBOX_RX_PROT_NONSEC_SHIFT             31                                        /**< Shift value for SEMAILBOX_NONSEC            */
#define _SEMAILBOX_RX_PROT_NONSEC_MASK              0x80000000UL                              /**< Bit mask for SEMAILBOX_NONSEC               */
#define _SEMAILBOX_RX_PROT_NONSEC_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for SEMAILBOX_RX_PROT          */
#define SEMAILBOX_RX_PROT_NONSEC_DEFAULT            (_SEMAILBOX_RX_PROT_NONSEC_DEFAULT << 31) /**< Shifted mode DEFAULT for SEMAILBOX_RX_PROT  */

/* Bit fields for SEMAILBOX TX_HEADER */
#define _SEMAILBOX_TX_HEADER_RESETVALUE             0x00000000UL                                 /**< Default value for SEMAILBOX_TX_HEADER       */
#define _SEMAILBOX_TX_HEADER_MASK                   0xFFFFFFFFUL                                 /**< Mask for SEMAILBOX_TX_HEADER                */
#define _SEMAILBOX_TX_HEADER_TXHEADER_SHIFT         0                                            /**< Shift value for SEMAILBOX_TXHEADER          */
#define _SEMAILBOX_TX_HEADER_TXHEADER_MASK          0xFFFFFFFFUL                                 /**< Bit mask for SEMAILBOX_TXHEADER             */
#define _SEMAILBOX_TX_HEADER_TXHEADER_DEFAULT       0x00000000UL                                 /**< Mode DEFAULT for SEMAILBOX_TX_HEADER        */
#define SEMAILBOX_TX_HEADER_TXHEADER_DEFAULT        (_SEMAILBOX_TX_HEADER_TXHEADER_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAILBOX_TX_HEADER*/

/* Bit fields for SEMAILBOX RX_HEADER */
#define _SEMAILBOX_RX_HEADER_RESETVALUE             0x00000000UL                                 /**< Default value for SEMAILBOX_RX_HEADER       */
#define _SEMAILBOX_RX_HEADER_MASK                   0xFFFFFFFFUL                                 /**< Mask for SEMAILBOX_RX_HEADER                */
#define _SEMAILBOX_RX_HEADER_RXHEADER_SHIFT         0                                            /**< Shift value for SEMAILBOX_RXHEADER          */
#define _SEMAILBOX_RX_HEADER_RXHEADER_MASK          0xFFFFFFFFUL                                 /**< Bit mask for SEMAILBOX_RXHEADER             */
#define _SEMAILBOX_RX_HEADER_RXHEADER_DEFAULT       0x00000000UL                                 /**< Mode DEFAULT for SEMAILBOX_RX_HEADER        */
#define SEMAILBOX_RX_HEADER_RXHEADER_DEFAULT        (_SEMAILBOX_RX_HEADER_RXHEADER_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAILBOX_RX_HEADER*/

/* Bit fields for SEMAILBOX CONFIGURATION */
#define _SEMAILBOX_CONFIGURATION_RESETVALUE         0x00000000UL                                    /**< Default value for SEMAILBOX_CONFIGURATION   */
#define _SEMAILBOX_CONFIGURATION_MASK               0x00000003UL                                    /**< Mask for SEMAILBOX_CONFIGURATION            */
#define SEMAILBOX_CONFIGURATION_TXINTEN             (0x1UL << 0)                                    /**< TXINTEN                                     */
#define _SEMAILBOX_CONFIGURATION_TXINTEN_SHIFT      0                                               /**< Shift value for SEMAILBOX_TXINTEN           */
#define _SEMAILBOX_CONFIGURATION_TXINTEN_MASK       0x1UL                                           /**< Bit mask for SEMAILBOX_TXINTEN              */
#define _SEMAILBOX_CONFIGURATION_TXINTEN_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for SEMAILBOX_CONFIGURATION    */
#define SEMAILBOX_CONFIGURATION_TXINTEN_DEFAULT     (_SEMAILBOX_CONFIGURATION_TXINTEN_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAILBOX_CONFIGURATION*/
#define SEMAILBOX_CONFIGURATION_RXINTEN             (0x1UL << 1)                                    /**< RXINTEN                                     */
#define _SEMAILBOX_CONFIGURATION_RXINTEN_SHIFT      1                                               /**< Shift value for SEMAILBOX_RXINTEN           */
#define _SEMAILBOX_CONFIGURATION_RXINTEN_MASK       0x2UL                                           /**< Bit mask for SEMAILBOX_RXINTEN              */
#define _SEMAILBOX_CONFIGURATION_RXINTEN_DEFAULT    0x00000000UL                                    /**< Mode DEFAULT for SEMAILBOX_CONFIGURATION    */
#define SEMAILBOX_CONFIGURATION_RXINTEN_DEFAULT     (_SEMAILBOX_CONFIGURATION_RXINTEN_DEFAULT << 1) /**< Shifted mode DEFAULT for SEMAILBOX_CONFIGURATION*/

/** @} End of group SIMG301_SEMAILBOX_HOST_BitFields */
/** @} End of group SIMG301_SEMAILBOX_HOST */
/**************************************************************************//**
 * @defgroup SIMG301_SEMAILBOX_APBSE SEMAILBOX_APBSE
 * @{
 * @brief SIMG301 SEMAILBOX_APBSE Register Declaration.
 *****************************************************************************/

/** SEMAILBOX_APBSE Register Declaration. */
typedef struct semailbox_apbse_typedef{
  uint32_t RESERVED0[1U];                            /**< Reserved for future use                            */
} SEMAILBOX_APBSE_TypeDef;
/** @} End of group SIMG301_SEMAILBOX_APBSE */

/**************************************************************************//**
 * @addtogroup SIMG301_SEMAILBOX_APBSE
 * @{
 * @defgroup SIMG301_SEMAILBOX_APBSE_BitFields SEMAILBOX_APBSE Bit Fields
 * @{
 *****************************************************************************/

/** @} End of group SIMG301_SEMAILBOX_APBSE_BitFields */
/** @} End of group SIMG301_SEMAILBOX_APBSE */
/** @} End of group Parts */

#endif // SIMG301_SEMAILBOX_H
