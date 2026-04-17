from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFiltersRainier(IPhyFilter):
    # Studio black list (files removed before Studio distribution)
    customer_phy_groups = []

    # Studio white list (these PHYs show in Studio as proprietary starting points)
    simplicity_studio_phy_groups = [
        'Phys_Studio_BLE',
        'Phys_Studio_Connect',
        'Phys_Studio_IEEE802154',
        'Phys_Studio_Base',
    ]

    # Special designation for simulation PHYs
    sim_tests_phy_groups = []

    # Special designation for non-functional PHYs
    non_functional_phy_groups = []
