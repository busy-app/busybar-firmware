from pyradioconfig.parts.bobcat.calculators.calc_misc import Calc_Misc_Bobcat
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum


class CalcMiscRainier(Calc_Misc_Bobcat):

    def calc_misc_synth_lockbwgear01(self, model):
        self._reg_write(model.vars.SYNTH_DLFCTRL_LOCKLPFBWGEAR0, 15)
        self._reg_write(model.vars.SYNTH_DLFCTRL_LOCKLPFBWGEAR1, 15)

    def calc_misc_Rainier(self, model):
        """
        These aren't really calculating right now.  Just using defaults or forced values.
       fefilt_selected = model.vars.fefilt_selected.value
        Args:
             model (ModelRoot) : Data model to read and write variables from
        """

        self._reg_write(model.vars.MODEM_EHDSSSCTRL_DSSSTIMEACQUTHD, 16)
        self._reg_write(model.vars.MODEM_EHDSSSCTRL_FOEBIAS, 1)
        self._reg_write(model.vars.MODEM_EHDSSSCTRL_FREQCORREN, 1)
        self._reg_write(model.vars.MODEM_EHDSSSCTRL_DSSSFRQLIM, 16)
        self._reg_write(model.vars.MODEM_EHDSSSCFG1_DSSSEXPSYNCLEN, 128)
        self._reg_write(model.vars.MODEM_EHDSSSCFG1_DSSSCORRTHD, 400)
        self._reg_write(model.vars.MODEM_EHDSSSCFG1_DSSSDSAQTHD, 700)
        self._reg_write(model.vars.MODEM_EHDSSSCFG2_DSSSTIMCORRTHD, 600)
        self._reg_write(model.vars.MODEM_EHDSSSCFG2_DSSSTRACKINGWIN, 5)
        self._reg_write(model.vars.MODEM_EHDSSSCFG2_DSSSCORRSCHWIN, 8)
        self._reg_write(model.vars.MODEM_EHDSSSCFG2_ONESYMBOLMBDD, 1)

        self._reg_write(model.vars.MODEM_CHFCTRL_ADJGAINWIN, 85)

        self._reg_write(model.vars.MODEM_TRECSCFG_SDSCALE, 3)
        self._reg_write(model.vars.MODEM_LRFRC_LRDSACORRTHD, 1000)

        # : TODO FIX ME! Following register values are not being calculated.
        self._reg_write_default(model.vars.SYNTH_DSMCTRLTX_DITHERDACTX)
        self._reg_write_default(model.vars.SYNTH_DSMCTRLRX_DITHERDACRX)
        self._reg_write_default(model.vars.SYNTH_DSMCTRLRX_REQORDERRX)
        self._reg_write_default(model.vars.SYNTH_DSMCTRLTX_REQORDERTX)
        self._reg_write_default(model.vars.SYNTH_QNCCTRL_QNCOFFSET)
        self._reg_write_default(model.vars.SYNTH_LMSOVERRIDE_GLMSOVERRIDEEN)
        self._reg_write_default(model.vars.SYNTH_LMSOVERRIDE_PLMSOVERRIDEEN)
        self._reg_write_default(model.vars.MODEM_EXPECTPATTDUAL_EXPECTPATTDUAL)
        self._reg_write_default(model.vars.MODEM_DUALTIM_DUALTIMEN)
        self._reg_write_default(model.vars.MODEM_DUALTIM_MINCOSTTHD2)
        self._reg_write_default(model.vars.MODEM_DUALTIM_SYNCACQWIN2)
        self._reg_write_default(model.vars.MODEM_PHDMODCTRL_CHPWRQUAL)
        self._reg_write_default(model.vars.MODEM_CTRL3_ANTDIVMODE)
        self._reg_write_default(model.vars.MODEM_DIGMIXCTRL_BLEORZB)
        self._reg_write_default(model.vars.MODEM_DIGMIXCTRL_MULTIPHYHOP)
        self._reg_write_default(model.vars.MODEM_DIGMIXCTRL_RXBRINTSHIFT)
        self._reg_write_default(model.vars.MODEM_DIGMIXCTRL_DSSSCFECOMBO)
        self._reg_write_default(model.vars.MODEM_SYNC3_SYNC3)
        self._reg_write_default(model.vars.MODEM_EHDSSSCTRL_DSSSPMTIMEOUT)
        self._reg_write_default(model.vars.MODEM_EHDSSSCTRL_DSSSFRMTIMEOUT)
        self._reg_write_default(model.vars.MODEM_COCURRMODE_DSSSDSACHK)
        self._reg_write_default(model.vars.MODEM_COCURRMODE_TRECSDSACHK)
        self._reg_write_default(model.vars.MODEM_COCURRMODE_CORRCHKMUTE)
        self._reg_write_default(model.vars.MODEM_SYNCWORDCTRL_SYNCBITS2TH)
        self._reg_write_default(model.vars.MODEM_SYNCWORDCTRL_SYNC3ERRORS)
        self._reg_write_default(model.vars.MODEM_SYNCWORDCTRL_DUALSYNC2TH)
        self._reg_write_default(model.vars.MODEM_SYNCWORDCTRL_SYNCSWFEC)

        # Needed for sim only??
        self._reg_write_default(model.vars.SYNTH_DSMCTRLTX_QNCMODETX)

    # FIXME: amtudave: Uncomment to remove SYNCERRORS from viper once tri sync word logic is working
    # def calc_syncerrors_reg(self, model):
    #     # MODEM_CTRL1_SYNCERRORS removed in viper, see MODEM_SYNCWORDCTRL_SYNC0ERRORS
    #     pass

    def calc_sync2_reg(self, model):
        self._reg_write(model.vars.MODEM_SYNC2_SYNC2, 0)

    def calc_misc(self, model):
        """
        copied from Bobcat and removing deprecated registers
        """
        # FIXME: calculate these

        self._reg_write(model.vars.MODEM_CTRL2_BRDIVA, 0)
        self._reg_write(model.vars.MODEM_CTRL2_BRDIVB, 0)
        self._reg_write(model.vars.MODEM_CTRL2_DEVMULA, 0)
        self._reg_write(model.vars.MODEM_CTRL2_DEVMULB, 0)
        self._reg_write(model.vars.MODEM_CTRL2_RATESELMODE, 0)
        self._reg_write(model.vars.MODEM_CTRL2_SQITHRESH, 0)
        self._reg_write(model.vars.MODEM_CTRL2_TXPINMODE, 0)
        self._reg_write(model.vars.MODEM_CTRL4_ADCSATDENS, 0)
        self._reg_write(model.vars.MODEM_CTRL4_ADCSATLEVEL, 6)
        self._reg_write(model.vars.MODEM_CTRL4_OFFSETPHASESCALING, 0)
        self._reg_write(model.vars.MODEM_CTRL4_PHASECLICKFILT, 0)
        self._reg_write(model.vars.MODEM_CTRL4_PREDISTAVG, 0)
        self._reg_write(model.vars.MODEM_CTRL4_PREDISTDEB, 0)
        self._reg_write(model.vars.MODEM_CTRL4_PREDISTGAIN, 0)
        self._reg_write(model.vars.MODEM_CTRL4_PREDISTRST, 0)
        self._reg_write(model.vars.MODEM_CTRL4_SOFTDSSSMODE, 0)
        self._reg_write(model.vars.MODEM_CTRL5_DEMODRAWDATASEL2, 0)
        self._reg_write(model.vars.MODEM_CTRL5_DETDEL, 0)
        self._reg_write(model.vars.MODEM_CTRL5_POEPER, 0)
        self._reg_write(model.vars.MODEM_CTRL5_RESYNCLIMIT, 0)
        self._reg_write(model.vars.MODEM_CTRL6_CODINGB, 0)
        self._reg_write(model.vars.MODEM_CTRL6_CPLXCORREN, 0)
        self._reg_write(model.vars.MODEM_CTRL6_DEMODRESTARTALL, 0)
        self._reg_write(model.vars.MODEM_CTRL6_DSSS3SYMBOLSYNCEN, 0)
        self._reg_write(model.vars.MODEM_CTRL6_PREBASES, 0)
        self._reg_write(model.vars.MODEM_CTRL6_RXRESTARTUPONRSSI, 0)
        self._reg_write(model.vars.MODEM_CTRL6_RXRESTARTUPONSHORTRSSI, 0)
        self._reg_write(model.vars.MODEM_CTRL6_TXDBPSKINV, 0)
        self._reg_write(model.vars.MODEM_CTRL6_TXDBPSKRAMPEN, 0)
        # self._reg_write(model.vars.MODEM_ANARAMPCTRL_VMIDCTRL, 1)
        # self._reg_write(model.vars.MODEM_ANARAMPCTRL_MUTEDLY, 0)
        self._reg_write(model.vars.MODEM_ETSTIM_ETSCOUNTEREN, 0)
        self._reg_write(model.vars.MODEM_ETSTIM_ETSTIMVAL, 0)

        self._reg_write(model.vars.MODEM_PRE_DSSSPRE, 0)
        self._reg_write(model.vars.MODEM_PRE_PRESYMB4FSK, 0)
        self._reg_write(model.vars.MODEM_PRE_SYNCSYMB4FSK, 0)
        self._reg_write(model.vars.MODEM_TIMING_FASTRESYNC, 0)
        self._reg_write(model.vars.MODEM_TIMING_TIMSEQINVEN, 0)
        self._reg_write(model.vars.MODEM_TIMING_TIMSEQSYNC, 0)
        self._reg_write(model.vars.MODEM_TIMING_TSAGCDEL, 0)

        # Long Range registers
        # FIXME: calculate these
        self._reg_write(model.vars.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE, 0)
        self._reg_write(model.vars.MODEM_LONGRANGE1_LOGICBASEDPUGATE, 0)
        self._reg_write(model.vars.MODEM_LONGRANGE1_LRSPIKETHADD, 0)
        self._reg_write(model.vars.MODEM_LONGRANGE1_LRSS, 0)
        self._reg_write(model.vars.MODEM_LRFRC_CI500, 1)
        self._reg_write(model.vars.MODEM_LRFRC_FRCACKTIMETHD, 0)
        self._reg_write(model.vars.MODEM_LRFRC_LRCORRMODE, 1)

        # FIXME: figure out how these AGC registers need to be calculated
        self._reg_write(model.vars.AGC_RSSISTEPTHR_DEMODRESTARTPER, 0)
        self._reg_write(model.vars.AGC_RSSISTEPTHR_DEMODRESTARTTHR, 0)
        self._reg_write(model.vars.AGC_RSSISTEPTHR_NEGSTEPTHR, 0)
        self._reg_write(model.vars.AGC_RSSISTEPTHR_POSSTEPTHR, 0)
        self._reg_write(model.vars.AGC_RSSISTEPTHR_STEPPER, 0)

        # Coherent Demod Registers
        #FIXME: Check with Yan/Per on how to calculate these
        self._reg_write(model.vars.MODEM_COH2_DSAPEAKCHPWRTH, 0)
        self._reg_write(model.vars.MODEM_COH3_COHDSADETDIS, 0)
        self._reg_write(model.vars.MODEM_COH3_DSAPEAKCHPWREN, 0)
        self._reg_write(model.vars.MODEM_COH3_LOGICBASEDCOHDEMODGATE, 0)
        self._reg_write(model.vars.MODEM_COH3_ONEPEAKQUALEN, 0)
        self._reg_write(model.vars.MODEM_COH3_PEAKCHKTIMOUT, 0)

        # Clock-gating register
        self._reg_write(model.vars.MODEM_AUTOCG_AUTOCGEN, 0) #We calculate MODEM_CGCLKSTOP_FORCEOFF in calculator instead
        self._reg_write(model.vars.FRC_AUTOCG_AUTOCGEN, 7)

        # Modem Registers with fixed value
        self._reg_write(model.vars.MODEM_CTRL0_DEMODRAWDATASEL, 0)
        self._reg_write(model.vars.MODEM_CTRL2_DMASEL, 0)
        self._reg_write(model.vars.MODEM_CTRL3_PRSDINEN, 0)
        self._reg_write(model.vars.MODEM_CTRL4_CLKUNDIVREQ, 0)
        self._reg_write(model.vars.MODEM_CTRL3_RAMTESTEN, 0)
        self._reg_write(model.vars.MODEM_DIRECTMODE_CLKWIDTH, 1)
        self._reg_write(model.vars.MODEM_DIRECTMODE_DMENABLE, 0)
        self._reg_write(model.vars.MODEM_DIRECTMODE_SYNCASYNC, 0)
        self._reg_write(model.vars.MODEM_DIRECTMODE_SYNCPREAM, 3)
        # self._reg_write(model.vars.MODEM_PADEBUG_ENMANPACLKAMPCTRL, 0)
        # self._reg_write(model.vars.MODEM_PADEBUG_ENMANPAPOWER, 0)
        # self._reg_write(model.vars.MODEM_PADEBUG_ENMANPASELSLICE, 0)
        # self._reg_write(model.vars.MODEM_PADEBUG_MANPACLKAMPCTRL, 0)
        self._reg_write(model.vars.MODEM_CTRL0_OOKASYNCPIN, 0)
        self._reg_write(model.vars.MODEM_CTRL0_DUALCORROPTDIS, 0)
        self._reg_write(model.vars.MODEM_CTRL0_FRAMEDETDEL, 0)
        self._reg_write(model.vars.MODEM_CTRL1_SYNC1INV, 0)

        # FRC Registers with fixed value
        self._reg_write(model.vars.FRC_BOICTRL_BOIBITPOS, 0)
        self._reg_write(model.vars.FRC_BOICTRL_BOIEN, 0)
        self._reg_write(model.vars.FRC_BOICTRL_BOIFIELDLOC, 0)
        self._reg_write(model.vars.FRC_BOICTRL_BOIMATCHVAL, 0)
        self._reg_write(model.vars.FRC_CTRL_RATESELECT, 0)
        self._reg_write(model.vars.FRC_CTRL_WAITEOFEN, 0)
        self._reg_write(model.vars.FRC_DFLCTRL_DFLBOIOFFSET, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_DSLBITORDER, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_DSLBITS, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_DSLMINLENGTH, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_DSLMODE, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_DSLOFFSET, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_DSLSHIFT, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_RXSUPRECEPMODE, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_STORESUP, 0)
        self._reg_write(model.vars.FRC_DSLCTRL_SUPSHFFACTOR, 0)
        self._reg_write(model.vars.FRC_TRAILTXDATACTRL_TXSUPPLENOVERIDE, 0)
        self._reg_write(model.vars.FRC_WCNTCMP3_SUPPLENFIELDLOC, 0)
        self._reg_write(model.vars.FRC_WCNTCMP4_SUPPLENGTH, 0)

        # Added new reg-fields related to 15.4 subG OQPSK phys
        self._reg_write(model.vars.MODEM_COH3_COHDSACMPLX, 0)
        self._reg_write(model.vars.MODEM_SYNCPROPERTIES_STATICSYNCTHRESH, 0)

        # Added new reg-fields related to Internal Long Range
        self._reg_write(model.vars.MODEM_PRE_PREWNDERRORS, 0)
        self._reg_write(model.vars.MODEM_CTRL3_TIMINGBASESGAIN, 0)

        #AGC default settings
        self._reg_write(model.vars.AGC_CTRL0_CFLOOPNFADJ, 0)
        self._reg_write(model.vars.AGC_DUALRFPKDTHD1_RFPKDHITHD0, 1)
        self._reg_write(model.vars.AGC_DUALRFPKDTHD1_RFPKDHITHD1, 40)
        self._reg_write(model.vars.AGC_DUALRFPKDTHD0_RFPKDLOWTHD0, 1)
        self._reg_write(model.vars.AGC_DUALRFPKDTHD0_RFPKDLOWTHD1, 10)

        # reg-fields to modify sync detection reset behavior PGOCELOT-5282
        self._reg_write(model.vars.MODEM_FRMSCHTIME_PMRSTSYCNEN, 0)
        self._reg_write(model.vars.MODEM_FRMSCHTIME_DSARSTSYCNEN, 0)

    def calc_adc_sidetone_amp_reg(self, model):
        pass

    def buildVariables(self, model):
        """Populates a list of needed variables for this calculator

        Args:
            model (ModelRoot) : Builds the variables specific to this calculator
        """
        # : Build variables using base
        super().buildVariables(model)
        model.vars.protocol_id.var_enum = CreateModelVariableEnum(
            enum_name='ProtocolIDEnum',
            enum_desc='List of supported protocols',
            member_data=[
                ['Custom', 0, 'Custom stack'],
                # ['Reserved', 1, 'Reserved'],
                ['Thread', 2, 'Thread on RAIL'],
                ['BLE', 3, 'BLE on RAIL'],
                ['Connect', 4, 'Connect on RAIL'],
                ['Zigbee', 5, 'Zigbee on RAIL'],
                ['ZWave', 6, 'ZWave on RAIL'],
                ['WiSUN', 7, 'WiSUN on RAIL'],
                ['Custom_802154', 8, 'Custom 802.15.4 on RAIL'],
                ['Sidewalk', 9, 'Sidewalk on RAIL'],
                ['BTC', 10, 'Bluetooth Classic on RAIL'],
                ['ANT', 11, 'ANT on RAIL'],
                ['Longrange', 12, 'Longrange'],
                ['Mbus', 13, 'Mbus'],
                ['Sigfox', 14, 'Sigfox'],
                ['HDT', 15, 'BLE High Data Throughput'],
            ])
