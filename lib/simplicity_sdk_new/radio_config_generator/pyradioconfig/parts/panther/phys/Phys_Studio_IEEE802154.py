from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy


class PhysStudioIEEE802154Panther(IPhy):

    def PHY_IEEE802154_2p4GHz_antdiv_prod(self, model, phy_name=None):
        #  PHY_IEEE802154_2p4GHz legacy, plus RSSI-based select best with antdiv repeat = NOREPEATFIRST
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK, readable_name='Production IEEE 802.15.4 2p4GHz antenna diversity PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.ANTDIV

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16

        return phy

    def PHY_IEEE802154_2p4GHz_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK, readable_name='Production IEEE 802.15.4 2p4GHz PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.COHERENT

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16

        return phy

    def PHY_IEEE802154_2p4GHz_antdiv_fastswitch_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK, readable_name='Production IEEE 802.15.4 2p4GHz Concurrent PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.FCS

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16

        return phy
