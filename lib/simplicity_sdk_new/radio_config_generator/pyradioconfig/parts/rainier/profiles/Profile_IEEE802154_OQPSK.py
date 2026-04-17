from pyradioconfig.parts.common.profiles.rainier_regs import build_modem_regs_rainier
from pyradioconfig.parts.rainier.profiles.sw_profile_outputs_common import SwProfileOutputsCommonRainier
from pyradioconfig.parts.bobcat.profiles.Profile_IEEE802154_OQPSK import ProfileIEEE802154OQPSKBobcat


class ProfileIEEE802154OQPSKRainier(ProfileIEEE802154OQPSKBobcat):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154OQPSK"
        self._readable_name = "IEEE802154OQPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 OQPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "rainier"
        self._sw_profile_outputs_common = SwProfileOutputsCommonRainier()
        self._skip_target_calculation = True

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_rainier(model, profile)

    def build_hidden_profile_inputs(self, model, profile):
        super().build_hidden_profile_inputs(model, profile)
        self.make_hidden_input(profile, model.vars.modulator_select, category="Advanced", readable_name="Modulator Select")

    def _build_delay_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.COHERENT:
            model.vars.rx_sync_delay_ns.value_forced = 2970
            model.vars.rx_eof_delay_ns.value_forced = 6970
            model.vars.tx_sync_delay_ns.value_forced = 2500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV:
            model.vars.rx_sync_delay_ns.value_forced = 6625
            model.vars.rx_eof_delay_ns.value_forced = 6625
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.FEM:
            model.vars.rx_sync_delay_ns.value_forced = 2970
            model.vars.rx_eof_delay_ns.value_forced = 6970
            model.vars.tx_sync_delay_ns.value_forced = 2500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_FEM:
            model.vars.rx_sync_delay_ns.value_forced = 6625
            model.vars.rx_eof_delay_ns.value_forced = 6625
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0