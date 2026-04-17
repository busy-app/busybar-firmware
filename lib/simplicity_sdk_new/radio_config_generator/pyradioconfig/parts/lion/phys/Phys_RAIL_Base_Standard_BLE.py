from pyradioconfig.parts.leopard.phys.Phys_RAIL_Base_Standard_Bluetooth_LE import PhysRailBaseStandardBluetoothLeLeopard


class PhysRailBaseStandardBluetoothLeLion(PhysRailBaseStandardBluetoothLeLeopard):

    def PHY_Bluetooth_LongRange_nodsa_125kbps(self, model, phy_name=None):
        phy = super().PHY_Bluetooth_LongRange_dsa_125kbps(model, phy_name=phy_name)
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        return phy

    def PHY_Bluetooth_LongRange_nodsa_500kbps(self, model, phy_name=None):
        phy = super().PHY_Bluetooth_LongRange_dsa_500kbps(model, phy_name=phy_name)
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        return phy