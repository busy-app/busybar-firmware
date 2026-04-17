/***************************************************************************//**
 * @file
 * @brief Energy management unit (EMU) peripheral API
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

#include "sl_hal_emu.h"

#if defined(EMU_PRESENT)

#include <stddef.h>
#include "sl_common.h"
#include "sl_assert.h"
#include "sl_hal_bus.h"

/*******************************************************************************
 ******************************   DEFINES   ************************************
 ******************************************************************************/

/* Used to figure out if a memory address is inside or outside of a RAM block.
 * A memory address is inside a RAM block if the address is greater than the
 * RAM block address. */
#define ADDRESS_NOT_IN_BLOCK(addr, block)  ((addr) <= (block) ? 1UL : 0UL)

/* RAM Block layout for various device families. Note that some devices
 * have special layout in RAM0 and some devices have a special RAM block
 * at the end of their block layout. */
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
#define RAM0_BLOCKS            6U
#define RAM0_BLOCK_SIZE   0x4000U // 16 kB blocks
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
#define RAM0_BLOCKS            4U
#define RAM0_BLOCK_SIZE   0x4000U // 16 kB blocks
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
#define RAM0_BLOCKS            4U
#define RAM0_BLOCK_SIZE   0x4000U // 16 kB blocks
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4)
#define RAM0_BLOCKS           16U
#define RAM0_BLOCK_SIZE   0x4000U // 16 kB blocks
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
#define RAM0_BLOCKS           16U
#define RAM0_BLOCK_SIZE   0x8000U // 32 kB blocks
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)
#define RAM0_BLOCKS           32U
#define RAM0_BLOCK_SIZE   0x4000U // 16 kB blocks
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8) \
  || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9)
#define RAM0_BLOCKS           16U
#define RAM0_BLOCK_SIZE   0x4000U // 16 kB blocks
#endif

#if defined(RAM_MEM_END)
#define RAM0_END    RAM_MEM_END
#endif

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) \
  || defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
// EMU DCDC MODE set timeout.
#define EMU_DCDC_MODE_SET_TIMEOUT           1000000
#endif

/*******************************************************************************
 **************************   LOCAL VARIABLES   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

extern __INLINE void sl_hal_emu_lock(void);
extern __INLINE void sl_hal_emu_unlock(void);
extern __INLINE void sl_hal_emu_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_emu_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_emu_clear_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_emu_get_pending_interrupts(void);
extern __INLINE uint32_t sl_hal_emu_get_enabled_pending_interrupts(void);
extern __INLINE void sl_hal_emu_set_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_emu_get_status(void);
extern __INLINE bool sl_hal_emu_is_temperature_ready(void);
#if defined(_EMU_TEMP_TEMPAVG_MASK)
extern __INLINE float sl_hal_emu_get_averaged_temperature(void);
extern __INLINE void sl_hal_emu_request_averaged_temperature(sl_hal_emu_temperature_average_num_t num_samples);
#endif
extern __INLINE float sl_hal_emu_get_temperature(void);
extern __INLINE void sl_hal_emu_clear_reset_cause(void);
extern __INLINE uint32_t sl_hal_emu_get_reset_cause(void);
extern __INLINE void sl_hal_emu_set_reset_control(sl_hal_emu_reset_source_t reset,
                                                  sl_hal_emu_reset_mode_t mode);
#if defined(_EMU_CTRL_HDREGEM2EXITCLIM_MASK)
extern __INLINE void sl_hal_emu_enable_hdreg_em2_exit_current_limit(void);
extern __INLINE void sl_hal_emu_disable_hdreg_em2_exit_current_limit(void);
#endif
#if defined(_EMU_CTRL_HDREGSTOPGEAR_MASK)
extern __INLINE void sl_hal_emu_set_hdreg_stop_gear(sl_hal_emu_hdreg_stop_gear_current_limit_t current);
#endif
#if defined(_EMU_FLREG_MASK)
extern __INLINE void sl_hal_emu_enable_flash_regulator(void);
extern __INLINE void sl_hal_emu_disable_flash_regulator(void);
#endif

#if defined(_EMU_EFPIF_MASK)
extern __INLINE void sl_hal_emu_efp_disable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_emu_efp_enable_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_emu_efp_get_pending_interrupts(void);
extern __INLINE void sl_hal_emu_efp_set_interrupts(uint32_t flags);
extern __INLINE void sl_hal_emu_efp_clear_interrupts(uint32_t flags);
extern __INLINE uint32_t sl_hal_emu_efp_get_enabled_pending_interrupts(void);
extern __INLINE void sl_hal_emu_enable_efp_drive_dvdd(void);
extern __INLINE void sl_hal_emu_disable_efp_drive_dvdd(void);
extern __INLINE void sl_hal_emu_enable_efp_direct_mode(void);
extern __INLINE void sl_hal_emu_disable_efp_direct_mode(void);
extern __INLINE void sl_hal_emu_enable_efp_drive_decouple(void);
extern __INLINE void sl_hal_emu_disable_efp_drive_decouple(void);
#endif /* defined(_EMU_EFPIF_MASK) */
#if defined(_DCDC_LOCK_MASK)
extern __INLINE void sl_hal_emu_dcdc_lock(void);
extern __INLINE void sl_hal_emu_dcdc_unlock(void);
#endif

#if defined(_DCDC_SYNCBUSY_MASK)
extern __INLINE void sl_hal_emu_dcdc_sync(uint32_t mask);
#endif

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
extern __INLINE void sl_hal_emu_enable_boost_external_shutdown(void);
extern __INLINE void sl_hal_emu_disable_boost_external_shutdown(void);
#endif

#if defined(_DCDC_EN_EN_MASK)
extern __INLINE void sl_hal_emu_enable_dcdc(void);
extern __INLINE void sl_hal_emu_disable_dcdc(void);
#endif

#if defined(_DCDC_IEN_MASK)
extern __INLINE void sl_hal_emu_dcdc_enable_interrupts(uint32_t flags);
extern __INLINE void sl_hal_emu_dcdc_disable_interrupts(uint32_t flags);
#endif

/***************************************************************************//**
 *   Power down RAM memory blocks.
 ******************************************************************************/
void sl_hal_emu_ram_power_down(uint32_t start,
                               uint32_t end)
{
#if defined(_SYSCFG_DMEM0RETNCTRL_MASK)
  uint32_t mask = 0;
  (void) start;

  if (end == 0U) {
    end = SRAM_BASE + SRAM_SIZE;
  }

  // Check to see if something in RAM0 can be powered down.
#if defined(RAM_MEM_END)
  if (end >= RAM0_END) {
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
    // Lynx has 2 blocks. We do no shut off block 0 because we dont want to disable all RAM0.
    mask |= ADDRESS_NOT_IN_BLOCK(start, 0x20006000UL) << 1; // Block 1, 8 kB.
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)
    // Leopard has 3 blocks. We do no shut off block 0 because we dont want to disable all RAM0.
    mask |= ADDRESS_NOT_IN_BLOCK(start, 0x20006000UL) << 1; // Block 1, 8 kB.
    mask |= ADDRESS_NOT_IN_BLOCK(start, 0x20008000UL) << 2; // Block 2, 32 kB.
#elif defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)  \
    || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_8) \
    || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_9) \
    || defined(_SILICON_LABS_32B_SERIES_2_CONFIG_13)
    // These platforms have equally-sized RAM blocks and block 0 can be powered down but should not.
    // This condition happens when the block 0 disable bit flag is available in the retention control register.
    for (unsigned i = 1; i < RAM0_BLOCKS; i++) {
      mask |= ADDRESS_NOT_IN_BLOCK(start, RAM_MEM_BASE + (i * RAM0_BLOCK_SIZE)) << (i);
    }
#elif defined(RAM0_BLOCKS)
    // These platforms have equally-sized RAM blocks and block 0 cannot be powered down.
    for (unsigned i = 1; i < RAM0_BLOCKS; i++) {
      mask |= ADDRESS_NOT_IN_BLOCK(start, RAM_MEM_BASE + (i * RAM0_BLOCK_SIZE)) << (i - 1U);
    }
#endif
  }
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_7)
  else if (end > 0x20006000UL) {
    // Leopard has 3 blocks. We do no shut off block 0 because we dont want to disable all RAM0.
    mask |= ADDRESS_NOT_IN_BLOCK(start, 0x20006000UL) << 1; // Block 1, 8 kB.
  }
#endif
#endif

  // Power down the selected blocks.
#if defined(CMU_CLKEN0_SYSCFG)
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
#endif
  sl_hal_syscfg_mask_dmem0retnctrl(mask);
#else
  // These devices are unable to power down RAM blocks.
  (void) start;
  (void) end;
#endif
}

/***************************************************************************//**
 *   Power up all available RAM memory blocks.
 ******************************************************************************/
void sl_hal_emu_ram_power_up(void)
{
#if defined(_SYSCFG_DMEM0RETNCTRL_MASK)
#if defined(CMU_CLKEN0_SYSCFG)
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
#endif
  sl_hal_syscfg_zero_dmem0retnctrl();
#endif
}

#if (defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) \
  || defined(SL_HAL_EMU_DCDC_BOOST_PRESENT))
/***************************************************************************//**
 * Set DCDC regulator operating mode.
 ******************************************************************************/
sl_status_t sl_hal_emu_set_dcdc_mode(sl_hal_emu_dcdc_mode_t dcdc_mode)
{
  uint32_t current_dcdc_mode;
  sl_status_t error = SL_STATUS_OK;
  uint32_t timeout = 0;

  if (dcdc_mode == SL_HAL_EMU_DCDC_MODE_BYPASS) {
#if defined(_DCDC_SYNCBUSY_MASK)
#if defined(_DCDC_SYNCBUSY_CTRL_MASK)
    sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_CTRL);
#else
    sl_hal_emu_dcdc_sync(_DCDC_SYNCBUSY_MASK);
#endif
#endif
    current_dcdc_mode = (DCDC->CTRL & _DCDC_CTRL_MODE_MASK) >> _DCDC_CTRL_MODE_SHIFT;

    if (current_dcdc_mode != SL_HAL_EMU_DCDC_MODE_BYPASS) {
      // Switch to BYPASS mode if it is not the current mode.
      DCDC->CTRL_CLR = DCDC_CTRL_MODE;
#if defined(_DCDC_DOCTRL_MASK)
      // Dual-output DCDC: wait for RUNNING clear and BYPSW set (fixes DECOUPLE→bypass).
      while ((((DCDC->STATUS & _DCDC_STATUS_RUNNING_MASK) != 0U)
              || ((DCDC->STATUS & _DCDC_STATUS_BYPSW_MASK) == 0U))
             && (timeout < EMU_DCDC_MODE_SET_TIMEOUT)) {
        timeout++;
      }
#else
      // Single-output DCDC: wait for BYPASS switch to be enabled.
      while (((DCDC->STATUS & _DCDC_STATUS_BYPSW_MASK) == 0U) && (timeout < EMU_DCDC_MODE_SET_TIMEOUT)) {
        timeout++;
      }
#endif
      if (timeout >= EMU_DCDC_MODE_SET_TIMEOUT) {
        error = SL_STATUS_TIMEOUT;
      }
    }
#if defined(_DCDC_EN_EN_MASK)
    sl_hal_emu_disable_dcdc();
#endif
  } else {
    while (((DCDC->STATUS & _DCDC_STATUS_VREGIN_MASK) != 0U) && (timeout < EMU_DCDC_MODE_SET_TIMEOUT)) {
      // Wait for VREGIN voltage to rise above threshold.
      timeout++;
    }
    if (timeout >= EMU_DCDC_MODE_SET_TIMEOUT) {
      error = SL_STATUS_TIMEOUT;
    } else {
      DCDC->IF_CLR = DCDC_IF_REGULATION;
      DCDC->CTRL_SET = DCDC_CTRL_MODE;
      timeout = 0;
      while (((DCDC->IF & _DCDC_IF_REGULATION_MASK) == 0U) && (timeout < EMU_DCDC_MODE_SET_TIMEOUT)) {
        // Wait for DCDC to complete it's startup.
        timeout++;
      }
      if (timeout >= EMU_DCDC_MODE_SET_TIMEOUT) {
        error = SL_STATUS_TIMEOUT;
      }
    }
  }

  sl_hal_emu_dcdc_updated_hook();

  return error;
}

/***************************************************************************//**
 * Indicate that the DCDC peripheral bus clock enable has changed allowing
 * RAIL to react accordingly.
 *
 * @details This function is called after DCDC has been enabled or disabled.
 *          The function implementation does not perform anything, but it is
 *          SL_WEAK so that it can use the RAIL version if needed.
 ******************************************************************************/
SL_WEAK void sl_hal_emu_dcdc_updated_hook(void)
{
  // This implementation is empty, but this function can be redefined as it's a weak implementation.
}
#endif /* (defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) || defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)) */

#if defined(SL_HAL_EMU_DCDC_BOOST_PRESENT)
/***************************************************************************//**
 * Configure the DCDC Boost regulator.
 ******************************************************************************/
void sl_hal_emu_init_dcdc_boost(const sl_hal_emu_dcdc_boost_init_t *init)
{
  EFM_ASSERT(init != NULL);

#if defined(_DCDC_SYNCBUSY_MASK)
  sl_hal_emu_dcdc_sync(_DCDC_SYNCBUSY_MASK);
#endif

#if defined(_DCDC_CTRL_DVDDBSTPRG_MASK)
  sl_hal_bus_reg_write_mask(&DCDC->CTRL,
                            _DCDC_CTRL_DVDDBSTPRG_MASK,
                            ((uint32_t)init->output_voltage << _DCDC_CTRL_DVDDBSTPRG_SHIFT));
#endif

  DCDC->BSTCTRL     = (DCDC->BSTCTRL & ~(_DCDC_BSTCTRL_IPKTMAXCTRL_MASK))
                      | ((uint32_t)init->ton_max << _DCDC_BSTCTRL_IPKTMAXCTRL_SHIFT);
  DCDC->BSTEM01CTRL = ((uint32_t)init->drive_speed_em01 << _DCDC_BSTEM01CTRL_DRVSPEED_SHIFT)
                      | ((uint32_t)init->peak_current_em01 << _DCDC_BSTEM01CTRL_IPKVAL_SHIFT);
  DCDC->BSTEM23CTRL = ((uint32_t)init->drive_speed_em23 << _DCDC_BSTEM23CTRL_DRVSPEED_SHIFT)
                      | ((uint32_t)init->peak_current_em23 << _DCDC_BSTEM23CTRL_IPKVAL_SHIFT);

  if (init->external_shutdown_enable) {
    sl_hal_emu_enable_boost_external_shutdown();
  } else {
    sl_hal_emu_disable_boost_external_shutdown();
  }

  sl_hal_emu_set_dcdc_mode(SL_HAL_EMU_DCDC_MODE_REGULATION);

  sl_hal_emu_dcdc_updated_hook();
}

/***************************************************************************//**
 * Set EM01 mode Boost Peak Current setting.
 ******************************************************************************/
void sl_hal_emu_set_em01_boost_peak_current(const sl_hal_emu_dcdc_boost_em01_peak_current_t boost_peak_current_em01)
{
  // Wait for synchronization before writing new value.
#if defined(_DCDC_SYNCBUSY_MASK)
  sl_hal_emu_dcdc_sync(_DCDC_SYNCBUSY_MASK);
#endif

  sl_hal_bus_reg_write_mask(&DCDC->BSTEM01CTRL,
                            _DCDC_BSTEM01CTRL_IPKVAL_MASK,
                            ((uint32_t)boost_peak_current_em01 << _DCDC_BSTEM01CTRL_IPKVAL_SHIFT));

  sl_hal_emu_dcdc_updated_hook();
}

#if defined(_DCDC_CTRL_DVDDBSTPRG_MASK)
/***************************************************************************//**
 * Set DCDC Boost output voltage.
 ******************************************************************************/
void sl_hal_emu_set_dcdc_boost_output_voltage(const sl_hal_emu_dcdc_boost_output_voltage_t boost_output_voltage)
{
  // Wait for synchronization before writing new value.
#if defined(_DCDC_SYNCBUSY_MASK)
  sl_hal_emu_dcdc_sync(_DCDC_SYNCBUSY_MASK);
#endif

  sl_hal_bus_reg_write_mask(&DCDC->CTRL,
                            _DCDC_CTRL_DVDDBSTPRG_MASK,
                            ((uint32_t)boost_output_voltage << _DCDC_CTRL_DVDDBSTPRG_SHIFT));

  sl_hal_emu_dcdc_updated_hook();
}
#endif
#endif /* defined(SL_HAL_EMU_DCDC_BOOST_PRESENT) */

#if defined(SL_HAL_EMU_DCDC_BUCK_PRESENT)
/***************************************************************************//**
 * Configure the DCDC regulator.
 ******************************************************************************/
void sl_hal_emu_init_dcdc(const sl_hal_emu_dcdc_init_t *init)
{
  EFM_ASSERT(init != NULL);

  EMU->VREGVDDCMPCTRL = ((uint32_t)init->comparator_threshold << _EMU_VREGVDDCMPCTRL_THRESSEL_SHIFT)
                        | EMU_VREGVDDCMPCTRL_VREGINCMPEN;

#if defined(_DCDC_SYNCBUSY_MASK)
#if defined(_DCDC_SYNCBUSY_CTRL_MASK)
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_CTRL | DCDC_SYNCBUSY_EM01CTRL0 | DCDC_SYNCBUSY_EM23CTRL0);
#else
  sl_hal_emu_dcdc_sync(_DCDC_SYNCBUSY_MASK);
#endif
#endif
#if defined(_DCDC_CTRL_DCMONLYEN_MASK)
  DCDC->CTRL = (DCDC->CTRL & ~(_DCDC_CTRL_IPKTMAXCTRL_MASK
                               | _DCDC_CTRL_DCMONLYEN_MASK))
               | ((uint32_t)init->ton_max << _DCDC_CTRL_IPKTMAXCTRL_SHIFT)
               | ((uint32_t)(init->dcm_only_enable) << _DCDC_CTRL_DCMONLYEN_SHIFT);
#else
  DCDC->CTRL = (DCDC->CTRL & ~_DCDC_CTRL_IPKTMAXCTRL_MASK)
               | ((uint32_t)init->ton_max << _DCDC_CTRL_IPKTMAXCTRL_SHIFT);
#endif
  DCDC->EM01CTRL0 = ((uint32_t)init->drive_speed_em01 << _DCDC_EM01CTRL0_DRVSPEED_SHIFT)
                    | ((uint32_t)init->peak_current_em01 << _DCDC_EM01CTRL0_IPKVAL_SHIFT)
#if defined(_DCDC_EM01CTRL0_IPKDECVAL_MASK) && !defined(_DCDC_DOCTRL_MASK)
                    | DCDC_EM01CTRL0_IPKDECVAL_DEFAULT
#endif
  ;
  DCDC->EM23CTRL0 = ((uint32_t)init->drive_speed_em23 << _DCDC_EM23CTRL0_DRVSPEED_SHIFT)
                    | ((uint32_t)init->peak_current_em23 << _DCDC_EM23CTRL0_IPKVAL_SHIFT)
#if defined(_DCDC_EM23CTRL0_IPKDECVAL_MASK)
                    | DCDC_EM23CTRL0_IPKDECVAL_DEFAULT
#endif
  ;

#if defined(_DCDC_DOCTRL_MASK)
  /* Reduce IPKDECVAL to 1 before transition. */
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_EM01CTRL0);
  DCDC->EM01CTRL0 = (DCDC->EM01CTRL0 & ~_DCDC_EM01CTRL0_IPKDECVAL_MASK)
                    | (1U << _DCDC_EM01CTRL0_IPKDECVAL_SHIFT);
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_EM01CTRL0);
#endif

  /* Set DCDC regulation type. */
#if defined(_DCDC_DOCTRL_REGULATIONTYPE_MASK)
  sl_hal_emu_dcdc_set_regulation_type(init->regulation_type);
#endif

  sl_hal_emu_set_dcdc_mode(init->mode);

#if defined(_DCDC_DOCTRL_MASK)
  /* Restore IPKDECVAL to default value after transition */
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_EM01CTRL0);
  DCDC->EM01CTRL0 = (DCDC->EM01CTRL0 & ~_DCDC_EM01CTRL0_IPKDECVAL_MASK)
                    | DCDC_EM01CTRL0_IPKDECVAL_DEFAULT;
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_EM01CTRL0);
#endif

  sl_hal_emu_dcdc_updated_hook();
}

/***************************************************************************//**
 * Power off the DCDC regulator.
 ******************************************************************************/
sl_status_t sl_hal_emu_dcdc_power_off(void)
{
  sl_status_t status;

  status = sl_hal_emu_set_dcdc_mode(SL_HAL_EMU_DCDC_MODE_BYPASS);

  return status;
}

/***************************************************************************//**
 * Set EMO1 mode Peak Current setting.
 ******************************************************************************/
void sl_hal_emu_set_em01_peak_current(const sl_hal_emu_dcdc_peak_current_t peak_current_em01)
{
  // Wait for synchronization before writing new value.
#if defined(_DCDC_SYNCBUSY_MASK)
  sl_hal_emu_dcdc_sync(_DCDC_SYNCBUSY_MASK);
#endif

  sl_hal_bus_reg_write_mask(&DCDC->EM01CTRL0,
                            _DCDC_EM01CTRL0_IPKVAL_MASK,
                            ((uint32_t)peak_current_em01 << _DCDC_EM01CTRL0_IPKVAL_SHIFT));

  sl_hal_emu_dcdc_updated_hook();
}

#if defined(_DCDC_PFMXCTRL_IPKVAL_MASK)
/***************************************************************************//**
 * Set PFMX mode Peak Current setting.
 ******************************************************************************/
void sl_hal_emu_set_dcdc_pfmx_mode_peak_current(uint32_t value)
{
  // Verification that the parameter is in range.
  // if not, restrict value to maximum allowed.
  EFM_ASSERT(value <= (_DCDC_PFMXCTRL_IPKVAL_MASK >> _DCDC_PFMXCTRL_IPKVAL_SHIFT));
  if (value > (_DCDC_PFMXCTRL_IPKVAL_MASK >> _DCDC_PFMXCTRL_IPKVAL_SHIFT)) {
    value = (_DCDC_PFMXCTRL_IPKVAL_MASK >> _DCDC_PFMXCTRL_IPKVAL_SHIFT);
  }

#if defined(_DCDC_SYNCBUSY_MASK)
  // Wait for synchronization before writing new value.
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_PFMXCTRL);
#endif

  DCDC->PFMXCTRL = ((DCDC->PFMXCTRL & ~_DCDC_PFMXCTRL_IPKVAL_MASK)
                    | value << _DCDC_PFMXCTRL_IPKVAL_SHIFT);

  sl_hal_emu_dcdc_updated_hook();
}
#endif

#if defined(_DCDC_PFMXCTRL_IPKTMAXCTRL_MASK)
/***************************************************************************//**
 * Set Ton_max timeout control.
 ******************************************************************************/
SL_WEAK void sl_hal_emu_set_dcdc_pfmx_timeout_max_control(sl_hal_emu_dcdc_ton_max_timeout_t value)
{
#if defined(_DCDC_SYNCBUSY_MASK)
  // Wait for synchronization before writing new value.
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_PFMXCTRL);
#endif

  DCDC->PFMXCTRL = ((DCDC->PFMXCTRL & ~_DCDC_PFMXCTRL_IPKTMAXCTRL_MASK)
                    | value << _DCDC_PFMXCTRL_IPKTMAXCTRL_SHIFT);

  sl_hal_emu_dcdc_updated_hook();
}
#endif /* defined(_DCDC_PFMXCTRL_IPKTMAXCTRL_MASK) */
#endif /* defined(SL_HAL_EMU_DCDC_BUCK_PRESENT) */

#if defined(_DCDC_DOCTRL_REGULATIONTYPE_MASK)
/***************************************************************************//**
 * Set DCDC dual output regulation type.
 ******************************************************************************/
void sl_hal_emu_dcdc_set_regulation_type(sl_hal_emu_dcdc_regulation_type_t regulation_type)
{
#if defined(DCDC_SYNCBUSY_DOCTRL)
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_DOCTRL);
#endif

  sl_hal_bus_reg_write_mask(&DCDC->DOCTRL,
                            _DCDC_DOCTRL_REGULATIONTYPE_MASK,
                            ((uint32_t)regulation_type << _DCDC_DOCTRL_REGULATIONTYPE_SHIFT));

  sl_hal_emu_dcdc_updated_hook();
}

/***************************************************************************//**
 * Get current DCDC regulation type.
 ******************************************************************************/
sl_hal_emu_dcdc_regulation_type_t sl_hal_emu_dcdc_get_regulation_type(void)
{
  uint32_t reg_value;

  reg_value = (DCDC->DOCTRL & _DCDC_DOCTRL_REGULATIONTYPE_MASK) >> _DCDC_DOCTRL_REGULATIONTYPE_SHIFT;

  return (sl_hal_emu_dcdc_regulation_type_t)reg_value;
}
#endif

#if defined(_DCDC_DOCTRL_DUALIPKEN_MASK)
/***************************************************************************//**
 * Enable dual IPK DAC mode.
 ******************************************************************************/
void sl_hal_emu_dcdc_dual_ipk_enable(void)
{
#if defined(DCDC_SYNCBUSY_DOCTRL)
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_DOCTRL);
#endif

  sl_hal_bus_reg_write_mask(&DCDC->DOCTRL,
                            _DCDC_DOCTRL_DUALIPKEN_MASK,
                            DCDC_DOCTRL_DUALIPKEN);

  sl_hal_emu_dcdc_updated_hook();
}

/***************************************************************************//**
 * Disable dual IPK DAC mode.
 ******************************************************************************/
void sl_hal_emu_dcdc_dual_ipk_disable(void)
{
#if defined(DCDC_SYNCBUSY_DOCTRL)
  sl_hal_emu_dcdc_sync(DCDC_SYNCBUSY_DOCTRL);
#endif

  sl_hal_bus_reg_write_mask(&DCDC->DOCTRL,
                            _DCDC_DOCTRL_DUALIPKEN_MASK,
                            0);

  sl_hal_emu_dcdc_updated_hook();
}

/***************************************************************************//**
 * Get current dual IPK enable state.
 ******************************************************************************/
bool sl_hal_emu_dcdc_get_dual_ipk_enable(void)
{
  bool result;

  result = ((DCDC->DOCTRL & DCDC_DOCTRL_DUALIPKEN) != 0);

  return result;
}
#endif

#endif /* defined(EMU_PRESENT) */
