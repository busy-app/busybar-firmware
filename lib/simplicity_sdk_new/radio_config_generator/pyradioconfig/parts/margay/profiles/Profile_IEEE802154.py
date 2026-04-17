from pyradioconfig.parts.ocelot.profiles.Profile_IEEE802154 import ProfileIEEE802154Ocelot
from pyradioconfig.parts.common.profiles.margay_regs import build_modem_regs_margay
from pyradioconfig.parts.margay.profiles.sw_profile_outputs_common import sw_profile_outputs_common_margay


class ProfileIEEE802154Margay(ProfileIEEE802154Ocelot):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154"
        self._readable_name = "IEEE802154 Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 phys"
        self._default = False
        self._activation_logic = ""
        self._family = "margay"
        self._sw_profile_outputs_common = sw_profile_outputs_common_margay()

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_margay(model, profile)

    def _build_feature_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.COHERENT:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz_cohdsa')
        else:
            super()._build_feature_settings(model)

    def _build_delay_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.COHERENT:
            model.vars.rx_sync_delay_ns.value_forced = 2964
            model.vars.rx_eof_delay_ns.value_forced = 2964
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0
        else:
            super()._build_delay_settings(model)
