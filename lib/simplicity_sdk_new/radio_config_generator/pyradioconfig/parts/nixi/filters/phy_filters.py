"""
Nixi specific filters
"""
from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFilters(IPhyFilter):
    customer_phy_groups = []

    sim_tests_phy_groups = ['Phys_sim_tests']

    simplicity_studio_phy_groups = [
        'default_phys',
        'Phys_Studio_LongRange',
        'Phys_connect',
        'Phys_MBus_Studio',
        'Phys_Datasheet',
    ]
