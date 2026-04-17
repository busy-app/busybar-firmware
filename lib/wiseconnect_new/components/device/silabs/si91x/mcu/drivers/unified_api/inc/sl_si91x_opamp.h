/******************************************************************************
* @file  sl_si91x_opamp.h
* @brief OPAMP API header file
*******************************************************************************
* # License
* <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef SL_SI91X_OPAMP_H
#define SL_SI91X_OPAMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "si91x_device.h" // Device-specific definitions
#include "sl_status.h"    // Status codes
#include "rsi_opamp.h"    // OPAMP driver functions

/***************************************************************************/
/**
 * @addtogroup OPAMP Operational Amplifier
 * @ingroup SI91X_PERIPHERAL_APIS
 * @{ 
 * 
 ******************************************************************************/
// -----------------------------------------------------------------------------
// Macros for opamp parameters
// -----------------------------------------------------------------------------
#define SL_OPAMP_OUT_MUX_SEL 1 ///< OPAMP1 (0-ULP_GPIO_4 , 1- GPIO_30)
/***************************************************************************/
/**
 * @brief Enumeration to represent the opamp number.
 * @details This enumeration defines the available OPAMP instances in the hardware.
 *          Each OPAMP instance corresponds to a specific hardware block.
 *
 *****************************************************************/
typedef enum {
  SL_OPAMP_1 = 1, ///< OPAMP 1 - First OPAMP instance
  SL_OPAMP_2,     ///< OPAMP 2 - Second OPAMP instance
  SL_OPAMP_3,     ///< OPAMP 3 - Third OPAMP instance
  SL_OPAMP_LAST   ///< Last member of enum for validation purposes
} sl_opamp_number_t;

/***************************************************************************/
/**
 * @brief Enumeration to represent OPAMP features.
 * @details This enumeration defines the various features and configurations supported
 *          by the OPAMP module. These features allow the OPAMP to be used in different
 *          operational modes depending on the application requirements.
 *
 *****************************************************************/
typedef enum {
  SL_OPAMP_UNITY_GAIN,                            ///< Unity gain - OPAMP operates as a buffer.
  SL_OPAMP_TRANS_IMPEDANCE_AMPLIFIER,             ///< Trans-Impedance Amplifier - Converts current to voltage.
  SL_OPAMP_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER, ///< Inverting Programmable Gain Amplifier - Amplifies signal with inversion.
  SL_OPAMP_NON_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER, ///< Non-inverting Programmable Gain Amplifier - Amplifies signal without inversion.
  SL_OPAMP_INVERTING_PROGRAMMABLE_HYST_COMP, ///< Inverting Programmable Hysteresis Comparator - Comparator with hysteresis.
  SL_OPAMP_NON_INVERTING_PROGRAMMABLE_HYST_COMP, ///< Non-inverting Programmable Hysteresis Comparator - Comparator with hysteresis.
  SL_OPAMP_INSTRUMENTATION_AMPLIFIER, ///< Instrumentation Amplifier - High precision differential amplifier.
  SL_OPAMP_CASCADED_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER, ///< Cascaded Inverting Programmable Gain Amp - Multi-stage amplification.
  SL_OPAMP_CASCADED_NON_INVERTING_PROGRAMMABLE_GAIN_AMPLIFIER, ///< Cascaded Non-inverting Programmable Gain Amp - Multi-stage amplification.
  SL_OPAMP_TWO_OPAMPS_DIFFERENTIAL_AMPLIFIER, ///< Two OPAMPs Differential Amplifier - Differential signal amplification.
  SL_OPAMP_CONFIGURATION_LAST                 ///< Last member of enum for validation purposes
} sl_opamp_feature_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the OPAMP instances and respective features.
 * @details This structure defines the configuration parameters and instances. 
 *          It is used to specify the OPAMP instance
 *          and the desired feature configuration.
 *
 *****************************************************************/
typedef struct {
  uint8_t opamp_number;               ///< OPAMP number, \ref sl_opamp_number_t for possible values.
  sl_opamp_feature_config_t features; ///< OPAMP features, \ref sl_opamp_feature_config_t for possible values.
  Configure_OPAMP_t opamp_config;     ///< Configuration parameters for all three OPAMPs (OPAMP1, OPAMP2, OPAMP3)
  Configure_OPAMP1_OPAMP2_t
    opamp1_opamp2_config; ///< Configuration parameters for cascaded OPAMP1 and OPAMP2 operation.
  Configure_OPAMP2_OPAMP3_t
    opamp2_opamp3_config; ///< Configuration parameters for cascaded OPAMP2 and OPAMP3 operation.
  OPAMP_CONFIG_T
  opamp123_config; ///< Combined configuration for OPAMP1, OPAMP2, and OPAMP3 (used for multi-opamp setups).
} sl_opamp_config_t;

/***************************************************************************/
/**
 * @brief Structure to hold the OPAMP GPIOs input and output.
 * @details This structure defines the configuration parameters for the OPAMP GPIO inputs
 *          and outputs, including pin selections.
 *
 *****************************************************************/
typedef struct {
  uint8_t vin_p_input;   ///< OPAMP GPIO Vinp input.
  uint8_t vin_n_input;   ///< OPAMP GPIO Vinn input.
  uint8_t vin_res_input; ///< OPAMP GPIO Resistor input.
  uint8_t vout_output;   ///< OPAMP GPIO Vout output.
} sl_opamp_pin_config_t;

/***************************************************************************/
/**
 * @brief Initialize the OPAMP module.
 * @details This API initializes the OPAMP module and prepares it for operation.
 *          It sets up the necessary hardware configurations and enables the OPAMP.
 *          This function must be called before using any other OPAMP-related APIs.
 *          Configures OPAMP reference voltage as 2.5V or 3.3V
 * 
 * @return Status of the OPAMP initialization operation:
 *         - SL_STATUS_OK if the initialization was successful.
 *         - SL_STATUS_FAIL if the initialization failed.
 *
 * For more information on the status documentation, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_opamp_init(void);

/***************************************************************************/
/**
 * @brief Deinitialize the OPAMP module.
 * @details This API deinitializes the OPAMP module and releases any resources used.
 *          It disables the OPAMP and resets its configuration to default values.
 *          This function should be called to clean up resources when OPAMP is no longer needed.
 * 
 * @pre Pre-condition: 
 *      - \ref sl_si91x_opamp_init() must be called before using this function.
 * 
 ******************************************************************************/
void sl_si91x_opamp_deinit(void);

/***************************************************************************/
/**
 * @brief Configure the OPAMP GPIOs by selecting pins for input and output.
 * @details This API sets up the configuration multiplexing for the OPAMP peripheral,
 *          ensuring the correct GPIO pins are configured for OPAMP operation.
 *          It maps the OPAMP input and output pins to the appropriate hardware pins.
 * 
 * @pre Pre-condition: 
 *      - \ref sl_si91x_opamp_init() must be called before using this function.
 * 
 * @param[in] opamp_config_ptr GPIO inputs and outputs pins selections Pointer to structure
 *             \ref sl_opamp_pin_config_t
 * 
 * @return Status code indicating the result:
 *         - SL_STATUS_OK if the OPAMP parameters were configured properly.
 *         - SL_STATUS_INVALID_PARAMETER if the configuration structure contains invalid values.
 *
 * For more information on the status documentation, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_opamp_pin_init(sl_opamp_pin_config_t *opamp_config_ptr);

/***************************************************************************/
/**
 * @brief Configure the OPAMP features and instance number.
 * @details This API configures the OPAMP by selecting features such as unity gain,
 *          non-inverting and inverting inputs, enabling/disabling hysteresis, and
 *          cascaded configurations based on the selected OPAMP instance number. 
 *
 * @pre Pre-condition: 
 *      - \ref sl_si91x_opamp_init() must be called before using this function.
 * 
 * @param[in] opamp_config OPAMP instance number and features selection Pointer to structure.
 *            \ref sl_opamp_config_t
 * 
 * @return Status of the OPAMP configuration operation:
 *         - SL_STATUS_OK if the configuration was successful.
 *         - SL_STATUS_INVALID_PARAMETER if the configuration structure contains invalid values.
 *
 * For more information on the status documentation, see 
 * [SL STATUS DOCUMENTATION](https://docs.silabs.com/gecko-platform/latest/platform-common/status).
 ******************************************************************************/
sl_status_t sl_si91x_opamp_set_configuration(sl_opamp_config_t *opamp_config);
/** @} end group OPAMP */

// ******** THE REST OF THE FILE IS DOCUMENTATION ONLY! ***********************
/*******************************************************************************/
/**
 * @addtogroup OPAMP Operational Amplifier
 * @{
 *
 * @details
 *
 * @section OPAMP_Intro Introduction
 *
 * The OPAMP peripheral is a versatile analog component that can be configured for various applications
 * such as signal amplification, buffering, and voltage comparison.
 * This APIs provides a step-by-step guide to initialize, configure and utilized the OPAMP peripheral based on
 * the selected instance,ensuring optimal performance for various use cases.
 *
 * **Key Features**:
 * 1. Support for both inverting and non-inverting inputs.
 * 2. Supports for two input terminals and single output.
 * 3. Supports multiple operational modes, including:
 *    - Unity Gain Buffer
 *    - Inverting and Non-Inverting Programmable Gain Amplifiers
 *    - Inverting and Non-Inverting with hysteresis comparators
 *    - Cascaded Inverting and Non-Inverting Programmable Gain Amplifiers
 *    - Two Opamps Differential Amplifier
 * 4. Provides programmable hysteresis for stable voltage comparison.
 * 5. Configurable GPIO pins for input and output signals.
 * 6. Provides flexible configuration through software APIs.
 *
 * @section OPAMP_Config Configuration
 *
 * The OPAMP can be configured using several features, including:
 * - **Unity Gain Buffer**: Operates as a voltage follower with no amplification.
 * - **Inverting Programmable Gain Amplifier**: Amplifies signal with inversion.
 * - **Non-Inverting Programmable Gain Amplifier**: Amplifies signal without inversion.
 * - **Inverting Programmable with Hysteresis Comparator**: The output switches when the input crosses a threshold and hysteresis helps prevent noise-induced toggling.
 * - **Non-Inverting Programmable with Hysteresis Comparator**: The output switches when the input exceeds a reference, with hysteresis for noise immunity.
 * - **Cascaded Inverting Programmable Gain Amplifier**: Supports cascaded inverting amplifier stages for increased gain and signal inversion.
 * - **Cascaded Non-Inverting Programmable Gain Amplifier**: Supports cascaded non-inverting amplifier stages for increased gain without signal inversion.
 * - **Two Opamps Differential Amplifier**: Amplifies the difference between two input signals using two OPAMPs, providing high common-mode noise rejection. Useful for extracting small differential signals in the presence of large common-mode voltages.
 * These configurations are encapsulated in the @ref sl_opamp_config_t structure and initialized using the @ref sl_si91x_opamp_set_configuration API.
 *
 * For more details on configuration parameters, see the respective peripheral example readme document.
 *
 * @section OPAMP_Usage Usage
 *
 * After defining the OPAMP configuration structures and passing an instance of @ref sl_opamp_config_t, the following functions can be used to initiate and configure the OPAMP features. The typical flow for implementation is as follows:
 * 1. Initialize the OPAMP driver: @ref sl_si91x_opamp_init
 * 2. Set desired GPIO pins for input and output: @ref sl_si91x_opamp_pin_init
 * 3. Set OPAMP instance and desired feature configuration: @ref sl_si91x_opamp_set_configuration
 * 4. Deinitialize the OPAMP driver: @ref sl_si91x_opamp_deinit
 *
 */
/** @} end group OPAMP */
#ifdef __cplusplus
}
#endif

#endif /* SL_SI91X_OPAMP_H */