from pyradioconfig.parts.common.profiles.lynx_regs import build_modem_regs_lynx
from pyradioconfig.parts.panther.profiles.Profile_IEEE802154_OQPSK import ProfileIEEE802154OQPSKPanther


class ProfileIEEE802154OQPSKLynx(ProfileIEEE802154OQPSKPanther):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154OQPSK"
        self._readable_name = "IEEE802154 OQPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 OQPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "lynx"

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_lynx(model, profile)

    def _build_feature_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.LEGACY:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz')

    def _build_delay_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.LEGACY:
            model.vars.rx_sync_delay_ns.value_forced = 6125
            model.vars.rx_eof_delay_ns.value_forced = 6125
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0