/***************************************************************************/ /**
 * @file sli_si91x_power_manager.c
 * @brief Power Manager Internal API implementation
 *******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_si91x_power_manager.h"
#include "sli_si91x_power_manager_board_config.h"
#include "clock_update.h"
#include "rsi_pll.h"
#include "rsi_power_save.h"
#include "rsi_rom_clks.h"
#include "rsi_rom_power_save.h"
#include "rsi_rom_ulpss_clk.h"
#include "rsi_ps_ram_func.h"
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
#include "rsi_m4.h"
#include "sl_si91x_types.h"
#include "sl_rsi_utility.h"
#include "sl_si91x_m4_ps.h"
#endif
#ifdef SL_SI91X_POWER_MANAGER_UC_AVAILABLE
#include "sl_si91x_power_manager_wakeup_handler.h"
#endif
#include "rsi_debug.h"
#include "sli_si91x_clock_manager.h"
#if defined(SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED) && (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED)
#include "sli_si91x_32khz_rc_calibration.h"
#endif
/*******************************************************************************
 ***************************  DEFINES / MACROS   ********************************
 ******************************************************************************/
#define P2P_STATUS_REG  *(volatile uint32_t *)(M4SS_P2P_INT_BASE_ADDRESS + 0x174) // P2P status register
#define JUMP_CB_ADDRESS (uint32_t) RSI_PS_RestoreCpuContext // Callback Address to restore CPU context
#if (SL_SI91X_SI917_RAM_MEM_CONFIG == 1)
#define MAX_M4SS_RAM_SIZE 192 // Maximum m4ss RAM size
#elif (SL_SI91X_SI917_RAM_MEM_CONFIG == 2)
#define MAX_M4SS_RAM_SIZE 256 // Maximum m4ss RAM size
#elif (SL_SI91X_SI917_RAM_MEM_CONFIG == 3)
#define MAX_M4SS_RAM_SIZE 320 // Maximum m4ss RAM size
#endif
#define MAX_ULPSS_RAM_SIZE     8         // Maximum ulpss RAM size
#define SOC_PLL_REF_FREQUENCY  40000000  // SOC Pll reference frequency
#define INTF_PLL_REF_FREQUENCY 40000000  // INTF Pll reference frequency
#define PS4_HP_FREQUENCY       180000000 // PS4 high power clock frequency
#define PS4_LP_FREQUENCY       100000000 // PS4 low power clock frequency
#ifdef SI_SI91X_NWP_SHUTDOWN
#define PS3_HP_FREQUENCY 90000000 // PS3 high power clock frequency for NWP shutdown case
#else
#define PS3_HP_FREQUENCY 80000000 // PS3 high power clock frequency
#endif
#define PS3_LP_FREQUENCY         40000000   // PS3 low power clock frequency
#define DIVISION_FACTOR          0          // Division Factor for clock
#define PMU_WAIT_TIME            15         // Max PMU turn on wait time
#define LDO_WAIT_TIME            15         // Max LDO turn on wait time
#define PMU_GOOD_TIME            0x34       // PMU good time duration
#define XTAL_GOOD_TIME           31         // XTAL good duration count
#define VALID_M4SS_PERIPHERAL    0x466A10   // Valid bits for M4SS peripheral
#define VALID_ULPSS_PERIPHERAL   0x1FEC0000 // Valid bits for ULPSS peripheral
#define VALID_NPSS_PERIPHERAL    0x107FE    // Valid bits for NPSS peripheral
#define VALID_WAKEUP_SOURCES     0x3F7F0000 // Valid bits for Wakeup sources
#define VALID_RAM_RETENTION_MODE 0xFC       // Valid bits for RAM retention mode
#if (SL_SI91X_SI917_RAM_MEM_CONFIG == 1)
#define VALID_M4SS_RAM 0xFF // Valid bits for M4SS RAM Banks
#elif (SL_SI91X_SI917_RAM_MEM_CONFIG == 2)
#define VALID_M4SS_RAM 0x1FF // Valid bits for M4SS RAM Banks
#elif (SL_SI91X_SI917_RAM_MEM_CONFIG == 3)
#define VALID_M4SS_RAM 0x3FF // Valid bits for M4SS RAM Banks
#endif
#define VALID_ULPSS_RAM         0xF // Valid bits for ULPSS RAM Banks
#define VALID                   1   // Valid
#define INVALID                 0   // Invalid
#define NO_OF_ACTIVE_STATES     3   // Number of active states
#define PS_OFFSET               2   // Power State offset
#define NO_OF_TRANSITIONS       7 // Number of all possible transitions, i.e., PS0, PS1, PS2, PS3, PS4, Sleep and Standby
#define NO_OF_STATE_TRANSITIONS 5   // Number of all state transitions, i.e., PS0, PS1, PS2, PS3, PS4.
#define RAM_2_KB                2   // Validation for 2 KB RAM
#define RAM_4_KB                4   // Validation for 4 KB RAM
#define RAM_6_KB                6   // Validation for 6 KB RAM
#define RAM_8_KB                8   // Validation for 8 KB RAM
#define RAM_12_KB               12  // Validation for 12 KB RAM
#define RAM_16_KB               16  // Validation for 16 KB RAM
#define RAM_32_KB               32  // Validation for 32 KB RAM
#define RAM_64_KB               64  // Validation for 64 KB RAM
#define RAM_128_KB              128 // Validation for 128 KB RAM
#define RAM_192_KB              192 // Validation for 192 KB RAM
#define RAM_256_KB              256 // Validation for 256 KB RAM
#define RAM_320_KB              320 // Validation for 320 KB RAM

#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
// MBR magic word for flash initialization
#ifdef SLI_SI91X_MCU_4MB_LITE_IMAGE
#define MBR_MAGIC_WORD (*(volatile uint32_t *)(0x8160000))
#else
#define MBR_MAGIC_WORD (*(volatile uint32_t *)(0x81F0000))
#endif

#define NVIC_ISER_INDEX_FOR_IRQ74 (2U)  // NVIC->ISER[2] covers IRQs 64�95
#define NVIC_ISER_BIT_FOR_IRQ74   (10U) // Bit 10 corresponds to IRQ #74
#define NVIC_ISER_MASK_FOR_IRQ74  BIT(NVIC_ISER_BIT_FOR_IRQ74)
#endif

/*******************************************************************************
 ***************************  Local Types  ********************************
 ******************************************************************************/
typedef void (*power_state_fptr)(void);

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/
static void ps4_to_ps2_state_change(void);
static void ps4_to_ps3_state_change(void);
static void ps4_to_ps0_state_change(void);
static void ps3_to_ps4_state_change(void);
static void ps3_to_ps2_state_change(void);
static void ps3_to_ps0_state_change(void);
static void ps2_to_ps4_state_change(void);
static void ps2_to_ps3_state_change(void);
static void ps2_to_ps1_state_change(void);
static void initialize_flash(void);
static void get_ram_retention_mode(uint32_t m4ss_ram, uint32_t *m4ss_ram_retention);
static sl_status_t configure_ram_memory(sl_power_ram_retention_config_t *config,
                                        uint32_t *m4ss_ram,
                                        uint32_t *ulpss_ram);
static sl_status_t trigger_sleep(sli_power_sleep_config_t *config, uint8_t sleep_type);
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error);
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
__WEAK sl_status_t sli_si91x_submit_rx_pkt(void);
#endif
/*******************************************************************************
 *************************** LOCAL VARIABLES   *******************************
 ******************************************************************************/

// Power state transition structure
typedef struct {
  boolean_t is_valid;    // State is valid or not
  power_state_fptr fptr; // State change function pointer
} power_state_transition_t;

typedef struct {
  sl_power_state_t from_ps;                          // From the power state it transits
  power_state_transition_t to_ps[NO_OF_TRANSITIONS]; // All possible transition states
} power_state_struct_t;

// Power State transition
static const power_state_struct_t ps_transition[NO_OF_ACTIVE_STATES] = {
  {
    .from_ps                               = SL_SI91X_POWER_MANAGER_PS2,
    .to_ps[SL_SI91X_POWER_MANAGER_PS0]     = { .is_valid = INVALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_PS1]     = { .is_valid = VALID, .fptr = ps2_to_ps1_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS2]     = { .is_valid = INVALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_PS3]     = { .is_valid = VALID, .fptr = ps2_to_ps3_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS4]     = { .is_valid = VALID, .fptr = ps2_to_ps4_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_SLEEP]   = { .is_valid = VALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_STANDBY] = { .is_valid = VALID, .fptr = NULL },
  },
  {
    .from_ps                               = SL_SI91X_POWER_MANAGER_PS3,
    .to_ps[SL_SI91X_POWER_MANAGER_PS0]     = { .is_valid = VALID, .fptr = ps3_to_ps0_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS1]     = { .is_valid = INVALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_PS2]     = { .is_valid = VALID, .fptr = ps3_to_ps2_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS3]     = { .is_valid = INVALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_PS4]     = { .is_valid = VALID, .fptr = ps3_to_ps4_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_SLEEP]   = { .is_valid = VALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_STANDBY] = { .is_valid = VALID, .fptr = NULL },
  },
  {
    .from_ps                               = SL_SI91X_POWER_MANAGER_PS4,
    .to_ps[SL_SI91X_POWER_MANAGER_PS0]     = { .is_valid = VALID, .fptr = ps4_to_ps0_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS1]     = { .is_valid = INVALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_PS2]     = { .is_valid = VALID, .fptr = ps4_to_ps2_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS3]     = { .is_valid = VALID, .fptr = ps4_to_ps3_state_change },
    .to_ps[SL_SI91X_POWER_MANAGER_PS4]     = { .is_valid = INVALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_SLEEP]   = { .is_valid = VALID, .fptr = NULL },
    .to_ps[SL_SI91X_POWER_MANAGER_STANDBY] = { .is_valid = VALID, .fptr = NULL },
  }
};

/*******************************************************************************
***********************  Global function Definitions *************************
 ******************************************************************************/

/*******************************************************************************
 * Updates the Power State as per the from and to parameters.
 * Calls the static functions from the constant structure
 * according to the state change requirement.
 * By default it returns SL_STATUS_INVALID_PARAMETER
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_change_power_state(sl_power_state_t from, sl_power_state_t to)
{
  sl_status_t status = SL_STATUS_OK;
  if ((from > SL_SI91X_POWER_MANAGER_PS4) || (from < SL_SI91X_POWER_MANAGER_PS2) || (to >= LAST_ENUM_POWER_STATE)) {
    // Validate the power state, if not in range returns error code.
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  if ((to == SL_SI91X_POWER_MANAGER_PS1) || (to == SL_SI91X_POWER_MANAGER_PS0)) {
    SL_SI91X_POWER_MANAGER_CORE_EXIT_CRITICAL;
  }
  if (ps_transition[from - PS_OFFSET].to_ps[to].fptr != NULL) {
    // If the from and to state transition function pointer is not null,
    // then it calls the function and state change is performed.
    ps_transition[from - PS_OFFSET].to_ps[to].fptr();
    status = SL_STATUS_OK;
  } else {
    // If it reached here, returns Null pointer
    status = SL_STATUS_NULL_POINTER;
  }
  return status;
}

/*******************************************************************************
 * To set the sleep configurations.
 * It takes macro from the sli_si91x_power_manager_board_config.h file where
 * stack address, callback address, vector offset are configured according to the
 * board.
 * It switches off the components which are not required and goes to sleep with retention.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_set_sleep_configuration(sl_power_state_t state)
{
  sl_status_t status;
  if ((state < SL_SI91X_POWER_MANAGER_PS2) || (state > SL_SI91X_POWER_MANAGER_PS4)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  sli_power_sleep_config_t config;
  config.low_freq_clock          = SLI_SI91X_POWER_MANAGER_DISABLE_LF_MODE;
  config.stack_address           = SL_SLEEP_RAM_USAGE_ADDRESS;
  config.vector_offset           = SL_SLEEP_VECTOR_OFFSET_WITH_RETENTION;
  config.wakeup_callback_address = JUMP_CB_ADDRESS;
  if (state == SL_SI91X_POWER_MANAGER_PS2) {
    config.mode = SLI_SI91X_POWER_MANAGER_WAKEUP_WITH_RETENTION;
  } else {
#if defined(SI_SI91X_NWP_SHUTDOWN)
    config.mode = SLI_SI91X_POWER_MANAGER_WAKEUP_WITH_RETENTION;
#else
    config.mode = SLI_SI91X_POWER_MANAGER_WAKEUP_FROM_FLASH_MODE;
#endif
  }
#ifdef SL_SI91X_POWER_MANAGER_UC_AVAILABLE
  // Initializing and configuring the wakeup sources as per UC inputs, if available
  status = sl_si91x_power_manager_wakeup_init();
  if (status != SL_STATUS_OK) {
    return status;
  }
#endif

#if (SL_SI91X_TICKLESS_MODE == 0)
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
  rsi_p2p_intr_status_bkp_t p2p_intr_status_bkp;
  p2p_intr_status_bkp.tass_p2p_intr_mask_clr_bkp = TASS_P2P_INTR_MASK_CLR;
  p2p_intr_status_bkp.m4ss_p2p_intr_set_reg_bkp  = M4SS_P2P_INTR_SET_REG;
  P2P_STATUS_REG &= ~M4_is_active;
#endif
  // Configuring the clocks as per the sleep state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_SLEEP, SL_SI91X_POWER_MANAGER_POWERSAVE);
#endif

  // If any error code, it returns it otherwise goes to sleep with retention.
  status = trigger_sleep(&config, SLEEP_WITH_RETENTION);
  if (status != SL_STATUS_OK) {
    return status;
  }

#if defined(SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED) && (SL_SI91X_32KHZ_RC_CALIBRATION_ENABLED)
  // set the wakeup overhead flag.
  sli_si91x_set_wakeup_overhead_flag();
#endif

// After waking up, setting the mcu status as active in P2P registers
#if (SL_SI91X_TICKLESS_MODE == 0)
  RSI_PS_SetMCUActiveStatus();
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
  SysTick_Config(SystemCoreClock / 1000);
  // Indicate M4 is active
  P2P_STATUS_REG |= M4_is_active;
  // Restore values from backup
  TASS_P2P_INTR_MASK_CLR = ~p2p_intr_status_bkp.tass_p2p_intr_mask_clr_bkp;
  M4SS_P2P_INTR_SET_REG  = p2p_intr_status_bkp.m4ss_p2p_intr_set_reg_bkp;
#endif
#endif
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Updates the peripheral power state, i.e., enables and disables the peripheral
 * as per requirements. 
 * Validates the members of sl_power_peripheral_t structure and it is passed to 
 * equivalent rsi apis to power-on / power-off the peripherals/components as per
 * the value of add.
 ******************************************************************************/
sl_status_t sli_power_manager_update_peripheral(sl_power_peripheral_t *peripheral, boolean_t add)
{
  if (peripheral == NULL) {
    // Validates peripheral, if null returns error code.
    return SL_STATUS_NULL_POINTER;
  }
  // Validates the M4SS peripherals, if invalid returns error code.
  if ((peripheral->m4ss_peripheral) && !(peripheral->m4ss_peripheral & VALID_M4SS_PERIPHERAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validates the ULPSS peripherals, if invalid returns error code.
  if ((peripheral->ulpss_peripheral) && !(peripheral->ulpss_peripheral & VALID_ULPSS_PERIPHERAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }
  // Validates the NPSS peripherals, if invalid returns error code.
  if ((peripheral->npss_peripheral) && !(peripheral->npss_peripheral & VALID_NPSS_PERIPHERAL)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (add) {
    // RSI APIs to power on the peripherals.
    if (peripheral->m4ss_peripheral != 0) {
      RSI_PS_M4ssPeriPowerUp(peripheral->m4ss_peripheral);
    }
    if (peripheral->ulpss_peripheral != 0) {
      RSI_PS_UlpssPeriPowerUp(peripheral->ulpss_peripheral);
    }
    if (peripheral->npss_peripheral != 0) {
      RSI_PS_NpssPeriPowerUp(peripheral->npss_peripheral);
    }
  } else {
    // RSI APIs to power off the peripherals.
    if (peripheral->m4ss_peripheral != 0) {
      RSI_PS_M4ssPeriPowerDown(peripheral->m4ss_peripheral);
    }
    if (peripheral->ulpss_peripheral != 0) {
      RSI_PS_UlpssPeriPowerDown(peripheral->ulpss_peripheral);
    }
    if (peripheral->npss_peripheral != 0) {
      RSI_PS_NpssPeriPowerDown(peripheral->npss_peripheral);
    }
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Validates the power state transitions. If invalid, returns false.
 * It verifies all the possible state transitions, if valid returns true otherwise
 * returns false.
 ******************************************************************************/
boolean_t sli_si91x_power_manager_is_valid_transition(sl_power_state_t from, sl_power_state_t to)
{
  boolean_t status = false;

  if ((from >= LAST_ENUM_POWER_STATE) || (to >= LAST_ENUM_POWER_STATE)) {
    // Validate the power state, if not in range returns error code.
    status = SL_STATUS_INVALID_PARAMETER;
    return status;
  }
  if (ps_transition[from - PS_OFFSET].to_ps[to].is_valid) {
    // If state change is valid, return true, otherwise return false
    status = true;
  }
  return status;
}

/*******************************************************************************
 * Configures the wakeup sources.
 * As per the flag, wakeup sources are set or cleared.
 * The source is the ored value of wakeup sources from \ref sl_power_wakeup_sources_t enum.
 ******************************************************************************/
sl_status_t sli_si91x_power_configure_wakeup_resource(uint32_t source, boolean_t add)
{
  if (!(source & VALID_WAKEUP_SOURCES)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (add) {
    // If flag is true, rsi API to set the wakeup sources is called.
    RSI_PS_SetWkpSources(source);
  } else {
    // If flag is false, rsi API to clear the wakeup sources is called.
    RSI_PS_ClrWkpSources(source);
  }
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Sets the RAM retention as well as configures the RAM banks which needs to be retained.
 * Validation of all the parameters is performed, if invalid then error code is returned.
 * If configure_ram_banks is enabled, user can configure RAM_BANKS using bank number.
 * If configure_ram_banks is disabled, user can enter the size of RAM to be retained.
 * It configures ram retention based on the selected ram banks.
 ******************************************************************************/
sl_status_t sli_si91x_power_manager_set_ram_retention_configuration(sl_power_ram_retention_config_t *config)
{
  sl_status_t status;
  uint32_t m4ss_ram, ulpss_ram, m4ss_ram_retention;
  m4ss_ram = ulpss_ram = m4ss_ram_retention = 0;
  if (config == NULL) {
    // Validates config, if null, returns error code
    return SL_STATUS_NULL_POINTER;
  }
  if ((!config->configure_ram_banks)
      && ((config->m4ss_ram_size_kb > MAX_M4SS_RAM_SIZE) || (config->ulpss_ram_size_kb > MAX_ULPSS_RAM_SIZE))) {
    // If m4ss ram size is more than 320KB or
    // If ulp4ss ram size is more than 8KB or , returns error code.
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((config->configure_ram_banks) && (config->m4ss_ram_banks) && !(config->m4ss_ram_banks & VALID_M4SS_RAM)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if ((config->configure_ram_banks) && (config->ulpss_ram_banks) && !(config->ulpss_ram_banks & VALID_ULPSS_RAM)) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (!config->configure_ram_banks) {
    // configure_ram_banks flag is false, i.e., memory size is provided.
    // static function to select appropriate RAM size and get the ored value of RAM banks
    status = configure_ram_memory(config, &m4ss_ram, &ulpss_ram);
    if (status != SL_STATUS_OK) {
      // If status is not OK, returns error code.
      return status;
    }
    get_ram_retention_mode(~m4ss_ram, &m4ss_ram_retention);
    // power down the ulpss RAM banks
    RSI_PS_UlpssRamBanksPeriPowerDown(ulpss_ram);
    RSI_PS_UlpssRamBanksPowerDown(ulpss_ram);
    // power down the m4ss RAM banks
    RSI_PS_M4ssRamBanksPeriPowerDown(m4ss_ram);
    RSI_PS_M4ssRamBanksPowerDown(m4ss_ram);
  } else {
    get_ram_retention_mode(~(config->m4ss_ram_banks), &m4ss_ram_retention);
    // configure_ram_banks flag is true, i.e., RAM bank number is provided.
    // power down the m4ss RAM banks
    if (config->m4ss_ram_banks != 0) {
      RSI_PS_M4ssRamBanksPeriPowerDown(config->m4ss_ram_banks);
      RSI_PS_M4ssRamBanksPowerDown(config->m4ss_ram_banks);
    }
    // power down the ulpss RAM banks
    if (config->ulpss_ram_banks != 0) {
      RSI_PS_UlpssRamBanksPeriPowerDown(config->ulpss_ram_banks);
      RSI_PS_UlpssRamBanksPowerDown(config->ulpss_ram_banks);
    }
  }
  m4ss_ram_retention |= ULPSS_RAM_RETENTION_MODE_EN;
  RSI_PS_SetRamRetention(m4ss_ram_retention);
  return SL_STATUS_OK;
}

/*******************************************************************************
 * Sets the initial hardware configuration.
 * Configures the system core clock to current powersave mode.
 ******************************************************************************/
void sli_si91x_power_manager_init_hardware(void)
{
  // Power-Down the deep-sleep timer
  RSI_PS_PowerSupplyDisable(POWER_ENABLE_DEEPSLEEP_TIMER);

  // Sets the system to default Power save mode.
  sli_si91x_clock_manager_config_clks_on_ps_change(sl_si91x_power_manager_get_current_state(),
                                                   sl_si91x_power_manager_get_clock_scaling());
  sli_si91x_power_manager_low_power_hw_config(true);
#if (SL_SI91X_TICKLESS_MODE == 1)
#ifdef SL_SLEEP_TIMER
  RSI_PS_SetWkpSources(SYSRTC_BASED_WAKEUP); //Setting SYSRTC as a wakeup source
#endif
  RSI_PS_SetWkpSources(WIRELESS_BASED_WAKEUP); //Enable Wake-On-Wireless Wakeup source
#endif
}

/*******************************************************************************
 * Configures the hardware for low power mode.
 * Disables the components and clocks which are not required.
 ******************************************************************************/
void sli_si91x_power_manager_low_power_hw_config(boolean_t is_sleep)
{
  // Disable OTHER_CLK which is enabled at Start-up
  RSI_CLK_PeripheralClkDisable3(M4CLK, M4_SOC_CLK_FOR_OTHER_ENABLE);
#if defined(SL_ADVANCE_POWERSAVE_CONFIG) && (SL_ADVANCE_POWERSAVE_CONFIG == ENABLE)
  RSI_ULPSS_TimerClkDisable(ULPCLK);
  RSI_ULPSS_DisableRefClks(MCU_ULP_32KHZ_RO_CLK_EN);
#endif
  if (is_sleep) {
    RSI_PS_AnalogPeriPtatDisable();
  } else {
    RSI_IPMU_ProgramConfigData(ana_perif_ptat_common_config2);
  }
  RSI_PS_PowerSupplyDisable(POWER_ENABLE_TIMESTAMPING);
  // Power-Down High-Frequency PLL Domain
  RSI_PS_SocPllSpiDisable();
  // Power-Down QSPI-DLL Domain
  RSI_PS_QspiDllDomainDisable();
  // Enable first boot up
  RSI_PS_EnableFirstBootUp(1);
  // Configure PMU Start-up Time to be used on Wake-up
  RSI_PS_PmuGoodTimeDurationConfig(PMU_GOOD_TIME);
  // Configure XTAL Start-up Time to be used on Wake-up
  RSI_PS_XtalGoodTimeDurationConfig(XTAL_GOOD_TIME);

  // The API allows configuring the time period for the sleep clock.
  // MSB 8-bits for the Integer part &c
  // LSB 17-bits for the Fractional part
  // Ex: 32kHz clock = 31.25us ==> 31.25*2^17 = 4096000 = 0x3E8000
  // Ex: 32.768kHz clock = 30.51us ==> 30.51*2^17 = 4000000 = 0x3D0900
  RSI_TIMEPERIOD_TimerClkSel(TIME_PERIOD, 0x003E7FFF);
}

/*******************************************************************************
 **********************  Local Function Definition****************************
 ******************************************************************************/

/*******************************************************************************
 * State Change from PS4 to PS2.
 * Hardware is configured for low power mode.
 * LDO and other registers are taken care by RSI api.
 ******************************************************************************/
static void ps4_to_ps2_state_change(void)
{
  // Low power hardware configuration to switch off the components which are not required.
  sli_si91x_power_manager_low_power_hw_config(false);
  // Change to 20MHz-RC to be used as Processor Clock in PS2 state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_PS2,
                                                   sl_si91x_power_manager_get_clock_scaling());
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1) \
  && (SLI_SI91X_MCU_COMMON_FLASH_MODE == 1)
  // Turn off XTAL
  sli_si91x_xtal_turn_off_request_from_m4_to_TA();
  // Reset M4_USING_FLASH bit
  M4SS_P2P_INTR_CLR_REG = M4_USING_FLASH;
  // Set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctr, AON domain power supply controls from M4 to NWP
  MCUAON_CONTROL_REG4 |= (MCU_TASS_REF_CLK_SEL_MUX_CTRL);
  MCUAON_CONTROL_REG4;
#else
  // Disable 40MHZ clock in case PS2 to PS4
  RSI_ULPSS_DisableRefClks(MCU_ULP_40MHZ_CLK_EN);
#endif
  // Switching from PS4 to PS2 state
  RSI_PS_PowerStateChangePs4toPs2(ULP_MCU_MODE,
                                  PWR_MUX_SEL_ULPSSRAM_SCDC_0_9,
                                  PWR_MUX_SEL_M4_ULP_RAM_SCDC_0_9,
                                  PWR_MUX_SEL_M4_ULP_RAM16K_SCDC_0_9,
                                  PWR_MUX_SEL_M4ULP_SCDC_0_6,
                                  PWR_MUX_SEL_ULPSS_SCDC_0_9,
                                  DISABLE_BG_SAMPLE_ENABLE,
                                  DISABLE_DC_DC_ENABLE,
                                  DISABLE_SOCLDO_ENABLE,
                                  DISABLE_STANDBYDC,
                                  DISABLE_TA192K_RAM_RET,
                                  DISABLE_M464K_RAM_RET);
}

/*******************************************************************************
 * State Change from PS4 to PS3.
 ******************************************************************************/
static void ps4_to_ps3_state_change(void)
{
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_PS3,
                                                   sl_si91x_power_manager_get_clock_scaling());
}

/*******************************************************************************
 * State Change from PS4 to PS0 (Sleep without retention).
 * It takes macro from the sli_si91x_power_manager_board_config.h file where
 * stack address, callback address, vector offset are configured according to the
 * board.
 * It switches off the components which are not required and goes to sleep without retention.
 ******************************************************************************/
static void ps4_to_ps0_state_change(void)
{
  sli_power_sleep_config_t config;
  config.low_freq_clock          = SLI_SI91X_POWER_MANAGER_DISABLE_LF_MODE;
  config.mode                    = SLI_SI91X_POWER_MANAGER_WAKEUP_WITHOUT_RETENTION;
  config.stack_address           = SL_SLEEP_STACK_USAGE_ADDRESS;
  config.vector_offset           = SL_SLEEP_VECTOR_OFFSET;
  config.wakeup_callback_address = SL_SLEEP_WAKEUP_CALLBACK_ADDRESS;
  // Low power hardware configuration to switch off the components which are not required.
  sli_si91x_power_manager_low_power_hw_config(true);
#ifdef SL_SI91X_POWER_MANAGER_UC_AVAILABLE
  sl_status_t status_ps4_to_ps0 = SL_STATUS_OK;
  // Initializing and configuring the wakeup sources as per UC inputs, if available
  status_ps4_to_ps0 = sl_si91x_power_manager_wakeup_init();
  if (status_ps4_to_ps0 != SL_STATUS_OK) {
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status_ps4_to_ps0);
    return;
  }
#endif
  // Configuring the clocks as per the sleep state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_SLEEP, SL_SI91X_POWER_MANAGER_POWERSAVE);
#if ((SL_SI91X_TICKLESS_MODE == 1) && (SL_SLEEP_TIMER == 1))
  RSI_PS_ClrWkpSources(SYSRTC_BASED_WAKEUP);
#endif
  // If any error code, it returns it otherwise goes to sleep without retention.
  trigger_sleep(&config, SLEEP_WITHOUT_RETENTION);
}

/*******************************************************************************
 * State Change from PS3 to PS4.
 ******************************************************************************/
static void ps3_to_ps4_state_change(void)
{
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_PS4,
                                                   sl_si91x_power_manager_get_clock_scaling());
}

/*******************************************************************************
 * State Change from PS3 to PS2.
 * Hardware is configured for low power mode.
 * LDO and other registers are taken care by RSI api.
 ******************************************************************************/
static void ps3_to_ps2_state_change(void)
{
  // Low power hardware configuration to switch off the components which are not required.
  sli_si91x_power_manager_low_power_hw_config(false);
  // Change to 20MHz-RC to be used as Processor Clock in PS2 state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_PS2,
                                                   sl_si91x_power_manager_get_clock_scaling());
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1) \
  && (SLI_SI91X_MCU_COMMON_FLASH_MODE == 1)
  // Turn off XTAL
  sli_si91x_xtal_turn_off_request_from_m4_to_TA();
  // Reset M4_USING_FLASH bit
  M4SS_P2P_INTR_CLR_REG = M4_USING_FLASH;
  // Set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctr, AON domain power supply controls from M4 to NWP
  MCUAON_CONTROL_REG4 |= (MCU_TASS_REF_CLK_SEL_MUX_CTRL);
  MCUAON_CONTROL_REG4;
#else
  // Disable 40MHz clock in case PS2
  RSI_ULPSS_DisableRefClks(MCU_ULP_40MHZ_CLK_EN);
#endif
  // Switching from PS3 to PS2 state
  RSI_PS_PowerStateChangePs4toPs2(ULP_MCU_MODE,
                                  PWR_MUX_SEL_ULPSSRAM_SCDC_0_9,
                                  PWR_MUX_SEL_M4_ULP_RAM_SCDC_0_9,
                                  PWR_MUX_SEL_M4_ULP_RAM16K_SCDC_0_9,
                                  PWR_MUX_SEL_M4ULP_SCDC_0_6,
                                  PWR_MUX_SEL_ULPSS_SCDC_0_9,
                                  DISABLE_BG_SAMPLE_ENABLE,
                                  DISABLE_DC_DC_ENABLE,
                                  DISABLE_SOCLDO_ENABLE,
                                  DISABLE_STANDBYDC,
                                  DISABLE_TA192K_RAM_RET,
                                  DISABLE_M464K_RAM_RET);
}

/*******************************************************************************
 * State Change from PS3 to PS0 (Sleep without retention).
 * It takes macro from the sli_si91x_power_manager_board_config.h file where
 * stack address, callback address, vector offset are configured according to the
 * board.
 * It switches off the components which are not required and goes to sleep without retention.
 ******************************************************************************/
static void ps3_to_ps0_state_change(void)
{
  sli_power_sleep_config_t config;
  config.low_freq_clock          = SLI_SI91X_POWER_MANAGER_DISABLE_LF_MODE;
  config.mode                    = SLI_SI91X_POWER_MANAGER_WAKEUP_WITHOUT_RETENTION;
  config.stack_address           = SL_SLEEP_STACK_USAGE_ADDRESS;
  config.vector_offset           = SL_SLEEP_VECTOR_OFFSET;
  config.wakeup_callback_address = SL_SLEEP_WAKEUP_CALLBACK_ADDRESS;
  sli_si91x_power_manager_low_power_hw_config(true);
#ifdef SL_SI91X_POWER_MANAGER_UC_AVAILABLE
  sl_status_t status_ps3_to_ps0 = SL_STATUS_OK;
  // Initializing and configuring the wakeup sources as per UC inputs, if available
  status_ps3_to_ps0 = sl_si91x_power_manager_wakeup_init();
  if (status_ps3_to_ps0 != SL_STATUS_OK) {
    DEBUGOUT("Error Code: 0x%lX, Power State Transition Failed \n", status_ps3_to_ps0);
    return;
  }
#endif
  // Configuring the clocks as per the sleep state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_SLEEP, SL_SI91X_POWER_MANAGER_POWERSAVE);
#if ((SL_SI91X_TICKLESS_MODE == 1) && (SL_SLEEP_TIMER == 1))
  RSI_PS_ClrWkpSources(SYSRTC_BASED_WAKEUP);
#endif
  trigger_sleep(&config, SLEEP_WITHOUT_RETENTION);
}

/*******************************************************************************
 * State Change from PS2 to PS4.
 * LDO and other registers are taken care by RSI api.
 ******************************************************************************/
static void ps2_to_ps4_state_change(void)
{
  ps_power_state_change_ps2_to_Ps4(PMU_WAIT_TIME, LDO_WAIT_TIME);
#if !(SLI_WIRELESS_COMPONENT_PRESENT)
  // Enable 40MHz XTAL clock
  RSI_ULPSS_EnableRefClks(MCU_ULP_40MHZ_CLK_EN, ULP_PERIPHERAL_CLK, 0);
#endif
  //Disable the NVIC interrupts.
  __asm volatile("cpsid i" ::: "memory");
  __asm volatile("dsb");
  __asm volatile("isb");
  // To initialize the flash
  initialize_flash();
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1) \
  && (SLI_SI91X_MCU_COMMON_FLASH_MODE == 1)
  // Set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl ,AON domain power supply controls from NWP to M4
  RSI_Set_Cntrls_To_M4();
  // Poll for flash magic word
  while (MBR_MAGIC_WORD != 0x5A5A)
    ;
#endif
  // Configuring the clocks as per the PS4 state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_PS4,
                                                   sl_si91x_power_manager_get_clock_scaling());
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
  // Clear M4 wakeup NWP bit
  sl_si91x_host_clear_sleep_indicator();
  // Enable the P2P interrupt IRQ
  NVIC_SetPriority(TASS_P2P_IRQn, TASS_P2P_INTR_PRI);
  if (!NVIC_GetEnableIRQ(TASS_P2P_IRQn)) {
    // Set the P2P interrupt priority
    NVIC_EnableIRQ(TASS_P2P_IRQn);
  }
  // Configure the NWP and M4 status registers to ensure the system resumes normal operation.
  sli_si91x_m4_ta_wakeup_configurations();
#endif
  // Enable the NVIC interrupts.
  __asm volatile("cpsie i" ::: "memory");
}

/*******************************************************************************
 * State Change from PS2 to PS3.
 * LDO and other registers are taken care by RSI api.
 ******************************************************************************/
static void ps2_to_ps3_state_change(void)
{
  ps_power_state_change_ps2_to_Ps4(PMU_WAIT_TIME, LDO_WAIT_TIME);
#if !(SLI_WIRELESS_COMPONENT_PRESENT)
  // Enable 40MHz XTAL clock
  RSI_ULPSS_EnableRefClks(MCU_ULP_40MHZ_CLK_EN, ULP_PERIPHERAL_CLK, 0);
#endif
  //Disable the NVIC interrupts.
  __asm volatile("cpsid i" ::: "memory");
  __asm volatile("dsb");
  __asm volatile("isb");
  // To initialize the flash
  initialize_flash();
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1) \
  && (SLI_SI91X_MCU_COMMON_FLASH_MODE == 1)
  // Set m4ss_ref_clk_mux_ctrl ,tass_ref_clk_mux_ctrl ,AON domain power supply controls from NWP to M4
  RSI_Set_Cntrls_To_M4();
  // Poll for flash magic word
  while (MBR_MAGIC_WORD != 0x5A5A)
    ;
#endif
  // Configuring the clocks as per the PS3 state
  sli_si91x_clock_manager_config_clks_on_ps_change(SL_SI91X_POWER_MANAGER_PS3,
                                                   sl_si91x_power_manager_get_clock_scaling());
#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
  // Clear M4 wakeup NWP bit
  sl_si91x_host_clear_sleep_indicator();
  // Enable the P2P interrupt IRQ
  NVIC_SetPriority(TASS_P2P_IRQn, TASS_P2P_INTR_PRI);
  if (!NVIC_GetEnableIRQ(TASS_P2P_IRQn)) {
    // Set the P2P interrupt priority
    NVIC_EnableIRQ(TASS_P2P_IRQn);
  }
  // Configure the NWP and M4 status registers to ensure the system resumes normal operation.
  sli_si91x_m4_ta_wakeup_configurations();
#endif
  // Enable the NVIC interrupts.
  __asm volatile("cpsie i" ::: "memory");
}

/*******************************************************************************
 * State Change from PS2 to PS1 (Sleep with retention).
 * It takes macro from the sli_si91x_power_manager_board_config.h file where
 * stack address, callback address, vector offset are configured according to the
 * board.
 * It switches off the components which are not required and goes to sleep with retention.
 * If the wakeup sources are configured as ULPSS or SDCSS then only it will transit to PS1.
 ******************************************************************************/
static void ps2_to_ps1_state_change(void)
{
  uint32_t get_wakeup_sources;
  get_wakeup_sources = RSI_PS_GetWkpSources();
  if ((get_wakeup_sources & SL_SI91X_POWER_MANAGER_ULPSS_WAKEUP)
      || (get_wakeup_sources & SL_SI91X_POWER_MANAGER_SDCSS_WAKEUP)) {

    sli_power_sleep_config_t config;
    // Configuring MCU-ULP Reference Clock source to be used for AHB Interface in ULP Mode
    config.low_freq_clock          = SLI_SI91X_POWER_MANAGER_DISABLE_LF_MODE;
    config.stack_address           = SL_SLEEP_RAM_USAGE_ADDRESS;
    config.vector_offset           = SL_SLEEP_VECTOR_OFFSET_WITH_RETENTION;
    config.wakeup_callback_address = JUMP_CB_ADDRESS;
    config.mode                    = SLI_SI91X_POWER_MANAGER_WAKEUP_WITH_RETENTION;

    // If any error code, it returns it otherwise goes to sleep with retention.
    trigger_sleep(&config, SLEEP_WITH_RETENTION);
#if (SL_SI91X_TICKLESS_MODE == 0)
    // Enable the NVIC interrupts.
    __asm volatile("cpsie i" ::: "memory");
#endif
  }
}

/*******************************************************************************
 * Trigger sleep as per the parameters passed in the config structure and sleep type.
 * RSI APIs handles the retention as per parameters and enters deep sleep mode.
 * If any error is encountered, error code is returned.
 ******************************************************************************/
static sl_status_t trigger_sleep(sli_power_sleep_config_t *config, uint8_t sleep_type)
{
  sl_status_t status;
  rsi_error_t error_code;
  if (config == NULL) {
    // Validates config, if null returns error code
    return SL_STATUS_NULL_POINTER;
  }
#if SL_WIFI_COMPONENT_INCLUDED
  if (!(M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)) {
    // Turn on SOC_PLL by default, as the next ROM API depends on the SOC PLL clock,
    // and the SOC PLL is typically turned off before entering sleep mode.
    sl_si91x_clock_manager_control_pll(SOC_PLL, ENABLE);
  }
#endif
  // Stack address, wakeup callback address, vector offset and mode is configured in it.
  RSI_PS_RetentionSleepConfig(config->stack_address,
                              config->wakeup_callback_address,
                              config->vector_offset,
                              config->mode);
#if SL_WIFI_COMPONENT_INCLUDED
  /* Check's if SOC is in PS2 state. If so, skip writing to PLL registers as they are unavailable in this state. */
  if (!(M4_ULP_SLP_STATUS_REG & ULP_MODE_SWITCHED_NPSS)) {
    /* To prevent an open-loop PLL clock condition, ensure the PLL's are turned off before turning off the XTAL (PLL source). */
    /* TurnOff the SOC_PLL */
    sl_si91x_clock_manager_control_pll(SOC_PLL, DISABLE);
    /* TurnOff the INTF_PLL */
    sl_si91x_clock_manager_control_pll(INTF_PLL, DISABLE);
    /* TurnOff the I2S_PLL */
    sl_si91x_clock_manager_control_pll(I2S_PLL, DISABLE);
  }
#endif
  // According to the sleep type, with retention or without retention it enters the sleep mode.
  error_code = RSI_PS_EnterDeepSleep(sleep_type, config->low_freq_clock);
  // If error is encountered, it is converted to sl error code.
  status = convert_rsi_to_sl_error_code(error_code);
  return status;
}

/*******************************************************************************
 * As per the memory passed, it sets the bits of RAM banks in m4ss and ulpss 
 * registers and updates it.
 ******************************************************************************/
static sl_status_t configure_ram_memory(sl_power_ram_retention_config_t *config,
                                        uint32_t *m4ss_ram,
                                        uint32_t *ulpss_ram)
{
  if ((config == NULL) || (m4ss_ram == NULL) || (ulpss_ram == NULL)) {
    // Validates config, m4ss_ram and ulpss_ram, if null pointer returns error code.
    return SL_STATUS_NULL_POINTER;
  }
  if (config->m4ss_ram_size_kb < RAM_4_KB) {
    // If M4SS RAM Size is less than 4 KB, bit 0 is set.
    *m4ss_ram |= RAM_BANK_0;
  }
  if (config->m4ss_ram_size_kb < RAM_8_KB) {
    // If M4SS RAM Size is less than 8 KB, bit 1 is set.
    *m4ss_ram |= RAM_BANK_1;
  }
  if (config->m4ss_ram_size_kb < RAM_12_KB) {
    // If M4SS RAM Size is less than 12 KB, bit 2 is set.
    *m4ss_ram |= RAM_BANK_2;
  }
  if (config->m4ss_ram_size_kb < RAM_16_KB) {
    // If M4SS RAM Size is less than 16 KB, bit 3 is set.
    *m4ss_ram |= RAM_BANK_3;
  }
  if (config->m4ss_ram_size_kb < RAM_32_KB) {
    // If M4SS RAM Size is less than 32 KB, bit 4 is set.
    *m4ss_ram |= RAM_BANK_4;
  }
  if (config->m4ss_ram_size_kb < RAM_64_KB) {
    // If M4SS RAM Size is less than 64 KB, bit 5 is set.
    *m4ss_ram |= RAM_BANK_5;
  }
  if (config->m4ss_ram_size_kb < RAM_128_KB) {
    // If M4SS RAM Size is less than 128 KB, bit 6 is set.
    *m4ss_ram |= RAM_BANK_6;
  }
  if (config->m4ss_ram_size_kb < RAM_192_KB) {
    // If M4SS RAM Size is less than 192 KB, bit 7 is set.
    *m4ss_ram |= RAM_BANK_7;
  }
#if (SL_SI91X_SI917_RAM_MEM_CONFIG == 2) || (SL_SI91X_SI917_RAM_MEM_CONFIG == 3)
  if (config->m4ss_ram_size_kb < RAM_256_KB) {
    // If M4SS RAM Size is less than 256 KB, bit 8 is set.
    *m4ss_ram |= RAM_BANK_8;
  }
#endif
#if (SL_SI91X_SI917_RAM_MEM_CONFIG == 3)
  if (config->m4ss_ram_size_kb < RAM_320_KB) {
    // If M4SS RAM Size is less than 320 KB, bit 9 is set.
    *m4ss_ram |= RAM_BANK_9;
  }
#endif
  if (config->ulpss_ram_size_kb < RAM_2_KB) {
    // If ULPSS RAM Size is less than 2 KB, bit 1 is set.
    *ulpss_ram |= ULPSS_2K_BANK_1;
  }
  if (config->ulpss_ram_size_kb < RAM_4_KB) {
    // If ULPSS RAM Size is less than 4 KB, bit 2 is set.
    *ulpss_ram |= ULPSS_2K_BANK_2;
  }
  return SL_STATUS_OK;
}

static void initialize_flash(void)
{
  RSI_PS_M4ssPeriPowerUp(M4SS_PWRGATE_ULP_QSPI_ICACHE | M4SS_PWRGATE_ULP_EFUSE_PERI);
  RSI_PS_QspiDllDomainEnable();
  RSI_PS_FlashLdoEnable();

#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1)
  RSI_PS_WakeupTAandProgramFlash();
#endif

#ifdef SLI_SI91X_MCU_COMMON_FLASH_MODE
  RSI_FLASH_Initialize();
#endif

#if defined(SLI_WIRELESS_COMPONENT_PRESENT) && (SLI_WIRELESS_COMPONENT_PRESENT == 1) \
  && (defined SLI_SI91X_MCU_COMMON_FLASH_MODE)
  // Indicate M4 is using flash
  M4SS_P2P_INTR_SET_REG = M4_USING_FLASH;
#endif
}

static void get_ram_retention_mode(uint32_t m4ss_ram, uint32_t *m4ss_ram_retention)
{
  if ((m4ss_ram & RAM_BANK_0) || (m4ss_ram & RAM_BANK_1) || (m4ss_ram & RAM_BANK_2) || (m4ss_ram & RAM_BANK_3)) {
    *m4ss_ram_retention |= M4ULP_RAM16K_RETENTION_MODE_EN;
  }
  if ((m4ss_ram & RAM_BANK_4) || (m4ss_ram & RAM_BANK_5) || (m4ss_ram & RAM_BANK_6) || (m4ss_ram & RAM_BANK_7)
      || (m4ss_ram & RAM_BANK_8) || (m4ss_ram & RAM_BANK_9)) {
    *m4ss_ram_retention |= M4ULP_RAM_RETENTION_MODE_EN;
  }
}

/*******************************************************************************
 * To validate the RSI error code
 * While calling the RSI APIs, it returns the RSI Error codes.
 * This function converts the RSI error codes into SL error codes.
 * It takes argument as RSI error type and returns the SL error type.
 * It has a single switch statement which is mapped with the SL error code and
 * after successful conversion it breaks the switch statement.
 * If the error code is not listed, by default is SL_STATUS_FAIL.
 ******************************************************************************/
static sl_status_t convert_rsi_to_sl_error_code(rsi_error_t error)
{
  sl_status_t status;
  switch (error) {
    case RSI_OK:
      status = SL_STATUS_OK;
      break;
    case INVALID_PARAMETERS:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_INVALID_INPUT_FREQUENCY:
      status = SL_STATUS_INVALID_PARAMETER;
      break;
    case ERROR_CLOCK_NOT_ENABLED:
      status = SL_STATUS_NOT_INITIALIZED;
      break;
    default:
      status = SL_STATUS_FAIL;
      break;
  }
  return status;
}
