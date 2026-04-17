/***************************************************************************//**
 * @file
 * @brief HAL functions to control antenna mode
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sl_status.h"
#include "sl_rail_util_ant_div.h"
#include "sl_rail.h"
#include "em_device.h"
#include "sl_gpio.h"

#if ((_SILICON_LABS_32B_SERIES_2_CONFIG == 1) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 3) \
  || (_SILICON_LABS_32B_SERIES_2_CONFIG == 8))
#include "sl_rail_util_rf_path_config.h"
#endif

// EFR32XG2 series doesn't use locations
// so the HAL configurator doesn't provide any. But EFR32XG2 does have rf_path
// selection, and SL_RAIL_UTIL_ANT_DIV_ANT0_LOC is used for that.
// On EFR32XG2 series, default location(s) to 1 to select RAIL scheme rf_path 1;
// to force use of legacy GPIO scheme (because their GPIO choice for Tx-only
// diversity isn't supported by the radio), user must define each respective
// SL_RAIL_UTIL_ANT_DIV_[N]SEL_LOC as -1 in their HAL config include.
#ifndef SL_RAIL_UTIL_ANT_DIV_ANT0_LOC
// Location to use RAIL scheme on rf_path 1
  #define SL_RAIL_UTIL_ANT_DIV_ANT0_LOC 1
#endif
#ifndef SL_RAIL_UTIL_ANT_DIV_ANT1_LOC
// Location to use RAIL scheme on rf_path 1
  #define SL_RAIL_UTIL_ANT_DIV_ANT1_LOC 1
#endif

// Determine scheme to use based on platform, PHY, debug, and GPIO location(s):
#if ((defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT)) \
  && (!defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || (SL_RAIL_UTIL_ANT_DIV_ANT0_LOC >= 0))   \
  && (!defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT) || (SL_RAIL_UTIL_ANT_DIV_ANT1_LOC >= 0)))
  #define ANTENNA_USE_RAIL_SCHEME 1
#else
  #define ANTENNA_USE_RAIL_SCHEME 0
#endif

#define RX_ANTENNA_SUPPORT (SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT \
                            || (SL_RAIL_UTIL_ANTENNA_RX_DEFAULT_MODE != SL_RAIL_UTIL_ANTENNA_MODE_DISABLED))

#if defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT)
#define SL_RAIL_UTIL_ANT_DIV_ENABLE 1
#else //!(defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT))
#define SL_RAIL_UTIL_ANT_DIV_ENABLE 0
#endif //defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT)

// Flag any mis-configuration
#if     ANTENNA_USE_RAIL_SCHEME
  #if     (defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) && !defined(SL_RAIL_UTIL_ANT_DIV_ANT0_LOC))
    #error "Antenna Diversity needs SL_RAIL_UTIL_ANT_DIV_ANT0_LOC specified"
  #endif//(defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) && !defined(SL_RAIL_UTIL_ANT_DIV_ANT0_LOC))
  #if     (defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT) && !defined(SL_RAIL_UTIL_ANT_DIV_ANT1_LOC))
    #error "Antenna Diversity needs SL_RAIL_UTIL_ANT_DIV_ANT1_LOC specified"
  #endif//(defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT) && !defined(SL_RAIL_UTIL_ANT_DIV_ANT1_LOC))
#else//!ANTENNA_USE_RAIL_SCHEME
  #if RX_ANTENNA_SUPPORT
    #error "Rx Antenna Diversity requires SL_RAIL_UTIL_ANT_DIV_ANT[0|1]SEL_PORT/PIN/LOC to be defined."
  #endif//RX_ANTENNA_SUPPORT
#endif//ANTENNA_USE_RAIL_SCHEME

// RAIL Antenna Configuration

sl_status_t sl_rail_util_ant_div_init(void)
{
 #if ANTENNA_USE_RAIL_SCHEME
#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  sl_rail_antenna_config_t antenna_config = { false, }; // Zero out structure
  sl_rail_util_ant_div_get_antenna_config(&antenna_config);
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  RAIL_AntennaConfig_t antenna_config = { false, }; // Zero out structure
  sl_rail_util_ant_div_get_antenna_config(&antenna_config);
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  if (sl_rail_config_antenna(SL_RAIL_EFR32_HANDLE,
                             (const sl_rail_antenna_config_t *)&antenna_config)
      != SL_RAIL_STATUS_NO_ERROR) {
    return SL_STATUS_NOT_SUPPORTED;
  }
  sl_status_t status = sl_rail_util_ant_div_set_rx_antenna_mode(SL_RAIL_UTIL_ANTENNA_RX_DEFAULT_MODE);
  if (status != SL_STATUS_OK) {
    return status;
  }
  return sl_rail_util_ant_div_set_tx_antenna_mode(SL_RAIL_UTIL_ANTENNA_TX_DEFAULT_MODE);
 #else//!ANTENNA_USE_RAIL_SCHEME
  return SL_STATUS_OK;
 #endif//ANTENNA_USE_RAIL_SCHEME
}

#if     (defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT))

static sl_rail_util_antenna_mode_t tx_antenna_mode = SL_RAIL_UTIL_ANTENNA_TX_DEFAULT_MODE;
// Default to first antenna
static sl_rail_util_antenna_selection_t tx_antenna_selection = SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1;
static sl_rail_util_antenna_mode_t rx_antenna_mode = SL_RAIL_UTIL_ANTENNA_RX_DEFAULT_MODE;

// Rx Antenna Diversity
sl_rail_util_antenna_mode_t sl_rail_util_ant_div_get_rx_antenna_mode(void)
{
  return rx_antenna_mode;
}

#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_get_antenna_config(sl_rail_antenna_config_t *p_antenna_config)
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_get_antenna_config(RAIL_AntennaConfig_t *p_antenna_config)
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
{
  if (NULL == p_antenna_config) {
    return;
  }
#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
#if (defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) \
  && defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PIN))
  p_antenna_config->antenna_0_pin_enable = true;
  p_antenna_config->antenna_0_port = (uint8_t)SL_RAIL_UTIL_ANT_DIV_ANT0_PORT;
  p_antenna_config->antenna_0_pin  = SL_RAIL_UTIL_ANT_DIV_ANT0_PIN;
  // This will be overriden if the PATH is specified separatly.
  p_antenna_config->default_rf_path = SL_RAIL_UTIL_ANT_DIV_ANT0_LOC;
#endif // ant0 port & pin

#if (defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT) \
  && defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PIN))
  p_antenna_config->antenna_1_pin_enable = true;
  p_antenna_config->antenna_1_port = (uint8_t)SL_RAIL_UTIL_ANT_DIV_ANT1_PORT;
  p_antenna_config->antenna_1_pin  = SL_RAIL_UTIL_ANT_DIV_ANT1_PIN;
#endif // ant1 port & pin

#if defined(SL_RAIL_UTIL_RF_PATH_INT_RF_PATH_MODE) // efr32xg2x chip-specific
  p_antenna_config->default_rf_path = SL_RAIL_UTIL_RF_PATH_INT_RF_PATH_MODE;
#endif // internal rf path
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
#if (defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) \
  && defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PIN))
  p_antenna_config->ant0PinEn = true;
  p_antenna_config->ant0Port = (uint8_t)SL_RAIL_UTIL_ANT_DIV_ANT0_PORT;
  p_antenna_config->ant0Pin  = SL_RAIL_UTIL_ANT_DIV_ANT0_PIN;
  // If we are on series 2, use the LOC define as the default path.
  // This will be overriden if the PATH is specified separatly.
  p_antenna_config->defaultPath = SL_RAIL_UTIL_ANT_DIV_ANT0_LOC;
#endif // ant0 port & pin

#if (defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT) \
  && defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PIN))
  p_antenna_config->ant1PinEn = true;
  p_antenna_config->ant1Port = (uint8_t)SL_RAIL_UTIL_ANT_DIV_ANT1_PORT;
  p_antenna_config->ant1Pin  = SL_RAIL_UTIL_ANT_DIV_ANT1_PIN;
#endif // ant1 port & pin

#if defined(SL_RAIL_UTIL_RF_PATH_INT_RF_PATH_MODE) // efr32xg2x chip-specific
  p_antenna_config->defaultPath = SL_RAIL_UTIL_RF_PATH_INT_RF_PATH_MODE;
#endif // internal rf path
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
}

static sl_rail_events_t get_rx_antenna_options(sl_rail_util_antenna_mode_t mode)
{
  switch (mode) {
    default:
    case SL_RAIL_UTIL_ANTENNA_MODE_DISABLED: // Leave antenna untouched (e.g. from Tx)
      return SL_RAIL_RX_OPTIONS_NONE;
      break;
    case SL_RAIL_UTIL_ANTENNA_MODE_ENABLE1:
      return SL_RAIL_RX_OPTION_ANTENNA_0;
      break;
    case SL_RAIL_UTIL_ANTENNA_MODE_ENABLE2:
      return SL_RAIL_RX_OPTION_ANTENNA_1;
      break;
    case SL_RAIL_UTIL_ANTENNA_MODE_DIVERSITY: // Use radio config's diversity scheme
      return SL_RAIL_RX_OPTION_ANTENNA_AUTO;
      break;
  }
}

#ifdef SL_CATALOG_RAIL_UTIL_INIT_PRESENT
#include "sl_rail_util_init.h"
#else
extern sl_rail_handle_t emPhyRailHandle;
#endif

static sl_status_t radio_config_rx_antenna(sl_rail_util_antenna_mode_t mode)
{
  sl_rail_handle_t rail_handle
#ifdef SL_CATALOG_RAIL_UTIL_INIT_PRESENT
#if SL_RAIL_UTIL_INIT_INST0_ENABLE
    = sl_rail_util_get_handle(SL_RAIL_UTIL_HANDLE_INST0);
#else
#error "RAIL instance not initialized"
#endif
#else
    = emPhyRailHandle;
#endif
  if ((rail_handle == NULL) || (rail_handle == SL_RAIL_EFR32_HANDLE)) {
    // This call is premature, before radio is initialized.
    // Defer to when we're re-called as part of halPluginConfig2p4GHzRadio().
    return SL_STATUS_OK;
  }
  // Tell RAIL what Rx antenna mode to use
  return (sl_rail_config_rx_options(rail_handle, SL_RAIL_RX_OPTION_ANTENNA_AUTO,
                                    get_rx_antenna_options(mode))
          == SL_RAIL_STATUS_NO_ERROR) ? SL_STATUS_OK : SL_STATUS_FAIL;
}

sl_status_t sl_rail_util_ant_div_update_antenna_config(void)
{
  return radio_config_rx_antenna(sl_rail_util_ant_div_get_rx_antenna_mode());
}

sl_status_t sl_rail_util_ant_div_set_rx_antenna_mode(sl_rail_util_antenna_mode_t mode)
{
 #if RX_ANTENNA_SUPPORT
  sl_status_t status  = SL_STATUS_NOT_SUPPORTED;
  if (mode <= SL_RAIL_UTIL_ANTENNA_MODE_DIVERSITY) {
    // Tell RAIL what Rx antenna mode to use, if we can
    status = radio_config_rx_antenna(mode);
  }
  if (status == SL_STATUS_OK) {
    rx_antenna_mode = mode;
  }
  return status;
 #else//!RX_ANTENNA_SUPPORT
  return ((mode == SL_RAIL_UTIL_ANTENNA_MODE_DISABLED) ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED);
 #endif//RX_ANTENNA_SUPPORT
}

// Tx Antenna Diversity

static void select_tx_antenna(sl_rail_util_antenna_selection_t tx_antenna)
{
  tx_antenna_selection = tx_antenna;
 #if     ANTENNA_USE_RAIL_SCHEME
  // PHY does selection when a transmit is begun
 #else//!ANTENNA_USE_RAIL_SCHEME
  if (tx_antenna == SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1) {
   #ifdef  SL_RAIL_UTIL_ANT_DIV_ANT1_PORT
    sl_gpio_clear_pin(&(sl_gpio_t){SL_RAIL_UTIL_ANT_DIV_ANT1_PORT, SL_RAIL_UTIL_ANT_DIV_ANT1_PIN });
   #endif//SL_RAIL_UTIL_ANT_DIV_ANT1_PORT
   #ifdef  SL_RAIL_UTIL_ANT_DIV_ANT0_PORT
    sl_gpio_set_pin(&(sl_gpio_t){SL_RAIL_UTIL_ANT_DIV_ANT0_PORT, SL_RAIL_UTIL_ANT_DIV_ANT0_PIN });
   #endif//SL_RAIL_UTIL_ANT_DIV_ANT0_PORT
  } else { // (tx_antenna == SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA2)
   #ifdef  SL_RAIL_UTIL_ANT_DIV_ANT0_PORT
    sl_gpio_clear_pin(&(sl_gpio_t){SL_RAIL_UTIL_ANT_DIV_ANT0_PORT, SL_RAIL_UTIL_ANT_DIV_ANT0_PIN });
   #endif//SL_RAIL_UTIL_ANT_DIV_ANT0_PORT
   #ifdef  SL_RAIL_UTIL_ANT_DIV_ANT1_PORT
    sl_gpio_set_pin(&(sl_gpio_t){SL_RAIL_UTIL_ANT_DIV_ANT1_PORT, SL_RAIL_UTIL_ANT_DIV_ANT1_PIN });
   #endif//SL_RAIL_UTIL_ANT_DIV_ANT1_PORT
  }
 #endif//ANTENNA_USE_RAIL_SCHEME
}

sl_rail_util_antenna_selection_t sl_rail_util_ant_div_get_tx_antenna_selected(void)
{
  return tx_antenna_selection;
}

sl_rail_util_antenna_mode_t sl_rail_util_ant_div_get_tx_antenna_mode(void)
{
  return tx_antenna_mode;
}

sl_status_t sl_rail_util_ant_div_set_tx_antenna_mode(sl_rail_util_antenna_mode_t mode)
{
  sl_status_t status = SL_STATUS_OK;

  switch (mode) {
    case SL_RAIL_UTIL_ANTENNA_MODE_DISABLED: // Leave antenna untouched (e.g. from Rx)
      break;
    case SL_RAIL_UTIL_ANTENNA_MODE_ENABLE1:
      select_tx_antenna(SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1);
      break;
    case SL_RAIL_UTIL_ANTENNA_MODE_ENABLE2:
      select_tx_antenna(SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA2);
      break;
    case SL_RAIL_UTIL_ANTENNA_MODE_DIVERSITY: // Ensure current selection is effected
      select_tx_antenna(tx_antenna_selection);
      break;
    default:
      status = SL_STATUS_NOT_SUPPORTED;
      break;
  }

  if (status == SL_STATUS_OK) {
    tx_antenna_mode = mode;
  }

  return status;
}

sl_status_t sl_rail_util_ant_div_toggle_tx_antenna(void)
{
  if (tx_antenna_mode == SL_RAIL_UTIL_ANTENNA_MODE_DIVERSITY) {
    select_tx_antenna(tx_antenna_selection ^ SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1 ^ SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA2);
    return SL_STATUS_OK;
  }
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_rail_util_ant_div_set_tx_antenna(sl_rail_util_antenna_selection_t tx_antenna)
{
  if (tx_antenna == tx_antenna_selection) { // Already selected!
    return SL_STATUS_OK;
  }
  return sl_rail_util_ant_div_toggle_tx_antenna();
}

#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_rx_options(sl_rail_rx_options_t *p_rx_options)
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_rx_options(RAIL_RxOptions_t *p_rx_options)
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
{
  if (NULL == p_rx_options) {
    return;
  }
#if (SL_RAIL_UTIL_ANT_DIV_RX_MODE != SL_RAIL_UTIL_ANT_DIV_DISABLED)
#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  *p_rx_options = (*p_rx_options
                   & ~(SL_RAIL_RX_OPTION_ANTENNA_0 | SL_RAIL_RX_OPTION_ANTENNA_1))
                  | ((((sl_rail_rx_options_t)SL_RAIL_UTIL_ANT_DIV_RX_MODE)
                      << SL_RAIL_RX_OPTION_ANTENNA_0_SHIFT)
                     & (SL_RAIL_RX_OPTION_ANTENNA_0 | SL_RAIL_RX_OPTION_ANTENNA_1));
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  *p_rx_options = (*p_rx_options
                   & ~(RAIL_RX_OPTION_ANTENNA0 | RAIL_RX_OPTION_ANTENNA1))
                  | ((((RAIL_RxOptions_t)SL_RAIL_UTIL_ANT_DIV_RX_MODE)
                      << RAIL_RX_OPTION_ANTENNA0_SHIFT)
                     & (RAIL_RX_OPTION_ANTENNA0 | RAIL_RX_OPTION_ANTENNA1));
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
#endif
}

#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_tx_options(sl_rail_tx_options_t *p_tx_options)
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_tx_options(RAIL_TxOptions_t *p_tx_options)
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
{
  if (NULL == p_tx_options) {
    return;
  }
#if (SL_RAIL_UTIL_ANT_DIV_TX_MODE != SL_RAIL_UTIL_ANT_DIV_DISABLED)
#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  *p_tx_options = (*p_tx_options
                   & ~(SL_RAIL_TX_OPTION_ANTENNA_0 | SL_RAIL_TX_OPTION_ANTENNA_1))
                  | ((((sl_rail_tx_options_t)SL_RAIL_UTIL_ANT_DIV_TX_MODE)
                      << SL_RAIL_TX_OPTION_ANTENNA_0_SHIFT)
                     & (SL_RAIL_TX_OPTION_ANTENNA_0 | SL_RAIL_TX_OPTION_ANTENNA_1));
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
  *p_tx_options = (*p_tx_options
                   & ~(RAIL_TX_OPTION_ANTENNA0 | RAIL_TX_OPTION_ANTENNA1))
                  | ((((RAIL_TxOptions_t)SL_RAIL_UTIL_ANT_DIV_TX_MODE)
                      << RAIL_TX_OPTION_ANTENNA0_SHIFT)
                     & (RAIL_TX_OPTION_ANTENNA0 | RAIL_TX_OPTION_ANTENNA1));
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
#endif
}

#define ANTDIV_RX_PHY_DEFAULT_ENABLED (SL_RAIL_UTIL_ANTENNA_RX_DEFAULT_MODE != SL_RAIL_UTIL_ANTENNA_MODE_DISABLED)

#if SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT
#define ANTDIV_RX_PHY_SELECTED (rx_antenna_mode == SL_RAIL_UTIL_ANTENNA_MODE_DIVERSITY)
#elif ANTDIV_RX_PHY_DEFAULT_ENABLED
#define ANTDIV_RX_PHY_SELECTED (true)
#else //!ANTDIV_RX_PHY_DEFAULT_ENABLED
#define ANTDIV_RX_PHY_SELECTED (false)
#endif //SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT

bool sl_rail_util_ant_div_get_phy_select(void)
{
  return ANTDIV_RX_PHY_SELECTED;
}
#else//!(defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT))

sl_rail_util_antenna_mode_t sl_rail_util_ant_div_get_rx_antenna_mode(void)
{
  return SL_RAIL_UTIL_ANTENNA_MODE_DISABLED;
}

sl_status_t sl_rail_util_ant_div_set_rx_antenna_mode(sl_rail_util_antenna_mode_t mode)
{
  return ((mode == SL_RAIL_UTIL_ANTENNA_MODE_DISABLED) ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED);
}

sl_rail_util_antenna_selection_t sl_rail_util_ant_div_get_tx_antenna_selected(void)
{
  return SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1;
}

sl_rail_util_antenna_mode_t sl_rail_util_ant_div_get_tx_antenna_mode(void)
{
  return SL_RAIL_UTIL_ANTENNA_MODE_ENABLE1;
}

sl_status_t sl_rail_util_ant_div_set_tx_antenna_mode(sl_rail_util_antenna_mode_t mode)
{
  return ((mode == SL_RAIL_UTIL_ANTENNA_MODE_ENABLE1) ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED);
}

sl_status_t sl_rail_util_ant_div_toggle_tx_antenna(void)
{
  return SL_STATUS_NOT_SUPPORTED;
}

sl_status_t sl_rail_util_ant_div_set_tx_antenna(sl_rail_util_antenna_selection_t tx_antenna)
{
  return ((tx_antenna == SL_RAIL_UTIL_ANTENNA_SELECT_ANTENNA1)
          ? SL_STATUS_OK : SL_STATUS_NOT_SUPPORTED);
}

#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_get_antenna_config(sl_rail_antenna_config_t *p_antenna_config)
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_get_antenna_config(RAIL_AntennaConfig_t *p_antenna_config)
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
{
  (void)p_antenna_config;
}

sl_status_t sl_rail_util_ant_div_update_antenna_config(void)
{
  return SL_STATUS_NOT_SUPPORTED;
}

#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_rx_options(sl_rail_rx_options_t *p_rx_options)
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_rx_options(RAIL_RxOptions_t *p_rx_options)
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
{
  (void)p_rx_options;
}

#ifdef  SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_tx_options(sl_rail_rx_options_t *p_tx_options)
#else//!SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
void sl_rail_util_ant_div_init_tx_options(RAIL_RxOptions_t *p_tx_options)
#endif//SL_CATALOG_SL_RAIL_UTIL_ANT_DIV_PRESENT
{
  (void)p_tx_options;
}

bool sl_rail_util_ant_div_get_phy_select(void)
{
  return false;
}
#endif//(defined(SL_RAIL_UTIL_ANT_DIV_ANT0_PORT) || defined(SL_RAIL_UTIL_ANT_DIV_ANT1_PORT))
