/**************************************************************************//**
 * @file
 * @brief SIBG301 L2CACHE register and bit field definitions
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
#ifndef SIBG301_L2CACHE_H
#define SIBG301_L2CACHE_H
#define L2CACHE_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_L2CACHE L2CACHE
 * @{
 * @brief SIBG301 L2CACHE Register Declaration.
 *****************************************************************************/

/** L2CACHE Register Declaration. */
typedef struct l2cache_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP Version                                         */
  __IOM uint32_t CTRL;                          /**< Control Register                                   */
  uint32_t       RESERVED0[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t PRIORITY;                      /**< Arbitration Priority                               */
  uint32_t       RESERVED1[1U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS;                        /**< Status Register                                    */
  __IOM uint32_t FLUSHCMD;                      /**< Flush Command Register                             */
  __IOM uint32_t TAMPERIF;                      /**< Tamper Interrupt Flag                              */
  __IOM uint32_t TAMPERIEN;                     /**< Tamper Interrupt Enable                            */
  __IOM uint32_t IF;                            /**< Interrupt Flag                                     */
  __IOM uint32_t IEN;                           /**< Interrupt Enable                                   */
  uint32_t       RESERVED2[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t FLUSHADDRTOP;                  /**< Flushrange Up Boundary                             */
  __IOM uint32_t FLUSHADDRBOT;                  /**< Flushrange Low Boundary                            */
  uint32_t       RESERVED3[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t PERFCMD;                       /**< Performance Command                                */
  __IOM uint32_t PERFSEL;                       /**< Performance Register Sel                           */
  __IOM uint32_t LINECOUNTSEL;                  /**< LineCount Register34 Sel                           */
  uint32_t       RESERVED4[5U];                 /**< Reserved for future use                            */
  __IM uint32_t  PC0HITS;                       /**< Performance Hit Register of Master 0               */
  __IM uint32_t  PC0MISSES;                     /**< Performance Miss Register of Master 0              */
  __IM uint32_t  PC1HITS;                       /**< Performance Hit Register of Master 1               */
  __IM uint32_t  PC1MISSES;                     /**< Performance Miss Register of Master 1              */
  __IM uint32_t  PCALLHITS;                     /**< PC Hit Register of all masters                     */
  __IM uint32_t  PCALLMISSES;                   /**< PC Miss Register of all Masters                    */
  uint32_t       RESERVED5[2U];                 /**< Reserved for future use                            */
  __IM uint32_t  MASTER0LC;                     /**< Master0 Number of Cache Line                       */
  __IM uint32_t  MASTER1LC;                     /**< Master1 Number of Cache Line                       */
  __IM uint32_t  MASTER2LC;                     /**< Master2 Number of Cache Line                       */
  __IM uint32_t  MASTER3LC;                     /**< Master3 Number of Cache Line                       */
  __IM uint32_t  MASTER4LC;                     /**< Master4 Number of Cache Line                       */
  __IM uint32_t  MASTERSLC;                     /**< MasterS Number of Cache Line                       */
  uint32_t       RESERVED6[985U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP Version                                         */
  __IOM uint32_t CTRL_SET;                      /**< Control Register                                   */
  uint32_t       RESERVED7[1U];                 /**< Reserved for future use                            */
  __IOM uint32_t PRIORITY_SET;                  /**< Arbitration Priority                               */
  uint32_t       RESERVED8[1U];                 /**< Reserved for future use                            */
  __IM uint32_t  STATUS_SET;                    /**< Status Register                                    */
  __IOM uint32_t FLUSHCMD_SET;                  /**< Flush Command Register                             */
  __IOM uint32_t TAMPERIF_SET;                  /**< Tamper Interrupt Flag                              */
  __IOM uint32_t TAMPERIEN_SET;                 /**< Tamper Interrupt Enable                            */
  __IOM uint32_t IF_SET;                        /**< Interrupt Flag                                     */
  __IOM uint32_t IEN_SET;                       /**< Interrupt Enable                                   */
  uint32_t       RESERVED9[2U];                 /**< Reserved for future use                            */
  __IOM uint32_t FLUSHADDRTOP_SET;              /**< Flushrange Up Boundary                             */
  __IOM uint32_t FLUSHADDRBOT_SET;              /**< Flushrange Low Boundary                            */
  uint32_t       RESERVED10[2U];                /**< Reserved for future use                            */
  __IOM uint32_t PERFCMD_SET;                   /**< Performance Command                                */
  __IOM uint32_t PERFSEL_SET;                   /**< Performance Register Sel                           */
  __IOM uint32_t LINECOUNTSEL_SET;              /**< LineCount Register34 Sel                           */
  uint32_t       RESERVED11[5U];                /**< Reserved for future use                            */
  __IM uint32_t  PC0HITS_SET;                   /**< Performance Hit Register of Master 0               */
  __IM uint32_t  PC0MISSES_SET;                 /**< Performance Miss Register of Master 0              */
  __IM uint32_t  PC1HITS_SET;                   /**< Performance Hit Register of Master 1               */
  __IM uint32_t  PC1MISSES_SET;                 /**< Performance Miss Register of Master 1              */
  __IM uint32_t  PCALLHITS_SET;                 /**< PC Hit Register of all masters                     */
  __IM uint32_t  PCALLMISSES_SET;               /**< PC Miss Register of all Masters                    */
  uint32_t       RESERVED12[2U];                /**< Reserved for future use                            */
  __IM uint32_t  MASTER0LC_SET;                 /**< Master0 Number of Cache Line                       */
  __IM uint32_t  MASTER1LC_SET;                 /**< Master1 Number of Cache Line                       */
  __IM uint32_t  MASTER2LC_SET;                 /**< Master2 Number of Cache Line                       */
  __IM uint32_t  MASTER3LC_SET;                 /**< Master3 Number of Cache Line                       */
  __IM uint32_t  MASTER4LC_SET;                 /**< Master4 Number of Cache Line                       */
  __IM uint32_t  MASTERSLC_SET;                 /**< MasterS Number of Cache Line                       */
  uint32_t       RESERVED13[985U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP Version                                         */
  __IOM uint32_t CTRL_CLR;                      /**< Control Register                                   */
  uint32_t       RESERVED14[1U];                /**< Reserved for future use                            */
  __IOM uint32_t PRIORITY_CLR;                  /**< Arbitration Priority                               */
  uint32_t       RESERVED15[1U];                /**< Reserved for future use                            */
  __IM uint32_t  STATUS_CLR;                    /**< Status Register                                    */
  __IOM uint32_t FLUSHCMD_CLR;                  /**< Flush Command Register                             */
  __IOM uint32_t TAMPERIF_CLR;                  /**< Tamper Interrupt Flag                              */
  __IOM uint32_t TAMPERIEN_CLR;                 /**< Tamper Interrupt Enable                            */
  __IOM uint32_t IF_CLR;                        /**< Interrupt Flag                                     */
  __IOM uint32_t IEN_CLR;                       /**< Interrupt Enable                                   */
  uint32_t       RESERVED16[2U];                /**< Reserved for future use                            */
  __IOM uint32_t FLUSHADDRTOP_CLR;              /**< Flushrange Up Boundary                             */
  __IOM uint32_t FLUSHADDRBOT_CLR;              /**< Flushrange Low Boundary                            */
  uint32_t       RESERVED17[2U];                /**< Reserved for future use                            */
  __IOM uint32_t PERFCMD_CLR;                   /**< Performance Command                                */
  __IOM uint32_t PERFSEL_CLR;                   /**< Performance Register Sel                           */
  __IOM uint32_t LINECOUNTSEL_CLR;              /**< LineCount Register34 Sel                           */
  uint32_t       RESERVED18[5U];                /**< Reserved for future use                            */
  __IM uint32_t  PC0HITS_CLR;                   /**< Performance Hit Register of Master 0               */
  __IM uint32_t  PC0MISSES_CLR;                 /**< Performance Miss Register of Master 0              */
  __IM uint32_t  PC1HITS_CLR;                   /**< Performance Hit Register of Master 1               */
  __IM uint32_t  PC1MISSES_CLR;                 /**< Performance Miss Register of Master 1              */
  __IM uint32_t  PCALLHITS_CLR;                 /**< PC Hit Register of all masters                     */
  __IM uint32_t  PCALLMISSES_CLR;               /**< PC Miss Register of all Masters                    */
  uint32_t       RESERVED19[2U];                /**< Reserved for future use                            */
  __IM uint32_t  MASTER0LC_CLR;                 /**< Master0 Number of Cache Line                       */
  __IM uint32_t  MASTER1LC_CLR;                 /**< Master1 Number of Cache Line                       */
  __IM uint32_t  MASTER2LC_CLR;                 /**< Master2 Number of Cache Line                       */
  __IM uint32_t  MASTER3LC_CLR;                 /**< Master3 Number of Cache Line                       */
  __IM uint32_t  MASTER4LC_CLR;                 /**< Master4 Number of Cache Line                       */
  __IM uint32_t  MASTERSLC_CLR;                 /**< MasterS Number of Cache Line                       */
  uint32_t       RESERVED20[985U];              /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP Version                                         */
  __IOM uint32_t CTRL_TGL;                      /**< Control Register                                   */
  uint32_t       RESERVED21[1U];                /**< Reserved for future use                            */
  __IOM uint32_t PRIORITY_TGL;                  /**< Arbitration Priority                               */
  uint32_t       RESERVED22[1U];                /**< Reserved for future use                            */
  __IM uint32_t  STATUS_TGL;                    /**< Status Register                                    */
  __IOM uint32_t FLUSHCMD_TGL;                  /**< Flush Command Register                             */
  __IOM uint32_t TAMPERIF_TGL;                  /**< Tamper Interrupt Flag                              */
  __IOM uint32_t TAMPERIEN_TGL;                 /**< Tamper Interrupt Enable                            */
  __IOM uint32_t IF_TGL;                        /**< Interrupt Flag                                     */
  __IOM uint32_t IEN_TGL;                       /**< Interrupt Enable                                   */
  uint32_t       RESERVED23[2U];                /**< Reserved for future use                            */
  __IOM uint32_t FLUSHADDRTOP_TGL;              /**< Flushrange Up Boundary                             */
  __IOM uint32_t FLUSHADDRBOT_TGL;              /**< Flushrange Low Boundary                            */
  uint32_t       RESERVED24[2U];                /**< Reserved for future use                            */
  __IOM uint32_t PERFCMD_TGL;                   /**< Performance Command                                */
  __IOM uint32_t PERFSEL_TGL;                   /**< Performance Register Sel                           */
  __IOM uint32_t LINECOUNTSEL_TGL;              /**< LineCount Register34 Sel                           */
  uint32_t       RESERVED25[5U];                /**< Reserved for future use                            */
  __IM uint32_t  PC0HITS_TGL;                   /**< Performance Hit Register of Master 0               */
  __IM uint32_t  PC0MISSES_TGL;                 /**< Performance Miss Register of Master 0              */
  __IM uint32_t  PC1HITS_TGL;                   /**< Performance Hit Register of Master 1               */
  __IM uint32_t  PC1MISSES_TGL;                 /**< Performance Miss Register of Master 1              */
  __IM uint32_t  PCALLHITS_TGL;                 /**< PC Hit Register of all masters                     */
  __IM uint32_t  PCALLMISSES_TGL;               /**< PC Miss Register of all Masters                    */
  uint32_t       RESERVED26[2U];                /**< Reserved for future use                            */
  __IM uint32_t  MASTER0LC_TGL;                 /**< Master0 Number of Cache Line                       */
  __IM uint32_t  MASTER1LC_TGL;                 /**< Master1 Number of Cache Line                       */
  __IM uint32_t  MASTER2LC_TGL;                 /**< Master2 Number of Cache Line                       */
  __IM uint32_t  MASTER3LC_TGL;                 /**< Master3 Number of Cache Line                       */
  __IM uint32_t  MASTER4LC_TGL;                 /**< Master4 Number of Cache Line                       */
  __IM uint32_t  MASTERSLC_TGL;                 /**< MasterS Number of Cache Line                       */
} L2CACHE_TypeDef;
/** @} End of group SIBG301_L2CACHE */

/**************************************************************************//**
 * @addtogroup SIBG301_L2CACHE
 * @{
 * @defgroup SIBG301_L2CACHE_BitFields L2CACHE Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for L2CACHE IPVERSION */
#define _L2CACHE_IPVERSION_RESETVALUE                0x00000000UL                                /**< Default value for L2CACHE_IPVERSION         */
#define _L2CACHE_IPVERSION_MASK                      0xFFFFFFFFUL                                /**< Mask for L2CACHE_IPVERSION                  */
#define _L2CACHE_IPVERSION_IPVERSION_SHIFT           0                                           /**< Shift value for L2CACHE_IPVERSION           */
#define _L2CACHE_IPVERSION_IPVERSION_MASK            0xFFFFFFFFUL                                /**< Bit mask for L2CACHE_IPVERSION              */
#define _L2CACHE_IPVERSION_IPVERSION_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for L2CACHE_IPVERSION          */
#define L2CACHE_IPVERSION_IPVERSION_DEFAULT          (_L2CACHE_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_IPVERSION  */

/* Bit fields for L2CACHE CTRL */
#define _L2CACHE_CTRL_RESETVALUE                     0x00000000UL                              /**< Default value for L2CACHE_CTRL              */
#define _L2CACHE_CTRL_MASK                           0x00000011UL                              /**< Mask for L2CACHE_CTRL                       */
#define L2CACHE_CTRL_DATACACHEDIS                    (0x1UL << 0)                              /**< Data cache disable                          */
#define _L2CACHE_CTRL_DATACACHEDIS_SHIFT             0                                         /**< Shift value for L2CACHE_DATACACHEDIS        */
#define _L2CACHE_CTRL_DATACACHEDIS_MASK              0x1UL                                     /**< Bit mask for L2CACHE_DATACACHEDIS           */
#define _L2CACHE_CTRL_DATACACHEDIS_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for L2CACHE_CTRL               */
#define L2CACHE_CTRL_DATACACHEDIS_DEFAULT            (_L2CACHE_CTRL_DATACACHEDIS_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_CTRL       */
#define L2CACHE_CTRL_RAMRETAINDIS                    (0x1UL << 4)                              /**< RAM retain disable                          */
#define _L2CACHE_CTRL_RAMRETAINDIS_SHIFT             4                                         /**< Shift value for L2CACHE_RAMRETAINDIS        */
#define _L2CACHE_CTRL_RAMRETAINDIS_MASK              0x10UL                                    /**< Bit mask for L2CACHE_RAMRETAINDIS           */
#define _L2CACHE_CTRL_RAMRETAINDIS_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for L2CACHE_CTRL               */
#define L2CACHE_CTRL_RAMRETAINDIS_DEFAULT            (_L2CACHE_CTRL_RAMRETAINDIS_DEFAULT << 4) /**< Shifted mode DEFAULT for L2CACHE_CTRL       */

/* Bit fields for L2CACHE PRIORITY */
#define _L2CACHE_PRIORITY_RESETVALUE                 0x00000000UL                            /**< Default value for L2CACHE_PRIORITY          */
#define _L2CACHE_PRIORITY_MASK                       0x000000FFUL                            /**< Mask for L2CACHE_PRIORITY                   */
#define _L2CACHE_PRIORITY_MASTER_SHIFT               0                                       /**< Shift value for L2CACHE_MASTER              */
#define _L2CACHE_PRIORITY_MASTER_MASK                0xFFUL                                  /**< Bit mask for L2CACHE_MASTER                 */
#define _L2CACHE_PRIORITY_MASTER_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_NONE                0x00000000UL                            /**< Mode NONE for L2CACHE_PRIORITY              */
#define _L2CACHE_PRIORITY_MASTER_MASTER0             0x00000001UL                            /**< Mode MASTER0 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER1             0x00000002UL                            /**< Mode MASTER1 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER2             0x00000003UL                            /**< Mode MASTER2 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER3             0x00000004UL                            /**< Mode MASTER3 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER4             0x00000005UL                            /**< Mode MASTER4 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER5             0x00000006UL                            /**< Mode MASTER5 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER6             0x00000007UL                            /**< Mode MASTER6 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER7             0x00000008UL                            /**< Mode MASTER7 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER8             0x00000009UL                            /**< Mode MASTER8 for L2CACHE_PRIORITY           */
#define _L2CACHE_PRIORITY_MASTER_MASTER9             0x0000000AUL                            /**< Mode MASTER9 for L2CACHE_PRIORITY           */
#define L2CACHE_PRIORITY_MASTER_DEFAULT              (_L2CACHE_PRIORITY_MASTER_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_NONE                 (_L2CACHE_PRIORITY_MASTER_NONE << 0)    /**< Shifted mode NONE for L2CACHE_PRIORITY      */
#define L2CACHE_PRIORITY_MASTER_MASTER0              (_L2CACHE_PRIORITY_MASTER_MASTER0 << 0) /**< Shifted mode MASTER0 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER1              (_L2CACHE_PRIORITY_MASTER_MASTER1 << 0) /**< Shifted mode MASTER1 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER2              (_L2CACHE_PRIORITY_MASTER_MASTER2 << 0) /**< Shifted mode MASTER2 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER3              (_L2CACHE_PRIORITY_MASTER_MASTER3 << 0) /**< Shifted mode MASTER3 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER4              (_L2CACHE_PRIORITY_MASTER_MASTER4 << 0) /**< Shifted mode MASTER4 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER5              (_L2CACHE_PRIORITY_MASTER_MASTER5 << 0) /**< Shifted mode MASTER5 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER6              (_L2CACHE_PRIORITY_MASTER_MASTER6 << 0) /**< Shifted mode MASTER6 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER7              (_L2CACHE_PRIORITY_MASTER_MASTER7 << 0) /**< Shifted mode MASTER7 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER8              (_L2CACHE_PRIORITY_MASTER_MASTER8 << 0) /**< Shifted mode MASTER8 for L2CACHE_PRIORITY   */
#define L2CACHE_PRIORITY_MASTER_MASTER9              (_L2CACHE_PRIORITY_MASTER_MASTER9 << 0) /**< Shifted mode MASTER9 for L2CACHE_PRIORITY   */

/* Bit fields for L2CACHE STATUS */
#define _L2CACHE_STATUS_RESETVALUE                   0x00000000UL                                /**< Default value for L2CACHE_STATUS            */
#define _L2CACHE_STATUS_MASK                         0x00000017UL                                /**< Mask for L2CACHE_STATUS                     */
#define L2CACHE_STATUS_PC0RUNNING                    (0x1UL << 0)                                /**< PC0 Running                                 */
#define _L2CACHE_STATUS_PC0RUNNING_SHIFT             0                                           /**< Shift value for L2CACHE_PC0RUNNING          */
#define _L2CACHE_STATUS_PC0RUNNING_MASK              0x1UL                                       /**< Bit mask for L2CACHE_PC0RUNNING             */
#define _L2CACHE_STATUS_PC0RUNNING_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for L2CACHE_STATUS             */
#define L2CACHE_STATUS_PC0RUNNING_DEFAULT            (_L2CACHE_STATUS_PC0RUNNING_DEFAULT << 0)   /**< Shifted mode DEFAULT for L2CACHE_STATUS     */
#define L2CACHE_STATUS_PC1RUNNING                    (0x1UL << 1)                                /**< PC1 Running                                 */
#define _L2CACHE_STATUS_PC1RUNNING_SHIFT             1                                           /**< Shift value for L2CACHE_PC1RUNNING          */
#define _L2CACHE_STATUS_PC1RUNNING_MASK              0x2UL                                       /**< Bit mask for L2CACHE_PC1RUNNING             */
#define _L2CACHE_STATUS_PC1RUNNING_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for L2CACHE_STATUS             */
#define L2CACHE_STATUS_PC1RUNNING_DEFAULT            (_L2CACHE_STATUS_PC1RUNNING_DEFAULT << 1)   /**< Shifted mode DEFAULT for L2CACHE_STATUS     */
#define L2CACHE_STATUS_PCALLRUNNING                  (0x1UL << 2)                                /**< PCALL Running                               */
#define _L2CACHE_STATUS_PCALLRUNNING_SHIFT           2                                           /**< Shift value for L2CACHE_PCALLRUNNING        */
#define _L2CACHE_STATUS_PCALLRUNNING_MASK            0x4UL                                       /**< Bit mask for L2CACHE_PCALLRUNNING           */
#define _L2CACHE_STATUS_PCALLRUNNING_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for L2CACHE_STATUS             */
#define L2CACHE_STATUS_PCALLRUNNING_DEFAULT          (_L2CACHE_STATUS_PCALLRUNNING_DEFAULT << 2) /**< Shifted mode DEFAULT for L2CACHE_STATUS     */
#define L2CACHE_STATUS_FLUSHRUNNING                  (0x1UL << 4)                                /**< FLUSH Running                               */
#define _L2CACHE_STATUS_FLUSHRUNNING_SHIFT           4                                           /**< Shift value for L2CACHE_FLUSHRUNNING        */
#define _L2CACHE_STATUS_FLUSHRUNNING_MASK            0x10UL                                      /**< Bit mask for L2CACHE_FLUSHRUNNING           */
#define _L2CACHE_STATUS_FLUSHRUNNING_DEFAULT         0x00000000UL                                /**< Mode DEFAULT for L2CACHE_STATUS             */
#define L2CACHE_STATUS_FLUSHRUNNING_DEFAULT          (_L2CACHE_STATUS_FLUSHRUNNING_DEFAULT << 4) /**< Shifted mode DEFAULT for L2CACHE_STATUS     */

/* Bit fields for L2CACHE FLUSHCMD */
#define _L2CACHE_FLUSHCMD_RESETVALUE                 0x00000000UL                              /**< Default value for L2CACHE_FLUSHCMD          */
#define _L2CACHE_FLUSHCMD_MASK                       0x00000003UL                              /**< Mask for L2CACHE_FLUSHCMD                   */
#define L2CACHE_FLUSHCMD_FLUSHALL                    (0x1UL << 0)                              /**< Flush All Cache Lines                       */
#define _L2CACHE_FLUSHCMD_FLUSHALL_SHIFT             0                                         /**< Shift value for L2CACHE_FLUSHALL            */
#define _L2CACHE_FLUSHCMD_FLUSHALL_MASK              0x1UL                                     /**< Bit mask for L2CACHE_FLUSHALL               */
#define _L2CACHE_FLUSHCMD_FLUSHALL_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for L2CACHE_FLUSHCMD           */
#define L2CACHE_FLUSHCMD_FLUSHALL_DEFAULT            (_L2CACHE_FLUSHCMD_FLUSHALL_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_FLUSHCMD   */

/* Bit fields for L2CACHE TAMPERIF */
#define _L2CACHE_TAMPERIF_RESETVALUE                 0x00000000UL                                    /**< Default value for L2CACHE_TAMPERIF          */
#define _L2CACHE_TAMPERIF_MASK                       0x00000003UL                                    /**< Mask for L2CACHE_TAMPERIF                   */
#define L2CACHE_TAMPERIF_DATARAMERRORSE              (0x1UL << 0)                                    /**< DATARAM error Interrupt Flag                */
#define _L2CACHE_TAMPERIF_DATARAMERRORSE_SHIFT       0                                               /**< Shift value for L2CACHE_DATARAMERRORSE      */
#define _L2CACHE_TAMPERIF_DATARAMERRORSE_MASK        0x1UL                                           /**< Bit mask for L2CACHE_DATARAMERRORSE         */
#define _L2CACHE_TAMPERIF_DATARAMERRORSE_DEFAULT     0x00000000UL                                    /**< Mode DEFAULT for L2CACHE_TAMPERIF           */
#define L2CACHE_TAMPERIF_DATARAMERRORSE_DEFAULT      (_L2CACHE_TAMPERIF_DATARAMERRORSE_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_TAMPERIF   */
#define L2CACHE_TAMPERIF_TAGRAMERRORSE               (0x1UL << 1)                                    /**< TAGRAM error Interrupt Flag                 */
#define _L2CACHE_TAMPERIF_TAGRAMERRORSE_SHIFT        1                                               /**< Shift value for L2CACHE_TAGRAMERRORSE       */
#define _L2CACHE_TAMPERIF_TAGRAMERRORSE_MASK         0x2UL                                           /**< Bit mask for L2CACHE_TAGRAMERRORSE          */
#define _L2CACHE_TAMPERIF_TAGRAMERRORSE_DEFAULT      0x00000000UL                                    /**< Mode DEFAULT for L2CACHE_TAMPERIF           */
#define L2CACHE_TAMPERIF_TAGRAMERRORSE_DEFAULT       (_L2CACHE_TAMPERIF_TAGRAMERRORSE_DEFAULT << 1)  /**< Shifted mode DEFAULT for L2CACHE_TAMPERIF   */

/* Bit fields for L2CACHE TAMPERIEN */
#define _L2CACHE_TAMPERIEN_RESETVALUE                0x00000000UL                                     /**< Default value for L2CACHE_TAMPERIEN         */
#define _L2CACHE_TAMPERIEN_MASK                      0x00000003UL                                     /**< Mask for L2CACHE_TAMPERIEN                  */
#define L2CACHE_TAMPERIEN_DATARAMERRORSE             (0x1UL << 0)                                     /**< RAM error Interrupt Enable                  */
#define _L2CACHE_TAMPERIEN_DATARAMERRORSE_SHIFT      0                                                /**< Shift value for L2CACHE_DATARAMERRORSE      */
#define _L2CACHE_TAMPERIEN_DATARAMERRORSE_MASK       0x1UL                                            /**< Bit mask for L2CACHE_DATARAMERRORSE         */
#define _L2CACHE_TAMPERIEN_DATARAMERRORSE_DEFAULT    0x00000000UL                                     /**< Mode DEFAULT for L2CACHE_TAMPERIEN          */
#define L2CACHE_TAMPERIEN_DATARAMERRORSE_DEFAULT     (_L2CACHE_TAMPERIEN_DATARAMERRORSE_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_TAMPERIEN  */
#define L2CACHE_TAMPERIEN_TAGRAMERRORSE              (0x1UL << 1)                                     /**< TAGRAM error Interrupt Enable               */
#define _L2CACHE_TAMPERIEN_TAGRAMERRORSE_SHIFT       1                                                /**< Shift value for L2CACHE_TAGRAMERRORSE       */
#define _L2CACHE_TAMPERIEN_TAGRAMERRORSE_MASK        0x2UL                                            /**< Bit mask for L2CACHE_TAGRAMERRORSE          */
#define _L2CACHE_TAMPERIEN_TAGRAMERRORSE_DEFAULT     0x00000000UL                                     /**< Mode DEFAULT for L2CACHE_TAMPERIEN          */
#define L2CACHE_TAMPERIEN_TAGRAMERRORSE_DEFAULT      (_L2CACHE_TAMPERIEN_TAGRAMERRORSE_DEFAULT << 1)  /**< Shifted mode DEFAULT for L2CACHE_TAMPERIEN  */

/* Bit fields for L2CACHE IF */
#define _L2CACHE_IF_RESETVALUE                       0x00000000UL                               /**< Default value for L2CACHE_IF                */
#define _L2CACHE_IF_MASK                             0x000003F3UL                               /**< Mask for L2CACHE_IF                         */
#define L2CACHE_IF_DATARAMERROR                      (0x1UL << 0)                               /**< DATA RAM Error Interrupt Flag               */
#define _L2CACHE_IF_DATARAMERROR_SHIFT               0                                          /**< Shift value for L2CACHE_DATARAMERROR        */
#define _L2CACHE_IF_DATARAMERROR_MASK                0x1UL                                      /**< Bit mask for L2CACHE_DATARAMERROR           */
#define _L2CACHE_IF_DATARAMERROR_DEFAULT             0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_DATARAMERROR_DEFAULT              (_L2CACHE_IF_DATARAMERROR_DEFAULT << 0)    /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_TAGRAMERROR                       (0x1UL << 1)                               /**< TAG RAM Error Interrupt Flag                */
#define _L2CACHE_IF_TAGRAMERROR_SHIFT                1                                          /**< Shift value for L2CACHE_TAGRAMERROR         */
#define _L2CACHE_IF_TAGRAMERROR_MASK                 0x2UL                                      /**< Bit mask for L2CACHE_TAGRAMERROR            */
#define _L2CACHE_IF_TAGRAMERROR_DEFAULT              0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_TAGRAMERROR_DEFAULT               (_L2CACHE_IF_TAGRAMERROR_DEFAULT << 1)     /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_PC0HITSOF                         (0x1UL << 4)                               /**< PC0HITS Overflow Interrupt Flag             */
#define _L2CACHE_IF_PC0HITSOF_SHIFT                  4                                          /**< Shift value for L2CACHE_PC0HITSOF           */
#define _L2CACHE_IF_PC0HITSOF_MASK                   0x10UL                                     /**< Bit mask for L2CACHE_PC0HITSOF              */
#define _L2CACHE_IF_PC0HITSOF_DEFAULT                0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_PC0HITSOF_DEFAULT                 (_L2CACHE_IF_PC0HITSOF_DEFAULT << 4)       /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_PC0MISSESOF                       (0x1UL << 5)                               /**< PC0MISSES Overflow Interrupt Flag           */
#define _L2CACHE_IF_PC0MISSESOF_SHIFT                5                                          /**< Shift value for L2CACHE_PC0MISSESOF         */
#define _L2CACHE_IF_PC0MISSESOF_MASK                 0x20UL                                     /**< Bit mask for L2CACHE_PC0MISSESOF            */
#define _L2CACHE_IF_PC0MISSESOF_DEFAULT              0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_PC0MISSESOF_DEFAULT               (_L2CACHE_IF_PC0MISSESOF_DEFAULT << 5)     /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_PC1HITSOF                         (0x1UL << 6)                               /**< PC1HITS Overflow Interrupt Flag             */
#define _L2CACHE_IF_PC1HITSOF_SHIFT                  6                                          /**< Shift value for L2CACHE_PC1HITSOF           */
#define _L2CACHE_IF_PC1HITSOF_MASK                   0x40UL                                     /**< Bit mask for L2CACHE_PC1HITSOF              */
#define _L2CACHE_IF_PC1HITSOF_DEFAULT                0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_PC1HITSOF_DEFAULT                 (_L2CACHE_IF_PC1HITSOF_DEFAULT << 6)       /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_PC1MISSESOF                       (0x1UL << 7)                               /**< PC1MISSES Overflow Interrupt Flag           */
#define _L2CACHE_IF_PC1MISSESOF_SHIFT                7                                          /**< Shift value for L2CACHE_PC1MISSESOF         */
#define _L2CACHE_IF_PC1MISSESOF_MASK                 0x80UL                                     /**< Bit mask for L2CACHE_PC1MISSESOF            */
#define _L2CACHE_IF_PC1MISSESOF_DEFAULT              0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_PC1MISSESOF_DEFAULT               (_L2CACHE_IF_PC1MISSESOF_DEFAULT << 7)     /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_PCALLHITSOFIF                     (0x1UL << 8)                               /**< PCALLHITS Overflow Interrupt Flag           */
#define _L2CACHE_IF_PCALLHITSOFIF_SHIFT              8                                          /**< Shift value for L2CACHE_PCALLHITSOFIF       */
#define _L2CACHE_IF_PCALLHITSOFIF_MASK               0x100UL                                    /**< Bit mask for L2CACHE_PCALLHITSOFIF          */
#define _L2CACHE_IF_PCALLHITSOFIF_DEFAULT            0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_PCALLHITSOFIF_DEFAULT             (_L2CACHE_IF_PCALLHITSOFIF_DEFAULT << 8)   /**< Shifted mode DEFAULT for L2CACHE_IF         */
#define L2CACHE_IF_PCALLMISSESOFIF                   (0x1UL << 9)                               /**< PCALLMISSES Overflow Interrupt Flag         */
#define _L2CACHE_IF_PCALLMISSESOFIF_SHIFT            9                                          /**< Shift value for L2CACHE_PCALLMISSESOFIF     */
#define _L2CACHE_IF_PCALLMISSESOFIF_MASK             0x200UL                                    /**< Bit mask for L2CACHE_PCALLMISSESOFIF        */
#define _L2CACHE_IF_PCALLMISSESOFIF_DEFAULT          0x00000000UL                               /**< Mode DEFAULT for L2CACHE_IF                 */
#define L2CACHE_IF_PCALLMISSESOFIF_DEFAULT           (_L2CACHE_IF_PCALLMISSESOFIF_DEFAULT << 9) /**< Shifted mode DEFAULT for L2CACHE_IF         */

/* Bit fields for L2CACHE IEN */
#define _L2CACHE_IEN_RESETVALUE                      0x00000000UL                                 /**< Default value for L2CACHE_IEN               */
#define _L2CACHE_IEN_MASK                            0x000003F3UL                                 /**< Mask for L2CACHE_IEN                        */
#define L2CACHE_IEN_DATARAMERROR                     (0x1UL << 0)                                 /**< DATA RAM Error Interrupt Enable             */
#define _L2CACHE_IEN_DATARAMERROR_SHIFT              0                                            /**< Shift value for L2CACHE_DATARAMERROR        */
#define _L2CACHE_IEN_DATARAMERROR_MASK               0x1UL                                        /**< Bit mask for L2CACHE_DATARAMERROR           */
#define _L2CACHE_IEN_DATARAMERROR_DEFAULT            0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_DATARAMERROR_DEFAULT             (_L2CACHE_IEN_DATARAMERROR_DEFAULT << 0)     /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_TAGRAMERROR                      (0x1UL << 1)                                 /**< TAG RAM Error Interrupt Enable              */
#define _L2CACHE_IEN_TAGRAMERROR_SHIFT               1                                            /**< Shift value for L2CACHE_TAGRAMERROR         */
#define _L2CACHE_IEN_TAGRAMERROR_MASK                0x2UL                                        /**< Bit mask for L2CACHE_TAGRAMERROR            */
#define _L2CACHE_IEN_TAGRAMERROR_DEFAULT             0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_TAGRAMERROR_DEFAULT              (_L2CACHE_IEN_TAGRAMERROR_DEFAULT << 1)      /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_PC0HITSOF                        (0x1UL << 4)                                 /**< PC0HITS Overflow Interrupt Enable           */
#define _L2CACHE_IEN_PC0HITSOF_SHIFT                 4                                            /**< Shift value for L2CACHE_PC0HITSOF           */
#define _L2CACHE_IEN_PC0HITSOF_MASK                  0x10UL                                       /**< Bit mask for L2CACHE_PC0HITSOF              */
#define _L2CACHE_IEN_PC0HITSOF_DEFAULT               0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_PC0HITSOF_DEFAULT                (_L2CACHE_IEN_PC0HITSOF_DEFAULT << 4)        /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_PC0MISSESOF                      (0x1UL << 5)                                 /**< PC0MISSES Overflow Interrupt Enable         */
#define _L2CACHE_IEN_PC0MISSESOF_SHIFT               5                                            /**< Shift value for L2CACHE_PC0MISSESOF         */
#define _L2CACHE_IEN_PC0MISSESOF_MASK                0x20UL                                       /**< Bit mask for L2CACHE_PC0MISSESOF            */
#define _L2CACHE_IEN_PC0MISSESOF_DEFAULT             0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_PC0MISSESOF_DEFAULT              (_L2CACHE_IEN_PC0MISSESOF_DEFAULT << 5)      /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_PC1HITSOF                        (0x1UL << 6)                                 /**< PC1HITS Overflow Interrupt Enable           */
#define _L2CACHE_IEN_PC1HITSOF_SHIFT                 6                                            /**< Shift value for L2CACHE_PC1HITSOF           */
#define _L2CACHE_IEN_PC1HITSOF_MASK                  0x40UL                                       /**< Bit mask for L2CACHE_PC1HITSOF              */
#define _L2CACHE_IEN_PC1HITSOF_DEFAULT               0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_PC1HITSOF_DEFAULT                (_L2CACHE_IEN_PC1HITSOF_DEFAULT << 6)        /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_PC1MISSESOF                      (0x1UL << 7)                                 /**< PC1MISSES Overflow Interrupt Enable         */
#define _L2CACHE_IEN_PC1MISSESOF_SHIFT               7                                            /**< Shift value for L2CACHE_PC1MISSESOF         */
#define _L2CACHE_IEN_PC1MISSESOF_MASK                0x80UL                                       /**< Bit mask for L2CACHE_PC1MISSESOF            */
#define _L2CACHE_IEN_PC1MISSESOF_DEFAULT             0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_PC1MISSESOF_DEFAULT              (_L2CACHE_IEN_PC1MISSESOF_DEFAULT << 7)      /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_PCALLHITSOFIEN                   (0x1UL << 8)                                 /**< PCALLHITS Overflow Interrupt Enable         */
#define _L2CACHE_IEN_PCALLHITSOFIEN_SHIFT            8                                            /**< Shift value for L2CACHE_PCALLHITSOFIEN      */
#define _L2CACHE_IEN_PCALLHITSOFIEN_MASK             0x100UL                                      /**< Bit mask for L2CACHE_PCALLHITSOFIEN         */
#define _L2CACHE_IEN_PCALLHITSOFIEN_DEFAULT          0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_PCALLHITSOFIEN_DEFAULT           (_L2CACHE_IEN_PCALLHITSOFIEN_DEFAULT << 8)   /**< Shifted mode DEFAULT for L2CACHE_IEN        */
#define L2CACHE_IEN_PCALLMISSESOFIEN                 (0x1UL << 9)                                 /**< PCALLMISSES Overflow Interrupt Enable       */
#define _L2CACHE_IEN_PCALLMISSESOFIEN_SHIFT          9                                            /**< Shift value for L2CACHE_PCALLMISSESOFIEN    */
#define _L2CACHE_IEN_PCALLMISSESOFIEN_MASK           0x200UL                                      /**< Bit mask for L2CACHE_PCALLMISSESOFIEN       */
#define _L2CACHE_IEN_PCALLMISSESOFIEN_DEFAULT        0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_IEN                */
#define L2CACHE_IEN_PCALLMISSESOFIEN_DEFAULT         (_L2CACHE_IEN_PCALLMISSESOFIEN_DEFAULT << 9) /**< Shifted mode DEFAULT for L2CACHE_IEN        */

/* Bit fields for L2CACHE FLUSHADDRTOP */
#define _L2CACHE_FLUSHADDRTOP_RESETVALUE             0x00000000UL                              /**< Default value for L2CACHE_FLUSHADDRTOP      */
#define _L2CACHE_FLUSHADDRTOP_MASK                   0xFFFFFFFFUL                              /**< Mask for L2CACHE_FLUSHADDRTOP               */
#define _L2CACHE_FLUSHADDRTOP_ADDR_SHIFT             0                                         /**< Shift value for L2CACHE_ADDR                */
#define _L2CACHE_FLUSHADDRTOP_ADDR_MASK              0xFFFFFFFFUL                              /**< Bit mask for L2CACHE_ADDR                   */
#define _L2CACHE_FLUSHADDRTOP_ADDR_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for L2CACHE_FLUSHADDRTOP       */
#define L2CACHE_FLUSHADDRTOP_ADDR_DEFAULT            (_L2CACHE_FLUSHADDRTOP_ADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_FLUSHADDRTOP*/

/* Bit fields for L2CACHE FLUSHADDRBOT */
#define _L2CACHE_FLUSHADDRBOT_RESETVALUE             0x00000000UL                              /**< Default value for L2CACHE_FLUSHADDRBOT      */
#define _L2CACHE_FLUSHADDRBOT_MASK                   0xFFFFFFFFUL                              /**< Mask for L2CACHE_FLUSHADDRBOT               */
#define _L2CACHE_FLUSHADDRBOT_ADDR_SHIFT             0                                         /**< Shift value for L2CACHE_ADDR                */
#define _L2CACHE_FLUSHADDRBOT_ADDR_MASK              0xFFFFFFFFUL                              /**< Bit mask for L2CACHE_ADDR                   */
#define _L2CACHE_FLUSHADDRBOT_ADDR_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for L2CACHE_FLUSHADDRBOT       */
#define L2CACHE_FLUSHADDRBOT_ADDR_DEFAULT            (_L2CACHE_FLUSHADDRBOT_ADDR_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_FLUSHADDRBOT*/

/* Bit fields for L2CACHE PERFCMD */
#define _L2CACHE_PERFCMD_RESETVALUE                  0x00000000UL                                /**< Default value for L2CACHE_PERFCMD           */
#define _L2CACHE_PERFCMD_MASK                        0x00000777UL                                /**< Mask for L2CACHE_PERFCMD                    */
#define L2CACHE_PERFCMD_PC0START                     (0x1UL << 0)                                /**< Start PerfReg0 Counter                      */
#define _L2CACHE_PERFCMD_PC0START_SHIFT              0                                           /**< Shift value for L2CACHE_PC0START            */
#define _L2CACHE_PERFCMD_PC0START_MASK               0x1UL                                       /**< Bit mask for L2CACHE_PC0START               */
#define _L2CACHE_PERFCMD_PC0START_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PC0START_DEFAULT             (_L2CACHE_PERFCMD_PC0START_DEFAULT << 0)    /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PC0STOP                      (0x1UL << 1)                                /**< Stop PerfReg0 Counter                       */
#define _L2CACHE_PERFCMD_PC0STOP_SHIFT               1                                           /**< Shift value for L2CACHE_PC0STOP             */
#define _L2CACHE_PERFCMD_PC0STOP_MASK                0x2UL                                       /**< Bit mask for L2CACHE_PC0STOP                */
#define _L2CACHE_PERFCMD_PC0STOP_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PC0STOP_DEFAULT              (_L2CACHE_PERFCMD_PC0STOP_DEFAULT << 1)     /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PC0CLEAR                     (0x1UL << 2)                                /**< CLEAR PerfReg0 Counter                      */
#define _L2CACHE_PERFCMD_PC0CLEAR_SHIFT              2                                           /**< Shift value for L2CACHE_PC0CLEAR            */
#define _L2CACHE_PERFCMD_PC0CLEAR_MASK               0x4UL                                       /**< Bit mask for L2CACHE_PC0CLEAR               */
#define _L2CACHE_PERFCMD_PC0CLEAR_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PC0CLEAR_DEFAULT             (_L2CACHE_PERFCMD_PC0CLEAR_DEFAULT << 2)    /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PC1START                     (0x1UL << 4)                                /**< Start PerfReg1 Counter                      */
#define _L2CACHE_PERFCMD_PC1START_SHIFT              4                                           /**< Shift value for L2CACHE_PC1START            */
#define _L2CACHE_PERFCMD_PC1START_MASK               0x10UL                                      /**< Bit mask for L2CACHE_PC1START               */
#define _L2CACHE_PERFCMD_PC1START_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PC1START_DEFAULT             (_L2CACHE_PERFCMD_PC1START_DEFAULT << 4)    /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PC1STOP                      (0x1UL << 5)                                /**< Stop PerfReg1 Counter                       */
#define _L2CACHE_PERFCMD_PC1STOP_SHIFT               5                                           /**< Shift value for L2CACHE_PC1STOP             */
#define _L2CACHE_PERFCMD_PC1STOP_MASK                0x20UL                                      /**< Bit mask for L2CACHE_PC1STOP                */
#define _L2CACHE_PERFCMD_PC1STOP_DEFAULT             0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PC1STOP_DEFAULT              (_L2CACHE_PERFCMD_PC1STOP_DEFAULT << 5)     /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PC1CLEAR                     (0x1UL << 6)                                /**< CLEAR PerfReg1 Counter                      */
#define _L2CACHE_PERFCMD_PC1CLEAR_SHIFT              6                                           /**< Shift value for L2CACHE_PC1CLEAR            */
#define _L2CACHE_PERFCMD_PC1CLEAR_MASK               0x40UL                                      /**< Bit mask for L2CACHE_PC1CLEAR               */
#define _L2CACHE_PERFCMD_PC1CLEAR_DEFAULT            0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PC1CLEAR_DEFAULT             (_L2CACHE_PERFCMD_PC1CLEAR_DEFAULT << 6)    /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PCALLSTART                   (0x1UL << 8)                                /**< Start PerfRegall Counter                    */
#define _L2CACHE_PERFCMD_PCALLSTART_SHIFT            8                                           /**< Shift value for L2CACHE_PCALLSTART          */
#define _L2CACHE_PERFCMD_PCALLSTART_MASK             0x100UL                                     /**< Bit mask for L2CACHE_PCALLSTART             */
#define _L2CACHE_PERFCMD_PCALLSTART_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PCALLSTART_DEFAULT           (_L2CACHE_PERFCMD_PCALLSTART_DEFAULT << 8)  /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PCALLSTOP                    (0x1UL << 9)                                /**< Stop PerfRegall Counter                     */
#define _L2CACHE_PERFCMD_PCALLSTOP_SHIFT             9                                           /**< Shift value for L2CACHE_PCALLSTOP           */
#define _L2CACHE_PERFCMD_PCALLSTOP_MASK              0x200UL                                     /**< Bit mask for L2CACHE_PCALLSTOP              */
#define _L2CACHE_PERFCMD_PCALLSTOP_DEFAULT           0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PCALLSTOP_DEFAULT            (_L2CACHE_PERFCMD_PCALLSTOP_DEFAULT << 9)   /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */
#define L2CACHE_PERFCMD_PCALLCLEAR                   (0x1UL << 10)                               /**< CLEAR PerfRegall Counter                    */
#define _L2CACHE_PERFCMD_PCALLCLEAR_SHIFT            10                                          /**< Shift value for L2CACHE_PCALLCLEAR          */
#define _L2CACHE_PERFCMD_PCALLCLEAR_MASK             0x400UL                                     /**< Bit mask for L2CACHE_PCALLCLEAR             */
#define _L2CACHE_PERFCMD_PCALLCLEAR_DEFAULT          0x00000000UL                                /**< Mode DEFAULT for L2CACHE_PERFCMD            */
#define L2CACHE_PERFCMD_PCALLCLEAR_DEFAULT           (_L2CACHE_PERFCMD_PCALLCLEAR_DEFAULT << 10) /**< Shifted mode DEFAULT for L2CACHE_PERFCMD    */

/* Bit fields for L2CACHE PERFSEL */
#define _L2CACHE_PERFSEL_RESETVALUE                  0x00000000UL                                 /**< Default value for L2CACHE_PERFSEL           */
#define _L2CACHE_PERFSEL_MASK                        0x00FF00FFUL                                 /**< Mask for L2CACHE_PERFSEL                    */
#define _L2CACHE_PERFSEL_PC0MASTERID_SHIFT           0                                            /**< Shift value for L2CACHE_PC0MASTERID         */
#define _L2CACHE_PERFSEL_PC0MASTERID_MASK            0xFFUL                                       /**< Bit mask for L2CACHE_PC0MASTERID            */
#define _L2CACHE_PERFSEL_PC0MASTERID_DEFAULT         0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_PERFSEL            */
#define L2CACHE_PERFSEL_PC0MASTERID_DEFAULT          (_L2CACHE_PERFSEL_PC0MASTERID_DEFAULT << 0)  /**< Shifted mode DEFAULT for L2CACHE_PERFSEL    */
#define _L2CACHE_PERFSEL_PC1MASTERID_SHIFT           16                                           /**< Shift value for L2CACHE_PC1MASTERID         */
#define _L2CACHE_PERFSEL_PC1MASTERID_MASK            0xFF0000UL                                   /**< Bit mask for L2CACHE_PC1MASTERID            */
#define _L2CACHE_PERFSEL_PC1MASTERID_DEFAULT         0x00000000UL                                 /**< Mode DEFAULT for L2CACHE_PERFSEL            */
#define L2CACHE_PERFSEL_PC1MASTERID_DEFAULT          (_L2CACHE_PERFSEL_PC1MASTERID_DEFAULT << 16) /**< Shifted mode DEFAULT for L2CACHE_PERFSEL    */

/* Bit fields for L2CACHE LINECOUNTSEL */
#define _L2CACHE_LINECOUNTSEL_RESETVALUE             0x00060015UL                                      /**< Default value for L2CACHE_LINECOUNTSEL      */
#define _L2CACHE_LINECOUNTSEL_MASK                   0x00FF00FFUL                                      /**< Mask for L2CACHE_LINECOUNTSEL               */
#define _L2CACHE_LINECOUNTSEL_LC3MASTERID_SHIFT      0                                                 /**< Shift value for L2CACHE_LC3MASTERID         */
#define _L2CACHE_LINECOUNTSEL_LC3MASTERID_MASK       0xFFUL                                            /**< Bit mask for L2CACHE_LC3MASTERID            */
#define _L2CACHE_LINECOUNTSEL_LC3MASTERID_DEFAULT    0x00000015UL                                      /**< Mode DEFAULT for L2CACHE_LINECOUNTSEL       */
#define L2CACHE_LINECOUNTSEL_LC3MASTERID_DEFAULT     (_L2CACHE_LINECOUNTSEL_LC3MASTERID_DEFAULT << 0)  /**< Shifted mode DEFAULT for L2CACHE_LINECOUNTSEL*/
#define _L2CACHE_LINECOUNTSEL_LC4MASTERID_SHIFT      16                                                /**< Shift value for L2CACHE_LC4MASTERID         */
#define _L2CACHE_LINECOUNTSEL_LC4MASTERID_MASK       0xFF0000UL                                        /**< Bit mask for L2CACHE_LC4MASTERID            */
#define _L2CACHE_LINECOUNTSEL_LC4MASTERID_DEFAULT    0x00000006UL                                      /**< Mode DEFAULT for L2CACHE_LINECOUNTSEL       */
#define L2CACHE_LINECOUNTSEL_LC4MASTERID_DEFAULT     (_L2CACHE_LINECOUNTSEL_LC4MASTERID_DEFAULT << 16) /**< Shifted mode DEFAULT for L2CACHE_LINECOUNTSEL*/

/* Bit fields for L2CACHE PC0HITS */
#define _L2CACHE_PC0HITS_RESETVALUE                  0x00000000UL                          /**< Default value for L2CACHE_PC0HITS           */
#define _L2CACHE_PC0HITS_MASK                        0xFFFFFFFFUL                          /**< Mask for L2CACHE_PC0HITS                    */
#define _L2CACHE_PC0HITS_COUNT_SHIFT                 0                                     /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_PC0HITS_COUNT_MASK                  0xFFFFFFFFUL                          /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_PC0HITS_COUNT_DEFAULT               0x00000000UL                          /**< Mode DEFAULT for L2CACHE_PC0HITS            */
#define L2CACHE_PC0HITS_COUNT_DEFAULT                (_L2CACHE_PC0HITS_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PC0HITS    */

/* Bit fields for L2CACHE PC0MISSES */
#define _L2CACHE_PC0MISSES_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_PC0MISSES         */
#define _L2CACHE_PC0MISSES_MASK                      0xFFFFFFFFUL                            /**< Mask for L2CACHE_PC0MISSES                  */
#define _L2CACHE_PC0MISSES_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_PC0MISSES_COUNT_MASK                0xFFFFFFFFUL                            /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_PC0MISSES_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_PC0MISSES          */
#define L2CACHE_PC0MISSES_COUNT_DEFAULT              (_L2CACHE_PC0MISSES_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PC0MISSES  */

/* Bit fields for L2CACHE PC1HITS */
#define _L2CACHE_PC1HITS_RESETVALUE                  0x00000000UL                          /**< Default value for L2CACHE_PC1HITS           */
#define _L2CACHE_PC1HITS_MASK                        0xFFFFFFFFUL                          /**< Mask for L2CACHE_PC1HITS                    */
#define _L2CACHE_PC1HITS_COUNT_SHIFT                 0                                     /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_PC1HITS_COUNT_MASK                  0xFFFFFFFFUL                          /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_PC1HITS_COUNT_DEFAULT               0x00000000UL                          /**< Mode DEFAULT for L2CACHE_PC1HITS            */
#define L2CACHE_PC1HITS_COUNT_DEFAULT                (_L2CACHE_PC1HITS_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PC1HITS    */

/* Bit fields for L2CACHE PC1MISSES */
#define _L2CACHE_PC1MISSES_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_PC1MISSES         */
#define _L2CACHE_PC1MISSES_MASK                      0xFFFFFFFFUL                            /**< Mask for L2CACHE_PC1MISSES                  */
#define _L2CACHE_PC1MISSES_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_PC1MISSES_COUNT_MASK                0xFFFFFFFFUL                            /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_PC1MISSES_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_PC1MISSES          */
#define L2CACHE_PC1MISSES_COUNT_DEFAULT              (_L2CACHE_PC1MISSES_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PC1MISSES  */

/* Bit fields for L2CACHE PCALLHITS */
#define _L2CACHE_PCALLHITS_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_PCALLHITS         */
#define _L2CACHE_PCALLHITS_MASK                      0xFFFFFFFFUL                            /**< Mask for L2CACHE_PCALLHITS                  */
#define _L2CACHE_PCALLHITS_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_PCALLHITS_COUNT_MASK                0xFFFFFFFFUL                            /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_PCALLHITS_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_PCALLHITS          */
#define L2CACHE_PCALLHITS_COUNT_DEFAULT              (_L2CACHE_PCALLHITS_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PCALLHITS  */

/* Bit fields for L2CACHE PCALLMISSES */
#define _L2CACHE_PCALLMISSES_RESETVALUE              0x00000000UL                              /**< Default value for L2CACHE_PCALLMISSES       */
#define _L2CACHE_PCALLMISSES_MASK                    0xFFFFFFFFUL                              /**< Mask for L2CACHE_PCALLMISSES                */
#define _L2CACHE_PCALLMISSES_COUNT_SHIFT             0                                         /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_PCALLMISSES_COUNT_MASK              0xFFFFFFFFUL                              /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_PCALLMISSES_COUNT_DEFAULT           0x00000000UL                              /**< Mode DEFAULT for L2CACHE_PCALLMISSES        */
#define L2CACHE_PCALLMISSES_COUNT_DEFAULT            (_L2CACHE_PCALLMISSES_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_PCALLMISSES*/

/* Bit fields for L2CACHE MASTER0LC */
#define _L2CACHE_MASTER0LC_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_MASTER0LC         */
#define _L2CACHE_MASTER0LC_MASK                      0x0000FFFFUL                            /**< Mask for L2CACHE_MASTER0LC                  */
#define _L2CACHE_MASTER0LC_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_MASTER0LC_COUNT_MASK                0xFFFFUL                                /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_MASTER0LC_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_MASTER0LC          */
#define L2CACHE_MASTER0LC_COUNT_DEFAULT              (_L2CACHE_MASTER0LC_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_MASTER0LC  */

/* Bit fields for L2CACHE MASTER1LC */
#define _L2CACHE_MASTER1LC_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_MASTER1LC         */
#define _L2CACHE_MASTER1LC_MASK                      0x0000FFFFUL                            /**< Mask for L2CACHE_MASTER1LC                  */
#define _L2CACHE_MASTER1LC_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_MASTER1LC_COUNT_MASK                0xFFFFUL                                /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_MASTER1LC_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_MASTER1LC          */
#define L2CACHE_MASTER1LC_COUNT_DEFAULT              (_L2CACHE_MASTER1LC_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_MASTER1LC  */

/* Bit fields for L2CACHE MASTER2LC */
#define _L2CACHE_MASTER2LC_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_MASTER2LC         */
#define _L2CACHE_MASTER2LC_MASK                      0x0000FFFFUL                            /**< Mask for L2CACHE_MASTER2LC                  */
#define _L2CACHE_MASTER2LC_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_MASTER2LC_COUNT_MASK                0xFFFFUL                                /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_MASTER2LC_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_MASTER2LC          */
#define L2CACHE_MASTER2LC_COUNT_DEFAULT              (_L2CACHE_MASTER2LC_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_MASTER2LC  */

/* Bit fields for L2CACHE MASTER3LC */
#define _L2CACHE_MASTER3LC_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_MASTER3LC         */
#define _L2CACHE_MASTER3LC_MASK                      0x0000FFFFUL                            /**< Mask for L2CACHE_MASTER3LC                  */
#define _L2CACHE_MASTER3LC_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_MASTER3LC_COUNT_MASK                0xFFFFUL                                /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_MASTER3LC_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_MASTER3LC          */
#define L2CACHE_MASTER3LC_COUNT_DEFAULT              (_L2CACHE_MASTER3LC_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_MASTER3LC  */

/* Bit fields for L2CACHE MASTER4LC */
#define _L2CACHE_MASTER4LC_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_MASTER4LC         */
#define _L2CACHE_MASTER4LC_MASK                      0x0000FFFFUL                            /**< Mask for L2CACHE_MASTER4LC                  */
#define _L2CACHE_MASTER4LC_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_MASTER4LC_COUNT_MASK                0xFFFFUL                                /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_MASTER4LC_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_MASTER4LC          */
#define L2CACHE_MASTER4LC_COUNT_DEFAULT              (_L2CACHE_MASTER4LC_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_MASTER4LC  */

/* Bit fields for L2CACHE MASTERSLC */
#define _L2CACHE_MASTERSLC_RESETVALUE                0x00000000UL                            /**< Default value for L2CACHE_MASTERSLC         */
#define _L2CACHE_MASTERSLC_MASK                      0x0000FFFFUL                            /**< Mask for L2CACHE_MASTERSLC                  */
#define _L2CACHE_MASTERSLC_COUNT_SHIFT               0                                       /**< Shift value for L2CACHE_COUNT               */
#define _L2CACHE_MASTERSLC_COUNT_MASK                0xFFFFUL                                /**< Bit mask for L2CACHE_COUNT                  */
#define _L2CACHE_MASTERSLC_COUNT_DEFAULT             0x00000000UL                            /**< Mode DEFAULT for L2CACHE_MASTERSLC          */
#define L2CACHE_MASTERSLC_COUNT_DEFAULT              (_L2CACHE_MASTERSLC_COUNT_DEFAULT << 0) /**< Shifted mode DEFAULT for L2CACHE_MASTERSLC  */

/** @} End of group SIBG301_L2CACHE_BitFields */
/** @} End of group SIBG301_L2CACHE */
/** @} End of group Parts */

#endif // SIBG301_L2CACHE_H
