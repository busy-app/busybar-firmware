from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy
from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_154
from py_2_and_3_compatibility import *


class PhysStudioIEEE802154BPSKOcelot(IPhy):

    def PHY_IEEE802154_868MHz_BPSK_20kbps_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154BPSK,
                            readable_name='Production IEEE802154 BPSK 868MHz 20kbps PHY',
                            phy_name=phy_name)
        phy.profile_inputs.base_frequency_hz.value = 868_300_000
        phy.profile_inputs.bitrate.value = 20_000
        phy.profile_inputs.bpsk_feature.value = model.vars.bpsk_feature.var_enum.STANDARD_20KBPS
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        phy.profile_inputs.chcfg_base_frequency_hz.value = 868_300_000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2_000_000

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 0
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1]

        return phy

    def PHY_IEEE802154_915MHz_BPSK_40kbps_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.IEEE802154BPSK,
                            readable_name='Production IEEE802154 BPSK 915MHz 40kbps PHY',
                            phy_name=phy_name)
        phy.profile_inputs.base_frequency_hz.value = 915_000_000
        phy.profile_inputs.bitrate.value = 40_000
        phy.profile_inputs.bpsk_feature.value = model.vars.bpsk_feature.var_enum.STANDARD_40KBPS
        phy.profile_inputs.xtal_frequency_hz.value = 39000000

        phy.profile_inputs.chcfg_base_frequency_hz.value = 906_000_000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2_000_000

        phy.profile_inputs.chcfg_channel_number_start.value = 1
        phy.profile_inputs.chcfg_channel_number_end.value = 10
        phy.profile_inputs.chcfg_physical_channel_offset.value = 1

        phy.profile_inputs.rail_tx_power_max.value = [-1]

        return phy
