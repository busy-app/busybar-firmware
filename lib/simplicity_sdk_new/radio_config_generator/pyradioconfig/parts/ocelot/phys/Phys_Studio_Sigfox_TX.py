from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.calculator_model_framework.interfaces.iphy import ModelPhy


class Phys_Studio_Sigfox_Tx_Ocelot(IPhy):

    def __configure_sigfox_tx_channel(self, phy: ModelPhy):
        phy.profile_inputs.syncword_tx_skip.value = True
        phy.profile_inputs.xtal_frequency_hz.value = 39_000_000
        phy.profile_inputs.channel_spacing_hz.value = 100
        phy.profile_inputs.fixed_length_size.value = 16
        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 1919

    def PHY_Sigfox_868MHz_DBPSK_100bps_TXOnly(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Sigfox_TX, readable_name='100bps 868MHz DBPSK TX Only PHY', phy_name=phy_name)

        phy.profile_inputs.bitrate.value = 100
        phy.profile_inputs.base_frequency_hz.value = 868_034_000

        self.__configure_sigfox_tx_channel(phy)

        return phy

    def PHY_Sigfox_915MHz_DBPSK_600bps_TXOnly(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Sigfox_TX, readable_name='600bps 915MHz DBPSK TX Only PHY', phy_name=phy_name)

        phy.profile_inputs.bitrate.value = 600
        phy.profile_inputs.base_frequency_hz.value = 905_104_000

        self.__configure_sigfox_tx_channel(phy)

        return phy
