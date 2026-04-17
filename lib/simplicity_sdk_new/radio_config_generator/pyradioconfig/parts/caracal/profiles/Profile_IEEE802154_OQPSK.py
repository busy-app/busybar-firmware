from pyradioconfig.parts.common.profiles.caracal_regs import build_momem_regs_caracal
from pyradioconfig.parts.bobcat.profiles.sw_profile_outputs_common import sw_profile_outputs_common_bobcat
from pyradioconfig.parts.bobcat.profiles.Profile_IEEE802154_OQPSK import ProfileIEEE802154OQPSKBobcat


class ProfileIEEE802154OQPSKCaracal(ProfileIEEE802154OQPSKBobcat):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154OQPSK"
        self._readable_name = "IEEE802154OQPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 OQPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "caracal"
        self._sw_profile_outputs_common = sw_profile_outputs_common_bobcat()

    def build_register_profile_outputs(self, model, profile):
        build_momem_regs_caracal(model, profile)