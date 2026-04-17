from pyradioconfig.parts.common.profiles.leopard_regs import build_modem_regs_leopard
from pyradioconfig.parts.lynx.profiles.Profile_BLE import ProfileBLELynx


class ProfileBLELeopard(ProfileBLELynx):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "leopard"

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_leopard(model, profile)

    def _build_delay_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 9000
            model.vars.tx_sync_delay_ns.value_forced = 500
            model.vars.tx_eof_delay_ns.value_forced = 500
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            model.vars.rx_sync_delay_ns.value_forced = 187125
            model.vars.rx_eof_delay_ns.value_forced = 10000
            model.vars.tx_sync_delay_ns.value_forced = 600
            model.vars.tx_eof_delay_ns.value_forced = 600
        else:
            super()._build_delay_settings(model)