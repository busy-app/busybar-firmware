#include "furi_hal_qspi_flash.h"
//#include <furi.h>

/*==============================================*/
/**
 * @fn             uint32_t furi_hal_qspi_flash_check_present(const M4CLK_Type *pCLK  ,CLK_PRESENT_T clkPresent)
 * @brief		   This API is used to enable the dynamic clock gate for peripherals
 * @param[in]	   pCLK       : Pointer to the pll register instance \ref M4CLK_Type
 * @param[in]	   clkPresent : structure variable of CLK_PRESENT_T , \ref CLK_PRESENT_T
 * @return         zero on success
 *                 RSI_OK on error error code
 *                 ERROR_CLOCK_NOT_ENABLED
 */

uint32_t furi_hal_qspi_flash_check_present(const M4CLK_Type* pCLK, CLK_PRESENT_T clkPresent) {
    uint32_t errorReturn = 0;
    switch(clkPresent) {
    case SOC_PLL_CLK_PRESENT:
        if(pCLK->PLL_STAT_REG_b.SOCPLL_LOCK == 1) {
            errorReturn = RSI_OK;
        } else {
            errorReturn = ERROR_CLOCK_NOT_ENABLED;
        }
        break;
    case INTF_PLL_CLK_PRESENT:
        if(pCLK->PLL_STAT_REG_b.INTFPLL_LOCK == 1) {
            errorReturn = RSI_OK;
        } else {
            errorReturn = ERROR_CLOCK_NOT_ENABLED;
        }
        break;
    case I2S_PLL_CLK_PRESENT:
        if(pCLK->PLL_STAT_REG_b.I2SPLL_LOCK == 1) {
            errorReturn = RSI_OK;
        } else {
            errorReturn = ERROR_CLOCK_NOT_ENABLED;
        }
        break;
    case MODEM_PLL_CLK_PRESENT:
        if(pCLK->PLL_STAT_REG_b.MODEMPLL_LOCK == 1) {
            errorReturn = RSI_OK;
        } else {
            errorReturn = ERROR_CLOCK_NOT_ENABLED;
        }
        break;
    }
    return errorReturn;
}
/*==============================================*/
/**
 * @fn       rsi_error_t furi_hal_qspi_flash_clk_config(M4CLK_Type *pCLK,
 *                          QSPI_CLK_SRC_SEL_T clkSource,
 *                          boolean_t swalloEn,
 *                          boolean_t OddDivEn,
 *                          uint32_t divFactor)
 * @brief    This API is used to configure the Qspi clocks 
 * @param[in] pCLK : pointer to the processor clock source
 * @param[in] clkSource : clock source for configure the Qspi clocks
 * @param[in] swalloEn : enable for Qspi clocks
 * @param[in] oddDivEn : enable for Qspi clocks
 * @param[in] divFactor : division factor for Qspi clocks
 * @return    clock spi on success           
 */

rsi_error_t furi_hal_qspi_flash_clk_config(
    M4CLK_Type* pCLK,
    QSPI_CLK_SRC_SEL_T clkSource,
    boolean_t swalloEn,
    boolean_t OddDivEn,
    uint32_t divFactor) {
    rsi_error_t errorCode = RSI_OK;
    /*Parameter validation */
    if((pCLK == NULL) || (divFactor > QSPI_MAX_CLK_DIVISION_FACTOR)) {
        return INVALID_PARAMETERS;
    }

    /*disabling the clocks*/
    //clk_peripheral_clk_disable(pCLK, QSPI_CLK);
    pCLK->CLK_ENABLE_CLEAR_REG2 = QSPI_CLK_ENABLE;
    pCLK->CLK_ENABLE_CLEAR_REG3 = (QSPI_CLK_ONEHOT_ENABLE | QSPI_M4_SOC_SYNC);
    /*Select clock MUX*/
    switch(clkSource) {
    case QSPI_ULPREFCLK:
        pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = clkSource;
        break;

    case QSPI_INTFPLLCLK:
        /*Check clock is present is or not before switching*/
        if(RSI_OK != furi_hal_qspi_flash_check_present(pCLK, INTF_PLL_CLK_PRESENT)) {
            errorCode = ERROR_CLOCK_NOT_ENABLED;
            break;
        } /*Update the clock MUX*/
        pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = 0x01;
        break;

    case QSPI_MODELPLLCLK2:
        /*Check clock is present is or not before switching*/
        if(RSI_OK != furi_hal_qspi_flash_check_present(pCLK, MODEM_PLL_CLK_PRESENT)) {
            errorCode = ERROR_CLOCK_NOT_ENABLED;
            break;
        }
        /*Update the clock MUX*/
        pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = 0x02;
        break;

    case QSPI_SOCPLLCLK:
        /*Check clock is present is or not before switching*/
        if(RSI_OK != furi_hal_qspi_flash_check_present(pCLK, SOC_PLL_CLK_PRESENT)) {
            errorCode = ERROR_CLOCK_NOT_ENABLED;
            break;
        }
        pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SEL = clkSource;
        break;

    case M4_SOCCLKNOSWLSYNCCLKTREEGATED:
        /*incase of qspi in sync with soc*/
        pCLK->CLK_ENABLE_SET_REG3 = QSPI_M4_SOC_SYNC;
        break;

    default:
        errorCode = INVALID_PARAMETERS;
        break;
    }
    if(errorCode == RSI_OK) {
        /*wait for QSPI clock switched */
        while((pCLK->PLL_STAT_REG_b.QSPI_CLK_SWITCHED) != true)
            ;

        /*update the division factor */
        pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_DIV_FAC = (unsigned int)(divFactor & 0x3F);
        /*Specifies whether QSPI clock is in sync with Soc clock.
	  Before enabling this make sure that qspi_clk_onehot_enable is 1\92b0 to enable glitch free switching*/
        /*Enable the QSPI clock*/
        pCLK->CLK_CONFIG_REG1_b.QSPI_CLK_SWALLOW_SEL = swalloEn ? ENABLE : DISABLE;
        pCLK->CLK_CONFIG_REG2_b.QSPI_ODD_DIV_SEL = OddDivEn ? ENABLE : DISABLE;
    }
    //clk_peripheral_clk_enable(pCLK, QSPI_CLK, ENABLE_STATIC_CLK);
    pCLK->CLK_ENABLE_SET_REG2 = (QSPI_CLK_ENABLE | QSPI_HCLK_ENABLE);
    pCLK->CLK_ENABLE_SET_REG3 = QSPI_CLK_ONEHOT_ENABLE;
    return errorCode;
}
