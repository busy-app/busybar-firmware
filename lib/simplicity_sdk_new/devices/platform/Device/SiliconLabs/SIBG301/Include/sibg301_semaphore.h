/**************************************************************************//**
 * @file
 * @brief SIBG301 SEMAPHORE register and bit field definitions
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
#ifndef SIBG301_SEMAPHORE_H
#define SIBG301_SEMAPHORE_H
#define SEMAPHORE_HAS_SET_CLEAR

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_SEMAPHORE SEMAPHORE
 * @{
 * @brief SIBG301 SEMAPHORE Register Declaration.
 *****************************************************************************/

/** SEMAPHORE Register Declaration. */
typedef struct semaphore_typedef{
  __IM uint32_t  IPVERSION;                     /**< IP VERSION Register                                */
  __IOM uint32_t LOCK0;                         /**< LOCK0 Register                                     */
  __IOM uint32_t LOCK1;                         /**< LOCK1 Register                                     */
  __IOM uint32_t LOCK2;                         /**< LOCK2 Register                                     */
  __IOM uint32_t LOCK3;                         /**< LOCK3 Register                                     */
  __IOM uint32_t LOCK4;                         /**< LOCK4 Register                                     */
  __IOM uint32_t LOCK5;                         /**< LOCK5 Register                                     */
  __IOM uint32_t LOCK6;                         /**< LOCK6 Register                                     */
  __IOM uint32_t LOCK7;                         /**< LOCK7 Register                                     */
  __IOM uint32_t LOCK8;                         /**< LOCK8 Register                                     */
  __IOM uint32_t LOCK9;                         /**< LOCK9 Register                                     */
  __IOM uint32_t LOCK10;                        /**< LOCK10 Register                                    */
  __IOM uint32_t LOCK11;                        /**< LOCK11 Register                                    */
  __IOM uint32_t LOCK12;                        /**< LOCK12 Register                                    */
  __IOM uint32_t LOCK13;                        /**< LOCK13 Register                                    */
  __IOM uint32_t LOCK14;                        /**< LOCK14 Register                                    */
  __IOM uint32_t LOCK15;                        /**< LOCK15 Register                                    */
  __IOM uint32_t LOCK16;                        /**< LOCK16 Register                                    */
  __IOM uint32_t LOCK17;                        /**< LOCK17 Register                                    */
  __IOM uint32_t LOCK18;                        /**< LOCK18 Register                                    */
  __IOM uint32_t LOCK19;                        /**< LOCK19 Register                                    */
  __IOM uint32_t LOCK20;                        /**< LOCK20 Register                                    */
  __IOM uint32_t LOCK21;                        /**< LOCK21 Register                                    */
  __IOM uint32_t LOCK22;                        /**< LOCK22 Register                                    */
  __IOM uint32_t LOCK23;                        /**< LOCK23 Register                                    */
  __IOM uint32_t LOCK24;                        /**< LOCK24 Register                                    */
  __IOM uint32_t LOCK25;                        /**< LOCK25 Register                                    */
  __IOM uint32_t LOCK26;                        /**< LOCK26 Register                                    */
  __IOM uint32_t LOCK27;                        /**< LOCK27 Register                                    */
  __IOM uint32_t LOCK28;                        /**< LOCK28 Register                                    */
  __IOM uint32_t LOCK29;                        /**< LOCK29 Register                                    */
  __IOM uint32_t LOCK30;                        /**< LOCK30 Register                                    */
  __IOM uint32_t LOCK31;                        /**< LOCK31 Register                                    */
  __IOM uint32_t LOCK32;                        /**< LOCK32 Register                                    */
  __IOM uint32_t LOCK33;                        /**< LOCK33 Register                                    */
  __IOM uint32_t LOCK34;                        /**< LOCK34 Register                                    */
  __IOM uint32_t LOCK35;                        /**< LOCK35 Register                                    */
  __IOM uint32_t LOCK36;                        /**< LOCK36 Register                                    */
  __IOM uint32_t LOCK37;                        /**< LOCK37 Register                                    */
  __IOM uint32_t LOCK38;                        /**< LOCK38 Register                                    */
  __IOM uint32_t LOCK39;                        /**< LOCK39 Register                                    */
  __IOM uint32_t LOCK40;                        /**< LOCK40 Register                                    */
  __IOM uint32_t LOCK41;                        /**< LOCK41 Register                                    */
  __IOM uint32_t LOCK42;                        /**< LOCK42 Register                                    */
  __IOM uint32_t LOCK43;                        /**< LOCK43 Register                                    */
  __IOM uint32_t LOCK44;                        /**< LOCK44 Register                                    */
  __IOM uint32_t LOCK45;                        /**< LOCK45 Register                                    */
  __IOM uint32_t LOCK46;                        /**< LOCK46 Register                                    */
  __IOM uint32_t LOCK47;                        /**< LOCK47 Register                                    */
  __IOM uint32_t LOCK48;                        /**< LOCK48 Register                                    */
  __IOM uint32_t LOCK49;                        /**< LOCK49 Register                                    */
  __IOM uint32_t LOCK50;                        /**< LOCK50 Register                                    */
  __IOM uint32_t LOCK51;                        /**< LOCK51 Register                                    */
  __IOM uint32_t LOCK52;                        /**< LOCK52 Register                                    */
  __IOM uint32_t LOCK53;                        /**< LOCK53 Register                                    */
  __IOM uint32_t LOCK54;                        /**< LOCK54 Register                                    */
  __IOM uint32_t LOCK55;                        /**< LOCK55 Register                                    */
  __IOM uint32_t LOCK56;                        /**< LOCK56 Register                                    */
  __IOM uint32_t LOCK57;                        /**< LOCK57 Register                                    */
  __IOM uint32_t LOCK58;                        /**< LOCK58 Register                                    */
  __IOM uint32_t LOCK59;                        /**< LOCK59 Register                                    */
  __IOM uint32_t LOCK60;                        /**< LOCK60 Register                                    */
  __IOM uint32_t LOCK61;                        /**< LOCK61 Register                                    */
  __IOM uint32_t LOCK62;                        /**< LOCK62 Register                                    */
  __IOM uint32_t LOCK63;                        /**< LOCK63 Register                                    */
  __IOM uint32_t LOCK64;                        /**< LOCK64 Register                                    */
  __IOM uint32_t LOCK65;                        /**< LOCK65 Register                                    */
  __IOM uint32_t LOCK66;                        /**< LOCK66 Register                                    */
  __IOM uint32_t LOCK67;                        /**< LOCK67 Register                                    */
  __IOM uint32_t LOCK68;                        /**< LOCK68 Register                                    */
  __IOM uint32_t LOCK69;                        /**< LOCK69 Register                                    */
  __IOM uint32_t LOCK70;                        /**< LOCK70 Register                                    */
  __IOM uint32_t LOCK71;                        /**< LOCK71 Register                                    */
  __IOM uint32_t LOCK72;                        /**< LOCK72 Register                                    */
  __IOM uint32_t LOCK73;                        /**< LOCK73 Register                                    */
  __IOM uint32_t LOCK74;                        /**< LOCK74 Register                                    */
  __IOM uint32_t LOCK75;                        /**< LOCK75 Register                                    */
  __IOM uint32_t LOCK76;                        /**< LOCK76 Register                                    */
  __IOM uint32_t LOCK77;                        /**< LOCK77 Register                                    */
  __IOM uint32_t LOCK78;                        /**< LOCK78 Register                                    */
  __IOM uint32_t LOCK79;                        /**< LOCK79 Register                                    */
  __IOM uint32_t LOCK80;                        /**< LOCK80 Register                                    */
  __IOM uint32_t LOCK81;                        /**< LOCK81 Register                                    */
  __IOM uint32_t LOCK82;                        /**< LOCK82 Register                                    */
  __IOM uint32_t LOCK83;                        /**< LOCK83 Register                                    */
  __IOM uint32_t LOCK84;                        /**< LOCK84 Register                                    */
  __IOM uint32_t LOCK85;                        /**< LOCK85 Register                                    */
  __IOM uint32_t LOCK86;                        /**< LOCK86 Register                                    */
  __IOM uint32_t LOCK87;                        /**< LOCK87 Register                                    */
  __IOM uint32_t LOCK88;                        /**< LOCK88 Register                                    */
  __IOM uint32_t LOCK89;                        /**< LOCK89 Register                                    */
  __IOM uint32_t LOCK90;                        /**< LOCK90 Register                                    */
  __IOM uint32_t LOCK91;                        /**< LOCK91 Register                                    */
  __IOM uint32_t LOCK92;                        /**< LOCK92 Register                                    */
  __IOM uint32_t LOCK93;                        /**< LOCK93 Register                                    */
  __IOM uint32_t LOCK94;                        /**< LOCK94 Register                                    */
  __IOM uint32_t LOCK95;                        /**< LOCK95 Register                                    */
  __IOM uint32_t LOCK96;                        /**< LOCK96 Register                                    */
  __IOM uint32_t LOCK97;                        /**< LOCK97 Register                                    */
  __IOM uint32_t LOCK98;                        /**< LOCK98 Register                                    */
  __IOM uint32_t LOCK99;                        /**< LOCK99 Register                                    */
  __IOM uint32_t LOCK100;                       /**< LOCK100 Register                                   */
  __IOM uint32_t LOCK101;                       /**< LOCK101 Register                                   */
  __IOM uint32_t LOCK102;                       /**< LOCK102 Register                                   */
  __IOM uint32_t LOCK103;                       /**< LOCK103 Register                                   */
  __IOM uint32_t LOCK104;                       /**< LOCK104 Register                                   */
  __IOM uint32_t LOCK105;                       /**< LOCK105 Register                                   */
  __IOM uint32_t LOCK106;                       /**< LOCK106 Register                                   */
  __IOM uint32_t LOCK107;                       /**< LOCK107 Register                                   */
  __IOM uint32_t LOCK108;                       /**< LOCK108 Register                                   */
  __IOM uint32_t LOCK109;                       /**< LOCK109 Register                                   */
  __IOM uint32_t LOCK110;                       /**< LOCK110 Register                                   */
  __IOM uint32_t LOCK111;                       /**< LOCK111 Register                                   */
  __IOM uint32_t LOCK112;                       /**< LOCK112 Register                                   */
  __IOM uint32_t LOCK113;                       /**< LOCK113 Register                                   */
  __IOM uint32_t LOCK114;                       /**< LOCK114 Register                                   */
  __IOM uint32_t LOCK115;                       /**< LOCK115 Register                                   */
  __IOM uint32_t LOCK116;                       /**< LOCK116 Register                                   */
  __IOM uint32_t LOCK117;                       /**< LOCK117 Register                                   */
  __IOM uint32_t LOCK118;                       /**< LOCK118 Register                                   */
  __IOM uint32_t LOCK119;                       /**< LOCK119 Register                                   */
  __IOM uint32_t LOCK120;                       /**< LOCK120 Register                                   */
  __IOM uint32_t LOCK121;                       /**< LOCK121 Register                                   */
  __IOM uint32_t LOCK122;                       /**< LOCK122 Register                                   */
  __IOM uint32_t LOCK123;                       /**< LOCK123 Register                                   */
  __IOM uint32_t LOCK124;                       /**< LOCK124 Register                                   */
  __IOM uint32_t LOCK125;                       /**< LOCK125 Register                                   */
  __IOM uint32_t LOCK126;                       /**< LOCK126 Register                                   */
  __IOM uint32_t LOCK127;                       /**< LOCK127 Register                                   */
  uint32_t       RESERVED0[895U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_SET;                 /**< IP VERSION Register                                */
  __IOM uint32_t LOCK0_SET;                     /**< LOCK0 Register                                     */
  __IOM uint32_t LOCK1_SET;                     /**< LOCK1 Register                                     */
  __IOM uint32_t LOCK2_SET;                     /**< LOCK2 Register                                     */
  __IOM uint32_t LOCK3_SET;                     /**< LOCK3 Register                                     */
  __IOM uint32_t LOCK4_SET;                     /**< LOCK4 Register                                     */
  __IOM uint32_t LOCK5_SET;                     /**< LOCK5 Register                                     */
  __IOM uint32_t LOCK6_SET;                     /**< LOCK6 Register                                     */
  __IOM uint32_t LOCK7_SET;                     /**< LOCK7 Register                                     */
  __IOM uint32_t LOCK8_SET;                     /**< LOCK8 Register                                     */
  __IOM uint32_t LOCK9_SET;                     /**< LOCK9 Register                                     */
  __IOM uint32_t LOCK10_SET;                    /**< LOCK10 Register                                    */
  __IOM uint32_t LOCK11_SET;                    /**< LOCK11 Register                                    */
  __IOM uint32_t LOCK12_SET;                    /**< LOCK12 Register                                    */
  __IOM uint32_t LOCK13_SET;                    /**< LOCK13 Register                                    */
  __IOM uint32_t LOCK14_SET;                    /**< LOCK14 Register                                    */
  __IOM uint32_t LOCK15_SET;                    /**< LOCK15 Register                                    */
  __IOM uint32_t LOCK16_SET;                    /**< LOCK16 Register                                    */
  __IOM uint32_t LOCK17_SET;                    /**< LOCK17 Register                                    */
  __IOM uint32_t LOCK18_SET;                    /**< LOCK18 Register                                    */
  __IOM uint32_t LOCK19_SET;                    /**< LOCK19 Register                                    */
  __IOM uint32_t LOCK20_SET;                    /**< LOCK20 Register                                    */
  __IOM uint32_t LOCK21_SET;                    /**< LOCK21 Register                                    */
  __IOM uint32_t LOCK22_SET;                    /**< LOCK22 Register                                    */
  __IOM uint32_t LOCK23_SET;                    /**< LOCK23 Register                                    */
  __IOM uint32_t LOCK24_SET;                    /**< LOCK24 Register                                    */
  __IOM uint32_t LOCK25_SET;                    /**< LOCK25 Register                                    */
  __IOM uint32_t LOCK26_SET;                    /**< LOCK26 Register                                    */
  __IOM uint32_t LOCK27_SET;                    /**< LOCK27 Register                                    */
  __IOM uint32_t LOCK28_SET;                    /**< LOCK28 Register                                    */
  __IOM uint32_t LOCK29_SET;                    /**< LOCK29 Register                                    */
  __IOM uint32_t LOCK30_SET;                    /**< LOCK30 Register                                    */
  __IOM uint32_t LOCK31_SET;                    /**< LOCK31 Register                                    */
  __IOM uint32_t LOCK32_SET;                    /**< LOCK32 Register                                    */
  __IOM uint32_t LOCK33_SET;                    /**< LOCK33 Register                                    */
  __IOM uint32_t LOCK34_SET;                    /**< LOCK34 Register                                    */
  __IOM uint32_t LOCK35_SET;                    /**< LOCK35 Register                                    */
  __IOM uint32_t LOCK36_SET;                    /**< LOCK36 Register                                    */
  __IOM uint32_t LOCK37_SET;                    /**< LOCK37 Register                                    */
  __IOM uint32_t LOCK38_SET;                    /**< LOCK38 Register                                    */
  __IOM uint32_t LOCK39_SET;                    /**< LOCK39 Register                                    */
  __IOM uint32_t LOCK40_SET;                    /**< LOCK40 Register                                    */
  __IOM uint32_t LOCK41_SET;                    /**< LOCK41 Register                                    */
  __IOM uint32_t LOCK42_SET;                    /**< LOCK42 Register                                    */
  __IOM uint32_t LOCK43_SET;                    /**< LOCK43 Register                                    */
  __IOM uint32_t LOCK44_SET;                    /**< LOCK44 Register                                    */
  __IOM uint32_t LOCK45_SET;                    /**< LOCK45 Register                                    */
  __IOM uint32_t LOCK46_SET;                    /**< LOCK46 Register                                    */
  __IOM uint32_t LOCK47_SET;                    /**< LOCK47 Register                                    */
  __IOM uint32_t LOCK48_SET;                    /**< LOCK48 Register                                    */
  __IOM uint32_t LOCK49_SET;                    /**< LOCK49 Register                                    */
  __IOM uint32_t LOCK50_SET;                    /**< LOCK50 Register                                    */
  __IOM uint32_t LOCK51_SET;                    /**< LOCK51 Register                                    */
  __IOM uint32_t LOCK52_SET;                    /**< LOCK52 Register                                    */
  __IOM uint32_t LOCK53_SET;                    /**< LOCK53 Register                                    */
  __IOM uint32_t LOCK54_SET;                    /**< LOCK54 Register                                    */
  __IOM uint32_t LOCK55_SET;                    /**< LOCK55 Register                                    */
  __IOM uint32_t LOCK56_SET;                    /**< LOCK56 Register                                    */
  __IOM uint32_t LOCK57_SET;                    /**< LOCK57 Register                                    */
  __IOM uint32_t LOCK58_SET;                    /**< LOCK58 Register                                    */
  __IOM uint32_t LOCK59_SET;                    /**< LOCK59 Register                                    */
  __IOM uint32_t LOCK60_SET;                    /**< LOCK60 Register                                    */
  __IOM uint32_t LOCK61_SET;                    /**< LOCK61 Register                                    */
  __IOM uint32_t LOCK62_SET;                    /**< LOCK62 Register                                    */
  __IOM uint32_t LOCK63_SET;                    /**< LOCK63 Register                                    */
  __IOM uint32_t LOCK64_SET;                    /**< LOCK64 Register                                    */
  __IOM uint32_t LOCK65_SET;                    /**< LOCK65 Register                                    */
  __IOM uint32_t LOCK66_SET;                    /**< LOCK66 Register                                    */
  __IOM uint32_t LOCK67_SET;                    /**< LOCK67 Register                                    */
  __IOM uint32_t LOCK68_SET;                    /**< LOCK68 Register                                    */
  __IOM uint32_t LOCK69_SET;                    /**< LOCK69 Register                                    */
  __IOM uint32_t LOCK70_SET;                    /**< LOCK70 Register                                    */
  __IOM uint32_t LOCK71_SET;                    /**< LOCK71 Register                                    */
  __IOM uint32_t LOCK72_SET;                    /**< LOCK72 Register                                    */
  __IOM uint32_t LOCK73_SET;                    /**< LOCK73 Register                                    */
  __IOM uint32_t LOCK74_SET;                    /**< LOCK74 Register                                    */
  __IOM uint32_t LOCK75_SET;                    /**< LOCK75 Register                                    */
  __IOM uint32_t LOCK76_SET;                    /**< LOCK76 Register                                    */
  __IOM uint32_t LOCK77_SET;                    /**< LOCK77 Register                                    */
  __IOM uint32_t LOCK78_SET;                    /**< LOCK78 Register                                    */
  __IOM uint32_t LOCK79_SET;                    /**< LOCK79 Register                                    */
  __IOM uint32_t LOCK80_SET;                    /**< LOCK80 Register                                    */
  __IOM uint32_t LOCK81_SET;                    /**< LOCK81 Register                                    */
  __IOM uint32_t LOCK82_SET;                    /**< LOCK82 Register                                    */
  __IOM uint32_t LOCK83_SET;                    /**< LOCK83 Register                                    */
  __IOM uint32_t LOCK84_SET;                    /**< LOCK84 Register                                    */
  __IOM uint32_t LOCK85_SET;                    /**< LOCK85 Register                                    */
  __IOM uint32_t LOCK86_SET;                    /**< LOCK86 Register                                    */
  __IOM uint32_t LOCK87_SET;                    /**< LOCK87 Register                                    */
  __IOM uint32_t LOCK88_SET;                    /**< LOCK88 Register                                    */
  __IOM uint32_t LOCK89_SET;                    /**< LOCK89 Register                                    */
  __IOM uint32_t LOCK90_SET;                    /**< LOCK90 Register                                    */
  __IOM uint32_t LOCK91_SET;                    /**< LOCK91 Register                                    */
  __IOM uint32_t LOCK92_SET;                    /**< LOCK92 Register                                    */
  __IOM uint32_t LOCK93_SET;                    /**< LOCK93 Register                                    */
  __IOM uint32_t LOCK94_SET;                    /**< LOCK94 Register                                    */
  __IOM uint32_t LOCK95_SET;                    /**< LOCK95 Register                                    */
  __IOM uint32_t LOCK96_SET;                    /**< LOCK96 Register                                    */
  __IOM uint32_t LOCK97_SET;                    /**< LOCK97 Register                                    */
  __IOM uint32_t LOCK98_SET;                    /**< LOCK98 Register                                    */
  __IOM uint32_t LOCK99_SET;                    /**< LOCK99 Register                                    */
  __IOM uint32_t LOCK100_SET;                   /**< LOCK100 Register                                   */
  __IOM uint32_t LOCK101_SET;                   /**< LOCK101 Register                                   */
  __IOM uint32_t LOCK102_SET;                   /**< LOCK102 Register                                   */
  __IOM uint32_t LOCK103_SET;                   /**< LOCK103 Register                                   */
  __IOM uint32_t LOCK104_SET;                   /**< LOCK104 Register                                   */
  __IOM uint32_t LOCK105_SET;                   /**< LOCK105 Register                                   */
  __IOM uint32_t LOCK106_SET;                   /**< LOCK106 Register                                   */
  __IOM uint32_t LOCK107_SET;                   /**< LOCK107 Register                                   */
  __IOM uint32_t LOCK108_SET;                   /**< LOCK108 Register                                   */
  __IOM uint32_t LOCK109_SET;                   /**< LOCK109 Register                                   */
  __IOM uint32_t LOCK110_SET;                   /**< LOCK110 Register                                   */
  __IOM uint32_t LOCK111_SET;                   /**< LOCK111 Register                                   */
  __IOM uint32_t LOCK112_SET;                   /**< LOCK112 Register                                   */
  __IOM uint32_t LOCK113_SET;                   /**< LOCK113 Register                                   */
  __IOM uint32_t LOCK114_SET;                   /**< LOCK114 Register                                   */
  __IOM uint32_t LOCK115_SET;                   /**< LOCK115 Register                                   */
  __IOM uint32_t LOCK116_SET;                   /**< LOCK116 Register                                   */
  __IOM uint32_t LOCK117_SET;                   /**< LOCK117 Register                                   */
  __IOM uint32_t LOCK118_SET;                   /**< LOCK118 Register                                   */
  __IOM uint32_t LOCK119_SET;                   /**< LOCK119 Register                                   */
  __IOM uint32_t LOCK120_SET;                   /**< LOCK120 Register                                   */
  __IOM uint32_t LOCK121_SET;                   /**< LOCK121 Register                                   */
  __IOM uint32_t LOCK122_SET;                   /**< LOCK122 Register                                   */
  __IOM uint32_t LOCK123_SET;                   /**< LOCK123 Register                                   */
  __IOM uint32_t LOCK124_SET;                   /**< LOCK124 Register                                   */
  __IOM uint32_t LOCK125_SET;                   /**< LOCK125 Register                                   */
  __IOM uint32_t LOCK126_SET;                   /**< LOCK126 Register                                   */
  __IOM uint32_t LOCK127_SET;                   /**< LOCK127 Register                                   */
  uint32_t       RESERVED1[895U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_CLR;                 /**< IP VERSION Register                                */
  __IOM uint32_t LOCK0_CLR;                     /**< LOCK0 Register                                     */
  __IOM uint32_t LOCK1_CLR;                     /**< LOCK1 Register                                     */
  __IOM uint32_t LOCK2_CLR;                     /**< LOCK2 Register                                     */
  __IOM uint32_t LOCK3_CLR;                     /**< LOCK3 Register                                     */
  __IOM uint32_t LOCK4_CLR;                     /**< LOCK4 Register                                     */
  __IOM uint32_t LOCK5_CLR;                     /**< LOCK5 Register                                     */
  __IOM uint32_t LOCK6_CLR;                     /**< LOCK6 Register                                     */
  __IOM uint32_t LOCK7_CLR;                     /**< LOCK7 Register                                     */
  __IOM uint32_t LOCK8_CLR;                     /**< LOCK8 Register                                     */
  __IOM uint32_t LOCK9_CLR;                     /**< LOCK9 Register                                     */
  __IOM uint32_t LOCK10_CLR;                    /**< LOCK10 Register                                    */
  __IOM uint32_t LOCK11_CLR;                    /**< LOCK11 Register                                    */
  __IOM uint32_t LOCK12_CLR;                    /**< LOCK12 Register                                    */
  __IOM uint32_t LOCK13_CLR;                    /**< LOCK13 Register                                    */
  __IOM uint32_t LOCK14_CLR;                    /**< LOCK14 Register                                    */
  __IOM uint32_t LOCK15_CLR;                    /**< LOCK15 Register                                    */
  __IOM uint32_t LOCK16_CLR;                    /**< LOCK16 Register                                    */
  __IOM uint32_t LOCK17_CLR;                    /**< LOCK17 Register                                    */
  __IOM uint32_t LOCK18_CLR;                    /**< LOCK18 Register                                    */
  __IOM uint32_t LOCK19_CLR;                    /**< LOCK19 Register                                    */
  __IOM uint32_t LOCK20_CLR;                    /**< LOCK20 Register                                    */
  __IOM uint32_t LOCK21_CLR;                    /**< LOCK21 Register                                    */
  __IOM uint32_t LOCK22_CLR;                    /**< LOCK22 Register                                    */
  __IOM uint32_t LOCK23_CLR;                    /**< LOCK23 Register                                    */
  __IOM uint32_t LOCK24_CLR;                    /**< LOCK24 Register                                    */
  __IOM uint32_t LOCK25_CLR;                    /**< LOCK25 Register                                    */
  __IOM uint32_t LOCK26_CLR;                    /**< LOCK26 Register                                    */
  __IOM uint32_t LOCK27_CLR;                    /**< LOCK27 Register                                    */
  __IOM uint32_t LOCK28_CLR;                    /**< LOCK28 Register                                    */
  __IOM uint32_t LOCK29_CLR;                    /**< LOCK29 Register                                    */
  __IOM uint32_t LOCK30_CLR;                    /**< LOCK30 Register                                    */
  __IOM uint32_t LOCK31_CLR;                    /**< LOCK31 Register                                    */
  __IOM uint32_t LOCK32_CLR;                    /**< LOCK32 Register                                    */
  __IOM uint32_t LOCK33_CLR;                    /**< LOCK33 Register                                    */
  __IOM uint32_t LOCK34_CLR;                    /**< LOCK34 Register                                    */
  __IOM uint32_t LOCK35_CLR;                    /**< LOCK35 Register                                    */
  __IOM uint32_t LOCK36_CLR;                    /**< LOCK36 Register                                    */
  __IOM uint32_t LOCK37_CLR;                    /**< LOCK37 Register                                    */
  __IOM uint32_t LOCK38_CLR;                    /**< LOCK38 Register                                    */
  __IOM uint32_t LOCK39_CLR;                    /**< LOCK39 Register                                    */
  __IOM uint32_t LOCK40_CLR;                    /**< LOCK40 Register                                    */
  __IOM uint32_t LOCK41_CLR;                    /**< LOCK41 Register                                    */
  __IOM uint32_t LOCK42_CLR;                    /**< LOCK42 Register                                    */
  __IOM uint32_t LOCK43_CLR;                    /**< LOCK43 Register                                    */
  __IOM uint32_t LOCK44_CLR;                    /**< LOCK44 Register                                    */
  __IOM uint32_t LOCK45_CLR;                    /**< LOCK45 Register                                    */
  __IOM uint32_t LOCK46_CLR;                    /**< LOCK46 Register                                    */
  __IOM uint32_t LOCK47_CLR;                    /**< LOCK47 Register                                    */
  __IOM uint32_t LOCK48_CLR;                    /**< LOCK48 Register                                    */
  __IOM uint32_t LOCK49_CLR;                    /**< LOCK49 Register                                    */
  __IOM uint32_t LOCK50_CLR;                    /**< LOCK50 Register                                    */
  __IOM uint32_t LOCK51_CLR;                    /**< LOCK51 Register                                    */
  __IOM uint32_t LOCK52_CLR;                    /**< LOCK52 Register                                    */
  __IOM uint32_t LOCK53_CLR;                    /**< LOCK53 Register                                    */
  __IOM uint32_t LOCK54_CLR;                    /**< LOCK54 Register                                    */
  __IOM uint32_t LOCK55_CLR;                    /**< LOCK55 Register                                    */
  __IOM uint32_t LOCK56_CLR;                    /**< LOCK56 Register                                    */
  __IOM uint32_t LOCK57_CLR;                    /**< LOCK57 Register                                    */
  __IOM uint32_t LOCK58_CLR;                    /**< LOCK58 Register                                    */
  __IOM uint32_t LOCK59_CLR;                    /**< LOCK59 Register                                    */
  __IOM uint32_t LOCK60_CLR;                    /**< LOCK60 Register                                    */
  __IOM uint32_t LOCK61_CLR;                    /**< LOCK61 Register                                    */
  __IOM uint32_t LOCK62_CLR;                    /**< LOCK62 Register                                    */
  __IOM uint32_t LOCK63_CLR;                    /**< LOCK63 Register                                    */
  __IOM uint32_t LOCK64_CLR;                    /**< LOCK64 Register                                    */
  __IOM uint32_t LOCK65_CLR;                    /**< LOCK65 Register                                    */
  __IOM uint32_t LOCK66_CLR;                    /**< LOCK66 Register                                    */
  __IOM uint32_t LOCK67_CLR;                    /**< LOCK67 Register                                    */
  __IOM uint32_t LOCK68_CLR;                    /**< LOCK68 Register                                    */
  __IOM uint32_t LOCK69_CLR;                    /**< LOCK69 Register                                    */
  __IOM uint32_t LOCK70_CLR;                    /**< LOCK70 Register                                    */
  __IOM uint32_t LOCK71_CLR;                    /**< LOCK71 Register                                    */
  __IOM uint32_t LOCK72_CLR;                    /**< LOCK72 Register                                    */
  __IOM uint32_t LOCK73_CLR;                    /**< LOCK73 Register                                    */
  __IOM uint32_t LOCK74_CLR;                    /**< LOCK74 Register                                    */
  __IOM uint32_t LOCK75_CLR;                    /**< LOCK75 Register                                    */
  __IOM uint32_t LOCK76_CLR;                    /**< LOCK76 Register                                    */
  __IOM uint32_t LOCK77_CLR;                    /**< LOCK77 Register                                    */
  __IOM uint32_t LOCK78_CLR;                    /**< LOCK78 Register                                    */
  __IOM uint32_t LOCK79_CLR;                    /**< LOCK79 Register                                    */
  __IOM uint32_t LOCK80_CLR;                    /**< LOCK80 Register                                    */
  __IOM uint32_t LOCK81_CLR;                    /**< LOCK81 Register                                    */
  __IOM uint32_t LOCK82_CLR;                    /**< LOCK82 Register                                    */
  __IOM uint32_t LOCK83_CLR;                    /**< LOCK83 Register                                    */
  __IOM uint32_t LOCK84_CLR;                    /**< LOCK84 Register                                    */
  __IOM uint32_t LOCK85_CLR;                    /**< LOCK85 Register                                    */
  __IOM uint32_t LOCK86_CLR;                    /**< LOCK86 Register                                    */
  __IOM uint32_t LOCK87_CLR;                    /**< LOCK87 Register                                    */
  __IOM uint32_t LOCK88_CLR;                    /**< LOCK88 Register                                    */
  __IOM uint32_t LOCK89_CLR;                    /**< LOCK89 Register                                    */
  __IOM uint32_t LOCK90_CLR;                    /**< LOCK90 Register                                    */
  __IOM uint32_t LOCK91_CLR;                    /**< LOCK91 Register                                    */
  __IOM uint32_t LOCK92_CLR;                    /**< LOCK92 Register                                    */
  __IOM uint32_t LOCK93_CLR;                    /**< LOCK93 Register                                    */
  __IOM uint32_t LOCK94_CLR;                    /**< LOCK94 Register                                    */
  __IOM uint32_t LOCK95_CLR;                    /**< LOCK95 Register                                    */
  __IOM uint32_t LOCK96_CLR;                    /**< LOCK96 Register                                    */
  __IOM uint32_t LOCK97_CLR;                    /**< LOCK97 Register                                    */
  __IOM uint32_t LOCK98_CLR;                    /**< LOCK98 Register                                    */
  __IOM uint32_t LOCK99_CLR;                    /**< LOCK99 Register                                    */
  __IOM uint32_t LOCK100_CLR;                   /**< LOCK100 Register                                   */
  __IOM uint32_t LOCK101_CLR;                   /**< LOCK101 Register                                   */
  __IOM uint32_t LOCK102_CLR;                   /**< LOCK102 Register                                   */
  __IOM uint32_t LOCK103_CLR;                   /**< LOCK103 Register                                   */
  __IOM uint32_t LOCK104_CLR;                   /**< LOCK104 Register                                   */
  __IOM uint32_t LOCK105_CLR;                   /**< LOCK105 Register                                   */
  __IOM uint32_t LOCK106_CLR;                   /**< LOCK106 Register                                   */
  __IOM uint32_t LOCK107_CLR;                   /**< LOCK107 Register                                   */
  __IOM uint32_t LOCK108_CLR;                   /**< LOCK108 Register                                   */
  __IOM uint32_t LOCK109_CLR;                   /**< LOCK109 Register                                   */
  __IOM uint32_t LOCK110_CLR;                   /**< LOCK110 Register                                   */
  __IOM uint32_t LOCK111_CLR;                   /**< LOCK111 Register                                   */
  __IOM uint32_t LOCK112_CLR;                   /**< LOCK112 Register                                   */
  __IOM uint32_t LOCK113_CLR;                   /**< LOCK113 Register                                   */
  __IOM uint32_t LOCK114_CLR;                   /**< LOCK114 Register                                   */
  __IOM uint32_t LOCK115_CLR;                   /**< LOCK115 Register                                   */
  __IOM uint32_t LOCK116_CLR;                   /**< LOCK116 Register                                   */
  __IOM uint32_t LOCK117_CLR;                   /**< LOCK117 Register                                   */
  __IOM uint32_t LOCK118_CLR;                   /**< LOCK118 Register                                   */
  __IOM uint32_t LOCK119_CLR;                   /**< LOCK119 Register                                   */
  __IOM uint32_t LOCK120_CLR;                   /**< LOCK120 Register                                   */
  __IOM uint32_t LOCK121_CLR;                   /**< LOCK121 Register                                   */
  __IOM uint32_t LOCK122_CLR;                   /**< LOCK122 Register                                   */
  __IOM uint32_t LOCK123_CLR;                   /**< LOCK123 Register                                   */
  __IOM uint32_t LOCK124_CLR;                   /**< LOCK124 Register                                   */
  __IOM uint32_t LOCK125_CLR;                   /**< LOCK125 Register                                   */
  __IOM uint32_t LOCK126_CLR;                   /**< LOCK126 Register                                   */
  __IOM uint32_t LOCK127_CLR;                   /**< LOCK127 Register                                   */
  uint32_t       RESERVED2[895U];               /**< Reserved for future use                            */
  __IM uint32_t  IPVERSION_TGL;                 /**< IP VERSION Register                                */
  __IOM uint32_t LOCK0_TGL;                     /**< LOCK0 Register                                     */
  __IOM uint32_t LOCK1_TGL;                     /**< LOCK1 Register                                     */
  __IOM uint32_t LOCK2_TGL;                     /**< LOCK2 Register                                     */
  __IOM uint32_t LOCK3_TGL;                     /**< LOCK3 Register                                     */
  __IOM uint32_t LOCK4_TGL;                     /**< LOCK4 Register                                     */
  __IOM uint32_t LOCK5_TGL;                     /**< LOCK5 Register                                     */
  __IOM uint32_t LOCK6_TGL;                     /**< LOCK6 Register                                     */
  __IOM uint32_t LOCK7_TGL;                     /**< LOCK7 Register                                     */
  __IOM uint32_t LOCK8_TGL;                     /**< LOCK8 Register                                     */
  __IOM uint32_t LOCK9_TGL;                     /**< LOCK9 Register                                     */
  __IOM uint32_t LOCK10_TGL;                    /**< LOCK10 Register                                    */
  __IOM uint32_t LOCK11_TGL;                    /**< LOCK11 Register                                    */
  __IOM uint32_t LOCK12_TGL;                    /**< LOCK12 Register                                    */
  __IOM uint32_t LOCK13_TGL;                    /**< LOCK13 Register                                    */
  __IOM uint32_t LOCK14_TGL;                    /**< LOCK14 Register                                    */
  __IOM uint32_t LOCK15_TGL;                    /**< LOCK15 Register                                    */
  __IOM uint32_t LOCK16_TGL;                    /**< LOCK16 Register                                    */
  __IOM uint32_t LOCK17_TGL;                    /**< LOCK17 Register                                    */
  __IOM uint32_t LOCK18_TGL;                    /**< LOCK18 Register                                    */
  __IOM uint32_t LOCK19_TGL;                    /**< LOCK19 Register                                    */
  __IOM uint32_t LOCK20_TGL;                    /**< LOCK20 Register                                    */
  __IOM uint32_t LOCK21_TGL;                    /**< LOCK21 Register                                    */
  __IOM uint32_t LOCK22_TGL;                    /**< LOCK22 Register                                    */
  __IOM uint32_t LOCK23_TGL;                    /**< LOCK23 Register                                    */
  __IOM uint32_t LOCK24_TGL;                    /**< LOCK24 Register                                    */
  __IOM uint32_t LOCK25_TGL;                    /**< LOCK25 Register                                    */
  __IOM uint32_t LOCK26_TGL;                    /**< LOCK26 Register                                    */
  __IOM uint32_t LOCK27_TGL;                    /**< LOCK27 Register                                    */
  __IOM uint32_t LOCK28_TGL;                    /**< LOCK28 Register                                    */
  __IOM uint32_t LOCK29_TGL;                    /**< LOCK29 Register                                    */
  __IOM uint32_t LOCK30_TGL;                    /**< LOCK30 Register                                    */
  __IOM uint32_t LOCK31_TGL;                    /**< LOCK31 Register                                    */
  __IOM uint32_t LOCK32_TGL;                    /**< LOCK32 Register                                    */
  __IOM uint32_t LOCK33_TGL;                    /**< LOCK33 Register                                    */
  __IOM uint32_t LOCK34_TGL;                    /**< LOCK34 Register                                    */
  __IOM uint32_t LOCK35_TGL;                    /**< LOCK35 Register                                    */
  __IOM uint32_t LOCK36_TGL;                    /**< LOCK36 Register                                    */
  __IOM uint32_t LOCK37_TGL;                    /**< LOCK37 Register                                    */
  __IOM uint32_t LOCK38_TGL;                    /**< LOCK38 Register                                    */
  __IOM uint32_t LOCK39_TGL;                    /**< LOCK39 Register                                    */
  __IOM uint32_t LOCK40_TGL;                    /**< LOCK40 Register                                    */
  __IOM uint32_t LOCK41_TGL;                    /**< LOCK41 Register                                    */
  __IOM uint32_t LOCK42_TGL;                    /**< LOCK42 Register                                    */
  __IOM uint32_t LOCK43_TGL;                    /**< LOCK43 Register                                    */
  __IOM uint32_t LOCK44_TGL;                    /**< LOCK44 Register                                    */
  __IOM uint32_t LOCK45_TGL;                    /**< LOCK45 Register                                    */
  __IOM uint32_t LOCK46_TGL;                    /**< LOCK46 Register                                    */
  __IOM uint32_t LOCK47_TGL;                    /**< LOCK47 Register                                    */
  __IOM uint32_t LOCK48_TGL;                    /**< LOCK48 Register                                    */
  __IOM uint32_t LOCK49_TGL;                    /**< LOCK49 Register                                    */
  __IOM uint32_t LOCK50_TGL;                    /**< LOCK50 Register                                    */
  __IOM uint32_t LOCK51_TGL;                    /**< LOCK51 Register                                    */
  __IOM uint32_t LOCK52_TGL;                    /**< LOCK52 Register                                    */
  __IOM uint32_t LOCK53_TGL;                    /**< LOCK53 Register                                    */
  __IOM uint32_t LOCK54_TGL;                    /**< LOCK54 Register                                    */
  __IOM uint32_t LOCK55_TGL;                    /**< LOCK55 Register                                    */
  __IOM uint32_t LOCK56_TGL;                    /**< LOCK56 Register                                    */
  __IOM uint32_t LOCK57_TGL;                    /**< LOCK57 Register                                    */
  __IOM uint32_t LOCK58_TGL;                    /**< LOCK58 Register                                    */
  __IOM uint32_t LOCK59_TGL;                    /**< LOCK59 Register                                    */
  __IOM uint32_t LOCK60_TGL;                    /**< LOCK60 Register                                    */
  __IOM uint32_t LOCK61_TGL;                    /**< LOCK61 Register                                    */
  __IOM uint32_t LOCK62_TGL;                    /**< LOCK62 Register                                    */
  __IOM uint32_t LOCK63_TGL;                    /**< LOCK63 Register                                    */
  __IOM uint32_t LOCK64_TGL;                    /**< LOCK64 Register                                    */
  __IOM uint32_t LOCK65_TGL;                    /**< LOCK65 Register                                    */
  __IOM uint32_t LOCK66_TGL;                    /**< LOCK66 Register                                    */
  __IOM uint32_t LOCK67_TGL;                    /**< LOCK67 Register                                    */
  __IOM uint32_t LOCK68_TGL;                    /**< LOCK68 Register                                    */
  __IOM uint32_t LOCK69_TGL;                    /**< LOCK69 Register                                    */
  __IOM uint32_t LOCK70_TGL;                    /**< LOCK70 Register                                    */
  __IOM uint32_t LOCK71_TGL;                    /**< LOCK71 Register                                    */
  __IOM uint32_t LOCK72_TGL;                    /**< LOCK72 Register                                    */
  __IOM uint32_t LOCK73_TGL;                    /**< LOCK73 Register                                    */
  __IOM uint32_t LOCK74_TGL;                    /**< LOCK74 Register                                    */
  __IOM uint32_t LOCK75_TGL;                    /**< LOCK75 Register                                    */
  __IOM uint32_t LOCK76_TGL;                    /**< LOCK76 Register                                    */
  __IOM uint32_t LOCK77_TGL;                    /**< LOCK77 Register                                    */
  __IOM uint32_t LOCK78_TGL;                    /**< LOCK78 Register                                    */
  __IOM uint32_t LOCK79_TGL;                    /**< LOCK79 Register                                    */
  __IOM uint32_t LOCK80_TGL;                    /**< LOCK80 Register                                    */
  __IOM uint32_t LOCK81_TGL;                    /**< LOCK81 Register                                    */
  __IOM uint32_t LOCK82_TGL;                    /**< LOCK82 Register                                    */
  __IOM uint32_t LOCK83_TGL;                    /**< LOCK83 Register                                    */
  __IOM uint32_t LOCK84_TGL;                    /**< LOCK84 Register                                    */
  __IOM uint32_t LOCK85_TGL;                    /**< LOCK85 Register                                    */
  __IOM uint32_t LOCK86_TGL;                    /**< LOCK86 Register                                    */
  __IOM uint32_t LOCK87_TGL;                    /**< LOCK87 Register                                    */
  __IOM uint32_t LOCK88_TGL;                    /**< LOCK88 Register                                    */
  __IOM uint32_t LOCK89_TGL;                    /**< LOCK89 Register                                    */
  __IOM uint32_t LOCK90_TGL;                    /**< LOCK90 Register                                    */
  __IOM uint32_t LOCK91_TGL;                    /**< LOCK91 Register                                    */
  __IOM uint32_t LOCK92_TGL;                    /**< LOCK92 Register                                    */
  __IOM uint32_t LOCK93_TGL;                    /**< LOCK93 Register                                    */
  __IOM uint32_t LOCK94_TGL;                    /**< LOCK94 Register                                    */
  __IOM uint32_t LOCK95_TGL;                    /**< LOCK95 Register                                    */
  __IOM uint32_t LOCK96_TGL;                    /**< LOCK96 Register                                    */
  __IOM uint32_t LOCK97_TGL;                    /**< LOCK97 Register                                    */
  __IOM uint32_t LOCK98_TGL;                    /**< LOCK98 Register                                    */
  __IOM uint32_t LOCK99_TGL;                    /**< LOCK99 Register                                    */
  __IOM uint32_t LOCK100_TGL;                   /**< LOCK100 Register                                   */
  __IOM uint32_t LOCK101_TGL;                   /**< LOCK101 Register                                   */
  __IOM uint32_t LOCK102_TGL;                   /**< LOCK102 Register                                   */
  __IOM uint32_t LOCK103_TGL;                   /**< LOCK103 Register                                   */
  __IOM uint32_t LOCK104_TGL;                   /**< LOCK104 Register                                   */
  __IOM uint32_t LOCK105_TGL;                   /**< LOCK105 Register                                   */
  __IOM uint32_t LOCK106_TGL;                   /**< LOCK106 Register                                   */
  __IOM uint32_t LOCK107_TGL;                   /**< LOCK107 Register                                   */
  __IOM uint32_t LOCK108_TGL;                   /**< LOCK108 Register                                   */
  __IOM uint32_t LOCK109_TGL;                   /**< LOCK109 Register                                   */
  __IOM uint32_t LOCK110_TGL;                   /**< LOCK110 Register                                   */
  __IOM uint32_t LOCK111_TGL;                   /**< LOCK111 Register                                   */
  __IOM uint32_t LOCK112_TGL;                   /**< LOCK112 Register                                   */
  __IOM uint32_t LOCK113_TGL;                   /**< LOCK113 Register                                   */
  __IOM uint32_t LOCK114_TGL;                   /**< LOCK114 Register                                   */
  __IOM uint32_t LOCK115_TGL;                   /**< LOCK115 Register                                   */
  __IOM uint32_t LOCK116_TGL;                   /**< LOCK116 Register                                   */
  __IOM uint32_t LOCK117_TGL;                   /**< LOCK117 Register                                   */
  __IOM uint32_t LOCK118_TGL;                   /**< LOCK118 Register                                   */
  __IOM uint32_t LOCK119_TGL;                   /**< LOCK119 Register                                   */
  __IOM uint32_t LOCK120_TGL;                   /**< LOCK120 Register                                   */
  __IOM uint32_t LOCK121_TGL;                   /**< LOCK121 Register                                   */
  __IOM uint32_t LOCK122_TGL;                   /**< LOCK122 Register                                   */
  __IOM uint32_t LOCK123_TGL;                   /**< LOCK123 Register                                   */
  __IOM uint32_t LOCK124_TGL;                   /**< LOCK124 Register                                   */
  __IOM uint32_t LOCK125_TGL;                   /**< LOCK125 Register                                   */
  __IOM uint32_t LOCK126_TGL;                   /**< LOCK126 Register                                   */
  __IOM uint32_t LOCK127_TGL;                   /**< LOCK127 Register                                   */
} SEMAPHORE_TypeDef;
/** @} End of group SIBG301_SEMAPHORE */

/**************************************************************************//**
 * @addtogroup SIBG301_SEMAPHORE
 * @{
 * @defgroup SIBG301_SEMAPHORE_BitFields SEMAPHORE Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for SEMAPHORE IPVERSION */
#define _SEMAPHORE_IPVERSION_RESETVALUE           0x00000001UL                                  /**< Default value for SEMAPHORE_IPVERSION       */
#define _SEMAPHORE_IPVERSION_MASK                 0xFFFFFFFFUL                                  /**< Mask for SEMAPHORE_IPVERSION                */
#define _SEMAPHORE_IPVERSION_IPVERSION_SHIFT      0                                             /**< Shift value for SEMAPHORE_IPVERSION         */
#define _SEMAPHORE_IPVERSION_IPVERSION_MASK       0xFFFFFFFFUL                                  /**< Bit mask for SEMAPHORE_IPVERSION            */
#define _SEMAPHORE_IPVERSION_IPVERSION_DEFAULT    0x00000001UL                                  /**< Mode DEFAULT for SEMAPHORE_IPVERSION        */
#define SEMAPHORE_IPVERSION_IPVERSION_DEFAULT     (_SEMAPHORE_IPVERSION_IPVERSION_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_IPVERSION*/

/* Bit fields for SEMAPHORE LOCK0 */
#define _SEMAPHORE_LOCK0_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK0           */
#define _SEMAPHORE_LOCK0_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK0                    */
#define SEMAPHORE_LOCK0_LOCK0                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 0 Bit Register               */
#define _SEMAPHORE_LOCK0_LOCK0_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK0             */
#define _SEMAPHORE_LOCK0_LOCK0_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK0                */
#define _SEMAPHORE_LOCK0_LOCK0_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK0            */
#define SEMAPHORE_LOCK0_LOCK0_DEFAULT             (_SEMAPHORE_LOCK0_LOCK0_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK0    */

/* Bit fields for SEMAPHORE LOCK1 */
#define _SEMAPHORE_LOCK1_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK1           */
#define _SEMAPHORE_LOCK1_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK1                    */
#define SEMAPHORE_LOCK1_LOCK1                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 1 Bit Register               */
#define _SEMAPHORE_LOCK1_LOCK1_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK1             */
#define _SEMAPHORE_LOCK1_LOCK1_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK1                */
#define _SEMAPHORE_LOCK1_LOCK1_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK1            */
#define SEMAPHORE_LOCK1_LOCK1_DEFAULT             (_SEMAPHORE_LOCK1_LOCK1_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK1    */

/* Bit fields for SEMAPHORE LOCK2 */
#define _SEMAPHORE_LOCK2_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK2           */
#define _SEMAPHORE_LOCK2_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK2                    */
#define SEMAPHORE_LOCK2_LOCK2                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 2 Bit Register               */
#define _SEMAPHORE_LOCK2_LOCK2_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK2             */
#define _SEMAPHORE_LOCK2_LOCK2_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK2                */
#define _SEMAPHORE_LOCK2_LOCK2_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK2            */
#define SEMAPHORE_LOCK2_LOCK2_DEFAULT             (_SEMAPHORE_LOCK2_LOCK2_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK2    */

/* Bit fields for SEMAPHORE LOCK3 */
#define _SEMAPHORE_LOCK3_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK3           */
#define _SEMAPHORE_LOCK3_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK3                    */
#define SEMAPHORE_LOCK3_LOCK3                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 3 Bit Register               */
#define _SEMAPHORE_LOCK3_LOCK3_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK3             */
#define _SEMAPHORE_LOCK3_LOCK3_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK3                */
#define _SEMAPHORE_LOCK3_LOCK3_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK3            */
#define SEMAPHORE_LOCK3_LOCK3_DEFAULT             (_SEMAPHORE_LOCK3_LOCK3_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK3    */

/* Bit fields for SEMAPHORE LOCK4 */
#define _SEMAPHORE_LOCK4_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK4           */
#define _SEMAPHORE_LOCK4_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK4                    */
#define SEMAPHORE_LOCK4_LOCK4                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 4 Bit Register               */
#define _SEMAPHORE_LOCK4_LOCK4_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK4             */
#define _SEMAPHORE_LOCK4_LOCK4_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK4                */
#define _SEMAPHORE_LOCK4_LOCK4_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK4            */
#define SEMAPHORE_LOCK4_LOCK4_DEFAULT             (_SEMAPHORE_LOCK4_LOCK4_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK4    */

/* Bit fields for SEMAPHORE LOCK5 */
#define _SEMAPHORE_LOCK5_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK5           */
#define _SEMAPHORE_LOCK5_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK5                    */
#define SEMAPHORE_LOCK5_LOCK5                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 5 Bit Register               */
#define _SEMAPHORE_LOCK5_LOCK5_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK5             */
#define _SEMAPHORE_LOCK5_LOCK5_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK5                */
#define _SEMAPHORE_LOCK5_LOCK5_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK5            */
#define SEMAPHORE_LOCK5_LOCK5_DEFAULT             (_SEMAPHORE_LOCK5_LOCK5_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK5    */

/* Bit fields for SEMAPHORE LOCK6 */
#define _SEMAPHORE_LOCK6_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK6           */
#define _SEMAPHORE_LOCK6_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK6                    */
#define SEMAPHORE_LOCK6_LOCK6                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 6 Bit Register               */
#define _SEMAPHORE_LOCK6_LOCK6_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK6             */
#define _SEMAPHORE_LOCK6_LOCK6_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK6                */
#define _SEMAPHORE_LOCK6_LOCK6_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK6            */
#define SEMAPHORE_LOCK6_LOCK6_DEFAULT             (_SEMAPHORE_LOCK6_LOCK6_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK6    */

/* Bit fields for SEMAPHORE LOCK7 */
#define _SEMAPHORE_LOCK7_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK7           */
#define _SEMAPHORE_LOCK7_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK7                    */
#define SEMAPHORE_LOCK7_LOCK7                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 7 Bit Register               */
#define _SEMAPHORE_LOCK7_LOCK7_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK7             */
#define _SEMAPHORE_LOCK7_LOCK7_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK7                */
#define _SEMAPHORE_LOCK7_LOCK7_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK7            */
#define SEMAPHORE_LOCK7_LOCK7_DEFAULT             (_SEMAPHORE_LOCK7_LOCK7_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK7    */

/* Bit fields for SEMAPHORE LOCK8 */
#define _SEMAPHORE_LOCK8_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK8           */
#define _SEMAPHORE_LOCK8_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK8                    */
#define SEMAPHORE_LOCK8_LOCK8                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 8 Bit Register               */
#define _SEMAPHORE_LOCK8_LOCK8_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK8             */
#define _SEMAPHORE_LOCK8_LOCK8_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK8                */
#define _SEMAPHORE_LOCK8_LOCK8_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK8            */
#define SEMAPHORE_LOCK8_LOCK8_DEFAULT             (_SEMAPHORE_LOCK8_LOCK8_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK8    */

/* Bit fields for SEMAPHORE LOCK9 */
#define _SEMAPHORE_LOCK9_RESETVALUE               0x00000000UL                          /**< Default value for SEMAPHORE_LOCK9           */
#define _SEMAPHORE_LOCK9_MASK                     0x00000001UL                          /**< Mask for SEMAPHORE_LOCK9                    */
#define SEMAPHORE_LOCK9_LOCK9                     (0x1UL << 0)                          /**< SEMAPHORE LOCK 9 Bit Register               */
#define _SEMAPHORE_LOCK9_LOCK9_SHIFT              0                                     /**< Shift value for SEMAPHORE_LOCK9             */
#define _SEMAPHORE_LOCK9_LOCK9_MASK               0x1UL                                 /**< Bit mask for SEMAPHORE_LOCK9                */
#define _SEMAPHORE_LOCK9_LOCK9_DEFAULT            0x00000000UL                          /**< Mode DEFAULT for SEMAPHORE_LOCK9            */
#define SEMAPHORE_LOCK9_LOCK9_DEFAULT             (_SEMAPHORE_LOCK9_LOCK9_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK9    */

/* Bit fields for SEMAPHORE LOCK10 */
#define _SEMAPHORE_LOCK10_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK10          */
#define _SEMAPHORE_LOCK10_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK10                   */
#define SEMAPHORE_LOCK10_LOCK10                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 10 Bit Register              */
#define _SEMAPHORE_LOCK10_LOCK10_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK10            */
#define _SEMAPHORE_LOCK10_LOCK10_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK10               */
#define _SEMAPHORE_LOCK10_LOCK10_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK10           */
#define SEMAPHORE_LOCK10_LOCK10_DEFAULT           (_SEMAPHORE_LOCK10_LOCK10_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK10   */

/* Bit fields for SEMAPHORE LOCK11 */
#define _SEMAPHORE_LOCK11_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK11          */
#define _SEMAPHORE_LOCK11_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK11                   */
#define SEMAPHORE_LOCK11_LOCK11                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 11 Bit Register              */
#define _SEMAPHORE_LOCK11_LOCK11_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK11            */
#define _SEMAPHORE_LOCK11_LOCK11_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK11               */
#define _SEMAPHORE_LOCK11_LOCK11_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK11           */
#define SEMAPHORE_LOCK11_LOCK11_DEFAULT           (_SEMAPHORE_LOCK11_LOCK11_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK11   */

/* Bit fields for SEMAPHORE LOCK12 */
#define _SEMAPHORE_LOCK12_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK12          */
#define _SEMAPHORE_LOCK12_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK12                   */
#define SEMAPHORE_LOCK12_LOCK12                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 12 Bit Register              */
#define _SEMAPHORE_LOCK12_LOCK12_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK12            */
#define _SEMAPHORE_LOCK12_LOCK12_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK12               */
#define _SEMAPHORE_LOCK12_LOCK12_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK12           */
#define SEMAPHORE_LOCK12_LOCK12_DEFAULT           (_SEMAPHORE_LOCK12_LOCK12_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK12   */

/* Bit fields for SEMAPHORE LOCK13 */
#define _SEMAPHORE_LOCK13_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK13          */
#define _SEMAPHORE_LOCK13_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK13                   */
#define SEMAPHORE_LOCK13_LOCK13                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 13 Bit Register              */
#define _SEMAPHORE_LOCK13_LOCK13_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK13            */
#define _SEMAPHORE_LOCK13_LOCK13_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK13               */
#define _SEMAPHORE_LOCK13_LOCK13_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK13           */
#define SEMAPHORE_LOCK13_LOCK13_DEFAULT           (_SEMAPHORE_LOCK13_LOCK13_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK13   */

/* Bit fields for SEMAPHORE LOCK14 */
#define _SEMAPHORE_LOCK14_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK14          */
#define _SEMAPHORE_LOCK14_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK14                   */
#define SEMAPHORE_LOCK14_LOCK14                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 14 Bit Register              */
#define _SEMAPHORE_LOCK14_LOCK14_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK14            */
#define _SEMAPHORE_LOCK14_LOCK14_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK14               */
#define _SEMAPHORE_LOCK14_LOCK14_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK14           */
#define SEMAPHORE_LOCK14_LOCK14_DEFAULT           (_SEMAPHORE_LOCK14_LOCK14_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK14   */

/* Bit fields for SEMAPHORE LOCK15 */
#define _SEMAPHORE_LOCK15_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK15          */
#define _SEMAPHORE_LOCK15_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK15                   */
#define SEMAPHORE_LOCK15_LOCK15                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 15 Bit Register              */
#define _SEMAPHORE_LOCK15_LOCK15_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK15            */
#define _SEMAPHORE_LOCK15_LOCK15_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK15               */
#define _SEMAPHORE_LOCK15_LOCK15_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK15           */
#define SEMAPHORE_LOCK15_LOCK15_DEFAULT           (_SEMAPHORE_LOCK15_LOCK15_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK15   */

/* Bit fields for SEMAPHORE LOCK16 */
#define _SEMAPHORE_LOCK16_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK16          */
#define _SEMAPHORE_LOCK16_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK16                   */
#define SEMAPHORE_LOCK16_LOCK16                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 16 Bit Register              */
#define _SEMAPHORE_LOCK16_LOCK16_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK16            */
#define _SEMAPHORE_LOCK16_LOCK16_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK16               */
#define _SEMAPHORE_LOCK16_LOCK16_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK16           */
#define SEMAPHORE_LOCK16_LOCK16_DEFAULT           (_SEMAPHORE_LOCK16_LOCK16_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK16   */

/* Bit fields for SEMAPHORE LOCK17 */
#define _SEMAPHORE_LOCK17_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK17          */
#define _SEMAPHORE_LOCK17_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK17                   */
#define SEMAPHORE_LOCK17_LOCK17                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 17 Bit Register              */
#define _SEMAPHORE_LOCK17_LOCK17_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK17            */
#define _SEMAPHORE_LOCK17_LOCK17_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK17               */
#define _SEMAPHORE_LOCK17_LOCK17_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK17           */
#define SEMAPHORE_LOCK17_LOCK17_DEFAULT           (_SEMAPHORE_LOCK17_LOCK17_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK17   */

/* Bit fields for SEMAPHORE LOCK18 */
#define _SEMAPHORE_LOCK18_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK18          */
#define _SEMAPHORE_LOCK18_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK18                   */
#define SEMAPHORE_LOCK18_LOCK18                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 18 Bit Register              */
#define _SEMAPHORE_LOCK18_LOCK18_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK18            */
#define _SEMAPHORE_LOCK18_LOCK18_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK18               */
#define _SEMAPHORE_LOCK18_LOCK18_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK18           */
#define SEMAPHORE_LOCK18_LOCK18_DEFAULT           (_SEMAPHORE_LOCK18_LOCK18_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK18   */

/* Bit fields for SEMAPHORE LOCK19 */
#define _SEMAPHORE_LOCK19_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK19          */
#define _SEMAPHORE_LOCK19_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK19                   */
#define SEMAPHORE_LOCK19_LOCK19                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 19 Bit Register              */
#define _SEMAPHORE_LOCK19_LOCK19_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK19            */
#define _SEMAPHORE_LOCK19_LOCK19_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK19               */
#define _SEMAPHORE_LOCK19_LOCK19_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK19           */
#define SEMAPHORE_LOCK19_LOCK19_DEFAULT           (_SEMAPHORE_LOCK19_LOCK19_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK19   */

/* Bit fields for SEMAPHORE LOCK20 */
#define _SEMAPHORE_LOCK20_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK20          */
#define _SEMAPHORE_LOCK20_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK20                   */
#define SEMAPHORE_LOCK20_LOCK20                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 20 Bit Register              */
#define _SEMAPHORE_LOCK20_LOCK20_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK20            */
#define _SEMAPHORE_LOCK20_LOCK20_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK20               */
#define _SEMAPHORE_LOCK20_LOCK20_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK20           */
#define SEMAPHORE_LOCK20_LOCK20_DEFAULT           (_SEMAPHORE_LOCK20_LOCK20_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK20   */

/* Bit fields for SEMAPHORE LOCK21 */
#define _SEMAPHORE_LOCK21_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK21          */
#define _SEMAPHORE_LOCK21_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK21                   */
#define SEMAPHORE_LOCK21_LOCK21                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 21 Bit Register              */
#define _SEMAPHORE_LOCK21_LOCK21_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK21            */
#define _SEMAPHORE_LOCK21_LOCK21_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK21               */
#define _SEMAPHORE_LOCK21_LOCK21_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK21           */
#define SEMAPHORE_LOCK21_LOCK21_DEFAULT           (_SEMAPHORE_LOCK21_LOCK21_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK21   */

/* Bit fields for SEMAPHORE LOCK22 */
#define _SEMAPHORE_LOCK22_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK22          */
#define _SEMAPHORE_LOCK22_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK22                   */
#define SEMAPHORE_LOCK22_LOCK22                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 22 Bit Register              */
#define _SEMAPHORE_LOCK22_LOCK22_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK22            */
#define _SEMAPHORE_LOCK22_LOCK22_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK22               */
#define _SEMAPHORE_LOCK22_LOCK22_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK22           */
#define SEMAPHORE_LOCK22_LOCK22_DEFAULT           (_SEMAPHORE_LOCK22_LOCK22_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK22   */

/* Bit fields for SEMAPHORE LOCK23 */
#define _SEMAPHORE_LOCK23_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK23          */
#define _SEMAPHORE_LOCK23_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK23                   */
#define SEMAPHORE_LOCK23_LOCK23                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 23 Bit Register              */
#define _SEMAPHORE_LOCK23_LOCK23_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK23            */
#define _SEMAPHORE_LOCK23_LOCK23_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK23               */
#define _SEMAPHORE_LOCK23_LOCK23_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK23           */
#define SEMAPHORE_LOCK23_LOCK23_DEFAULT           (_SEMAPHORE_LOCK23_LOCK23_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK23   */

/* Bit fields for SEMAPHORE LOCK24 */
#define _SEMAPHORE_LOCK24_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK24          */
#define _SEMAPHORE_LOCK24_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK24                   */
#define SEMAPHORE_LOCK24_LOCK24                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 24 Bit Register              */
#define _SEMAPHORE_LOCK24_LOCK24_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK24            */
#define _SEMAPHORE_LOCK24_LOCK24_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK24               */
#define _SEMAPHORE_LOCK24_LOCK24_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK24           */
#define SEMAPHORE_LOCK24_LOCK24_DEFAULT           (_SEMAPHORE_LOCK24_LOCK24_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK24   */

/* Bit fields for SEMAPHORE LOCK25 */
#define _SEMAPHORE_LOCK25_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK25          */
#define _SEMAPHORE_LOCK25_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK25                   */
#define SEMAPHORE_LOCK25_LOCK25                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 25 Bit Register              */
#define _SEMAPHORE_LOCK25_LOCK25_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK25            */
#define _SEMAPHORE_LOCK25_LOCK25_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK25               */
#define _SEMAPHORE_LOCK25_LOCK25_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK25           */
#define SEMAPHORE_LOCK25_LOCK25_DEFAULT           (_SEMAPHORE_LOCK25_LOCK25_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK25   */

/* Bit fields for SEMAPHORE LOCK26 */
#define _SEMAPHORE_LOCK26_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK26          */
#define _SEMAPHORE_LOCK26_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK26                   */
#define SEMAPHORE_LOCK26_LOCK26                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 26 Bit Register              */
#define _SEMAPHORE_LOCK26_LOCK26_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK26            */
#define _SEMAPHORE_LOCK26_LOCK26_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK26               */
#define _SEMAPHORE_LOCK26_LOCK26_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK26           */
#define SEMAPHORE_LOCK26_LOCK26_DEFAULT           (_SEMAPHORE_LOCK26_LOCK26_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK26   */

/* Bit fields for SEMAPHORE LOCK27 */
#define _SEMAPHORE_LOCK27_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK27          */
#define _SEMAPHORE_LOCK27_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK27                   */
#define SEMAPHORE_LOCK27_LOCK27                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 27 Bit Register              */
#define _SEMAPHORE_LOCK27_LOCK27_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK27            */
#define _SEMAPHORE_LOCK27_LOCK27_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK27               */
#define _SEMAPHORE_LOCK27_LOCK27_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK27           */
#define SEMAPHORE_LOCK27_LOCK27_DEFAULT           (_SEMAPHORE_LOCK27_LOCK27_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK27   */

/* Bit fields for SEMAPHORE LOCK28 */
#define _SEMAPHORE_LOCK28_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK28          */
#define _SEMAPHORE_LOCK28_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK28                   */
#define SEMAPHORE_LOCK28_LOCK28                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 28 Bit Register              */
#define _SEMAPHORE_LOCK28_LOCK28_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK28            */
#define _SEMAPHORE_LOCK28_LOCK28_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK28               */
#define _SEMAPHORE_LOCK28_LOCK28_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK28           */
#define SEMAPHORE_LOCK28_LOCK28_DEFAULT           (_SEMAPHORE_LOCK28_LOCK28_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK28   */

/* Bit fields for SEMAPHORE LOCK29 */
#define _SEMAPHORE_LOCK29_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK29          */
#define _SEMAPHORE_LOCK29_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK29                   */
#define SEMAPHORE_LOCK29_LOCK29                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 29 Bit Register              */
#define _SEMAPHORE_LOCK29_LOCK29_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK29            */
#define _SEMAPHORE_LOCK29_LOCK29_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK29               */
#define _SEMAPHORE_LOCK29_LOCK29_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK29           */
#define SEMAPHORE_LOCK29_LOCK29_DEFAULT           (_SEMAPHORE_LOCK29_LOCK29_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK29   */

/* Bit fields for SEMAPHORE LOCK30 */
#define _SEMAPHORE_LOCK30_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK30          */
#define _SEMAPHORE_LOCK30_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK30                   */
#define SEMAPHORE_LOCK30_LOCK30                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 30 Bit Register              */
#define _SEMAPHORE_LOCK30_LOCK30_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK30            */
#define _SEMAPHORE_LOCK30_LOCK30_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK30               */
#define _SEMAPHORE_LOCK30_LOCK30_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK30           */
#define SEMAPHORE_LOCK30_LOCK30_DEFAULT           (_SEMAPHORE_LOCK30_LOCK30_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK30   */

/* Bit fields for SEMAPHORE LOCK31 */
#define _SEMAPHORE_LOCK31_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK31          */
#define _SEMAPHORE_LOCK31_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK31                   */
#define SEMAPHORE_LOCK31_LOCK31                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 31 Bit Register              */
#define _SEMAPHORE_LOCK31_LOCK31_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK31            */
#define _SEMAPHORE_LOCK31_LOCK31_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK31               */
#define _SEMAPHORE_LOCK31_LOCK31_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK31           */
#define SEMAPHORE_LOCK31_LOCK31_DEFAULT           (_SEMAPHORE_LOCK31_LOCK31_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK31   */

/* Bit fields for SEMAPHORE LOCK32 */
#define _SEMAPHORE_LOCK32_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK32          */
#define _SEMAPHORE_LOCK32_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK32                   */
#define SEMAPHORE_LOCK32_LOCK32                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 32 Bit Register              */
#define _SEMAPHORE_LOCK32_LOCK32_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK32            */
#define _SEMAPHORE_LOCK32_LOCK32_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK32               */
#define _SEMAPHORE_LOCK32_LOCK32_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK32           */
#define SEMAPHORE_LOCK32_LOCK32_DEFAULT           (_SEMAPHORE_LOCK32_LOCK32_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK32   */

/* Bit fields for SEMAPHORE LOCK33 */
#define _SEMAPHORE_LOCK33_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK33          */
#define _SEMAPHORE_LOCK33_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK33                   */
#define SEMAPHORE_LOCK33_LOCK33                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 33 Bit Register              */
#define _SEMAPHORE_LOCK33_LOCK33_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK33            */
#define _SEMAPHORE_LOCK33_LOCK33_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK33               */
#define _SEMAPHORE_LOCK33_LOCK33_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK33           */
#define SEMAPHORE_LOCK33_LOCK33_DEFAULT           (_SEMAPHORE_LOCK33_LOCK33_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK33   */

/* Bit fields for SEMAPHORE LOCK34 */
#define _SEMAPHORE_LOCK34_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK34          */
#define _SEMAPHORE_LOCK34_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK34                   */
#define SEMAPHORE_LOCK34_LOCK34                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 34 Bit Register              */
#define _SEMAPHORE_LOCK34_LOCK34_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK34            */
#define _SEMAPHORE_LOCK34_LOCK34_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK34               */
#define _SEMAPHORE_LOCK34_LOCK34_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK34           */
#define SEMAPHORE_LOCK34_LOCK34_DEFAULT           (_SEMAPHORE_LOCK34_LOCK34_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK34   */

/* Bit fields for SEMAPHORE LOCK35 */
#define _SEMAPHORE_LOCK35_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK35          */
#define _SEMAPHORE_LOCK35_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK35                   */
#define SEMAPHORE_LOCK35_LOCK35                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 35 Bit Register              */
#define _SEMAPHORE_LOCK35_LOCK35_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK35            */
#define _SEMAPHORE_LOCK35_LOCK35_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK35               */
#define _SEMAPHORE_LOCK35_LOCK35_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK35           */
#define SEMAPHORE_LOCK35_LOCK35_DEFAULT           (_SEMAPHORE_LOCK35_LOCK35_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK35   */

/* Bit fields for SEMAPHORE LOCK36 */
#define _SEMAPHORE_LOCK36_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK36          */
#define _SEMAPHORE_LOCK36_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK36                   */
#define SEMAPHORE_LOCK36_LOCK36                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 36 Bit Register              */
#define _SEMAPHORE_LOCK36_LOCK36_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK36            */
#define _SEMAPHORE_LOCK36_LOCK36_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK36               */
#define _SEMAPHORE_LOCK36_LOCK36_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK36           */
#define SEMAPHORE_LOCK36_LOCK36_DEFAULT           (_SEMAPHORE_LOCK36_LOCK36_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK36   */

/* Bit fields for SEMAPHORE LOCK37 */
#define _SEMAPHORE_LOCK37_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK37          */
#define _SEMAPHORE_LOCK37_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK37                   */
#define SEMAPHORE_LOCK37_LOCK37                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 37 Bit Register              */
#define _SEMAPHORE_LOCK37_LOCK37_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK37            */
#define _SEMAPHORE_LOCK37_LOCK37_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK37               */
#define _SEMAPHORE_LOCK37_LOCK37_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK37           */
#define SEMAPHORE_LOCK37_LOCK37_DEFAULT           (_SEMAPHORE_LOCK37_LOCK37_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK37   */

/* Bit fields for SEMAPHORE LOCK38 */
#define _SEMAPHORE_LOCK38_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK38          */
#define _SEMAPHORE_LOCK38_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK38                   */
#define SEMAPHORE_LOCK38_LOCK38                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 38 Bit Register              */
#define _SEMAPHORE_LOCK38_LOCK38_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK38            */
#define _SEMAPHORE_LOCK38_LOCK38_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK38               */
#define _SEMAPHORE_LOCK38_LOCK38_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK38           */
#define SEMAPHORE_LOCK38_LOCK38_DEFAULT           (_SEMAPHORE_LOCK38_LOCK38_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK38   */

/* Bit fields for SEMAPHORE LOCK39 */
#define _SEMAPHORE_LOCK39_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK39          */
#define _SEMAPHORE_LOCK39_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK39                   */
#define SEMAPHORE_LOCK39_LOCK39                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 39 Bit Register              */
#define _SEMAPHORE_LOCK39_LOCK39_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK39            */
#define _SEMAPHORE_LOCK39_LOCK39_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK39               */
#define _SEMAPHORE_LOCK39_LOCK39_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK39           */
#define SEMAPHORE_LOCK39_LOCK39_DEFAULT           (_SEMAPHORE_LOCK39_LOCK39_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK39   */

/* Bit fields for SEMAPHORE LOCK40 */
#define _SEMAPHORE_LOCK40_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK40          */
#define _SEMAPHORE_LOCK40_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK40                   */
#define SEMAPHORE_LOCK40_LOCK40                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 40 Bit Register              */
#define _SEMAPHORE_LOCK40_LOCK40_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK40            */
#define _SEMAPHORE_LOCK40_LOCK40_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK40               */
#define _SEMAPHORE_LOCK40_LOCK40_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK40           */
#define SEMAPHORE_LOCK40_LOCK40_DEFAULT           (_SEMAPHORE_LOCK40_LOCK40_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK40   */

/* Bit fields for SEMAPHORE LOCK41 */
#define _SEMAPHORE_LOCK41_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK41          */
#define _SEMAPHORE_LOCK41_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK41                   */
#define SEMAPHORE_LOCK41_LOCK41                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 41 Bit Register              */
#define _SEMAPHORE_LOCK41_LOCK41_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK41            */
#define _SEMAPHORE_LOCK41_LOCK41_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK41               */
#define _SEMAPHORE_LOCK41_LOCK41_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK41           */
#define SEMAPHORE_LOCK41_LOCK41_DEFAULT           (_SEMAPHORE_LOCK41_LOCK41_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK41   */

/* Bit fields for SEMAPHORE LOCK42 */
#define _SEMAPHORE_LOCK42_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK42          */
#define _SEMAPHORE_LOCK42_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK42                   */
#define SEMAPHORE_LOCK42_LOCK42                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 42 Bit Register              */
#define _SEMAPHORE_LOCK42_LOCK42_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK42            */
#define _SEMAPHORE_LOCK42_LOCK42_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK42               */
#define _SEMAPHORE_LOCK42_LOCK42_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK42           */
#define SEMAPHORE_LOCK42_LOCK42_DEFAULT           (_SEMAPHORE_LOCK42_LOCK42_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK42   */

/* Bit fields for SEMAPHORE LOCK43 */
#define _SEMAPHORE_LOCK43_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK43          */
#define _SEMAPHORE_LOCK43_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK43                   */
#define SEMAPHORE_LOCK43_LOCK43                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 43 Bit Register              */
#define _SEMAPHORE_LOCK43_LOCK43_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK43            */
#define _SEMAPHORE_LOCK43_LOCK43_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK43               */
#define _SEMAPHORE_LOCK43_LOCK43_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK43           */
#define SEMAPHORE_LOCK43_LOCK43_DEFAULT           (_SEMAPHORE_LOCK43_LOCK43_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK43   */

/* Bit fields for SEMAPHORE LOCK44 */
#define _SEMAPHORE_LOCK44_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK44          */
#define _SEMAPHORE_LOCK44_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK44                   */
#define SEMAPHORE_LOCK44_LOCK44                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 44 Bit Register              */
#define _SEMAPHORE_LOCK44_LOCK44_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK44            */
#define _SEMAPHORE_LOCK44_LOCK44_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK44               */
#define _SEMAPHORE_LOCK44_LOCK44_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK44           */
#define SEMAPHORE_LOCK44_LOCK44_DEFAULT           (_SEMAPHORE_LOCK44_LOCK44_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK44   */

/* Bit fields for SEMAPHORE LOCK45 */
#define _SEMAPHORE_LOCK45_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK45          */
#define _SEMAPHORE_LOCK45_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK45                   */
#define SEMAPHORE_LOCK45_LOCK45                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 45 Bit Register              */
#define _SEMAPHORE_LOCK45_LOCK45_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK45            */
#define _SEMAPHORE_LOCK45_LOCK45_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK45               */
#define _SEMAPHORE_LOCK45_LOCK45_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK45           */
#define SEMAPHORE_LOCK45_LOCK45_DEFAULT           (_SEMAPHORE_LOCK45_LOCK45_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK45   */

/* Bit fields for SEMAPHORE LOCK46 */
#define _SEMAPHORE_LOCK46_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK46          */
#define _SEMAPHORE_LOCK46_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK46                   */
#define SEMAPHORE_LOCK46_LOCK46                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 46 Bit Register              */
#define _SEMAPHORE_LOCK46_LOCK46_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK46            */
#define _SEMAPHORE_LOCK46_LOCK46_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK46               */
#define _SEMAPHORE_LOCK46_LOCK46_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK46           */
#define SEMAPHORE_LOCK46_LOCK46_DEFAULT           (_SEMAPHORE_LOCK46_LOCK46_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK46   */

/* Bit fields for SEMAPHORE LOCK47 */
#define _SEMAPHORE_LOCK47_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK47          */
#define _SEMAPHORE_LOCK47_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK47                   */
#define SEMAPHORE_LOCK47_LOCK47                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 47 Bit Register              */
#define _SEMAPHORE_LOCK47_LOCK47_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK47            */
#define _SEMAPHORE_LOCK47_LOCK47_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK47               */
#define _SEMAPHORE_LOCK47_LOCK47_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK47           */
#define SEMAPHORE_LOCK47_LOCK47_DEFAULT           (_SEMAPHORE_LOCK47_LOCK47_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK47   */

/* Bit fields for SEMAPHORE LOCK48 */
#define _SEMAPHORE_LOCK48_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK48          */
#define _SEMAPHORE_LOCK48_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK48                   */
#define SEMAPHORE_LOCK48_LOCK48                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 48 Bit Register              */
#define _SEMAPHORE_LOCK48_LOCK48_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK48            */
#define _SEMAPHORE_LOCK48_LOCK48_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK48               */
#define _SEMAPHORE_LOCK48_LOCK48_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK48           */
#define SEMAPHORE_LOCK48_LOCK48_DEFAULT           (_SEMAPHORE_LOCK48_LOCK48_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK48   */

/* Bit fields for SEMAPHORE LOCK49 */
#define _SEMAPHORE_LOCK49_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK49          */
#define _SEMAPHORE_LOCK49_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK49                   */
#define SEMAPHORE_LOCK49_LOCK49                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 49 Bit Register              */
#define _SEMAPHORE_LOCK49_LOCK49_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK49            */
#define _SEMAPHORE_LOCK49_LOCK49_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK49               */
#define _SEMAPHORE_LOCK49_LOCK49_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK49           */
#define SEMAPHORE_LOCK49_LOCK49_DEFAULT           (_SEMAPHORE_LOCK49_LOCK49_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK49   */

/* Bit fields for SEMAPHORE LOCK50 */
#define _SEMAPHORE_LOCK50_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK50          */
#define _SEMAPHORE_LOCK50_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK50                   */
#define SEMAPHORE_LOCK50_LOCK50                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 50 Bit Register              */
#define _SEMAPHORE_LOCK50_LOCK50_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK50            */
#define _SEMAPHORE_LOCK50_LOCK50_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK50               */
#define _SEMAPHORE_LOCK50_LOCK50_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK50           */
#define SEMAPHORE_LOCK50_LOCK50_DEFAULT           (_SEMAPHORE_LOCK50_LOCK50_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK50   */

/* Bit fields for SEMAPHORE LOCK51 */
#define _SEMAPHORE_LOCK51_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK51          */
#define _SEMAPHORE_LOCK51_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK51                   */
#define SEMAPHORE_LOCK51_LOCK51                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 51 Bit Register              */
#define _SEMAPHORE_LOCK51_LOCK51_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK51            */
#define _SEMAPHORE_LOCK51_LOCK51_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK51               */
#define _SEMAPHORE_LOCK51_LOCK51_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK51           */
#define SEMAPHORE_LOCK51_LOCK51_DEFAULT           (_SEMAPHORE_LOCK51_LOCK51_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK51   */

/* Bit fields for SEMAPHORE LOCK52 */
#define _SEMAPHORE_LOCK52_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK52          */
#define _SEMAPHORE_LOCK52_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK52                   */
#define SEMAPHORE_LOCK52_LOCK52                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 52 Bit Register              */
#define _SEMAPHORE_LOCK52_LOCK52_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK52            */
#define _SEMAPHORE_LOCK52_LOCK52_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK52               */
#define _SEMAPHORE_LOCK52_LOCK52_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK52           */
#define SEMAPHORE_LOCK52_LOCK52_DEFAULT           (_SEMAPHORE_LOCK52_LOCK52_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK52   */

/* Bit fields for SEMAPHORE LOCK53 */
#define _SEMAPHORE_LOCK53_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK53          */
#define _SEMAPHORE_LOCK53_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK53                   */
#define SEMAPHORE_LOCK53_LOCK53                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 53 Bit Register              */
#define _SEMAPHORE_LOCK53_LOCK53_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK53            */
#define _SEMAPHORE_LOCK53_LOCK53_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK53               */
#define _SEMAPHORE_LOCK53_LOCK53_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK53           */
#define SEMAPHORE_LOCK53_LOCK53_DEFAULT           (_SEMAPHORE_LOCK53_LOCK53_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK53   */

/* Bit fields for SEMAPHORE LOCK54 */
#define _SEMAPHORE_LOCK54_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK54          */
#define _SEMAPHORE_LOCK54_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK54                   */
#define SEMAPHORE_LOCK54_LOCK54                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 54 Bit Register              */
#define _SEMAPHORE_LOCK54_LOCK54_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK54            */
#define _SEMAPHORE_LOCK54_LOCK54_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK54               */
#define _SEMAPHORE_LOCK54_LOCK54_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK54           */
#define SEMAPHORE_LOCK54_LOCK54_DEFAULT           (_SEMAPHORE_LOCK54_LOCK54_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK54   */

/* Bit fields for SEMAPHORE LOCK55 */
#define _SEMAPHORE_LOCK55_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK55          */
#define _SEMAPHORE_LOCK55_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK55                   */
#define SEMAPHORE_LOCK55_LOCK55                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 55 Bit Register              */
#define _SEMAPHORE_LOCK55_LOCK55_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK55            */
#define _SEMAPHORE_LOCK55_LOCK55_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK55               */
#define _SEMAPHORE_LOCK55_LOCK55_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK55           */
#define SEMAPHORE_LOCK55_LOCK55_DEFAULT           (_SEMAPHORE_LOCK55_LOCK55_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK55   */

/* Bit fields for SEMAPHORE LOCK56 */
#define _SEMAPHORE_LOCK56_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK56          */
#define _SEMAPHORE_LOCK56_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK56                   */
#define SEMAPHORE_LOCK56_LOCK56                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 56 Bit Register              */
#define _SEMAPHORE_LOCK56_LOCK56_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK56            */
#define _SEMAPHORE_LOCK56_LOCK56_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK56               */
#define _SEMAPHORE_LOCK56_LOCK56_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK56           */
#define SEMAPHORE_LOCK56_LOCK56_DEFAULT           (_SEMAPHORE_LOCK56_LOCK56_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK56   */

/* Bit fields for SEMAPHORE LOCK57 */
#define _SEMAPHORE_LOCK57_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK57          */
#define _SEMAPHORE_LOCK57_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK57                   */
#define SEMAPHORE_LOCK57_LOCK57                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 57 Bit Register              */
#define _SEMAPHORE_LOCK57_LOCK57_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK57            */
#define _SEMAPHORE_LOCK57_LOCK57_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK57               */
#define _SEMAPHORE_LOCK57_LOCK57_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK57           */
#define SEMAPHORE_LOCK57_LOCK57_DEFAULT           (_SEMAPHORE_LOCK57_LOCK57_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK57   */

/* Bit fields for SEMAPHORE LOCK58 */
#define _SEMAPHORE_LOCK58_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK58          */
#define _SEMAPHORE_LOCK58_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK58                   */
#define SEMAPHORE_LOCK58_LOCK58                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 58 Bit Register              */
#define _SEMAPHORE_LOCK58_LOCK58_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK58            */
#define _SEMAPHORE_LOCK58_LOCK58_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK58               */
#define _SEMAPHORE_LOCK58_LOCK58_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK58           */
#define SEMAPHORE_LOCK58_LOCK58_DEFAULT           (_SEMAPHORE_LOCK58_LOCK58_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK58   */

/* Bit fields for SEMAPHORE LOCK59 */
#define _SEMAPHORE_LOCK59_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK59          */
#define _SEMAPHORE_LOCK59_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK59                   */
#define SEMAPHORE_LOCK59_LOCK59                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 59 Bit Register              */
#define _SEMAPHORE_LOCK59_LOCK59_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK59            */
#define _SEMAPHORE_LOCK59_LOCK59_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK59               */
#define _SEMAPHORE_LOCK59_LOCK59_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK59           */
#define SEMAPHORE_LOCK59_LOCK59_DEFAULT           (_SEMAPHORE_LOCK59_LOCK59_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK59   */

/* Bit fields for SEMAPHORE LOCK60 */
#define _SEMAPHORE_LOCK60_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK60          */
#define _SEMAPHORE_LOCK60_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK60                   */
#define SEMAPHORE_LOCK60_LOCK60                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 60 Bit Register              */
#define _SEMAPHORE_LOCK60_LOCK60_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK60            */
#define _SEMAPHORE_LOCK60_LOCK60_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK60               */
#define _SEMAPHORE_LOCK60_LOCK60_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK60           */
#define SEMAPHORE_LOCK60_LOCK60_DEFAULT           (_SEMAPHORE_LOCK60_LOCK60_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK60   */

/* Bit fields for SEMAPHORE LOCK61 */
#define _SEMAPHORE_LOCK61_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK61          */
#define _SEMAPHORE_LOCK61_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK61                   */
#define SEMAPHORE_LOCK61_LOCK61                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 61 Bit Register              */
#define _SEMAPHORE_LOCK61_LOCK61_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK61            */
#define _SEMAPHORE_LOCK61_LOCK61_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK61               */
#define _SEMAPHORE_LOCK61_LOCK61_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK61           */
#define SEMAPHORE_LOCK61_LOCK61_DEFAULT           (_SEMAPHORE_LOCK61_LOCK61_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK61   */

/* Bit fields for SEMAPHORE LOCK62 */
#define _SEMAPHORE_LOCK62_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK62          */
#define _SEMAPHORE_LOCK62_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK62                   */
#define SEMAPHORE_LOCK62_LOCK62                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 62 Bit Register              */
#define _SEMAPHORE_LOCK62_LOCK62_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK62            */
#define _SEMAPHORE_LOCK62_LOCK62_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK62               */
#define _SEMAPHORE_LOCK62_LOCK62_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK62           */
#define SEMAPHORE_LOCK62_LOCK62_DEFAULT           (_SEMAPHORE_LOCK62_LOCK62_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK62   */

/* Bit fields for SEMAPHORE LOCK63 */
#define _SEMAPHORE_LOCK63_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK63          */
#define _SEMAPHORE_LOCK63_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK63                   */
#define SEMAPHORE_LOCK63_LOCK63                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 63 Bit Register              */
#define _SEMAPHORE_LOCK63_LOCK63_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK63            */
#define _SEMAPHORE_LOCK63_LOCK63_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK63               */
#define _SEMAPHORE_LOCK63_LOCK63_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK63           */
#define SEMAPHORE_LOCK63_LOCK63_DEFAULT           (_SEMAPHORE_LOCK63_LOCK63_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK63   */

/* Bit fields for SEMAPHORE LOCK64 */
#define _SEMAPHORE_LOCK64_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK64          */
#define _SEMAPHORE_LOCK64_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK64                   */
#define SEMAPHORE_LOCK64_LOCK64                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 64 Bit Register              */
#define _SEMAPHORE_LOCK64_LOCK64_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK64            */
#define _SEMAPHORE_LOCK64_LOCK64_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK64               */
#define _SEMAPHORE_LOCK64_LOCK64_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK64           */
#define SEMAPHORE_LOCK64_LOCK64_DEFAULT           (_SEMAPHORE_LOCK64_LOCK64_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK64   */

/* Bit fields for SEMAPHORE LOCK65 */
#define _SEMAPHORE_LOCK65_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK65          */
#define _SEMAPHORE_LOCK65_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK65                   */
#define SEMAPHORE_LOCK65_LOCK65                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 65 Bit Register              */
#define _SEMAPHORE_LOCK65_LOCK65_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK65            */
#define _SEMAPHORE_LOCK65_LOCK65_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK65               */
#define _SEMAPHORE_LOCK65_LOCK65_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK65           */
#define SEMAPHORE_LOCK65_LOCK65_DEFAULT           (_SEMAPHORE_LOCK65_LOCK65_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK65   */

/* Bit fields for SEMAPHORE LOCK66 */
#define _SEMAPHORE_LOCK66_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK66          */
#define _SEMAPHORE_LOCK66_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK66                   */
#define SEMAPHORE_LOCK66_LOCK66                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 66 Bit Register              */
#define _SEMAPHORE_LOCK66_LOCK66_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK66            */
#define _SEMAPHORE_LOCK66_LOCK66_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK66               */
#define _SEMAPHORE_LOCK66_LOCK66_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK66           */
#define SEMAPHORE_LOCK66_LOCK66_DEFAULT           (_SEMAPHORE_LOCK66_LOCK66_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK66   */

/* Bit fields for SEMAPHORE LOCK67 */
#define _SEMAPHORE_LOCK67_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK67          */
#define _SEMAPHORE_LOCK67_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK67                   */
#define SEMAPHORE_LOCK67_LOCK67                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 67 Bit Register              */
#define _SEMAPHORE_LOCK67_LOCK67_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK67            */
#define _SEMAPHORE_LOCK67_LOCK67_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK67               */
#define _SEMAPHORE_LOCK67_LOCK67_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK67           */
#define SEMAPHORE_LOCK67_LOCK67_DEFAULT           (_SEMAPHORE_LOCK67_LOCK67_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK67   */

/* Bit fields for SEMAPHORE LOCK68 */
#define _SEMAPHORE_LOCK68_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK68          */
#define _SEMAPHORE_LOCK68_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK68                   */
#define SEMAPHORE_LOCK68_LOCK68                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 68 Bit Register              */
#define _SEMAPHORE_LOCK68_LOCK68_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK68            */
#define _SEMAPHORE_LOCK68_LOCK68_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK68               */
#define _SEMAPHORE_LOCK68_LOCK68_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK68           */
#define SEMAPHORE_LOCK68_LOCK68_DEFAULT           (_SEMAPHORE_LOCK68_LOCK68_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK68   */

/* Bit fields for SEMAPHORE LOCK69 */
#define _SEMAPHORE_LOCK69_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK69          */
#define _SEMAPHORE_LOCK69_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK69                   */
#define SEMAPHORE_LOCK69_LOCK69                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 69 Bit Register              */
#define _SEMAPHORE_LOCK69_LOCK69_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK69            */
#define _SEMAPHORE_LOCK69_LOCK69_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK69               */
#define _SEMAPHORE_LOCK69_LOCK69_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK69           */
#define SEMAPHORE_LOCK69_LOCK69_DEFAULT           (_SEMAPHORE_LOCK69_LOCK69_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK69   */

/* Bit fields for SEMAPHORE LOCK70 */
#define _SEMAPHORE_LOCK70_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK70          */
#define _SEMAPHORE_LOCK70_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK70                   */
#define SEMAPHORE_LOCK70_LOCK70                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 70 Bit Register              */
#define _SEMAPHORE_LOCK70_LOCK70_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK70            */
#define _SEMAPHORE_LOCK70_LOCK70_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK70               */
#define _SEMAPHORE_LOCK70_LOCK70_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK70           */
#define SEMAPHORE_LOCK70_LOCK70_DEFAULT           (_SEMAPHORE_LOCK70_LOCK70_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK70   */

/* Bit fields for SEMAPHORE LOCK71 */
#define _SEMAPHORE_LOCK71_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK71          */
#define _SEMAPHORE_LOCK71_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK71                   */
#define SEMAPHORE_LOCK71_LOCK71                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 71 Bit Register              */
#define _SEMAPHORE_LOCK71_LOCK71_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK71            */
#define _SEMAPHORE_LOCK71_LOCK71_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK71               */
#define _SEMAPHORE_LOCK71_LOCK71_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK71           */
#define SEMAPHORE_LOCK71_LOCK71_DEFAULT           (_SEMAPHORE_LOCK71_LOCK71_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK71   */

/* Bit fields for SEMAPHORE LOCK72 */
#define _SEMAPHORE_LOCK72_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK72          */
#define _SEMAPHORE_LOCK72_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK72                   */
#define SEMAPHORE_LOCK72_LOCK72                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 72 Bit Register              */
#define _SEMAPHORE_LOCK72_LOCK72_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK72            */
#define _SEMAPHORE_LOCK72_LOCK72_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK72               */
#define _SEMAPHORE_LOCK72_LOCK72_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK72           */
#define SEMAPHORE_LOCK72_LOCK72_DEFAULT           (_SEMAPHORE_LOCK72_LOCK72_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK72   */

/* Bit fields for SEMAPHORE LOCK73 */
#define _SEMAPHORE_LOCK73_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK73          */
#define _SEMAPHORE_LOCK73_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK73                   */
#define SEMAPHORE_LOCK73_LOCK73                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 73 Bit Register              */
#define _SEMAPHORE_LOCK73_LOCK73_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK73            */
#define _SEMAPHORE_LOCK73_LOCK73_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK73               */
#define _SEMAPHORE_LOCK73_LOCK73_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK73           */
#define SEMAPHORE_LOCK73_LOCK73_DEFAULT           (_SEMAPHORE_LOCK73_LOCK73_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK73   */

/* Bit fields for SEMAPHORE LOCK74 */
#define _SEMAPHORE_LOCK74_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK74          */
#define _SEMAPHORE_LOCK74_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK74                   */
#define SEMAPHORE_LOCK74_LOCK74                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 74 Bit Register              */
#define _SEMAPHORE_LOCK74_LOCK74_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK74            */
#define _SEMAPHORE_LOCK74_LOCK74_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK74               */
#define _SEMAPHORE_LOCK74_LOCK74_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK74           */
#define SEMAPHORE_LOCK74_LOCK74_DEFAULT           (_SEMAPHORE_LOCK74_LOCK74_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK74   */

/* Bit fields for SEMAPHORE LOCK75 */
#define _SEMAPHORE_LOCK75_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK75          */
#define _SEMAPHORE_LOCK75_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK75                   */
#define SEMAPHORE_LOCK75_LOCK75                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 75 Bit Register              */
#define _SEMAPHORE_LOCK75_LOCK75_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK75            */
#define _SEMAPHORE_LOCK75_LOCK75_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK75               */
#define _SEMAPHORE_LOCK75_LOCK75_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK75           */
#define SEMAPHORE_LOCK75_LOCK75_DEFAULT           (_SEMAPHORE_LOCK75_LOCK75_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK75   */

/* Bit fields for SEMAPHORE LOCK76 */
#define _SEMAPHORE_LOCK76_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK76          */
#define _SEMAPHORE_LOCK76_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK76                   */
#define SEMAPHORE_LOCK76_LOCK76                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 76 Bit Register              */
#define _SEMAPHORE_LOCK76_LOCK76_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK76            */
#define _SEMAPHORE_LOCK76_LOCK76_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK76               */
#define _SEMAPHORE_LOCK76_LOCK76_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK76           */
#define SEMAPHORE_LOCK76_LOCK76_DEFAULT           (_SEMAPHORE_LOCK76_LOCK76_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK76   */

/* Bit fields for SEMAPHORE LOCK77 */
#define _SEMAPHORE_LOCK77_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK77          */
#define _SEMAPHORE_LOCK77_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK77                   */
#define SEMAPHORE_LOCK77_LOCK77                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 77 Bit Register              */
#define _SEMAPHORE_LOCK77_LOCK77_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK77            */
#define _SEMAPHORE_LOCK77_LOCK77_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK77               */
#define _SEMAPHORE_LOCK77_LOCK77_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK77           */
#define SEMAPHORE_LOCK77_LOCK77_DEFAULT           (_SEMAPHORE_LOCK77_LOCK77_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK77   */

/* Bit fields for SEMAPHORE LOCK78 */
#define _SEMAPHORE_LOCK78_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK78          */
#define _SEMAPHORE_LOCK78_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK78                   */
#define SEMAPHORE_LOCK78_LOCK78                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 78 Bit Register              */
#define _SEMAPHORE_LOCK78_LOCK78_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK78            */
#define _SEMAPHORE_LOCK78_LOCK78_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK78               */
#define _SEMAPHORE_LOCK78_LOCK78_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK78           */
#define SEMAPHORE_LOCK78_LOCK78_DEFAULT           (_SEMAPHORE_LOCK78_LOCK78_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK78   */

/* Bit fields for SEMAPHORE LOCK79 */
#define _SEMAPHORE_LOCK79_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK79          */
#define _SEMAPHORE_LOCK79_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK79                   */
#define SEMAPHORE_LOCK79_LOCK79                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 79 Bit Register              */
#define _SEMAPHORE_LOCK79_LOCK79_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK79            */
#define _SEMAPHORE_LOCK79_LOCK79_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK79               */
#define _SEMAPHORE_LOCK79_LOCK79_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK79           */
#define SEMAPHORE_LOCK79_LOCK79_DEFAULT           (_SEMAPHORE_LOCK79_LOCK79_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK79   */

/* Bit fields for SEMAPHORE LOCK80 */
#define _SEMAPHORE_LOCK80_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK80          */
#define _SEMAPHORE_LOCK80_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK80                   */
#define SEMAPHORE_LOCK80_LOCK80                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 80 Bit Register              */
#define _SEMAPHORE_LOCK80_LOCK80_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK80            */
#define _SEMAPHORE_LOCK80_LOCK80_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK80               */
#define _SEMAPHORE_LOCK80_LOCK80_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK80           */
#define SEMAPHORE_LOCK80_LOCK80_DEFAULT           (_SEMAPHORE_LOCK80_LOCK80_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK80   */

/* Bit fields for SEMAPHORE LOCK81 */
#define _SEMAPHORE_LOCK81_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK81          */
#define _SEMAPHORE_LOCK81_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK81                   */
#define SEMAPHORE_LOCK81_LOCK81                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 81 Bit Register              */
#define _SEMAPHORE_LOCK81_LOCK81_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK81            */
#define _SEMAPHORE_LOCK81_LOCK81_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK81               */
#define _SEMAPHORE_LOCK81_LOCK81_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK81           */
#define SEMAPHORE_LOCK81_LOCK81_DEFAULT           (_SEMAPHORE_LOCK81_LOCK81_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK81   */

/* Bit fields for SEMAPHORE LOCK82 */
#define _SEMAPHORE_LOCK82_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK82          */
#define _SEMAPHORE_LOCK82_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK82                   */
#define SEMAPHORE_LOCK82_LOCK82                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 82 Bit Register              */
#define _SEMAPHORE_LOCK82_LOCK82_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK82            */
#define _SEMAPHORE_LOCK82_LOCK82_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK82               */
#define _SEMAPHORE_LOCK82_LOCK82_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK82           */
#define SEMAPHORE_LOCK82_LOCK82_DEFAULT           (_SEMAPHORE_LOCK82_LOCK82_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK82   */

/* Bit fields for SEMAPHORE LOCK83 */
#define _SEMAPHORE_LOCK83_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK83          */
#define _SEMAPHORE_LOCK83_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK83                   */
#define SEMAPHORE_LOCK83_LOCK83                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 83 Bit Register              */
#define _SEMAPHORE_LOCK83_LOCK83_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK83            */
#define _SEMAPHORE_LOCK83_LOCK83_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK83               */
#define _SEMAPHORE_LOCK83_LOCK83_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK83           */
#define SEMAPHORE_LOCK83_LOCK83_DEFAULT           (_SEMAPHORE_LOCK83_LOCK83_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK83   */

/* Bit fields for SEMAPHORE LOCK84 */
#define _SEMAPHORE_LOCK84_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK84          */
#define _SEMAPHORE_LOCK84_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK84                   */
#define SEMAPHORE_LOCK84_LOCK84                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 84 Bit Register              */
#define _SEMAPHORE_LOCK84_LOCK84_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK84            */
#define _SEMAPHORE_LOCK84_LOCK84_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK84               */
#define _SEMAPHORE_LOCK84_LOCK84_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK84           */
#define SEMAPHORE_LOCK84_LOCK84_DEFAULT           (_SEMAPHORE_LOCK84_LOCK84_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK84   */

/* Bit fields for SEMAPHORE LOCK85 */
#define _SEMAPHORE_LOCK85_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK85          */
#define _SEMAPHORE_LOCK85_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK85                   */
#define SEMAPHORE_LOCK85_LOCK85                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 85 Bit Register              */
#define _SEMAPHORE_LOCK85_LOCK85_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK85            */
#define _SEMAPHORE_LOCK85_LOCK85_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK85               */
#define _SEMAPHORE_LOCK85_LOCK85_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK85           */
#define SEMAPHORE_LOCK85_LOCK85_DEFAULT           (_SEMAPHORE_LOCK85_LOCK85_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK85   */

/* Bit fields for SEMAPHORE LOCK86 */
#define _SEMAPHORE_LOCK86_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK86          */
#define _SEMAPHORE_LOCK86_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK86                   */
#define SEMAPHORE_LOCK86_LOCK86                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 86 Bit Register              */
#define _SEMAPHORE_LOCK86_LOCK86_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK86            */
#define _SEMAPHORE_LOCK86_LOCK86_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK86               */
#define _SEMAPHORE_LOCK86_LOCK86_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK86           */
#define SEMAPHORE_LOCK86_LOCK86_DEFAULT           (_SEMAPHORE_LOCK86_LOCK86_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK86   */

/* Bit fields for SEMAPHORE LOCK87 */
#define _SEMAPHORE_LOCK87_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK87          */
#define _SEMAPHORE_LOCK87_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK87                   */
#define SEMAPHORE_LOCK87_LOCK87                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 87 Bit Register              */
#define _SEMAPHORE_LOCK87_LOCK87_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK87            */
#define _SEMAPHORE_LOCK87_LOCK87_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK87               */
#define _SEMAPHORE_LOCK87_LOCK87_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK87           */
#define SEMAPHORE_LOCK87_LOCK87_DEFAULT           (_SEMAPHORE_LOCK87_LOCK87_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK87   */

/* Bit fields for SEMAPHORE LOCK88 */
#define _SEMAPHORE_LOCK88_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK88          */
#define _SEMAPHORE_LOCK88_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK88                   */
#define SEMAPHORE_LOCK88_LOCK88                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 88 Bit Register              */
#define _SEMAPHORE_LOCK88_LOCK88_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK88            */
#define _SEMAPHORE_LOCK88_LOCK88_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK88               */
#define _SEMAPHORE_LOCK88_LOCK88_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK88           */
#define SEMAPHORE_LOCK88_LOCK88_DEFAULT           (_SEMAPHORE_LOCK88_LOCK88_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK88   */

/* Bit fields for SEMAPHORE LOCK89 */
#define _SEMAPHORE_LOCK89_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK89          */
#define _SEMAPHORE_LOCK89_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK89                   */
#define SEMAPHORE_LOCK89_LOCK89                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 89 Bit Register              */
#define _SEMAPHORE_LOCK89_LOCK89_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK89            */
#define _SEMAPHORE_LOCK89_LOCK89_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK89               */
#define _SEMAPHORE_LOCK89_LOCK89_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK89           */
#define SEMAPHORE_LOCK89_LOCK89_DEFAULT           (_SEMAPHORE_LOCK89_LOCK89_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK89   */

/* Bit fields for SEMAPHORE LOCK90 */
#define _SEMAPHORE_LOCK90_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK90          */
#define _SEMAPHORE_LOCK90_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK90                   */
#define SEMAPHORE_LOCK90_LOCK90                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 90 Bit Register              */
#define _SEMAPHORE_LOCK90_LOCK90_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK90            */
#define _SEMAPHORE_LOCK90_LOCK90_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK90               */
#define _SEMAPHORE_LOCK90_LOCK90_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK90           */
#define SEMAPHORE_LOCK90_LOCK90_DEFAULT           (_SEMAPHORE_LOCK90_LOCK90_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK90   */

/* Bit fields for SEMAPHORE LOCK91 */
#define _SEMAPHORE_LOCK91_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK91          */
#define _SEMAPHORE_LOCK91_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK91                   */
#define SEMAPHORE_LOCK91_LOCK91                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 91 Bit Register              */
#define _SEMAPHORE_LOCK91_LOCK91_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK91            */
#define _SEMAPHORE_LOCK91_LOCK91_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK91               */
#define _SEMAPHORE_LOCK91_LOCK91_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK91           */
#define SEMAPHORE_LOCK91_LOCK91_DEFAULT           (_SEMAPHORE_LOCK91_LOCK91_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK91   */

/* Bit fields for SEMAPHORE LOCK92 */
#define _SEMAPHORE_LOCK92_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK92          */
#define _SEMAPHORE_LOCK92_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK92                   */
#define SEMAPHORE_LOCK92_LOCK92                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 92 Bit Register              */
#define _SEMAPHORE_LOCK92_LOCK92_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK92            */
#define _SEMAPHORE_LOCK92_LOCK92_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK92               */
#define _SEMAPHORE_LOCK92_LOCK92_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK92           */
#define SEMAPHORE_LOCK92_LOCK92_DEFAULT           (_SEMAPHORE_LOCK92_LOCK92_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK92   */

/* Bit fields for SEMAPHORE LOCK93 */
#define _SEMAPHORE_LOCK93_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK93          */
#define _SEMAPHORE_LOCK93_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK93                   */
#define SEMAPHORE_LOCK93_LOCK93                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 93 Bit Register              */
#define _SEMAPHORE_LOCK93_LOCK93_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK93            */
#define _SEMAPHORE_LOCK93_LOCK93_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK93               */
#define _SEMAPHORE_LOCK93_LOCK93_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK93           */
#define SEMAPHORE_LOCK93_LOCK93_DEFAULT           (_SEMAPHORE_LOCK93_LOCK93_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK93   */

/* Bit fields for SEMAPHORE LOCK94 */
#define _SEMAPHORE_LOCK94_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK94          */
#define _SEMAPHORE_LOCK94_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK94                   */
#define SEMAPHORE_LOCK94_LOCK94                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 94 Bit Register              */
#define _SEMAPHORE_LOCK94_LOCK94_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK94            */
#define _SEMAPHORE_LOCK94_LOCK94_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK94               */
#define _SEMAPHORE_LOCK94_LOCK94_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK94           */
#define SEMAPHORE_LOCK94_LOCK94_DEFAULT           (_SEMAPHORE_LOCK94_LOCK94_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK94   */

/* Bit fields for SEMAPHORE LOCK95 */
#define _SEMAPHORE_LOCK95_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK95          */
#define _SEMAPHORE_LOCK95_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK95                   */
#define SEMAPHORE_LOCK95_LOCK95                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 95 Bit Register              */
#define _SEMAPHORE_LOCK95_LOCK95_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK95            */
#define _SEMAPHORE_LOCK95_LOCK95_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK95               */
#define _SEMAPHORE_LOCK95_LOCK95_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK95           */
#define SEMAPHORE_LOCK95_LOCK95_DEFAULT           (_SEMAPHORE_LOCK95_LOCK95_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK95   */

/* Bit fields for SEMAPHORE LOCK96 */
#define _SEMAPHORE_LOCK96_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK96          */
#define _SEMAPHORE_LOCK96_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK96                   */
#define SEMAPHORE_LOCK96_LOCK96                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 96 Bit Register              */
#define _SEMAPHORE_LOCK96_LOCK96_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK96            */
#define _SEMAPHORE_LOCK96_LOCK96_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK96               */
#define _SEMAPHORE_LOCK96_LOCK96_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK96           */
#define SEMAPHORE_LOCK96_LOCK96_DEFAULT           (_SEMAPHORE_LOCK96_LOCK96_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK96   */

/* Bit fields for SEMAPHORE LOCK97 */
#define _SEMAPHORE_LOCK97_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK97          */
#define _SEMAPHORE_LOCK97_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK97                   */
#define SEMAPHORE_LOCK97_LOCK97                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 97 Bit Register              */
#define _SEMAPHORE_LOCK97_LOCK97_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK97            */
#define _SEMAPHORE_LOCK97_LOCK97_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK97               */
#define _SEMAPHORE_LOCK97_LOCK97_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK97           */
#define SEMAPHORE_LOCK97_LOCK97_DEFAULT           (_SEMAPHORE_LOCK97_LOCK97_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK97   */

/* Bit fields for SEMAPHORE LOCK98 */
#define _SEMAPHORE_LOCK98_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK98          */
#define _SEMAPHORE_LOCK98_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK98                   */
#define SEMAPHORE_LOCK98_LOCK98                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 98 Bit Register              */
#define _SEMAPHORE_LOCK98_LOCK98_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK98            */
#define _SEMAPHORE_LOCK98_LOCK98_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK98               */
#define _SEMAPHORE_LOCK98_LOCK98_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK98           */
#define SEMAPHORE_LOCK98_LOCK98_DEFAULT           (_SEMAPHORE_LOCK98_LOCK98_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK98   */

/* Bit fields for SEMAPHORE LOCK99 */
#define _SEMAPHORE_LOCK99_RESETVALUE              0x00000000UL                            /**< Default value for SEMAPHORE_LOCK99          */
#define _SEMAPHORE_LOCK99_MASK                    0x00000001UL                            /**< Mask for SEMAPHORE_LOCK99                   */
#define SEMAPHORE_LOCK99_LOCK99                   (0x1UL << 0)                            /**< SEMAPHORE LOCK 99 Bit Register              */
#define _SEMAPHORE_LOCK99_LOCK99_SHIFT            0                                       /**< Shift value for SEMAPHORE_LOCK99            */
#define _SEMAPHORE_LOCK99_LOCK99_MASK             0x1UL                                   /**< Bit mask for SEMAPHORE_LOCK99               */
#define _SEMAPHORE_LOCK99_LOCK99_DEFAULT          0x00000000UL                            /**< Mode DEFAULT for SEMAPHORE_LOCK99           */
#define SEMAPHORE_LOCK99_LOCK99_DEFAULT           (_SEMAPHORE_LOCK99_LOCK99_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK99   */

/* Bit fields for SEMAPHORE LOCK100 */
#define _SEMAPHORE_LOCK100_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK100         */
#define _SEMAPHORE_LOCK100_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK100                  */
#define SEMAPHORE_LOCK100_LOCK100                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 100 Bit Register             */
#define _SEMAPHORE_LOCK100_LOCK100_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK100           */
#define _SEMAPHORE_LOCK100_LOCK100_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK100              */
#define _SEMAPHORE_LOCK100_LOCK100_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK100          */
#define SEMAPHORE_LOCK100_LOCK100_DEFAULT         (_SEMAPHORE_LOCK100_LOCK100_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK100  */

/* Bit fields for SEMAPHORE LOCK101 */
#define _SEMAPHORE_LOCK101_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK101         */
#define _SEMAPHORE_LOCK101_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK101                  */
#define SEMAPHORE_LOCK101_LOCK101                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 101 Bit Register             */
#define _SEMAPHORE_LOCK101_LOCK101_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK101           */
#define _SEMAPHORE_LOCK101_LOCK101_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK101              */
#define _SEMAPHORE_LOCK101_LOCK101_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK101          */
#define SEMAPHORE_LOCK101_LOCK101_DEFAULT         (_SEMAPHORE_LOCK101_LOCK101_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK101  */

/* Bit fields for SEMAPHORE LOCK102 */
#define _SEMAPHORE_LOCK102_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK102         */
#define _SEMAPHORE_LOCK102_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK102                  */
#define SEMAPHORE_LOCK102_LOCK102                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 102 Bit Register             */
#define _SEMAPHORE_LOCK102_LOCK102_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK102           */
#define _SEMAPHORE_LOCK102_LOCK102_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK102              */
#define _SEMAPHORE_LOCK102_LOCK102_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK102          */
#define SEMAPHORE_LOCK102_LOCK102_DEFAULT         (_SEMAPHORE_LOCK102_LOCK102_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK102  */

/* Bit fields for SEMAPHORE LOCK103 */
#define _SEMAPHORE_LOCK103_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK103         */
#define _SEMAPHORE_LOCK103_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK103                  */
#define SEMAPHORE_LOCK103_LOCK103                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 103 Bit Register             */
#define _SEMAPHORE_LOCK103_LOCK103_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK103           */
#define _SEMAPHORE_LOCK103_LOCK103_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK103              */
#define _SEMAPHORE_LOCK103_LOCK103_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK103          */
#define SEMAPHORE_LOCK103_LOCK103_DEFAULT         (_SEMAPHORE_LOCK103_LOCK103_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK103  */

/* Bit fields for SEMAPHORE LOCK104 */
#define _SEMAPHORE_LOCK104_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK104         */
#define _SEMAPHORE_LOCK104_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK104                  */
#define SEMAPHORE_LOCK104_LOCK104                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 104 Bit Register             */
#define _SEMAPHORE_LOCK104_LOCK104_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK104           */
#define _SEMAPHORE_LOCK104_LOCK104_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK104              */
#define _SEMAPHORE_LOCK104_LOCK104_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK104          */
#define SEMAPHORE_LOCK104_LOCK104_DEFAULT         (_SEMAPHORE_LOCK104_LOCK104_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK104  */

/* Bit fields for SEMAPHORE LOCK105 */
#define _SEMAPHORE_LOCK105_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK105         */
#define _SEMAPHORE_LOCK105_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK105                  */
#define SEMAPHORE_LOCK105_LOCK105                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 105 Bit Register             */
#define _SEMAPHORE_LOCK105_LOCK105_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK105           */
#define _SEMAPHORE_LOCK105_LOCK105_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK105              */
#define _SEMAPHORE_LOCK105_LOCK105_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK105          */
#define SEMAPHORE_LOCK105_LOCK105_DEFAULT         (_SEMAPHORE_LOCK105_LOCK105_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK105  */

/* Bit fields for SEMAPHORE LOCK106 */
#define _SEMAPHORE_LOCK106_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK106         */
#define _SEMAPHORE_LOCK106_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK106                  */
#define SEMAPHORE_LOCK106_LOCK106                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 106 Bit Register             */
#define _SEMAPHORE_LOCK106_LOCK106_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK106           */
#define _SEMAPHORE_LOCK106_LOCK106_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK106              */
#define _SEMAPHORE_LOCK106_LOCK106_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK106          */
#define SEMAPHORE_LOCK106_LOCK106_DEFAULT         (_SEMAPHORE_LOCK106_LOCK106_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK106  */

/* Bit fields for SEMAPHORE LOCK107 */
#define _SEMAPHORE_LOCK107_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK107         */
#define _SEMAPHORE_LOCK107_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK107                  */
#define SEMAPHORE_LOCK107_LOCK107                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 107 Bit Register             */
#define _SEMAPHORE_LOCK107_LOCK107_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK107           */
#define _SEMAPHORE_LOCK107_LOCK107_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK107              */
#define _SEMAPHORE_LOCK107_LOCK107_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK107          */
#define SEMAPHORE_LOCK107_LOCK107_DEFAULT         (_SEMAPHORE_LOCK107_LOCK107_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK107  */

/* Bit fields for SEMAPHORE LOCK108 */
#define _SEMAPHORE_LOCK108_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK108         */
#define _SEMAPHORE_LOCK108_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK108                  */
#define SEMAPHORE_LOCK108_LOCK108                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 108 Bit Register             */
#define _SEMAPHORE_LOCK108_LOCK108_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK108           */
#define _SEMAPHORE_LOCK108_LOCK108_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK108              */
#define _SEMAPHORE_LOCK108_LOCK108_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK108          */
#define SEMAPHORE_LOCK108_LOCK108_DEFAULT         (_SEMAPHORE_LOCK108_LOCK108_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK108  */

/* Bit fields for SEMAPHORE LOCK109 */
#define _SEMAPHORE_LOCK109_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK109         */
#define _SEMAPHORE_LOCK109_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK109                  */
#define SEMAPHORE_LOCK109_LOCK109                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 109 Bit Register             */
#define _SEMAPHORE_LOCK109_LOCK109_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK109           */
#define _SEMAPHORE_LOCK109_LOCK109_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK109              */
#define _SEMAPHORE_LOCK109_LOCK109_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK109          */
#define SEMAPHORE_LOCK109_LOCK109_DEFAULT         (_SEMAPHORE_LOCK109_LOCK109_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK109  */

/* Bit fields for SEMAPHORE LOCK110 */
#define _SEMAPHORE_LOCK110_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK110         */
#define _SEMAPHORE_LOCK110_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK110                  */
#define SEMAPHORE_LOCK110_LOCK110                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 110 Bit Register             */
#define _SEMAPHORE_LOCK110_LOCK110_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK110           */
#define _SEMAPHORE_LOCK110_LOCK110_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK110              */
#define _SEMAPHORE_LOCK110_LOCK110_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK110          */
#define SEMAPHORE_LOCK110_LOCK110_DEFAULT         (_SEMAPHORE_LOCK110_LOCK110_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK110  */

/* Bit fields for SEMAPHORE LOCK111 */
#define _SEMAPHORE_LOCK111_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK111         */
#define _SEMAPHORE_LOCK111_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK111                  */
#define SEMAPHORE_LOCK111_LOCK111                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 111 Bit Register             */
#define _SEMAPHORE_LOCK111_LOCK111_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK111           */
#define _SEMAPHORE_LOCK111_LOCK111_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK111              */
#define _SEMAPHORE_LOCK111_LOCK111_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK111          */
#define SEMAPHORE_LOCK111_LOCK111_DEFAULT         (_SEMAPHORE_LOCK111_LOCK111_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK111  */

/* Bit fields for SEMAPHORE LOCK112 */
#define _SEMAPHORE_LOCK112_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK112         */
#define _SEMAPHORE_LOCK112_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK112                  */
#define SEMAPHORE_LOCK112_LOCK112                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 112 Bit Register             */
#define _SEMAPHORE_LOCK112_LOCK112_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK112           */
#define _SEMAPHORE_LOCK112_LOCK112_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK112              */
#define _SEMAPHORE_LOCK112_LOCK112_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK112          */
#define SEMAPHORE_LOCK112_LOCK112_DEFAULT         (_SEMAPHORE_LOCK112_LOCK112_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK112  */

/* Bit fields for SEMAPHORE LOCK113 */
#define _SEMAPHORE_LOCK113_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK113         */
#define _SEMAPHORE_LOCK113_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK113                  */
#define SEMAPHORE_LOCK113_LOCK113                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 113 Bit Register             */
#define _SEMAPHORE_LOCK113_LOCK113_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK113           */
#define _SEMAPHORE_LOCK113_LOCK113_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK113              */
#define _SEMAPHORE_LOCK113_LOCK113_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK113          */
#define SEMAPHORE_LOCK113_LOCK113_DEFAULT         (_SEMAPHORE_LOCK113_LOCK113_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK113  */

/* Bit fields for SEMAPHORE LOCK114 */
#define _SEMAPHORE_LOCK114_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK114         */
#define _SEMAPHORE_LOCK114_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK114                  */
#define SEMAPHORE_LOCK114_LOCK114                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 114 Bit Register             */
#define _SEMAPHORE_LOCK114_LOCK114_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK114           */
#define _SEMAPHORE_LOCK114_LOCK114_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK114              */
#define _SEMAPHORE_LOCK114_LOCK114_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK114          */
#define SEMAPHORE_LOCK114_LOCK114_DEFAULT         (_SEMAPHORE_LOCK114_LOCK114_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK114  */

/* Bit fields for SEMAPHORE LOCK115 */
#define _SEMAPHORE_LOCK115_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK115         */
#define _SEMAPHORE_LOCK115_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK115                  */
#define SEMAPHORE_LOCK115_LOCK115                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 115 Bit Register             */
#define _SEMAPHORE_LOCK115_LOCK115_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK115           */
#define _SEMAPHORE_LOCK115_LOCK115_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK115              */
#define _SEMAPHORE_LOCK115_LOCK115_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK115          */
#define SEMAPHORE_LOCK115_LOCK115_DEFAULT         (_SEMAPHORE_LOCK115_LOCK115_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK115  */

/* Bit fields for SEMAPHORE LOCK116 */
#define _SEMAPHORE_LOCK116_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK116         */
#define _SEMAPHORE_LOCK116_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK116                  */
#define SEMAPHORE_LOCK116_LOCK116                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 116 Bit Register             */
#define _SEMAPHORE_LOCK116_LOCK116_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK116           */
#define _SEMAPHORE_LOCK116_LOCK116_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK116              */
#define _SEMAPHORE_LOCK116_LOCK116_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK116          */
#define SEMAPHORE_LOCK116_LOCK116_DEFAULT         (_SEMAPHORE_LOCK116_LOCK116_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK116  */

/* Bit fields for SEMAPHORE LOCK117 */
#define _SEMAPHORE_LOCK117_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK117         */
#define _SEMAPHORE_LOCK117_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK117                  */
#define SEMAPHORE_LOCK117_LOCK117                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 117 Bit Register             */
#define _SEMAPHORE_LOCK117_LOCK117_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK117           */
#define _SEMAPHORE_LOCK117_LOCK117_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK117              */
#define _SEMAPHORE_LOCK117_LOCK117_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK117          */
#define SEMAPHORE_LOCK117_LOCK117_DEFAULT         (_SEMAPHORE_LOCK117_LOCK117_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK117  */

/* Bit fields for SEMAPHORE LOCK118 */
#define _SEMAPHORE_LOCK118_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK118         */
#define _SEMAPHORE_LOCK118_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK118                  */
#define SEMAPHORE_LOCK118_LOCK118                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 118 Bit Register             */
#define _SEMAPHORE_LOCK118_LOCK118_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK118           */
#define _SEMAPHORE_LOCK118_LOCK118_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK118              */
#define _SEMAPHORE_LOCK118_LOCK118_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK118          */
#define SEMAPHORE_LOCK118_LOCK118_DEFAULT         (_SEMAPHORE_LOCK118_LOCK118_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK118  */

/* Bit fields for SEMAPHORE LOCK119 */
#define _SEMAPHORE_LOCK119_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK119         */
#define _SEMAPHORE_LOCK119_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK119                  */
#define SEMAPHORE_LOCK119_LOCK119                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 119 Bit Register             */
#define _SEMAPHORE_LOCK119_LOCK119_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK119           */
#define _SEMAPHORE_LOCK119_LOCK119_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK119              */
#define _SEMAPHORE_LOCK119_LOCK119_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK119          */
#define SEMAPHORE_LOCK119_LOCK119_DEFAULT         (_SEMAPHORE_LOCK119_LOCK119_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK119  */

/* Bit fields for SEMAPHORE LOCK120 */
#define _SEMAPHORE_LOCK120_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK120         */
#define _SEMAPHORE_LOCK120_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK120                  */
#define SEMAPHORE_LOCK120_LOCK120                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 120 Bit Register             */
#define _SEMAPHORE_LOCK120_LOCK120_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK120           */
#define _SEMAPHORE_LOCK120_LOCK120_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK120              */
#define _SEMAPHORE_LOCK120_LOCK120_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK120          */
#define SEMAPHORE_LOCK120_LOCK120_DEFAULT         (_SEMAPHORE_LOCK120_LOCK120_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK120  */

/* Bit fields for SEMAPHORE LOCK121 */
#define _SEMAPHORE_LOCK121_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK121         */
#define _SEMAPHORE_LOCK121_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK121                  */
#define SEMAPHORE_LOCK121_LOCK121                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 121 Bit Register             */
#define _SEMAPHORE_LOCK121_LOCK121_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK121           */
#define _SEMAPHORE_LOCK121_LOCK121_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK121              */
#define _SEMAPHORE_LOCK121_LOCK121_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK121          */
#define SEMAPHORE_LOCK121_LOCK121_DEFAULT         (_SEMAPHORE_LOCK121_LOCK121_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK121  */

/* Bit fields for SEMAPHORE LOCK122 */
#define _SEMAPHORE_LOCK122_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK122         */
#define _SEMAPHORE_LOCK122_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK122                  */
#define SEMAPHORE_LOCK122_LOCK122                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 122 Bit Register             */
#define _SEMAPHORE_LOCK122_LOCK122_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK122           */
#define _SEMAPHORE_LOCK122_LOCK122_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK122              */
#define _SEMAPHORE_LOCK122_LOCK122_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK122          */
#define SEMAPHORE_LOCK122_LOCK122_DEFAULT         (_SEMAPHORE_LOCK122_LOCK122_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK122  */

/* Bit fields for SEMAPHORE LOCK123 */
#define _SEMAPHORE_LOCK123_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK123         */
#define _SEMAPHORE_LOCK123_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK123                  */
#define SEMAPHORE_LOCK123_LOCK123                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 123 Bit Register             */
#define _SEMAPHORE_LOCK123_LOCK123_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK123           */
#define _SEMAPHORE_LOCK123_LOCK123_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK123              */
#define _SEMAPHORE_LOCK123_LOCK123_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK123          */
#define SEMAPHORE_LOCK123_LOCK123_DEFAULT         (_SEMAPHORE_LOCK123_LOCK123_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK123  */

/* Bit fields for SEMAPHORE LOCK124 */
#define _SEMAPHORE_LOCK124_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK124         */
#define _SEMAPHORE_LOCK124_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK124                  */
#define SEMAPHORE_LOCK124_LOCK124                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 124 Bit Register             */
#define _SEMAPHORE_LOCK124_LOCK124_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK124           */
#define _SEMAPHORE_LOCK124_LOCK124_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK124              */
#define _SEMAPHORE_LOCK124_LOCK124_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK124          */
#define SEMAPHORE_LOCK124_LOCK124_DEFAULT         (_SEMAPHORE_LOCK124_LOCK124_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK124  */

/* Bit fields for SEMAPHORE LOCK125 */
#define _SEMAPHORE_LOCK125_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK125         */
#define _SEMAPHORE_LOCK125_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK125                  */
#define SEMAPHORE_LOCK125_LOCK125                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 125 Bit Register             */
#define _SEMAPHORE_LOCK125_LOCK125_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK125           */
#define _SEMAPHORE_LOCK125_LOCK125_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK125              */
#define _SEMAPHORE_LOCK125_LOCK125_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK125          */
#define SEMAPHORE_LOCK125_LOCK125_DEFAULT         (_SEMAPHORE_LOCK125_LOCK125_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK125  */

/* Bit fields for SEMAPHORE LOCK126 */
#define _SEMAPHORE_LOCK126_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK126         */
#define _SEMAPHORE_LOCK126_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK126                  */
#define SEMAPHORE_LOCK126_LOCK126                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 126 Bit Register             */
#define _SEMAPHORE_LOCK126_LOCK126_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK126           */
#define _SEMAPHORE_LOCK126_LOCK126_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK126              */
#define _SEMAPHORE_LOCK126_LOCK126_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK126          */
#define SEMAPHORE_LOCK126_LOCK126_DEFAULT         (_SEMAPHORE_LOCK126_LOCK126_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK126  */

/* Bit fields for SEMAPHORE LOCK127 */
#define _SEMAPHORE_LOCK127_RESETVALUE             0x00000000UL                              /**< Default value for SEMAPHORE_LOCK127         */
#define _SEMAPHORE_LOCK127_MASK                   0x00000001UL                              /**< Mask for SEMAPHORE_LOCK127                  */
#define SEMAPHORE_LOCK127_LOCK127                 (0x1UL << 0)                              /**< SEMAPHORE LOCK 127 Bit Register             */
#define _SEMAPHORE_LOCK127_LOCK127_SHIFT          0                                         /**< Shift value for SEMAPHORE_LOCK127           */
#define _SEMAPHORE_LOCK127_LOCK127_MASK           0x1UL                                     /**< Bit mask for SEMAPHORE_LOCK127              */
#define _SEMAPHORE_LOCK127_LOCK127_DEFAULT        0x00000000UL                              /**< Mode DEFAULT for SEMAPHORE_LOCK127          */
#define SEMAPHORE_LOCK127_LOCK127_DEFAULT         (_SEMAPHORE_LOCK127_LOCK127_DEFAULT << 0) /**< Shifted mode DEFAULT for SEMAPHORE_LOCK127  */

/** @} End of group SIBG301_SEMAPHORE_BitFields */
/** @} End of group SIBG301_SEMAPHORE */
/** @} End of group Parts */

#endif // SIBG301_SEMAPHORE_H
