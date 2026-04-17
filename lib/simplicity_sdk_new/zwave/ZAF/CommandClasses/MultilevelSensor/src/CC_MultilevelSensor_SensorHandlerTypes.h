/***************************************************************************//**
 * @file
 * @brief CC_MultilevelSensor_SensorHandlerTypes.h
 * @copyright 2020 Silicon Laboratories Inc.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef CC_MULTILEVELSENSOR_SENSORHANDLER_TYPES_H
#define CC_MULTILEVELSENSOR_SENSORHANDLER_TYPES_H
// -----------------------------------------------------------------------------
//                   Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "cc_multilevel_sensor_support_config.h"
#include "ZAF_types.h"

/**
 * @addtogroup CC
 * @{
 * @addtogroup MultilevelSensor
 * @{
 */

// -----------------------------------------------------------------------------
//                Macros and Typedefs
// -----------------------------------------------------------------------------
/**
 * Defined possible sensor scales
 */
#define SENSOR_SCALE_DEFAULT 0x00
#define SENSOR_SCALE_ABSOLUTE_HUMIDITY 0x01 /**< Absolute humidity */
#define SENSOR_SCALE_ACIDITY 0x00           /**< Acidity */
#define SENSOR_SCALE_AMPERE 0x00            /**< Ampere */
#define SENSOR_SCALE_AWEIGHTED_DECIBELS 0x01 /**< A-weighted decibels */
#define SENSOR_SCALE_BEATS_PER_MINUTE 0x00  /**< Beats per minute */
#define SENSOR_SCALE_BECQUEREL_PER_CUBIC_METER 0x00 /**< Becquerel per cubic meter */
#define SENSOR_SCALE_BMI_INDEX 0x00         /**< BMI Index */
#define SENSOR_SCALE_BODY_WAVE 0x03         /**< Body wave */
#define SENSOR_SCALE_BREATHS_PER_MINUTE 0x00 /**< Breaths per minute */
#define SENSOR_SCALE_BTU_H 0x01             /**< Btu/h */
#define SENSOR_SCALE_CELSIUS 0x00           /**< Celsius */
#define SENSOR_SCALE_CENTIMETER 0x01        /**< Centimeter */
#define SENSOR_SCALE_CUBIC_FEET_PER_MINUTE 0x01 /**< Cubic feet per minute */
#define SENSOR_SCALE_CUBIC_METER 0x01       /**< Cubic meter */
#define SENSOR_SCALE_CUBIC_METER_PER_HOUR 0x00  /**< Cubic meter per hour */
#define SENSOR_SCALE_DBM 0x01               /**< dBm */
#define SENSOR_SCALE_DECIBEL 0x00           /**< Decibel */
#define SENSOR_SCALE_DEGREES 0x00           /**< Degrees */
#define SENSOR_SCALE_DEGREES_RELATIVE_TO_NORTH_POLE 0x01 /**< Degrees relative to north pole */
#define SENSOR_SCALE_DIASTOLIC 0x01         /**< Diastolic */
#define SENSOR_SCALE_DIMENSIONLESS_VALUE 0x01 /**< Dimensionless value */
#define SENSOR_SCALE_EUROPEAN_MACROSEISMIC 0x01 /**< European Macroseismic */
#define SENSOR_SCALE_FAHRENHEIT 0x01        /**< Fahrenheit */
#define SENSOR_SCALE_FEET 0x01              /**< Feet */
#define SENSOR_SCALE_GALLONS 0x02           /**< Gallons */
#define SENSOR_SCALE_HERTZ 0x01             /**< Hertz */
#define SENSOR_SCALE_IMPEDANCE 0x02         /**< Impedance */
#define SENSOR_SCALE_INCHES_OF_MERCURY 0x01 /**< Inches of Mercury */
#define SENSOR_SCALE_INCHES_PER_HOUR 0x01   /**< Inches per hour */
#define SENSOR_SCALE_JOULE 0x00             /**< Joule */
#define SENSOR_SCALE_KILOGRAM 0x00          /**< Kilogram */
#define SENSOR_SCALE_KILOHERTZ 0x01         /**< Kilohertz */
#define SENSOR_SCALE_KILOPASCAL 0x00        /**< Kilopascal */
#define SENSOR_SCALE_LIEDU 0x02             /**< Liedu */
#define SENSOR_SCALE_LITER 0x00             /**< Liter */
#define SENSOR_SCALE_LITER_PER_HOUR 0x00    /**< Liter per hour */
#define SENSOR_SCALE_LOCAL 0x00             /**< Local */
#define SENSOR_SCALE_LUX 0x01               /**< Lux */
#define SENSOR_SCALE_MERCALLI 0x00          /**< Mercalli */
#define SENSOR_SCALE_METER 0x00             /**< Meter */
#define SENSOR_SCALE_METER_PER_SECOND 0x00  /**< Meter per second */
#define SENSOR_SCALE_METER_PER_SQUARE_SECOND 0x00 /**< Meter per square second */
#define SENSOR_SCALE_MICROGRAM_PER_CUBIC_METER 0x01 /**< Microgram per cubic meter */
#define SENSOR_SCALE_MICRO_GRAM_PER_CUBIC_METER 0x00 /**< Micro gram per cubic meter */
#define SENSOR_SCALE_MILES_PER_HOUR 0x01    /**< Miles per hour */
#define SENSOR_SCALE_MILLIAMPERE 0x01       /**< Milliampere */
#define SENSOR_SCALE_MILLIGRAM_PER_LITER 0x00 /**< Milligram per liter */
#define SENSOR_SCALE_MILLIMETER_PER_HOUR 0x00 /**< Millimeter per hour */
#define SENSOR_SCALE_MILLIVOLT 0x01         /**< Millivolt */
#define SENSOR_SCALE_MOLE_PER_CUBIC_METER 0x00 /**< Mole per cubic meter */
#define SENSOR_SCALE_MOMENT 0x01            /**< Moment */
#define SENSOR_SCALE_NEWTON 0x00            /**< Newton */
#define SENSOR_SCALE_OHM_METER 0x00         /**< Ohm meter */
#define SENSOR_SCALE_PARTS_PER_MILLION 0x00 /**< Parts per million */
#define SENSOR_SCALE_PERCENTAGE_VALUE 0x00  /**< Percentage value */
#define SENSOR_SCALE_PICOCURIES_PER_LITER 0x01 /**< Picocuries per liter */
#define SENSOR_SCALE_POUNDS 0x01            /**< Pounds */
#define SENSOR_SCALE_POUND_PER_SQUARE_INCH 0x01 /**< Pound per square inch */
#define SENSOR_SCALE_REVOLUTIONS_PER_MINUTE 0x00 /**< Revolutions per minute */
#define SENSOR_SCALE_RSSI 0x00              /**< RSSI */
#define SENSOR_SCALE_SECOND 0x00            /**< Second */
#define SENSOR_SCALE_SHINDO 0x03            /**< Shindo */
#define SENSOR_SCALE_SIEMENS_PER_METER 0x00 /**< Siemens per meter */
#define SENSOR_SCALE_SURFACE_WAVE 0x02      /**< Surface wave */
#define SENSOR_SCALE_SYSTOLIC 0x00          /**< Systolic */
#define SENSOR_SCALE_UNITLESS 0x00          /**< Unitless */
#define SENSOR_SCALE_UV_INDEX 0x00          /**< UV index */
#define SENSOR_SCALE_VOLT 0x00              /**< Volt */
#define SENSOR_SCALE_VOLUME_WATER_CONTENT 0x01 /**< Volume water content */
#define SENSOR_SCALE_WATER_ACTIVITY 0x03    /**< Water activity */
#define SENSOR_SCALE_WATT 0x00              /**< Watt */
#define SENSOR_SCALE_WATT_PER_SQUARE_METER 0x00 /**< Watt per square meter */

/**
 * Sensor scale aliases for backward compatibility
 */
#define SENSOR_SCALE_DIMENSIONLESS SENSOR_SCALE_DIMENSIONLESS_VALUE
#define SENSOR_SCALE_METER_SQUARE_SECOND SENSOR_SCALE_METER_PER_SQUARE_SECOND
#define SENSOR_SCALE_PERCENTAGE SENSOR_SCALE_PERCENTAGE_VALUE

/**
 * Defined possible sensor types
 */
typedef enum sensor_name {
  SENSOR_NAME_AIR_TEMPERATURE = 0x00,   ///< 0x01 - Air temperature
  SENSOR_NAME_GENERAL_PURPOSE,          ///< 0x02 - General purpose
  SENSOR_NAME_ILLUMINANCE,              ///< 0x03 - Illuminance
  SENSOR_NAME_POWER,                    ///< 0x04 - Power
  SENSOR_NAME_HUMIDITY,                 ///< 0x05 - Humidity
  SENSOR_NAME_VELOCITY,                 ///< 0x06 - Velocity
  SENSOR_NAME_DIRECTION,                ///< 0x07 - Direction
  SENSOR_NAME_ATMOSPHERIC_PRESSURE,     ///< 0x08 - Atmospheric pressure
  SENSOR_NAME_BAROMETRIC_PRESSURE,      ///< 0x09 - Barometric pressure
  SENSOR_NAME_SOLAR_RADIATION,          ///< 0x0A - Solar radiation
  SENSOR_NAME_DEW_POINT,                ///< 0x0B - Dew point
  SENSOR_NAME_RAIN_RATE,                ///< 0x0C - Rain rate
  SENSOR_NAME_TIDE_LEVEL,               ///< 0x0D - Tide level
  SENSOR_NAME_WEIGHT,                   ///< 0x0E - Weight
  SENSOR_NAME_VOLTAGE,                  ///< 0x0F - Voltage
  SENSOR_NAME_CURRENT,                  ///< 0x10 - Current
  SENSOR_NAME_CARBON_DIOXIDE_CO2_LEVEL, ///< 0x11 - Carbon dioxide (CO2-level)
  SENSOR_NAME_AIR_FLOW,                 ///< 0x12 - Air flow
  SENSOR_NAME_TANK_CAPACITY,            ///< 0x13 - Tank capacity
  SENSOR_NAME_DISTANCE,                 ///< 0x14 - Distance
  SENSOR_NAME_ANGLE_POSITION,           ///< 0x15 - Angle position
  SENSOR_NAME_ROTATION,                 ///< 0x16 - Rotation
  SENSOR_NAME_WATER_TEMPERATURE,        ///< 0x17 - Water temperature
  SENSOR_NAME_SOIL_TEMPERATURE,         ///< 0x18 - Soil temperature
  SENSOR_NAME_SEISMIC_INTENSITY,        ///< 0x19 - Seismic Intensity
  SENSOR_NAME_SEISMIC_MAGNITUDE,        ///< 0x1A - Seismic magnitude
  SENSOR_NAME_ULTRAVIOLET,              ///< 0x1B - Ultraviolet
  SENSOR_NAME_ELECTRICAL_RESISTIVITY,   ///< 0x1C - Electrical resistivity
  SENSOR_NAME_ELECTRICAL_CONDUCTIVITY,  ///< 0x1D - Electrical conductivity
  SENSOR_NAME_LOUDNESS,                 ///< 0x1E - Loudness
  SENSOR_NAME_MOISTURE,                 ///< 0x1F - Moisture
  SENSOR_NAME_FREQUENCY,                ///< 0x20 - Frequency
  SENSOR_NAME_TIME,                     ///< 0x21 - Time
  SENSOR_NAME_TARGET_TEMPERATURE,       ///< 0x22 - Target temperature
  SENSOR_NAME_PARTICULATE_MATTER_2_5,   ///< 0x23 - Particulate Matter 2.5
  SENSOR_NAME_FORMALDEHYDE_CH2O_LEVEL,  ///< 0x24 - Formaldehyde (CH2O-level)
  SENSOR_NAME_RADON_CONCENTRATION,      ///< 0x25 - Radon concentration
  SENSOR_NAME_METHANE_CH4_DENSITY,      ///< 0x26 - Methane (CH4) density
  SENSOR_NAME_VOLATILE_ORGANIC_COMPOUND_LEVEL, ///< 0x27 - Volatile Organic Compound level
  SENSOR_NAME_CARBON_MONOXIDE_CO_LEVEL, ///< 0x28 - Carbon monoxide (CO) level
  SENSOR_NAME_SOIL_HUMIDITY,            ///< 0x29 - Soil humidity
  SENSOR_NAME_SOIL_REACTIVITY,          ///< 0x2A - Soil reactivity
  SENSOR_NAME_SOIL_SALINITY,            ///< 0x2B - Soil salinity
  SENSOR_NAME_HEART_RATE,               ///< 0x2C - Heart rate
  SENSOR_NAME_BLOOD_PRESSURE,           ///< 0x2D - Blood pressure
  SENSOR_NAME_MUSCLE_MASS,              ///< 0x2E - Muscle mass
  SENSOR_NAME_FAT_MASS,                 ///< 0x2F - Fat mass
  SENSOR_NAME_BONE_MASS,                ///< 0x30 - Bone mass
  SENSOR_NAME_TOTAL_BODY_WATER_TBW,     ///< 0x31 - Total body water (TBW)
  SENSOR_NAME_BASIS_METABOLIC_RATE_BMR, ///< 0x32 - Basis metabolic rate (BMR)
  SENSOR_NAME_BODY_MASS_INDEX_BMI,      ///< 0x33 - Body Mass Index (BMI)
  SENSOR_NAME_ACCELERATION_X,           ///< 0x34 - Acceleration X-axis
  SENSOR_NAME_ACCELERATION_Y,           ///< 0x35 - Acceleration Y-axis
  SENSOR_NAME_ACCELERATION_Z,           ///< 0x36 - Acceleration Z-axis
  SENSOR_NAME_SMOKE_DENSITY,            ///< 0x37 - Smoke density
  SENSOR_NAME_WATER_FLOW,               ///< 0x38 - Water flow
  SENSOR_NAME_WATER_PRESSURE,           ///< 0x39 - Water pressure
  SENSOR_NAME_RF_SIGNAL_STRENGTH,       ///< 0x3A - RF signal strength
  SENSOR_NAME_PARTICULATE_MATTER_10,    ///< 0x3B - Particulate Matter 10
  SENSOR_NAME_RESPIRATORY_RATE,         ///< 0x3C - Respiratory rate
  SENSOR_NAME_RELATIVE_MODULATION_LEVEL, ///< 0x3D - Relative Modulation level
  SENSOR_NAME_BOILER_WATER_TEMPERATURE, ///< 0x3E - Boiler water temperature
  SENSOR_NAME_DOMESTIC_HOT_WATER_DHW_TEMPERATURE, ///< 0x3F - Domestic Hot Water (DHW) temperature
  SENSOR_NAME_OUTSIDE_TEMPERATURE,      ///< 0x40 - Outside temperature
  SENSOR_NAME_EXHAUST_TEMPERATURE,      ///< 0x41 - Exhaust temperature
  SENSOR_NAME_WATER_CHLORINE_LEVEL,     ///< 0x42 - Water Chlorine level
  SENSOR_NAME_WATER_ACIDITY,            ///< 0x43 - Water acidity
  SENSOR_NAME_WATER_OXIDATION_REDUCTION_POTENTIAL, ///< 0x44 - Water Oxidation reduction potential
  SENSOR_NAME_HEART_RATE_LF_HF_RATIO,   ///< 0x45 - Heart Rate LF/HF ratio
  SENSOR_NAME_MOTION_DIRECTION,         ///< 0x46 - Motion Direction
  SENSOR_NAME_APPLIED_FORCE_ON_THE_SENSOR, ///< 0x47 - Applied force on the sensor
  SENSOR_NAME_RETURN_AIR_TEMPERATURE,   ///< 0x48 - Return Air temperature
  SENSOR_NAME_SUPPLY_AIR_TEMPERATURE,   ///< 0x49 - Supply Air temperature
  SENSOR_NAME_CONDENSER_COIL_TEMPERATURE, ///< 0x4A - Condenser Coil temperature
  SENSOR_NAME_EVAPORATOR_COIL_TEMPERATURE, ///< 0x4B - Evaporator Coil temperature
  SENSOR_NAME_LIQUID_LINE_TEMPERATURE,  ///< 0x4C - Liquid Line temperature
  SENSOR_NAME_DISCHARGE_LINE_TEMPERATURE, ///< 0x4D - Discharge Line temperature
  SENSOR_NAME_SUCTION_INPUT_PUMP_COMPRESSOR_PRESSURE, ///< 0x4E - Suction (input pump/compressor) Pressure
  SENSOR_NAME_DISCHARGE_OUTPUT_PUMP_COMPRESSOR_PRESSURE, ///< 0x4F - Discharge (output pump/compressor) Pressure
  SENSOR_NAME_DEFROST_TEMPERATURE_DEFROST, ///< 0x50 - Defrost temperature (sensor used to decide when to defrost)
  SENSOR_NAME_OZONE_O3,                 ///< 0x51 - Ozone (O3)
  SENSOR_NAME_SULFUR_DIOXIDE_SO2,       ///< 0x52 - Sulfur dioxide (SO2)
  SENSOR_NAME_NITROGEN_DIOXIDE_NO2,     ///< 0x53 - Nitrogen dioxide (NO2)
  SENSOR_NAME_AMMONIA_NH3,              ///< 0x54 - Ammonia (NH3)
  SENSOR_NAME_LEAD_PB,                  ///< 0x55 - Lead (Pb)
  SENSOR_NAME_PARTICULATE_MATTER_1,     ///< 0x56 - Particulate Matter 1
  SENSOR_NAME_PERSON_COUNTER_ENTERING,  ///< 0x57 - Person counter (entering)
  SENSOR_NAME_PERSON_COUNTER_EXITING,   ///< 0x58 - Person counter (exiting)
  SENSOR_NAME_MAX_COUNT
} sensor_name_t;

/**
 * Structure that holds the attributes of a sensor type.
 */
typedef struct _sensor_type{
  uint8_t value;        ///< Sensor type id from SDS13812
  uint8_t byte_offset;    ///< Supported bitmask byte number from SDS13812
  uint8_t bit_mask;       ///< Supported bitmask bit number from SDS13812
  uint8_t max_scale_value;  ///< Maximum supported scale number from SDS13812
}sensor_type_t;

/**
 * Macros for sensor type definitions - allows inlining at call sites for better optimization
 * These expand to compound literals that can be used directly in assignments
 */
#define SENSOR_TYPE_AIR_TEMPERATURE .value = 0x01, .byte_offset = 1, .bit_mask = 0, .max_scale_value = 0x01
#define SENSOR_TYPE_GENERAL_PURPOSE .value = 0x02, .byte_offset = 1, .bit_mask = 1, .max_scale_value = 0x01
#define SENSOR_TYPE_ILLUMINANCE .value = 0x03, .byte_offset = 1, .bit_mask = 2, .max_scale_value = 0x01
#define SENSOR_TYPE_POWER .value = 0x04, .byte_offset = 1, .bit_mask = 3, .max_scale_value = 0x01
#define SENSOR_TYPE_HUMIDITY .value = 0x05, .byte_offset = 1, .bit_mask = 4, .max_scale_value = 0x01
#define SENSOR_TYPE_VELOCITY .value = 0x06, .byte_offset = 1, .bit_mask = 5, .max_scale_value = 0x01
#define SENSOR_TYPE_DIRECTION .value = 0x07, .byte_offset = 1, .bit_mask = 6, .max_scale_value = 0x00
#define SENSOR_TYPE_ATMOSPHERIC_PRESSURE .value = 0x08, .byte_offset = 1, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_BAROMETRIC_PRESSURE .value = 0x09, .byte_offset = 2, .bit_mask = 0, .max_scale_value = 0x01
#define SENSOR_TYPE_SOLAR_RADIATION .value = 0x0A, .byte_offset = 2, .bit_mask = 1, .max_scale_value = 0x00
#define SENSOR_TYPE_DEW_POINT .value = 0x0B, .byte_offset = 2, .bit_mask = 2, .max_scale_value = 0x01
#define SENSOR_TYPE_RAIN_RATE .value = 0x0C, .byte_offset = 2, .bit_mask = 3, .max_scale_value = 0x01
#define SENSOR_TYPE_TIDE_LEVEL .value = 0x0D, .byte_offset = 2, .bit_mask = 4, .max_scale_value = 0x01
#define SENSOR_TYPE_WEIGHT .value = 0x0E, .byte_offset = 2, .bit_mask = 5, .max_scale_value = 0x01
#define SENSOR_TYPE_VOLTAGE .value = 0x0F, .byte_offset = 2, .bit_mask = 6, .max_scale_value = 0x01
#define SENSOR_TYPE_CURRENT .value = 0x10, .byte_offset = 2, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_CARBON_DIOXIDE_CO2_LEVEL .value = 0x11, .byte_offset = 3, .bit_mask = 0, .max_scale_value = 0x00
#define SENSOR_TYPE_AIR_FLOW .value = 0x12, .byte_offset = 3, .bit_mask = 1, .max_scale_value = 0x01
#define SENSOR_TYPE_TANK_CAPACITY .value = 0x13, .byte_offset = 3, .bit_mask = 2, .max_scale_value = 0x02
#define SENSOR_TYPE_DISTANCE .value = 0x14, .byte_offset = 3, .bit_mask = 3, .max_scale_value = 0x02
#define SENSOR_TYPE_ANGLE_POSITION .value = 0x15, .byte_offset = 3, .bit_mask = 4, .max_scale_value = 0x02
#define SENSOR_TYPE_ROTATION .value = 0x16, .byte_offset = 3, .bit_mask = 5, .max_scale_value = 0x01
#define SENSOR_TYPE_WATER_TEMPERATURE .value = 0x17, .byte_offset = 3, .bit_mask = 6, .max_scale_value = 0x01
#define SENSOR_TYPE_SOIL_TEMPERATURE .value = 0x18, .byte_offset = 3, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_SEISMIC_INTENSITY .value = 0x19, .byte_offset = 4, .bit_mask = 0, .max_scale_value = 0x03
#define SENSOR_TYPE_SEISMIC_MAGNITUDE .value = 0x1A, .byte_offset = 4, .bit_mask = 1, .max_scale_value = 0x03
#define SENSOR_TYPE_ULTRAVIOLET .value = 0x1B, .byte_offset = 4, .bit_mask = 2, .max_scale_value = 0x00
#define SENSOR_TYPE_ELECTRICAL_RESISTIVITY .value = 0x1C, .byte_offset = 4, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_ELECTRICAL_CONDUCTIVITY .value = 0x1D, .byte_offset = 4, .bit_mask = 4, .max_scale_value = 0x00
#define SENSOR_TYPE_LOUDNESS .value = 0x1E, .byte_offset = 4, .bit_mask = 5, .max_scale_value = 0x01
#define SENSOR_TYPE_MOISTURE .value = 0x1F, .byte_offset = 4, .bit_mask = 6, .max_scale_value = 0x03
#define SENSOR_TYPE_FREQUENCY .value = 0x20, .byte_offset = 4, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_TIME .value = 0x21, .byte_offset = 5, .bit_mask = 0, .max_scale_value = 0x00
#define SENSOR_TYPE_TARGET_TEMPERATURE .value = 0x22, .byte_offset = 5, .bit_mask = 1, .max_scale_value = 0x01
#define SENSOR_TYPE_PARTICULATE_MATTER_2_5 .value = 0x23, .byte_offset = 5, .bit_mask = 2, .max_scale_value = 0x01
#define SENSOR_TYPE_FORMALDEHYDE_CH2O_LEVEL .value = 0x24, .byte_offset = 5, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_RADON_CONCENTRATION .value = 0x25, .byte_offset = 5, .bit_mask = 4, .max_scale_value = 0x01
#define SENSOR_TYPE_METHANE_CH4_DENSITY .value = 0x26, .byte_offset = 5, .bit_mask = 5, .max_scale_value = 0x00
#define SENSOR_TYPE_VOLATILE_ORGANIC_COMPOUND_LEVEL .value = 0x27, .byte_offset = 5, .bit_mask = 6, .max_scale_value = 0x01
#define SENSOR_TYPE_CARBON_MONOXIDE_CO_LEVEL .value = 0x28, .byte_offset = 5, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_SOIL_HUMIDITY .value = 0x29, .byte_offset = 6, .bit_mask = 0, .max_scale_value = 0x00
#define SENSOR_TYPE_SOIL_REACTIVITY .value = 0x2A, .byte_offset = 6, .bit_mask = 1, .max_scale_value = 0x00
#define SENSOR_TYPE_SOIL_SALINITY .value = 0x2B, .byte_offset = 6, .bit_mask = 2, .max_scale_value = 0x00
#define SENSOR_TYPE_HEART_RATE .value = 0x2C, .byte_offset = 6, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_BLOOD_PRESSURE .value = 0x2D, .byte_offset = 6, .bit_mask = 4, .max_scale_value = 0x01
#define SENSOR_TYPE_MUSCLE_MASS .value = 0x2E, .byte_offset = 6, .bit_mask = 5, .max_scale_value = 0x00
#define SENSOR_TYPE_FAT_MASS .value = 0x2F, .byte_offset = 6, .bit_mask = 6, .max_scale_value = 0x00
#define SENSOR_TYPE_BONE_MASS .value = 0x30, .byte_offset = 6, .bit_mask = 7, .max_scale_value = 0x00
#define SENSOR_TYPE_TOTAL_BODY_WATER_TBW .value = 0x31, .byte_offset = 7, .bit_mask = 0, .max_scale_value = 0x00
#define SENSOR_TYPE_BASIS_METABOLIC_RATE_BMR .value = 0x32, .byte_offset = 7, .bit_mask = 1, .max_scale_value = 0x00
#define SENSOR_TYPE_BODY_MASS_INDEX_BMI .value = 0x33, .byte_offset = 7, .bit_mask = 2, .max_scale_value = 0x00
#define SENSOR_TYPE_ACCELERATION_X .value = 0x34, .byte_offset = 7, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_ACCELERATION_Y .value = 0x35, .byte_offset = 7, .bit_mask = 4, .max_scale_value = 0x00
#define SENSOR_TYPE_ACCELERATION_Z .value = 0x36, .byte_offset = 7, .bit_mask = 5, .max_scale_value = 0x00
#define SENSOR_TYPE_SMOKE_DENSITY .value = 0x37, .byte_offset = 7, .bit_mask = 6, .max_scale_value = 0x00
#define SENSOR_TYPE_WATER_FLOW .value = 0x38, .byte_offset = 7, .bit_mask = 7, .max_scale_value = 0x00
#define SENSOR_TYPE_WATER_PRESSURE .value = 0x39, .byte_offset = 8, .bit_mask = 0, .max_scale_value = 0x00
#define SENSOR_TYPE_RF_SIGNAL_STRENGTH .value = 0x3A, .byte_offset = 8, .bit_mask = 1, .max_scale_value = 0x01
#define SENSOR_TYPE_PARTICULATE_MATTER_10 .value = 0x3B, .byte_offset = 8, .bit_mask = 2, .max_scale_value = 0x01
#define SENSOR_TYPE_RESPIRATORY_RATE .value = 0x3C, .byte_offset = 8, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_RELATIVE_MODULATION_LEVEL .value = 0x3D, .byte_offset = 8, .bit_mask = 4, .max_scale_value = 0x00
#define SENSOR_TYPE_BOILER_WATER_TEMPERATURE .value = 0x3E, .byte_offset = 8, .bit_mask = 5, .max_scale_value = 0x01
#define SENSOR_TYPE_DOMESTIC_HOT_WATER_DHW_TEMPERATURE .value = 0x3F, .byte_offset = 8, .bit_mask = 6, .max_scale_value = 0x01
#define SENSOR_TYPE_OUTSIDE_TEMPERATURE .value = 0x40, .byte_offset = 8, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_EXHAUST_TEMPERATURE .value = 0x41, .byte_offset = 9, .bit_mask = 0, .max_scale_value = 0x01
#define SENSOR_TYPE_WATER_CHLORINE_LEVEL .value = 0x42, .byte_offset = 9, .bit_mask = 1, .max_scale_value = 0x00
#define SENSOR_TYPE_WATER_ACIDITY .value = 0x43, .byte_offset = 9, .bit_mask = 2, .max_scale_value = 0x00
#define SENSOR_TYPE_WATER_OXIDATION_REDUCTION_POTENTIAL .value = 0x44, .byte_offset = 9, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_HEART_RATE_LF_HF_RATIO .value = 0x45, .byte_offset = 9, .bit_mask = 4, .max_scale_value = 0x00
#define SENSOR_TYPE_MOTION_DIRECTION .value = 0x46, .byte_offset = 9, .bit_mask = 5, .max_scale_value = 0x00
#define SENSOR_TYPE_APPLIED_FORCE_ON_THE_SENSOR .value = 0x47, .byte_offset = 9, .bit_mask = 6, .max_scale_value = 0x00
#define SENSOR_TYPE_RETURN_AIR_TEMPERATURE .value = 0x48, .byte_offset = 9, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_SUPPLY_AIR_TEMPERATURE .value = 0x49, .byte_offset = 10, .bit_mask = 0, .max_scale_value = 0x01
#define SENSOR_TYPE_CONDENSER_COIL_TEMPERATURE .value = 0x4A, .byte_offset = 10, .bit_mask = 1, .max_scale_value = 0x01
#define SENSOR_TYPE_EVAPORATOR_COIL_TEMPERATURE .value = 0x4B, .byte_offset = 10, .bit_mask = 2, .max_scale_value = 0x01
#define SENSOR_TYPE_LIQUID_LINE_TEMPERATURE .value = 0x4C, .byte_offset = 10, .bit_mask = 3, .max_scale_value = 0x01
#define SENSOR_TYPE_DISCHARGE_LINE_TEMPERATURE .value = 0x4D, .byte_offset = 10, .bit_mask = 4, .max_scale_value = 0x01
#define SENSOR_TYPE_SUCTION_INPUT_PUMP_COMPRESSOR_PRESSURE .value = 0x4E, .byte_offset = 10, .bit_mask = 5, .max_scale_value = 0x01
#define SENSOR_TYPE_DISCHARGE_OUTPUT_PUMP_COMPRESSOR_PRESSURE .value = 0x4F, .byte_offset = 10, .bit_mask = 6, .max_scale_value = 0x01
#define SENSOR_TYPE_DEFROST_TEMPERATURE_DEFROST .value = 0x50, .byte_offset = 10, .bit_mask = 7, .max_scale_value = 0x01
#define SENSOR_TYPE_OZONE_O3 .value = 0x51, .byte_offset = 11, .bit_mask = 0, .max_scale_value = 0x00
#define SENSOR_TYPE_SULFUR_DIOXIDE_SO2 .value = 0x52, .byte_offset = 11, .bit_mask = 1, .max_scale_value = 0x00
#define SENSOR_TYPE_NITROGEN_DIOXIDE_NO2 .value = 0x53, .byte_offset = 11, .bit_mask = 2, .max_scale_value = 0x00
#define SENSOR_TYPE_AMMONIA_NH3 .value = 0x54, .byte_offset = 11, .bit_mask = 3, .max_scale_value = 0x00
#define SENSOR_TYPE_LEAD_PB .value = 0x55, .byte_offset = 11, .bit_mask = 4, .max_scale_value = 0x00
#define SENSOR_TYPE_PARTICULATE_MATTER_1 .value = 0x56, .byte_offset = 11, .bit_mask = 5, .max_scale_value = 0x00
#define SENSOR_TYPE_PERSON_COUNTER_ENTERING .value = 0x57, .byte_offset = 11, .bit_mask = 6, .max_scale_value = 0x00
#define SENSOR_TYPE_PERSON_COUNTER_EXITING .value = 0x58, .byte_offset = 11, .bit_mask = 7, .max_scale_value = 0x00

/**
 * Helper macro to generate a single case in the sensor type lookup
 * Sensor types that are accessed by the application code are allocated statically.
 * Unused sensor types are optimized out at compile time.
 */
#define SENSOR_TYPE_CASE(i_name, name) \
  (i_name) == SENSOR_NAME_##name       \
  ?({ static const sensor_type_t _st_##name = (const sensor_type_t){ SENSOR_TYPE_##name }; &_st_##name; }) :

/**
 * Macro to map sensor_name_t enum to corresponding sensor type
 */
#define SENSOR_TYPE_FROM_NAME(sensor_name)                                   \
  ((SENSOR_TYPE_CASE(sensor_name, AIR_TEMPERATURE)                           \
    SENSOR_TYPE_CASE(sensor_name, GENERAL_PURPOSE)                           \
    SENSOR_TYPE_CASE(sensor_name, ILLUMINANCE)                               \
    SENSOR_TYPE_CASE(sensor_name, POWER)                                     \
    SENSOR_TYPE_CASE(sensor_name, HUMIDITY)                                  \
    SENSOR_TYPE_CASE(sensor_name, VELOCITY)                                  \
    SENSOR_TYPE_CASE(sensor_name, DIRECTION)                                 \
    SENSOR_TYPE_CASE(sensor_name, ATMOSPHERIC_PRESSURE)                      \
    SENSOR_TYPE_CASE(sensor_name, BAROMETRIC_PRESSURE)                       \
    SENSOR_TYPE_CASE(sensor_name, SOLAR_RADIATION)                           \
    SENSOR_TYPE_CASE(sensor_name, DEW_POINT)                                 \
    SENSOR_TYPE_CASE(sensor_name, RAIN_RATE)                                 \
    SENSOR_TYPE_CASE(sensor_name, TIDE_LEVEL)                                \
    SENSOR_TYPE_CASE(sensor_name, WEIGHT)                                    \
    SENSOR_TYPE_CASE(sensor_name, VOLTAGE)                                   \
    SENSOR_TYPE_CASE(sensor_name, CURRENT)                                   \
    SENSOR_TYPE_CASE(sensor_name, CARBON_DIOXIDE_CO2_LEVEL)                  \
    SENSOR_TYPE_CASE(sensor_name, AIR_FLOW)                                  \
    SENSOR_TYPE_CASE(sensor_name, TANK_CAPACITY)                             \
    SENSOR_TYPE_CASE(sensor_name, DISTANCE)                                  \
    SENSOR_TYPE_CASE(sensor_name, ANGLE_POSITION)                            \
    SENSOR_TYPE_CASE(sensor_name, ROTATION)                                  \
    SENSOR_TYPE_CASE(sensor_name, WATER_TEMPERATURE)                         \
    SENSOR_TYPE_CASE(sensor_name, SOIL_TEMPERATURE)                          \
    SENSOR_TYPE_CASE(sensor_name, SEISMIC_INTENSITY)                         \
    SENSOR_TYPE_CASE(sensor_name, SEISMIC_MAGNITUDE)                         \
    SENSOR_TYPE_CASE(sensor_name, ULTRAVIOLET)                               \
    SENSOR_TYPE_CASE(sensor_name, ELECTRICAL_RESISTIVITY)                    \
    SENSOR_TYPE_CASE(sensor_name, ELECTRICAL_CONDUCTIVITY)                   \
    SENSOR_TYPE_CASE(sensor_name, LOUDNESS)                                  \
    SENSOR_TYPE_CASE(sensor_name, MOISTURE)                                  \
    SENSOR_TYPE_CASE(sensor_name, FREQUENCY)                                 \
    SENSOR_TYPE_CASE(sensor_name, TIME)                                      \
    SENSOR_TYPE_CASE(sensor_name, TARGET_TEMPERATURE)                        \
    SENSOR_TYPE_CASE(sensor_name, PARTICULATE_MATTER_2_5)                    \
    SENSOR_TYPE_CASE(sensor_name, FORMALDEHYDE_CH2O_LEVEL)                   \
    SENSOR_TYPE_CASE(sensor_name, RADON_CONCENTRATION)                       \
    SENSOR_TYPE_CASE(sensor_name, METHANE_CH4_DENSITY)                       \
    SENSOR_TYPE_CASE(sensor_name, VOLATILE_ORGANIC_COMPOUND_LEVEL)           \
    SENSOR_TYPE_CASE(sensor_name, CARBON_MONOXIDE_CO_LEVEL)                  \
    SENSOR_TYPE_CASE(sensor_name, SOIL_HUMIDITY)                             \
    SENSOR_TYPE_CASE(sensor_name, SOIL_REACTIVITY)                           \
    SENSOR_TYPE_CASE(sensor_name, SOIL_SALINITY)                             \
    SENSOR_TYPE_CASE(sensor_name, HEART_RATE)                                \
    SENSOR_TYPE_CASE(sensor_name, BLOOD_PRESSURE)                            \
    SENSOR_TYPE_CASE(sensor_name, MUSCLE_MASS)                               \
    SENSOR_TYPE_CASE(sensor_name, FAT_MASS)                                  \
    SENSOR_TYPE_CASE(sensor_name, BONE_MASS)                                 \
    SENSOR_TYPE_CASE(sensor_name, TOTAL_BODY_WATER_TBW)                      \
    SENSOR_TYPE_CASE(sensor_name, BASIS_METABOLIC_RATE_BMR)                  \
    SENSOR_TYPE_CASE(sensor_name, BODY_MASS_INDEX_BMI)                       \
    SENSOR_TYPE_CASE(sensor_name, ACCELERATION_X)                            \
    SENSOR_TYPE_CASE(sensor_name, ACCELERATION_Y)                            \
    SENSOR_TYPE_CASE(sensor_name, ACCELERATION_Z)                            \
    SENSOR_TYPE_CASE(sensor_name, SMOKE_DENSITY)                             \
    SENSOR_TYPE_CASE(sensor_name, WATER_FLOW)                                \
    SENSOR_TYPE_CASE(sensor_name, WATER_PRESSURE)                            \
    SENSOR_TYPE_CASE(sensor_name, RF_SIGNAL_STRENGTH)                        \
    SENSOR_TYPE_CASE(sensor_name, PARTICULATE_MATTER_10)                     \
    SENSOR_TYPE_CASE(sensor_name, RESPIRATORY_RATE)                          \
    SENSOR_TYPE_CASE(sensor_name, RELATIVE_MODULATION_LEVEL)                 \
    SENSOR_TYPE_CASE(sensor_name, BOILER_WATER_TEMPERATURE)                  \
    SENSOR_TYPE_CASE(sensor_name, DOMESTIC_HOT_WATER_DHW_TEMPERATURE)        \
    SENSOR_TYPE_CASE(sensor_name, OUTSIDE_TEMPERATURE)                       \
    SENSOR_TYPE_CASE(sensor_name, EXHAUST_TEMPERATURE)                       \
    SENSOR_TYPE_CASE(sensor_name, WATER_CHLORINE_LEVEL)                      \
    SENSOR_TYPE_CASE(sensor_name, WATER_ACIDITY)                             \
    SENSOR_TYPE_CASE(sensor_name, WATER_OXIDATION_REDUCTION_POTENTIAL)       \
    SENSOR_TYPE_CASE(sensor_name, HEART_RATE_LF_HF_RATIO)                    \
    SENSOR_TYPE_CASE(sensor_name, MOTION_DIRECTION)                          \
    SENSOR_TYPE_CASE(sensor_name, APPLIED_FORCE_ON_THE_SENSOR)               \
    SENSOR_TYPE_CASE(sensor_name, RETURN_AIR_TEMPERATURE)                    \
    SENSOR_TYPE_CASE(sensor_name, SUPPLY_AIR_TEMPERATURE)                    \
    SENSOR_TYPE_CASE(sensor_name, CONDENSER_COIL_TEMPERATURE)                \
    SENSOR_TYPE_CASE(sensor_name, EVAPORATOR_COIL_TEMPERATURE)               \
    SENSOR_TYPE_CASE(sensor_name, LIQUID_LINE_TEMPERATURE)                   \
    SENSOR_TYPE_CASE(sensor_name, DISCHARGE_LINE_TEMPERATURE)                \
    SENSOR_TYPE_CASE(sensor_name, SUCTION_INPUT_PUMP_COMPRESSOR_PRESSURE)    \
    SENSOR_TYPE_CASE(sensor_name, DISCHARGE_OUTPUT_PUMP_COMPRESSOR_PRESSURE) \
    SENSOR_TYPE_CASE(sensor_name, DEFROST_TEMPERATURE_DEFROST)               \
    SENSOR_TYPE_CASE(sensor_name, OZONE_O3)                                  \
    SENSOR_TYPE_CASE(sensor_name, SULFUR_DIOXIDE_SO2)                        \
    SENSOR_TYPE_CASE(sensor_name, NITROGEN_DIOXIDE_NO2)                      \
    SENSOR_TYPE_CASE(sensor_name, AMMONIA_NH3)                               \
    SENSOR_TYPE_CASE(sensor_name, LEAD_PB)                                   \
    SENSOR_TYPE_CASE(sensor_name, PARTICULATE_MATTER_1)                      \
    SENSOR_TYPE_CASE(sensor_name, PERSON_COUNTER_ENTERING)                   \
    SENSOR_TYPE_CASE(sensor_name, PERSON_COUNTER_EXITING)                    \
    (const sensor_type_t*)NULL))

/**
 * Defined possible sensor read size
 */
typedef enum {
  SENSOR_READ_RESULT_SIZE_1 = 1,
  SENSOR_READ_RESULT_SIZE_2 = 2,
  SENSOR_READ_RESULT_SIZE_4 = 4,
  SENSOR_READ_RESULT_SIZE_END = 5
}sensor_read_result_size_t;

/**
 * Defined possible sensor read precision
 */
typedef enum {
  SENSOR_READ_RESULT_PRECISION_1 = 1,
  SENSOR_READ_RESULT_PRECISION_2,
  SENSOR_READ_RESULT_PRECISION_3,
  SENSOR_READ_RESULT_PRECISION_END
}sensor_read_result_precision;

/**
 * Structure that holds a read result from the read interface.
 */
#define SLI_MAX_RAW_RESULT_BYTES 4
typedef struct _sensor_read_result {
  uint8_t raw_result[SLI_MAX_RAW_RESULT_BYTES];   ///< The raw buffer which holds the result.
  sensor_read_result_precision precision;     ///< The precision which the raw result should be interpret with
  sensor_read_result_size_t size_bytes;     ///< The size which the raw result should be interpret with
}sensor_read_result_t;

/**
 * Structure that holds a read result from the read interface.
 */
typedef struct _sensor_interface {
  RECEIVE_OPTIONS_TYPE_EX rxOpt;            ///< Contains information required for Supervision and
                                            ///< True Status. Must be the first element in this
                                            ///< struct because TSE assumes this location.
  uint8_t endpoint;                         ///< The sensor must be tied to an endpoint. Must be set
                                            ///< to 0 if no endpoints. The endpoint value MUST be
                                            ///< located as the second element in this struct as ZAF
                                            ///< depends on that for generation of the Node
                                            ///< Information Frame.
  const sensor_type_t* sensor_type;         ///< Reference of a sensor type structure
  uint8_t supported_scale;                  ///< Each bit represents a supported scale
  bool (*init)(void);                           ///< Function pointer to initialize a sensor
  bool (*deinit)(void);                         ///< Function pointer to deinitialize a sensor
  bool (*read_value)(sensor_read_result_t* o_result, uint8_t i_scale); ///< Function pointer to read a sensor value
}sensor_interface_t, sensor_interface_iterator_t;

/**
 * Defined sensor interface return values
 */
typedef enum {
  SENSOR_INTERFACE_RETURN_VALUE_OK,
  SENSOR_INTERFACE_RETURN_VALUE_ALREADY_SET,
  SENSOR_INTERFACE_RETURN_VALUE_INVALID_SCALE_VALUE,
  SENSOR_INTERFACE_RETURN_VALUE_ERROR,
  SENSOR_INTERFACE_RETURN_VALUE_END
}sensor_interface_return_value_t;

/**
 * Defined Multilevel Sensor return values
 */
typedef enum {
  CC_MULTILEVEL_SENSOR_RETURN_VALUE_OK,
  CC_MULTILEVEL_SENSOR_RETURN_VALUE_ERROR,
  CC_MULTILEVEL_SENSOR_RETURN_VALUE_NOT_FOUND,
  CC_MULTILEVEL_SENSOR_RETURN_VALUE_ALREADY_REGISTRATED,
  CC_MULTILEVEL_SENSOR_RETURN_VALUE_REGISTRATION_LIMIT_REACHED,
  CC_MULTILEVEL_SENSOR_RETURN_VALUE_END
}cc_multilevel_sensor_return_value;

/**
 * Structure that administrate all of the registered sensors.
 */

typedef struct _sensor_administration {
  sensor_interface_t* registrated_sensors[MULTILEVEL_SENSOR_REGISTERED_SENSOR_NUMBER_LIMIT];  ///< Reference of the sensor interfaces
  uint8_t number_of_registrated_sensors;                  ///< Stores how many sensors are registered
}sensor_administration_t;

// -----------------------------------------------------------------------------
//                Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//              Public Function Declarations
// -----------------------------------------------------------------------------
/**
 * Returns a sensor_type_t struct from the sensor_types config table
 * @param[in] i_sensor_name The name of the sensor type, the sensor type attributes'
 * reference will be look for based on this value
 *
 * @return sensor_type_t struct which includes the type attributes.
 */
const sensor_type_t*
cc_multilevel_sensor_get_sensor_type(sensor_name_t i_sensor_name);

/**
 * Initialize a sensor interface instance. Sets the whole struct to zero and sets
 * the reference to the appropiate sensor_type_t struct.
 * @param[in] i_instance Pointer to an existing sensor interface to init
 * @param[in] i_name The name of the sensor type, the sensor type attributes'
 * reference will be look for based on this value
 *
 * @return Status of the initialization.
 */
sensor_interface_return_value_t
cc_multilevel_sensor_init_interface(sensor_interface_t* i_instance, sensor_name_t i_name);

/**
 * Adds a new scale to an existing interface. Supported scales must be set by this function during initialization.
 * @param[in] i_instance Pointer to an existing sensor interface which the new scale will be registered to
 * @param[in] i_scale The new scale which will be registered
 *
 * @return Status of the new scale value registration.
 */
sensor_interface_return_value_t
cc_multilevel_sensor_add_supported_scale_interface(sensor_interface_t* i_instance, uint8_t i_scale);

/**
 * @}
 * @}
 */

#endif // CC_MULTILEVELSENSOR_SENSORHANDLER_TYPES_H
