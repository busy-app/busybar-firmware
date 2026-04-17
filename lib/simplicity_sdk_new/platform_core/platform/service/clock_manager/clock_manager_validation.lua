local hfxo_enable = slc.config("SL_CLOCK_MANAGER_HFXO_EN")
local lfxo_enable = slc.config("SL_CLOCK_MANAGER_LFXO_EN")
local dpll_enable = slc.config("SL_CLOCK_MANAGER_HFRCO_DPLL_EN")
local dpll_refclk = slc.config("SL_CLOCK_MANAGER_DPLL_REFCLK")
local lfrco_precision_enable = slc.config("SL_CLOCK_MANAGER_LFRCO_PRECISION")
local hf_default_clock_source = slc.config("SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE")
local lf_default_clock_source = slc.config("SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE")
local eusart0_lf_default_clock_source = slc.config("SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE")
local sysclk_source = slc.config("SL_CLOCK_MANAGER_SYSCLK_SOURCE")
local em01grpaclk_source = slc.config("SL_CLOCK_MANAGER_EM01GRPACLK_SOURCE")
local em01grpbclk_source = slc.config("SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE")
local em01grpcclk_source = slc.config("SL_CLOCK_MANAGER_EM01GRPCCLK_SOURCE")
local em01grpdclk_source = slc.config("SL_CLOCK_MANAGER_EM01GRPDCLK_SOURCE")
local em23grpaclk_source = slc.config("SL_CLOCK_MANAGER_EM23GRPACLK_SOURCE")
local em4grpaclk_source = slc.config("SL_CLOCK_MANAGER_EM4GRPACLK_SOURCE")
local pixelrzclk_source = slc.config("SL_CLOCK_MANAGER_PIXELRZCLK_SOURCE")
local wdog0clk_source = slc.config("SL_CLOCK_MANAGER_WDOG0CLK_SOURCE")
local wdog1clk_source = slc.config("SL_CLOCK_MANAGER_WDOG1CLK_SOURCE")
local rtccclk_source = slc.config("SL_CLOCK_MANAGER_RTCCCLK_SOURCE")
local sysrtcclk_source = slc.config("SL_CLOCK_MANAGER_SYSRTCCLK_SOURCE")
local lcdclk_source = slc.config("SL_CLOCK_MANAGER_LCDCLK_SOURCE")
local eusartclk_source = slc.config("SL_CLOCK_MANAGER_EUSART0CLK_SOURCE")
local i2c0clk_source = slc.config("SL_CLOCK_MANAGER_I2C0CLK_SOURCE")
local hfxo_crystal_sharing = slc.config("SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_EN")
local hfxo_mode = slc.config("SL_CLOCK_MANAGER_HFXO_MODE")
local hfxo_freq = slc.config("SL_CLOCK_MANAGER_HFXO_FREQ")
local ctune_mfg_hfxo_enable = slc.config("SL_CLOCK_MANAGER_CTUNE_MFG_HFXO_EN")
local hfxo_separate_ctune_enable = slc.config("SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN")
local socpll_enable = slc.config("SL_CLOCK_MANAGER_SOCPLL_EN")
local socpll_refclk = slc.config("SL_CLOCK_MANAGER_SOCPLL_REFCLK")
local socpll_fraq = slc.config("SL_CLOCK_MANAGER_SOCPLL_FRACTIONAL_EN")
local socpll_divf = slc.config("SL_CLOCK_MANAGER_SOCPLL_DIVF")
local socpll_divn = slc.config("SL_CLOCK_MANAGER_SOCPLL_DIVN")
local socpll_freq = slc.config("SL_CLOCK_MANAGER_SOCPLL_FREQ")
local clkin0_freq = slc.config("SL_CLOCK_MANAGER_CLKIN0_FREQ")
local qspi_advanced_config_enable = slc.config("SL_CLOCK_MANAGER_QSPICLK_ADVANCED_CONFIG_EN")
local qspi_custom_freq = slc.config("SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ")
local ext_flash_max_freq = slc.config("SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ")
local socpll_advanced_settings = slc.config("SL_CLOCK_MANAGER_SOCPLL_ADVANCED_SETTINGS")
local hclk_divider = slc.config("SL_CLOCK_MANAGER_HCLK_DIVIDER")
local pclk_divider = slc.config("SL_CLOCK_MANAGER_PCLK_DIVIDER")
local hfrcoem23_band = slc.config("SL_CLOCK_MANAGER_HFRCOEM23_BAND")
local hfrco_band = slc.config("SL_CLOCK_MANAGER_HFRCO_BAND")
local dpll_freq = slc.config("SL_CLOCK_MANAGER_DPLL_FREQ")
local rffpll_freq = slc.config("SL_CLOCK_MANAGER_RFFPLL_FREQ")
local max_freq_is_38m = slc.is_provided("device_has_hfrco_38m")
local is_series_2 = slc.is_provided("device_series_2")
local is_efr32xg21 = slc.is_provided("device_generic_family_efr32xg21")
local is_efr32xg22 = slc.is_provided("device_generic_family_efr32xg22")
local is_efr32xg23 = slc.is_provided("device_generic_family_efr32xg23")
local is_efr32xg24 = slc.is_provided("device_generic_family_efr32xg24")
local is_efr32xg25 = slc.is_provided("device_generic_family_efr32xg25")
local is_efr32xg26 = slc.is_provided("device_generic_family_efr32xg26")
local is_efr32xg27 = slc.is_provided("device_generic_family_efr32xg27")
local is_efr32xg28 = slc.is_provided("device_generic_family_efr32xg28")
local is_efr32xg29 = slc.is_provided("device_generic_family_efr32xg29")
local is_efr32xg2b = slc.is_provided("device_generic_family_efr32xg2b")
local is_efr32xg2d = slc.is_provided("device_generic_family_efr32xg2d")
local is_sixx301 = slc.is_provided("device_generic_family_sixx301")

-- HELPER FUNCTIONS --

-- Convert HFRCO band enum to frequency for Series 2 devices
local function hfrco_band_to_frequency(band_config)
  local freq = string.match(band_config, "(%d+)M")

  if freq ~= nil then
    return tonumber(freq) * 1000000
  else
    return tonumber(band_config)
  end
end

-- OSCILLATORS VALIDATION --
-- HFXO related
if hfxo_enable.value == "0" and dpll_enable.value == "1" and dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_HFXO" then
    validation.error(
    "DPLL module needs HFXO but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_REFCLK"}),
    nil,
    nil)
end
if lfrco_precision_enable ~= nil then
  if hfxo_enable.value == "0" and (lfrco_precision_enable.value == "1" or lfrco_precision_enable.value == "cmuPrecisionHigh") then
    validation.error(
    "LFRCO High Precision Mode is enabled and requires HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_LFRCO_PRECISION"}),
    nil,
    nil)
  end
end

-- HFXO CRYSTAL SHARING
if hfxo_crystal_sharing ~= nil then
  if hfxo_crystal_sharing.value == "1" then
    local hfxo_crystal_sharing_leader = slc.config("SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_LEADER_EN")
    local hfxo_crystal_sharing_follower = slc.config("SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_FOLLOWER_EN")
    if hfxo_crystal_sharing_leader.value == "1" and hfxo_crystal_sharing_follower.value == "1" then
      validation.error(
        "Crystal sharing cannot be enabled for both the leader and follower",
        validation.target_for_defines({"SL_CLOCK_MANAGER_HFXO_CRYSTAL_SHARING_FOLLOWER_EN"}),
        nil,
        nil)
    end

    if hfxo_crystal_sharing_follower.value == "1" and hfxo_mode.value == "HFXO_CFG_MODE_XTAL" then
      validation.error(
        "HFXO mode must be HFXO_CFG_MODE_EXTCLKPKDET or HFXO_CFG_MODE_EXTCLK when crystal sharing follower is enabled",
        validation.target_for_defines({"SL_CLOCK_MANAGER_HFXO_MODE"}),
        nil,
        nil)
    end
  end
end

-- HFXO SEPARATE CTUNE VALIDATION
if ctune_mfg_hfxo_enable ~= nil and hfxo_separate_ctune_enable ~= nil then
  if ctune_mfg_hfxo_enable.value == "1" and hfxo_separate_ctune_enable.value == "1" then
    validation.warning(
    "Both manufacturing CTUNE and separate XI/XO CTUNE are enabled. The separate CTUNE values will override the manufacturing CTUNE.",
    validation.target_for_defines({"SL_CLOCK_MANAGER_CTUNE_MFG_HFXO_EN", "SL_CLOCK_MANAGER_HFXO_SEPARATE_CTUNEXIANA_CTUNEXOANA_EN"}),
    nil,
    nil)
  end
end

-- LFXO related
if lfxo_enable.value == "0" and dpll_enable.value == "1" and dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_LFXO" then
  validation.error(
  "DPLL module needs LFXO but SL_CLOCK_MANAGER_LFXO_EN is disabled",
  validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_REFCLK"}),
  nil,
  nil)
end

-- CLKIN0 related
if (sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_CLKIN0") or (em01grpbclk_source ~= nil and em01grpbclk_source.value == "CMU_EM01GRPBCLKCTRL_CLKSEL_CLKIN0") or (dpll_enable.value == "1" and dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0") then
  if slc.config("SL_CLOCK_MANAGER_CLKIN0_PORT") == "SL_GPIO_PORT_A" or slc.config("SL_CLOCK_MANAGER_CLKIN0_PORT") == "SL_GPIO_PORT_B" then
    validation.error(
    "CLKIN0 is not supported on Ports A and B. Only Ports C and D",
    validation.target_for_defines({"SL_CLOCK_MANAGER_CLKIN0_PORT"}),
    nil,
    nil)
  end
  if tonumber(clkin0_freq.value) < 1000000 or tonumber(clkin0_freq.value) > 38000000 then
    validation.warning(
    "Unsupported CLKIN0 frequency. It should be between 1MHz and 38MHz",
    validation.target_for_defines({"SL_CLOCK_MANAGER_CLKIN0_FREQ"}),
    nil,
    nil)
  end
end

-- HFRCO related
local hfrco_freq = hfrco_band_to_frequency(hfrco_band.value)
if max_freq_is_38m and hfrco_freq > 38000000 then
  validation.error(
  "HFRCO band frequency must not exceed 38MHz",
  validation.target_for_defines({"SL_CLOCK_MANAGER_HFRCO_BAND"}),
  nil,
  nil)
elseif is_series_2 and not is_efr32xg25 and hfrco_freq > 80000000 then
  validation.error(
  "HFRCO band frequency must not exceed 80MHz",
  validation.target_for_defines({"SL_CLOCK_MANAGER_HFRCO_BAND"}),
  nil,
  nil)
elseif hfrco_freq > 100000000 then
  validation.error(
  "HFRCO band frequency must not exceed 100MHz",
  validation.target_for_defines({"SL_CLOCK_MANAGER_HFRCO_BAND"}),
  nil,
  nil)
end

-- DPLL related
 if dpll_enable.value == "1" then
  local dpll_n = tonumber(slc.config("SL_CLOCK_MANAGER_DPLL_N").value)
  local dpll_m = tonumber(slc.config("SL_CLOCK_MANAGER_DPLL_M").value)
  local dpll_freq_expected = tonumber(dpll_freq.value)
  local dpll_refclk_freq
  if dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_HFXO" then
    dpll_refclk_freq = tonumber(hfxo_freq.value)
  elseif dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_LFXO" then
    dpll_refclk_freq = 32768
  elseif dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_CLKIN0" then
    dpll_refclk_freq = tonumber(slc.config("SL_CLOCK_MANAGER_CLKIN0_FREQ").value)
  elseif dpll_refclk.value == "CMU_DPLLREFCLKCTRL_CLKSEL_DISABLED" then
    validation.error(
      "DPLL module is enabled but no Reference clock is selected",
      validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_REFCLK"}),
      nil,
      nil)
  end
  if dpll_refclk_freq ~= nil then
    -- check formula validation: dpll_freq = hfxo_freq * (N+1)/(M+1)
    local dpll_freq = dpll_refclk_freq * (dpll_n + 1) / (dpll_m + 1)
    if dpll_freq ~= dpll_freq_expected then
      validation.error(
      "Target frequency is not reachable based on DPLL settings and Reference clock",
      validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_FREQ"}),
      nil,
      nil)
    end
  end
  if max_freq_is_38m and dpll_freq_expected > 38000000 then
    validation.error(
    "DPLL frequency must not exceed 38MHz",
    validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_FREQ"}),
    nil,
    nil)
  elseif is_series_2 and dpll_freq_expected > 80000000 then
    validation.error(
    "DPLL frequency must not exceed 80MHz",
    validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_FREQ"}),
    nil,
    nil)
  elseif dpll_freq_expected > 100000000 then
    validation.error(
    "DPLL frequency must not exceed 100MHz",
    validation.target_for_defines({"SL_CLOCK_MANAGER_DPLL_FREQ"}),
    nil,
    nil)
  end
 end

 -- SOCPLL related
 if socpll_refclk ~= nil and socpll_enable ~= nil and socpll_enable.value == "1" then
  local socpll_refclk_freq
  if socpll_refclk.value == "SOCPLL_CTRL_REFCLKSEL_REF_HFXO" then
    if hfxo_enable.value == "0" then
      validation.error(
      "SOCPLL module needs HFXO but SL_CLOCK_MANAGER_HFXO_EN is disabled",
      validation.target_for_defines({"SL_CLOCK_MANAGER_SOCPLL_REFCLK"}),
      nil,
      nil)
    end
    socpll_refclk_freq = tonumber(hfxo_freq.value)
  elseif socpll_refclk.value == "SOCPLL_CTRL_REFCLKSEL_REF_HFRCO" then
    if dpll_enable.value == "1" then
      socpll_refclk_freq = tonumber(dpll_freq.value)
    else
      socpll_refclk_freq = hfrco_band_to_frequency(hfrco_band.value)
    end
  end
  if socpll_refclk_freq ~= nil and socpll_advanced_settings ~= nil and socpll_advanced_settings.value == "1"
     and (socpll_refclk_freq < 34000000 or socpll_refclk_freq > 44000000) then
    validation.error(
      "SOCPLL reference clock frequency must be between 34MHz and 44MHz inclusively",
      validation.target_for_defines({"SL_CLOCK_MANAGER_SOCPLL_REFCLK"}),
      nil,
      nil)
  end
  if socpll_refclk_freq ~= nil and socpll_advanced_settings ~= nil and socpll_advanced_settings.value == "1"
     and socpll_fraq ~= nil and socpll_divf ~= nil and socpll_divn ~= nil then
    -- check formula validation: socpll_freq = Fref * (DIVN+2 + DIVF/1024) / 6
    local socpll_freq_expected = tonumber(socpll_freq.value)
    local socpll_divf_val = tonumber(socpll_divf.value)
    local socpll_divn_val = tonumber(socpll_divn.value)
    if socpll_fraq.value == "1" then
      socpll_freq = socpll_refclk_freq * (socpll_divn_val + 2 + socpll_divf_val/1024) / 6
    else
      socpll_freq = socpll_refclk_freq * (socpll_divn_val + 2) / 6
    end
    local socpll_max_ppm = 41
    local socpll_range_max = socpll_freq + socpll_max_ppm * socpll_freq/1000000
    local socpll_range_min = socpll_freq - socpll_max_ppm * socpll_freq/1000000
    if socpll_freq_expected < socpll_range_min or socpll_freq_expected > socpll_range_max then
      validation.error(
      "Target frequency is not reachable based on SOCPLL settings and Reference clock",
      validation.target_for_defines({"SL_CLOCK_MANAGER_SOCPLL_FREQ"}),
      nil,
      nil)
    end
  end
end

-- EXTERNAL FLASH
if ext_flash_max_freq ~= nil and ext_flash_max_freq.value == "0" then
  validation.error(
    "The part has an External Flash without SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ being set to the maximum supported frequency.",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ"}),
    nil,
    nil)
end

-- CLOCK BRANCHES VALIDATION --
-- SYSCLK
if sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "SYSCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_SYSCLK_SOURCE"}),
    nil,
    nil)
end
if sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS" and hfxo_enable.value == "0" then
  validation.error(
    "SYSCLK source branch is configured on RFFPLL, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_SYSCLK_SOURCE"}),
    nil,
    nil)
end
if sysclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" and hf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "SYSCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_SYSCLK_SOURCE"}),
    nil,
    nil)
end
if sysclk_source.value == "CMU_SYSCLKCTRL_CLKSEL_SOCPLL" and socpll_enable ~= nil and socpll_enable.value == "0" then
  validation.error(
    "SYSCLK source branch is configured on SOCPLL, but SL_CLOCK_MANAGER_SOCPLL_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_SYSCLK_SOURCE"}),
    nil,
    nil)
end

-- EM01GRACLK
if em01grpaclk_source.value == "CMU_EM01GRPACLKCTRL_CLKSEL_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPACLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPACLK_SOURCE"}),
    nil,
    nil)
end
if em01grpaclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" and hf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPACLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPACLK_SOURCE"}),
    nil,
    nil)
end

-- EM01GRPBCLK
if em01grpbclk_source ~= nil then
if em01grpbclk_source.value == "CMU_EM01GRPBCLKCTRL_CLKSEL_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPBCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE"}),
    nil,
    nil)
end
if em01grpbclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" and hf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPBCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPBCLK_SOURCE"}),
    nil,
    nil)
end
end

-- EM01GRPCCLK
if em01grpcclk_source ~= nil then
if em01grpcclk_source.value == "CMU_EM01GRPCCLKCTRL_CLKSEL_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPCCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPCCLK_SOURCE"}),
    nil,
    nil)
end
if em01grpcclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" and hf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPCCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPCCLK_SOURCE"}),
    nil,
    nil)
end
end

-- EM01GRPDCLK
if em01grpdclk_source ~= nil then
if em01grpdclk_source.value == "CMU_EM01GRPDCLKCTRL_CLKSEL_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPDCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPDCLK_SOURCE"}),
    nil,
    nil)
end
if em01grpdclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" and hf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "EM01GRPDCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM01GRPDCLK_SOURCE"}),
    nil,
    nil)
end
end

-- PIXELRZCLK
if pixelrzclk_source ~= nil then
if pixelrzclk_source.value == "CMU_PIXELRZCLKCTRL_CLKSEL_HFXO" and hfxo_enable.value == "0" then
  validation.error(
    "PIXELRZCLK source branch is configured on HFXO, but SL_CLOCK_MANAGER_HFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_PIXELRZCLK_SOURCE"}),
    nil,
    nil)
end
end

-- EM23GRPACLK
if em23grpaclk_source.value == "CMU_EM23GRPACLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "EM23GRPACLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM23GRPACLK_SOURCE"}),
    nil,
    nil)
end
if em23grpaclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "EM23GRACLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM23GRPACLK_SOURCE"}),
    nil,
    nil)
end

-- EM4GRPACLK
if em4grpaclk_source.value == "CMU_EM4GRPACLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "EM4GRPACLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM4GRPACLK_SOURCE"}),
    nil,
    nil)
end
if em4grpaclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "EM4GRACLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_EM4GRPACLK_SOURCE"}),
    nil,
    nil)
end

-- WDOG0CLK
if wdog0clk_source.value == "CMU_WDOG0CLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "WDOG0CLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_WDOG0CLK_SOURCE"}),
    nil,
    nil)
end
if wdog0clk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "WDOG0CLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_WDOG0CLK_SOURCE"}),
    nil,
    nil)
end

-- WDOG1CLK
if wdog1clk_source ~= nil then
if wdog1clk_source.value == "CMU_WDOG1CLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "WDOG1CLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_WDOG1CLK_SOURCE"}),
    nil,
    nil)
end
if wdog1clk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "WDOG1CLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_WDOG1CLK_SOURCE"}),
    nil,
    nil)
end
end

-- RTCCCLK
if rtccclk_source ~= nil then
if rtccclk_source.value == "CMU_RTCCCLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "RTCCCLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_RTCCCLK_SOURCE"}),
    nil,
    nil)
end
if rtccclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "RTCCCLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_RTCCCLK_SOURCE"}),
    nil,
    nil)
end
end

-- SYSRTCCLK
if sysrtcclk_source ~= nil then
if sysrtcclk_source.value == "CMU_SYSRTC0CLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "SYSRTCCLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_SYSRTCCLK_SOURCE"}),
    nil,
    nil)
end
if sysrtcclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "SYSRTCCLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_SYSRTCCLK_SOURCE"}),
    nil,
    nil)
end
end

-- LCDCLK
if lcdclk_source ~= nil then
if lcdclk_source.value == "CMU_LCDCLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "LCDCLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_LCDCLK_SOURCE"}),
    nil,
    nil)
end
if lcdclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_LFXO" and lfxo_enable.value == "0" then
  validation.error(
    "LCDCLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
    validation.target_for_defines({"SL_CLOCK_MANAGER_LCDCLK_SOURCE"}),
    nil,
    nil)
end
end

-- EUSART0CLK
if eusartclk_source ~= nil then
  if eusartclk_source.value == "CMU_EUSART0CLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
    validation.error(
      "EUSART0CLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
      validation.target_for_defines({"SL_CLOCK_MANAGER_EUSART0CLK_SOURCE"}),
      nil,
      nil)
  end
  if eusart0_lf_default_clock_source ~= nil then
    if eusart0_lf_default_clock_source.value ~= "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and eusart0_lf_default_clock_source.value ~= "CMU_EUSART0CLKCTRL_CLKSEL_EM23GRPACLK" then
      validation.error(
        "EUSART0CLK default LF clock source is modified, which is not supported",
        validation.target_for_defines({"SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE"}),
        nil,
        nil)
    end
    if eusartclk_source.value == "SL_CLOCK_MANAGER_DEFAULT_EUSART0_LF_CLOCK_SOURCE" then
      if (eusart0_lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE" and lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_ULFRCO") or eusart0_lf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_LF_CLOCK_SOURCE_ULFRCO" then
        validation.error(
          "EUSART0CLK source branch is configured on ULFRCO from default lf clock source, which is not supported",
          validation.target_for_defines({"SL_CLOCK_MANAGER_EUSART0CLK_SOURCE"}),
          nil,
          nil)
      end
    end
  end
end

-- I2C0CLK
if i2c0clk_source ~= nil then
  if i2c0clk_source.value == "CMU_I2C0CLKCTRL_CLKSEL_LFXO" and lfxo_enable.value == "0" then
    validation.error(
      "I2C0CLK source branch is configured on LFXO, but SL_CLOCK_MANAGER_LFXO_EN is disabled",
      validation.target_for_defines({"SL_CLOCK_MANAGER_I2C0CLK_SOURCE"}),
      nil,
      nil)
  end
  end

-- QSPICLK
if qspi_advanced_config_enable ~= nil then
  if qspi_advanced_config_enable.value == "1" then
    if tonumber(qspi_custom_freq.value) < 90000000 then
      validation.error(
        "QSPI advanced configuration are enabled and requires a QSPI custom frequency greater than 90MHz",
        validation.target_for_defines({"SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ"}),
        nil,
        nil)
    end
    if ext_flash_max_freq ~= nil and tonumber(qspi_custom_freq.value) > tonumber(ext_flash_max_freq.value) then
      validation.error(
        "QSPI advanced configuration are enabled and frequency must be equal or lower than the SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ configuration",
        validation.target_for_defines({"SL_CLOCK_MANAGER_QSPICLK_CUSTOM_FREQ"}),
        nil,
        nil)
    end
  end
end

-- FREQUENCY VALIDATION --

-- Define device-specific frequency thresholds
local device_thresholds = {}

-- Helper function to create Series 2 common thresholds
local function series2_thresholds(hclk_max_mhz, pclk_max_mhz)
  return {
    sysclk_min = nil,
    sysclk_max = hclk_max_mhz * 1000000,
    hclk_min = nil,
    hclk_max = hclk_max_mhz * 1000000,
    pclk_min = nil,
    pclk_max = pclk_max_mhz * 1000000,
    lspclk_min = nil,
    lspclk_max = nil
  }
end

-- SIXX301 (Series 3)
if is_sixx301 then
  device_thresholds = {
    sysclk_min = 2000000,      -- 2MHz
    sysclk_max = 150000000,    -- 150MHz
    hclk_min = 2000000,        -- 2MHz  
    hclk_max = 150000000,      -- 150MHz
    pclk_min = 2000000,        -- 2MHz
    pclk_max = 75000000,       -- 75MHz
    lspclk_min = 1000000,      -- 1MHz
    lspclk_max = 37500000,     -- 37.5MHz
  }

-- Series 2 devices
-- In some cases, the maximum HCLK or PCLK frequency is lower than the specified frequency below
-- to account for the HFRCO band used in the configurations. This validation is basing itself on the
-- maximum configuration value, but the datasheet numbers might be lower. For example, if the datasheet
-- indicates a 78 MHz maximum HCLK frequency, we'll use a maximum of 80 MHz, because it's the corresponding
-- HFRCO band.
elseif max_freq_is_38m then
  device_thresholds = series2_thresholds(38.4, 38.4)  -- HCLK < 38.4MHz, PCLK < 38.4MHz

elseif is_efr32xg21 or is_efr32xg22 then
  device_thresholds = series2_thresholds(80, 50)  -- HCLK < 80MHz, PCLK < 50MHz

elseif is_efr32xg23 or is_efr32xg24 or is_efr32xg26 or is_efr32xg27 or is_efr32xg28 or is_efr32xg29 or is_efr32xg2d then
  device_thresholds = series2_thresholds(80, 40)  -- HCLK < 80MHz, PCLK < 40MHz

elseif is_efr32xg25 then
  device_thresholds = series2_thresholds(100, 50)  -- HCLK < 100MHz, PCLK < 50MHz
end

-- Generic frequency calculation and validation function
local function calculate_and_validate_clocks()
  if not device_thresholds.sysclk_max and not device_thresholds.hclk_max and not device_thresholds.pclk_max then
    -- No thresholds defined for this device family, skip validation
    return
  end

  -- SYSCLK frequency calculation --
  local sysclk_freq
  local sysclk_source_val = sysclk_source.value
  
  if sysclk_source_val == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE" then
    if hf_default_clock_source.value == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_AUTO" then
      hf_default_clock_source = slc.config("SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_AUTO")
    end
    sysclk_source_val = hf_default_clock_source.value
  end
  
  if sysclk_source_val == "CMU_SYSCLKCTRL_CLKSEL_FSRCO" or sysclk_source_val == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_FSRCO" then
    sysclk_freq = 20000000
  elseif sysclk_source_val == "CMU_SYSCLKCTRL_CLKSEL_HFRCODPLL" or sysclk_source_val == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFRCODPLL" then
    if dpll_enable.value == "1" then
      sysclk_freq = tonumber(dpll_freq.value)
    else
      sysclk_freq = hfrco_band_to_frequency(hfrco_band.value)
    end
  elseif sysclk_source_val == "CMU_SYSCLKCTRL_CLKSEL_HFXO" or sysclk_source_val == "SL_CLOCK_MANAGER_DEFAULT_HF_CLOCK_SOURCE_HFXO" then
    sysclk_freq = tonumber(hfxo_freq.value)
  elseif sysclk_source_val == "CMU_SYSCLKCTRL_CLKSEL_CLKIN0" then
    sysclk_freq = tonumber(clkin0_freq.value)
  elseif sysclk_source_val == "CMU_SYSCLKCTRL_CLKSEL_SOCPLL" then
    sysclk_freq = tonumber(socpll_freq.value)
  elseif sysclk_source_val == "CMU_SYSCLKCTRL_CLKSEL_RFFPLL0SYS" then
    sysclk_freq = tonumber(rffpll_freq.value)
  else
    validation.error(
      "Invalid SYSCLK source selected",
      validation.target_for_defines({"SL_CLOCK_MANAGER_SYSCLK_SOURCE"}),
      nil,
      nil)
    return
  end

  -- SYSCLK validation
  if (device_thresholds.sysclk_min and sysclk_freq < device_thresholds.sysclk_min) or 
     (device_thresholds.sysclk_max and sysclk_freq > device_thresholds.sysclk_max) then
    local min_text = device_thresholds.sysclk_min and string.format("%gMHz", device_thresholds.sysclk_min/1000000) or "no minimum"
    local max_text = device_thresholds.sysclk_max and string.format("%gMHz", device_thresholds.sysclk_max/1000000) or "no maximum"
    local error_string = string.format("Unsupported SYSCLK frequency. It should be between %s and %s", min_text, max_text)
    if min_text == "no minimum" then
      error_string = string.format("Unsupported SYSCLK frequency. It should be lower than %s", max_text)
    end
    validation.warning(
      error_string,
      validation.target_for_defines({"SL_CLOCK_MANAGER_SYSCLK_SOURCE"}),
      nil,
      nil)
  end

  -- HCLK frequency calculation and validation --
  local hclk_divider_val = tonumber(string.match(hclk_divider.value, "%d+"))
  local hclk_freq = sysclk_freq / hclk_divider_val

  if (device_thresholds.hclk_min and hclk_freq < device_thresholds.hclk_min) or 
     (device_thresholds.hclk_max and hclk_freq > device_thresholds.hclk_max) then
    local min_text = device_thresholds.hclk_min and string.format("%gMHz", device_thresholds.hclk_min/1000000) or "no minimum"
    local max_text = device_thresholds.hclk_max and string.format("%gMHz", device_thresholds.hclk_max/1000000) or "no maximum"
    local error_string = string.format("Unsupported HCLK frequency. It should be between %s and %s", min_text, max_text)
    if min_text == "no minimum" then
      error_string = string.format("Unsupported HCLK frequency. It should be lower than %s", max_text)
    end
    validation.warning(
      error_string,
      validation.target_for_defines({"SL_CLOCK_MANAGER_HCLK_DIVIDER"}),
      nil,
      nil)
  end

  -- PCLK frequency calculation and validation --
  local pclk_freq
  if pclk_divider.value ~= "SL_CLOCK_MANAGER_PCLK_DIV_MIN" then
    local pclk_divider_val = tonumber(string.match(pclk_divider.value, "%d"))
    pclk_freq = hclk_freq / pclk_divider_val
  else
    -- In Minimal Divider mode, the PCLK divider is automatically calculated to respect the maximum frequency
    pclk_freq = hclk_freq / 1
    if device_thresholds.pclk_max and pclk_freq > device_thresholds.pclk_max then
      pclk_freq = hclk_freq / 2
      if pclk_freq > device_thresholds.pclk_max then
        validation.warning(
        "Unsupported PCLK frequency. No PCLK divider available to reach a valid PCLK frequency.",
        validation.target_for_defines({"SL_CLOCK_MANAGER_PCLK_DIVIDER"}),
        nil,
        nil)
      end
    end
  end
  
  if (device_thresholds.pclk_min and pclk_freq < device_thresholds.pclk_min) or 
     (device_thresholds.pclk_max and pclk_freq > device_thresholds.pclk_max) then
    local min_text = device_thresholds.pclk_min and string.format("%gMHz", device_thresholds.pclk_min/1000000) or "no minimum"
    local max_text = device_thresholds.pclk_max and string.format("%gMHz", device_thresholds.pclk_max/1000000) or "no maximum"
    local error_string = string.format("Unsupported PCLK frequency. It should be between %s and %s", min_text, max_text)
    if min_text == "no minimum" then
      error_string = string.format("Unsupported PCLK frequency. It should be lower than %s", max_text)
    end
    validation.warning(
      error_string,
      validation.target_for_defines({"SL_CLOCK_MANAGER_PCLK_DIVIDER"}),
      nil,
      nil)
  end

  -- LSPCLK validation --
  if device_thresholds.lspclk_min and device_thresholds.lspclk_max then
    local lspclk_divider_val = 2
    local lspclk_freq = pclk_freq / lspclk_divider_val
    if lspclk_freq < device_thresholds.lspclk_min or lspclk_freq > device_thresholds.lspclk_max then
      validation.warning(
        string.format("Unsupported LSPCLK frequency. It should be between %gMHz and %gMHz", 
          device_thresholds.lspclk_min/1000000, device_thresholds.lspclk_max/1000000),
        validation.target_for_defines({"SL_CLOCK_MANAGER_LSPCLK_DIVIDER"}),
        nil,
        nil)
    end
  end
end

-- Run frequency validation for supported device families
calculate_and_validate_clocks()
