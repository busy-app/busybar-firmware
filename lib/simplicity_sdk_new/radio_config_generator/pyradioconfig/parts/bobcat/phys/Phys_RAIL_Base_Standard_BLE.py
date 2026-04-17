from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE, PHY_COMMON_FRAME_BLE_AOX, PHY_COMMON_FRAME_BLE_CODED
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *


class PHYS_Bluetooth_LE_Bobcat(IPhy):

    def BLE_TX_Shaping_Coeffs(self, phy, model):
        "(Column C) Fb=1Mbps 1 dB boost from Optimized shaping coefficients for panther PLL_Ranga_MSW_v2.xlsx"
        # This should be used with MODE1 = 1 MHz PLL BW
        # This shaping filter is specifically chosen to deal with PA-VCO coupling on 20 dBm PA
        # PGBOBCATVALTEST-2122
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 28
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 47
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 66
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 83
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 98
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 96
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 83
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 64
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 46
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 31
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 17
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 10
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 8
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 7
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 6
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 5
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 3
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 2
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 1
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 1
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
        # Fb=2mbps, bw=1.5 MHz 2dB df2 boost from Optimized shaping coefficients for panther PLL_Ranga_MSW_v2.xlsx
        # This should be used with MODE2 = 1.5 MHz PLL BW (Column Q)
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
        # Fb=2mbps, bw=2.5 MHz 2dB df2 boost from Optimized shaping coefficients for panther PLL_Ranga_MSW_v2.xlsx
        # This should be used with MODE4 = 3 MHz PLL BW (Column S)
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3
        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 26
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 45
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 63
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 80
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 96
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 99
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 87
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 70
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 53
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 37
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 21
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 13
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 9
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 7
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 6
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 5
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 5
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 1 # lynx goes to 0s here, xlsx uses 1s. Matching lynx
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 1

    def BLE_AGC(self, phy, model):
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 245
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5

    def _ble_agc_halfrate(self, phy, model):
        self.BLE_AGC(phy, model)
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

    def _ble_agc_fullrate(self, phy, model):
        self.BLE_AGC(phy, model)
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 5
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 8

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

    def BLE_2M_DSA(self, phy, model):
        phy.profile_outputs.MODEM_DSACTRL_ARRTHD.override = 5
        phy.profile_outputs.MODEM_DSATHD0_FDEVMAXTHD.override = 180
        phy.profile_outputs.MODEM_DSATHD3_FDEVMAXTHDLO.override = 180
        phy.profile_outputs.MODEM_DSATHD4_POWABSTHDLO.override = 1500

    # BLE 1M Legacy
    def Bluetooth_LE_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1260000

        """ Demod """
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = 0
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameters """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.rx_xtal_error_ppm.value = 20
        phy.profile_inputs.tx_xtal_error_ppm.value = 0

        """ Bandwidth """
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE1

        """ Timing """
        # : TODO REMOVE and Test on Wired FPGA
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_outputs.MODEM_TIMING_OFFSUBNUM.override = 7

        """  """
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ Advanced Overrides """
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1 # : TODO remove and Test on Wired FPGA
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL2_SQITHRESH.override = 200


    def PHY_Bluetooth_LE(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official BLE 1Mbps Legacy PHY for Panther',
                            phy_name=phy_name)
        self.Bluetooth_LE_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        return phy

    # BLE 1M Viterbi
    def Bluetooth_LE_Viterbi_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)
        self.BLE_DSA(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1099233

        """ Demod """
        phy.profile_inputs.dsa_enable.value = True

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = 0
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameters """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.rx_xtal_error_ppm.value = 20

        """ Bandwidth """
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE

        """ Timing """
        # : TODO REMOVE and Test on Wired FPGA
        #phy.profile_inputs.symbols_in_timing_window.value = 0
        #phy.profile_inputs.errors_in_timing_window.value = 1
        #phy.profile_inputs.timing_sample_threshold.value = 0

        """  """
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ Advanced Overrides """
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100

        """ Viterbi Demod Overrides """
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24

    def PHY_Bluetooth_LE_Viterbi(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LE Viterbi phase-DSA',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        self.Bluetooth_LE_Viterbi_base(phy, model)
        return phy

    # BLE 2M Viterbi
    def Bluetooth_LE_2M_Viterbi_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_2M_TX_Shaping_Coeffs(phy, model)
        self.BLE_DSA(phy, model)
        self.BLE_2M_DSA(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 2500000

        """ Demod """
        phy.profile_inputs.dsa_enable.value = True

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = 0
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameters """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Preamble """
        phy.profile_inputs.preamble_length.value = 16

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.rx_xtal_error_ppm.value = 20

        """ Bandwidth """
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE_FULLRATE

        """  """
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ Timing """
        # : TODO REMOVE and Test on Wired FPGA
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.timing_sample_threshold.value = 0

        """ Frequency Compensation Mode """
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.AFC_LOCK_AT_FRAME_DETECT

        """ Advanced Overrides """
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_AFCADJTX_AFCSCALEM.override = 27
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEM.override = 27
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 220
        phy.profile_outputs.MODEM_VTBLETIMING_FLENOFF.override = 4
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 148
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 220
        phy.profile_outputs.MODEM_VTTRACK_HIPWRTHD.override = 43
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950


    def PHY_Bluetooth_LE_2M_Viterbi(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Official 2Mbps BLE phase-DSA PHY for Panther',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced= model.vars.ble_feature.var_enum.LE_2M
        self.Bluetooth_LE_2M_Viterbi_base(phy, model)
        return phy

    # BLE 1M TRecS + Viterbi
    def Bluetooth_LE_Viterbi_noDSA_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1099233

        """ Demod """
        phy.profile_inputs.dsa_enable.value = False

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = 0
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameters """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.rx_xtal_error_ppm.value = 62
        phy.profile_inputs.tx_xtal_error_ppm.value = 62

        """ Bandwidth """
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE

        """ Timing """
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.symbols_in_timing_window.value = 0

        """  """
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ Frequency comp """
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON

        """ Advanced Overrides """
        # Decimation and Demod Overrides
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL6_RXBRCALCDIS.override = 1
        phy.profile_outputs.MODEM_PRE_BASE.override = 2

        # FRC Overrides
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1

        # This should be set only on DSA PHYs (PGLYNX-566), so we can clear it.
        phy.profile_outputs.FRC_CTRL_WAITEOFEN.override = 0

        # Radio Controller Overrides
        phy.profile_outputs.RAC_SYNTHREGCTRL_MMDLDOVREFTRIM.override = 3

        # Viterbi Demod Overrides
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5

        #Half rate 1M  and full rate
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 120
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

    def Bluetooth_LE_Viterbi_noDSA_halfrate_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        self._ble_agc_halfrate(phy, model)
        phy.profile_inputs.if_frequency_hz.value = 700000
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.HALFRATE

    # Owner      : Young-Joon Choi
    # Jira Link  : https://jira.silabs.com/browse/PGBOBCATVALTEST-2
    def PHY_Bluetooth_LE_Viterbi_noDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Lynx',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)

        # PGBOBCATVALTEST-2122: 1 MHz BW, 1 dB df2 boost to deal with PA-VCO coupling for 20 dBm PA
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE1
        self.BLE_TX_Shaping_Coeffs(phy, model)

        return phy

    def PHY_Bluetooth_LE_1M_Viterbi_917M_noDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='PHY_Bluetooth_LE_1M_Viterbi_917M_noDSA',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)

        phy.profile_inputs.if_frequency_hz.value = 1066666
        phy.profile_inputs.base_frequency_hz.value = long(917000000)

        phy.profile_outputs.SYNTH_DIVCTRL_LODIVFREQCTRL.override = 3
        return phy

    # Owner      : Young-Joon Choi
    # Jira Link  : https://jira.silabs.com/browse/PGBOBCATVALTEST-204
    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA Fullrate PHY for Lynx',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        return phy

    # BLE 2M TRecs + Viterbi
    def Bluetooth_LE_2M_Viterbi_noDSA_base(self, phy, model):
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        self.BLE_2M_TX_Shaping_Coeffs(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE_FULLRATE

        phy.profile_inputs.bitrate.value = 2000000
        phy.profile_inputs.bandwidth_hz.value = 2200000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.preamble_length.value = 16
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.src1_range_available_minimum.value = 133

        # AGC Overrides (match Lynx) - Needed to match intermod performance MCW_RADIO_CFG-1814
        self._ble_agc_fullrate(phy, model)

    # Owner      : Young-Joon Choi
    # Jira Link  : https://jira.silabs.com/browse/PGBOBCATVALTEST-3
    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi 2M No DSA Fullrate PHY for Lynx',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy, model)

        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        # 2M Full rate
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 60

        # : https://jira.silabs.com/browse/MCUW_RADIO_CFG-1900
        # : Michael Wu: Whether a packet contains a CTE is not known until the packet payload,
        # : thus BLE 1M & 2M PHYs must be able to receive AoX packets
        phy.profile_inputs.aox_enable.value = model.vars.aox_enable.var_enum.ENABLED
        self.BLE_RX_AOX_CHF_Coeffs(phy, model)

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 5750
        phy.profile_outputs.tx_sync_delay_ns.override = 375
        phy.profile_outputs.tx_eof_delay_ns.override = 375

        return phy

    def PHY_Bluetooth_2M_38M4Hz_prod(self, model, phy_name=None):
        phy = self.PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(model, phy_name='PHY_Bluetooth_2M_38M4Hz_prod')
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_outputs.rx_eof_delay_ns.override = 7000
        return phy

    # BLR 125K
    def Bluetooth_LongRange_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_CODED(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 500000  # Net bitrate(data only)
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1230000

        """ Demod """
        phy.profile_inputs.dsa_enable.value = True

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameter """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.rx_xtal_error_ppm.value = 62
        phy.profile_inputs.tx_xtal_error_ppm.value = 62

        """ Bandwidth """
        phy.profile_inputs.synth_settling_mode.value = model.vars.synth_settling_mode.var_enum.BLE_LR
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE

        """ Preamble """
        phy.profile_inputs.preamble_length.value = 20

        """ Timing """
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0

        """  """
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ FRC """
        phy.profile_inputs.header_addtrailtxdata_en.value = True # We need to add Trail TX Data to the header (subframe 0), which is the Coding/Rate Indication (CI/RI)
        phy.profile_inputs.header_calc_crc.value = False # The header (subframe 0) does not include a CRC
        phy.profile_inputs.header_excludesubframewcnt_en.value = True # With Dynamic frame length (DFL) mode, we need to exclude the header (subframe 0) from Word Counter
        phy.profile_inputs.header_white_en.value = False # No whitening in the header (subframe 0)

        # FIXME :MR Do we need this for LR?
        phy.profile_outputs.FRC_FECCTRL_CONVDECODEMODE.override = 0

        """ Advanced Overrides """
        #phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.src1_range_available_minimum.value = 128
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LONGRANGE
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        phy.profile_outputs.AGC_LNABOOST_LNABWADJ.override = 0
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.FRC_CTRL_RXFCDMODE.override = 0 # Override the RXFCDMODE to FCDMODE0 (FCD2 is reloaded when SCNT reaches 0)
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 0
        phy.profile_outputs.FRC_FCD0_WORDS.override = 3 # SYNCWORD size (4 bytes) - 1 = 3
        phy.profile_outputs.FRC_FCD2_ADDTRAILTXDATA.override = 0
        phy.profile_outputs.FRC_FCD2_CALCCRC.override = 1
        phy.profile_outputs.FRC_FCD2_EXCLUDESUBFRAMEWCNT.override = 0
        phy.profile_outputs.FRC_FCD2_INCLUDECRC.override = 1
        phy.profile_outputs.FRC_FCD2_WORDS.override = 2 ** phy.profile_outputs.FRC_FCD2_WORDS.var.get_bit_width() - 1
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_CONVGENERATOR_GENERATOR0.override = 13
        phy.profile_outputs.FRC_CONVGENERATOR_GENERATOR1.override = 15
        phy.profile_outputs.FRC_FECCTRL_CONVMODE.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATACNT.override = 4
        phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 0 # Was missed
        phy.profile_outputs.MODEM_CTRL0_CODING.override = 3
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 1
        phy.profile_outputs.MODEM_CTRL1_COMPMODE.override = 3
        phy.profile_outputs.MODEM_CTRL1_SYNCDATA.override = 1
        phy.profile_outputs.MODEM_CTRL2_RATESELMODE.override = 2
        phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override =2 # Was missed
        phy.profile_outputs.MODEM_DSACTRL_DSAMODE.override = 0 # Was missed
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 0
        phy.profile_outputs.MODEM_LONGRANGE1_CHPWRACCUDEL.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDPUGATE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_LRSS.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 400

        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 8
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 14
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 20
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 26
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 32
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 38
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 44
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 50
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 56
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 62
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 68

        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 1
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 2
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 3
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 4
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 10
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 11

        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 29
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 105  # MCUW-RADIO_CFG-1666: Reverting change from RC CharA2 this cleans PER Noise
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 120  # LR 125K MCUW-RADIO_CFG-1666: Reverting change from RC CharA2 this cleans PER Noise
        phy.profile_outputs.MODEM_LONGRANGE_LRBLE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRCORRTHD.override = 1100
        phy.profile_outputs.MODEM_LONGRANGE_LRDEC.override = 3
        phy.profile_outputs.MODEM_LONGRANGE_LRTIMCORRTHD.override = 1000
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 2

        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTTRACK_TIMEACQUTHD.override = 32

    # Owner      : Young-Joon Choi
    # Jira Link  : https://jira.silabs.com/browse/PGBOBCATVALTEST-201
    def PHY_Bluetooth_LongRange_dsa_125kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 125kbps PHY for Lynx',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K
        self.Bluetooth_LongRange_base(phy, model)
        return phy

    def PHY_Bluetooth_LongRange_NOdsa_125kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange No DSA 125kbps',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K
        self.Bluetooth_LongRange_base(phy, model)
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 4
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        return phy

    # BLR 500k
    def Bluetooth_LongRange_500kbps_base(self, phy, model):
        self.Bluetooth_LongRange_base(phy, model)
        phy.profile_outputs.FRC_CTRL_RATESELECT.override = 2
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATA.override = 1
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 10
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 16
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 22
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 28
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 34
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 40
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 46
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 52
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 64
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 58
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 10
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 72

        phy.profile_outputs.rx_eof_delay_ns.override = 9000
        phy.profile_outputs.tx_sync_delay_ns.override = 875
        phy.profile_outputs.tx_eof_delay_ns.override = 875

    # Owner      : Young-Joon Choi
    # Jira Link  : https://jira.silabs.com/browse/PGBOBCATVALTEST-202
    def PHY_Bluetooth_LongRange_dsa_500kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 500kbps PHY for Lynx',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K
        self.Bluetooth_LongRange_500kbps_base(phy, model)
        return phy

    def PHY_Bluetooth_LongRange_NOdsa_500kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 500kbps',
                            phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K
        self.Bluetooth_LongRange_500kbps_base(phy, model)
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 4
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        return phy

    # Owner     : Young-Joon Choi
    # Jira Link : https://jira.silabs.com/browse/PGBOBCATVALTEST-205
    def PHY_Bluetooth_1M_Concurrent(self, model, phy_name=None):
        # Start with the BLE LR 125k PHY
        phy = self.PHY_Bluetooth_LongRange_dsa_125kbps(model, phy_name)
        phy.profile_inputs.preamble_pattern.value = 0b10
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CONCURRENT

        # The concurrent PHY needs to have:
        # Enable concurrent mode
        # FRC = BLE LR 125k, with added sub-frame description for BLE 1M uncoded
        # Demod = BLE LR 125k
        # TRECS/Viterbi = BLE 1M
        # BLE LR = BLE LR 125k
        # DSA = BLE LR 125k

        # Enable concurrent mode
        phy.profile_outputs.MODEM_COCURRMODE_CONCURRENT.override = 1

        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 220
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 146
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 130
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

        # Overrides for TX mode (copied from BLE 1M Fullrate)
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        phy.profile_outputs.FRC_FCD0_ADDTRAILTXDATA.override = 0
        phy.profile_outputs.FRC_FCD0_CALCCRC.override = 1
        phy.profile_outputs.FRC_FCD0_EXCLUDESUBFRAMEWCNT.override = 0
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 1
        phy.profile_outputs.FRC_FCD0_WORDS.override = 2 ** phy.profile_outputs.FRC_FCD0_WORDS.var.get_bit_width() - 1
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATACNT.override = 0
        phy.profile_outputs.MODEM_CTRL1_SYNCDATA.override = 0
        phy.profile_outputs.MODEM_CTRL2_RATESELMODE.override = 0
        phy.profile_outputs.MODEM_PRE_TXBASES.override = 4
        phy.profile_outputs.MODEM_CTRL0_CODING.override = 0

        # : Set for both Bobcat and Lynx
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831  # : copy clock gating settings from BLE 1M
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 32  # Needed because BW is wider than non-concurrent 1M

        # : needed as part of IPMCUSRW-771 workaround
        # : https://jira.silabs.com/browse/IPMCUSRW-771
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE.override = 0
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 0 # MCUW_RADIO_CFG-1705, RAIL_LIB-6014 to be compatible with lynx workaround of MCUW_RADIO_CFG-1701

        phy.profile_outputs.FRC_FECCTRL_CONVMODE.override = 0

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 11750
        phy.profile_outputs.tx_sync_delay_ns.override = 2000
        phy.profile_outputs.tx_eof_delay_ns.override = 2000
        return phy

    def PHY_Bluetooth_1M_AOX(self, model, phy_name=None):
        # bobcat uses half-rate PHY compared to Lynx, due to jitter and settling improvements
        # For Bobcat BLE 1M, use PHY_Bluetooth_1M_AOX_prod as there is no difference in TX SYNTH PLL BW
        phy = self.PHY_Bluetooth_LE_Viterbi_noDSA(model, phy_name='PHY_Bluetooth_1M_AOX')
        phy.profile_inputs.aox_enable.value = model.vars.aox_enable.var_enum.ENABLED
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_1M
        self.BLE_RX_AOX_CHF_Coeffs(phy, model)

        # Override for AoX
        # 1 Mbps AoX SYNTH PLL should track the non-AOX PHY for whatever is needed for emissions / PA-VCO coupling.
        # Both 1 MHz BW, 1.5 MHz BW pass AoX mod char, do whatever it takes in the non-AOX PHY to make it pass mod char.
        # No override of the SYNTH for 1 Mbps unlike for 2 Mbps
        phy.profile_outputs.MODEM_VTCORRCFG1_VITERBIKSI3WB.override = 28 # MCUW_RADIO_CFG-1849: minimize register diff for RAIL, KSI3WB is not used

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 11250
        phy.profile_outputs.tx_sync_delay_ns.override = 750
        phy.profile_outputs.tx_eof_delay_ns.override = 750

        return phy

    def PHY_Bluetooth_2M_AOX(self, model, phy_name="PHY_Bluetooth_2M_AOX"):
        # always use full-rate PHY on all parts
        # For Bobcat BLE 2M, there are separate PHY_Bluetooth_2M_AOX_prod and PHY_Bluetooth_2M_prod to meet emissions
        # Both PHYs must support AoX RX, the difference is in the SYNTH PLL setting + shaping filter for TX
        phy = self.PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(model, phy_name)
        phy.profile_inputs.aox_enable.value = model.vars.aox_enable.var_enum.ENABLED
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_2M
        self.BLE_RX_AOX_CHF_Coeffs(phy, model)

        # Override for AoX
        self.BLE_2M_AOX_TX_Shaping_Coeffs(phy, model)
        phy.profile_outputs.MODEM_VTCORRCFG1_VITERBIKSI3WB.override = 35  # MCUW_RADIO_CFG-1849: minimize register diff for RAIL, KSI3WB is not used
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE4  # 3M PLL BW - this is important for CTE modulation characteristic
        """
        This higher PLL BW of AoX affects emissions. For low output power (< 6 dBm, i.e. Lynx) this is not a concern.
        For high output power, must consider BLE in-band emissions and FCC restricted band emissions.
        Max output lower must be limited by RAIL appropriately
        """

        return phy

    def PHY_Bluetooth_2M_38M4Hz_AOX_prod(self, model, phy_name=None):
        phy = self.PHY_Bluetooth_2M_AOX(model, phy_name="PHY_Bluetooth_2M_38M4Hz_AOX_prod")

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_outputs.rx_eof_delay_ns.override = 7000
        return phy

    def BLE_RX_AOX_CHF_Coeffs(self, phy, model):
        """
        Coeffiients to be used in the secondary channel filter coeff during the CTE only.
        See PGBOBCATVALTEST-807 regard the filter design for time + freq domain constraints
        to minimize the phase error. Bobcat uses a step response transient minimization, while
        lynx increases the same rate to shorten the real-time transient response
        :param phy:
        :param model:
        :return:
        """

        coeff = [969, 173, 172, 11, 1892, 1603, 0, 601, 3894, 15986, 601, 15783, 100, 8912, 15345]
        phy.profile_outputs.MODEM_CHFCOE10_SET1COEFF0.override = coeff[0]
        phy.profile_outputs.MODEM_CHFCOE10_SET1COEFF1.override = coeff[1]
        phy.profile_outputs.MODEM_CHFCOE10_SET1COEFF2.override = coeff[2]
        phy.profile_outputs.MODEM_CHFCOE11_SET1COEFF3.override = coeff[3]
        phy.profile_outputs.MODEM_CHFCOE11_SET1COEFF4.override = coeff[4]
        phy.profile_outputs.MODEM_CHFCOE12_SET1COEFF5.override = coeff[5]
        phy.profile_outputs.MODEM_CHFCOE12_SET1COEFF6.override = coeff[6]
        phy.profile_outputs.MODEM_CHFCOE13_SET1COEFF7.override = coeff[7]
        phy.profile_outputs.MODEM_CHFCOE13_SET1COEFF8.override = coeff[8]
        phy.profile_outputs.MODEM_CHFCOE14_SET1COEFF9.override = coeff[9]
        phy.profile_outputs.MODEM_CHFCOE14_SET1COEFF10.override = coeff[10]
        phy.profile_outputs.MODEM_CHFCOE15_SET1COEFF11.override = coeff[11]
        phy.profile_outputs.MODEM_CHFCOE15_SET1COEFF12.override = coeff[12]
        phy.profile_outputs.MODEM_CHFCOE16_SET1COEFF13.override = coeff[13]
        phy.profile_outputs.MODEM_CHFCOE16_SET1COEFF14.override = coeff[14]

    def PHY_Bluetooth_1M_HADM(self, model, phy_name=None):
        # https://confluence.silabs.com/display/BGHADM/Bobcat#Bobcat-PHYStrategy
        # Only valid for 40 MHz
        phy = self.PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 40000000
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.HADM_1M
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.HFXOMULT

        phy.profile_inputs.target_osr.value = 5

        # Packet Inputs
        phy.profile_inputs.frame_bitendian.value = model.vars.frame_bitendian.var_enum.LSB_FIRST
        phy.profile_inputs.frame_length_type.value = model.vars.frame_length_type.var_enum.FIXED_LENGTH
        phy.profile_inputs.payload_white_en.value = False
        phy.profile_inputs.payload_crc_en.value = False

        # Variable length includes header
        phy.profile_inputs.header_en.value = False

        # NOTE: Currently the 'header_include_crc' variable is not part of the
        # profile inputs, so we can't set it here, instead we need to force the
        # output for both FCDs (TX/RX).
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 0
        phy.profile_outputs.FRC_FCD2_INCLUDECRC.override = 0

        # AFC at syncword via VTAFCFRAME. Disable feedback to SYNTH as frequency estimates may not be valid with UCAS
        phy.profile_outputs.MODEM_REALTIMCFE_VTAFCFRAME.override = 1  # estimate from syncword
        phy.profile_outputs.MODEM_AFC_AFCONESHOT.override = 0
        phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 0  # no estimate from DSA
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEM.override = 0  # disable feedback to avoid updates during tracking cycle
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEE.override = 0

        # needed for precise HADM event timing - HADM PHYs are 40 MHz only
        phy.profile_outputs.rx_sync_delay_ns.override = 24000
        phy.profile_outputs.rx_eof_delay_ns.override = 8000
        phy.profile_outputs.tx_sync_delay_ns.override = 0
        phy.profile_outputs.tx_eof_delay_ns.override = 0

        return phy

    def PHY_Bluetooth_2M_HADM(self, model, phy_name=None):
        # https://confluence.silabs.com/display/BGHADM/Bobcat#Bobcat-PHYStrategy
        # Only valid for 40 MHz
        phy = self.PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 40000000
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.HADM_2M
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.HFXOMULT

        phy.profile_inputs.target_osr.value = 5

        # Packet Inputs
        phy.profile_inputs.frame_bitendian.value = model.vars.frame_bitendian.var_enum.LSB_FIRST
        phy.profile_inputs.frame_length_type.value = model.vars.frame_length_type.var_enum.FIXED_LENGTH
        phy.profile_inputs.payload_white_en.value = False
        phy.profile_inputs.payload_crc_en.value = False

        # Variable length includes header
        phy.profile_inputs.header_en.value = False

        # NOTE: Currently the 'header_include_crc' variable is not part of the
        # profile inputs, so we can't set it here, instead we need to force the
        # output for both FCDs (TX/RX).
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 0
        phy.profile_outputs.FRC_FCD2_INCLUDECRC.override = 0

        # AFC at syncword via VTAFCFRAME. Disable feedback to SYNTH as frequency estimates may not be valid with UCAS
        phy.profile_outputs.MODEM_REALTIMCFE_VTAFCFRAME.override = 1 # estimate from syncword
        phy.profile_outputs.MODEM_AFC_AFCONESHOT.override = 0
        phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 0 # no estimate from DSA
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEM.override = 0 # disable feedback to avoid updates during tracking cycle
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEE.override = 0

        return phy
