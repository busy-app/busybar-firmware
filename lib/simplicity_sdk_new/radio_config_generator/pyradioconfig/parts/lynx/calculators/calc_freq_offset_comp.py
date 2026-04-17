from pyradioconfig.parts.panther.calculators.calc_freq_offset_comp import CALC_Freq_Offset_Comp_panther
import math


class CALC_Freq_Offset_Comp_lynx(CALC_Freq_Offset_Comp_panther):

    def calc_freq_comp_mode(self, model):
        """
        determine best frequency compensation mode based on emprical data

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """
        demod_select = model.vars.demod_select.value

        if demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI or demod_select == model.vars.demod_select.var_enum.TRECS_SLICER:
            # default modes. Not enabling afc one shot in Lynx until feature is validated on silicon.
            freq_mode = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
            afc_mode = model.vars.afc_run_mode.var_enum.CONTINUOUS

            model.vars.frequency_comp_mode.value = freq_mode
            model.vars.afc_run_mode.value = afc_mode
        else:
            super().calc_freq_comp_mode(model)

    def calc_afcdel_reg(self, model):
        """
        calculate AFC Delay based on over sampling rate (osr) if AFC is enabled

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """
        osr = model.vars.oversampling_rate_actual.value
        demod_select = model.vars.demod_select.value
        afconeshot = model.vars.MODEM_AFC_AFCONESHOT.value
        del_digmix_to_demod = model.vars.grpdelay_to_demod.value

        if demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI or demod_select == model.vars.demod_select.var_enum.TRECS_SLICER:
            if afconeshot:
                # AFCDEL is in symbols when used for Viterbi Demod so divide by OSR
                # if REMODDWN is not 1 that will also need to be taken into account
                afcdel = math.ceil(del_digmix_to_demod / osr)
            else:
                afcdel = 0

            if afcdel > 31:
                afcdel = 31

            self._reg_write(model.vars.MODEM_AFC_AFCDEL, int(afcdel))
        else:
            super().calc_afcdel_reg(model)