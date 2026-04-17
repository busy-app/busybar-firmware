from pyradioconfig.parts.common.profiles.panther_regs import build_modem_regs_panther
from pyradioconfig.calculator_model_framework.interfaces.iprofile import IProfile
from pyradioconfig.parts.common.utils.units_multiplier import UnitsMultiplier
from pyradioconfig.parts.common.profiles.profile_common import buildCrcOutputs, buildFecOutputs, buildFrameOutputs, \
    buildWhiteOutputs, build_ircal_sw_vars
from pyradioconfig.parts.common.profiles.profile_modem import buildModemInfoOutputs
from pycalcmodel.core.output import ModelOutput, ModelOutputType
from py_2_and_3_compatibility import *

from pyradioconfig.calculator_model_framework.CalcManager import CalcManager

class ProfileBLEPanther(IProfile):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "panther"

    def buildProfileModel(self, model):

        # Build profile object and append it to the model
        profile = self._makeProfile(model)

        # Build inputs
        self.build_required_profile_inputs(model, profile)
        self.build_optional_profile_inputs(model, profile)
        self.build_advanced_profile_inputs(model, profile)
        self.build_hidden_profile_inputs(model, profile)
        self.build_metadata_profile_inputs(model, profile)

        # Build outputs
        buildFrameOutputs(model, profile)
        buildCrcOutputs(model, profile)
        buildWhiteOutputs(model, profile)
        buildFecOutputs(model, profile)
        self.build_register_profile_outputs(model, profile)
        self.build_variable_profile_outputs(model, profile)
        self.build_info_profile_outputs(model, profile)

    def build_required_profile_inputs(self, model, profile):
        IProfile.make_required_input(profile, model.vars.xtal_frequency_hz, "crystal",
                                     readable_name="Crystal Frequency", value_limit_min=38000000,
                                     value_limit_max=40000000, units_multiplier=UnitsMultiplier.MEGA)

    def build_optional_profile_inputs(self, model, profile):
        # No optional inputs for this profile
        pass

    def build_advanced_profile_inputs(self, model, profile):
        self.make_linked_io(profile, model.vars.ble_feature, 'Advanced', readable_name="Bluetooth LE PHY Feature Selection")
        pass

    def build_hidden_profile_inputs(self, model, profile):
        # Hidden inputs to allow for fixed frame length testing
        self.make_hidden_input(profile, model.vars.frame_length_type, 'frame_general',
                               readable_name="Frame Length Algorithm")
        self.make_hidden_input(profile, model.vars.fixed_length_size, category='frame_fixed_length',
                               readable_name="Fixed Payload Size", value_limit_min=0, value_limit_max=4095)
        self.make_hidden_input(profile, model.vars.frame_bitendian, category='frame_general',
                                   readable_name="Frame Bit Endian")

        # Hidden input for changing syncword
        self.make_hidden_input(profile, model.vars.syncword_0, "syncword", readable_name="Sync Word 0",
                                     value_limit_min=long(0), value_limit_max=long(0xffffffff))
        self.make_hidden_input(profile, model.vars.syncword_1, "syncword", readable_name="Sync Word 1",
                                     value_limit_min=long(0), value_limit_max=long(0xffffffff))

        # Hidden input for CRC
        IProfile.make_hidden_input(profile, model.vars.payload_crc_en, category='frame_payload', readable_name="Insert/Check CRC after payload")
        IProfile.make_hidden_input(profile, model.vars.crc_poly, category='crc', readable_name="CRC Polynomial")
        IProfile.make_hidden_input(profile, model.vars.crc_seed,        category='crc', readable_name="CRC Seed", value_limit_min=long(0), value_limit_max=long(0xffffffff))
        IProfile.make_hidden_input(profile, model.vars.crc_byte_endian, category='crc', readable_name="CRC Byte Endian")
        IProfile.make_hidden_input(profile, model.vars.crc_bit_endian, category='crc', readable_name="CRC Output Bit Endian")
        IProfile.make_hidden_input(profile, model.vars.crc_pad_input, category='crc', readable_name="CRC Input Padding")
        IProfile.make_hidden_input(profile, model.vars.crc_input_order, category='crc', readable_name="CRC Input Bit Endian")
        IProfile.make_hidden_input(profile, model.vars.crc_invert, category='crc', readable_name="CRC Invert")

        # Hidden input for BER testing
        IProfile.make_hidden_input(profile, model.vars.test_ber, category="testing", readable_name="Reconfigure for BER testing")

    def build_metadata_profile_inputs(self, model, profile):
        self.make_metadata_input(profile, model.vars.chcfg_base_frequency_hz, "metadata",
                                 readable_name="Channel Config Base Channel Frequency", value_limit_min=100000000,
                                 value_limit_max=2500000000, units_multiplier=UnitsMultiplier.MEGA)

        # Hidden input needed for rail_test generation
        self.make_metadata_input(profile, model.vars.chcfg_channel_spacing_hz, "metadata",
                                 readable_name="Channel Config Channel Spacing", value_limit_min=0,
                                 value_limit_max=10000000,
                                 units_multiplier=UnitsMultiplier.KILO)

        self.make_metadata_input(profile, model.vars.chcfg_channel_number_start, 'metadata', readable_name='Channel Config Start channel index', value_limit_min=0, value_limit_max=99)
        self.make_metadata_input(profile, model.vars.chcfg_channel_number_end, 'metadata', readable_name='Channel Config Last channel index', value_limit_min=0, value_limit_max=99)
        self.make_metadata_input(profile, model.vars.chcfg_physical_channel_offset, 'metadata', readable_name='Channel Config Physical channel offset', value_limit_min=0, value_limit_max=99)
        self.make_metadata_input(profile, model.vars.rail_tx_power_max, 'metadata', readable_name='Max TX Power', value_limit_min=[-1], value_limit_max=[999])

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_panther(model, profile)

    def build_variable_profile_outputs(self, model, profile):
        self.buildRailOutputs(model, profile)
        build_ircal_sw_vars(model, profile)

    def build_info_profile_outputs(self, model, profile):
        buildModemInfoOutputs(model, profile)
        profile.outputs.append(ModelOutput(model.vars.base_frequency_hz, '', ModelOutputType.INFO, readable_name="Base Channel Frequency"))
        profile.outputs.append(ModelOutput(model.vars.channel_spacing_hz, '', ModelOutputType.INFO, readable_name="Channel Spacing"))
        profile.outputs.append(ModelOutput(model.vars.afc_period, '', ModelOutputType.INFO, readable_name="Frequency Offset Compensation (AFC) Period"))
        profile.outputs.append(ModelOutput(model.vars.afc_step_scale, '', ModelOutputType.INFO, readable_name="Frequency Offset Compensation (AFC) Step Scale"))
        profile.outputs.append(ModelOutput(model.vars.agc_hysteresis, '', ModelOutputType.INFO, readable_name="AGC Hysteresis"))
        profile.outputs.append(ModelOutput(model.vars.agc_period, '', ModelOutputType.INFO, readable_name="AGC Period"))
        profile.outputs.append(ModelOutput(model.vars.agc_power_target, '', ModelOutputType.INFO, readable_name="AGC Power Target"))
        profile.outputs.append(ModelOutput(model.vars.agc_scheme, '', ModelOutputType.INFO, readable_name="AGC backoff scheme"))
        profile.outputs.append(ModelOutput(model.vars.agc_settling_delay, '', ModelOutputType.INFO, readable_name="AGC Settling Delay"))
        profile.outputs.append(ModelOutput(model.vars.agc_speed, '', ModelOutputType.INFO, readable_name="AGC Speed"))
        profile.outputs.append(ModelOutput(model.vars.bandwidth_hz, '', ModelOutputType.INFO, readable_name="Acquisition Channel Bandwidth"))
        profile.outputs.append(ModelOutput(model.vars.errors_in_timing_window, '', ModelOutputType.INFO, readable_name="Number of Errors Allowed in a Timing Window"))
        profile.outputs.append(ModelOutput(model.vars.etsi_cat1_compatible, '', ModelOutputType.INFO, readable_name="ETSI Category 1 Compatibility"))
        profile.outputs.append(ModelOutput(model.vars.firstframe_bitsperword, '', ModelOutputType.INFO, readable_name="Length of the First Word"))
        profile.outputs.append(ModelOutput(model.vars.freq_offset_hz, '', ModelOutputType.INFO, readable_name="Frequency Offset Compensation (AFC) Limit"))
        profile.outputs.append(ModelOutput(model.vars.frequency_comp_mode, '', ModelOutputType.INFO, readable_name="Frequency Compensation Mode"))
        profile.outputs.append(ModelOutput(model.vars.frequency_offset_period, '', ModelOutputType.INFO, readable_name="Frequency Offset Period"))
        profile.outputs.append(ModelOutput(model.vars.if_frequency_hz, '', ModelOutputType.INFO, readable_name="IF Frequency"))
        profile.outputs.append(ModelOutput(model.vars.ircal_power_level, '', ModelOutputType.INFO, readable_name="IR cal power level (amplitude)"))
        profile.outputs.append(ModelOutput(model.vars.ircal_rxtx_path_common, '', ModelOutputType.INFO, readable_name="Common RX/TX circuit"))
        profile.outputs.append(ModelOutput(model.vars.lo_injection_side, '', ModelOutputType.INFO, readable_name="Injection side"))
        profile.outputs.append(ModelOutput(model.vars.number_of_timing_windows, '', ModelOutputType.INFO, readable_name="Number of Timing Windows to Detect"))
        profile.outputs.append(ModelOutput(model.vars.pll_bandwidth_rx, '', ModelOutputType.INFO, readable_name="PLL Bandwidth in RX mode"))
        profile.outputs.append(ModelOutput(model.vars.pll_bandwidth_tx, '', ModelOutputType.INFO, readable_name="PLL Bandwidth in TX mode"))
        profile.outputs.append(ModelOutput(model.vars.src_disable, '', ModelOutputType.INFO, readable_name="SRC Operation"))
        profile.outputs.append(ModelOutput(model.vars.symbols_in_timing_window, '', ModelOutputType.INFO, readable_name="Number of Symbols in Timing Window"))
        profile.outputs.append(ModelOutput(model.vars.target_osr, '', ModelOutputType.INFO, readable_name="Target oversampling rate"))
        profile.outputs.append(ModelOutput(model.vars.timing_detection_threshold, '', ModelOutputType.INFO, readable_name="Timing Detection Threshold"))
        profile.outputs.append(ModelOutput(model.vars.timing_resync_period, '', ModelOutputType.INFO, readable_name="Timing Resync Period"))
        profile.outputs.append(ModelOutput(model.vars.timing_sample_threshold, '', ModelOutputType.INFO, readable_name="Timing Samples Threshold"))
        profile.outputs.append(ModelOutput(model.vars.var_length_loc, '', ModelOutputType.INFO, readable_name="Byte position of dynamic length byte"))
        profile.outputs.append(ModelOutput(model.vars.rssi_period, '', ModelOutputType.INFO, readable_name="RSSI Update Period"))
        profile.outputs.append(ModelOutput(model.vars.antdivmode, '', ModelOutputType.INFO, readable_name="Antenna diversity mode"))
        profile.outputs.append(ModelOutput(model.vars.antdivrepeatdis, '', ModelOutputType.INFO, readable_name="Diversity Select-Best repeat"))

    def buildRailOutputs(self, model, profile):
        profile.outputs.append(ModelOutput(model.vars.frequency_offset_factor, '', ModelOutputType.RAIL_CONFIG, readable_name='Frequency Offset Factor'))
        profile.outputs.append(ModelOutput(model.vars.frequency_offset_factor_fxp, '', ModelOutputType.RAIL_CONFIG, readable_name='Frequency Offset Factor FXP'))
        profile.outputs.append(ModelOutput(model.vars.dynamic_slicer_enabled, '', ModelOutputType.RAIL_CONFIG, readable_name='Dynamic Slicer Feature Enabled'))
        profile.outputs.append(ModelOutput(model.vars.dynamic_slicer_threshold_values, '', ModelOutputType.RAIL_CONFIG, readable_name='Dynamic Slicer Threshold Values'))
        profile.outputs.append(ModelOutput(model.vars.dynamic_slicer_level_values, '', ModelOutputType.RAIL_CONFIG, readable_name='Dynamic Slicer Level Values'))
        profile.outputs.append(ModelOutput(model.vars.src1_calcDenominator, '', ModelOutputType.RAIL_CONFIG, readable_name='SRC1 Helper Calculation'))
        profile.outputs.append(ModelOutput(model.vars.src2_calcDenominator, '', ModelOutputType.RAIL_CONFIG, readable_name='SRC2 Helper Calculation'))
        profile.outputs.append(ModelOutput(model.vars.tx_baud_rate_actual, '', ModelOutputType.RAIL_CONFIG, readable_name='TX Baud Rate'))
        profile.outputs.append(ModelOutput(model.vars.baud_per_symbol_actual, '', ModelOutputType.RAIL_CONFIG, readable_name='Number of baud to transmit 1 symbol'))
        profile.outputs.append(ModelOutput(model.vars.bits_per_symbol_actual, '', ModelOutputType.RAIL_CONFIG, readable_name='Number of bits contained in 1 symbol'))
        profile.outputs.append(ModelOutput(model.vars.rx_ch_hopping_order_num, '', ModelOutputType.RAIL_CONFIG, readable_name='For receive scanning PHYs: order of PHY in scanning sequence'))
        profile.outputs.append(ModelOutput(model.vars.rx_ch_hopping_mode, '', ModelOutputType.RAIL_CONFIG, readable_name='For receive scanning PHYs: event to trigger a hop to next PHY'))
        profile.outputs.append(ModelOutput(model.vars.rx_ch_hopping_delay_usec, '', ModelOutputType.RAIL_CONFIG,readable_name='For receive scanning PHYs: delay in microseconds to look for RX on a particular PHY'))
        profile.outputs.append(ModelOutput(model.vars.div_antdivmode, '', ModelOutputType.RAIL_CONFIG, readable_name='Antenna diversity mode'))
        profile.outputs.append(ModelOutput(model.vars.div_antdivrepeatdis, '', ModelOutputType.RAIL_CONFIG, readable_name='Disable repeated measurement of first antenna when Select-Best algorithm is used'))
        profile.outputs.append(ModelOutput(model.vars.stack_info, '', ModelOutputType.RAIL_CONFIG, readable_name='Stack information containing protocol and PHY IDs'))
        profile.outputs.append(ModelOutput(model.vars.rssi_adjust_db, '', ModelOutputType.RAIL_CONFIG, readable_name='RSSI compensation value calculated from decimation and digital gains'))
        profile.outputs.append(ModelOutput(model.vars.rx_sync_delay_ns, '', ModelOutputType.RAIL_CONFIG, readable_name='Receive chain delay ns'))
        profile.outputs.append(ModelOutput(model.vars.rx_eof_delay_ns, '', ModelOutputType.RAIL_CONFIG, readable_name='Receive end of frame delay ns'))
        profile.outputs.append(ModelOutput(model.vars.tx_sync_delay_ns, '', ModelOutputType.RAIL_CONFIG, readable_name='Time needed from TXEN to start of preamble on-air'))
        profile.outputs.append(ModelOutput(model.vars.tx_eof_delay_ns, '', ModelOutputType.RAIL_CONFIG, readable_name='Time from end of frame on-air to TX EOF timestamp'))

    def profile_calculate(self, model):
        model.vars.protocol_id.value_forced = model.vars.protocol_id.var_enum.BLE

        self._build_feature_settings(model)
        self._build_delay_settings(model)

    def _build_feature_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M')
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_2M_Viterbi_BLEIQDSA')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LR_125k')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LR_500k')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M_AOX')

    def _build_delay_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 5500
            model.vars.tx_sync_delay_ns.value_forced = 1500
            model.vars.tx_eof_delay_ns.value_forced = 1500
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0

    def _copy_model_variables_from_phy(self, model, phy_name):

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

