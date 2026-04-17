from pyradioconfig.calculator_model_framework.interfaces.iphy import IPhy


class PhysStudioBLEPanther(IPhy):

    def PHY_Bluetooth_1M_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE 1Mbps PHY for Panther', phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.LE_1M

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2402000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2000000

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40

        return phy

    def PHY_Bluetooth_2M_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE 2Mbps PHY for Panther', phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.LE_2M

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2402000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2000000

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40
        phy.profile_inputs.rail_tx_power_max.value[35] = 182
        phy.profile_inputs.rail_tx_power_max.value[36] = 157
        phy.profile_inputs.rail_tx_power_max.value[37] = 125
        phy.profile_inputs.rail_tx_power_max.value[38] = 83

        return phy

    def PHY_Bluetooth_LR_125k_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE LongRange 125kbps PHY for Panther', phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.CODED_125K

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2402000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2000000

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40

        return phy

    def PHY_Bluetooth_LR_500k_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE LongRange 500kbps PHY for Panther', phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.CODED_500K

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2402000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2000000

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40

        return phy

    def PHY_Bluetooth_1M_AOX_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE 1Mbps AOX PHY for Panther',
                            phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.AOX_1M

        phy.profile_inputs.chcfg_base_frequency_hz.value = 2402000000
        phy.profile_inputs.chcfg_channel_spacing_hz.value = 2000000

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40

        return phy
