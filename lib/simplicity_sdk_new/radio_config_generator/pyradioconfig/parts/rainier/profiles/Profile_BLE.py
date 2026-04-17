from pyradioconfig.parts.common.profiles.rainier_regs import build_modem_regs_rainier
from pyradioconfig.parts.bobcat.profiles.Profile_BLE import ProfileBLEBobcat
from pyradioconfig.parts.rainier.profiles.sw_profile_outputs_common import SwProfileOutputsCommonRainier


class ProfileBLERainier(ProfileBLEBobcat):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "rainier"
        self._sw_profile_outputs_common = SwProfileOutputsCommonRainier()
        self._skip_target_calculation = True

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_rainier(model, profile)

    def _build_feature_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_Viterbi_noDSA')
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LongRange_NOdsa_125kbps')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LongRange_NOdsa_500kbps')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M_AOX')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_2M_AOX')
        elif ble_feature == model.vars.ble_feature.var_enum.CONCURRENT:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M_Concurrent')
        elif ble_feature == model.vars.ble_feature.var_enum.HADM_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M_HADM')
        elif ble_feature == model.vars.ble_feature.var_enum.HADM_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_2M_HADM')
        elif ble_feature == model.vars.ble_feature.var_enum.HADM_2M_2BT:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_2M_HADM_2BT')

    def _build_delay_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 9980
            model.vars.tx_sync_delay_ns.value_forced = 3210
            model.vars.tx_eof_delay_ns.value_forced = 1106
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            model.vars.rx_sync_delay_ns.value_forced = 12492
            model.vars.rx_eof_delay_ns.value_forced = 5460
            model.vars.tx_sync_delay_ns.value_forced = 1520
            model.vars.tx_eof_delay_ns.value_forced = 416
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            model.vars.rx_sync_delay_ns.value_forced = 7000
            model.vars.rx_eof_delay_ns.value_forced = 7930
            model.vars.tx_sync_delay_ns.value_forced = 3182
            model.vars.tx_eof_delay_ns.value_forced = 1058
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            model.vars.rx_sync_delay_ns.value_forced = 6750
            model.vars.rx_eof_delay_ns.value_forced = 7930
            model.vars.tx_sync_delay_ns.value_forced = 3182
            model.vars.tx_eof_delay_ns.value_forced = 1058
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 9980
            model.vars.tx_sync_delay_ns.value_forced = 3210
            model.vars.tx_eof_delay_ns.value_forced = 1106
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_2M:
            model.vars.rx_sync_delay_ns.value_forced = 12492
            model.vars.rx_eof_delay_ns.value_forced = 5460
            model.vars.tx_sync_delay_ns.value_forced = 1520
            model.vars.tx_eof_delay_ns.value_forced = 416
        elif ble_feature == model.vars.ble_feature.var_enum.CONCURRENT:
            model.vars.rx_sync_delay_ns.value_forced = 50000
            model.vars.rx_eof_delay_ns.value_forced = 10700
            model.vars.tx_sync_delay_ns.value_forced = 2000
            model.vars.tx_eof_delay_ns.value_forced = 2000
        elif ble_feature == model.vars.ble_feature.var_enum.HADM_1M:
            model.vars.rx_sync_delay_ns.value_forced = 24000
            model.vars.rx_eof_delay_ns.value_forced = 8000
            model.vars.tx_sync_delay_ns.value_forced = 0
            model.vars.tx_eof_delay_ns.value_forced = 0
        elif ble_feature == model.vars.ble_feature.var_enum.HADM_2M:
            model.vars.rx_sync_delay_ns.value_forced = 12492
            model.vars.rx_eof_delay_ns.value_forced = 5460
            model.vars.tx_sync_delay_ns.value_forced = 1520
            model.vars.tx_eof_delay_ns.value_forced = 416
        elif ble_feature == model.vars.ble_feature.var_enum.HADM_2M_2BT:
            model.vars.rx_sync_delay_ns.value_forced = 12492
            model.vars.rx_eof_delay_ns.value_forced = 5460
            model.vars.tx_sync_delay_ns.value_forced = 1520
            model.vars.tx_eof_delay_ns.value_forced = 416