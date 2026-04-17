"""
Jumbo specific filters
"""
from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFilters(IPhyFilter):
    customer_phy_groups = []

    sim_tests_phy_groups = ['Phys_sim_tests']

    simplicity_studio_phy_groups = [
        'phys_studio_wisun_fan_1_1_virtual',
        'phys_studio_wisun_han',
        'phys_studio_wisun_fan_1_1',
        'Phys_Studio_LongRange',
        'Phys_MBus_Studio',
        'Phys_connect',
        'phys_studio_wisun_fan_1_0',
        'Phys_Studio_SUNFSK',
        'default_phys',
        'Phys_Datasheet',
        'phy_internal_base',
    ]

    non_functional_phy_groups = ['Phys_ASK']

    # PHYs to exclude from regression
    virtual_phy_groups = ['phys_studio_wisun_fan_1_1_virtual']
