from pyradioconfig.parts.common.profiles.caracal_regs import build_momem_regs_caracal
from pyradioconfig.parts.bobcat.profiles.Profile_BLE import ProfileBLEBobcat
from pyradioconfig.parts.caracal.profiles.sw_profile_outputs_common import sw_profile_outputs_common_caracal


class ProfileBLECaracal(ProfileBLEBobcat):

    def __init__(self):
        super().__init__()
        self._profileName = "BLE"
        self._readable_name = "Bluetooth Low Energy Profile"
        self._category = ""
        self._description = "Profile used for BLE phys"
        self._default = False
        self._activation_logic = ""
        self._family = "caracal"
        self._sw_profile_outputs_common = sw_profile_outputs_common_caracal()

    def build_register_profile_outputs(self, model, profile):
        build_momem_regs_caracal(model, profile)
