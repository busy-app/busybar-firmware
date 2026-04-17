from pyradioconfig.parts.ocelot.phys.Phys_Internal_Base_Standard_IEEE802154 import PhysInternalBaseStandardIEEE802154Ocelot


class PhysRAILBaseStandardIEEE802154Margay(PhysInternalBaseStandardIEEE802154Ocelot):

    def PHY_IEEE802154_2p4GHz_cohdsa(self, model, phy_name=None):
        phy = super().PHY_IEEE802154_2p4GHz_cohdsa(model, phy_name=None)
        # Remove Ocelot overrides as they are irrelevant
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = None
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = None
        return phy

    def PHY_IEEE802154_2p4GHz_cohdsa_diversity(self, model, phy_name=None):
        phy = super().PHY_IEEE802154_2p4GHz_cohdsa_diversity(model, phy_name=None)
        # Remove Ocelot overrides as they are irrelevant
        phy.profile_outputs.MODEM_TXBR_TXBRDEN.override = None
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = None
        return phy
