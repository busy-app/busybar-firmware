from pyradioconfig.parts.margay.calculators.calc_radio import CALC_Radio_Margay


class calc_radio_serval(CALC_Radio_Margay):

    def calc_txtrimdregbleed_reg(self, model):
        modulation_type = model.vars.modulation_type.value

        if modulation_type == model.vars.modulation_type.var_enum.ASK:
                bleed = 2

                self._reg_write(model.vars.RAC_PATRIM3_TXTRIMDREGBLEED, bleed)
        else:
            super().calc_txtrimdregbleed_reg(model)
