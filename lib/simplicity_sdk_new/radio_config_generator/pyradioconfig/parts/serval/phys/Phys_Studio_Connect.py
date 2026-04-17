from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.margay.phys.Phys_Studio_Connect import PHYS_connect_Margay


class PHYS_Studio_Connect_Serval(IPhy):

    def PHY_Studio_Connect_915MHz_2GFSK_500kbps(self, model):
        phy = PHYS_connect_Margay().PHY_Studio_Connect_915MHz_2GFSK_500kbps(model)
        return phy

    def PHY_Studio_Connect_863MHz_2GFSK_100kbps(self, model):
        phy = PHYS_connect_Margay().PHY_Studio_Connect_863MHz_2GFSK_100kbps(model)
        return phy

    def PHY_Studio_Connect_915MHz_OQPSK_500kbps(self, model):
        phy = PHYS_connect_Margay().PHY_Studio_Connect_915MHz_OQPSK_500kbps(model)
        return phy

    def PHY_Studio_Connect_915mhz_oqpsk_800kcps_100kbps(self, model):
        phy = PHYS_connect_Margay().PHY_Studio_Connect_915mhz_oqpsk_800kcps_100kbps(model)
        return phy

    def PHY_Studio_Connect_915mhz_oqpsk_2Mcps_250kbps(self, model):
        phy = PHYS_connect_Margay().PHY_Studio_Connect_915mhz_oqpsk_2Mcps_250kbps(model)
        return phy
