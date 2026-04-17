from pyradioconfig.parts.common.profiles.serval_regs import build_modem_regs_serval
from pyradioconfig.parts.lynx.profiles.Profile_IEEE802154_OQPSK import ProfileIEEE802154OQPSKLynx
from pyradioconfig.parts.serval.profiles.sw_profile_outputs_common import sw_profile_outputs_common_serval
from pycalcmodel.core.output import ModelOutput, ModelOutputType
from pyradioconfig.parts.common.profiles.profile_common import buildCrcOutputs, buildFecOutputs, buildFrameOutputs, \
    buildWhiteOutputs


class ProfileIEEE802154BPSKServal(ProfileIEEE802154OQPSKLynx):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154BPSK"
        self._readable_name = "IEEE802154 BPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 BPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "serval"
        self._sw_profile_outputs_common = sw_profile_outputs_common_serval()

    def buildProfileModel(self, model):
        # Build profile object and append it to the model
        profile = self._makeProfile(model)

        # Build inputs
        self.build_required_profile_inputs(model, profile)
        self.build_optional_profile_inputs(model, profile)
        self.build_advanced_profile_inputs(model, profile)
        self.build_hidden_profile_inputs(model, profile)

        # Build outputs
        buildFrameOutputs(model, profile)
        buildCrcOutputs(model, profile)
        buildWhiteOutputs(model, profile)
        buildFecOutputs(model, profile)
        self.build_register_profile_outputs(model, profile)
        self.build_variable_profile_outputs(model, profile)
        self.build_info_profile_outputs(model, profile)

        self._sw_profile_outputs_common.buildStudioLogOutput(model, profile)

    def build_hidden_profile_inputs(self, model, profile):
        super().build_hidden_profile_inputs(model, profile)
        self.make_hidden_input(profile, model.vars.preamble_length, "preamble",
                                     readable_name="Preamble Length Total", value_limit_min=0,
                                     value_limit_max=2097151)

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_serval(model, profile)

    def build_variable_profile_outputs(self, model, profile):
        # RAIL Outputs
        self._sw_profile_outputs_common.build_rail_outputs(model, profile)

        # IRCal outputs
        self._sw_profile_outputs_common.build_ircal_outputs(model, profile)

    def build_info_profile_outputs(self, model, profile):
        # Informational output
        self._sw_profile_outputs_common.build_info_outputs(model, profile)
        profile.outputs.append(ModelOutput(model.vars.agc_scheme, '', ModelOutputType.INFO, readable_name="AGC backoff scheme"))
        profile.outputs.append(ModelOutput(model.vars.bandwidth_hz, '', ModelOutputType.INFO, readable_name="Acquisition Channel Bandwidth"))
        profile.outputs.append(ModelOutput(model.vars.etsi_cat1_compatible, '', ModelOutputType.INFO, readable_name="ETSI Category 1 Compatibility"))
        profile.outputs.append(ModelOutput(model.vars.firstframe_bitsperword, '', ModelOutputType.INFO, readable_name="Length of the First Word"))
        profile.outputs.append(ModelOutput(model.vars.if_frequency_hz, '', ModelOutputType.INFO, readable_name="IF Frequency"))
        profile.outputs.append(ModelOutput(model.vars.lo_injection_side, '', ModelOutputType.INFO, readable_name="Injection side"))
        profile.outputs.append(ModelOutput(model.vars.pll_bandwidth_rx, '', ModelOutputType.INFO, readable_name="PLL Bandwidth in RX mode"))
        profile.outputs.append(ModelOutput(model.vars.pll_bandwidth_tx, '', ModelOutputType.INFO, readable_name="PLL Bandwidth in TX mode"))
        profile.outputs.append(ModelOutput(model.vars.src_disable, '', ModelOutputType.INFO, readable_name="SRC Operation"))
        profile.outputs.append(ModelOutput(model.vars.var_length_loc, '', ModelOutputType.INFO, readable_name="Byte position of dynamic length byte"))
        profile.outputs.append(ModelOutput(model.vars.fast_detect_enable, '', ModelOutputType.INFO, readable_name="Fast preamble detect"))
        profile.outputs.append(ModelOutput(model.vars.fec_tx_enable, '', ModelOutputType.INFO, readable_name="Enable FEC"))
        profile.outputs.append(ModelOutput(model.vars.preamble_detection_length, '', ModelOutputType.INFO, readable_name="Preamble Detection Length"))
        profile.outputs.append(ModelOutput(model.vars.skip2ant, '', ModelOutputType.INFO, readable_name="Skip 2nd antenna check with phase demod antenna diversity"))

    def _build_chip_rate_settings(self, model):
        model.vars.bitrate.value_forced = 20000
        model.vars.channel_spacing_hz.value_forced = 0
        model.vars.deviation.value_forced = 150000

        model.vars.dsss_chipping_code.value_forced = 0x9AF
        model.vars.dsss_len.value_forced = 15
        model.vars.dsss_spreading_factor.value_forced = 15

    def _build_frame_settings(self, model):
        super()._build_frame_settings(model)
        model.vars.asynchronous_rx_enable.value_forced = None
        model.vars.frame_coding.value_forced = None
        model.vars.manchester_mapping.value_forced = None
        model.vars.var_length_minlength.value_forced = 5

    def _build_modulation_settings(self, model):
        model.vars.modulation_type.value_forced = model.vars.modulation_type.var_enum.BPSK
        model.vars.symbol_encoding.value_forced = model.vars.symbol_encoding.var_enum.DSSS
        model.vars.manchester_mapping.value_forced = model.vars.manchester_mapping.var_enum.Default
        model.vars.fsk_symbol_map.value_forced = model.vars.fsk_symbol_map.var_enum.MAP0
        model.vars.diff_encoding_mode.value_forced = model.vars.diff_encoding_mode.var_enum.RE0

        model.vars.shaping_filter.value_forced = model.vars.shaping_filter.var_enum.Raised_Cosine
        model.vars.shaping_filter_param.value_forced = 1.0

    def _build_modem_settings(self, model):
        if model.vars.demod_select.value == model.vars.demod_select.var_enum.COHERENT:
            self._build_cohdsa_settings(model)
        else:
            self._build_legacy_settings(model)

        model.vars.deviation_tol_ppm.value_forced = 0
        model.vars.directmode_rx.value_forced = model.vars.directmode_rx.var_enum.DISABLED

    def _build_cohdsa_settings(self, model):
        model.vars.rx_xtal_error_ppm.value_forced = 0
        model.vars.tx_xtal_error_ppm.value_forced = 0
        model.vars.preamble_length.value_forced = 32
        model.vars.agc_period.value_forced = 3
        model.vars.agc_power_target.value_forced = -17
        model.vars.baudrate_tol_ppm.value_forced = 0
        model.vars.if_frequency_hz.value_forced = 600000
        model.vars.frequency_comp_mode.value_forced = model.vars.frequency_comp_mode.var_enum.DISABLED
        model.vars.number_of_timing_windows.value_forced = 11
        model.vars.rssi_period.value_forced = 7
        model.vars.timing_detection_threshold.value_forced = 28
        model.vars.agc_power_target.value_forced = -27
        model.vars.bandwidth_hz.value_forced = 525000
        model.vars.MODEM_COH3_COHDSAEN.value_forced = 0
        model.vars.AGC_CTRL7_SUBDEN.value_forced = 4
        model.vars.AGC_CTRL7_SUBINT.value_forced = 3
        model.vars.AGC_CTRL7_SUBNUM.value_forced = 3
        model.vars.AGC_CTRL7_SUBPERIOD.value_forced = 1
        model.vars.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.value_forced = 5
        model.vars.AGC_RSSISTEPTHR_POSSTEPTHR.value_forced = 3
        model.vars.AGC_RSSISTEPTHR_DEMODRESTARTPER.value_forced = 6
        model.vars.MODEM_AFCADJLIM_AFCADJLIM.value_forced = 2000
        model.vars.MODEM_CTRL0_DUALCORROPTDIS.value_forced = 1
        model.vars.MODEM_CTRL1_PHASEDEMOD.value_forced = 2
        model.vars.MODEM_CTRL1_RESYNCPER.value_forced = 4
        model.vars.MODEM_CTRL3_TSAMPDEL.value_forced = 2
        model.vars.MODEM_CTRL5_TDEDGE.value_forced = 0
        model.vars.MODEM_CTRL5_TREDGE.value_forced = 0
        model.vars.MODEM_CTRL6_RXBRCALCDIS.value_forced = 1
        model.vars.MODEM_MODINDEX_MODINDEXE.value_forced = 29
        model.vars.MODEM_MODINDEX_MODINDEXM.value_forced = 15
        model.vars.MODEM_PRE_PREERRORS.value_forced = 15
        model.vars.MODEM_TIMING_ADDTIMSEQ.value_forced = 8
        model.vars.MODEM_TIMING_FASTRESYNC.value_forced = 1
        model.vars.MODEM_TIMING_TIMINGBASES.value_forced = 8
        model.vars.MODEM_TIMING_TIMTHRESH.value_forced = 50
        model.vars.MODEM_TXBR_TXBRDEN.value_forced = 1
        model.vars.MODEM_TXBR_TXBRNUM.value_forced = 16

    def _build_legacy_settings(self, model):
        model.vars.rx_xtal_error_ppm.value_forced = 0
        model.vars.tx_xtal_error_ppm.value_forced = 0
        model.vars.preamble_length.value_forced = 128
        model.vars.agc_hysteresis.value_forced = 0
        model.vars.agc_period.value_forced = 3
        model.vars.agc_power_target.value_forced = -17
        model.vars.baudrate_tol_ppm.value_forced = 1875
        model.vars.frequency_comp_mode.value_forced = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        model.vars.number_of_timing_windows.value_forced = 11
        model.vars.rssi_period.value_forced = 7
        model.vars.timing_detection_threshold.value_forced = 30
        model.vars.timing_resync_period.value_forced = 4
        model.vars.AGC_CTRL7_SUBDEN.value_forced = 7
        model.vars.AGC_CTRL7_SUBINT.value_forced = 4
        model.vars.AGC_CTRL7_SUBNUM.value_forced = 2
        model.vars.AGC_CTRL7_SUBPERIOD.value_forced = 1
        model.vars.AGC_GAINSTEPLIM0_CFLOOPSTEPMAX.value_forced = 4
        model.vars.MODEM_CTRL2_DATAFILTER.value_forced = 0
        model.vars.MODEM_TIMING_FASTRESYNC.value_forced = 1
        model.vars.MODEM_CTRL4_PREDISTDEB.value_forced = 1
        model.vars.MODEM_CTRL4_PREDISTGAIN.value_forced = 3