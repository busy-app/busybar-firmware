/***************************************************************************//**
 * @file
 * @brief API defining access to SYSCFG registers
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

#ifndef SL_HAL_SYSCFG_H
#define SL_HAL_SYSCFG_H

#include "em_device.h"
#include <stdint.h>
#include <sl_enum.h>
#if defined(SL_TRUSTZONE_NONSECURE)
#include "sli_tz_service_syscfg.h"
#endif
#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#if defined(SL_CATALOG_TZ_SECURE_KEY_LIBRARY_NS_PRESENT)
#include "sli_tz_ns_interface.h"
#include "sli_tz_service_msc.h"
#include "sli_tz_s_interface.h"
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup syscfg SYSTEM CONFIGURATION
 * @brief System Configuration (SYSCFG) peripheral API
 * @details The SYSCFG module provides access to various system configuration
 *          registers including chip revision information, system tick clock
 *          configuration, and memory controller settings.
 *
 * @{
 ******************************************************************************/

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/

/// Bit mask used to extract the part number value without the new naming bitfield.
#define SYSCFG_CHIPREV_PARTNUMBER1  0xFE0
#define SYSCFG_CHIPREV_PARTNUMBER0  0xF

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)
#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_6)

/// xG26 chips incorporate 2 memory banks including ECC support.
#define SL_HAL_SYSCFG_ECC_BANKS  (2)
/// Default syscfg ecc_config initialization.
#define SL_HAL_MSC_ECC_CONFIG_DEFAULT \
  {                                   \
    { false, false },                 \
    { 0, 1 },                         \
  }

#else

/// Series 2 chips incorporate 1 memory bank including ECC support.
#define SL_HAL_SYSCFG_ECC_BANKS  (1)
/// Default syscfg ecc_config initialization.
#define SL_HAL_MSC_ECC_CONFIG_DEFAULT \
  {                                   \
    { false },                        \
    { 0, 1 },                         \
  }

#endif
#endif /* #if defined(_SYSCFG_DMEM0ECCCTRL_MASK) || #if defined(_MPAHBRAM_CTRL_MASK) */

/*******************************************************************************
 ********************************   ENUMS   ************************************
 ******************************************************************************/

#if defined(_SYSCFG_DMEM0PORTMAPSEL_MASK)
/// AHBHOST masters that can use alternate MPAHBRAM ports.
SL_ENUM(sl_hal_syscfg_dmem_master_t) {
  SL_HAL_SYSCFG_DMEM_MASTER_LDMA      = _SYSCFG_DMEM0PORTMAPSEL_LDMAPORTSEL_SHIFT,            ///< LDMA master.
  SL_HAL_SYSCFG_DMEM_MASTER_SRWAES    = _SYSCFG_DMEM0PORTMAPSEL_SRWAESPORTSEL_SHIFT,          ///< SRWAES master.
  SL_HAL_SYSCFG_DMEM_MASTER_AHBSRW    = _SYSCFG_DMEM0PORTMAPSEL_AHBSRWPORTSEL_SHIFT,          ///< AHBSRW master.
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_IFADCDEBUGPORTSEL_MASK)
  SL_HAL_DMEM_MASTER_IFADC_DEBUG      = _SYSCFG_DMEM0PORTMAPSEL_IFADCDEBUGPORTSEL_SHIFT,      ///< IFADC_DEBUG master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_SRWECA0PORTSEL_MASK)
  SL_HAL_DMEM_MASTER_SRWECA0          = _SYSCFG_DMEM0PORTMAPSEL_SRWECA0PORTSEL_SHIFT,         ///< SRWECA0 master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_SRWECA1PORTSEL_MASK)
  SL_HAL_DMEM_MASTER_SRWECA1          = _SYSCFG_DMEM0PORTMAPSEL_SRWECA1PORTSEL_SHIFT,         ///< SRWECA1 master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_MVPAHBDATA0PORTSEL_MASK)
  SL_HAL_DMEM_MASTER_MVPAHB_DATA0     = _SYSCFG_DMEM0PORTMAPSEL_MVPAHBDATA0PORTSEL_SHIFT,     ///< MVPAHB_DATA0 master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_MVPAHBDATA1PORTSEL_MASK)
  SL_HAL_DMEM_MASTER_MVPAHB_DATA1     = _SYSCFG_DMEM0PORTMAPSEL_MVPAHBDATA1PORTSEL_SHIFT,     ///< MVPAHB_DATA1 master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_MVPAHBDATA2PORTSEL_MASK)
  SL_HAL_DMEM_MASTER_MVPAHB_DATA2     = _SYSCFG_DMEM0PORTMAPSEL_MVPAHBDATA2PORTSEL_SHIFT,     ///< MVPAHB_DATA2 master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_LDMA1PORTSEL_MASK)
  SL_HAL_SYSCFG_DMEM_MASTER_LDMA1     = _SYSCFG_DMEM0PORTMAPSEL_LDMA1PORTSEL_SHIFT,           ///< LDMA1 master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_SRWLDMAPORTSEL_MASK)
  SL_HAL_SYSCFG_DMEM_MASTER_SRW_LDMA  = _SYSCFG_DMEM0PORTMAPSEL_SRWLDMAPORTSEL_SHIFT,         ///< SRW_LDMA master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_USBPORTSEL_MASK)
  SL_HAL_SYSCFG_DMEM_MASTER_USB       = _SYSCFG_DMEM0PORTMAPSEL_USBPORTSEL_SHIFT,             ///< USB master.
 #endif
 #if defined(_SYSCFG_DMEM0PORTMAPSEL_BUFCPORTSEL_MASK)
  SL_HAL_SYSCFG_DMEM_MASTER_BUFC      = _SYSCFG_DMEM0PORTMAPSEL_BUFCPORTSEL_SHIFT             ///< BUFC master.
 #endif
};
 #endif

 #if defined(_MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK)
/// AHB port given priority.
SL_ENUM(sl_hal_syscfg_port_priority_t) {
  SL_HAL_SYSCFG_PORT_PRIORITY_NONE  = _MPAHBRAM_CTRL_AHBPORTPRIORITY_NONE,        ///< Priority none.
  SL_HAL_SYSCFG_PORT_PRIORITY_PORT0 = _MPAHBRAM_CTRL_AHBPORTPRIORITY_PORT0,       ///< Priority is given to port 0.
  SL_HAL_SYSCFG_PORT_PRIORITY_PORT1 = _MPAHBRAM_CTRL_AHBPORTPRIORITY_PORT1,       ///< Priority is given to port 1.
 #if defined(_MPAHBRAM_CTRL_AHBPORTPRIORITY_PORT2)
  SL_HAL_SYSCFG_PORT_PRIORITY_PORT2 = _MPAHBRAM_CTRL_AHBPORTPRIORITY_PORT2,       ///< Priority is given to port 2.
 #endif
 #if defined(_MPAHBRAM_CTRL_AHBPORTPRIORITY_PORT3)
  SL_HAL_SYSCFG_PORT_PRIORITY_PORT3 = _MPAHBRAM_CTRL_AHBPORTPRIORITY_PORT3,       ///< Priority is given to port 3.
 #endif
};
 #endif

/*******************************************************************************
 *******************************   STRUCTS   ***********************************
 ******************************************************************************/

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)
/// ECC configuration.
typedef struct {
  bool     enable_ecc_bank[SL_HAL_SYSCFG_ECC_BANKS]; ///< Array of bools to enable/disable error correcting Code (ECC).
  uint32_t dma_channels[2];                          ///< Array of 2 DMA channel numbers to use for ECC initialization.
} sl_hal_syscfg_ecc_config_t;
#endif

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/

/***************************************************************************//**
 * Reads CHIPREV register.
 *
 * @return The current value of the CHIPREV register, containing chip revision
 *         information.
 ******************************************************************************/

#if defined(_SYSCFG_CHIPREV_FAMILY_MASK) || defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
uint32_t sl_hal_syscfg_read_chip_rev(void);
#else  // !SL_TRUSTZONE_NONSECURE
#if !defined(FPGA)
__INLINE uint32_t sl_hal_syscfg_read_chip_rev(void)
{
  return SYSCFG->CHIPREV;
}
#endif
#endif  // !SL_TRUSTZONE_NONSECURE
#endif

#if defined(_SYSCFG_CFGSYSTIC_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
void sl_hal_syscfg_set_systicextclken_cfgsystic(void);
void sl_hal_syscfg_clear_systicextclken_cfgsystic(void);
#else  // !SL_TRUSTZONE_NONSECURE
#if !defined(FPGA)
/***************************************************************************//**
 * Enables the external clock for the SysTick timer.
 ******************************************************************************/
__INLINE void sl_hal_syscfg_set_systicextclken_cfgsystic(void)
{
  SYSCFG->CFGSYSTIC = (SYSCFG->CFGSYSTIC | _SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_MASK);
}

/***************************************************************************//**
 * Disables the external clock source for the SysTick timer.
 ******************************************************************************/
__INLINE void sl_hal_syscfg_clear_systicextclken_cfgsystic(void)
{
  SYSCFG->CFGSYSTIC = (SYSCFG->CFGSYSTIC & ~_SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_MASK);
}
#endif
#endif  // !SL_TRUSTZONE_NONSECURE
#endif

#if defined(_SYSCFG_DMEM0ECCCTRL_MASK) \
  || defined(_MPAHBRAM_CTRL_MASK)
/***************************************************************************//**
 * Configure Error Correcting Code (ECC).
 *
 * @param[in] ecc_config ECC configuration.
 *
 * @details This function configures ECC support according to the configuration
 *          input parameter. If the user requests enabling ECC for a given RAM
 *          bank this function will initialize ECC memory (syndromes) for the
 *          bank by reading and writing the existing values in memory. I.e. all
 *          data is preserved. The initialization process runs in a critical
 *          section disallowing interrupts and thread scheduling, and will
 *          consume a considerable amount of clock cycles. Therefore the user
 *          should carefully assess where to call this function. The user can
 *          consider to increase the clock frequency in order to reduce the
 *          execution time.
 *          This function makes use of 2 DMA channels to move data to/from RAM
 *          in an efficient way. The user can select which 2 DMA channels to
 *          use in order to avoid conflicts with the application. However the
 *          user must make sure that no other DMA operations takes place while
 *          this function is executing.
 *          If the application has been using the DMA controller prior to calling
 *          this function, the application will need to reinitialize DMA registers
 *          after this function has completed.
 *
 * @note This function protects the ECC initialization procedure from interrupts
 *       and other threads by using a critical section (defined by sl_core.h)
 *       When running on RTOS the user may need to override CORE_EnterCritical
 *       CORE_ExitCritical.
 ******************************************************************************/
void sl_hal_syscfg_ecc_set_config(const sl_hal_syscfg_ecc_config_t *ecc_config);
#endif

#if defined(_SYSCFG_DMEM0PORTMAPSEL_MASK)
/***************************************************************************//**
 * Set MPAHBRAM port to use to access DMEM.
 *
 * @param[in] master AHBHOST master to be configured.
 * @param[in] port AHBHOST slave port to use.
 *
 * @details This function configures which MPAHBRAM slave port is used to access
 *          DMEM. Depending on the use case, it might improve performance by
 *          spreading the load over the N ports (N is usually 2 or 4),instead of
 *          starving because a port is used by another master.
 ******************************************************************************/
void sl_hal_syscfg_set_dmem_port_map(sl_hal_syscfg_dmem_master_t master,
                                     uint8_t port);
#endif

#if defined(_MPAHBRAM_CTRL_AHBPORTPRIORITY_MASK)
/***************************************************************************//**
 * Set MPAHBRAM port priority for arbitration when multiple concurrent
 * transactions to DMEM.
 *
 * @param[in] port_priority AHBHOST slave port having elevated priority.
 *
 * @details This function configures which MPAHBRAM slave port will have priority.
 *          The AHB port arbitration default scheme, round-robin arbitration, is
 *          selected when port_priority == SL_HAL_SYSCFG_PORT_PRIORITY_NONE.
 *
 * @note Doing this can potentially starve the others AHB port(s).
 ******************************************************************************/
void sl_hal_syscfg_set_port_priority(sl_hal_syscfg_port_priority_t port_priority);

/***************************************************************************//**
 * Get MPAHBRAM port arbitration priority selection.
 *
 * @return Returns the AHBHOST slave port given priority or none.
 *
 * @details This function returns the AHBHOST slave with raised priority.
 ******************************************************************************/
sl_hal_syscfg_port_priority_t sl_hal_syscfg_get_port_priority(void);
#endif

#if defined(_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
// Map to tz_ns veneers in sli_tz_service_syscfg
void sl_hal_syscfg_set_dmem0ramctrl_ramwsen_bit(void);
void sl_hal_syscfg_clear_dmem0ramctrl_ramwsen_bit(void);
uint32_t sl_hal_syscfg_get_dmem0ramctrl_ramwsen_bit(void);
#else // !SL_TRUSTZONE_NONSECURE
/***************************************************************************//**
 * Enables the RAM wait state feature for DMEM0.
 *
 * @details This function sets the RAMWSEN bit in the DMEM0RAMCTRL register to
 *          `1`, enabling the RAM wait state feature for DMEM0. This feature is
 *          used to configure the memory access timing for DMEM0.
 ******************************************************************************/
__INLINE void sl_hal_syscfg_set_dmem0ramctrl_ramwsen_bit(void)
{
  SYSCFG->DMEM0RAMCTRL = SYSCFG->DMEM0RAMCTRL | _SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK;
}

/***************************************************************************//**
 * Disables the RAM wait state feature for DMEM0.
 *
 * @details This function clears the RAMWSEN bit in the DMEM0RAMCTRL register
 *          to  `0`,disabling the RAM wait state feature for DMEM0. This ensures
 *          that memory access timing for DMEM0 operates without additional wait
 *          states.
 ******************************************************************************/
__INLINE void sl_hal_syscfg_clear_dmem0ramctrl_ramwsen_bit(void)
{
  SYSCFG->DMEM0RAMCTRL = SYSCFG->DMEM0RAMCTRL & ~_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK;
}

/***************************************************************************//**
 * Reads the RAMWSEN bit from the DMEM0RAMCTRL register.
 *
 * @return The value of the RAMWSEN bit (1 if enabled, 0 if disabled).
 *
 * @details This function retrieves the value of the RAMWSEN bit in the DMEM0RAMCTRL
 *          register. The RAMWSEN bit indicates whether the RAM wait state feature
 *          for DMEM0 is enabled (`1`) or disabled (`0`).
 ******************************************************************************/
__INLINE uint32_t sl_hal_syscfg_get_dmem0ramctrl_ramwsen_bit(void)
{
  return (SYSCFG->DMEM0RAMCTRL & _SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK)
         >> _SYSCFG_DMEM0RAMCTRL_RAMWSEN_SHIFT;
}
#endif // !SL_TRUSTZONE_NONSECURE
#endif /* _SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK */

#if defined(_SYSCFG_DMEM0RETNCTRL_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
// Map to tz_ns veneers in sli_tz_service_syscfg
uint32_t sl_hal_syscfg_read_dmem0retnctrl(void);
void sl_hal_syscfg_mask_dmem0retnctrl(uint32_t mask);
void sl_hal_syscfg_zero_dmem0retnctrl(void);
#else // !SL_TRUSTZONE_NONSECURE
/***************************************************************************//**
 * Reads the DMEM0RETNCTRL register.
 *
 * @return The current value of the DMEM0RETNCTRL register.
 *
 * @details This function retrieves the current value of the DMEM0RETNCTRL
 *          register, which is used to control the retention settings for
 *          DMEM0 memory blocks.
 ******************************************************************************/
__INLINE uint32_t sl_hal_syscfg_read_dmem0retnctrl(void)
{
  return SYSCFG->DMEM0RETNCTRL;
}

/***************************************************************************//**
 * Applies a mask to the DMEM0RETNCTRL register.
 *
 * @param[in] mask The bit mask to apply to the DMEM0RETNCTRL register.
 *
 * @details This function performs a bitwise OR operation between the current
 *          value of the DMEM0RETNCTRL register and the provided mask,
 *          setting specific bits in the register.
 ******************************************************************************/
__INLINE void sl_hal_syscfg_mask_dmem0retnctrl(uint32_t mask)
{
  SYSCFG->DMEM0RETNCTRL = SYSCFG->DMEM0RETNCTRL | mask;
}

/***************************************************************************//**
 * Sets the DMEM0RETNCTRL register to zero.
 *
 * @details This function clears all bits in the DMEM0RETNCTRL register by
 *          setting its value to 0. This effectively disables all retention
 *          settings for DMEM0 memory blocks.
 ******************************************************************************/
__INLINE void sl_hal_syscfg_zero_dmem0retnctrl(void)
{
  SYSCFG->DMEM0RETNCTRL = 0x0UL;
}
#endif // !SL_TRUSTZONE_NONSECURE
#endif /* _SYSCFG_DMEM0RETNCTRL_MASK */

/** @} (end addtogroup syscfg) */

#ifdef __cplusplus
}
#endif
#endif // SL_HAL_SYSCFG_H
