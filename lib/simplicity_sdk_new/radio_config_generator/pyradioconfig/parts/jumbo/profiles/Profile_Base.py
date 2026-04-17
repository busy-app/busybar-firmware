from pyradioconfig.parts.common.profiles.Profile_Base import *



class Profile_Base_Jumbo(Profile_Base):

    """
    Init internal variables
    """
    def __init__(self):
        super(self.__class__, self).__init__()
        self._description = "Profile used for most phy's on EFR32xG12 parts"
        self._family = "jumbo"

    def buildProfileModel(self, model):
        profile = super().buildProfileModel(model)
        IProfile.make_linked_io(profile, model.vars.fec_tx_enable, 'Channel_Coding', readable_name="Enable FEC")
        profile.inputs.get_input("shaping_filter_param").value_limit_max = 2.0
