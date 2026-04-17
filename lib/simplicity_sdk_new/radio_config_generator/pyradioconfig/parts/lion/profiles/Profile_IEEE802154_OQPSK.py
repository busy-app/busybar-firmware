from pyradioconfig.parts.common.profiles.lion_regs import build_modem_regs_lion
from pyradioconfig.parts.leopard.profiles.Profile_IEEE802154_OQPSK import ProfileIEEE802154OQPSKLeopard


class ProfileIEEE802154OQPSKLion(ProfileIEEE802154OQPSKLeopard):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154OQPSK"
        self._readable_name = "IEEE802154 OQPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 OQPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "lion"

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_lion(model, profile)