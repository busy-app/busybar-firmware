from pyradioconfig.parts.common.profiles.Profile_Sigfox_TX import Profile_Sigfox_TX, IProfile
import pyradioconfig.parts.common.profiles.profile_common as pc
from pyradioconfig.parts.ocelot.profiles.frame_profile_inputs_common import frame_profile_inputs_common_ocelot
from pyradioconfig.parts.common.profiles.ocelot_regs import build_modem_regs_ocelot
from pyradioconfig.parts.common.profiles.profile_modem import buildModemInfoOutputs, buildRailOutputs
from pyradioconfig.parts.common.utils.units_multiplier import UnitsMultiplier
from pycalcmodel.core.output import ModelOutput, ModelOutputType
from pycalcmodel.core.profile import ModelProfile
from pycalcmodel.core.model import ModelRoot
from pyradioconfig.parts.ocelot.profiles.sw_profile_outputs_common import sw_profile_outputs_common_ocelot


class Profile_Sigfox_TX_Ocelot(Profile_Sigfox_TX):

    def __init__(self):
        super().__init__()
        self._family = "ocelot"
        self._frame_profile_inputs_common = frame_profile_inputs_common_ocelot()
        self._sw_profile_outputs_common = sw_profile_outputs_common_ocelot()

    def buildProfileModel(self, model) -> ModelProfile:
        # Build profile
        profile = self._makeProfile(model)

        self.build_profile_inputs(model, profile)

        self.build_frame_configuration_inputs(model, profile)

        self.build_modem_inputs(profile)

        self.build_outputs(model, profile)

        # Deprecated inputs
        # These inputs were exposed on or after Ocelot Alpha 1 release, so they may be present in radioconf XML
        self.make_deprecated_input(profile, model.vars.max_tx_power_dbm)

        # Add register profile outputs
        self._add_reg_profile_outputs(model, profile)

        return profile

    def set_frame_configuration_defaults(self, model) -> None:
        """
        Set frame configuration related defaults
        :param model:
        :param profile: ModelProfile
        """
        self.set_packet_inputs_defaults(model)
        self.set_header_defaults(model)
        self.set_variable_length_defaults(model)
        self.set_crc_input_defaults(model)
        self.set_misc_input_defaults(model)

    def set_misc_input_defaults(self, model) -> None:
        """
        Misc default inputs that now have to be forced
        :param model:
        :param profile: ModelProfile
        """
        model.vars.fec_en.value_forced = model.vars.fec_en.var_enum.NONE
        model.vars.fixed_length_size.value_forced = 1
        model.vars.frame_type_0_length.value_forced = 0
        model.vars.frame_type_0_valid.value_forced = False
        model.vars.frame_type_1_length.value_forced = 0
        model.vars.frame_type_1_valid.value_forced = False
        model.vars.frame_type_2_length.value_forced = 0
        model.vars.frame_type_2_valid.value_forced = False
        model.vars.frame_type_3_length.value_forced = 0
        model.vars.frame_type_3_valid.value_forced = False
        model.vars.frame_type_4_length.value_forced = 0
        model.vars.frame_type_4_valid.value_forced = False
        model.vars.frame_type_5_length.value_forced = 0
        model.vars.frame_type_5_valid.value_forced = False
        model.vars.frame_type_6_length.value_forced = 0
        model.vars.frame_type_6_valid.value_forced = False
        model.vars.frame_type_7_length.value_forced = 0
        model.vars.frame_type_7_valid.value_forced = False
        model.vars.frame_type_bits.value_forced = 3
        model.vars.frame_type_loc.value_forced = 0
        model.vars.frame_type_lsbit.value_forced = 0
        model.vars.var_length_byteendian.value_forced = model.vars.var_length_byteendian.var_enum.LSB_FIRST
        model.vars.white_output_bit.value_forced = 0
        model.vars.white_poly.value_forced = model.vars.white_poly.var_enum.NONE
        model.vars.white_seed.value_forced = 0

    def set_crc_input_defaults(self, model) -> None:
        """
        CRC Inputs' Defaults
        :param model:
        :param profile: ModelProfile
        """
        model.vars.crc_poly.value_forced = model.vars.crc_poly.var_enum.CCITT_16
        model.vars.crc_seed.value_forced = int(0x00000000)
        model.vars.crc_input_order.value_forced = model.vars.crc_input_order.var_enum.LSB_FIRST
        model.vars.crc_bit_endian.value_forced = model.vars.crc_bit_endian.var_enum.MSB_FIRST
        model.vars.crc_byte_endian.value_forced = model.vars.crc_byte_endian.var_enum.MSB_FIRST
        model.vars.crc_pad_input.value_forced = False
        model.vars.crc_invert.value_forced = False

    def set_variable_length_defaults(self, model) -> None:
        """
        Variable length defaults
        :param model:
        :param profile: ModelProfile
        :return:
        """
        model.vars.var_length_numbits.value_forced = 8
        model.vars.var_length_bitendian.value_forced = model.vars.var_length_bitendian.var_enum.LSB_FIRST
        model.vars.var_length_shift.value_forced = 0
        model.vars.var_length_minlength.value_forced = 5
        model.vars.var_length_maxlength.value_forced = 0x7F
        model.vars.var_length_includecrc.value_forced = True
        model.vars.var_length_adjust.value_forced = 0

    def set_header_defaults(self, model) -> None:
        """
        Header Defaults
        :param profile: ModelProfile
        :return:
        """
        model.vars.header_en.value_forced = True
        model.vars.header_size.value_forced = 1
        model.vars.header_calc_crc.value_forced = False
        model.vars.header_white_en.value_forced = False

    def set_packet_inputs_defaults(self, model) -> None:
        # Packet Inputs
        model.vars.frame_bitendian.value_forced = model.vars.frame_bitendian.var_enum.MSB_FIRST
        model.vars.frame_length_type.value_forced = model.vars.frame_length_type.var_enum.FIXED_LENGTH
        model.vars.payload_white_en.value_forced = False
        model.vars.payload_crc_en.value_forced = False

    def build_modem_inputs(self, profile: ModelProfile) -> None:
        pass

    def build_outputs(self, model, profile: ModelProfile) -> None:
        # Sigfox profile does not include advanced inputs,
        # but we do need an output for IR cal power level for rail_scripts to consume
        # So add that here. It will default to 0 (which triggers to change to IR cal).

        # Informational Output
        buildModemInfoOutputs(model, profile)
        # Output fields
        pc.buildFrameOutputs(model, profile)
        pc.buildCrcOutputs(model, profile)
        pc.buildWhiteOutputs(model, profile)
        pc.buildFecOutputs(model, profile)

        self._sw_profile_outputs_common.build_rail_outputs(model, profile)
        self._sw_profile_outputs_common.build_ircal_outputs(model, profile)
        self._sw_profile_outputs_common.buildStudioLogOutput(model, profile)

    def build_profile_inputs(self, model, profile: ModelProfile) -> None:
        """
        Create required, hidden and optional inputs
        :param model:
        :param profile:
        """
        self.build_required_profile_inputs(model, profile)
        self.build_hidden_profile_inputs(model, profile)
        self.build_optional_profile_inputs(model, profile)
        self.build_metadata_profile_inputs(model,profile)

    def build_required_profile_inputs(self, model: ModelRoot, profile: ModelProfile) -> None:
        IProfile.make_required_input(profile, model.vars.base_frequency_hz, "operational_frequency",
                                     readable_name="Base Channel Frequency", value_limit_min=100000000,
                                     value_limit_max=2500000000, units_multiplier=UnitsMultiplier.MEGA)
        IProfile.make_required_input(profile, model.vars.channel_spacing_hz, "operational_frequency",
                                     readable_name="Channel Spacing", value_limit_min=0, value_limit_max=10000000,
                                     units_multiplier=UnitsMultiplier.KILO)
        IProfile.make_required_input(profile, model.vars.xtal_frequency_hz, "crystal",
                                     readable_name="Crystal Frequency", value_limit_min=38000000,
                                     value_limit_max=40000000, units_multiplier=UnitsMultiplier.MEGA)
        IProfile.make_required_input(profile, model.vars.bitrate, "modem", readable_name="Bitrate",
                                     value_limit_min=100, value_limit_max=600,
                                     units_multiplier=UnitsMultiplier.KILO)
        IProfile.make_required_input(profile, model.vars.fixed_length_size, category='frame_fixed_length',
                                     readable_name="Fixed Payload Size", value_limit_min=0, value_limit_max=4095)

    def build_hidden_profile_inputs(self, model: ModelRoot, profile: ModelProfile) -> None:
        # Add Ocelot-specific variables
        IProfile.make_hidden_input(profile, model.vars.demod_select, 'Advanced',
                                   readable_name="Demod Selection")
        IProfile.make_hidden_input(profile, model.vars.synth_settling_mode, 'modem',
                                   readable_name="Synth Settling Mode")
        IProfile.make_hidden_input(profile, model.vars.agc_lock_mode, 'Advanced',
                                   readable_name="AGC lock mode")
        IProfile.make_hidden_input(profile, model.vars.agc_enable_adc_attenuation, 'Advanced',
                                   readable_name="Enable ADC attenuation")
        IProfile.make_hidden_input(profile, model.vars.preamble_length, "preamble",
                                   readable_name="Preamble Length Total", value_limit_min=0, value_limit_max=2097151)

        IProfile.make_optional_input(profile, model.vars.syncword_tx_skip, "syncword",
                                     readable_name="Sync Word TX Skip", default=False)
        IProfile.make_optional_input(profile, model.vars.test_ber, "testing", default=False,
                                     readable_name="Reconfigure for BER testing")

        # Hidden inputs to allow for fixed frame length testing
        self.make_hidden_input(profile, model.vars.frame_length_type, 'frame_general',
                               readable_name="Frame Length Algorithm")

    def build_metadata_profile_inputs(self, model: ModelRoot, profile: ModelProfile) -> None:
        self.make_metadata_input(profile, model.vars.chcfg_channel_number_start, "metadata",
                                 readable_name="Channel Config Start channel index", value_limit_min=0,
                                 value_limit_max=65535)
        self.make_metadata_input(profile, model.vars.chcfg_channel_number_end, "metadata",
                                 readable_name="Channel Config Last channel index", value_limit_min=0,
                                 value_limit_max=65535)

    def build_optional_profile_inputs(self, model: ModelRoot, profile: ModelProfile) -> None:
        pass

    def build_frame_configuration_inputs(self, model: ModelRoot, profile: ModelProfile) -> None:
        pass
        # self._frame_profile_inputs_common.build_frame_inputs(model, profile)
        # pc.buildCrcInputs(model, profile)
        # pc.buildWhiteInputs(model, profile)
        # pc.buildFecInputs(model, profile)

    def _add_reg_profile_outputs(self, model: ModelRoot, profile: ModelProfile) -> None:
        build_modem_regs_ocelot(model, profile)

    def profile_calculate(self, model: ModelRoot):
        super().profile_calculate(model)

        self.set_frame_configuration_defaults(model)

        # The above code is really just a copy/paste of the Base profile with some inputs commented out.
        # It should be diffed against the Base profile for accuracy.
        # Now set default values for all of the above inputs.  Assume any defaults above
        # were just copy/pasted from the base profile and set the correct values here
        self._set_DBPSK_regs(model)
        self._set_transmit_shaping_coefficient(model)
        self._disable_analog_ramping(model)

        # : preamble
        model.vars.preamble_pattern_len.value_forced = 1
        model.vars.preamble_length.value_forced = 2

        # : sync
        model.vars.syncword_length.value_forced = 8
        model.vars.syncword_0.value_forced = int(0x0)

        # : advanced
        model.vars.deviation_tol_ppm.value_forced = 0
        model.vars.agc_settling_delay.value_forced = 44
        model.vars.agc_power_target.value_forced = 254
        model.vars.errors_in_timing_window.value_forced = 0
        model.vars.timing_detection_threshold.value_forced = 0
        model.vars.timing_sample_threshold.value_forced = 10

    def _set_DBPSK_regs(self, model: ModelRoot):
        # New Ocelot regs, need to determine how to calculate these for all DBPSK
        model.vars.MODEM_CTRL6_TXDBPSKINV.value_forced = 1
        model.vars.MODEM_CTRL6_TXDBPSKRAMPEN.value_forced = 1

    def _set_transmit_shaping_coefficient(self, model: ModelRoot):
        # To be compliant with spectrum template required by Sigfox with digital ramping
        # Symmetrical Shaping filter RRC alpha=0.5 with only 15 taps [-Ts:Ts]
        # (15 taps provides better power envelope than 64 taps)
        # This filter improves the Sriharsha filter which provides higher power envelope variation
        # (see MCUW_RADIO_CFG-2573 and MCUW_RADIO_CFG-2592)
        # transmit shaping coefficient

        model.vars.shaping_filter_param.value_forced = 0.5

        model.vars.MODEM_CTRL0_SHAPING.value_forced = 3
        model.vars.MODEM_SHAPING0_COEFF0.value_forced = 255
        model.vars.MODEM_SHAPING0_COEFF1.value_forced = 17
        model.vars.MODEM_SHAPING0_COEFF2.value_forced = 39
        model.vars.MODEM_SHAPING0_COEFF3.value_forced = 64
        model.vars.MODEM_SHAPING1_COEFF4.value_forced = 88
        model.vars.MODEM_SHAPING1_COEFF5.value_forced = 108
        model.vars.MODEM_SHAPING1_COEFF6.value_forced = 122
        model.vars.MODEM_SHAPING1_COEFF7.value_forced = 127
        model.vars.MODEM_SHAPING2_COEFF8.value_forced = 122
        model.vars.MODEM_SHAPING2_COEFF9.value_forced = 108
        model.vars.MODEM_SHAPING2_COEFF10.value_forced = 88
        model.vars.MODEM_SHAPING2_COEFF11.value_forced = 64
        model.vars.MODEM_SHAPING3_COEFF12.value_forced = 39
        model.vars.MODEM_SHAPING3_COEFF13.value_forced = 17
        model.vars.MODEM_SHAPING3_COEFF14.value_forced = 255
        model.vars.MODEM_SHAPING3_COEFF15.value_forced = 0
        model.vars.MODEM_SHAPING4_COEFF16.value_forced = 0
        model.vars.MODEM_SHAPING4_COEFF17.value_forced = 0
        model.vars.MODEM_SHAPING4_COEFF18.value_forced = 0
        model.vars.MODEM_SHAPING4_COEFF19.value_forced = 0
        model.vars.MODEM_SHAPING5_COEFF20.value_forced = 0
        model.vars.MODEM_SHAPING5_COEFF21.value_forced = 0
        model.vars.MODEM_SHAPING5_COEFF22.value_forced = 0
        model.vars.MODEM_SHAPING5_COEFF23.value_forced = 0
        model.vars.MODEM_SHAPING6_COEFF24.value_forced = 0
        model.vars.MODEM_SHAPING6_COEFF25.value_forced = 0
        model.vars.MODEM_SHAPING6_COEFF26.value_forced = 0
        model.vars.MODEM_SHAPING6_COEFF27.value_forced = 0
        model.vars.MODEM_SHAPING7_COEFF28.value_forced = 0
        model.vars.MODEM_SHAPING7_COEFF29.value_forced = 0
        model.vars.MODEM_SHAPING7_COEFF30.value_forced = 0
        model.vars.MODEM_SHAPING7_COEFF31.value_forced = 0
        model.vars.MODEM_SHAPING8_COEFF32.value_forced = 0
        model.vars.MODEM_SHAPING8_COEFF33.value_forced = 0
        model.vars.MODEM_SHAPING8_COEFF34.value_forced = 0
        model.vars.MODEM_SHAPING8_COEFF35.value_forced = 0
        model.vars.MODEM_SHAPING9_COEFF36.value_forced = 0
        model.vars.MODEM_SHAPING9_COEFF37.value_forced = 0
        model.vars.MODEM_SHAPING9_COEFF38.value_forced = 0
        model.vars.MODEM_SHAPING9_COEFF39.value_forced = 0
        model.vars.MODEM_SHAPING10_COEFF40.value_forced = 0
        model.vars.MODEM_SHAPING10_COEFF41.value_forced = 0
        model.vars.MODEM_SHAPING10_COEFF42.value_forced = 0
        model.vars.MODEM_SHAPING10_COEFF43.value_forced = 0
        model.vars.MODEM_SHAPING11_COEFF44.value_forced = 0
        model.vars.MODEM_SHAPING11_COEFF45.value_forced = 0
        model.vars.MODEM_SHAPING11_COEFF46.value_forced = 0
        model.vars.MODEM_SHAPING11_COEFF47.value_forced = 0
        model.vars.MODEM_SHAPING12_COEFF48.value_forced = 0
        model.vars.MODEM_SHAPING12_COEFF49.value_forced = 0
        model.vars.MODEM_SHAPING12_COEFF50.value_forced = 0
        model.vars.MODEM_SHAPING12_COEFF51.value_forced = 0
        model.vars.MODEM_SHAPING13_COEFF52.value_forced = 0
        model.vars.MODEM_SHAPING13_COEFF53.value_forced = 0
        model.vars.MODEM_SHAPING13_COEFF54.value_forced = 0
        model.vars.MODEM_SHAPING13_COEFF55.value_forced = 0
        model.vars.MODEM_SHAPING14_COEFF56.value_forced = 0
        model.vars.MODEM_SHAPING14_COEFF57.value_forced = 0
        model.vars.MODEM_SHAPING14_COEFF58.value_forced = 0
        model.vars.MODEM_SHAPING14_COEFF59.value_forced = 0
        model.vars.MODEM_SHAPING15_COEFF60.value_forced = 0
        model.vars.MODEM_SHAPING15_COEFF61.value_forced = 0
        model.vars.MODEM_SHAPING15_COEFF62.value_forced = 0
        model.vars.MODEM_SHAPING15_COEFF63.value_forced = 0

    def _disable_analog_ramping(self, model: ModelRoot):
        # Disable the Analog Ramping which it provides not compliant spectrum mask with the Sigfox standard in Tx
        model.vars.SEQ_MISC_DIG_RAMP_EN.value_forced = 1
