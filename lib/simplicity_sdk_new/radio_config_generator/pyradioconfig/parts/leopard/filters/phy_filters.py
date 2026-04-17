from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFilters(IPhyFilter):
    customer_phy_groups = []

    sim_tests_phy_groups = ['Phys_sim_tests']

    simplicity_studio_phy_groups = [
        'Phys_Studio_BLE',
        'Phys_Studio_Connect',
        'Phys_Studio_IEEE802154',
        'Phys_Datasheet',
    ]

    # PHYs to exclude from regression
    virtual_phy_groups = ['Phys_Internal_Base_ValOnly_aliases']
