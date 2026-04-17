/***************************************************************************//**
 * @brief Internal types and defines for Bluetooth configuration
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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

#ifndef SLI_BT_CONFIG_DEFS_H
#define SLI_BT_CONFIG_DEFS_H

#include "sl_status.h"

/** @brief Forward declaration of the internal feature structure */
struct sli_bt_feature;

/** @brief Forward declaration of the internal BGAPI class structure */
struct sli_bgapi_class;

/**
 * @brief Forward declaration of internal feature config structure
 *
 * Note that this is only used as an opaque type that is never instantiated
 * directly. The actual configuration structure is defined and populated in
 * feature-specific files.
 */
struct sli_bt_feature_config;

/** @brief Structure to specify the use of a feature and its configuration */
struct sli_bt_feature_use {
  const struct sli_bt_feature        *feature; /**< Pointer to the feature definition */
  const struct sli_bt_feature_config *config;  /**< Pointer to the feature-specific configuration */
};

/**
 * @brief Macro that expands to the name of a BGAPI class structure
 *
 * @param[in] category_name Short category name string without quotes, for
 *   example bt, btmesh, or btctrl
 * @param[in] class_name Short class name string without quotes, for example
 *   advertiser or connection
 */
#define SLI_BT_BGAPI_CLASS_NAME(category_name, class_name, mode) \
  sli_bgapi_class_ ## category_name ## _ ## class_name ## _ ## mode

/**
 * @brief Macro to forward declare a BGAPI class structure
 */
#define SLI_BT_DECLARE_BGAPI_CLASS(category_name, class_name)                                   \
  extern const struct sli_bgapi_class SLI_BT_BGAPI_CLASS_NAME(category_name, class_name, full); \
  extern const struct sli_bgapi_class SLI_BT_BGAPI_CLASS_NAME(category_name, class_name, optimized)

#ifdef SL_BT_API_FULL
/**
 * @brief Macro to declare the use of a BGAPI class
 *
 * This macro is used by stack-specific initialization code to declare the use
 * of a BGAPI class in the array of used BGAPI classes.
 *
 * @param[in] category_name Short category name string without quotes, for
 *   example bt, btmesh, or btctrl
 * @param[in] class_name Short class name string without quotes, for example
 *   advertiser or connection
 */
#define SLI_BT_USE_BGAPI_CLASS(category_name, class_name) \
  & SLI_BT_BGAPI_CLASS_NAME(category_name, class_name, full)
#else
#define SLI_BT_USE_BGAPI_CLASS(category_name, class_name) \
  & SLI_BT_BGAPI_CLASS_NAME(category_name, class_name, optimized)
#endif

#endif // SLI_BT_CONFIG_DEFS_H
