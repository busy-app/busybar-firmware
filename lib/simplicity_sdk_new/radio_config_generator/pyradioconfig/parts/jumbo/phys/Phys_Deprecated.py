
#class PHYS_Deprecated(IPhy):


'''
def unknown_and_disabled_PHY_WMbus_Nabef_calc(self, model):
    phy = self._makePhy(model, model.profiles.Base, 'PHY WMbus Nabef')

    self.WMbus_common(phy, model)

    phy.profile_inputs.agc_hysteresis.value = 0
    phy.profile_inputs.agc_power_target.value = -22
    phy.profile_inputs.bandwidth_hz.value = 10977
    phy.profile_inputs.base_frequency_hz.value =  long(2405000000)
    phy.profile_inputs.baudrate_tol_ppm.value = 0
    phy.profile_inputs.bitrate.value = 4800
    phy.profile_inputs.channel_spacing_hz.value = 1000000
    phy.profile_inputs.deviation.value = 2400
    phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
    phy.profile_inputs.if_frequency_hz.value = 150000
    phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
    phy.profile_inputs.preamble_length.value = 16
    phy.profile_inputs.rx_xtal_error_ppm.value = 0
    phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
    phy.profile_inputs.shaping_filter_param.value = 0.5
    phy.profile_inputs.src_disable.value = True
    phy.profile_inputs.symbols_in_timing_window.value = 0
    phy.profile_inputs.syncword_0.value = long(0xf68d)
    phy.profile_inputs.syncword_1.value = long(0xf672)
    phy.profile_inputs.syncword_length.value = 16
    phy.profile_inputs.timing_detection_threshold.value = 16
    phy.profile_inputs.timing_resync_period.value = 0
    phy.profile_inputs.timing_sample_threshold.value = 0
    phy.profile_inputs.tx_xtal_error_ppm.value = 0

    phy.profile_outputs.AGC_CTRL2_ADCRSTSTARTUP.override = 0
    phy.profile_outputs.AGC_CTRL2_CFLOOPDEL.override = 40
    phy.profile_outputs.AGC_CTRL2_FASTLOOPDEL.override = 1
    phy.profile_outputs.AGC_GAINSTEPLIM_CFLOOPSTEPMAX.override = 0
    phy.profile_outputs.AGC_GAINSTEPLIM_SLOWDECAYCNT.override = 0
    phy.profile_outputs.AGC_LOOPDEL_IFPGADEL.override = 1
    phy.profile_outputs.AGC_LOOPDEL_LNASLICESDEL.override = 1
    phy.profile_outputs.AGC_LOOPDEL_PKDWAIT.override = 2
    #phy.profile_outputs.AGC_MANGAIN_MANGAINLNAATTEN.override = 0
    phy.profile_outputs.MODEM_CF_CFOSR.override = 4
    phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 1
    phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 0
    phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override = 2
    phy.profile_outputs.MODEM_CTRL4_ADCSATLEVEL.override = 1
    phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXFREQ.override = 0
    phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXMODE.override = 0
    phy.profile_outputs.MODEM_MODINDEX_MODINDEXM.override = 26
    phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
    phy.profile_outputs.MODEM_TIMING_FDM0THRESH.override = 0
    phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 0
    phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0
    phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 1
    phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 1000

def unknown_and_disabled_PHY_WMbus_Nabef_oneshotafc_dsafoest(self, model):
    phy = self._makePhy(model, model.profiles.Base, 'PHY WMbus Nabef with 1-shot AFC enable with freq offset from DSA')

    self.WMbus_common(phy, model)

    phy.profile_inputs.agc_power_target.value = -22
    phy.profile_inputs.bandwidth_hz.value = 10000
    phy.profile_inputs.base_frequency_hz.value =  long(2405000000)
    phy.profile_inputs.baudrate_tol_ppm.value = 100
    phy.profile_inputs.bitrate.value = 4800
    phy.profile_inputs.channel_spacing_hz.value = 1000000
    phy.profile_inputs.deviation.value = 2400
    phy.profile_inputs.if_frequency_hz.value = 150000
    phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
    phy.profile_inputs.preamble_length.value = 16
    phy.profile_inputs.rx_xtal_error_ppm.value = 0
    phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
    phy.profile_inputs.shaping_filter_param.value = 0.5
    phy.profile_inputs.src_disable.value = True
    phy.profile_inputs.symbols_in_timing_window.value = 6
    phy.profile_inputs.syncword_0.value = long(0xf68d)
    phy.profile_inputs.syncword_length.value = 16
    phy.profile_inputs.timing_detection_threshold.value = 12
    phy.profile_inputs.timing_resync_period.value = 0
    phy.profile_inputs.timing_sample_threshold.value = 0
    phy.profile_inputs.tx_xtal_error_ppm.value = 0

    phy.profile_outputs.AGC_GAINSTEPLIM_CFLOOPSTEPMAX.override = 0
    phy.profile_outputs.AGC_GAINSTEPLIM_SLOWDECAYCNT.override = 0
    phy.profile_outputs.MODEM_AFC_AFCDEL.override = 2
    phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 4
    phy.profile_outputs.MODEM_AFC_AFCSCALEE.override = 11
    phy.profile_outputs.MODEM_AFC_AFCSCALEM.override = 17
    phy.profile_outputs.MODEM_AFCADJLIM_AFCADJLIM.override = 150
    phy.profile_outputs.MODEM_CTRL4_ADCSATLEVEL.override = 1
    phy.profile_outputs.MODEM_DSACTRL_AGCBAUDEN.override = 1
    phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 3
    phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 3
    phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 150 #120 removes floor (150 - 0.2% per)
    phy.profile_outputs.MODEM_DSATHD0_FDEVMINTHD.override = 25
    phy.profile_outputs.MODEM_DSATHD1_AMPFLTBYP.override = 0
    phy.profile_outputs.MODEM_DSATHD1_DSARSTCNT.override = 6
    phy.profile_outputs.MODEM_DSATHD1_FREQSCALE.override = 1
    phy.profile_outputs.MODEM_DSATHD1_POWABSTHD.override = 200
    phy.profile_outputs.MODEM_DSATHD1_POWRELTHD.override = 3
    phy.profile_outputs.MODEM_DSATHD1_RSSIJMPTHD.override = 9
    phy.profile_outputs.MODEM_MODINDEX_MODINDEXM.override = 26
    phy.profile_outputs.MODEM_TIMING_FDM0THRESH.override = 0
    phy.profile_outputs.MODEM_TIMING_TIMINGBASES.override = 0
    phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 1
    phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 1000
    phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 0
    phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 450


def unknown_and_disabled_PHY_WMbus_Nabef_oneshotafc_legacyfoest(self, model):
    phy = self._makePhy(model, model.profiles.Base, 'PHY WMbus Nabef with 1-shot AFC enable with freq offset from legacy')

    self.WMbus_common(phy, model)

    phy.profile_inputs.agc_hysteresis.value = 0
    phy.profile_inputs.agc_power_target.value = -22
    phy.profile_inputs.bandwidth_hz.value = 10977
    phy.profile_inputs.base_frequency_hz.value =  long(2405000000)
    phy.profile_inputs.baudrate_tol_ppm.value = 100
    phy.profile_inputs.bitrate.value = 4800
    phy.profile_inputs.channel_spacing_hz.value = 1000000
    phy.profile_inputs.deviation.value = 2400
    phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
    phy.profile_inputs.preamble_length.value = 16
    phy.profile_inputs.rx_xtal_error_ppm.value = 0
    phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
    phy.profile_inputs.shaping_filter_param.value = 0.5
    phy.profile_inputs.src_disable.value = True
    phy.profile_inputs.symbols_in_timing_window.value = 6
    phy.profile_inputs.syncword_0.value = long(0xf68d)
    phy.profile_inputs.syncword_1.value = long(0xf672)
    phy.profile_inputs.syncword_length.value = 16
    phy.profile_inputs.timing_detection_threshold.value = 16
    phy.profile_inputs.timing_resync_period.value = 0
    phy.profile_inputs.timing_sample_threshold.value = 0
    phy.profile_inputs.tx_xtal_error_ppm.value = 0

    phy.profile_outputs.AGC_CTRL2_ADCRSTSTARTUP.override = 0
    phy.profile_outputs.AGC_CTRL2_CFLOOPDEL.override = 40
    phy.profile_outputs.AGC_CTRL2_FASTLOOPDEL.override = 1
    phy.profile_outputs.AGC_GAINSTEPLIM_CFLOOPSTEPMAX.override = 0
    phy.profile_outputs.AGC_GAINSTEPLIM_SLOWDECAYCNT.override = 0
    phy.profile_outputs.AGC_LOOPDEL_IFPGADEL.override = 1
    phy.profile_outputs.AGC_LOOPDEL_LNASLICESDEL.override = 1
    phy.profile_outputs.AGC_LOOPDEL_PKDWAIT.override = 2
    #phy.profile_outputs.AGC_MANGAIN_MANGAINLNAATTEN.override = 0
    phy.profile_outputs.MODEM_AFC_AFCDEL.override = 2
    phy.profile_outputs.MODEM_AFC_AFCDSAFREQOFFEST.override = 0
    phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 4
    phy.profile_outputs.MODEM_AFC_AFCSCALEE.override = 15
    phy.profile_outputs.MODEM_AFC_AFCSCALEM.override = 1
    phy.profile_outputs.MODEM_AFCADJLIM_AFCADJLIM.override = 150
    phy.profile_outputs.MODEM_CF_CFOSR.override = 4
    phy.profile_outputs.MODEM_CTRL1_DUALSYNC.override = 0
    phy.profile_outputs.MODEM_CTRL2_DATAFILTER.override = 1
    phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override = 2
    phy.profile_outputs.MODEM_CTRL4_ADCSATLEVEL.override = 1
    phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXFREQ.override = 0
    phy.profile_outputs.MODEM_DIGMIXCTRL_DIGMIXMODE.override = 0
    phy.profile_outputs.MODEM_DSACTRL_AGCBAUDEN.override = 1
    phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 3
    phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 3
    phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 240
    phy.profile_outputs.MODEM_DSATHD0_FDEVMINTHD.override = 25
    phy.profile_outputs.MODEM_DSATHD1_AMPFLTBYP.override = 0
    phy.profile_outputs.MODEM_DSATHD1_DSARSTCNT.override = 6
    phy.profile_outputs.MODEM_DSATHD1_FREQSCALE.override = 1
    phy.profile_outputs.MODEM_DSATHD1_POWABSTHD.override = 200
    phy.profile_outputs.MODEM_DSATHD1_POWRELTHD.override = 3
    phy.profile_outputs.MODEM_DSATHD1_RSSIJMPTHD.override = 9
    phy.profile_outputs.MODEM_MODINDEX_MODINDEXM.override = 26
    phy.profile_outputs.MODEM_SRCCHF_BWSEL.override = 1
    phy.profile_outputs.MODEM_TIMING_FDM0THRESH.override = 0
    phy.profile_outputs.MODEM_TIMING_OFFSUBDEN.override = 0
    phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 0
    phy.profile_outputs.MODEM_TIMING_TIMINGBASES.override = 0
    phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 1
    phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 1000
    phy.profile_outputs.MODEM_VTCORRCFG1_CORRSHFTLEN.override = 0
    phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 450
    phy.profile_outputs.SYNTH_IFFREQ_IFFREQ.override = 0x800
'''

