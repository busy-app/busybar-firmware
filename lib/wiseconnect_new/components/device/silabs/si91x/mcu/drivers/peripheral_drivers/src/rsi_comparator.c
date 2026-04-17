/******************************************************************************
* @file  rsi_comparator.c
*******************************************************************************
* # License
* <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "rsi_comparator.h"
#include "rsi_reg_spi.h"
#include "rsi_rom_egpio.h"

/** @addtogroup SOC20
* @{
*/
/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_Config(AUX_ADC_DAC_COMP_Type *comp, uint8_t comp_number, uint8_t sel_p_mux, uint8_t sel_n_mux, uint8_t hyst_en, uint8_t filter_en)
 * @brief        This API is used to configure the comparator parameters.
 * @param[in]    comp        : Pointer to the comparator register instance.
 * @param[in]    comp_number : comparator number to be enabled or disabled
 *                             - 1:for comparator 1 
 *                             - 2:for comparator 2    
 * @param[in]    sel_p_mux   : select positive input for comparator 
 *                             - 0: compx_p0 x is 1 or 2 i.e comp1 and comp2
 *                             - 1: compx_p1 
 *                             - 2: DAC	 
 *                             - 3: reference buffer out 
 *                             - 4: reference scalar out 
 *                             - 5: resistor bank out 
 *                             - 6: opamp1_out 
 *                             - 7: opamp2_out 
 *                             - 8: opamp3_out 
 * @param[in]    sel_n_mux   : select negative input for comparator 
 *                             - 0: compx_n0 x is 1 or 2 i.e comp1 and comp2
 *                             - 1: compx_n1 
 *                             - 2: DAC	 
 *                             - 3: reference buffer out 
 *                             - 4: reference scalar out 
 *                             - 5: resistor bank out 
 *                             - 6: opamp1_out 
 *                             - 7: opamp2_out 
 *                             - 8: opamp3_out                             
 * @param[in]    hyst_en     :  control the hysteresis of the comparator(0 to 4)   
 * @param[in]    filter_en   :  To enable filter for comparator 
 *                             - 1:enable
 *                             - 0:disable 
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.  
 *       
 * @b Example    
 *     - RSI_COMP_Config(COMPARATOR_1,COMP1, COMP1_POSITIVE_INPUT, COMP1_NEGATIVE_INPUT, HYST_ENABLE,FILTER_ENABLE); 
 */
rsi_error_t RSI_COMP_Config(AUX_ADC_DAC_COMP_Type *comp,
                            uint8_t comp_number,
                            uint8_t sel_p_mux,
                            uint8_t sel_n_mux,
                            uint8_t hyst_en,
                            uint8_t filter_en)
{
  /* Address of a target register ULP */
  ULP_SPI_MEM_MAP(BG_SCDC_PROG_REG1) |= BIT(3);

  /* Parameter validation */
  if ((comp_number > MAX_NUM_OF_COMPARATORS) || (comp_number < MIN_NUM_OF_COMPARATORS) || (sel_p_mux > MAX_P_SEL_MUX)
      || (sel_n_mux > MAX_N_SEL_MUX) || (hyst_en > MAX_NO_OF_HYST)) {
    return INVALID_PARAMETERS;
  }

  /*Configure the comparator1 parameter*/
  if (comp_number == COMP1) {
    /* Disable the comparator */
    comp->COMPARATOR1_b.CMP1_EN        = DISABLE;
    comp->COMPARATOR1_b.CMP1_MUX_SEL_P = (unsigned int)(sel_p_mux & 0x0F);
    comp->COMPARATOR1_b.CMP1_MUX_SEL_N = (unsigned int)(sel_n_mux & 0x0F);
    comp->COMPARATOR1_b.CMP1_HYST      = (unsigned int)(hyst_en & 0x03);
    comp->COMPARATOR1_b.CMP1_EN_FILTER = (unsigned int)(filter_en & 0x01);
  } else {
    /*Configure the comparator2 parameter*/
    /* Disable the comparator */
    comp->COMPARATOR1_b.CMP2_EN        = DISABLE;
    comp->COMPARATOR1_b.CMP2_MUX_SEL_P = (unsigned int)(sel_p_mux & 0x0F);
    comp->COMPARATOR1_b.CMP2_MUX_SEL_N = (unsigned int)(sel_n_mux & 0x0F);
    comp->COMPARATOR1_b.CMP2_HYST      = (unsigned int)(hyst_en & 0x03);
    comp->COMPARATOR1_b.CMP2_EN_FILTER = (unsigned int)(filter_en & 0x01);
  }

  /* Reference buffer enable */
  if (((sel_p_mux == 3) || (sel_n_mux == 3)) || ((sel_p_mux == 4) || (sel_n_mux == 4))) {
    comp->BOD_b.REFBUF_EN = ENABLE;
  }
  /* Resistor bank enable */
  if ((sel_p_mux == 5) || (sel_n_mux == 5)) {
    comp->BOD_b.BOD_RES_EN = ENABLE;
  }

  /*Configure the required AGPIO */
  RSI_COMP_PinMux(comp_number, sel_p_mux, sel_n_mux);

  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_Enable(AUX_ADC_DAC_COMP_Type* comp,uint8_t comp_number , uint8_t enable)
 * @brief        This API is used to enable and disable the comparator
 * @param[in]    comp        : Pointer to the comparator register instance.
 * @param[in]    comp_number : comparator no to be enabled or disabled
 *                             - 1:for comparator 1 
 *                             - 2:for comparator 2                           
 * @param[in] 	 enable      :enable and disable comp
 *                             - 1:enable
 *                             - 0:disable
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.
 *       
 * @b Example    
 *     - RSI_COMP_Enable(COMPARATOR_1,COMP1,ENABLE,METAL_REV1P3); 
 */
rsi_error_t RSI_COMP_Enable(AUX_ADC_DAC_COMP_Type *comp, uint8_t comp_number, uint8_t enable)
{
  uint32_t i = 0;
  if ((comp_number > MAX_NUM_OF_COMPARATORS) || (comp_number < MIN_NUM_OF_COMPARATORS)) {
    return INVALID_PARAMETERS;
  }
#ifdef CHIP_9118
  /* Configure the comparator if metal_rev_1p4 revision enable */
  if (SiliconRev >= METAL_REV1P4) {
    /* Enable comparator1 */
    if (comp_number == COMP1) {
      RSI_COMP_IntrEnableDisable(comp_number, DISABLE);

      comp->COMPARATOR1_b.CMP1_EN = enable;

      if (enable) {
        for (i = 0; i < 300; i++) {
          __ASM("nop");
        }
        RSI_COMP_IntrEnableDisable(comp_number, ENABLE);
        NVIC_EnableIRQ(COMP1_IRQn);
      } else {
        NVIC_DisableIRQ(COMP1_IRQn);
      }
    }
    /* Enable Comparator2 */
    else {
      RSI_COMP_IntrEnableDisable(comp_number, DISABLE);

      comp->COMPARATOR1_b.CMP2_EN = enable;

      if (enable) {
        for (i = 0; i < 300; i++) {
          __ASM("nop");
        }
        RSI_COMP_IntrEnableDisable(comp_number, ENABLE);
        NVIC_EnableIRQ(COMP2_IRQn);
      } else {
        NVIC_DisableIRQ(COMP2_IRQn);
      }
    }
  }
  /* Configure the comparator if metal_rev_1p3 revision enable */
  else {
    /* Enable comparator1 */
    if (comp_number == COMP1) {
      NVIC_DisableIRQ(COMP1_IRQn);

      comp->COMPARATOR1_b.CMP1_EN = enable;

      if (enable) {
        for (i = 0; i < 300; i++) {
          __ASM("nop");
        }
        NVIC_EnableIRQ(COMP1_IRQn);
      }
    }
    /* Enable Comparator2 */
    else {
      NVIC_DisableIRQ(COMP2_IRQn);

      comp->COMPARATOR1_b.CMP2_EN = enable;

      if (enable) {
        for (i = 0; i < 300; i++) {
          __ASM("nop");
        }
        NVIC_EnableIRQ(COMP2_IRQn);
      }
    }
  }
#endif
#if defined(SLI_SI917)
  /* Enable comparator1 */
  if (comp_number == COMP1) {
    RSI_COMP_IntrEnableDisable(comp_number, DISABLE);

    comp->COMPARATOR1_b.CMP1_EN = (unsigned int)(enable & 0x01);

    if (enable) {
      for (i = 0; i < 300; i++) {
        __ASM("nop");
      }
      RSI_COMP_IntrEnableDisable(comp_number, ENABLE);
      NVIC_EnableIRQ(COMP1_IRQn);
    } else {
      NVIC_DisableIRQ(COMP1_IRQn);
    }
  }
  /* Enable Comparator2 */
  else {
    RSI_COMP_IntrEnableDisable(comp_number, DISABLE);

    comp->COMPARATOR1_b.CMP2_EN = (unsigned int)(enable & 0x01);

    if (enable) {
      for (i = 0; i < 300; i++) {
        __ASM("nop");
      }
      RSI_COMP_IntrEnableDisable(comp_number, ENABLE);
      NVIC_EnableIRQ(COMP2_IRQn);
    } else {
      NVIC_DisableIRQ(COMP2_IRQn);
    }
  }
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_ResBank(AUX_ADC_DAC_COMP_Type* comp,uint16_t value_thrsh)
 * @brief        This API is used to set register bank threshold value.
 * @param[in]    comp        : Pointer to the comparator register instance                       
 * @param[in] 	 value_thrsh : Value of register threshold value. 
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.
 *       
 * @b Example    
 *     - RSI_COMP_ResistorBank_Thrsh(COMPARATOR_1,value_thrsh);
 */
rsi_error_t RSI_COMP_ResBank(AUX_ADC_DAC_COMP_Type *comp, uint16_t value_thrsh)
{
  if ((value_thrsh > MAX_THRSHOLD_VALUE)) {
    return INVALID_PARAMETERS;
  }
  /* Configure the Comparator1 Res bank threshold value */
  if ((comp->COMPARATOR1_b.CMP1_MUX_SEL_P == 5) || (comp->COMPARATOR1_b.CMP1_MUX_SEL_N == 5)) {
    /* Configure the Comparator1 Res bank threshold value if
  		 metal revision select as metal_rev_1p4 */
#ifdef CHIP_9118
    if (SiliconRev >= METAL_REV1P4) {
      RSI_COMP_IntrEnableDisable(COMP1, DISABLE);

      comp->BOD_b.BOD_THRSH = value_thrsh;

      RSI_COMP_IntrEnableDisable(COMP1, ENABLE);
    }
    /* Configure the Comparator1 Res bank threshold value if
  		 metal revision select as metal_rev_1p3 */
    else {
      NVIC_DisableIRQ(COMP1_IRQn);

      comp->BOD_b.BOD_THRSH = value_thrsh;

      NVIC_EnableIRQ(COMP1_IRQn);
    }
#endif
#if defined(SLI_SI917)
    RSI_COMP_IntrEnableDisable(COMP1, DISABLE);

    comp->BOD_b.BOD_THRSH = (unsigned int)(value_thrsh & 0x1F);

    RSI_COMP_IntrEnableDisable(COMP1, ENABLE);
#endif
  }
  /* Configure the Comparator2 Res bank threshold value */
  if ((comp->COMPARATOR1_b.CMP2_MUX_SEL_P == 5) || (comp->COMPARATOR1_b.CMP2_MUX_SEL_N == 5)) {
    /* Configure the Comparator2 Res bank threshold value if
  		 metal revision select as metal_rev_1p4 */
#ifdef CHIP_9118
    if (SiliconRev >= METAL_REV1P4) {
      RSI_COMP_IntrEnableDisable(COMP2, DISABLE);

      comp->BOD_b.BOD_THRSH = value_thrsh;

      RSI_COMP_IntrEnableDisable(COMP2, ENABLE);
    }
    /* Configure the Comparator2 Res bank threshold value if
  		 metal revision select as metal_rev_1p3 */
    else {
      NVIC_DisableIRQ(COMP2_IRQn);

      comp->BOD_b.BOD_THRSH = value_thrsh;

      NVIC_EnableIRQ(COMP2_IRQn);
    }
#endif
#if defined(SLI_SI917)
    RSI_COMP_IntrEnableDisable(COMP2, DISABLE);

    comp->BOD_b.BOD_THRSH = (unsigned int)(value_thrsh & 0x1F);

    RSI_COMP_IntrEnableDisable(COMP2, ENABLE);
#endif
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_ReferenceScaler(AUX_ADC_DAC_COMP_Type* comp,uint16_t scalar_factor_value)
 * @brief        This API is used to set the scalar output value.
 * @param[in]    comp                : Pointer to the comparator register instance                      
 * @param[in] 	 scalar_factor_value : Set reference scalar value.
 * @param[in]    comp_number         : Select comparator for operation
 *                                     - 1:Select comparator 1. 
 *                                     - 2:Select comparator 2.      
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.
 *       
 * @b Example    
 *     - RSI_COMP_ReferenceScalarOut(COMPARATOR_1,scalar_factor_value,METAL_REV1P3);    
 */
rsi_error_t RSI_COMP_ReferenceScaler(AUX_ADC_DAC_COMP_Type *comp, uint16_t scalar_factor_value)
{
  if ((scalar_factor_value > MAX_SCALAR_FACTOR)) {
    return INVALID_PARAMETERS;
  }
  /* Configure the Comparator1 reference scaler output value */
  if ((comp->COMPARATOR1_b.CMP1_MUX_SEL_P == 4) || (comp->COMPARATOR1_b.CMP1_MUX_SEL_N == 4)) {
    /* Configure the Comparator1 Reference scaler output value if
  	   metal revision select as metal_rev_1p4 */
#ifdef CHIP_9118
    if (SiliconRev >= METAL_REV1P4) {
      RSI_COMP_IntrEnableDisable(COMP1, DISABLE);

      comp->BOD_b.REFBUF_VOLT_SEL = scalar_factor_value;

      RSI_COMP_IntrEnableDisable(COMP1, ENABLE);
    }
    /* Configure the Comparator1 Reference scaler output value if
  	   metal revision select as metal_rev_1p3 */
    else {
      NVIC_DisableIRQ(COMP1_IRQn);

      comp->BOD_b.REFBUF_VOLT_SEL = scalar_factor_value;

      NVIC_EnableIRQ(COMP1_IRQn);
    }
#endif
#if defined(SLI_SI917)
    RSI_COMP_IntrEnableDisable(COMP1, DISABLE);

    comp->BOD_b.REFBUF_VOLT_SEL = (unsigned int)(scalar_factor_value & 0x0F);

    RSI_COMP_IntrEnableDisable(COMP1, ENABLE);
#endif
  }
  /* Configure the Comparator2 reference scaler output value */
  if ((comp->COMPARATOR1_b.CMP2_MUX_SEL_P == 4) || (comp->COMPARATOR1_b.CMP2_MUX_SEL_N == 4)) {
    /* Configure the Comparator2 Reference scaler output value if
  	   metal revision select as metal_rev_1p4 */
#ifdef CHIP_9118
    if (SiliconRev >= METAL_REV1P4) {
      RSI_COMP_IntrEnableDisable(COMP2, DISABLE);

      comp->BOD_b.REFBUF_VOLT_SEL = scalar_factor_value;

      RSI_COMP_IntrEnableDisable(COMP2, ENABLE);
    }
    /* Configure the Comparator2 Reference scaler output value if
  	   metal revision select as metal_rev_1p3 */
    else {
      NVIC_DisableIRQ(COMP2_IRQn);

      comp->BOD_b.REFBUF_VOLT_SEL = scalar_factor_value;

      NVIC_EnableIRQ(COMP2_IRQn);
    }
#endif
#if defined(SLI_SI917)
    RSI_COMP_IntrEnableDisable(COMP2, DISABLE);

    comp->BOD_b.REFBUF_VOLT_SEL = (unsigned int)(scalar_factor_value & 0x0F);

    RSI_COMP_IntrEnableDisable(COMP2, ENABLE);
#endif
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_OutputMaskConfig(uint8_t comp_number,uint8_t MaskConfig)
 * @brief        This API is used for masking or unmasking the comparator interrupt.     
 * @param[in]    comp_number         : Select comparator for operation
 *                                     - 1:Select comparator 1. 
 *                                     - 2:Select comparator 2.      
 * @param[in]    MaskConfig          : This parameter masking or unmasking comparator interrupt. 
 *                                     - 1:Mask the comparator interrupt. 
 *                                     - 0:UnMask the comparator interrupt.      
 * @param[in] 	 metalRev            : Metal revision number
 *                                     - 1:metal_rev_1p4
 *                                     - 0:metal_rev_1p3
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.
 *       
 * @b Example    
 *     - RSI_COMP_ReferenceScalarOut(COMPARATOR_1,ENABLE);    
 */
rsi_error_t RSI_COMP_OutputMaskConfig(uint8_t comp_number, uint8_t MaskConfig)
{
  if ((comp_number > MAX_NUM_OF_COMPARATORS) || (comp_number < MIN_NUM_OF_COMPARATORS)) {
    return INVALID_PARAMETERS;
  }
#ifdef CHIP_9118
  /* Set the comparator interrupt mask or unmask if metal revision select as metal_rev_1p4 */
  if (SiliconRev >= METAL_REV1P4) {
    if (comp_number == COMP1) {
      /* Mask the comparator1 interrupt */
      if (MaskConfig) {
        RSI_COMP_IntrEnableDisable(COMP1, DISABLE);
      }
      /* UnMask the comparator1 interrupt */
      else {
        RSI_COMP_IntrEnableDisable(COMP1, ENABLE);
      }
    } else {
      /* Mask the comparator2 interrupt */
      if (MaskConfig) {
        RSI_COMP_IntrEnableDisable(COMP2, DISABLE);
      }
      /* UnMask the comparator2 interrupt */
      else {
        RSI_COMP_IntrEnableDisable(COMP2, ENABLE);
      }
    }
  }
  /* Set the comparator interrupt mask or unmask if metal revision select as metal_rev_1p3 */
  else {
    if (comp_number == COMP1) {
      /* Mask the comparator1 interrupt */
      if (MaskConfig) {
        NVIC_DisableIRQ(COMP1_IRQn);
      }
      /* UnMask the comparator1 interrupt */
      else {
        NVIC_EnableIRQ(COMP1_IRQn);
      }
    } else {
      /* Mask the comparator2 interrupt */
      if (MaskConfig) {
        NVIC_DisableIRQ(COMP2_IRQn);
      }
      /* UnMask the comparator2 interrupt */
      else {
        NVIC_EnableIRQ(COMP2_IRQn);
      }
    }
  }
#endif
#if defined(SLI_SI917)
  if (comp_number == COMP1) {
    /* Mask the comparator1 interrupt */
    if (MaskConfig) {
      RSI_COMP_IntrEnableDisable(COMP1, DISABLE);
    }
    /* UnMask the comparator1 interrupt */
    else {
      RSI_COMP_IntrEnableDisable(COMP1, ENABLE);
    }
  } else {
    /* Mask the comparator2 interrupt */
    if (MaskConfig) {
      RSI_COMP_IntrEnableDisable(COMP2, DISABLE);
    }
    /* UnMask the comparator2 interrupt */
    else {
      RSI_COMP_IntrEnableDisable(COMP2, ENABLE);
    }
  }
#endif
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_IntrEnableDisable(uint8_t comp_number,uint8_t enable)
 * @brief        This API is used to disable or enable comparator interrupt
 *               if metal revision select as metal_rev_1p4.
 * @param[in]    comp_number : comparator no to be enabled or disabled
 *                             - 1:for comparator 1 
 *                             - 2:for comparator 2                           
 * @param[in] 	 enable      : enable and disable comparator interrupt
 *                             - 1:enable
 *                             - 0:disable
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.
 *       
 * @b Example    
 *     - RSI_COMP_IntrEnableDisable(COMPARATOR_1,ENABLE)
 */
rsi_error_t RSI_COMP_IntrEnableDisable(uint8_t comp_number, uint8_t enable)
{
  if ((comp_number > MAX_NUM_OF_COMPARATORS) || (comp_number < MIN_NUM_OF_COMPARATORS)) {
    return INVALID_PARAMETERS;
  }
  /* Enable the comparator interrupt if metal revision select as metal_rev_1p4 */
  if (enable) {
    /* Enable the comparator1 interrupt */
    if (comp_number == COMP1) {
      ULPCLK->ULP_TA_PERI_RESET_REG_b.COMP1_OUTPUT_CNTRL_b = ENABLE;
    }
    /* Enable the comparator2 interrupt */
    else {
      ULPCLK->ULP_TA_PERI_RESET_REG_b.COMP2_OUTPUT_CNTRL_b = ENABLE;
    }
  }
  /* Disable the comparator interrupt if metal revision select as metal_rev_1p4 */
  else {
    /* Disable the comparator1 interrupt */
    if (comp_number == COMP1) {
      ULPCLK->ULP_TA_PERI_RESET_REG_b.COMP1_OUTPUT_CNTRL_b = DISABLE;
    }
    /* Disable the comparator2 interrupt */
    else {
      ULPCLK->ULP_TA_PERI_RESET_REG_b.COMP2_OUTPUT_CNTRL_b = DISABLE;
    }
  }
  return RSI_OK;
}

/*==============================================*/
/**
 * @fn           rsi_error_t RSI_COMP_PinMux(uint8_t comp_number,uint8_t pos_pin,uint8_t neg_pin)
 * @brief        This API configure the required pin in analog for comparator operation.  
 * @param[in]    comp_number : comparator no to be enabled or disabled
 *                             - 1:for comparator 1 
 *                             - 2:for comparator 2                           
 * @param[in] 	 pos_pin     : positive input pin selection.
 * @param[in] 	 neg_pin     : negative input pin selection.
 * @return       - \ref RSI_OK If valid comp
 *                   else if fail return Invalid parameters.
 *       
 * @b Example    
 *     - RSI_COMP_PinMux(COMPARATOR_1,sel_p_mux,sel_n_mux)
 */
rsi_error_t RSI_COMP_PinMux(uint8_t comp_number, uint8_t pos_pin, uint8_t neg_pin)
{
  if ((pos_pin == 0) || (pos_pin == 1)) {
    if (comp_number == COMP1) {
      if (pos_pin) {
        RSI_EGPIO_PadReceiverDisable(AGPIO_PIN5);
        RSI_EGPIO_SetPinMux(EGPIO1, AGPIO_PORT, AGPIO_PIN5, AGPIO_MODE); /* non-inverting external input pin */
      } else {
        RSI_EGPIO_PadReceiverDisable(AGPIO_PIN0);
        RSI_EGPIO_SetPinMux(EGPIO1, AGPIO_PORT, AGPIO_PIN0, AGPIO_MODE); /* non-inverting external input pin */
      }
    } else {
      if (pos_pin) {
        RSI_EGPIO_PadReceiverDisable(TGPIO_PIN2);
        RSI_EGPIO_HostPadsGpioModeDisable(TGPIO_PIN2);
        RSI_EGPIO_PadSdioConnected();
        RSI_EGPIO_SetPinMux(EGPIO, 0, TGPIO_PIN2, TGPIO_MODE);
      } else {
        RSI_EGPIO_PadReceiverDisable(AGPIO_PIN2);
        RSI_EGPIO_SetPinMux(EGPIO1, AGPIO_PORT, AGPIO_PIN2, AGPIO_MODE); /* non-inverting external input pin */
      }
    }
  }
  if ((neg_pin == 0) || (neg_pin == 1)) {
    if (comp_number == COMP1) {
      if (neg_pin) {
        RSI_EGPIO_PadReceiverDisable(AGPIO_PIN4);
        RSI_EGPIO_SetPinMux(EGPIO1, AGPIO_PORT, AGPIO_PIN4, AGPIO_MODE); /* inverting external input pin */
      } else {
        RSI_EGPIO_PadReceiverDisable(AGPIO_PIN1);
        RSI_EGPIO_SetPinMux(EGPIO1, AGPIO_PORT, AGPIO_PIN1, AGPIO_MODE); /* inverting external input pin */
      }
    } else {
      if (neg_pin) {
        RSI_EGPIO_PadReceiverDisable(TGPIO_PIN3);
        RSI_EGPIO_HostPadsGpioModeDisable(TGPIO_PIN3);
        RSI_EGPIO_PadSdioConnected();
        RSI_EGPIO_SetPinMux(EGPIO, 0, TGPIO_PIN3, TGPIO_MODE);
      } else {
        RSI_EGPIO_PadReceiverDisable(AGPIO_PIN3);
        RSI_EGPIO_SetPinMux(EGPIO1, AGPIO_PORT, AGPIO_PIN3, AGPIO_MODE); /* inverting external input pin */
      }
    }
  }

  return RSI_OK;
}
/** @} */
