from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_154
from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *


class PhysRAILBaseStandardIEEE802154Lynx(IPhy):

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 38400000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq

    def _part_specific_phy_overrides(self, phy, model):
        pass

    def IEEE802154_2p4GHz_base(self, phy, model):

        # Use the commmon ZB frame definition and override min length for 802.15.4E Seq# Suppression
        PHY_COMMON_FRAME_154(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.LEGACY
        phy.profile_inputs.var_length_minlength.value = 4

        # Configure non-framing Profile Inputs
        phy.profile_inputs.bandwidth_hz.value = 2524800
        phy.profile_inputs.base_frequency_hz.value = long(2405000000)
        phy.profile_inputs.baudrate_tol_ppm.value = 4000
        phy.profile_inputs.bitrate.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 5000000
        phy.profile_inputs.deviation.value = 500000
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED
        phy.profile_inputs.dsss_chipping_code.value = long(0x744AC39B)
        phy.profile_inputs.dsss_len.value = 32
        phy.profile_inputs.dsss_spreading_factor.value = 8
        phy.profile_inputs.frame_bitendian.value = model.vars.frame_bitendian.var_enum.LSB_FIRST
        phy.profile_inputs.frame_length_type.value = model.vars.frame_length_type.var_enum.VARIABLE_LENGTH
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.if_frequency_hz.value = 1370000
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.OQPSK
        phy.profile_inputs.pll_bandwidth_tx.value = model.vars.pll_bandwidth_tx.var_enum.BW_1500KHz
        phy.profile_inputs.pll_bandwidth_rx.value = model.vars.pll_bandwidth_rx.var_enum.BW_250KHz
        phy.profile_inputs.preamble_length.value = 32
        phy.profile_inputs.preamble_pattern.value = 0
        phy.profile_inputs.preamble_pattern_len.value = 4
        phy.profile_inputs.rx_xtal_error_ppm.value = 0
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Custom_OQPSK
        phy.profile_inputs.shaping_filter_param.value = 0.0
        phy.profile_inputs.symbol_encoding.value = model.vars.symbol_encoding.var_enum.DSSS
        phy.profile_inputs.syncword_0.value = long(0xe5)
        phy.profile_inputs.syncword_1.value = long(0x0)
        phy.profile_inputs.syncword_length.value = 8
        phy.profile_inputs.timing_detection_threshold.value = 75
        phy.profile_inputs.timing_resync_period.value = 2
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        # phy.profile_inputs.xtal_frequency_hz.value = 38400000
        self._set_xtal_frequency(phy)

        # Gate clocks (reduce current)
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 0x1003  # 0, 1, 12

        # Set RSSI period differently than the standard LUT on Lynx
        phy.profile_outputs.AGC_CTRL1_RSSIPERIOD.override = 8

        # Configure synth cal registers (likely not needed, but avoids regression)
        self._set_synth_cal_regs(phy)

        # RAIL timings
        phy.profile_outputs.rx_sync_delay_ns.override = 6125
        phy.profile_outputs.rx_eof_delay_ns.override = 6125

        self._part_specific_phy_overrides(phy, model)

    def _set_synth_cal_regs(self, phy):
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1BWCAL.override = 11
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_OP1COMPCAL.override = 14
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RFBVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RPVALCAL.override = 0
        phy.profile_outputs.SYNTH_LPFCTRL1CAL_RZVALCAL.override = 9

    def PHY_IEEE802154_2p4GHz(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Legacy IEEE 802.15.4 2p4GHz PHY from Jumbo',phy_name=phy_name)
        self.IEEE802154_2p4GHz_base(phy, model)

        phy.profile_outputs.AGC_CTRL2_DISRFPKD.override = 1
        phy.profile_outputs.AGC_CTRL4_RFPKDCNTEN.override = 0

        return phy