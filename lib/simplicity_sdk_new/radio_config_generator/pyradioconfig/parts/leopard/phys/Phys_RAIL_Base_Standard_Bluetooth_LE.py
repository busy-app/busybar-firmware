from pyradioconfig.parts.lynx.phys.Phys_RAIL_Base_Standard_Bluetooth_LE import PhysRAILBaseStandardBluetoothLELynx


class PhysRailBaseStandardBluetoothLeLeopard(PhysRAILBaseStandardBluetoothLELynx):
    # PHYs are inherited from Lynx unless overridden

    def PHY_Bluetooth_LongRange_dsa_125kbps(self, model, phy_name=None):

        phy = super().PHY_Bluetooth_LongRange_dsa_125kbps(model, phy_name=phy_name)
        phy.profile_outputs.tx_sync_delay_ns.override = 600
        phy.profile_outputs.tx_eof_delay_ns.override = 600
        return phy