from pyradioconfig.parts.ocelot.phys.Phys_Studio_IEEE802154 import PhysStudioIEEE802154Ocelot
from py_2_and_3_compatibility import *


class PhysStudioIEEE802154Margay(PhysStudioIEEE802154Ocelot):

    def PHY_IEEE802154_2p4GHz_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154,
                            readable_name='Coherent IEEE 802.15.4 2p4GHz PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.COHERENT

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16

        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH0_prod(self, model, phy_name=None):
        phy = super().PHY_IEEE802154_GB868_863MHz_PHR2_CH0_prod(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 39000000
        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH1_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 1 PHY', phy_name=phy_name)

        phy = super().PHY_IEEE802154_GB868_863MHz_PHR2_CH1_prod(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH2_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 2 PHY', phy_name=phy_name)

        phy = super().PHY_IEEE802154_GB868_863MHz_PHR2_CH2_prod(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH3_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 3 PHY', phy_name=phy_name)

        phy = super().PHY_IEEE802154_GB868_863MHz_PHR2_CH3_prod(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        return phy

    def PHY_IEEE802154_GB868_915MHz_PHR2_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 915 MHz PHY', phy_name=phy_name)

        phy = super().PHY_IEEE802154_GB868_915MHz_PHR2_prod(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        return phy

    def PHY_IEEE802154_915MHz_2GFSK_R23_NA_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 2GFSK R23 NA 915 MHz PHY', phy_name=phy_name)

        phy = super().PHY_IEEE802154_915MHz_2GFSK_R23_NA_prod(model, phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        return phy