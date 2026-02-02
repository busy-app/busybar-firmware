#pragma once

#include <stdbool.h>
#include <rsi_pll.h>

#ifdef __cplusplus
extern "C" {
#endif

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
    uint32_t divFactor);

#ifdef __cplusplus
}
#endif
