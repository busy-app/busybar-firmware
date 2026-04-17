"""Serval specific filters"""

from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFilters(IPhyFilter):
    # Studio black list (files removed before Studio distribution)
    customer_phy_groups = []

    # Studio white list (these PHYs show in Studio as proprietary starting points)
    simplicity_studio_phy_groups = [
        'Phys_Studio_Base',
        'Phys_Studio_Connect',
        'Phys_Studio_LongRange',
        'Phys_Studio_Base_Standard_SUNFSK',
        'Phys_Studio_MBus',
        'Phys_Studio_Sidewalk'
    ]

    # Special designation for simulation PHYs
    sim_tests_phy_groups = []

    # Special designation for non-functional PHYs
    non_functional_phy_groups = []

    # PHYs to exclude from regression
    virtual_phy_groups = ['phys_studio_wisun_fan_1_1_virtual']
