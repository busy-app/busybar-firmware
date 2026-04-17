"""
Nerio specific filters
"""
from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFilters(IPhyFilter):
    customer_phy_groups = []

    sim_tests_phy_groups = ['Phys_sim_tests']

    simplicity_studio_phy_groups = [
        'Phys_MBus_Studio',
        'Phys_connect',
        'default_phys',
        'Phys_Studio_Sidewalk',
        'Phys_Studio_LongRange',
        'Phys_Datasheet',
    ]
