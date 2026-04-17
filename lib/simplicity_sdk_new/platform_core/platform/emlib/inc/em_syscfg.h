/***************************************************************************//**
 * @file
 * @brief API defining acces to SYSCFG registers
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

#ifndef EM_SYSCFG_H
#define EM_SYSCFG_H

#include "em_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SYSCFG)
/*******************************************************************************
 ******************************** TZ SERVICES **********************************
 ******************************************************************************/

#if defined(_SYSCFG_CHIPREV_FAMILY_MASK) || defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)
/*******************************************************************************
 * @brief Reads CHIPREV register
 ******************************************************************************/
#if defined(SL_TRUSTZONE_NONSECURE)
uint32_t SYSCFG_readChipRev(void);
#else // !SL_TRUSTZONE_NONSECURE
__STATIC_INLINE uint32_t SYSCFG_readChipRev(void)
{
#if defined(CMU_CLKEN0_SYSCFG)
  CMU->CLKEN0_SET = CMU_CLKEN0_SYSCFG;
#endif
  return SYSCFG->CHIPREV;
}
#endif // !SL_TRUSTZONE_NONSECURE
#endif // defined(_SYSCFG_CHIPREV_FAMILY_MASK) || defined(_SYSCFG_CHIPREV_PARTNUMBER_MASK)

#if defined(_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
// Map to tz_ns veneers in sli_tz_service_syscfg
void SYSCFG_setDmem0RamCtrlRamwsenBit(void);
void SYSCFG_clearDmem0RamCtrlRamwsenBit(void);
uint32_t SYSCFG_getDmem0RamCtrlRamwsenBit(void);
#else // !SL_TRUSTZONE_NONSECURE
/*******************************************************************************
 * @brief Sets DMEM0RAMCTRL RAMWSEN bit to 1
 ******************************************************************************/
__STATIC_INLINE void SYSCFG_setDmem0RamCtrlRamwsenBit(void)
{
  SYSCFG->DMEM0RAMCTRL = SYSCFG->DMEM0RAMCTRL | _SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK;
}

/*******************************************************************************
 * @brief Clears DMEM0RAMCTRL RAMWSEN bit to 0
 ******************************************************************************/
__STATIC_INLINE void SYSCFG_clearDmem0RamCtrlRamwsenBit(void)
{
  SYSCFG->DMEM0RAMCTRL = SYSCFG->DMEM0RAMCTRL & ~_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK;
}

/*******************************************************************************
 * @brief Reads DMEM0RAMCTRL RAMWSEN bit
 ******************************************************************************/
__STATIC_INLINE uint32_t SYSCFG_getDmem0RamCtrlRamwsenBit(void)
{
  return (SYSCFG->DMEM0RAMCTRL & _SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK) >> _SYSCFG_DMEM0RAMCTRL_RAMWSEN_SHIFT;
}
#endif // !SL_TRUSTZONE_NONSECURE
#endif //_SYSCFG_DMEM0RAMCTRL_RAMWSEN_MASK

#if defined(_SYSCFG_DMEM0RETNCTRL_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
// Map to tz_ns veneers in sli_tz_service_syscfg
uint32_t SYSCFG_readDmem0RetnCtrl(void);
void SYSCFG_maskDmem0RetnCtrl(uint32_t mask);
void SYSCFG_zeroDmem0RetnCtrl(void);
#else // !SL_TRUSTZONE_NONSECURE
/*******************************************************************************
 * @brief Reads DMEM0RETNCTRL register
 ******************************************************************************/
__STATIC_INLINE uint32_t SYSCFG_readDmem0RetnCtrl(void)
{
  return SYSCFG->DMEM0RETNCTRL;
}

/*******************************************************************************
 * @brief This function sets all bits in mask in the DEMORETNCTL register
 ******************************************************************************/
__STATIC_INLINE void SYSCFG_maskDmem0RetnCtrl(uint32_t mask)
{
  SYSCFG->DMEM0RETNCTRL = SYSCFG->DMEM0RETNCTRL | mask;
}

/*******************************************************************************
 * @brief Set DMEM0RETNCTRL to zero
 ******************************************************************************/
__STATIC_INLINE void SYSCFG_zeroDmem0RetnCtrl(void)
{
  SYSCFG->DMEM0RETNCTRL = 0x0UL;
}
#endif // !SL_TRUSTZONE_NONSECURE
#endif // _SYSCFG_DMEM0RETNCTRL_MASK

#if defined(_SYSCFG_CFGSYSTIC_MASK)
#if defined(SL_TRUSTZONE_NONSECURE)
void SYSCFG_setSysTicExtClkEnCfgSysTic(void);
void SYSCFG_clearSysTicExtClkEnCfgSysTic(void);
#else // !SL_TRUSTZONE_NONSECURE
/*******************************************************************************
 * @brief Set SYSTICEXTCLKEN bit in CFGSYSTIC to one
 ******************************************************************************/
__STATIC_INLINE void SYSCFG_setSysTicExtClkEnCfgSysTic(void)
{
  SYSCFG->CFGSYSTIC = (SYSCFG->CFGSYSTIC | _SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_MASK);
}

/*******************************************************************************
 * @brief Clear SYSTICEXTCLKEN bit in CFGSYSTIC to zero
 ******************************************************************************/
__STATIC_INLINE void SYSCFG_clearSysTicExtClkEnCfgSysTic(void)
{
  SYSCFG->CFGSYSTIC = (SYSCFG->CFGSYSTIC & ~_SYSCFG_CFGSYSTIC_SYSTICEXTCLKEN_MASK);
}
#endif //!SL_TRUSTZONE_NONSECURE
#endif //_SYSCFG_CFGSYSTIC_MASK
#endif //SYSCFG
#ifdef __cplusplus
}
#endif
#endif // EM_SYSCFG_H
