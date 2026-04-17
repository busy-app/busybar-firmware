from pyradioconfig.parts.bobcat.phys.Phys_Studio_Base import PHYS_Studio_Base_Bobcat
from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_INTERNAL


class PhysStudioBaseRainier(PHYS_Studio_Base_Bobcat):

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 38400000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq
