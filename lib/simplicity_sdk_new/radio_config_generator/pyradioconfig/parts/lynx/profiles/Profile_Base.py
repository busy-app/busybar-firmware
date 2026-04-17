from pyradioconfig.parts.common.profiles.Profile_Base import *



class Profile_Base_Lynx(Profile_Base):

    """
    Init internal variables
    """
    def __init__(self):
        super(self.__class__, self).__init__()
        self._description = "Profile used for most phy's on EFR32xG22 parts"
        self._family = "lynx"

    def buildProfileModel(self, model):
        profile = super().buildProfileModel(model)

        IProfile.make_hidden_input(profile, model.vars.demod_select, 'Advanced', readable_name="Demod Selection")
        IProfile.make_hidden_input(profile, model.vars.agc_lock_mode, 'Advanced', readable_name="AGC lock mode")
        IProfile.make_hidden_input(profile, model.vars.agc_enable_adc_attenuation, 'Advanced', readable_name="Enable ADC attenuation")