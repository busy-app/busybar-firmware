from pyradioconfig.parts.bobcat.calculators.calc_global import Calc_Global_Bobcat
from pycalcmodel.core.variable import ModelVariableFormat


class CalcGlobalRainier(Calc_Global_Bobcat):
    def buildVariables(self, model):
        # Build variables from the Ocelot calculations
        super().buildVariables(model)
        self._add_SYNTH_S3_regs(model)
        self._add_syctrl_regs_s3(model)
        self._add_spare_regs(model)

    def _add_shaping_filter_gain_variable(self, model):
        self._addModelActual(model, 'shaping_filter_gain', float, ModelVariableFormat.FLOAT)

    def _add_spare_regs(self, model):
        self._addModelRegister(model, 'MODEM.SPARE.SPARE', int, ModelVariableFormat.HEX)

    def _add_common_regs(self, model):
        super()._add_common_regs(model)
        self._addModelRegister(model, 'RAC.RX.SYPFDCHPLPENRX', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.DIGMIXCTRL.BLEORZB', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DIGMIXCTRL.MULTIPHYHOP', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DIGMIXCTRL.HOPPINGSRC', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DIGMIXCTRL.RXBRINTSHIFT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DIGMIXCTRL.DSSSCFECOMBO', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.DSSSPMTIMEOUT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.DSSSFRMTIMEOUT', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.COCURRMODE.DSSSDSACHK', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.COCURRMODE.TRECSDSACHK', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.COCURRMODE.CORRCHKMUTE', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNCBITS2TH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNC0ERRORS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNC1ERRORS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNC2ERRORS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNC3ERRORS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.DUALSYNC', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.DUALSYNC2TH', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNCDET2TH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNCWORDCTRL.SYNCSWFEC', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SYNC3.SYNC3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL0.SYMIDENTDIS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.FREQOFFTOLERANCE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.SISTARTDELAYMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.SISTARTDELAY', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'RAC.CLKMULTEN1.CLKMULTBWCAL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN1.CLKMULTFREQCAL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN0.CLKMULTLDFNIB', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN0.CLKMULTLDMNIB', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN0.CLKMULTRDNIBBLE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN0.CLKMULTLDCNIB', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'RAC.TIACTRL0.TIATHRPKDHISEL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.TIACTRL0.TIATHRPKDLOSEL', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'RAC.SYLOCTRL0.SYLODIVADCDIVRATIO', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.TXCTRL.BR2M', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.TXCTRL.TXMOD', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SYNC2.SYNC2', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.LOG2X4.LOG2X4FWSEL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.LOG2X4.LOG2X4', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.EXPECTPATTDUAL.EXPECTPATTDUAL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DUALTIM.DUALTIMEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DUALTIM.MINCOSTTHD2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DUALTIM.SYNCACQWIN2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.COCURRMODE.DSSSCONCURRENT', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.EHDSSSEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.DSSSTIMEACQUTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.FOEBIAS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.FREQCORREN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.DSSSFRQLIM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.DSSSDSATHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCTRL.DUALDSA', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.EHDSSSCFG0.DSSSPATT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG1.DSSSEXPSYNCLEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG1.DSSSCORRTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG1.DSSSDSAQTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.DSSSTIMCORRTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.DSSSFRTCORRTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.DSSSTRACKINGWIN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.DSSSCORRSCHWIN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.ONESYMBOLMBDD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.MAXSCHMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG2.DSSSDSAQUALEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG3.DSSSDASMAXTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG3.DSSSFOETRACKGEAR', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG3.OPMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG3.DSSSINITIMLEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.EHDSSSCFG3.LQIAVGWIN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.LRFRC.LRDSACORRTHD', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.CTRL3.ANTDIVMODE', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SICTRL0.SIMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL0.PEAKNUMTHRESHLW', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL0.FREQNOMINAL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL0.PEAKNUMADJ', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL0.NOISETHRESH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL0.NOISETHRESHADJ', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SICTRL1.TWOSYMBEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.SMALLSAMPLETHRESH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.SUPERCHIPTOLERANCE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.FASTMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.PEAKNUMP2ADJ', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.ZCEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.ZCSAMPLETHRESH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.SOFTCLIPBYPASS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL1.SOFTCLIPTHRESH', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SICTRL2.SIRSTAGCMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.SIRSTPRSMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.SIRSTCCAMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.DISSIFRAMEDET', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.AGCRSTUPONSI', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.SUPERCHIPNUM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.CORRNUM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.SHFTWIN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.NARROWPULSETHRESH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICTRL2.PEAKNUMADJEN', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.SICORR.CORRTHRESH', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICORR.CORRTHRESHLOW', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICORR.CORRTHRESHUP', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SICORR.CORRTHRESH2SYMB', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'RAC.CLKMULTEN1.CLKMULTDIVN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN1.CLKMULTDIVR', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN1.CLKMULTDIVX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN2.CLKMULTDIVNTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN2.CLKMULTDIVRTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN2.CLKMULTDIVXTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.CLKMULTEN2.CLKMULTENBYPASS40MHZTX', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.TRECSCFG.SOFTD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.TRECSCFG.SDSCALE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.DIGMIXCTRL.FWHOPPING', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.PHDMODCTRL.FASTHOPPINGEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.PHDMODCTRL.CHPWRQUAL', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.CHFCTRL.ADJGAINWIN', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.GAINRANGE.PNINDEXBORDER', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.AGCPERIOD0.PERIODHISTL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.AGCPERIOD0.PERIODLOSTL', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.PNGAIN1.PNGAINDB1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN1.PNGAINDB2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN1.PNGAINDB3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN1.PNGAINDB4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN2.PNGAINDB5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN2.PNGAINDB6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN2.PNGAINDB7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN2.PNGAINDB8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN3.PNGAINDB9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN3.PNGAINDB10', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN3.PNGAINDB11', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN3.PNGAINDB12', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN4.PNGAINDB13', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN4.PNGAINDB14', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN4.PNGAINDB15', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PNGAIN4.PNGAINDB16', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.LNAGAIN1.LNAGAINDB1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN1.LNAGAINDB2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN1.LNAGAINDB3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN1.LNAGAINDB4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN2.LNAGAINDB5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN2.LNAGAINDB6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN2.LNAGAINDB7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN2.LNAGAINDB8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN3.LNAGAINDB9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.LNAGAIN3.LNAGAINDB10', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.LNAMIXCODE2.LNAMIXCUREN', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.TIACODE0.TIACOMP1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE0.TIACOMP2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE0.TIACOMP3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE0.TIACOMP4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE0.TIACOMP5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE0.TIACOMP6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE1.TIACOMP7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE1.TIACOMP8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE1.TIACOMP9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE1.TIACOMP10', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE1.TIACOMP11', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.TIACODE2.TIACAPFB1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE2.TIACAPFB2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE2.TIACAPFB3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE2.TIACAPFB4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE3.TIACAPFB5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE3.TIACAPFB6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE3.TIACAPFB7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE3.TIACAPFB8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE4.TIACAPFB9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE4.TIACAPFB10', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.TIACODE4.TIACAPFB11', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'AGC.PGAGAIN1.PGAGAINDB1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN1.PGAGAINDB2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN1.PGAGAINDB3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN2.PGAGAINDB4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN2.PGAGAINDB5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN2.PGAGAINDB6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN3.PGAGAINDB7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN3.PGAGAINDB8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN3.PGAGAINDB9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN4.PGAGAINDB10', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'AGC.PGAGAIN4.PGAGAINDB11', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.CTRL5.FEFILTCLKSEL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.CTRL5.AFIFOBYP', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.TXCTRL.TXAFIFOBYP', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'SEQ.MODEMINFO.LEGACY_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.RXDC1CH_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.TRECS_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.BCR_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.COHERENT_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.LONGRANGE_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.ENHDSSS_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.FSW_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.SOFTMODEM_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.BTC_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.RXDC2CH_EN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODEMINFO.SPARE0', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'SEQ.FSWCOREINFO.CONFIG', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'RAC.LNAMIXEN1.LNAMIXMXRLOSEL', int, ModelVariableFormat.HEX)

    def _add_TXBR_regs(self, model):
        self._addModelRegister(model, 'MODEM.TXBR.TXBRNUM', int, ModelVariableFormat.HEX)

    def _add_REALTIMECFE_EXTENSCHBYP_field(self, model):
        pass

    #################################
    # Update for Series 3 SYIF
    #################################
    def _add_SYNTH_LPFCTRL_regs(self, model):
        self._addModelRegister(model, 'SYNTH.DLFCTRL.LOCKLPFBWGEAR0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRL.LOCKLPFBWGEAR1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR9', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR9', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'SYNTH.DLFCTRL.LOCKLPFBWGEARSLOT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRL.LPFBWLOADDEL', int, ModelVariableFormat.HEX)

    def _add_SYNTH_DSMCTRL_regs(self, model):
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.LSBFORCERX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.DITHERDACRX', int,
                               ModelVariableFormat.HEX)  # QNC need for RX HOP
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.DEMMODERX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.REQORDERRX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.QNCMODERX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.GLMSOVERRIDEVALRX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.LSBFORCETX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.DITHERDACTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.PHISELTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLRX.PHISELRX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.DEMMODETX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.REQORDERTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.QNCMODETX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DSMCTRLTX.GLMSOVERRIDEVALTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.QNCCTRL.QNCOFFSET', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.QNCCTRL.ENABLEDQNCTIME', int, ModelVariableFormat.HEX)

    def _add_SYNTH_DENOMINIT_regs(self, model):
        self._addModelRegister(model, 'SYNTH.MMDDENOMINIT.DENOMINIT0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.MMDDENOMINIT.DENOMINIT1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MMDDENOMINIT_CALC.DENOMINIT0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MMDDENOMINIT_CALC.DENOMINIT1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MMDDENOMINIT_CALC_DOUBLED.DENOMINIT0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MMDDENOMINIT_CALC_DOUBLED.DENOMINIT1', int, ModelVariableFormat.HEX)

    def _add_SYNTH_S3_regs(self, model):
        # Hopping
        self._addModelRegister(model, 'SYNTH.HOPPING.HOPLPFBWGEARSLOT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.HOPPING.HOPHCAPDELAY', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.HOPPING.HCAPRETIMEEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.HOPPING.HCAPRETIMESEL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRL.LPFBWDURINGHOP', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.DLFCTRL.LPFBWAFTERHOP', int, ModelVariableFormat.HEX)
        # LMS
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSENABLEDELAY', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSGEAR0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSGEAR1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSGEAR2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSGEAR3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSGEAR4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.GLMS.GLMSGEARSLOT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSENABLEDELAY', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSGEAR0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSGEAR1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSGEAR2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSGEAR3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSGEAR4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.PLMS.PLMSGEARSLOT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.LMSOVERRIDE.GLMSOVERRIDEEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.LMSOVERRIDE.PLMSOVERRIDEEN', int, ModelVariableFormat.HEX)
        # FCAL HW
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION0', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.COMPANION.COMPANION7', int, ModelVariableFormat.HEX)

        # self._addModelRegister(model, 'SYNTH.LOCNTCTRL.NUMCYCLE', int, ModelVariableFormat.HEX) #already defined
        self._addModelRegister(model, 'SYNTH.LOCNTCTRL.NUMCYCLE1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.LOCNTCTRL.NUMCYCLE2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.LOCNTCTRL.NUMCYCLE3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.LOCNTCTRL.NUMCYCLE4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.LOCNTCTRL.NUMCYCLE5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALCTRL.NUMCYCLE6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALCTRL.NUMCYCLE7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALCTRL.NUMCYCLE8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALCTRL.NUMCYCLE9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALCTRL.NUMCYCLE10', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'SYNTH.FCALCTRL.STEPWAIT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT1', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT2', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT3', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT4', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT5', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT6', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT7', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT8', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT9', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALSTEPWAIT.STEPWAIT10', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SYNTH.FCALCTRL.FCALMODE', int, ModelVariableFormat.HEX)

        # RFSYNTH CTRL
        self._addModelRegister(model, 'RAC.SYLOCTRL0.SYLODIVDSMDACCLKDIVRATIO', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYLOCTRL0.SYLODIVREGTRIMVREG', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYLOCTRLTX0.SYLODIVDSMDACCLKDIVRATIOTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC.MODINDEXE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC.MODINDEXM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC.FREQGAINE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC.FREQGAINM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.MODINDEXE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.MODINDEXM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.FREQGAINE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'SEQ.MODINDEX_CALC_MODINDEXE_DOUBLED.FREQGAINM', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'MODEM.TRECSCFG.DTIMLOSSEN', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.TRECSCFG.DTIMLOSSTHD', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'MODEM.SRCCHF.CHMUTETIMER', int, ModelVariableFormat.HEX)

        self._addModelRegister(model, 'SEQ.MISC.SYNTH_MODE_TX00D_EN', int, ModelVariableFormat.HEX)

        if not (model.part_family.upper() == 'RAINIER' and model.part_revision in ['A0', 'A1', 'A2']):
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.FASTCWDETECTEN', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.FASTSUPERCHIPEN', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.FASTNARROWPULSEEN', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.EYEOPENINGMODE', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.CWDETECTSTARTWINDOW', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.CWDETECTTHRESHOLD', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.SUPERCHIPSTARTWINDOW', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.NPULSEFIXEDTHRESHOLD', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.NPULSETHRESHADJEN', int, ModelVariableFormat.HEX)
            self._addModelRegister(model, 'MODEM.SIFASTDETECTCTRL.NPULSETHRESHADJ', int, ModelVariableFormat.HEX)

    def _add_RAC_CLKMULTEN0_en(self, model):
        """CLKMULTENADCDRV, CLKMULTENDACDRV become FW controlled as must shut them off during opposite operation"""
        pass

    def _add_RAC_ADC_regs(self, model):
        self._addModelRegister(model, 'RAC.ADCCTRL1.ADCENHALFMODE', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.ADCCTRL0.ADCCLKSEL', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.ADCEN0.ADCENNEGRES', int, ModelVariableFormat.HEX)

    def _add_RFPKDTHD_reg(self, model):
        self._addModelRegister(model, 'RAC.LNAMIXCTRL1.LNAMIXRFPKDTHRESHSELHI', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.LNAMIXCTRL1.LNAMIXRFPKDTHRESHSELLO', int, ModelVariableFormat.HEX)

    def _add_RAC_PGACTRL(self, model):
        pass

    def _add_IFADCTRIM1_regs(self, model):
        pass

    def _add_ADCPLL_regs(self, model):
        pass

    def _add_AGC_LNABOOST_reg(self, model):
        pass

    def _add_IFADCPLLDCO_regs(self, model):
        pass

    def _add_IFADCPLLEN_regs(self, model):
        pass

    def _add_RAC_SYTRIM_regs(self, model):
        pass

    def _add_RAC_RXTX_SY_regs(self, model):
        pass

    def _add_RAC_SYMMDCTRL_SYMMDMODE_regs(self, model):
        pass

    def _add_RAC_SYEN_regs(self, model):
        pass

    def _add_dig_ramp_en_reg_field(self, model):
        pass

    def _add_RAC_CLKMULTEN1_en(self, model):
        pass

    def _add_syctrl_regs_s3(self, model):
        self._addModelRegister(model, 'RAC.SYCTRL0.SYVCOTRIMIPTAT', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRL0.SYVCOTRIMIBIAS', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRL0.SYDSMDACTRIMLOADBALDLF', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRL0.SYDSMDACTRIMLOADBALDSM', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRL0.SYENMMDREGREPLICA', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRLTX0.SYVCOTRIMIPTATTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRLTX0.SYVCOTRIMIBIASTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRLTX0.SYDSMDACTRIMLOADBALDLFTX', int, ModelVariableFormat.HEX)
        self._addModelRegister(model, 'RAC.SYCTRLTX0.SYENMMDREGREPLICATX', int, ModelVariableFormat.HEX)
