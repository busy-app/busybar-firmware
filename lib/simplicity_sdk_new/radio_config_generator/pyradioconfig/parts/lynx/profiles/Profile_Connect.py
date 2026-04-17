from pyradioconfig.parts.common.profiles.Profile_Connect import *


class Profile_Connect_Lynx(Profile_Connect):

    def __init__(self):
        super(self.__class__, self).__init__()
        self._family = "lynx"

    def buildProfileModel(self, model):
        profile = super().buildProfileModel(model)

        IProfile.make_hidden_input(profile, model.vars.agc_lock_mode, 'Advanced', readable_name="AGC lock mode")
        IProfile.make_hidden_input(profile, model.vars.agc_enable_adc_attenuation, 'Advanced', readable_name="Enable ADC attenuation")

