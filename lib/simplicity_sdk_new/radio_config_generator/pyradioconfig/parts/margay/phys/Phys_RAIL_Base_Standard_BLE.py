from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *
from pyradioconfig.parts.ocelot.phys.Phys_RAIL_Base_Standard_BLE import PHYS_Bluetooth_LE_Ocelot


class PHYS_Bluetooth_LE_Margay(PHYS_Bluetooth_LE_Ocelot):
    #Inherit all from Ocelot
    # For Margay we decided to use different IFs for BLE1Mbps and BLE2Mbps - MCUW_RADIO_CFG-2149
    # Overwriting only TRECs + Viterbi Ocelot Phy profiles that define IFs for BLE for now

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
        phy.profile_inputs.if_frequency_hz.value = 1200000
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
        phy.profile_inputs.if_frequency_hz.value = 600000
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

    pass
