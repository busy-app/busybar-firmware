local spidrv = slc.component("spidrv_usart")

-- Floor helper function
local function floor(x)
    return x - (x % 1)
end

-- Absolute value helper function  
local function abs(x)
    if x < 0 then
        return -x
    else
        return x
    end
end

-- Bitwise AND helper function
local function band(a, b)
    local result = 0
    local bitval = 1
    while a > 0 or b > 0 do
        if (a % 2 == 1) and (b % 2 == 1) then
            result = result + bitval
        end
        a = floor(a / 2)
        b = floor(b / 2)
        bitval = bitval * 2
    end
    return result
end

for k, v in pairs(spidrv.instances) do 
    local instance = string.upper(k)
    local spidrv_config_prefix = "SL_SPIDRV_USART_" .. instance 
    local str_cs_port = spidrv_config_prefix .. "_CS_PORT"
    local str_cs_control = spidrv_config_prefix .. "_CS_CONTROL"
    local str_spi_bitrate = spidrv_config_prefix .. "_BITRATE"
    
    -- Get configuration values
    local config_control = slc.config(str_cs_control)
    local config_cs = slc.config(str_cs_port)
    local spi_bitrate = tonumber(slc.config(str_spi_bitrate).value)
    
    -- pclk_freq = SYSCLK / HCLK_prescaler / PCLK_prescaler
    local pclk_freq = 0
    local sysclk_freq = 0
    local hclk_divider = 1
    local pclk_divider = 1
    
    -- Get HCLK and PCLK prescalers from clock manager
    local hclk_divider_config = slc.config("SL_CLOCK_MANAGER_HCLK_DIVIDER")
    local pclk_divider_config = slc.config("SL_CLOCK_MANAGER_PCLK_DIVIDER")
    
    -- Get SYSCLK frequency from clock manager
    local sysclk_source = slc.config("SL_CLOCK_MANAGER_SYSCLK_SOURCE")
    local dpll_enable = slc.config("SL_CLOCK_MANAGER_HFRCO_DPLL_EN")
    
    if sysclk_source then
        -- Handle default HF clock source resolution
        if sysclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" then
            sysclk_source = slc.config("SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE")
            if sysclk_source and sysclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_AUTO" then
                sysclk_source = slc.config("SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_AUTO")
            end
        end
        
        if sysclk_source then
            -- Determine SYSCLK frequency based on source
            if sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_FSRCO" or sysclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_FSRCO" then
                sysclk_freq = 20000000  -- FSRCO is 20MHz
            elseif sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL" or sysclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFRCODPLL" then
                if dpll_enable and dpll_enable.value == "1" then
                    local dpll_freq_config = slc.config("SL_CLOCK_MANAGER_DPLL_FREQ")
                    if dpll_freq_config then
                        sysclk_freq = tonumber(dpll_freq_config.value)
                    end
                else
                    local hfrco_band_config = slc.config("SL_CLOCK_MANAGER_HFRCO_BAND")
                    if hfrco_band_config and hfrco_band_config.value then
                        local mhz = string.match(hfrco_band_config.value, "(%d+)M")
                        if mhz then
                            sysclk_freq = tonumber(mhz) * 1000000
                        end
                    end
                end
            elseif sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_HFXO" or sysclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" then
                local hfxo_freq_config = slc.config("SL_CLOCK_MANAGER_HFXO_FREQ")
                if hfxo_freq_config then
                    sysclk_freq = tonumber(hfxo_freq_config.value)
                end
            elseif sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_CLKIN0" then
                local clkin0_freq_config = slc.config("SL_CLOCK_MANAGER_CLKIN0_FREQ")
                if clkin0_freq_config then
                    sysclk_freq = tonumber(clkin0_freq_config.value)
                end
            elseif sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS" then
                local rffpll_freq_config = slc.config("SL_CLOCK_MANAGER_RFFPLL_FREQ")
                if rffpll_freq_config then
                    sysclk_freq = tonumber(rffpll_freq_config.value)
                end
            end
        end
    end
    
    -- Extract prescaler values if available
    if hclk_divider_config and hclk_divider_config.value then
        hclk_divider = tonumber(string.match(hclk_divider_config.value, "DIV(%d+)")) or 1
    end
    if pclk_divider_config and pclk_divider_config.value then
        pclk_divider = tonumber(string.match(pclk_divider_config.value, "DIV(%d+)")) or 1
    end
    
    -- Calculate PCLK frequency: SYSCLK / HCLK_prescaler / PCLK_prescaler
    if sysclk_freq and sysclk_freq > 0 then
        pclk_freq = sysclk_freq / hclk_divider / pclk_divider
    end

    if pclk_freq and pclk_freq > 0 and spi_bitrate > pclk_freq / 2 then
        validation.warning(
        "SPIDRV USART " .. instance .. ": Bitrate (" .. spi_bitrate .. " Hz) is too high for the reference clock frequency (" .. pclk_freq .. " Hz) ",
        validation.target_for_defines({str_spi_bitrate}),
        "Set bitrate equal or lower than " .. pclk_freq / 2 .. " Hz",
        nil)

    else 
        if spi_bitrate ~= nil and spi_bitrate > 0 and pclk_freq ~= nil and pclk_freq > 0 then
            -- Calculate clock divider using USARTn_CLKDIV = 256 x (fPCLK/(2 x brdesired) - 1)
            local clkdiv = 256 * (pclk_freq/(2 * spi_bitrate) - 1)
            
            -- Round up to nearest integer and clear fractional part by removing the 5 CLKDIV LSB bits and the reserved 3 LSB bits
            clkdiv = band(clkdiv + 0xF8, 0xFFFFFF00)

            -- Calculate actual baudrate: br = fPCLK/(1 * (1 + (CLKDIV / 256)))
            local actual_baudrate = pclk_freq / (2 * (1 + (clkdiv / 256)))
            
            -- Issue warning if difference is more than 20%
            local diff_percent = abs(actual_baudrate - spi_bitrate) / spi_bitrate * 100
            if diff_percent > 20 then
                validation.warning(
                    "SPIDRV USART " .. instance .. ": Actual baudrate (" .. floor(actual_baudrate) .. " Hz) differs from configured bitrate (" .. spi_bitrate .. " Hz) by " .. floor(diff_percent * 10) / 10 .. "%",
                    validation.target_for_defines({str_spi_bitrate}),
                    "USART sync mode uses integer divisions of the reference clock. Consider adjusting the bitrate to a value that can be more accurately achieved, or change the reference clock frequency",
                    nil)
            end
        end
    end
    
    if (config_control.value == "spidrvCsControlAuto") and config_cs == nil then
        local msg = instance .. " : SPIDRV is configured to control CS, but no CS pin is selected"
        validation.error(msg,
                        validation.target_for_defines({str_cs_port}),
                        "CS must be controlled by the application, or a CS pin must be configured",
                        nil)
    end
end