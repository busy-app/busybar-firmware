from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.margay.phys.Phys_Studio_Base_Standard_SUNFSK import PHYS_Studio_Base_Standard_SUNFSK_Margay


class PHYS_Studio_Base_Standard_SUNFSK_Serval(IPhy):

    def PHY_IEEE802154_SUN_FSK_169MHz_2FSK_2p4kbps(self, model):
        phy = PHYS_Studio_Base_Standard_SUNFSK_Margay().PHY_IEEE802154_SUN_FSK_169MHz_2FSK_2p4kbps(model)

    def PHY_IEEE802154_SUN_FSK_169MHz_2FSK_4p8kbps(self, model):
        phy = PHYS_Studio_Base_Standard_SUNFSK_Margay().PHY_IEEE802154_SUN_FSK_169MHz_2FSK_4p8kbps(model)

    def PHY_IEEE802154_SUN_FSK_169MHz_4FSK_9p6kbps(self, model):
        phy = PHYS_Studio_Base_Standard_SUNFSK_Margay().PHY_IEEE802154_SUN_FSK_169MHz_4FSK_9p6kbps(model)

    def PHY_IEEE802154_SUN_FSK_896MHz_2FSK_40kbps(self, model):
        phy = PHYS_Studio_Base_Standard_SUNFSK_Margay().PHY_IEEE802154_SUN_FSK_896MHz_2FSK_40kbps(model)

    def PHY_IEEE802154_SUN_FSK_920MHz_4FSK_400kbps(self, model):
        phy = PHYS_Studio_Base_Standard_SUNFSK_Margay().PHY_IEEE802154_SUN_FSK_920MHz_4FSK_400kbps(model)
