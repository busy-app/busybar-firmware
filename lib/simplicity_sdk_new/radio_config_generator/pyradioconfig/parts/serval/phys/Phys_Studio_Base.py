from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.margay.phys.Phys_Studio_Base import PHYS_Studio_Base_Margay


class PHYS_Studio_Base_Serval(IPhy):

    def PHY_Datasheet_169M_2GFSK_2p4Kbps_1p2K_ETSI(self, model):
        phy = PHYS_Studio_Base_Margay().PHY_Datasheet_169M_2GFSK_2p4Kbps_1p2K_ETSI(model)

    def PHY_Datasheet_868M_2GFSK_2p4Kbps_1p2K_ETSI(self, model):
        phy = PHYS_Studio_Base_Margay().PHY_Datasheet_868M_2GFSK_2p4Kbps_1p2K_ETSI(model)

    def PHY_Studio_915M_OOK_4p8kbps(self, model):
        phy = PHYS_Studio_Base_Margay().PHY_Studio_915M_OOK_4p8kbps(model)

    def PHY_Studio_868M_GMSK_500Kbps(self, model, phy_name=None):
        phy = PHYS_Studio_Base_Margay().PHY_Studio_868M_GMSK_500Kbps(model, phy_name)
        return phy

    def PHY_Studio_915M_2GFSK_500Kbps_175K_mi0p7(self, model, phy_name=None):
        phy = PHYS_Studio_Base_Margay().PHY_Studio_915M_2GFSK_500Kbps_175K_mi0p7(model, phy_name)
        return phy
