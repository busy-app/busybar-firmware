from pyradioconfig.parts.ocelot.profiles.Profile_BLE import ProfileBLEOcelot
from pyradioconfig.parts.common.profiles.margay_regs import build_modem_regs_margay
from pyradioconfig.parts.margay.profiles.sw_profile_outputs_common import sw_profile_outputs_common_margay


class ProfileBLEMargay(ProfileBLEOcelot):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "margay"
        self._sw_profile_outputs_common = sw_profile_outputs_common_margay()

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_margay(model, profile)