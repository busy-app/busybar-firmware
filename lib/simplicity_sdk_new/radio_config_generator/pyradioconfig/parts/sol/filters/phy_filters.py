from pyradioconfig.calculator_model_framework.interfaces.iphy_filter import IPhyFilter


class PhyFilters_Sol(IPhyFilter):
    # Studio black list (files removed before Studio distribution)
    customer_phy_groups = []

    # Studio white list (these PHYs show in Studio as proprietary starting points)
    simplicity_studio_phy_groups = [
        'Phys_Studio_IEEE802154',
        'Phys_Studio_SUN_OQPSK',
        'Phys_Studio_Base',
        'Phys_Studio_Sidewalk',
        'Phys_Studio_Connect',
        'phys_studio_wisun_han',
        'Phys_Studio_LongRange',
        'Phys_Studio_Base_Standard_SUNFSK',
        'phys_studio_wisun_fan_1_0',
        'phys_studio_wisun_fan_1_1',
        'phys_studio_wisun_fan_1_1_virtual',
        'Phys_Studio_Connect_OFDM',
    ]

    # Special designation for simulation PHYs
    sim_tests_phy_groups = []

    # Special designation for non-functional PHYs
    non_functional_phy_groups = []

    # PHYs to exclude from regression
    virtual_phy_groups = ['Phys_Internal_WiSUN_OFDM_MCSx', 'phys_studio_wisun_fan_1_1_virtual']
