#include <furi_hal_bus.h>
#include <furi.h>

#include <rsi_rom_clks.h>
#include <rsi_rom_ulpss_clk.h>

#define FURI_HAL_BUS_IGNORE (0x0U)

static const uint32_t furi_hal_bus[] = {
    /* Set 1*/
    [FuriHalBusUSART1_PCLK] = USART1_PCLK_ENABLE,
    [FuriHalBusUSART1_SCLK] = USART1_SCLK_ENABLE,
    [FuriHalBusUSART2_PCLK] = USART2_PCLK_ENABLE,
    [FuriHalBusUSART2_SCLK] = USART2_SCLK_ENABLE,
#ifdef SLI_SI917B0
    [FuriHalBusQSPI_2_CLK] = QSPI_2_CLK_ENABLE,
    [FuriHalBusQSPI_2_HCLK] = QSPI_2_HCLK_ENABLE,
    [FuriHalBusQSPI_2_M4_SOC_SYNC] = QSPI_2_M4_SOC_SYNC,
    [FuriHalBusQSPI_2_CLK_ONEHOT] = QSPI_2_CLK_ONEHOT_ENABLE,
#endif
    [FuriHalBusCT_CLK] = CT_CLK_ENABLE,
    [FuriHalBusCT_PCLK] = CT_PCLK_ENABLE,
    [FuriHalBusICACHE_CLK] = ICACHE_CLK_ENABLE,
    [FuriHalBusICACHE_CLK_2X] = ICACHE_CLK_2X_ENABLE,
    [FuriHalBusRPDMA_HCLK] = RPDMA_HCLK_ENABLE,
    [FuriHalBusSOC_PLL_SPI_CLK] = SOC_PLL_SPI_CLK_ENABLE,
    [FuriHalBusIID_CLK] = IID_CLK_ENABLE,
    [FuriHalBusSDIO_SYS_HCLK] = SDIO_SYS_HCLK_ENABLE,
    [FuriHalBusCRC_CLK_M4] = CRC_CLK_ENABLE_M4,
    [FuriHalBusM4SS_UM_CLK_STATIC_EN] = M4SS_UM_CLK_STATIC_EN,
    [FuriHalBusETH_HCLK] = ETH_HCLK_ENABLE,
    [FuriHalBusHWRNG_PCLK] = HWRNG_PCLK_ENABLE,
    [FuriHalBusGNSS_MEM_CLK] = GNSS_MEM_CLK_ENABLE,
    [FuriHalBusCCI_PCLK] = CCI_PCLK_ENABLE,
    [FuriHalBusCCI_HCLK] = CCI_HCLK_ENABLE,
    [FuriHalBusCCI_CLK] = CCI_CLK_ENABLE,
    [FuriHalBusMASK_HOST_CLK_WAIT_FIX] = MASK_HOST_CLK_WAIT_FIX,
    [FuriHalBusMASK31_HOST_CLK_CNT] = MASK31_HOST_CLK_CNT,
    [FuriHalBusSD_MEM_INTF_CLK] = SD_MEM_INTF_CLK_ENABLE,
    [FuriHalBusMASK_HOST_CLK_AVAILABLE_FIX] = MASK_HOST_CLK_AVAILABLE_FIX,
    [FuriHalBusULPSS_CLK] = ULPSS_CLK_ENABLE,

    /* Set 2*/
    [FuriHalBusGEN_SPI_MST1_HCLK] = GEN_SPI_MST1_HCLK_ENABLE,
    [FuriHalBusCAN1_PCLK] = CAN1_PCLK_ENABLE,
    [FuriHalBusCAN1_CLK] = CAN1_CLK_ENABLE,
    [FuriHalBusUDMA_HCLK] = UDMA_HCLK_ENABLE,
    [FuriHalBusI2C_BUS_CLK] = I2C_BUS_CLK_ENABLE,
    [FuriHalBusI2C_2_BUS_CLK] = I2C_2_BUS_CLK_ENABLE,
    [FuriHalBusSSI_SLV_PCLK] = SSI_SLV_PCLK_ENABLE,
    [FuriHalBusSSI_SLV_SCLK] = SSI_SLV_SCLK_ENABLE,
    [FuriHalBusQSPI_CLK] = QSPI_CLK_ENABLE,
    [FuriHalBusQSPI_HCLK] = QSPI_HCLK_ENABLE,
    [FuriHalBusI2SM_SCLK] = I2SM_SCLK_ENABLE,
    [FuriHalBusI2SM_INTF_SCLK] = I2SM_INTF_SCLK_ENABLE,
    [FuriHalBusI2SM_PCLK] = I2SM_PCLK_ENABLE,
    [FuriHalBusQEI_PCLK] = QE_PCLK_ENABLE,
    [FuriHalBusMCPWM_PCLK] = MCPWM_PCLK_ENABLE,
    [FuriHalBusSGPIO_PCLK] = SGPIO_PCLK_ENABLE,
    [FuriHalBusEGPIO_PCLK] = EGPIO_PCLK_ENABLE,
    [FuriHalBusARM_CLK] = ARM_CLK_ENABLE,
    [FuriHalBusSSI_MST_PCLK] = SSI_MST_PCLK_ENABLE,
    [FuriHalBusSSI_MST_SCLK] = SSI_MST_SCLK_ENABLE,
    [FuriHalBusMEM2_CLK] = MEM2_CLK_ENABLE,
    [FuriHalBusMEM_CLK_ULP] = MEM_CLK_ULP_ENABLE,
    [FuriHalBusROM_CLK] = ROM_CLK_ENABLE,
    [FuriHalBusPLL_INTF_CLK] = PLL_INTF_CLK_ENABLE,
    [FuriHalBusSEMAPHORE_CLK] = SEMAPHORE_CLK_ENABLE,
    [FuriHalBusTOT_CLK] = TOT_CLK_ENABLE,
    [FuriHalBusRMII_SOFT_RESET] = RMII_SOFT_RESET,

    /* Set 3*/
    [FuriHalBusBUS_CLK] = BUS_CLK_ENABLE,
    [FuriHalBusM4_CORE_CLK] = M4_CORE_CLK_ENABLE,
    [FuriHalBusCM_BUS_CLK] = CM_BUS_CLK_ENABLE,
    [FuriHalBusMISC_CONFIG_PCLK] = MISC_CONFIG_PCLK_ENABLE,
    [FuriHalBusEFUSE_CLK] = EFUSE_CLK_ENABLE,
    [FuriHalBusICM_CLK] = ICM_CLK_ENABLE,
    [FuriHalBusMEM1_CLK] = MEM1_CLK_ENABLE,
    [FuriHalBusMEM3_CLK] = MEM3_CLK_ENABLE,
    [FuriHalBusUSB_PHY_CLK_IN] = USB_PHY_CLK_IN_ENABLE,
    [FuriHalBusQSPI_CLK_ONEHOT] = QSPI_CLK_ONEHOT_ENABLE,
    [FuriHalBusQSPI_M4_SOC_SYNC] = QSPI_M4_SOC_SYNC,
    [FuriHalBusEGPIO_CLK] = EGPIO_CLK_ENABLE,
    [FuriHalBusI2C_CLK] = I2C_CLK_ENABLE,
    [FuriHalBusI2C_2_CLK] = I2C_2_CLK_ENABLE,
    [FuriHalBusEFUSE_PCLK] = EFUSE_PCLK_ENABLE,
    [FuriHalBusSGPIO_CLK] = SGPIO_CLK_ENABLE,
    [FuriHalBusTASS_M4SS_64K_SWITCH_CLK] = TASS_M4SS_64K_SWITCH_CLK_ENABLE,
    [FuriHalBusTASS_M4SS_128K_SWITCH_CLK] = TASS_M4SS_128K_SWITCH_CLK_ENABLE,
    [FuriHalBusTASS_M4SS_SDIO_SWITCH_CLK] = TASS_M4SS_SDIO_SWITCH_CLK_ENABLE,
    [FuriHalBusTASS_M4SS_USB_SWITCH_CLK] = TASS_M4SS_USB_SWITCH_CLK_ENABLE,
    [FuriHalBusROM_MISC_STATIC] = ROM_MISC_STATIC_ENABLE,
    [FuriHalBusM4_SOC_CLK_FOR_OTHER] = M4_SOC_CLK_FOR_OTHER_ENABLE,
    [FuriHalBusICACHE] = ICACHE_ENABLE,

    /* ULP Section */
    [FuriHalBusUlpTOUCH_SENSOR_PCLK] = TOUCH_SENSOR_PCLK_ENABLE,
    [FuriHalBusUlpFIM_AHB_CLK] = FIM_AHB_CLK_ENABLE,
    [FuriHalBusUlpULPSS_TASS_QUASI_SYNC] = ULPSS_TASS_QUASI_SYNC,
    [FuriHalBusUlpULPSS_M4SS_SLV_SEL] = ULPSS_M4SS_SLV_SEL,
    [FuriHalBusUlpAUX_SOC_EXT_TRIG_2_SEL] = AUX_SOC_EXT_TRIG_2_SEL,
    [FuriHalBusUlpAUX_SOC_EXT_TRIG_1_SEL] = AUX_SOC_EXT_TRIG_1_SEL,
    [FuriHalBusUlpAUX_ULP_EXT_TRIG_2_SEL] = AUX_ULP_EXT_TRIG_2_SEL,
    [FuriHalBusUlpAUX_ULP_EXT_TRIG_1_SEL] = AUX_ULP_EXT_TRIG_1_SEL,
    [FuriHalBusUlpTIMER_PCLK_EN] = TIMER_PCLK_EN,
    [FuriHalBusUlpEGPIO_PCLK_EN] = EGPIO_PCLK_EN,
    [FuriHalBusUlpEGPIO_PCLK_DYN_CTRL_DISABLE_ULP] = EGPIO_PCLK_DYN_CTRL_DISABLE_ULP,
    [FuriHalBusUlpCLK_ULP_MEMORIES] = CLK_ENABLE_ULP_MEMORIES,
    [FuriHalBusUlpVAD_CLK_EN] = VAD_CLK_EN,
    [FuriHalBusUlpFIM_CLK_EN] = FIM_CLK_EN,
    [FuriHalBusUlpREG_ACCESS_SPI_CLK_EN] = REG_ACCESS_SPI_CLK_EN,
    [FuriHalBusUlpEGPIO_CLK_EN] = EGPIO_CLK_EN,
    [FuriHalBusUlpCLK_TIMER] = CLK_ENABLE_TIMER,
    [FuriHalBusUlpVAD_PCLK] = VAD_PCLK_ENABLE,
    [FuriHalBusUlpFIM_PCLK] = FIM_PCLK_ENABLE,
    [FuriHalBusUlpSCLK_UART] = SCLK_ENABLE_UART,
    [FuriHalBusUlpPCLK_UART] = PCLK_ENABLE_UART,
    [FuriHalBusUlpSCLK_SSI_MASTER] = SCLK_ENABLE_SSI_MASTER,
    [FuriHalBusUlpPCLK_SSI_MASTER] = PCLK_ENABLE_SSI_MASTER,
    [FuriHalBusUlpCLK_I2S] = CLK_ENABLE_I2S,
    [FuriHalBusUlpPCLK_I2C] = PCLK_ENABLE_I2C,
    [FuriHalBusUlpIR_PCLK_EN] = IR_PCLK_EN,
    [FuriHalBusUlpPCM_FSYNC_START] = PCM_FSYNC_START,
    [FuriHalBusUlpPCM] = PCM_ENABLE,
};

void furi_hal_bus_init_early(void) {
}

void furi_hal_bus_deinit_early(void) {
}

void furi_hal_bus_enable(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);

    const uint32_t value = furi_hal_bus[bus];

    if(value == FURI_HAL_BUS_IGNORE) {
        return;
    }

    FURI_CRITICAL_ENTER();

    if(bus < FuriHalBusGEN_SPI_MST1_HCLK) {
        if((M4CLK->CLK_ENABLE_SET_REG1 & value) == 0) M4CLK->CLK_ENABLE_SET_REG1 = value;
    } else if(bus < FuriHalBusBUS_CLK) {
        if((M4CLK->CLK_ENABLE_SET_REG2 & value) == 0) M4CLK->CLK_ENABLE_SET_REG2 = value;
    } else if(bus < FuriHalBusUlpTOUCH_SENSOR_PCLK) {
        if((M4CLK->CLK_ENABLE_SET_REG3 & value) == 0) M4CLK->CLK_ENABLE_SET_REG3 = value;
    } else {
        if((ULPCLK->ULP_MISC_SOFT_SET_REG & value) == 0) ULPCLK->ULP_MISC_SOFT_SET_REG |= value;
    }

    FURI_CRITICAL_EXIT();
}

void furi_hal_bus_reset(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);

    /* TODO: No known method of resetting a peripheral */
}

void furi_hal_bus_disable(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);

    const uint32_t value = furi_hal_bus[bus];

    if(value == FURI_HAL_BUS_IGNORE) {
        return;
    }

    FURI_CRITICAL_ENTER();

    if(bus < FuriHalBusGEN_SPI_MST1_HCLK) {
        furi_check((M4CLK->CLK_ENABLE_SET_REG1 & value) != 0);
        M4CLK->CLK_ENABLE_CLEAR_REG1 = value;
    } else if(bus < FuriHalBusBUS_CLK) {
        furi_check((M4CLK->CLK_ENABLE_SET_REG2 & value) != 0);
        M4CLK->CLK_ENABLE_CLEAR_REG2 = value;
    } else if(bus < FuriHalBusUlpTOUCH_SENSOR_PCLK) {
        furi_check((M4CLK->CLK_ENABLE_SET_REG3 & value) != 0);
        M4CLK->CLK_ENABLE_CLEAR_REG3 = value;
    } else {
        furi_check((ULPCLK->ULP_MISC_SOFT_SET_REG & value) != 0);
        ULPCLK->ULP_MISC_SOFT_SET_REG &= ~value;
    }

    FURI_CRITICAL_EXIT();
}

bool furi_hal_bus_is_enabled(FuriHalBus bus) {
    furi_check(bus < FuriHalBusMAX);

    const uint32_t value = furi_hal_bus[bus];

    bool ret = false;

    FURI_CRITICAL_ENTER();

    if(bus < FuriHalBusGEN_SPI_MST1_HCLK) {
        ret = M4CLK->CLK_ENABLE_SET_REG1 & value;
    } else if(bus < FuriHalBusBUS_CLK) {
        ret = M4CLK->CLK_ENABLE_SET_REG2 & value;
    } else if(bus < FuriHalBusUlpTOUCH_SENSOR_PCLK) {
        ret = M4CLK->CLK_ENABLE_SET_REG3 & value;
    } else {
        ret = ULPCLK->ULP_MISC_SOFT_SET_REG & value;
    }

    FURI_CRITICAL_EXIT();

    return ret;
}
