from pyradioconfig.parts.ocelot.profiles.Profile_IEEE802154 import ProfileIEEE802154Ocelot
from pyradioconfig.parts.sol.profiles.sw_profile_outputs_common import sw_profile_outputs_common_sol


class ProfileIEEE802154Sol(ProfileIEEE802154Ocelot):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154"
        self._readable_name = "IEEE802154 Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 phys"
        self._default = False
        self._activation_logic = ""
        self._family = "sol"
        self._sw_profile_outputs_common = sw_profile_outputs_common_sol()

    def build_advanced_profile_inputs(self, model, profile):
        super().build_advanced_profile_inputs(model, profile)
        self.make_linked_io(profile, model.vars.fpll_band, 'crystal', readable_name="RF Frequency Planning Band")

    def _build_feature_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.GB868_863:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_GB868_863MHz_PHR2')
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.GB868_915:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_GB868_915MHz_PHR2')
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.NA915_R23:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_915MHz_2GFSK_R23_NA')

    def _build_delay_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.GB868_863:
            model.vars.rx_sync_delay_ns.value_forced = 56500
            model.vars.rx_eof_delay_ns.value_forced = 56500
            model.vars.tx_sync_delay_ns.value_forced = 10000
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.GB868_915:
            model.vars.rx_sync_delay_ns.value_forced = 56500
            model.vars.rx_eof_delay_ns.value_forced = 56500
            model.vars.tx_sync_delay_ns.value_forced = 10000
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.NA915_R23:
            model.vars.rx_sync_delay_ns.value_forced = 56500
            model.vars.rx_eof_delay_ns.value_forced = 56500
            model.vars.tx_sync_delay_ns.value_forced = 2000
            model.vars.tx_eof_delay_ns.value_forced = 0