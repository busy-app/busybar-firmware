from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy


class PhysStudioIEEE802154Bobcat(IPhy):


    def _part_specific_phy_overrides(self, phy, model):
        pass

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 39000000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq

    def PHY_IEEE802154_2p4GHz_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK,
                            readable_name='Production IEEE 802.15.4 2p4GHz PHY',
                            phy_name=phy_name)

        self._set_xtal_frequency(phy)
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.COHERENT

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16
        return phy

    def PHY_IEEE802154_2p4GHz_fem_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK,
                            readable_name='Production IEEE 802.15.4 2p4GHz with External LNA PHY',
                            phy_name=phy_name)

        self._set_xtal_frequency(phy)
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.FEM

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16
        return phy

    def PHY_IEEE802154_2p4GHz_diversity_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK, readable_name='Production IEEE 802.15.4 2p4GHz Antenna Diversity PHY',
                            phy_name=phy_name)

        self._set_xtal_frequency(phy)
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.ANTDIV

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16
        return phy

    def PHY_IEEE802154_2p4GHz_diversity_fem_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK, readable_name='Production IEEE 802.15.4 2p4GHz Antenna Diversity with External LNA PHY',
                            phy_name=phy_name)

        self._set_xtal_frequency(phy)
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.ANTDIV_FEM

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16
        return phy

    def PHY_IEEE802154_2p4GHz_fastswitch_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154OQPSK,
                            readable_name='Production IEEE 802.15.4 2p4GHz Concurrent PHY',
                            phy_name=phy_name)

        self._set_xtal_frequency(phy)
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.FCS

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2405000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 5000000

        phy.profile_inputs.chcfg_channel_number_start.value = 11
        phy.profile_inputs.chcfg_channel_number_end.value = 26
        phy.profile_inputs.chcfg_physical_channel_offset.value = 11

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 16
        return phy