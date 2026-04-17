from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.margay.phys.Phys_Studio_Sidewalk import PhysStudioSidewalkMargay


class Phys_Studio_Sidewalk_Serval(IPhy):

    def PHY_Sidewalk_2GFSK_50Kbps(self, model):
        phy = PhysStudioSidewalkMargay().PHY_Sidewalk_2GFSK_50Kbps(model)
