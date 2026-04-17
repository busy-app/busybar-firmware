from pyradioconfig.parts.ocelot.profiles.Profile_IEEE802154_BPSK import ProfileIEEE802154BPSKOcelot
from pyradioconfig.parts.common.profiles.margay_regs import build_modem_regs_margay
from pyradioconfig.parts.margay.profiles.sw_profile_outputs_common import sw_profile_outputs_common_margay


class ProfileIEEE802154BPSKMargay(ProfileIEEE802154BPSKOcelot):

    def __init__(self):
        super().__init__()
        self._profileName = "IEEE802154BPSK"
        self._readable_name = "IEEE802154 BPSK Profile"
        self._category = ""
        self._description = "Profile used for IEEE802154 BPSK phys"
        self._default = False
        self._activation_logic = ""
        self._family = "margay"
        self._sw_profile_outputs_common = sw_profile_outputs_common_margay()

    def build_register_profile_outputs(self, model, profile):
        build_modem_regs_margay(model, profile)

    def _build_modem_settings(self, model):
        super()._build_modem_settings(model)
        model.vars.directmode_rx.value_forced = None
