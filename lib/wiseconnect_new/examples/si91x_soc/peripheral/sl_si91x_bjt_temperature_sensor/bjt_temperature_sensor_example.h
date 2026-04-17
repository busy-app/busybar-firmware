/***************************************************************************/ /**
* @file bjt_temperature_Sensor_example.h
* @brief BJT Temperature Sensor example - Die temperature measurement using internal BJT circuit
*******************************************************************************
* # License
* <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* The licensor of this software is Silicon Laboratories Inc. Your use of this
* software is governed by the terms of Silicon Labs Master Software License
* Agreement (MSLA) available at
* www.silabs.com/about-us/legal/master-software-license-agreement. This
* software is distributed to you in Source Code format and is governed by the
* sections of the MSLA applicable to Source Code.
*
******************************************************************************/

#ifndef BJT_TEMPERATURE_SENSOR_EXAMPLE_H_
#define BJT_TEMPERATURE_SENSOR_EXAMPLE_H_

/**
 * @brief BJT Temperature Sensor Example API
 * 
 * This example demonstrates die temperature measurement using the SI91x internal
 * BJT temperature sensor. Unlike dedicated digital temperature sensors, this
 * implementation uses a BJT circuit that outputs a temperature-dependent voltage
 * which must be measured and converted to temperature.
 * 
 * @section BJT_TEMP_SYSTEM System Overview
 * 
 * The temperature measurement requires coordination of multiple subsystems:
 * - **BJT Circuit**: Provides temperature-dependent voltage (~2mV/°C change)
 * - **ADC Channel 0**: Measures both BJT output and band-gap reference
 * - **OPAMP1**: Unity gain buffer for signal conditioning
 * - **eFuse Calibration**: Factory values for measurement accuracy
 * 
 * @section BJT_TEMP_RESOURCES Resource Usage
 * 
 * **During temperature measurement, these resources are temporarily unavailable:**
 * - ADC Channel 0 (cannot be used for other measurements)
 * - OPAMP1 (cannot be used for other signal conditioning)
 * 
 * **Resources NOT affected:**
 * - ADC Channels 1-15 (remain available)
 * - OPAMP2 and OPAMP3 (remain available)
 * 
 * @section BJT_TEMP_CONFIG Fixed Configuration Requirements
 * 
 * **Cannot be changed (hardware requirements):**
 * - Must use ADC Channel 0 (BJT sensor hardwired to this channel)
 * - Must use OPAMP1 (dedicated connections for temperature measurement)
 * - ADC input 20 for band-gap reference measurement
 * - ADC input 23 for BJT temperature sensor output
 * - Unity gain (required for calibration accuracy)
 */

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
/***************************************************************************/ /**
* @brief Initialize the BJT temperature sensor system.
*
* @details This function configures all subsystems required for temperature measurement:
* 
* **SUBSYSTEMS CONFIGURED:**
* - ADC Channel 0: Set up for 12-bit single-ended measurements
* - OPAMP1: Configured as a unity gain buffer for signal conditioning
* - Internal routing: Sets up ADC input switching between band-gap (20) and BJT (23)
* - eFuse interface: Reads factory calibration values for accuracy
* 
* **CALIBRATION VALUES RETRIEVED:**
* - ADC offset correction (compensates for ADC measurement errors)
* - Band-gap reference voltage (precise reference for each chip)
* - Temperature sensor offset voltage (BJT-specific calibration)
* 
* **SYSTEM IMPACT:**
* - ADC Channel 0 becomes dedicated to temperature measurement.
* - OPAMP1 becomes dedicated to temperature sensor buffering.
* - Other ADC channels and OPAMPs remain available.
* - Brief system clock change during eFuse access (automatically handled).
*
* @pre None required - this is the initialization function
* @post System ready for temperature measurements via bjt_temperature_sensor_example_process_action()
* 
* @param none
* @return none
* 
* @note Must be called before any temperature readings. Clock frequency may 
*       briefly change during initialization due to eFuse access - this is normal.
******************************************************************************/
void bjt_temperature_sensor_example_init(void);

/***************************************************************************/ /**
* @brief Perform temperature measurement and display results
*
* @details Executes complete temperature measurement cycle:
* 
* **MEASUREMENT SEQUENCE (all handled internally):**
* 1. Switch ADC to input 20 → Measure band-gap reference via OPAMP1
* 2. Switch ADC to input 23 → Measure BJT temperature output  
* 3. Apply factory calibration using eFuse values
* 4. Calculate temperature: Temp(°C) = -273 + 310 * (calibrated_ratio + offset)
* 5. Return calibrated temperature in Celsius
* 
* **WHY BOTH MEASUREMENTS ARE NEEDED:**
* - Band-gap reference: Provides stable voltage reference (~1.2 V, temperature-independent)
* - BJT output: Provides temperature-dependent voltage (~2mV/°C change)
* - Ratio calculation: Eliminates ADC gain variations and supply voltage effects
* 
* **RESOURCE USAGE DURING CALL:**
* - ADC Channel 0: Temporarily occupied for measurements
* - OPAMP1: Temporarily configured for voltage buffering
* - Duration: ~1ms for complete measurement cycle
* 
* **EXPECTED TEMPERATURE RANGE:** -40°C to +125°C
* 
* @pre bjt_temperature_sensor_example_init() must have been called successfully.
* @post Temperature reading displayed on debug console
* 
* @param none  
* @return none 
* 
* @note Call this function periodically for continuous temperature monitoring.
*       Each call performs a fresh measurement with current calibration.
******************************************************************************/
void bjt_temperature_sensor_example_process_action(void);

#endif /* BJT_TEMPERATURE_SENSOR_EXAMPLE_H_ */
