from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *


class PHYS_Bluetooth_LE_Sol(IPhy):

    def BLE_TX_Shaping_Coeffs(self, phy, model):
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 14
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 28
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 47
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 67
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 83
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 97
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 85
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 67
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 50
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 4
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 22
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 16
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 14
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 14
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 13
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 12
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 10
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 8
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 7
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 6
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 5
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 4
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 0
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
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY
        phy.profile_inputs.adc_clock_mode.value = model.vars.adc_clock_mode.var_enum.VCODIV

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
        self.Bluetooth_LE_base(phy, model)
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
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.adc_clock_mode.value = model.vars.adc_clock_mode.var_enum.VCODIV

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

    def Bluetooth_LE_Viterbi_noDSA_halfrate_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        phy.profile_inputs.if_frequency_hz.value = 700000

        phy.profile_outputs.RAC_IFADCTRIM0_IFADCENHALFMODE.override = 1
        phy.profile_outputs.AGC_AGCPERIOD0_MAXHICNTTHD.override = 9
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHI.override = 14
        phy.profile_outputs.AGC_AGCPERIOD1_PERIODLOW.override = 45
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 245
        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.override = 4
        phy.profile_outputs.AGC_GAINSTEPLIM0_HYST.override = 3
        phy.profile_outputs.AGC_GAINSTEPLIM1_PNINDEXMAX.override = 14
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 5
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 8
        #FIXME phy.profile_outputs.AGC_PNRFATT1_LNAMIXRFATT3.override = 3
        #FIXME phy.profile_outputs.AGC_PNRFATT1_LNAMIXRFATT4.override = 6
        #FIXME phy.profile_outputs.AGC_PNRFATT2_LNAMIXRFATT5.override = 9
        #FIXME phy.profile_outputs.AGC_PNRFATT2_LNAMIXRFATT6.override = 13
        #FIXME phy.profile_outputs.AGC_PNRFATT3_LNAMIXRFATT7.override = 16
        #FIXME phy.profile_outputs.AGC_PNRFATT3_LNAMIXRFATT8.override = 22
        #FIXME phy.profile_outputs.AGC_PNRFATT4_LNAMIXRFATT9.override = 30
        #FIXME phy.profile_outputs.AGC_PNRFATT4_LNAMIXRFATT10.override = 32
        #FIXME phy.profile_outputs.AGC_PNRFATT5_LNAMIXRFATT11.override = 43
        #FIXME phy.profile_outputs.AGC_PNRFATT5_LNAMIXRFATT12.override = 51
        #FIXME phy.profile_outputs.AGC_PNRFATT6_LNAMIXRFATT13.override = 63
        #FIXME phy.profile_outputs.AGC_PNRFATT6_LNAMIXRFATT14.override = 126
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5

    def PHY_Bluetooth_LE_Viterbi_noDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Lynx',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        #FIXME AW has to override for viterbi phy
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950
        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA Fullrate PHY for Lynx',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        return phy

    # BLE 2M TRecs + Viterbi
    def Bluetooth_LE_2M_Viterbi_noDSA_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)

        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.bandwidth_hz.value = 2200000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.src1_range_available_minimum.value = 133

    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi 2M No DSA Fullrate PHY for Lynx',
                            phy_name=phy_name)
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy, model)
        return phy
