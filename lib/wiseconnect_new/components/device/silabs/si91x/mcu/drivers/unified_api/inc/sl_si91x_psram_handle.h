/******************************************************************************
* @file  sl_si91x_psram_handle.h
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
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
******************************************************************************/

#ifndef __SL_SI91X_PSRAM_HANDLE_H_
#define __SL_SI91X_PSRAM_HANDLE_H_

#include "sl_si91x_psram.h"
#include "sl_si91x_psram_pin_config.h"
#ifdef SLI_SI91X_MCU_PSRAM_APS1604M_SQR
#include "sl_si91x_psram_aps1604m_sqr_config.h"
#elif defined(SLI_SI91X_MCU_PSRAM_APS6404L_SQH)
#include "sl_si91x_psram_aps6404l_sqh_config.h"
#else
#include "sl_si91x_psram_aps6404l_sqrh_config.h"
#endif

/**
 * @file sl_si91x_psram_handle.h
 * @brief Handle for PSRAM operations and configuration.
 */

/**
 * @brief Handle for PSRAM Operations
 */
extern struct sl_psram_info_type_t PSRAM_Device;

/**
 * @addtogroup PSRAM_GPIO_PIN_SET PSRAM GPIO Pin Sets
 * @ingroup PSRAM
 * @{
 */
#define PSRAM_GPIO_PIN_SET_52_TO_57      1 /**< GPIO Pin Set 52 to 57 */
#define PSRAM_GPIO_PIN_SET_0_TO_5        2 /**< GPIO Pin Set 0 to 5 */
#define PSRAM_GPIO_PIN_SET_46_TO_51_CS_0 3 /**< GPIO Pin Set 46 to 51 with Chip Select 0 */
#define PSRAM_GPIO_PIN_SET_46_TO_51_CS_1 4 /**< GPIO Pin Set 46 to 51 with Chip Select 1 */
#define PSRAM_GPIO_PIN_SET_46_TO_57_CS_0 5 /**< GPIO Pin Set 46 to 57 with Chip Select 0 */
#define PSRAM_GPIO_PIN_SET_46_TO_57_CS_1 6 /**< GPIO Pin Set 46 to 57 with Chip Select 1 */
/// @} end group PSRAM_GPIO_PIN_SET

/**
 * @addtogroup PSRAM_CHIP_SELECT PSRAM Chip Select and Base Address
 * @ingroup PSRAM
 * @{
 */
#if (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_52_TO_57)
#define PSRAM_CHIP_SELECT  (CHIP_ZERO)  /**< Chip Select for GPIO Pin Set 52 to 57 */
#define PSRAM_BASE_ADDRESS (0x0A000000) /**< Base Address for GPIO Pin Set 52 to 57 */
#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_0_TO_5)
#define PSRAM_CHIP_SELECT  (CHIP_ZERO)  /**< Chip Select for GPIO Pin Set 0 to 5 */
#define PSRAM_BASE_ADDRESS (0x0A000000) /**< Base Address for GPIO Pin Set 0 to 5 */
#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_0)
#define PSRAM_CHIP_SELECT  (CHIP_ZERO)  /**< Chip Select for GPIO Pin Set 46 to 51 with CS 0 */
#define PSRAM_BASE_ADDRESS (0x0A000000) /**< Base Address for GPIO Pin Set 46 to 51 with CS 0 */
#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_1)
#define PSRAM_CHIP_SELECT  (CHIP_ONE)   /**< Chip Select for GPIO Pin Set 46 to 51 with CS 1 */
#define PSRAM_BASE_ADDRESS (0x0B000000) /**< Base Address for GPIO Pin Set 46 to 51 with CS 1 */
#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_0)
#define PSRAM_CHIP_SELECT  (CHIP_ZERO)  /**< Chip Select for GPIO Pin Set 46 to 57 with CS 0 */
#define PSRAM_BASE_ADDRESS (0x0A000000) /**< Base Address for GPIO Pin Set 46 to 57 with CS 0 */
#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_1)
#define PSRAM_CHIP_SELECT  (CHIP_ONE)   /**< Chip Select for GPIO Pin Set 46 to 57 with CS 1 */
#define PSRAM_BASE_ADDRESS (0x0B000000) /**< Base Address for GPIO Pin Set 46 to 57 with CS 1 */
#endif
/// @} end group PSRAM_CHIP_SELECT

/**
 * @addtogroup PSRAM_PIN_CONFIG PSRAM Pin Configuration
 * @ingroup PSRAM
 * @{
 */
#if (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_0_TO_5)

#define M4SS_PSRAM_CLK_PORT (0) /**< Clock Port for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_CLK_PIN  (0) /**< Clock Pin for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_CLK_MUX  (2) /**< Clock Mux for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_CLK_PAD  (0) /**< Clock Pad for GPIO Pin Set 0 to 5 */

#define M4SS_PSRAM_CSN_PORT (0) /**< Chip Select Port for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_CSN_PIN  (1) /**< Chip Select Pin for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_CSN_MUX  (2) /**< Chip Select Mux for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_CSN_PAD  (0) /**< Chip Select Pad for GPIO Pin Set 0 to 5 */

#define M4SS_PSRAM_D0_PORT (0) /**< Data 0 Port for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D0_PIN  (2) /**< Data 0 Pin for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D0_MUX  (2) /**< Data 0 Mux for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D0_PAD  (0) /**< Data 0 Pad for GPIO Pin Set 0 to 5 */

#define M4SS_PSRAM_D1_PORT (0) /**< Data 1 Port for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D1_PIN  (3) /**< Data 1 Pin for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D1_MUX  (2) /**< Data 1 Mux for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D1_PAD  (0) /**< Data 1 Pad for GPIO Pin Set 0 to 5 */

#define M4SS_PSRAM_D2_PORT (0) /**< Data 2 Port for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D2_PIN  (4) /**< Data 2 Pin for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D2_MUX  (2) /**< Data 2 Mux for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D2_PAD  (0) /**< Data 2 Pad for GPIO Pin Set 0 to 5 */

#define M4SS_PSRAM_D3_PORT (0) /**< Data 3 Port for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D3_PIN  (5) /**< Data 3 Pin for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D3_MUX  (2) /**< Data 3 Mux for GPIO Pin Set 0 to 5 */
#define M4SS_PSRAM_D3_PAD  (0) /**< Data 3 Pad for GPIO Pin Set 0 to 5 */

#define NUM_OF_PSRAM_PINS (6) /**< Number of PSRAM Pins for GPIO Pin Set 0 to 5 */

#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_0) \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_1)  \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_0)  \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_1)

#define M4SS_PSRAM_CLK_PORT (2)  /**< Clock Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_CLK_PIN  (14) /**< Clock Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_CLK_MUX  (11) /**< Clock Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_CLK_PAD  (10) /**< Clock Pad for GPIO Pin Set 46 to 57 */

#if (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_0) \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_0)
#define M4SS_PSRAM_CSN_PORT (3)  /**< Chip Select Port for GPIO Pin Set 46 to 51 with CS 0 */
#define M4SS_PSRAM_CSN_PIN  (01) /**< Chip Select Pin for GPIO Pin Set 46 to 51 with CS 0 */
#define M4SS_PSRAM_CSN_MUX  (11) /**< Chip Select Mux for GPIO Pin Set 46 to 51 with CS 0 */
#define M4SS_PSRAM_CSN_PAD  (13) /**< Chip Select Pad for GPIO Pin Set 46 to 51 with CS 0 */
#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_1) \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_1)
#define M4SS_PSRAM_CSN_PORT (3)  /**< Chip Select Port for GPIO Pin Set 46 to 51 with CS 1 */
#define M4SS_PSRAM_CSN_PIN  (05) /**< Chip Select Pin for GPIO Pin Set 46 to 51 with CS 1 */
#define M4SS_PSRAM_CSN_MUX  (11) /**< Chip Select Mux for GPIO Pin Set 46 to 51 with CS 1 */
#define M4SS_PSRAM_CSN_PAD  (17) /**< Chip Select Pad for GPIO Pin Set 46 to 51 with CS 1 */
#endif

#define M4SS_PSRAM_D0_PORT (2)  /**< Data 0 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D0_PIN  (15) /**< Data 0 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D0_MUX  (11) /**< Data 0 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D0_PAD  (11) /**< Data 0 Pad for GPIO Pin Set 46 to 57 */

#define M4SS_PSRAM_D1_PORT (3)  /**< Data 1 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D1_PIN  (0)  /**< Data 1 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D1_MUX  (11) /**< Data 1 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D1_PAD  (12) /**< Data 1 Pad for GPIO Pin Set 46 to 57 */

#define M4SS_PSRAM_D2_PORT (3)  /**< Data 2 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D2_PIN  (2)  /**< Data 2 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D2_MUX  (11) /**< Data 2 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D2_PAD  (14) /**< Data 2 Pad for GPIO Pin Set 46 to 57 */

#define M4SS_PSRAM_D3_PORT (3)  /**< Data 3 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D3_PIN  (3)  /**< Data 3 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D3_MUX  (11) /**< Data 3 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D3_PAD  (15) /**< Data 3 Pad for GPIO Pin Set 46 to 57 */

#if (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_0) \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_57_CS_1)

#define M4SS_PSRAM_D4_PORT (3)  /**< Data 4 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D4_PIN  (6)  /**< Data 4 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D4_MUX  (11) /**< Data 4 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D4_PAD  (11) /**< Data 4 Pad for GPIO Pin Set 46 to 57 */

#define M4SS_PSRAM_D5_PORT (3)  /**< Data 5 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D5_PIN  (7)  /**< Data 5 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D5_MUX  (11) /**< Data 5 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D5_PAD  (12) /**< Data 5 Pad for GPIO Pin Set 46 to 57 */

#define M4SS_PSRAM_D6_PORT (3)  /**< Data 6 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D6_PIN  (8)  /**< Data 6 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D6_MUX  (11) /**< Data 6 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D6_PAD  (14) /**< Data 6 Pad for GPIO Pin Set 46 to 57 */

#define M4SS_PSRAM_D7_PORT (3)  /**< Data 7 Port for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D7_PIN  (9)  /**< Data 7 Pin for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D7_MUX  (11) /**< Data 7 Mux for GPIO Pin Set 46 to 57 */
#define M4SS_PSRAM_D7_PAD  (15) /**< Data 7 Pad for GPIO Pin Set 46 to 57 */

#endif

#if (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_0) \
  || (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_46_TO_51_CS_1)
#define NUM_OF_PSRAM_PINS (6) /**< Number of PSRAM Pins for GPIO Pin Set 46 to 51 */
#else
#define NUM_OF_PSRAM_PINS (10) /**< Number of PSRAM Pins for GPIO Pin Set 46 to 57 */
#endif

#elif (PSRAM_GPIO_PIN_SET_SEL == PSRAM_GPIO_PIN_SET_52_TO_57)

#define M4SS_PSRAM_CLK_PORT (3)  /**< Clock Port for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_CLK_PIN  (4)  /**< Clock Pin for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_CLK_MUX  (12) /**< Clock Mux for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_CLK_PAD  (16) /**< Clock Pad for GPIO Pin Set 52 to 57 */

#define M4SS_PSRAM_CSN_PORT (3)  /**< Chip Select Port for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_CSN_PIN  (7)  /**< Chip Select Pin for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_CSN_MUX  (12) /**< Chip Select Mux for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_CSN_PAD  (19) /**< Chip Select Pad for GPIO Pin Set 52 to 57 */

#define M4SS_PSRAM_D0_PORT (3)  /**< Data 0 Port for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D0_PIN  (5)  /**< Data 0 Pin for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D0_MUX  (12) /**< Data 0 Mux for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D0_PAD  (17) /**< Data 0 Pad for GPIO Pin Set 52 to 57 */

#define M4SS_PSRAM_D1_PORT (3)  /**< Data 1 Port for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D1_PIN  (6)  /**< Data 1 Pin for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D1_MUX  (12) /**< Data 1 Mux for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D1_PAD  (18) /**< Data 1 Pad for GPIO Pin Set 52 to 57 */

#define M4SS_PSRAM_D2_PORT (3)  /**< Data 2 Port for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D2_PIN  (8)  /**< Data 2 Pin for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D2_MUX  (12) /**< Data 2 Mux for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D2_PAD  (20) /**< Data 2 Pad for GPIO Pin Set 52 to 57 */

#define M4SS_PSRAM_D3_PORT (3)  /**< Data 3 Port for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D3_PIN  (9)  /**< Data 3 Pin for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D3_MUX  (12) /**< Data 3 Mux for GPIO Pin Set 52 to 57 */
#define M4SS_PSRAM_D3_PAD  (21) /**< Data 3 Pad for GPIO Pin Set 52 to 57 */

#define NUM_OF_PSRAM_PINS (6) /**< Number of PSRAM Pins for GPIO Pin Set 52 to 57 */

#endif
/// @} end group PSRAM_PIN_CONFIG

#endif //__SL_SI91X_PSRAM_HANDLE_H_