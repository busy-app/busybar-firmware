from pyradioconfig.parts.bobcat.phys.Phys_Studio_IEEE802154 import PhysStudioIEEE802154Bobcat


class PhysStudioIEEE802154Rainier(PhysStudioIEEE802154Bobcat):

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 38400000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq

    def PHY_IEEE802154_2p4GHz_fastswitch_prod(self, model, phy_name=None):
        pass
