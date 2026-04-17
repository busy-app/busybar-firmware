/***************************************************************************//**
 * @file
 * @brief Silicon Labs TrustZone secure SYSCFG service (non-secure side).
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "sli_tz_ns_interface.h"
#include "sli_tz_s_interface.h"
#include "sli_tz_funcs_sids_autogen.h"
#include "em_device.h"

#if defined(_SYSCFG_CHIPREV_FAMILY_MASK) || defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)
/*******************************************************************************
 * @brief TZ Veneer - Reads CHIPREV register
 ******************************************************************************/
uint32_t sli_tz_syscfg_read_chiprev_register(void)
{
  return (uint32_t) sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_READ_CHIPREV_REGISTER_SID);
}
uint32_t SYSCFG_readChipRev(void)
{
  return sli_tz_syscfg_read_chiprev_register();
}
uint32_t sl_hal_syscfg_read_chip_rev(void)
{
  return sli_tz_syscfg_read_chiprev_register();
}
#endif // defined(_SYSCFG_CHIPREV_FAMILY_MASK) || defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)

#if defined(_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK)
/*******************************************************************************
 * @brief TZ Veneer - Sets DMEM0RAMCTRL RAMWSEN bit to 1
 ******************************************************************************/
uint32_t sli_tz_syscfg_set_dmem0ramctrl_ramwsen_bit(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_SET_DMEM0RAMCTRL_RAMWSEN_BIT_SID);
}
void SYSCFG_setDmem0RamCtrlRamwsenBit(void)
{
  sli_tz_syscfg_set_dmem0ramctrl_ramwsen_bit();
}
void sl_hal_syscfg_set_dmem0ramctrl_ramwsen_bit(void)
{
  sli_tz_syscfg_set_dmem0ramctrl_ramwsen_bit();
}

/*******************************************************************************
 * @brief TZ Veneer - Clears DMEM0RAMCTRL RAMWSEN bit to 0
 ******************************************************************************/
uint32_t sli_tz_syscfg_clear_dmem0ramctrl_ramwsen_bit(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_CLEAR_DMEM0RAMCTRL_RAMWSEN_BIT_SID);
}
void SYSCFG_clearDmem0RamCtrlRamwsenBit(void)
{
  sli_tz_syscfg_clear_dmem0ramctrl_ramwsen_bit();
}
void sl_hal_syscfg_clear_dmem0ramctrl_ramwsen_bit(void)
{
  sli_tz_syscfg_clear_dmem0ramctrl_ramwsen_bit();
}

/*******************************************************************************
 * @brief TZ Veneer - Reads DMEM0RAMCTRL RAMWSEN bit
 ******************************************************************************/
uint32_t sli_tz_syscfg_get_dmem0ramctrl_ramwsen_bit(void)
{
  return (uint32_t) sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_GET_DMEM0RAMCTRL_RAMWSEN_BIT_SID);
}
uint32_t SYSCFG_getDmem0RamCtrlRamwsenBit(void)
{
  return sli_tz_syscfg_get_dmem0ramctrl_ramwsen_bit();
}
uint32_t sl_hal_syscfg_get_dmem0ramctrl_ramwsen_bit(void)
{
  return sli_tz_syscfg_get_dmem0ramctrl_ramwsen_bit();
}
#endif //_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK

#if defined(_SYSCFG_DMEM0RETNCTRL_MASK)
/*******************************************************************************
 * @brief TZ Veneer - Reads DMEM0RETNCTRL register
 ******************************************************************************/
uint32_t sli_tz_syscfg_read_dmem0retnctrl_register(void)
{
  return (uint32_t) sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_READ_DMEM0RETNCTRL_REGISTER_SID);
}
uint32_t SYSCFG_readDmem0RetnCtrl(void)
{
  return sli_tz_syscfg_read_dmem0retnctrl_register();
}
uint32_t sl_hal_syscfg_read_dmem0retnctrl(void)
{
  return sli_tz_syscfg_read_dmem0retnctrl_register();
}

/*******************************************************************************
 * @brief Applies a mask to the DMEM0RETNCTRL register.
 *
 * @param[in] mask The bit mask to apply to the DMEM0RETNCTRL register.
 *
 * @details This function performs a bitwise OR operation between the current
 *          value of the DMEM0RETNCTRL register and the provided mask,
 *          setting specific bits in the register.
 ******************************************************************************/
uint32_t sli_tz_syscfg_mask_dmem0retnctrl_register(uint32_t mask)
{
  return sli_tz_ns_interface_dispatch_simple(
    (sli_tz_veneer_simple_fn)sli_tz_s_interface_dispatch_simple,
    SLI_TZ_SYSCFG_MASK_DMEM0RETNCTRL_REGISTER_SID,
    mask);
}
void SYSCFG_maskDmem0RetnCtrl(uint32_t mask)
{
  sli_tz_syscfg_mask_dmem0retnctrl_register(mask);
}
void sl_hal_syscfg_mask_dmem0retnctrl(uint32_t mask)
{
  sli_tz_syscfg_mask_dmem0retnctrl_register(mask);
}

/*******************************************************************************
 * @brief TZ Veneer - Set DMEM0RETNCTRL to zero
 ******************************************************************************/
uint32_t sli_tz_syscfg_zero_dmem0retnctrl_register(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_ZERO_DMEM0RETNCTRL_REGISTER_SID);
}
void SYSCFG_zeroDmem0RetnCtrl(void)
{
  sli_tz_syscfg_zero_dmem0retnctrl_register();
}
void sl_hal_syscfg_zero_dmem0retnctrl(void)
{
  sli_tz_syscfg_zero_dmem0retnctrl_register();
}

#endif // _SYSCFG_DMEM0RETNCTRL_MASK

#if defined(_SYSCFG_CFGSYSTIC_MASK)
/*******************************************************************************
 * @brief TZ Veneer - Set SYSTICEXTCLKEN bit in CFGSYSTIC to one
 ******************************************************************************/
uint32_t sli_tz_syscfg_set_systicextclken_cfgsystic(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_SET_SYSTICEXTCLKEN_CFGSYSTIC_SID);
}
void SYSCFG_setSysTicExtClkEnCfgSysTic(void)
{
  sli_tz_syscfg_set_systicextclken_cfgsystic();
}
void sl_hal_syscfg_set_systicextclken_cfgsystic(void)
{
  sli_tz_syscfg_set_systicextclken_cfgsystic();
}

/*******************************************************************************
 * @brief TZ Veneer - Clear SYSTICEXTCLKEN bit in CFGSYSTIC to zero
 ******************************************************************************/
uint32_t sli_tz_syscfg_clear_systicextclken_cfgsystic(void)
{
  return sli_tz_ns_interface_dispatch_simple_noarg(
    (sli_tz_veneer_simple_noarg_fn)sli_tz_s_interface_dispatch_simple_no_args,
    SLI_TZ_SYSCFG_CLEAR_SYSTICEXTCLKEN_CFGSYSTIC_SID);
}
void SYSCFG_clearSysTicExtClkEnCfgSysTic(void)
{
  sli_tz_syscfg_clear_systicextclken_cfgsystic();
}
void sl_hal_syscfg_clear_systicextclken_cfgsystic(void)
{
  sli_tz_syscfg_clear_systicextclken_cfgsystic();
}
#endif //_SYSCFG_CFGSYSTIC_MASK
