from pyradioconfig.parts.bobcat.calculators.calc_dsa import Calc_DSA_Bobcat


class CalcDsaRainier(Calc_DSA_Bobcat):

    # Rainier has no Phase based DSA
    def calc_dsamode_reg(self, model):
        pass

    def calc_arrthd_reg(self, model):
        pass

    def calc_phdsa_defaults(self, model):
        pass

    def calc_dsactrl_lowduty(self, model):
        pass