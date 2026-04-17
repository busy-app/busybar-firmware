from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.margay.phys.Phys_Studio_LongRange import PHYS_OQPSK_LoRa_Margay


class PHYS_Studio_LongRange_Serval(IPhy):

    def PHY_Longrange_915M_OQPSK_DSSS8_2p4kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_2p4kbps(model)

    def PHY_Longrange_915M_OQPSK_DSSS8_4p8kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_4p8kbps(model)

    def PHY_Longrange_915M_OQPSK_DSSS8_9p6kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_9p6kbps(model)

    def PHY_Longrange_915M_OQPSK_DSSS8_19p2kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_19p2kbps(model)

    def PHY_Longrange_915M_OQPSK_DSSS8_38p4kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_38p4kbps(model)

    def PHY_Longrange_915M_OQPSK_DSSS8_80p0kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_80p0kbps(model)

    def PHY_Longrange_915M_OQPSK_DSSS8_76p8kbps(self, model):
        phy = PHYS_OQPSK_LoRa_Margay().PHY_Longrange_915M_OQPSK_DSSS8_76p8kbps(model)
