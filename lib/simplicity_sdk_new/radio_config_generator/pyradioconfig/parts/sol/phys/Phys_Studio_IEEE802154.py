from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from py_2_and_3_compatibility import *


class PhysStudioIEEE802154Sol(IPhy):

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH0_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 0 PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.GB868_863
        phy.profile_inputs.fpll_band.value = model.vars.fpll_band.var_enum.BAND_9xx

        phy.profile_inputs.chcfg_base_frequency_hz.value = 863250000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 200000

        phy.profile_inputs.chcfg_channel_number_start.value = 128
        phy.profile_inputs.chcfg_channel_number_end.value = 154
        phy.profile_inputs.chcfg_physical_channel_offset.value = 128

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 27

        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH1_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 1 PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.GB868_863
        phy.profile_inputs.fpll_band.value = model.vars.fpll_band.var_enum.BAND_9xx

        phy.profile_inputs.chcfg_base_frequency_hz.value = 863250000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 200000

        phy.profile_inputs.chcfg_channel_number_start.value = 160
        phy.profile_inputs.chcfg_channel_number_end.value = 167
        phy.profile_inputs.chcfg_physical_channel_offset.value = 133

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 8

        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH2_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 2 PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.GB868_863
        phy.profile_inputs.fpll_band.value = model.vars.fpll_band.var_enum.BAND_9xx

        phy.profile_inputs.chcfg_base_frequency_hz.value = 863250000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 200000

        phy.profile_inputs.chcfg_channel_number_start.value = 192
        phy.profile_inputs.chcfg_channel_number_end.value = 218
        phy.profile_inputs.chcfg_physical_channel_offset.value = 157

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 27

        return phy

    def PHY_IEEE802154_GB868_863MHz_PHR2_CH3_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 863 MHz Channel Config 3 PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.GB868_863
        phy.profile_inputs.fpll_band.value = model.vars.fpll_band.var_enum.BAND_9xx

        phy.profile_inputs.chcfg_base_frequency_hz.value = 863250000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 200000

        phy.profile_inputs.chcfg_channel_number_start.value = 168
        phy.profile_inputs.chcfg_channel_number_end.value = 168
        phy.profile_inputs.chcfg_physical_channel_offset.value = 106

        phy.profile_inputs.rail_tx_power_max.value = [-1]

        return phy

    def PHY_IEEE802154_GB868_915MHz_PHR2_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 UK Metering 915 MHz PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.GB868_915
        phy.profile_inputs.fpll_band.value = model.vars.fpll_band.var_enum.BAND_9xx

        phy.profile_inputs.chcfg_base_frequency_hz.value = 915350000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 200000

        phy.profile_inputs.chcfg_channel_number_start.value = 224
        phy.profile_inputs.chcfg_channel_number_end.value = 250
        phy.profile_inputs.chcfg_physical_channel_offset.value = 224

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 27

        return phy

    def PHY_IEEE802154_915MHz_2GFSK_R23_NA_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154, readable_name='Production IEEE 802.15.4 2GFSK R23 NA 915 MHz PHY', phy_name=phy_name)

        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.zigbee_feature.value = model.vars.zigbee_feature.var_enum.NA915_R23
        phy.profile_inputs.fpll_band.value = model.vars.fpll_band.var_enum.BAND_9xx

        phy.profile_inputs.chcfg_base_frequency_hz.value = 915350000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 200000

        phy.profile_inputs.chcfg_channel_number_start.value = 32
        phy.profile_inputs.chcfg_channel_number_end.value = 56
        phy.profile_inputs.chcfg_physical_channel_offset.value = 32

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 25

        return phy