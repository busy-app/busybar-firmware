from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *


class PHYS_Bluetooth_LE_Ocelot(IPhy):

    def BLE_TX_Shaping_Coeffs(self, phy, model):
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 27
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 47
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 66
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 83
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 95
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 81
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 61
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 42
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 26
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 12
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 4
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 2
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 4
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 3
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 1
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 1
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 1
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 1
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 0
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 0
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 0
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 0
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 0
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 0
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 0
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 0
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 0

    def BLE_2M_TX_Shaping_Coeffs(self, phy, model):
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 1
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 7
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 17
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 33
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 55
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 75
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 90
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 103
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 84
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 64
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 41
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 21
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 5
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 0
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 0
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 1
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 2
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 3
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 0

    def BLE_2M_AOX_TX_Shaping_Coeffs(self, phy, model):
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 1
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 4
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 11
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 23
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 40
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 58
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 76
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 92
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 102
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 101
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 90
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 73
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 55
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 38
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 21
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 10
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 4
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 2
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 1
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 1
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 1
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 1
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 2
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 0
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 0
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 0

    def BLE_AGC(self, phy, model):
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 245
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5

    def BLE_DSA(selfs, phy, model):
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD0.override = 1
        phy.profile_outputs.MODEM_DSACTRL_ARRTOLERTHD1.override = 2
        phy.profile_outputs.MODEM_DSACTRL_FREQAVGSYM.override = 0
        phy.profile_outputs.MODEM_DSACTRL_GAINREDUCDLY.override = 2
        phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 90
        phy.profile_outputs.MODEM_DSATHD0_FDEVMINTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD0_SPIKETHD.override = 60
        phy.profile_outputs.MODEM_DSATHD0_UNMODTHD.override = 8
        phy.profile_outputs.MODEM_DSATHD1_AMPFLTBYP.override = 0
        phy.profile_outputs.MODEM_DSATHD1_FREQSCALE.override = 1
        phy.profile_outputs.MODEM_DSATHD1_POWABSTHD.override = 3000
        phy.profile_outputs.MODEM_DSATHD1_POWRELTHD.override = 2
        phy.profile_outputs.MODEM_DSATHD1_PWRDETDIS.override = 0
        phy.profile_outputs.MODEM_DSATHD2_FDADJTHD.override = 20
        phy.profile_outputs.MODEM_DSATHD2_FREQESTTHD.override = 16
        phy.profile_outputs.MODEM_DSATHD2_INTERFERDET.override = 1
        phy.profile_outputs.MODEM_DSATHD2_JUMPDETEN.override = 0
        phy.profile_outputs.MODEM_DSATHD2_PMDETFORCE.override = 1
        phy.profile_outputs.MODEM_DSATHD2_PMDETPASSTHD.override = 4
        phy.profile_outputs.MODEM_DSATHD2_POWABSTHDLOG.override = 248
        phy.profile_outputs.MODEM_DSATHD3_FDEVMAXTHDLO.override = 160
        phy.profile_outputs.MODEM_DSATHD3_FDEVMINTHDLO.override = 8
        phy.profile_outputs.MODEM_DSATHD3_SPIKETHDLO.override = 90
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD0LO.override = 1
        phy.profile_outputs.MODEM_DSATHD4_ARRTOLERTHD1LO.override = 3
        phy.profile_outputs.MODEM_DSATHD4_POWABSTHDLO.override = 1000
        phy.profile_outputs.MODEM_DSATHD4_SWTHD.override = 1

    # BLE 1M Legacy
    def Bluetooth_LE_base(self, phy, model):
        PHY_COMMON_FRAME_BLE(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 1260000
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 8
        phy.profile_inputs.rssi_period.value = 3
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 5
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY

        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.VCODIV
        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.AGC_LNABOOST_LNABWADJ.override = 0
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 200
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 0
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 7

    def PHY_Bluetooth_LE(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official BLE 1Mbps Legacy PHY for Panther',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        self.Bluetooth_LE_base(phy, model)
        return phy

    def PHY_Bluetooth_LE_EnPkd(self, model):
        phy = self.PHY_Bluetooth_LE(model, 'PHY_Bluetooth_LE_EnPkd')
        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 0
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 1
        return phy

    # BLE 1M Viterbi
    def Bluetooth_LE_Viterbi_base(self, phy, model):
        PHY_COMMON_FRAME_BLE(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)
        self.BLE_DSA(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 1099233
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 8
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.VCODIV

        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = 5
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 62
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 42
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 30
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

    def PHY_Bluetooth_LE_Viterbi(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LE Viterbi phase-DSA',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        self.Bluetooth_LE_Viterbi_base(phy, model)
        return phy

    # BLE 2M Viterbi
    def Bluetooth_LE_2M_Viterbi_base(self, phy, model):
        PHY_COMMON_FRAME_BLE(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_2M_TX_Shaping_Coeffs(phy, model)
        self.BLE_DSA(phy, model)

        phy.profile_inputs.bandwidth_hz.value = 2500000
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = True
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.src1_range_available_minimum.value = 133
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.AFC_LOCK_AT_FRAME_DETECT
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.VCODIV

        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 5
        phy.profile_outputs.MODEM_AFCADJTX_AFCSCALEM.override = 27
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEM.override = 27
        phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 180
        phy.profile_outputs.MODEM_DSATHD3_FDEVMAXTHDLO.override = 180
        phy.profile_outputs.MODEM_DSATHD4_POWABSTHDLO.override = 1500
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 220
        phy.profile_outputs.MODEM_VTBLETIMING_FLENOFF.override = 4
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 220
        phy.profile_outputs.MODEM_VTTRACK_HIPWRTHD.override = 43
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

    def PHY_Bluetooth_LE_2M_Viterbi(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official 2Mbps BLE phase-DSA PHY for Panther',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M
        self.Bluetooth_LE_2M_Viterbi_base(phy, model)
        return phy

    # BLE 1M TRecS + Viterbi
    def Bluetooth_LE_Viterbi_noDSA_base(self, phy, model):
        phy.profile_inputs.bandwidth_hz.value = 1099233
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 1000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsa_enable.value = False
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1000KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 8
        phy.profile_inputs.rssi_period.value = 3
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.VCODIV

        # Decimation and Demod Overrides
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL6_RXBRCALCDIS.override = 1
        phy.profile_outputs.MODEM_PRE_BASE.override = 2

        # Shaping overrides
        self.BLE_TX_Shaping_Coeffs(phy, model)

        # FRC Overrides
        PHY_COMMON_FRAME_BLE(phy, model)
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1

        # This should be set only on DSA PHYs (PGLYNX-566), sowe can clear it.
        phy.profile_outputs.FRC_CTRL_WAITEOFEN.override = 0

        # Radio Controller Overrides
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3

        # Viterbi Demod Overrides
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

        # KSI overrides
        model.vars.ksi3wb.value_forced = 28

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 10010
        phy.profile_outputs.tx_sync_delay_ns.override = 0
        phy.profile_outputs.tx_eof_delay_ns.override = 0

    def Bluetooth_LE_Viterbi_noDSA_halfrate_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        phy.profile_inputs.if_frequency_hz.value = 700000
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.HALFRATE

        phy.profile_outputs.RAC_IFADCTRIM0_IFADCENHALFMODE.override = 1
        phy.profile_outputs.AGC_AGCPERIOD0_MAXHICNTTHD.override = 9
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHI.override = 14
        phy.profile_outputs.AGC_AGCPERIOD1_PERIODLOW.override = 45
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 245
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.override = 4
        phy.profile_outputs.AGC_GAINSTEPLIM0_HYST.override = 3
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 5
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 8
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5

        # Frequency limiting
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 120

    def PHY_Bluetooth_LE_Viterbi_noDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Lynx',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_EnPkd(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base,
                            readable_name='BLE Viterbi No DSA PHY for Lynx with Peak Detector Enabled',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 0
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 1
        # Additional settings
        phy.profile_outputs.rx_eof_delay_ns.override = 10855
        phy.profile_outputs.tx_sync_delay_ns.override = 104
        phy.profile_outputs.tx_eof_delay_ns.override = 104

    def PHY_Bluetooth_LE_1M_Viterbi_917M_noDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='PHY_Bluetooth_LE_1M_Viterbi_917M_noDSA',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        phy.profile_inputs.if_frequency_hz.value = 1066666
        phy.profile_inputs.base_frequency_hz.value = long(917000000)

        phy.profile_outputs.SYNTH_DIVCTRL_LODIVFREQCTRL.override = 3
        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA Fullrate PHY for Lynx',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate_EnPkd(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base,
                            readable_name='BLE Viterbi No DSA Fullrate PHY for Lynx with Peak Detector Enabled',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 0
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 1

        return phy

    # BLE 2M TRecs + Viterbi
    def Bluetooth_LE_2M_Viterbi_noDSA_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        self.BLE_2M_TX_Shaping_Coeffs(phy, model)

        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.bandwidth_hz.value = 2200000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.src1_range_available_minimum.value = 133

        #AGC overrides
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 5
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 8
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3

        #Frequency limiting
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 60

        phy.profile_outputs.rx_sync_delay_ns.override = 14500
        phy.profile_outputs.rx_eof_delay_ns.override = 4300
        phy.profile_outputs.tx_sync_delay_ns.override = 0
        phy.profile_outputs.tx_eof_delay_ns.override = 0

    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi 2M No DSA Fullrate PHY for Lynx',
                            phy_name=phy_name)
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy, model)
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M

        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        return phy

    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate_EnPkd(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base,
                            readable_name='BLE Viterbi 2M No DSA Fullrate PHY for Lynx with Peak Detector Enabled',
                            phy_name=phy_name)
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M

        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 0
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 1
        # Additional settings
        phy.profile_outputs.rx_eof_delay_ns.override = 5865
        phy.profile_outputs.tx_sync_delay_ns.override = 61
        phy.profile_outputs.tx_eof_delay_ns.override = 61

    def PHY_Bluetooth_1M_AOX(self, model, phy_name=None):
        phy = self.PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(model, phy_name='PHY_Bluetooth_1M_AOX')
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_1M

        # force fullrate and dec1 = 1 to match LYNX AOX config (which uses DEC1 switching)
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.VCODIV
        model.vars.adc_rate_mode.value_forced = model.vars.adc_rate_mode.var_enum.FULLRATE
        model.vars.dec0.value_forced = 4
        model.vars.dec1.value_forced = 2
        return phy

    def PHY_Bluetooth_2M_AOX(self, model, phy_name=None):
        phy = self.PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(model, phy_name='PHY_Bluetooth_2M_AOX')
        self.BLE_2M_AOX_TX_Shaping_Coeffs(phy, model)
        phy.profile_inputs.synth_tx_mode.value = model.vars.synth_tx_mode.var_enum.MODE8
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_2M
        return phy
