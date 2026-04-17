from pycalcmodel.core.output import ModelOutput, ModelOutputType
from pyradioconfig.parts.common.profiles.bobcat_regs import build_modem_regs_bobcat


def build_modem_regs_rainier(model, profile):
    build_modem_regs_bobcat(model, profile)
    build_synth_regs_s3(model, profile)
    build_syctrl_regs_s3(model, profile)

    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNC2_SYNC2, '',         ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNC2.SYNC2'                       ))

    profile.outputs.append(ModelOutput(model.vars.MODEM_LOG2X4_LOG2X4FWSEL, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.LOG2X4.LOG2X4FWSEL'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_LOG2X4_LOG2X4, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.LOG2X4.LOG2X4'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_EXPECTPATTDUAL_EXPECTPATTDUAL, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EXPECTPATTDUAL.EXPECTPATTDUAL'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_COCURRMODE_DSSSCONCURRENT, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.COCURRMODE.DSSSCONCURRENT'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_EHDSSSEN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.EHDSSSEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_DSSSTIMEACQUTHD, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.DSSSTIMEACQUTHD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_FOEBIAS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.FOEBIAS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_FREQCORREN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.FREQCORREN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_DSSSFRQLIM, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.DSSSFRQLIM'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_DSSSDSATHD, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.DSSSDSATHD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_DUALDSA, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.DUALDSA'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG0_DSSSPATT, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG0.DSSSPATT'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG1_DSSSEXPSYNCLEN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG1.DSSSEXPSYNCLEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG1_DSSSCORRTHD, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG1.DSSSCORRTHD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG1_DSSSDSAQTHD, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG1.DSSSDSAQTHD'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_DSSSTIMCORRTHD, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.DSSSTIMCORRTHD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_DSSSFRTCORRTHD, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.DSSSFRTCORRTHD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_DSSSTRACKINGWIN,'', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.DSSSTRACKINGWIN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_DSSSCORRSCHWIN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.DSSSCORRSCHWIN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_ONESYMBOLMBDD,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.ONESYMBOLMBDD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_MAXSCHMODE,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.MAXSCHMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG2_DSSSDSAQUALEN,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG2.DSSSDSAQUALEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG3_DSSSDASMAXTHD,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG3.DSSSDASMAXTHD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG3_DSSSFOETRACKGEAR,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG3.DSSSFOETRACKGEAR'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG3_OPMODE,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG3.OPMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG3_DSSSINITIMLEN,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG3.DSSSINITIMLEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCFG3_LQIAVGWIN,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCFG3.LQIAVGWIN'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_TRECSCFG_SOFTD,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.TRECSCFG.SOFTD'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_TRECSCFG_SDSCALE,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.TRECSCFG.SDSCALE'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_DUALTIM_DUALTIMEN,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DUALTIM_DUALTIMEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_DUALTIM_MINCOSTTHD2,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DUALTIM_MINCOSTTHD2'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_DUALTIM_SYNCACQWIN2,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DUALTIM_SYNCACQWIN2'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_LRFRC_LRDSACORRTHD,  '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.LRFRC_LRDSACORRTHD'))



    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_SIMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.SIMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_NOISETHRESH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.NOISETHRESH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_PEAKNUMTHRESHLW, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.PEAKNUMTHRESHLW'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_PEAKNUMADJ, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.PEAKNUMADJ'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_NOISETHRESHADJ, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.NOISETHRESHADJ'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_FREQNOMINAL, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.FREQNOMINAL'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_SUPERCHIPTOLERANCE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.SUPERCHIPTOLERANCE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_SMALLSAMPLETHRESH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.SMALLSAMPLETHRESH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_PEAKNUMP2ADJ, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.PEAKNUMP2ADJ'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_FASTMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.FASTMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_TWOSYMBEN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.TWOSYMBEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_ZCEN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.ZCEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_ZCSAMPLETHRESH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.ZCSAMPLETHRESH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_SOFTCLIPBYPASS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.SOFTCLIPBYPASS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_SOFTCLIPTHRESH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.SOFTCLIPTHRESH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SIRSTAGCMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SIRSTAGCMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SIRSTPRSMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SIRSTPRSMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SIRSTCCAMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SIRSTCCAMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_DISSIFRAMEDET, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.DISSIFRAMEDET'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_AGCRSTUPONSI, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.AGCRSTUPONSI'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SHFTWIN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SHFTWIN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SUPERCHIPNUM, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SUPERCHIPNUM'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_CORRNUM, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.CORRNUM'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_NARROWPULSETHRESH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.NARROWPULSETHRESH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_PEAKNUMADJEN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.PEAKNUMADJEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICORR_CORRTHRESH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICORR.CORRTHRESH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICORR_CORRTHRESHLOW, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICORR.CORRTHRESHLOW'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICORR_CORRTHRESHUP, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICORR.CORRTHRESHUP'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICORR_CORRTHRESH2SYMB, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICORR.CORRTHRESH2SYMB'))

    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN1_CLKMULTDIVN, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN1.CLKMULTDIVN'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN1_CLKMULTDIVR, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN1.CLKMULTDIVR'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN1_CLKMULTDIVX, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN1.CLKMULTDIVX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN2_CLKMULTDIVNTX, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN2.CLKMULTDIVNTX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN2_CLKMULTDIVRTX, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN2.CLKMULTDIVRTX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN2_CLKMULTDIVXTX, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN2.CLKMULTDIVXTX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN2_CLKMULTENBYPASS40MHZTX, '', ModelOutputType.SVD_REG_FIELD,readable_name='RAC.CLKMULTEN2.CLKMULTENBYPASS40MHZTX'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_DIGMIXCTRL_FWHOPPING, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DIGMIXCTRL.FWHOPPING'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_PHDMODCTRL_FASTHOPPINGEN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.PHDMODCTRL.FASTHOPPINGEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_PHDMODCTRL_CHPWRQUAL, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.PHDMODCTRL.CHPWRQUAL'))
    # profile.outputs.append(ModelOutput(model.vars., '', ModelOutputType.SVD_REG_FIELD, readable_name=''))
    profile.outputs.append(ModelOutput(model.vars.MODEM_CTRL3_ANTDIVMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.CTRL3.ANTDIVMODE'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_DIGMIXCTRL_BLEORZB, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DIGMIXCTRL.BLEORZB'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_DIGMIXCTRL_MULTIPHYHOP, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DIGMIXCTRL.MULTIPHYHOP'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_DIGMIXCTRL_HOPPINGSRC, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DIGMIXCTRL.HOPPINGSRC'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_DIGMIXCTRL_RXBRINTSHIFT, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DIGMIXCTRL.RXBRINTSHIFT'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_DIGMIXCTRL_DSSSCFECOMBO, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.DIGMIXCTRL.DSSSCFECOMBO'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNC3_SYNC3, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNC3.SYNC3'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SRCCHF_CHMUTETIMER, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SRCCHF.CHMUTETIMER'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_DSSSPMTIMEOUT, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.DSSSPMTIMEOUT'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_EHDSSSCTRL_DSSSFRMTIMEOUT, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.EHDSSSCTRL.DSSSFRMTIMEOUT'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_COCURRMODE_DSSSDSACHK, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.COCURRMODE.DSSSDSACHK'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_COCURRMODE_TRECSDSACHK, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.COCURRMODE.TRECSDSACHK'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_COCURRMODE_CORRCHKMUTE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.COCURRMODE.CORRCHKMUTE'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNCBITS2TH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNCBITS2TH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNC0ERRORS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNC0ERRORS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNC1ERRORS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNC1ERRORS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNC2ERRORS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNC2ERRORS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNC3ERRORS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNC2ERRORS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_DUALSYNC, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.DUALSYNC'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_DUALSYNC2TH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.DUALSYNC2TH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNCDET2TH, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNCDET2TH'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SYNCWORDCTRL_SYNCSWFEC, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SYNCWORDCTRL.SYNCSWFEC'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL0_SYMIDENTDIS, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL0.SYMIDENTDIS'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL1_FREQOFFTOLERANCE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL1.FREQOFFTOLERANCE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SISTARTDELAYMODE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SISTARTDELAYMODE'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_SICTRL2_SISTARTDELAY, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SICTRL2.SISTARTDELAY'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_CHFCTRL_ADJGAINWIN, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.CHFCTRL.ADJGAINWIN'))

    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN1_CLKMULTBWCAL, '', ModelOutputType.SVD_REG_FIELD,    readable_name='RAC.CLKMULTEN1.CLKMULTBWCAL'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN1_CLKMULTFREQCAL, '', ModelOutputType.SVD_REG_FIELD,  readable_name='RAC.CLKMULTEN1.CLKMULTFREQCAL'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN0_CLKMULTLDFNIB, '', ModelOutputType.SVD_REG_FIELD,   readable_name='RAC.CLKMULTEN0.CLKMULTLDFNIB'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN0_CLKMULTLDMNIB, '', ModelOutputType.SVD_REG_FIELD,   readable_name='RAC.CLKMULTEN0.CLKMULTLDMNIB'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN0_CLKMULTRDNIBBLE, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.CLKMULTEN0.CLKMULTRDNIBBLE'))
    profile.outputs.append(ModelOutput(model.vars.RAC_CLKMULTEN0_CLKMULTLDCNIB, '', ModelOutputType.SVD_REG_FIELD,   readable_name='RAC.CLKMULTEN0.CLKMULTLDCNIB'))
    profile.outputs.append(ModelOutput(model.vars.RAC_ADCCTRL1_ADCENHALFMODE, '', ModelOutputType.SVD_REG_FIELD,     readable_name='RAC.ADCCTRL1.ADCENHALFMODE'))
    profile.outputs.append(ModelOutput(model.vars.RAC_ADCCTRL0_ADCCLKSEL, '', ModelOutputType.SVD_REG_FIELD,         readable_name='RAC.ADCCTRL0.ADCCLKSEL'))
    profile.outputs.append(ModelOutput(model.vars.RAC_ADCEN0_ADCENNEGRES, '', ModelOutputType.SVD_REG_FIELD,         readable_name='RAC.ADCEN0.ADCENNEGRES'))
    profile.outputs.append(ModelOutput(model.vars.RAC_LNAMIXCTRL1_LNAMIXRFPKDTHRESHSELHI, '', ModelOutputType.SVD_REG_FIELD,  readable_name='RAC.LNAMIXCTRL1.LNAMIXRFPKDTHRESHSELHI'))
    profile.outputs.append(ModelOutput(model.vars.RAC_LNAMIXCTRL1_LNAMIXRFPKDTHRESHSELLO, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC_LNAMIXCTRL1_LNAMIXRFPKDTHRESHSELLO'))
    profile.outputs.append(ModelOutput(model.vars.RAC_TIACTRL0_TIATHRPKDHISEL, '', ModelOutputType.SVD_REG_FIELD,            readable_name='RAC.TIACTRL0.TIATHRPKDHISEL'))
    profile.outputs.append(ModelOutput(model.vars.RAC_TIACTRL0_TIATHRPKDLOSEL, '', ModelOutputType.SVD_REG_FIELD,            readable_name='RAC.TIACTRL0.TIATHRPKDLOSEL'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYLOCTRL0_SYLODIVADCDIVRATIO, '', ModelOutputType.SVD_REG_FIELD,         readable_name='RAC.SYLOCTRL0.SYLODIVADCDIVRATIO'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYLOCTRL0_SYLODIVREGTRIMVREG, '', ModelOutputType.SVD_REG_FIELD,         readable_name='RAC.SYLOCTRL0.SYLODIVREGTRIMVREG'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_TXCTRL_BR2M, '', ModelOutputType.SVD_REG_FIELD,readable_name='MODEM.TXCTRL.BR2M'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_TXCTRL_TXMOD, '', ModelOutputType.SVD_REG_FIELD,readable_name='MODEM.TXCTRL.TXMOD'))

    profile.outputs.append(ModelOutput(model.vars.AGC_GAINRANGE_PNINDEXBORDER, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.GAINRANGE.PNINDEXBORDER'))
    profile.outputs.append(ModelOutput(model.vars.AGC_AGCPERIOD0_PERIODHISTL, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.AGCPERIOD0.PERIODHISTL'))
    profile.outputs.append(ModelOutput(model.vars.AGC_AGCPERIOD0_PERIODLOSTL, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.AGCPERIOD0.PERIODLOSTL'))

    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN1_PNGAINDB1, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN1.PNGAINDB1'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN1_PNGAINDB2, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN1.PNGAINDB2'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN1_PNGAINDB3, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN1.PNGAINDB3'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN1_PNGAINDB4, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN1.PNGAINDB4'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN2_PNGAINDB5, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN2.PNGAINDB5'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN2_PNGAINDB6, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN2.PNGAINDB6'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN2_PNGAINDB7, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN2.PNGAINDB7'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN2_PNGAINDB8, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN2.PNGAINDB8'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN3_PNGAINDB9, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN3.PNGAINDB9'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN3_PNGAINDB10, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN3.PNGAINDB10'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN3_PNGAINDB11, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN3.PNGAINDB11'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN3_PNGAINDB12, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN3.PNGAINDB12'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN4_PNGAINDB13, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN4.PNGAINDB13'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN4_PNGAINDB14, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN4.PNGAINDB14'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN4_PNGAINDB15, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN4.PNGAINDB15'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNGAIN4_PNGAINDB16, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNGAIN4.PNGAINDB16'))

    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN1_LNAGAINDB1, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN1.LNAGAINDB1'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN1_LNAGAINDB2, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN1.LNAGAINDB2'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN1_LNAGAINDB3, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN1.LNAGAINDB3'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN1_LNAGAINDB4, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN1.LNAGAINDB4'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN2_LNAGAINDB5, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN2.LNAGAINDB5'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN2_LNAGAINDB6, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN2.LNAGAINDB6'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN2_LNAGAINDB7, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN2.LNAGAINDB7'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN2_LNAGAINDB8, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN2.LNAGAINDB8'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN3_LNAGAINDB9, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN3.LNAGAINDB9'))
    profile.outputs.append(ModelOutput(model.vars.AGC_LNAGAIN3_LNAGAINDB10, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAGAIN3.LNAGAINDB10'))

    profile.outputs.append(ModelOutput(model.vars.AGC_LNAMIXCODE2_LNAMIXCUREN, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.LNAMIXCODE2.LNAMIXCUREN'))

    profile.outputs.append(ModelOutput(model.vars.AGC_GAINSTEPLIM1_LNAINDEXMAX, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.GAINSTEPLIM1.LNAINDEXMAX'))

    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE0_TIACOMP1, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE0.TIACOMP1'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE0_TIACOMP2, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE0.TIACOMP2'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE0_TIACOMP3, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE0.TIACOMP3'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE0_TIACOMP4, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE0.TIACOMP4'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE0_TIACOMP5, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE0.TIACOMP5'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE0_TIACOMP6, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE0.TIACOMP6'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE1_TIACOMP7, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE1.TIACOMP7'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE1_TIACOMP8, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE1.TIACOMP8'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE1_TIACOMP9, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE1.TIACOMP9'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE1_TIACOMP10, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE1.TIACOMP10'))
    profile.outputs.append(ModelOutput(model.vars.AGC_TIACODE1_TIACOMP11, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.TIACODE1.TIACOMP11'))

    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN1_PGAGAINDB1, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN1.PGAGAINDB1'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN1_PGAGAINDB2, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN1.PGAGAINDB2'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN1_PGAGAINDB3, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN1.PGAGAINDB3'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN2_PGAGAINDB4, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN2.PGAGAINDB4'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN2_PGAGAINDB5, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN2.PGAGAINDB5'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN2_PGAGAINDB6, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN2.PGAGAINDB6'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN3_PGAGAINDB7, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN3.PGAGAINDB7'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN3_PGAGAINDB8, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN3.PGAGAINDB8'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN3_PGAGAINDB9, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN3.PGAGAINDB9'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN4_PGAGAINDB10, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN4.PGAGAINDB10'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PGAGAIN4_PGAGAINDB11, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PGAGAIN4.PGAGAINDB11'))

    #profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT0_LNAMIXRFATT1, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT0.LNAMIXRFATT1'))
    #profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT0_LNAMIXRFATT2, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT0.LNAMIXRFATT2'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT1_LNAMIXRFATT3, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT1.LNAMIXRFATT3'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT1_LNAMIXRFATT4, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT1.LNAMIXRFATT4'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT2_LNAMIXRFATT5, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT2.LNAMIXRFATT5'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT2_LNAMIXRFATT6, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT2.LNAMIXRFATT6'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT3_LNAMIXRFATT7, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT3.LNAMIXRFATT7'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT3_LNAMIXRFATT8, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT3.LNAMIXRFATT8'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT4_LNAMIXRFATT9, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT4.LNAMIXRFATT9'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT4_LNAMIXRFATT10, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT4.LNAMIXRFATT10'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT5_LNAMIXRFATT11, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT5.LNAMIXRFATT11'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT5_LNAMIXRFATT12, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT5.LNAMIXRFATT12'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT6_LNAMIXRFATT13, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT6.LNAMIXRFATT13'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT6_LNAMIXRFATT14, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT6.LNAMIXRFATT14'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT7_LNAMIXRFATT15, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT7.LNAMIXRFATT15'))
    profile.outputs.append(ModelOutput(model.vars.AGC_PNRFATT7_LNAMIXRFATT16, '', ModelOutputType.SVD_REG_FIELD, readable_name='AGC.PNRFATT7.LNAMIXRFATT16'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_CTRL5_FEFILTCLKSEL, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.CTRL5.FEFILTCLKSEL'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_CTRL5_AFIFOBYP, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.CTRL5.AFIFOBYP'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_TXCTRL_TXAFIFOBYP, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.TXCTRL.TXAFIFOBYP'))

    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_LEGACY_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.LEGACY_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_RXDC1CH_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.RXDC1CH_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_TRECS_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.TRECS_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_BCR_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.BCR_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_COHERENT_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.COHERENT_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_LONGRANGE_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.LONGRANGE_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_ENHDSSS_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.ENHDSSS_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_FSW_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.FSW_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_SOFTMODEM_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.SOFTMODEM_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_BTC_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.BTC_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_RXDC2CH_EN, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.RXDC2CH_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODEMINFO_SPARE0, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODEMINFO.SPARE0'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MMDDENOMINIT_CALC_DENOMINIT0, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MMDDENOMINIT_CALC.DENOMINIT0'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MMDDENOMINIT_CALC_DENOMINIT1, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MMDDENOMINIT_CALC.DENOMINIT1'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT0, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MMDDENOMINIT_CALC_DOUBLED.DENOMINIT0'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT1, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MMDDENOMINIT_CALC_DOUBLED.DENOMINIT1'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_MODINDEXE, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODINDEX_CALC.MODINDEXE'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_MODINDEXM, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODINDEX_CALC.MODINDEXM'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_FREQGAINE, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODINDEX_CALC.FREQGAINE'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_FREQGAINM, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODINDEX_CALC.FREQGAINM'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXE, '', ModelOutputType.SVD_REG_FIELD, readable_name='SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.MODINDEXE'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXM, '', ModelOutputType.SVD_REG_FIELD,readable_name='SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.MODINDEXM'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_FREQGAINE, '', ModelOutputType.SVD_REG_FIELD,readable_name='SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.FREQGAINE'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_FREQGAINM, '', ModelOutputType.SVD_REG_FIELD,readable_name='SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.FREQGAINM'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_MISC_SYNTH_MODE_TX00D_EN, '', ModelOutputType.SVD_REG_FIELD,readable_name='SEQ.MISC.SYNTH_MODE_TX00D_EN'))
    profile.outputs.append(ModelOutput(model.vars.SEQ_FSWCOREINFO_CONFIG, '', ModelOutputType.SVD_REG_FIELD,readable_name='SEQ.FSWCOREINFO.CONFIG'))

    profile.outputs.append(ModelOutput(model.vars.MODEM_SPARE_SPARE, '', ModelOutputType.SVD_REG_FIELD, readable_name='MODEM.SPARE.SPARE'))

## Synth_IF Series 3 Reigsters
def build_synth_regs_s3(model, profile):
    ## LPF
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRL_LOCKLPFBWGEAR0, '',         ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRL.LOCKLPFBWGEAR0'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRL_LOCKLPFBWGEAR1, '',         ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRL.LOCKLPFBWGEAR1'))

    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR2, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR3, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR4, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR5, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR5'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR6, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR6'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR7, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR7'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR8, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR8'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR9, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR9'))

    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR2, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR3, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR4, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR5, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR5'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR6, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR6'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR7, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR7'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR8, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR8'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR9, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR9'))

    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRL_LOCKLPFBWGEARSLOT, '',      ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRL.LOCKLPFBWGEARSLOT'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRL_LPFBWLOADDEL, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRL.LPFBWLOADDEL'))

    ## DSM
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLRX_DEMMODERX, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLRX.DEMMODERX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLRX_REQORDERRX, '',            ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLRX.REQORDERRX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLRX_QNCMODERX, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLRX.QNCMODERX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLRX_GLMSOVERRIDEVALRX, '',     ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLRX.GLMSOVERRIDEVALRX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLTX_LSBFORCETX, '',            ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLTX.LSBFORCETX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLTX_PHISELTX, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLTX.PHISELTX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLRX_PHISELRX, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLRX.PHISELRX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLTX_REQORDERTX, '',            ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLTX.REQORDERTX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DSMCTRLTX_QNCMODETX, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DSMCTRLTX.QNCMODETX'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_QNCCTRL_QNCOFFSET, '',               ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.QNCCTRL.QNCOFFSET'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_QNCCTRL_ENABLEDQNCTIME, '',          ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.QNCCTRL.ENABLEDQNCTIME'))
    ## DENOMINIT
    profile.outputs.append(ModelOutput(model.vars.SYNTH_MMDDENOMINIT_DENOMINIT0, '',         ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.MMDDENOMINIT.DENOMINIT0'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_MMDDENOMINIT_DENOMINIT1, '',         ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.MMDDENOMINIT.DENOMINIT1'))
    ## HOPPING
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRL_LPFBWDURINGHOP, '',          ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRL.LPFBWDURINGHOP'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_DLFCTRL_LPFBWAFTERHOP, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.DLFCTRL.LPFBWAFTERHOP'))
    ## LMS
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSENABLEDELAY, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSENABLEDELAY'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSGEAR0, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSGEAR0'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSGEAR1, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSGEAR1'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSGEAR2, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSGEAR2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSGEAR3, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSGEAR3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSGEAR4, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSGEAR4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_GLMS_GLMSGEARSLOT, '',                ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.GLMS.GLMSGEARSLOT'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSENABLEDELAY, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSENABLEDELAY'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSGEAR0, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSGEAR0'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSGEAR1, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSGEAR1'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSGEAR2, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSGEAR2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSGEAR3, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSGEAR3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSGEAR4, '',                   ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSGEAR4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_PLMS_PLMSGEARSLOT, '',                ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.PLMS.PLMSGEARSLOT'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LMSOVERRIDE_GLMSOVERRIDEEN, '',       ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LMSOVERRIDE.GLMSOVERRIDEEN'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LMSOVERRIDE_PLMSOVERRIDEEN, '',       ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LMSOVERRIDE.PLMSOVERRIDEEN'))
    ## FCAL
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION0, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION0'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION1, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION1'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION2, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION3, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION4, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION5, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION5'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION6, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION6'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_COMPANION_COMPANION7, '',             ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.COMPANION.COMPANION7'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE, '',               ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LOCNTCTRL.NUMCYCLE'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE1, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LOCNTCTRL.NUMCYCLE1'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE2, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LOCNTCTRL.NUMCYCLE2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE3, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LOCNTCTRL.NUMCYCLE3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE4, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LOCNTCTRL.NUMCYCLE4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE5, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.LOCNTCTRL.NUMCYCLE5'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_NUMCYCLE6, '',               ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.NUMCYCLE6'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_NUMCYCLE7, '',               ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.NUMCYCLE7'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_NUMCYCLE8, '',               ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.NUMCYCLE8'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_NUMCYCLE9, '',               ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.NUMCYCLE9'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_NUMCYCLE10, '',              ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.NUMCYCLE10'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_STEPWAIT, '',                ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.STEPWAIT'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT1, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT1'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT2, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT2'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT3, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT3'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT4, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT4'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT5, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT5'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT6, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT6'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT7, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT7'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT8, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT8'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT9, '',           ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT9'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT10, '',          ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALSTEPWAIT.STEPWAIT10'))
    profile.outputs.append(ModelOutput(model.vars.SYNTH_FCALCTRL_FCALMODE, '',                ModelOutputType.SVD_REG_FIELD, readable_name='SYNTH.FCALCTRL.FCALMODE'))
    # RFSYNTH CTRL
    profile.outputs.append(ModelOutput(model.vars.RAC_SYLOCTRL0_SYLODIVDSMDACCLKDIVRATIO, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYLOCTRL0.SYLODIVDSMDACCLKDIVRATIO'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYLOCTRLTX0_SYLODIVDSMDACCLKDIVRATIOTX, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYLOCTRLTX0.SYLODIVDSMDACCLKDIVRATIOTX'))
    # Select LODIV path (normal vs HADM)
    profile.outputs.append(ModelOutput(model.vars.RAC_LNAMIXEN1_LNAMIXMXRLOSEL, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.LNAMIXEN1.LNAMIXMXRLOSEL'))

  # fp select depend on TX power, move to rail code
  #  profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL1_SYLODIVSELFP4G82G4, '',          ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL1.SYLODIVSELFP4G82G4'))
  #  profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL1_SYLODIVSELFP4G82G4TX, '',       ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL1.SYLODIVSELFP4G82G4TX'))
  #  profile.outputs.append(ModelOutput(model.vars.RAC_SYMMDCTRL_SYMMDSEL56STG, '',            ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYMMDCTRL.SYMMDSEL56STG'))
  #  profile.outputs.append(ModelOutput(model.vars.RAC_SYMMDCTRL_SYMMDSEL56STGTX, '',          ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYMMDCTRL.SYMMDSEL56STGTX'))


    # DTIMLOSS feature for fast switch PHY optimization. See https://jira.silabs.com/browse/MCUW_RADIO_CFG-2525
    profile.outputs.append(ModelOutput(model.vars.MODEM_TRECSCFG_DTIMLOSSEN, '', ModelOutputType.SVD_REG_FIELD,
                                       readable_name='MODEM.TRECSCFG.DTIMLOSSEN'))
    profile.outputs.append(ModelOutput(model.vars.MODEM_TRECSCFG_DTIMLOSSTHD, '', ModelOutputType.SVD_REG_FIELD,
                                       readable_name='MODEM.TRECSCFG.DTIMLOSSTHD'))

    if (not (model.part_family.upper() == 'RAINIER' and model.part_revision in ['A0', 'A1', 'A2'])):
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_FASTCWDETECTEN, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.FASTCWDETECTEN'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_FASTSUPERCHIPEN, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.FASTSUPERCHIPEN'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_FASTNARROWPULSEEN, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.FASTNARROWPULSEEN'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_EYEOPENINGMODE, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.EYEOPENINGMODE'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_CWDETECTSTARTWINDOW, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.CWDETECTSTARTWINDOW'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_CWDETECTTHRESHOLD, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.CWDETECTTHRESHOLD'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_SUPERCHIPSTARTWINDOW, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.SUPERCHIPSTARTWINDOW'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_NPULSEFIXEDTHRESHOLD, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.NPULSEFIXEDTHRESHOLD'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_NPULSETHRESHADJEN, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.NPULSETHRESHADJEN'))
        profile.outputs.append(
            ModelOutput(model.vars.MODEM_SIFASTDETECTCTRL_NPULSETHRESHADJ, '', ModelOutputType.SVD_REG_FIELD,
                        readable_name='MODEM.SIFASTDETECTCTRL.NPULSETHRESHADJ'))

def build_syctrl_regs_s3(model, profile):
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL0_SYVCOTRIMIPTAT, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL0.SYVCOTRIMIPTAT'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL0_SYVCOTRIMIBIAS, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL0.SYVCOTRIMIBIAS'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL0_SYDSMDACTRIMLOADBALDLF, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL0.SYDSMDACTRIMLOADBALDLF'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL0_SYDSMDACTRIMLOADBALDSM, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL0.SYDSMDACTRIMLOADBALDSM'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRL0_SYENMMDREGREPLICA, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRL0.SYENMMDREGREPLICA'))

    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRLTX0_SYVCOTRIMIPTATTX, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRLTX0.SYVCOTRIMIPTATTX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRLTX0_SYVCOTRIMIBIASTX, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRLTX0.SYVCOTRIMIBIASTX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRLTX0_SYDSMDACTRIMLOADBALDLFTX, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRLTX0.SYDSMDACTRIMLOADBALDLFTX'))
    profile.outputs.append(ModelOutput(model.vars.RAC_SYCTRLTX0_SYENMMDREGREPLICATX, '', ModelOutputType.SVD_REG_FIELD, readable_name='RAC.SYCTRLTX0.SYENMMDREGREPLICATX'))