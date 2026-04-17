from pycalcmodel.core.model import ModelRoot
from pycalcmodel.core.profile import ModelProfile
from pyradioconfig import CalcManager
from pyradioconfig.calculator_model_framework.interfaces.iprofile import IProfile
from pyradioconfig.parts.common.utils.units_multiplier import UnitsMultiplier
from pyradioconfig.parts.common.profiles.ocelot_regs import build_modem_regs_ocelot
from pyradioconfig.parts.ocelot.profiles.frame_profile_inputs_common import frame_profile_inputs_common_ocelot
from pyradioconfig.parts.ocelot.profiles.sw_profile_outputs_common import sw_profile_outputs_common_ocelot
from pycalcmodel.core.output import ModelOutput, ModelOutputType
from pyradioconfig.parts.common.profiles.profile_common import buildCrcOutputs, buildFecOutputs, buildFrameOutputs, \
    buildWhiteOutputs


class ProfileIEEE802154BPSKOcelot(IProfile):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154BPSK"
        self._readable_name = "IEEE802154 BPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 BPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "ocelot"
        self._sw_profile_outputs_common = sw_profile_outputs_common_ocelot()
        self._frame_profile_inputs_common = frame_profile_inputs_common_ocelot()

    def buildProfileModel(self, model: ModelRoot):
        # Build profile object and append it to the model
        profile = self._makeProfile(model)

        self.build_inputs(model, profile)

        self.build_outputs(model, profile)

    def build_inputs(self, model: ModelRoot, profile: ModelProfile):
        # Build inputs
        self.build_required_profile_inputs(model, profile)
        self.build_optional_profile_inputs(model, profile)
        self.build_advanced_profile_inputs(model, profile)
        self.build_hidden_profile_inputs(model, profile)
        self.build_metadata_profile_inputs(model, profile)

    def build_required_profile_inputs(self, model: ModelRoot, profile: ModelProfile):
        self.make_required_input(profile, model.vars.base_frequency_hz, "operational_frequency",
                                 readable_name="Base Channel Frequency", value_limit_min=38_000_000,
                                 value_limit_max=956_000_000, units_multiplier=UnitsMultiplier.MEGA)
        self.make_required_input(profile, model.vars.bitrate, category="modem", readable_name="Bitrate",
                                 value_limit_min=100, value_limit_max=2000_000, units_multiplier=UnitsMultiplier.KILO)
        self.make_required_input(profile, model.vars.xtal_frequency_hz, "crystal",
                                 readable_name="Crystal Frequency", value_limit_min=38000000,
                                 value_limit_max=40000000, units_multiplier=UnitsMultiplier.MEGA)

    def build_optional_profile_inputs(self, model: ModelRoot, profile: ModelProfile):
        pass

    def build_advanced_profile_inputs(self, model: ModelRoot, profile: ModelProfile):
        self.make_linked_io(profile, model.vars.bpsk_feature, 'Advanced',
                            readable_name="BPSK PHY Feature Selection")

    def build_metadata_profile_inputs(self, model: ModelRoot, profile: ModelProfile):
        self.make_metadata_input(profile, model.vars.chcfg_base_frequency_hz, "metadata",
                                 readable_name="Channel Config Base Channel Frequency", value_limit_min=100000000,
                                 value_limit_max=2500000000, units_multiplier=UnitsMultiplier.MEGA)

        # Hidden input needed for rail_test generation
        self.make_metadata_input(profile, model.vars.chcfg_channel_spacing_hz, "metadata",
                                 readable_name="Channel Config Channel Spacing", value_limit_min=0,
                                 value_limit_max=10000000,
                                 units_multiplier=UnitsMultiplier.KILO)

        self.make_metadata_input(profile, model.vars.chcfg_channel_number_start, 'metadata',
                                 readable_name='Channel Config Start channel index',
                                 value_limit_min=0, value_limit_max=9999)
        self.make_metadata_input(profile, model.vars.chcfg_channel_number_end, 'metadata',
                                 readable_name='Channel Config Last channel index',
                                 value_limit_min=0, value_limit_max=9999)
        self.make_metadata_input(profile, model.vars.chcfg_physical_channel_offset, 'metadata',
                                 readable_name='Channel Config Physical channel offset',
                                 value_limit_min=0, value_limit_max=9999)
        self.make_metadata_input(profile, model.vars.rail_tx_power_max, 'metadata', readable_name='Max TX Power',
                                 value_limit_min=[-1], value_limit_max=[999])

    def build_hidden_profile_inputs(self, model: ModelRoot, profile: ModelProfile):
        # Hidden inputs to allow for fixed frame length testing
        self.make_hidden_input(profile, model.vars.frame_length_type, 'frame_general',
                               readable_name="Frame Length Algorithm")
        self.make_hidden_input(profile, model.vars.fixed_length_size, category='frame_fixed_length',
                               readable_name="Fixed Payload Size", value_limit_min=0, value_limit_max=4095)
        self.make_hidden_input(profile, model.vars.frame_bitendian, category='frame_general',
                               readable_name="Frame Bit Endian")

        # Hidden input for changing syncword
        self.make_hidden_input(profile, model.vars.syncword_0, "syncword", readable_name="Sync Word 0",
                               value_limit_min=int(0), value_limit_max=int(0xffffffff))
        self.make_hidden_input(profile, model.vars.syncword_1, "syncword", readable_name="Sync Word 1",
                               value_limit_min=int(0), value_limit_max=int(0xffffffff))

        # Hidden input for CRC
        self.make_hidden_input(profile, model.vars.payload_crc_en, category='frame_payload',
                               readable_name="Insert/Check CRC after payload")
        self.make_hidden_input(profile, model.vars.crc_poly, category='crc',
                               readable_name="CRC Polynomial")
        self.make_hidden_input(profile, model.vars.crc_seed, category='crc', readable_name="CRC Seed",
                               value_limit_min=int(0), value_limit_max=int(0xffffffff))
        self.make_hidden_input(profile, model.vars.crc_byte_endian, category='crc',
                               readable_name="CRC Byte Endian")
        self.make_hidden_input(profile, model.vars.crc_bit_endian, category='crc',
                               readable_name="CRC Output Bit Endian")
        self.make_hidden_input(profile, model.vars.crc_pad_input, category='crc',
                               readable_name="CRC Input Padding")
        self.make_hidden_input(profile, model.vars.crc_input_order, category='crc',
                               readable_name="CRC Input Bit Endian")
        self.make_hidden_input(profile, model.vars.crc_invert, category='crc',
                               readable_name="CRC Invert")

        # Hidden input for BER testing
        self.make_hidden_input(profile, model.vars.test_ber, category="testing",
                               readable_name="Reconfigure for BER testing")

    def build_outputs(self, model: ModelRoot, profile: ModelProfile):
        # Build outputs
        buildFrameOutputs(model, profile)
        buildCrcOutputs(model, profile)
        buildWhiteOutputs(model, profile)
        buildFecOutputs(model, profile)
        self.build_register_profile_outputs(model, profile)
        self.build_variable_profile_outputs(model, profile)
        self.build_info_profile_outputs(model, profile)
        self._sw_profile_outputs_common.buildStudioLogOutput(model, profile)

    def build_register_profile_outputs(self, model: ModelRoot, profile: ModelProfile):
        build_modem_regs_ocelot(model, profile)

    def build_variable_profile_outputs(self, model: ModelRoot, profile: ModelProfile):
        # RAIL Outputs
        self._sw_profile_outputs_common.build_rail_outputs(model, profile)

        # IRCal outputs
        self._sw_profile_outputs_common.build_ircal_outputs(model, profile)

    def build_info_profile_outputs(self, model: ModelRoot, profile: ModelProfile):
        # Informational output
        self._sw_profile_outputs_common.build_info_outputs(model, profile)
        profile.outputs.append(ModelOutput(model.vars.base_frequency_hz, '', ModelOutputType.INFO,
                                           readable_name="Base Channel Frequency"))
        profile.outputs.append(ModelOutput(model.vars.channel_spacing_hz, '', ModelOutputType.INFO,
                                           readable_name="Channel Spacing"))
        profile.outputs.append(ModelOutput(model.vars.agc_scheme, '', ModelOutputType.INFO,
                                           readable_name="AGC backoff scheme"))
        profile.outputs.append(ModelOutput(model.vars.bandwidth_hz, '', ModelOutputType.INFO,
                                           readable_name="Acquisition Channel Bandwidth"))
        profile.outputs.append(ModelOutput(model.vars.etsi_cat1_compatible, '', ModelOutputType.INFO,
                                           readable_name="ETSI Category 1 Compatibility"))
        profile.outputs.append(ModelOutput(model.vars.firstframe_bitsperword, '', ModelOutputType.INFO,
                                           readable_name="Length of the First Word"))
        profile.outputs.append(ModelOutput(model.vars.if_frequency_hz, '', ModelOutputType.INFO,
                                           readable_name="IF Frequency"))
        profile.outputs.append(ModelOutput(model.vars.lo_injection_side, '', ModelOutputType.INFO,
                                           readable_name="Injection side"))
        profile.outputs.append(ModelOutput(model.vars.pll_bandwidth_rx, '', ModelOutputType.INFO,
                                           readable_name="PLL Bandwidth in RX mode"))
        profile.outputs.append(ModelOutput(model.vars.pll_bandwidth_tx, '', ModelOutputType.INFO,
                                           readable_name="PLL Bandwidth in TX mode"))
        profile.outputs.append(ModelOutput(model.vars.src_disable, '', ModelOutputType.INFO,
                                           readable_name="SRC Operation"))
        profile.outputs.append(ModelOutput(model.vars.var_length_loc, '', ModelOutputType.INFO,
                                           readable_name="Byte position of dynamic length byte"))
        profile.outputs.append(ModelOutput(model.vars.fast_detect_enable, '', ModelOutputType.INFO,
                                           readable_name="Fast preamble detect"))
        profile.outputs.append(ModelOutput(model.vars.fec_tx_enable, '', ModelOutputType.INFO,
                                           readable_name="Enable FEC"))
        profile.outputs.append(ModelOutput(model.vars.preamble_detection_length, '', ModelOutputType.INFO,
                                           readable_name="Preamble Detection Length"))
        profile.outputs.append(ModelOutput(model.vars.skip2ant, '', ModelOutputType.INFO,
                                           readable_name="Skip 2nd antenna check with phase demod antenna diversity"))

    def profile_calculate(self, model: ModelRoot):
        model.vars.protocol_id.value_forced = model.vars.protocol_id.var_enum.Custom_802154

        self._build_feature_settings(model)
        self._build_delay_settings(model)

    def _build_feature_settings(self, model: ModelRoot):
        bpsk_feature = model.profile.inputs.bpsk_feature.var_value

        if bpsk_feature == model.vars.bpsk_feature.var_enum.STANDARD_20KBPS:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_868MHz_BPSK_20kbps_coh')
        elif bpsk_feature == model.vars.bpsk_feature.var_enum.STANDARD_40KBPS:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_915MHz_BPSK_40kbps_coh')

    def _build_delay_settings(self, model: ModelRoot):
        bpsk_feature = model.profile.inputs.bpsk_feature.var_value

        if bpsk_feature == model.vars.bpsk_feature.var_enum.STANDARD_20KBPS:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
        elif bpsk_feature == model.vars.bpsk_feature.var_enum.STANDARD_40KBPS:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0


    def _copy_model_variables_from_phy(self, model: ModelRoot, phy_name: str):

        non_prod_phy_model = CalcManager(part_family=model.part_family, part_rev=model.part_revision,
                                         target=model.target).calc_config(phy_name, None)

        for non_prod_profile_input in non_prod_phy_model.profile.inputs:
            model_var = getattr(model.vars, non_prod_profile_input.var_name)
            model_var.value_forced = non_prod_profile_input.var_value

        for non_prod_profile_output in non_prod_phy_model.profile.outputs:
            model_var = getattr(model.vars, non_prod_profile_output.var_name)
            if non_prod_profile_output.override is not None:
                model_var.value_forced = non_prod_profile_output.override

        for non_prod_model_var in non_prod_phy_model.vars:
            if non_prod_model_var.value_forced is not None:
                model_var = getattr(model.vars, non_prod_model_var.name)
                model_var.value_forced = non_prod_model_var.value

        if model.phy is not None:
            model.phy.phy_points_to = phy_name
