from pyradioconfig.parts.bobcat.phys.Phys_Studio_Connect import PHYS_connect_Bobcat
from py_2_and_3_compatibility import *

class PhysStudioConnectRainier(PHYS_connect_Bobcat):

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 38400000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq
