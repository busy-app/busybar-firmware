from pyradioconfig.parts.bobcat.calculators.calc_rail import CalcRailBobcat


class CalcRailRainier(CalcRailBobcat):

    def calc_ircal_index(self, model):
        # RAC_PGACTRL_PGABWMODE not defined in Series 3
        pass
