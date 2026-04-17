from pyradioconfig.parts.common.profiles.bobcat_regs import build_modem_regs_bobcat
from pyradioconfig.parts.bobcat.profiles.sw_profile_outputs_common import sw_profile_outputs_common_bobcat
from pyradioconfig.parts.ocelot.profiles.Profile_IEEE802154 import ProfileIEEE802154Ocelot


class ProfileIEEE802154OQPSKBobcat(ProfileIEEE802154Ocelot):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154OQPSK"
        self._readable_name = "IEEE802154OQPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 OQPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "bobcat"
        self._sw_profile_outputs_common = sw_profile_outputs_common_bobcat()

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_bobcat(model, profile)

    def _build_feature_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.COHERENT:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz_cohdsa')
        elif (zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV or
              zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_COEX):
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz_diversity')
        elif (zigbee_feature == model.vars.zigbee_feature.var_enum.FEM or
              zigbee_feature == model.vars.zigbee_feature.var_enum.COEX_FEM):
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz_cohdsa_fem')
        elif (zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_FEM or
              zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_COEX_FEM):
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz_diversity_fem')
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.FCS:
            self._copy_model_variables_from_phy(model, 'PHY_IEEE802154_2p4GHz_antdiv_fastswitch')

    def _build_delay_settings(self, model):
        zigbee_feature = model.profile.inputs.zigbee_feature.var_value

        if zigbee_feature == model.vars.zigbee_feature.var_enum.COHERENT:
            model.vars.rx_sync_delay_ns.value_forced = 6125
            model.vars.rx_eof_delay_ns.value_forced = 6125
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif (zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV or
              zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_COEX):
            model.vars.rx_sync_delay_ns.value_forced = 6625
            model.vars.rx_eof_delay_ns.value_forced = 6625
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif (zigbee_feature == model.vars.zigbee_feature.var_enum.FEM or
              zigbee_feature == model.vars.zigbee_feature.var_enum.COEX_FEM):
            model.vars.rx_sync_delay_ns.value_forced = 6125
            model.vars.rx_eof_delay_ns.value_forced = 6125
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif (zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_FEM or
              zigbee_feature == model.vars.zigbee_feature.var_enum.ANTDIV_COEX_FEM):
            model.vars.rx_sync_delay_ns.value_forced = 6625
            model.vars.rx_eof_delay_ns.value_forced = 6625
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif zigbee_feature == model.vars.zigbee_feature.var_enum.FCS:
            model.vars.rx_sync_delay_ns.value_forced = 6625
            model.vars.rx_eof_delay_ns.value_forced = 6625
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 0

