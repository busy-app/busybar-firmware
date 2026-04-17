from pyradioconfig.parts.common.profiles.lion_regs import build_modem_regs_lion
from pyradioconfig.parts.leopard.profiles.Profile_BLE import ProfileBLELeopard


class ProfileBLELion(ProfileBLELeopard):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "lion"

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_lion(model, profile)

    def _build_feature_settings(self, model):
        ble_feature = model.profile.inputs.ble_feature.var_value

        if ble_feature == model.vars.ble_feature.var_enum.LE_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_Viterbi_noDSA')
        elif ble_feature == model.vars.ble_feature.var_enum.LE_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_125K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LongRange_nodsa_125kbps')
        elif ble_feature == model.vars.ble_feature.var_enum.CODED_500K:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LongRange_nodsa_500kbps')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_1M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.AOX_2M:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate')
        elif ble_feature == model.vars.ble_feature.var_enum.CONCURRENT:
            self._copy_model_variables_from_phy(model, 'PHY_Bluetooth_1M_Concurrent')