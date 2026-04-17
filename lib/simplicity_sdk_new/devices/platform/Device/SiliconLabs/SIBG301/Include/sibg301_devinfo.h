/**************************************************************************//**
 * @file
 * @brief SIBG301 DEVINFO register and bit field definitions
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
#ifndef SIBG301_DEVINFO_H
#define SIBG301_DEVINFO_H

/**************************************************************************//**
* @addtogroup Parts
* @{
******************************************************************************/
/**************************************************************************//**
 * @defgroup SIBG301_DEVINFO DEVINFO
 * @{
 * @brief SIBG301 DEVINFO Register Declaration.
 *****************************************************************************/

/** DEVINFO Register Declaration. */
typedef struct devinfo_typedef{
  uint32_t      RESERVED0[64U];                 /**< Reserved for future use                            */
  __IM uint32_t REVISION;                       /**< DI Information                                     */
  __IM uint32_t EMBSIZE;                        /**< Embedded Memory Information                        */
  __IM uint32_t STACKMSIZE;                     /**< Co-packaged Memory Information                     */
  __IM uint32_t SWCAPA;                         /**< Software Restriction                               */
  __IM uint32_t HFRCOCALDEFAULT;                /**< Default HFRCODPLL Calibration Information          */
  __IM uint32_t HFRCOCALSPEED;                  /**< Fastest HFRCODPLL Calibration Information          */
  __IM uint32_t EUI64L;                         /**< EUI64 Low                                          */
  __IM uint32_t EUI64H;                         /**< EUI64 High                                         */
  __IM uint32_t PART0;                          /**< Part Information                                   */
  __IM uint32_t PART1;                          /**< Part Information                                   */
  __IM uint32_t PART2;                          /**< External Component Info                            */
} DEVINFO_TypeDef;
/** @} End of group SIBG301_DEVINFO */

/**************************************************************************//**
 * @addtogroup SIBG301_DEVINFO
 * @{
 * @defgroup SIBG301_DEVINFO_BitFields DEVINFO Bit Fields
 * @{
 *****************************************************************************/

/* Bit fields for DEVINFO REVISION */
#define _DEVINFO_REVISION_RESETVALUE                   0x00000005UL                                /**< Default value for DEVINFO_REVISION          */
#define _DEVINFO_REVISION_MASK                         0x0000FFFFUL                                /**< Mask for DEVINFO_REVISION                   */
#define _DEVINFO_REVISION_DEVINFOREV_SHIFT             0                                           /**< Shift value for DEVINFO_DEVINFOREV          */
#define _DEVINFO_REVISION_DEVINFOREV_MASK              0xFFUL                                      /**< Bit mask for DEVINFO_DEVINFOREV             */
#define _DEVINFO_REVISION_DEVINFOREV_DEFAULT           0x00000005UL                                /**< Mode DEFAULT for DEVINFO_REVISION           */
#define DEVINFO_REVISION_DEVINFOREV_DEFAULT            (_DEVINFO_REVISION_DEVINFOREV_DEFAULT << 0) /**< Shifted mode DEFAULT for DEVINFO_REVISION   */
#define _DEVINFO_REVISION_PRODREV_SHIFT                8                                           /**< Shift value for DEVINFO_PRODREV             */
#define _DEVINFO_REVISION_PRODREV_MASK                 0xFF00UL                                    /**< Bit mask for DEVINFO_PRODREV                */
#define _DEVINFO_REVISION_PRODREV_DEFAULT              0x00000000UL                                /**< Mode DEFAULT for DEVINFO_REVISION           */
#define DEVINFO_REVISION_PRODREV_DEFAULT               (_DEVINFO_REVISION_PRODREV_DEFAULT << 8)    /**< Shifted mode DEFAULT for DEVINFO_REVISION   */

/* Bit fields for DEVINFO EMBSIZE */
#define _DEVINFO_EMBSIZE_RESETVALUE                    0x00000000UL                         /**< Default value for DEVINFO_EMBSIZE           */
#define _DEVINFO_EMBSIZE_MASK                          0x0FFFFFFFUL                         /**< Mask for DEVINFO_EMBSIZE                    */
#define _DEVINFO_EMBSIZE_NVM_SHIFT                     0                                    /**< Shift value for DEVINFO_NVM                 */
#define _DEVINFO_EMBSIZE_NVM_MASK                      0xFFFFUL                             /**< Bit mask for DEVINFO_NVM                    */
#define _DEVINFO_EMBSIZE_NVM_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for DEVINFO_EMBSIZE            */
#define DEVINFO_EMBSIZE_NVM_DEFAULT                    (_DEVINFO_EMBSIZE_NVM_DEFAULT << 0)  /**< Shifted mode DEFAULT for DEVINFO_EMBSIZE    */
#define _DEVINFO_EMBSIZE_RAM_SHIFT                     16                                   /**< Shift value for DEVINFO_RAM                 */
#define _DEVINFO_EMBSIZE_RAM_MASK                      0xFFF0000UL                          /**< Bit mask for DEVINFO_RAM                    */
#define _DEVINFO_EMBSIZE_RAM_DEFAULT                   0x00000000UL                         /**< Mode DEFAULT for DEVINFO_EMBSIZE            */
#define DEVINFO_EMBSIZE_RAM_DEFAULT                    (_DEVINFO_EMBSIZE_RAM_DEFAULT << 16) /**< Shifted mode DEFAULT for DEVINFO_EMBSIZE    */

/* Bit fields for DEVINFO STACKMSIZE */
#define _DEVINFO_STACKMSIZE_RESETVALUE                 0x00000000UL                              /**< Default value for DEVINFO_STACKMSIZE        */
#define _DEVINFO_STACKMSIZE_MASK                       0xFFFFFFFFUL                              /**< Mask for DEVINFO_STACKMSIZE                 */
#define _DEVINFO_STACKMSIZE_FLASH_SHIFT                0                                         /**< Shift value for DEVINFO_FLASH               */
#define _DEVINFO_STACKMSIZE_FLASH_MASK                 0xFFFFUL                                  /**< Bit mask for DEVINFO_FLASH                  */
#define _DEVINFO_STACKMSIZE_FLASH_DEFAULT              0x00000000UL                              /**< Mode DEFAULT for DEVINFO_STACKMSIZE         */
#define DEVINFO_STACKMSIZE_FLASH_DEFAULT               (_DEVINFO_STACKMSIZE_FLASH_DEFAULT << 0)  /**< Shifted mode DEFAULT for DEVINFO_STACKMSIZE */
#define _DEVINFO_STACKMSIZE_PSRAM_SHIFT                16                                        /**< Shift value for DEVINFO_PSRAM               */
#define _DEVINFO_STACKMSIZE_PSRAM_MASK                 0xFFFF0000UL                              /**< Bit mask for DEVINFO_PSRAM                  */
#define _DEVINFO_STACKMSIZE_PSRAM_DEFAULT              0x00000000UL                              /**< Mode DEFAULT for DEVINFO_STACKMSIZE         */
#define DEVINFO_STACKMSIZE_PSRAM_DEFAULT               (_DEVINFO_STACKMSIZE_PSRAM_DEFAULT << 16) /**< Shifted mode DEFAULT for DEVINFO_STACKMSIZE */

/* Bit fields for DEVINFO SWCAPA */
#define _DEVINFO_SWCAPA_RESETVALUE                     0xE0000000UL                            /**< Default value for DEVINFO_SWCAPA            */
#define _DEVINFO_SWCAPA_MASK                           0xE0001FFFUL                            /**< Mask for DEVINFO_SWCAPA                     */
#define _DEVINFO_SWCAPA_ZIGBEE_SHIFT                   0                                       /**< Shift value for DEVINFO_ZIGBEE              */
#define _DEVINFO_SWCAPA_ZIGBEE_MASK                    0x3UL                                   /**< Bit mask for DEVINFO_ZIGBEE                 */
#define _DEVINFO_SWCAPA_ZIGBEE_DEFAULT                 0x00000000UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define _DEVINFO_SWCAPA_ZIGBEE_LEVEL0                  0x00000000UL                            /**< Mode LEVEL0 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_ZIGBEE_LEVEL1                  0x00000001UL                            /**< Mode LEVEL1 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_ZIGBEE_LEVEL2                  0x00000002UL                            /**< Mode LEVEL2 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_ZIGBEE_LEVEL3                  0x00000003UL                            /**< Mode LEVEL3 for DEVINFO_SWCAPA              */
#define DEVINFO_SWCAPA_ZIGBEE_DEFAULT                  (_DEVINFO_SWCAPA_ZIGBEE_DEFAULT << 0)   /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_ZIGBEE_LEVEL0                   (_DEVINFO_SWCAPA_ZIGBEE_LEVEL0 << 0)    /**< Shifted mode LEVEL0 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_ZIGBEE_LEVEL1                   (_DEVINFO_SWCAPA_ZIGBEE_LEVEL1 << 0)    /**< Shifted mode LEVEL1 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_ZIGBEE_LEVEL2                   (_DEVINFO_SWCAPA_ZIGBEE_LEVEL2 << 0)    /**< Shifted mode LEVEL2 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_ZIGBEE_LEVEL3                   (_DEVINFO_SWCAPA_ZIGBEE_LEVEL3 << 0)    /**< Shifted mode LEVEL3 for DEVINFO_SWCAPA      */
#define _DEVINFO_SWCAPA_THREAD_SHIFT                   2                                       /**< Shift value for DEVINFO_THREAD              */
#define _DEVINFO_SWCAPA_THREAD_MASK                    0xCUL                                   /**< Bit mask for DEVINFO_THREAD                 */
#define _DEVINFO_SWCAPA_THREAD_DEFAULT                 0x00000000UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define _DEVINFO_SWCAPA_THREAD_LEVEL0                  0x00000000UL                            /**< Mode LEVEL0 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_THREAD_LEVEL1                  0x00000001UL                            /**< Mode LEVEL1 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_THREAD_LEVEL2                  0x00000002UL                            /**< Mode LEVEL2 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_THREAD_LEVEL3                  0x00000003UL                            /**< Mode LEVEL3 for DEVINFO_SWCAPA              */
#define DEVINFO_SWCAPA_THREAD_DEFAULT                  (_DEVINFO_SWCAPA_THREAD_DEFAULT << 2)   /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_THREAD_LEVEL0                   (_DEVINFO_SWCAPA_THREAD_LEVEL0 << 2)    /**< Shifted mode LEVEL0 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_THREAD_LEVEL1                   (_DEVINFO_SWCAPA_THREAD_LEVEL1 << 2)    /**< Shifted mode LEVEL1 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_THREAD_LEVEL2                   (_DEVINFO_SWCAPA_THREAD_LEVEL2 << 2)    /**< Shifted mode LEVEL2 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_THREAD_LEVEL3                   (_DEVINFO_SWCAPA_THREAD_LEVEL3 << 2)    /**< Shifted mode LEVEL3 for DEVINFO_SWCAPA      */
#define _DEVINFO_SWCAPA_BTSMART_SHIFT                  4                                       /**< Shift value for DEVINFO_BTSMART             */
#define _DEVINFO_SWCAPA_BTSMART_MASK                   0x30UL                                  /**< Bit mask for DEVINFO_BTSMART                */
#define _DEVINFO_SWCAPA_BTSMART_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define _DEVINFO_SWCAPA_BTSMART_LEVEL0                 0x00000000UL                            /**< Mode LEVEL0 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_BTSMART_LEVEL1                 0x00000001UL                            /**< Mode LEVEL1 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_BTSMART_LEVEL2                 0x00000002UL                            /**< Mode LEVEL2 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_BTSMART_LEVEL3                 0x00000003UL                            /**< Mode LEVEL3 for DEVINFO_SWCAPA              */
#define DEVINFO_SWCAPA_BTSMART_DEFAULT                 (_DEVINFO_SWCAPA_BTSMART_DEFAULT << 4)  /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_BTSMART_LEVEL0                  (_DEVINFO_SWCAPA_BTSMART_LEVEL0 << 4)   /**< Shifted mode LEVEL0 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_BTSMART_LEVEL1                  (_DEVINFO_SWCAPA_BTSMART_LEVEL1 << 4)   /**< Shifted mode LEVEL1 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_BTSMART_LEVEL2                  (_DEVINFO_SWCAPA_BTSMART_LEVEL2 << 4)   /**< Shifted mode LEVEL2 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_BTSMART_LEVEL3                  (_DEVINFO_SWCAPA_BTSMART_LEVEL3 << 4)   /**< Shifted mode LEVEL3 for DEVINFO_SWCAPA      */
#define _DEVINFO_SWCAPA_CONNECT_SHIFT                  6                                       /**< Shift value for DEVINFO_CONNECT             */
#define _DEVINFO_SWCAPA_CONNECT_MASK                   0xC0UL                                  /**< Bit mask for DEVINFO_CONNECT                */
#define _DEVINFO_SWCAPA_CONNECT_DEFAULT                0x00000000UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define _DEVINFO_SWCAPA_CONNECT_LEVEL0                 0x00000000UL                            /**< Mode LEVEL0 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_CONNECT_LEVEL1                 0x00000001UL                            /**< Mode LEVEL1 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_CONNECT_LEVEL2                 0x00000002UL                            /**< Mode LEVEL2 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_CONNECT_LEVEL3                 0x00000003UL                            /**< Mode LEVEL3 for DEVINFO_SWCAPA              */
#define DEVINFO_SWCAPA_CONNECT_DEFAULT                 (_DEVINFO_SWCAPA_CONNECT_DEFAULT << 6)  /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_CONNECT_LEVEL0                  (_DEVINFO_SWCAPA_CONNECT_LEVEL0 << 6)   /**< Shifted mode LEVEL0 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_CONNECT_LEVEL1                  (_DEVINFO_SWCAPA_CONNECT_LEVEL1 << 6)   /**< Shifted mode LEVEL1 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_CONNECT_LEVEL2                  (_DEVINFO_SWCAPA_CONNECT_LEVEL2 << 6)   /**< Shifted mode LEVEL2 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_CONNECT_LEVEL3                  (_DEVINFO_SWCAPA_CONNECT_LEVEL3 << 6)   /**< Shifted mode LEVEL3 for DEVINFO_SWCAPA      */
#define _DEVINFO_SWCAPA_RAIL_SHIFT                     8                                       /**< Shift value for DEVINFO_RAIL                */
#define _DEVINFO_SWCAPA_RAIL_MASK                      0x300UL                                 /**< Bit mask for DEVINFO_RAIL                   */
#define _DEVINFO_SWCAPA_RAIL_DEFAULT                   0x00000000UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define _DEVINFO_SWCAPA_RAIL_LEVEL0                    0x00000000UL                            /**< Mode LEVEL0 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_RAIL_LEVEL1                    0x00000001UL                            /**< Mode LEVEL1 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_RAIL_LEVEL2                    0x00000002UL                            /**< Mode LEVEL2 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_RAIL_LEVEL3                    0x00000003UL                            /**< Mode LEVEL3 for DEVINFO_SWCAPA              */
#define DEVINFO_SWCAPA_RAIL_DEFAULT                    (_DEVINFO_SWCAPA_RAIL_DEFAULT << 8)     /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_RAIL_LEVEL0                     (_DEVINFO_SWCAPA_RAIL_LEVEL0 << 8)      /**< Shifted mode LEVEL0 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_RAIL_LEVEL1                     (_DEVINFO_SWCAPA_RAIL_LEVEL1 << 8)      /**< Shifted mode LEVEL1 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_RAIL_LEVEL2                     (_DEVINFO_SWCAPA_RAIL_LEVEL2 << 8)      /**< Shifted mode LEVEL2 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_RAIL_LEVEL3                     (_DEVINFO_SWCAPA_RAIL_LEVEL3 << 8)      /**< Shifted mode LEVEL3 for DEVINFO_SWCAPA      */
#define _DEVINFO_SWCAPA_ZWAVE_SHIFT                    10                                      /**< Shift value for DEVINFO_ZWAVE               */
#define _DEVINFO_SWCAPA_ZWAVE_MASK                     0x1C00UL                                /**< Bit mask for DEVINFO_ZWAVE                  */
#define _DEVINFO_SWCAPA_ZWAVE_DEFAULT                  0x00000000UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define _DEVINFO_SWCAPA_ZWAVE_LEVEL0                   0x00000000UL                            /**< Mode LEVEL0 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_ZWAVE_LEVEL1                   0x00000001UL                            /**< Mode LEVEL1 for DEVINFO_SWCAPA              */
#define _DEVINFO_SWCAPA_ZWAVE_LEVEL2                   0x00000002UL                            /**< Mode LEVEL2 for DEVINFO_SWCAPA              */
#define DEVINFO_SWCAPA_ZWAVE_DEFAULT                   (_DEVINFO_SWCAPA_ZWAVE_DEFAULT << 10)   /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_ZWAVE_LEVEL0                    (_DEVINFO_SWCAPA_ZWAVE_LEVEL0 << 10)    /**< Shifted mode LEVEL0 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_ZWAVE_LEVEL1                    (_DEVINFO_SWCAPA_ZWAVE_LEVEL1 << 10)    /**< Shifted mode LEVEL1 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_ZWAVE_LEVEL2                    (_DEVINFO_SWCAPA_ZWAVE_LEVEL2 << 10)    /**< Shifted mode LEVEL2 for DEVINFO_SWCAPA      */
#define DEVINFO_SWCAPA_GWEN                            (0x1UL << 29)                           /**< Gateway                                     */
#define _DEVINFO_SWCAPA_GWEN_SHIFT                     29                                      /**< Shift value for DEVINFO_GWEN                */
#define _DEVINFO_SWCAPA_GWEN_MASK                      0x20000000UL                            /**< Bit mask for DEVINFO_GWEN                   */
#define _DEVINFO_SWCAPA_GWEN_DEFAULT                   0x00000001UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define DEVINFO_SWCAPA_GWEN_DEFAULT                    (_DEVINFO_SWCAPA_GWEN_DEFAULT << 29)    /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_NCPEN                           (0x1UL << 30)                           /**< NCP                                         */
#define _DEVINFO_SWCAPA_NCPEN_SHIFT                    30                                      /**< Shift value for DEVINFO_NCPEN               */
#define _DEVINFO_SWCAPA_NCPEN_MASK                     0x40000000UL                            /**< Bit mask for DEVINFO_NCPEN                  */
#define _DEVINFO_SWCAPA_NCPEN_DEFAULT                  0x00000001UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define DEVINFO_SWCAPA_NCPEN_DEFAULT                   (_DEVINFO_SWCAPA_NCPEN_DEFAULT << 30)   /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */
#define DEVINFO_SWCAPA_RFMCUEN                         (0x1UL << 31)                           /**< RF-MCU                                      */
#define _DEVINFO_SWCAPA_RFMCUEN_SHIFT                  31                                      /**< Shift value for DEVINFO_RFMCUEN             */
#define _DEVINFO_SWCAPA_RFMCUEN_MASK                   0x80000000UL                            /**< Bit mask for DEVINFO_RFMCUEN                */
#define _DEVINFO_SWCAPA_RFMCUEN_DEFAULT                0x00000001UL                            /**< Mode DEFAULT for DEVINFO_SWCAPA             */
#define DEVINFO_SWCAPA_RFMCUEN_DEFAULT                 (_DEVINFO_SWCAPA_RFMCUEN_DEFAULT << 31) /**< Shifted mode DEFAULT for DEVINFO_SWCAPA     */

/* Bit fields for DEVINFO HFRCOCALDEFAULT */
#define _DEVINFO_HFRCOCALDEFAULT_RESETVALUE            0x00000000UL                                        /**< Default value for DEVINFO_HFRCOCALDEFAULT   */
#define _DEVINFO_HFRCOCALDEFAULT_MASK                  0xFFFFFFFFUL                                        /**< Mask for DEVINFO_HFRCOCALDEFAULT            */
#define _DEVINFO_HFRCOCALDEFAULT_TUNING_SHIFT          0                                                   /**< Shift value for DEVINFO_TUNING              */
#define _DEVINFO_HFRCOCALDEFAULT_TUNING_MASK           0x3FUL                                              /**< Bit mask for DEVINFO_TUNING                 */
#define _DEVINFO_HFRCOCALDEFAULT_TUNING_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_TUNING_DEFAULT         (_DEVINFO_HFRCOCALDEFAULT_TUNING_DEFAULT << 0)      /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_FINETUNING_SHIFT      6                                                   /**< Shift value for DEVINFO_FINETUNING          */
#define _DEVINFO_HFRCOCALDEFAULT_FINETUNING_MASK       0xFC0UL                                             /**< Bit mask for DEVINFO_FINETUNING             */
#define _DEVINFO_HFRCOCALDEFAULT_FINETUNING_DEFAULT    0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_FINETUNING_DEFAULT     (_DEVINFO_HFRCOCALDEFAULT_FINETUNING_DEFAULT << 6)  /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_BANDCOARSE_SHIFT      12                                                  /**< Shift value for DEVINFO_BANDCOARSE          */
#define _DEVINFO_HFRCOCALDEFAULT_BANDCOARSE_MASK       0x7000UL                                            /**< Bit mask for DEVINFO_BANDCOARSE             */
#define _DEVINFO_HFRCOCALDEFAULT_BANDCOARSE_DEFAULT    0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_BANDCOARSE_DEFAULT     (_DEVINFO_HFRCOCALDEFAULT_BANDCOARSE_DEFAULT << 12) /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_BANDFINE_SHIFT        15                                                  /**< Shift value for DEVINFO_BANDFINE            */
#define _DEVINFO_HFRCOCALDEFAULT_BANDFINE_MASK         0x1F8000UL                                          /**< Bit mask for DEVINFO_BANDFINE               */
#define _DEVINFO_HFRCOCALDEFAULT_BANDFINE_DEFAULT      0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_BANDFINE_DEFAULT       (_DEVINFO_HFRCOCALDEFAULT_BANDFINE_DEFAULT << 15)   /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_CLKDIV_SHIFT          21                                                  /**< Shift value for DEVINFO_CLKDIV              */
#define _DEVINFO_HFRCOCALDEFAULT_CLKDIV_MASK           0x600000UL                                          /**< Bit mask for DEVINFO_CLKDIV                 */
#define _DEVINFO_HFRCOCALDEFAULT_CLKDIV_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_CLKDIV_DEFAULT         (_DEVINFO_HFRCOCALDEFAULT_CLKDIV_DEFAULT << 21)     /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_SIGMADELTA_SHIFT      23                                                  /**< Shift value for DEVINFO_SIGMADELTA          */
#define _DEVINFO_HFRCOCALDEFAULT_SIGMADELTA_MASK       0x1800000UL                                         /**< Bit mask for DEVINFO_SIGMADELTA             */
#define _DEVINFO_HFRCOCALDEFAULT_SIGMADELTA_DEFAULT    0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_SIGMADELTA_DEFAULT     (_DEVINFO_HFRCOCALDEFAULT_SIGMADELTA_DEFAULT << 23) /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_BIAS_SHIFT            25                                                  /**< Shift value for DEVINFO_BIAS                */
#define _DEVINFO_HFRCOCALDEFAULT_BIAS_MASK             0xE000000UL                                         /**< Bit mask for DEVINFO_BIAS                   */
#define _DEVINFO_HFRCOCALDEFAULT_BIAS_DEFAULT          0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_BIAS_DEFAULT           (_DEVINFO_HFRCOCALDEFAULT_BIAS_DEFAULT << 25)       /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/
#define _DEVINFO_HFRCOCALDEFAULT_TEMPCO_SHIFT          28                                                  /**< Shift value for DEVINFO_TEMPCO              */
#define _DEVINFO_HFRCOCALDEFAULT_TEMPCO_MASK           0xF0000000UL                                        /**< Bit mask for DEVINFO_TEMPCO                 */
#define _DEVINFO_HFRCOCALDEFAULT_TEMPCO_DEFAULT        0x00000000UL                                        /**< Mode DEFAULT for DEVINFO_HFRCOCALDEFAULT    */
#define DEVINFO_HFRCOCALDEFAULT_TEMPCO_DEFAULT         (_DEVINFO_HFRCOCALDEFAULT_TEMPCO_DEFAULT << 28)     /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALDEFAULT*/

/* Bit fields for DEVINFO HFRCOCALSPEED */
#define _DEVINFO_HFRCOCALSPEED_RESETVALUE              0x00000000UL                                      /**< Default value for DEVINFO_HFRCOCALSPEED     */
#define _DEVINFO_HFRCOCALSPEED_MASK                    0xFFFFFFFFUL                                      /**< Mask for DEVINFO_HFRCOCALSPEED              */
#define _DEVINFO_HFRCOCALSPEED_TUNING_SHIFT            0                                                 /**< Shift value for DEVINFO_TUNING              */
#define _DEVINFO_HFRCOCALSPEED_TUNING_MASK             0x3FUL                                            /**< Bit mask for DEVINFO_TUNING                 */
#define _DEVINFO_HFRCOCALSPEED_TUNING_DEFAULT          0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_TUNING_DEFAULT           (_DEVINFO_HFRCOCALSPEED_TUNING_DEFAULT << 0)      /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_FINETUNING_SHIFT        6                                                 /**< Shift value for DEVINFO_FINETUNING          */
#define _DEVINFO_HFRCOCALSPEED_FINETUNING_MASK         0xFC0UL                                           /**< Bit mask for DEVINFO_FINETUNING             */
#define _DEVINFO_HFRCOCALSPEED_FINETUNING_DEFAULT      0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_FINETUNING_DEFAULT       (_DEVINFO_HFRCOCALSPEED_FINETUNING_DEFAULT << 6)  /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_BANDCOARSE_SHIFT        12                                                /**< Shift value for DEVINFO_BANDCOARSE          */
#define _DEVINFO_HFRCOCALSPEED_BANDCOARSE_MASK         0x7000UL                                          /**< Bit mask for DEVINFO_BANDCOARSE             */
#define _DEVINFO_HFRCOCALSPEED_BANDCOARSE_DEFAULT      0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_BANDCOARSE_DEFAULT       (_DEVINFO_HFRCOCALSPEED_BANDCOARSE_DEFAULT << 12) /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_BANDFINE_SHIFT          15                                                /**< Shift value for DEVINFO_BANDFINE            */
#define _DEVINFO_HFRCOCALSPEED_BANDFINE_MASK           0x1F8000UL                                        /**< Bit mask for DEVINFO_BANDFINE               */
#define _DEVINFO_HFRCOCALSPEED_BANDFINE_DEFAULT        0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_BANDFINE_DEFAULT         (_DEVINFO_HFRCOCALSPEED_BANDFINE_DEFAULT << 15)   /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_CLKDIV_SHIFT            21                                                /**< Shift value for DEVINFO_CLKDIV              */
#define _DEVINFO_HFRCOCALSPEED_CLKDIV_MASK             0x600000UL                                        /**< Bit mask for DEVINFO_CLKDIV                 */
#define _DEVINFO_HFRCOCALSPEED_CLKDIV_DEFAULT          0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_CLKDIV_DEFAULT           (_DEVINFO_HFRCOCALSPEED_CLKDIV_DEFAULT << 21)     /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_SIGMADELTA_SHIFT        23                                                /**< Shift value for DEVINFO_SIGMADELTA          */
#define _DEVINFO_HFRCOCALSPEED_SIGMADELTA_MASK         0x1800000UL                                       /**< Bit mask for DEVINFO_SIGMADELTA             */
#define _DEVINFO_HFRCOCALSPEED_SIGMADELTA_DEFAULT      0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_SIGMADELTA_DEFAULT       (_DEVINFO_HFRCOCALSPEED_SIGMADELTA_DEFAULT << 23) /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_BIAS_SHIFT              25                                                /**< Shift value for DEVINFO_BIAS                */
#define _DEVINFO_HFRCOCALSPEED_BIAS_MASK               0xE000000UL                                       /**< Bit mask for DEVINFO_BIAS                   */
#define _DEVINFO_HFRCOCALSPEED_BIAS_DEFAULT            0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_BIAS_DEFAULT             (_DEVINFO_HFRCOCALSPEED_BIAS_DEFAULT << 25)       /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/
#define _DEVINFO_HFRCOCALSPEED_TEMPCO_SHIFT            28                                                /**< Shift value for DEVINFO_TEMPCO              */
#define _DEVINFO_HFRCOCALSPEED_TEMPCO_MASK             0xF0000000UL                                      /**< Bit mask for DEVINFO_TEMPCO                 */
#define _DEVINFO_HFRCOCALSPEED_TEMPCO_DEFAULT          0x00000000UL                                      /**< Mode DEFAULT for DEVINFO_HFRCOCALSPEED      */
#define DEVINFO_HFRCOCALSPEED_TEMPCO_DEFAULT           (_DEVINFO_HFRCOCALSPEED_TEMPCO_DEFAULT << 28)     /**< Shifted mode DEFAULT for DEVINFO_HFRCOCALSPEED*/

/* Bit fields for DEVINFO EUI64L */
#define _DEVINFO_EUI64L_RESETVALUE                     0x00000000UL                           /**< Default value for DEVINFO_EUI64L            */
#define _DEVINFO_EUI64L_MASK                           0xFFFFFFFFUL                           /**< Mask for DEVINFO_EUI64L                     */
#define _DEVINFO_EUI64L_UNIQUEL_SHIFT                  0                                      /**< Shift value for DEVINFO_UNIQUEL             */
#define _DEVINFO_EUI64L_UNIQUEL_MASK                   0xFFFFFFFFUL                           /**< Bit mask for DEVINFO_UNIQUEL                */
#define _DEVINFO_EUI64L_UNIQUEL_DEFAULT                0x00000000UL                           /**< Mode DEFAULT for DEVINFO_EUI64L             */
#define DEVINFO_EUI64L_UNIQUEL_DEFAULT                 (_DEVINFO_EUI64L_UNIQUEL_DEFAULT << 0) /**< Shifted mode DEFAULT for DEVINFO_EUI64L     */

/* Bit fields for DEVINFO EUI64H */
#define _DEVINFO_EUI64H_RESETVALUE                     0x00000000UL                           /**< Default value for DEVINFO_EUI64H            */
#define _DEVINFO_EUI64H_MASK                           0xFFFFFFFFUL                           /**< Mask for DEVINFO_EUI64H                     */
#define _DEVINFO_EUI64H_UNIQUEH_SHIFT                  0                                      /**< Shift value for DEVINFO_UNIQUEH             */
#define _DEVINFO_EUI64H_UNIQUEH_MASK                   0xFFUL                                 /**< Bit mask for DEVINFO_UNIQUEH                */
#define _DEVINFO_EUI64H_UNIQUEH_DEFAULT                0x00000000UL                           /**< Mode DEFAULT for DEVINFO_EUI64H             */
#define DEVINFO_EUI64H_UNIQUEH_DEFAULT                 (_DEVINFO_EUI64H_UNIQUEH_DEFAULT << 0) /**< Shifted mode DEFAULT for DEVINFO_EUI64H     */
#define _DEVINFO_EUI64H_OUI_SHIFT                      8                                      /**< Shift value for DEVINFO_OUI                 */
#define _DEVINFO_EUI64H_OUI_MASK                       0xFFFFFF00UL                           /**< Bit mask for DEVINFO_OUI                    */
#define _DEVINFO_EUI64H_OUI_DEFAULT                    0x00000000UL                           /**< Mode DEFAULT for DEVINFO_EUI64H             */
#define DEVINFO_EUI64H_OUI_DEFAULT                     (_DEVINFO_EUI64H_OUI_DEFAULT << 8)     /**< Shifted mode DEFAULT for DEVINFO_EUI64H     */

/* Bit fields for DEVINFO PART0 */
#define _DEVINFO_PART0_RESETVALUE                      0x3033474DUL                                /**< Default value for DEVINFO_PART0             */
#define _DEVINFO_PART0_MASK                            0xFFFFFFFFUL                                /**< Mask for DEVINFO_PART0                      */
#define _DEVINFO_PART0_PROTOCOL_SHIFT                  0                                           /**< Shift value for DEVINFO_PROTOCOL            */
#define _DEVINFO_PART0_PROTOCOL_MASK                   0xFFUL                                      /**< Bit mask for DEVINFO_PROTOCOL               */
#define _DEVINFO_PART0_PROTOCOL_DEFAULT                0x0000004DUL                                /**< Mode DEFAULT for DEVINFO_PART0              */
#define _DEVINFO_PART0_PROTOCOL_BLUETOOTH              0x00000042UL                                /**< Mode BLUETOOTH for DEVINFO_PART0            */
#define _DEVINFO_PART0_PROTOCOL_PROPRIETARY            0x00000046UL                                /**< Mode PROPRIETARY for DEVINFO_PART0          */
#define _DEVINFO_PART0_PROTOCOL_FIFTEENPFOUR           0x0000004DUL                                /**< Mode FIFTEENPFOUR for DEVINFO_PART0         */
#define _DEVINFO_PART0_PROTOCOL_PEARL                  0x00000050UL                                /**< Mode PEARL for DEVINFO_PART0                */
#define _DEVINFO_PART0_PROTOCOL_WIFI                   0x00000057UL                                /**< Mode WIFI for DEVINFO_PART0                 */
#define _DEVINFO_PART0_PROTOCOL_ZWAVE                  0x0000005AUL                                /**< Mode ZWAVE for DEVINFO_PART0                */
#define DEVINFO_PART0_PROTOCOL_DEFAULT                 (_DEVINFO_PART0_PROTOCOL_DEFAULT << 0)      /**< Shifted mode DEFAULT for DEVINFO_PART0      */
#define DEVINFO_PART0_PROTOCOL_BLUETOOTH               (_DEVINFO_PART0_PROTOCOL_BLUETOOTH << 0)    /**< Shifted mode BLUETOOTH for DEVINFO_PART0    */
#define DEVINFO_PART0_PROTOCOL_PROPRIETARY             (_DEVINFO_PART0_PROTOCOL_PROPRIETARY << 0)  /**< Shifted mode PROPRIETARY for DEVINFO_PART0  */
#define DEVINFO_PART0_PROTOCOL_FIFTEENPFOUR            (_DEVINFO_PART0_PROTOCOL_FIFTEENPFOUR << 0) /**< Shifted mode FIFTEENPFOUR for DEVINFO_PART0 */
#define DEVINFO_PART0_PROTOCOL_PEARL                   (_DEVINFO_PART0_PROTOCOL_PEARL << 0)        /**< Shifted mode PEARL for DEVINFO_PART0        */
#define DEVINFO_PART0_PROTOCOL_WIFI                    (_DEVINFO_PART0_PROTOCOL_WIFI << 0)         /**< Shifted mode WIFI for DEVINFO_PART0         */
#define DEVINFO_PART0_PROTOCOL_ZWAVE                   (_DEVINFO_PART0_PROTOCOL_ZWAVE << 0)        /**< Shifted mode ZWAVE for DEVINFO_PART0        */
#define _DEVINFO_PART0_TYPE_SHIFT                      8                                           /**< Shift value for DEVINFO_TYPE                */
#define _DEVINFO_PART0_TYPE_MASK                       0xFF00UL                                    /**< Bit mask for DEVINFO_TYPE                   */
#define _DEVINFO_PART0_TYPE_DEFAULT                    0x00000047UL                                /**< Mode DEFAULT for DEVINFO_PART0              */
#define DEVINFO_PART0_TYPE_DEFAULT                     (_DEVINFO_PART0_TYPE_DEFAULT << 8)          /**< Shifted mode DEFAULT for DEVINFO_PART0      */
#define _DEVINFO_PART0_SERIES_SHIFT                    16                                          /**< Shift value for DEVINFO_SERIES              */
#define _DEVINFO_PART0_SERIES_MASK                     0xFF0000UL                                  /**< Bit mask for DEVINFO_SERIES                 */
#define _DEVINFO_PART0_SERIES_DEFAULT                  0x00000033UL                                /**< Mode DEFAULT for DEVINFO_PART0              */
#define DEVINFO_PART0_SERIES_DEFAULT                   (_DEVINFO_PART0_SERIES_DEFAULT << 16)       /**< Shifted mode DEFAULT for DEVINFO_PART0      */
#define _DEVINFO_PART0_DIECODE0_SHIFT                  24                                          /**< Shift value for DEVINFO_DIECODE0            */
#define _DEVINFO_PART0_DIECODE0_MASK                   0xFF000000UL                                /**< Bit mask for DEVINFO_DIECODE0               */
#define _DEVINFO_PART0_DIECODE0_DEFAULT                0x00000030UL                                /**< Mode DEFAULT for DEVINFO_PART0              */
#define DEVINFO_PART0_DIECODE0_DEFAULT                 (_DEVINFO_PART0_DIECODE0_DEFAULT << 24)     /**< Shifted mode DEFAULT for DEVINFO_PART0      */

/* Bit fields for DEVINFO PART1 */
#define _DEVINFO_PART1_RESETVALUE                      0x30314D31UL                            /**< Default value for DEVINFO_PART1             */
#define _DEVINFO_PART1_MASK                            0xFFFFFFFFUL                            /**< Mask for DEVINFO_PART1                      */
#define _DEVINFO_PART1_DIECODE1_SHIFT                  0                                       /**< Shift value for DEVINFO_DIECODE1            */
#define _DEVINFO_PART1_DIECODE1_MASK                   0xFFUL                                  /**< Bit mask for DEVINFO_DIECODE1               */
#define _DEVINFO_PART1_DIECODE1_DEFAULT                0x00000031UL                            /**< Mode DEFAULT for DEVINFO_PART1              */
#define DEVINFO_PART1_DIECODE1_DEFAULT                 (_DEVINFO_PART1_DIECODE1_DEFAULT << 0)  /**< Shifted mode DEFAULT for DEVINFO_PART1      */
#define _DEVINFO_PART1_PACKAGE_SHIFT                   8                                       /**< Shift value for DEVINFO_PACKAGE             */
#define _DEVINFO_PART1_PACKAGE_MASK                    0xFF00UL                                /**< Bit mask for DEVINFO_PACKAGE                */
#define _DEVINFO_PART1_PACKAGE_DEFAULT                 0x0000004DUL                            /**< Mode DEFAULT for DEVINFO_PART1              */
#define DEVINFO_PART1_PACKAGE_DEFAULT                  (_DEVINFO_PART1_PACKAGE_DEFAULT << 8)   /**< Shifted mode DEFAULT for DEVINFO_PART1      */
#define _DEVINFO_PART1_FEATURE1_SHIFT                  16                                      /**< Shift value for DEVINFO_FEATURE1            */
#define _DEVINFO_PART1_FEATURE1_MASK                   0xFF0000UL                              /**< Bit mask for DEVINFO_FEATURE1               */
#define _DEVINFO_PART1_FEATURE1_DEFAULT                0x00000031UL                            /**< Mode DEFAULT for DEVINFO_PART1              */
#define DEVINFO_PART1_FEATURE1_DEFAULT                 (_DEVINFO_PART1_FEATURE1_DEFAULT << 16) /**< Shifted mode DEFAULT for DEVINFO_PART1      */
#define _DEVINFO_PART1_FEATURE2_SHIFT                  24                                      /**< Shift value for DEVINFO_FEATURE2            */
#define _DEVINFO_PART1_FEATURE2_MASK                   0xFF000000UL                            /**< Bit mask for DEVINFO_FEATURE2               */
#define _DEVINFO_PART1_FEATURE2_DEFAULT                0x00000030UL                            /**< Mode DEFAULT for DEVINFO_PART1              */
#define _DEVINFO_PART1_FEATURE2_NOLED                  0x00000030UL                            /**< Mode NOLED for DEVINFO_PART1                */
#define _DEVINFO_PART1_FEATURE2_LED                    0x00000031UL                            /**< Mode LED for DEVINFO_PART1                  */
#define DEVINFO_PART1_FEATURE2_DEFAULT                 (_DEVINFO_PART1_FEATURE2_DEFAULT << 24) /**< Shifted mode DEFAULT for DEVINFO_PART1      */
#define DEVINFO_PART1_FEATURE2_NOLED                   (_DEVINFO_PART1_FEATURE2_NOLED << 24)   /**< Shifted mode NOLED for DEVINFO_PART1        */
#define DEVINFO_PART1_FEATURE2_LED                     (_DEVINFO_PART1_FEATURE2_LED << 24)     /**< Shifted mode LED for DEVINFO_PART1          */

/* Bit fields for DEVINFO PART2 */
#define _DEVINFO_PART2_RESETVALUE                      0x00000000UL                             /**< Default value for DEVINFO_PART2             */
#define _DEVINFO_PART2_MASK                            0xFFFFFFFFUL                             /**< Mask for DEVINFO_PART2                      */
#define _DEVINFO_PART2_FEATURE3_SHIFT                  0                                        /**< Shift value for DEVINFO_FEATURE3            */
#define _DEVINFO_PART2_FEATURE3_MASK                   0xFFUL                                   /**< Bit mask for DEVINFO_FEATURE3               */
#define _DEVINFO_PART2_FEATURE3_DEFAULT                0x00000000UL                             /**< Mode DEFAULT for DEVINFO_PART2              */
#define DEVINFO_PART2_FEATURE3_DEFAULT                 (_DEVINFO_PART2_FEATURE3_DEFAULT << 0)   /**< Shifted mode DEFAULT for DEVINFO_PART2      */
#define _DEVINFO_PART2_FLASH_SHIFT                     8                                        /**< Shift value for DEVINFO_FLASH               */
#define _DEVINFO_PART2_FLASH_MASK                      0xFF00UL                                 /**< Bit mask for DEVINFO_FLASH                  */
#define _DEVINFO_PART2_FLASH_DEFAULT                   0x00000000UL                             /**< Mode DEFAULT for DEVINFO_PART2              */
#define DEVINFO_PART2_FLASH_DEFAULT                    (_DEVINFO_PART2_FLASH_DEFAULT << 8)      /**< Shifted mode DEFAULT for DEVINFO_PART2      */
#define _DEVINFO_PART2_TEMPGRADE_SHIFT                 16                                       /**< Shift value for DEVINFO_TEMPGRADE           */
#define _DEVINFO_PART2_TEMPGRADE_MASK                  0xFF0000UL                               /**< Bit mask for DEVINFO_TEMPGRADE              */
#define _DEVINFO_PART2_TEMPGRADE_DEFAULT               0x00000000UL                             /**< Mode DEFAULT for DEVINFO_PART2              */
#define DEVINFO_PART2_TEMPGRADE_DEFAULT                (_DEVINFO_PART2_TEMPGRADE_DEFAULT << 16) /**< Shifted mode DEFAULT for DEVINFO_PART2      */
#define _DEVINFO_PART2_PINCOUNT_SHIFT                  24                                       /**< Shift value for DEVINFO_PINCOUNT            */
#define _DEVINFO_PART2_PINCOUNT_MASK                   0xFF000000UL                             /**< Bit mask for DEVINFO_PINCOUNT               */
#define _DEVINFO_PART2_PINCOUNT_DEFAULT                0x00000000UL                             /**< Mode DEFAULT for DEVINFO_PART2              */
#define DEVINFO_PART2_PINCOUNT_DEFAULT                 (_DEVINFO_PART2_PINCOUNT_DEFAULT << 24)  /**< Shifted mode DEFAULT for DEVINFO_PART2      */

/** @} End of group SIBG301_DEVINFO_BitFields */
/** @} End of group SIBG301_DEVINFO */
/** @} End of group Parts */

#endif // SIBG301_DEVINFO_H
