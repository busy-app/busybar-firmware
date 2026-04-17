/*******************************************************************************
* @file  doxygen_template.h
* @brief 
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

/* ------------------------------------------------------------------------
 * The following APIs appear in the docs.silabs.com menu in the order shown
 * ------------------------------------------------------------------------
 */

/*! @defgroup SL_SI91X APIs
 *
 *   This section provides a reference to the Si91x Device Management API including functions, data types, and constants.
 *
 *  @defgroup SL_SI91X_API Functions
 *  @ingroup SL_SI91X
 *
 *   This section provides a reference to Si91x Device Management functions:
 *
 *  - @ref SI91X_DRIVER_FUNCTIONS
 *    functions to initialize and configure the device.
 * 
 *  - @ref SI91X_RADIO_FUNCTIONS
 *    functions to access radio features such as calibration, low-power settings, and others.
 * 
 *  - @ref SI91X_FIRMWARE_UPDATE_FUNCTIONS
 *    functions to perform firmware updates on the device.
 * 
 *  - @ref SI91X_NETWORK_FUNCTIONS
 *    Functions to perform network configurations on device
 *
 */

/*! @defgroup SI91X_DRIVER_FUNCTIONS Core
 *  @ingroup SL_SI91X_API
 *
 */

/*! @defgroup SI91X_RADIO_FUNCTIONS Radio
 *  @ingroup SL_SI91X_API
 *
 */

/*! @defgroup SI91X_FIRMWARE_UPDATE_FUNCTIONS Firmware Update
 *  @ingroup SL_SI91X_API
 *	
 *   Firmware update can be handled on Si91x Chipsets in following ways
 * 
 * - @ref SI91X_FIRMWARE_UPDATE_FROM_HOST_FUNCTIONS
 *   functions to perform firmware update from the host.
 * 
 * - @ref SI91X_FIRMWARE_UPDATE_FROM_MODULE_FUNCTIONS
 *   functions to perform firmware update from the module.
 *
 */

/*! @defgroup SI91X_FIRMWARE_UPDATE_FROM_HOST_FUNCTIONS Firmware Update From Host
 *  @ingroup SI91X_FIRMWARE_UPDATE_FUNCTIONS
 *
 *   This section provides a reference to the APIs used to perform firmware updates from the host.
 *
 */

/*! @defgroup SI91X_FIRMWARE_UPDATE_FROM_MODULE_FUNCTIONS Firmware Update from Module
 *  @ingroup SI91X_FIRMWARE_UPDATE_FUNCTIONS
 *
 *   This section provides a reference to the APIs used to perform firmware updates from the module.
 *
 */

/*! @defgroup SI91X_NETWORK_FUNCTIONS Network Configuration
 *  @ingroup SL_SI91X_API
 *
 */

/*! @defgroup SL_SI91X_TYPES Types
 *  @ingroup SL_SI91X
 *	
 *   This section provides a reference to Si91x Device Management data types.
 *	
 */

/*! @defgroup SL_SI91X_CONSTANTS Constants
 *  @ingroup SL_SI91X
 *	
 *   This section provides a reference to Si91x Device Management constants.
 *	
 */

/*! @defgroup SL_SI91X_BOOT_CONFIGURATION Boot Configuration Feature Bitmaps
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to Si91x boot configuration feature bitmaps parameters.
 */

/*! @defgroup SI91X_TCP_IP_FEATURE_BITMAP TCP/IP Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the Transport Control Protocol and Internet Protocol (TCP/IP) feature bitmap in the boot configuration feature bitmaps.
 */

/*! @defgroup SI91X_EXTENDED_TCP_IP_FEATURE_BITMAP Extended TCP/IP Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the Transport Control Protocol and Internet Protocol (TCP/IP) extended feature bitmap in the boot configuration feature bitmaps.
 */

/*! @defgroup SI91X_BLE_FEATURE_BITMAP BLE Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the Bluetooth Low Energy (BLE) feature bitmap in the boot configuration feature bitmaps.
 * 
 *   This bitmap is valid only when BIT(31) is set to 1 in custom_feature_bitmap, ext_custom_feature_bit_map, and bt_feature_bit_map.
 */

/*! @defgroup SI91X_EXTENDED_BLE_CUSTOM_FEATURE_BITMAP Extended BLE Custom Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the Bluetooth Low Energy (BLE) extended custom feature bitmap in the boot configuration feature bitmaps.
 * 
 *   This bitmap is valid only when BIT(31) is set to 1 in ble_feature_bit_map, custom_feature_bitmap, ext_custom_feature_bit_map, and bt_feature_bit_map.
 */

/*! @defgroup SI91X_BT_FEATURE_BITMAP Bluetooth Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the Bluetooth feature bitmap in the boot configuration feature bitmaps.
 * 
 *   This bitmap is valid only when BIT(31) is set to 1 in both custom_feature_bit_map and ext_custom_feature_bit_map.
 */

/*! @defgroup SI91X_FEATURE_BITMAP Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the SiWx91x feature bitmap in the boot configuration feature bitmaps.
 */

/*! @defgroup SI91X_CALIBRATION_FLAGS Calibration Flags
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the calibration flags in the boot configuration feature bitmaps.
 */

/*! @defgroup SI91X_BURN_TARGET_OPTIONS Burn Target Options
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the SiWx91x burn target options in the boot configuration feature bitmaps.
 */

/*! @defgroup SI91X_CONFIG_FEATURE_BITMAP Configuration Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the configuration feature bitmap in the boot configuration feature bitmaps.
 * 
 *   This bitmap is valid only when BIT(31) is set to 1 in both ext_tcp_ip_feature_bit_map and tcp_ip_feature_bit_map.
 */

/*! @defgroup SI91X_CUSTOM_FEATURE_BITMAP Custom Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the custom feature bitmap in the boot configuration feature bitmaps.
 */

/*! @defgroup SI91X_EXTENDED_CUSTOM_FEATURE_BITMAP Extended Custom Feature Bitmap
 *  @ingroup SL_SI91X_BOOT_CONFIGURATION
 *	
 *   This section provides a reference to the extended custom feature bitmap in the boot configuration feature bitmaps.
 * 
 * The following defines should be used based on the mode of operation.
 *
 * | Define                                        | SoC | NCP |
 * |-----------------------------------------------|-----|-----|
 * | SL_SI91X_EXT_FEAT_352K_M4SS_320K **or** SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV      |  X  |     |
 * | SL_SI91X_EXT_FEAT_416K_M4SS_256K **or** SL_SI91X_RAM_LEVEL_NWP_MEDIUM_MCU_MEDIUM  |  X  |     |
 * | SL_SI91X_EXT_FEAT_480K_M4SS_192K **or** SL_SI91X_RAM_LEVEL_NWP_ADV_MCU_BASIC      |  X  |     |
 * | SL_SI91X_EXT_FEAT_352K **or** SL_SI91X_RAM_LEVEL_NWP_BASIC                        |     |  X  |
 * | SL_SI91X_EXT_FEAT_672K **or** SL_SI91X_RAM_LEVEL_NWP_ALL_AVAILABLE               |     |  X  |
 * 
 * > **Note:** 
 * > X indicates supported mode of operation.
 * 
 * > **Note**
 * > The following defines for NCP mode will be deprecated in future releases.
 * > 
 * > | Define                                        | SoC | NCP |
 * > |-----------------------------------------------|-----|-----|
 * > | SL_SI91X_EXT_FEAT_352K_M4SS_320K **or** SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV    |     |  X  |
 * > | SL_SI91X_EXT_FEAT_672K_M4SS_0K **or** SL_SI91X_RAM_LEVEL_NWP_ALL_MCU_ZERO       |     |  X  |
 */

/*! @defgroup SL_SI91X_DEFAULT_DEVICE_CONFIGURATION Default Device Configuration
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to Si91x default device configuration.
 */

/*! @defgroup SI91X_LOAD_IMAGE_TYPES Load Image Types
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to Si91x load image types.
 */

/*! @defgroup SI91X_NWP_FW_IMAGE_NUMBERS NWP Firmware Image Numbers
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to Si91x NWP firmware image numbers
 */

/*! @defgroup SI91X_TLS_DEFINE TLS Flags
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to Si91x flags for use in the Transport Layer Security (TLS) protocol functionality.
 */

/*! @defgroup SL_FWUP_PKT_DEFINE Firmware upgrade flags
 *  @ingroup SL_SI91X_CONSTANTS
 *
 *   This section provides a reference to Firmware Update functionality.
 */

/*! @defgroup SI91X_HTTP_CLIENT_DEFINE HTTP Flags
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to Si91x flags for use in the Hyper-Text Transfer Protocol (HTTP) functionality.
 */

/*! @defgroup SI91X_JOIN_FEATURE_BIT_MAP Join Feature Bitmap
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to the Si91x join feature bitmap.
 */

/*! @defgroup SI91X_DTIM_ALIGNMENT_TYPES DTIM Alignment Types
 *  @ingroup SL_SI91X_CONSTANTS
 *	
 *   This section provides a reference to the Si91x DTIM alignment types.
 */
