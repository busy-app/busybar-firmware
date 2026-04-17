from pyradioconfig.parts.common.profiles.leopard_regs import build_modem_regs_leopard
from pyradioconfig.parts.lynx.profiles.Profile_IEEE802154_OQPSK import ProfileIEEE802154OQPSKLynx


class ProfileIEEE802154OQPSKLeopard(ProfileIEEE802154OQPSKLynx):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154OQPSK"
        self._readable_name = "IEEE802154 OQPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 OQPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "leopard"

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_leopard(model, profile)