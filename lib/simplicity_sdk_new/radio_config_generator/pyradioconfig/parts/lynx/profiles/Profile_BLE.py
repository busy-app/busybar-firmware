from pyradioconfig.parts.common.profiles.lynx_regs import build_modem_regs_lynx
from pyradioconfig.parts.panther.profiles.Profile_BLE import ProfileBLEPanther
from pyradioconfig.calculator_model_framework.CalcManager import CalcManager


class ProfileBLELynx(ProfileBLEPanther):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "lynx"

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_lynx(model, profile)

    def _build_feature_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_Viterbi_noDSA')
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LongRange_dsa_125kbps')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LongRange_dsa_500kbps')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.CONCURRENT:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M_Concurrent')

    def _build_delay_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 8700
            model.vars.tx_sync_delay_ns.value_forced = 435
            model.vars.tx_eof_delay_ns.value_forced = 435
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 5200
            model.vars.tx_sync_delay_ns.value_forced = 250
            model.vars.tx_eof_delay_ns.value_forced = 250
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            model.vars.rx_sync_delay_ns.value_forced = 187125
            model.vars.rx_eof_delay_ns.value_forced = 10000
            model.vars.tx_sync_delay_ns.value_forced = 300
            model.vars.tx_eof_delay_ns.value_forced = 300
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            model.vars.rx_sync_delay_ns.value_forced = 49125
            model.vars.rx_eof_delay_ns.value_forced = 10000
            model.vars.tx_sync_delay_ns.value_forced = 450
            model.vars.tx_eof_delay_ns.value_forced = 450
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_2M:
            model.vars.rx_sync_delay_ns.value_forced = 0
            model.vars.rx_eof_delay_ns.value_forced = 0
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif ble_feature == model.vars.ble_feature.var_enum.CONCURRENT:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 11750
            model.vars.tx_sync_delay_ns.value_forced = 2000
            model.vars.tx_eof_delay_ns.value_forced = 2000
