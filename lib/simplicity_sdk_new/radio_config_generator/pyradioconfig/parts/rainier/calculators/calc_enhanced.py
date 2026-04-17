from pyradioconfig.calculator_model_framework.interfaces.icalculator import ICalculator


class CalcEnhancedRainier(ICalculator):

    def buildVariables(self, modem_model):
        #Required method
        pass

    def calc_ehdsssen_reg(self, model):
        demod_select = model.vars.demod_select.value

        if demod_select == model.vars.demod_select.var_enum.ENHANCED_DSSS:
            ehdsssen = 1
        else:
            ehdsssen = 0

        self._reg_write(model.vars.MODEM_EHDSSSCTRL_EHDSSSEN, ehdsssen)

    def calc_enhanced_dsssdsaqualen_reg(self, model):
        duty_cycled = model.vars.rxdc_power_save_mode.value != model.vars.rxdc_power_save_mode.var_enum.DISABLED
        enhanced_en = (model.vars.MODEM_EHDSSSCTRL_EHDSSSEN.value == 1)

        if enhanced_en:
            do_not_care = False
            default = False

            if duty_cycled:
                # For duty cycle, we have to disable DSSSDSAQUALEN to increase off time
                # because if we're detecting the last 0 of the preamble, then we can't use the next symbol to qualify
                # since the next symbol is not a 0 (start of SFD).
                dsssdsaqualen = 0
            else:
                # If not duty-cycled, DSSSDSAQALEN improves screening of false detects
                dsssdsaqualen = 1
        else:
            dsssdsaqualen = 0
            do_not_care = True
            default = True

        self._reg_write(model.vars.MODEM_EHDSSSCFG2_DSSSDSAQUALEN,
                        value=dsssdsaqualen,
                        default=default,
                        do_not_care=do_not_care)

    def calc_enhanced_misc(self, model):
        enhanced_en = (model.vars.MODEM_EHDSSSCTRL_EHDSSSEN.value == 1)

        if enhanced_en:
            do_not_care = False
        else:
            do_not_care = True

        #Set the following to POR values for now when Enhanced demod is selected, otherwise don't care
        self._reg_write(model.vars.MODEM_SICTRL0_SIMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL0_NOISETHRESH, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL0_PEAKNUMTHRESHLW, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL0_PEAKNUMADJ, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL0_NOISETHRESHADJ, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL0_FREQNOMINAL, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL0_SYMIDENTDIS, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_SUPERCHIPTOLERANCE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_SMALLSAMPLETHRESH, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_PEAKNUMP2ADJ, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_FASTMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_TWOSYMBEN, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_ZCEN, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_ZCSAMPLETHRESH, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_SOFTCLIPBYPASS, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_SOFTCLIPTHRESH, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SIRSTAGCMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SIRSTPRSMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SIRSTCCAMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_DISSIFRAMEDET, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_AGCRSTUPONSI, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SHFTWIN, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SUPERCHIPNUM, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_CORRNUM, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_NARROWPULSETHRESH, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_PEAKNUMADJEN, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICORR_CORRTHRESH, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICORR_CORRTHRESHLOW, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICORR_CORRTHRESHUP, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICORR_CORRTHRESH2SYMB, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL1_FREQOFFTOLERANCE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SISTARTDELAY, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_SICTRL2_SISTARTDELAYMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCTRL_DSSSDSATHD, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCTRL_DUALDSA, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCFG2_MAXSCHMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCFG3_DSSSDASMAXTHD, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCFG3_DSSSFOETRACKGEAR, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCFG3_OPMODE, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCFG3_DSSSINITIMLEN, default=True, do_not_care=do_not_care)
        self._reg_write(model.vars.MODEM_EHDSSSCFG3_LQIAVGWIN, default=True, do_not_care=do_not_care)

        self._write_sifastdetect_regs(model, do_not_care)

    def _write_sifastdetect_regs(self, model, do_not_care):

        if (not (model.part_family.upper() == 'RAINIER' and model.part_revision in ['A0', 'A1', 'A2'])):
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_FASTCWDETECTEN, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_FASTSUPERCHIPEN, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_FASTNARROWPULSEEN, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_EYEOPENINGMODE, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_CWDETECTSTARTWINDOW, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_CWDETECTTHRESHOLD, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_SUPERCHIPSTARTWINDOW, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_NPULSEFIXEDTHRESHOLD, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_NPULSETHRESHADJEN, default=True, do_not_care=do_not_care)
            self._reg_write(model.vars.MODEM_SIFASTDETECTCTRL_NPULSETHRESHADJ, default=True, do_not_care=do_not_care)

    def calc_enhanced_dsssfrtcorrthd_reg(self, model):

        enhanced_en = (model.vars.MODEM_EHDSSSCTRL_EHDSSSEN.value == 1)
        dssscfecombomode = model.vars.MODEM_DIGMIXCTRL_DSSSCFECOMBO.value
        hop_enable = model.vars.hop_enable.value == model.vars.hop_enable.var_enum.ENABLED

        # DSSSFRTCORRTHD is fast first symbol detection threshold
        if enhanced_en:
            do_not_care = False
            if hop_enable and dssscfecombomode != 2:
                dsssfrtcorrthd = 875        # Improves sensitivity and rejects PER floor from adjacent channel side lobes
            else:
                dsssfrtcorrthd = 600      # Improves ZB sensitivity by 1dB
        else:
            dsssfrtcorrthd = 700
            do_not_care = True

        self._reg_write(model.vars.MODEM_EHDSSSCFG2_DSSSFRTCORRTHD, dsssfrtcorrthd, do_not_care=do_not_care)

    def calc_enhanced_dssspatt_reg(self, model):
        # Use DSSSPATT=0 for all cases for best frequency offset tolerance
        enhanced_en = (model.vars.MODEM_EHDSSSCTRL_EHDSSSEN.value == 1)

        dssspatt = 0
        if enhanced_en:
            do_not_care = False
        else:
            do_not_care = True

        self._reg_write(model.vars.MODEM_EHDSSSCFG0_DSSSPATT, dssspatt, do_not_care=do_not_care)
