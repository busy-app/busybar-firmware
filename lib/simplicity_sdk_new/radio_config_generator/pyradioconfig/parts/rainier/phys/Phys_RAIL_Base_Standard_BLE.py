from pyradioconfig.parts.bobcat.phys.Phys_RAIL_Base_Standard_BLE import PHYS_Bluetooth_LE_Bobcat
from pyradioconfig.parts.rainier.phys.Phys_RAIL_Base_Standard_IEEE802154 import PhysRailBaseStandardIeee802154Rainier
from pyradioconfig.parts.common.phys.phy_common import PHY_COMMON_FRAME_BLE_CODED, PHY_COMMON_FRAME_BLE_AOX
from py_2_and_3_compatibility import *
from pyradioconfig.calculator_model_framework.decorators.phy_decorators import do_not_inherit_prod_phys
from pyradioconfig.parts.rainier.phys.Phys_common import fast_detection_agc_settings


@do_not_inherit_prod_phys
class PhysRailBaseStandardBleRainier(PHYS_Bluetooth_LE_Bobcat):

    def BLE_1M_Signal_Identifier_Regs (self, phy, model):
        phy.profile_outputs.MODEM_SICTRL0_SIMODE.override = 3
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMTHRESHLW.override = 4
        phy.profile_outputs.MODEM_SICTRL0_FREQNOMINAL.override = 55
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMADJ.override = 1
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESH.override = 150
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESHADJ.override = 30
        phy.profile_outputs.MODEM_SICTRL0_SYMIDENTDIS.override = 0
        phy.profile_outputs.MODEM_SICTRL1_TWOSYMBEN.override = 0
        phy.profile_outputs.MODEM_SICTRL1_SMALLSAMPLETHRESH.override = 20
        phy.profile_outputs.MODEM_SICTRL1_SUPERCHIPTOLERANCE.override = 33
        phy.profile_outputs.MODEM_SICTRL1_FASTMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL1_PEAKNUMP2ADJ.override = 3
        phy.profile_outputs.MODEM_SICTRL1_ZCEN.override = 1
        phy.profile_outputs.MODEM_SICTRL1_ZCSAMPLETHRESH.override = 8
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPBYPASS.override = 1
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPTHRESH.override = 28
        phy.profile_outputs.MODEM_SICTRL1_FREQOFFTOLERANCE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SIRSTAGCMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL2_SIRSTPRSMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SIRSTCCAMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_DISSIFRAMEDET.override = 0
        phy.profile_outputs.MODEM_SICTRL2_AGCRSTUPONSI.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SUPERCHIPNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_CORRNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SHFTWIN.override = 7
        phy.profile_outputs.MODEM_SICTRL2_NARROWPULSETHRESH.override = 6
        phy.profile_outputs.MODEM_SICTRL2_PEAKNUMADJEN.override = 51  # 0b00110011
        phy.profile_outputs.MODEM_SICTRL2_SISTARTDELAYMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL2_SISTARTDELAY.override = 1
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH.override = 306
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHLOW.override = 56
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHUP.override = 31
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH2SYMB.override = 219

    def BLE_1M_Signal_Identifier_fast_hopping_overrides(self, phy, model):
        phy.profile_outputs.MODEM_SICTRL0_SYMIDENTDIS.override  = 1
        phy.profile_outputs.MODEM_SICTRL2_SISTARTDELAYMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL2_SISTARTDELAY.override = 1

        if model.part_revision not in ['A0', 'A1', 'A2']:
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_FASTCWDETECTEN.override = 1
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_FASTSUPERCHIPEN.override = 0
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_FASTNARROWPULSEEN.override = 1
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_EYEOPENINGMODE.override = 2
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_CWDETECTSTARTWINDOW.override = 5
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_CWDETECTTHRESHOLD.override = 25
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_SUPERCHIPSTARTWINDOW.override = 9
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_NPULSEFIXEDTHRESHOLD.override = 0
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_NPULSETHRESHADJEN.override = 5
            phy.profile_outputs.MODEM_SIFASTDETECTCTRL_NPULSETHRESHADJ.override = 2

    def BLE_2M_Signal_Identifier_Regs (self, phy, model):
        phy.profile_outputs.MODEM_SICTRL0_SIMODE.override = 2
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMTHRESHLW.override = 10
        phy.profile_outputs.MODEM_SICTRL0_FREQNOMINAL.override = 63
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMADJ.override = 2
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESH.override = 160
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESHADJ.override = 40
        phy.profile_outputs.MODEM_SICTRL0_SYMIDENTDIS.override = 0
        phy.profile_outputs.MODEM_SICTRL1_TWOSYMBEN.override = 0
        phy.profile_outputs.MODEM_SICTRL1_SMALLSAMPLETHRESH.override = 20
        phy.profile_outputs.MODEM_SICTRL1_SUPERCHIPTOLERANCE.override = 38
        phy.profile_outputs.MODEM_SICTRL1_FASTMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL1_PEAKNUMP2ADJ.override = 3
        phy.profile_outputs.MODEM_SICTRL1_ZCEN.override = 0
        phy.profile_outputs.MODEM_SICTRL1_ZCSAMPLETHRESH.override = 8
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPBYPASS.override = 0
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPTHRESH.override = 100
        phy.profile_outputs.MODEM_SICTRL1_FREQOFFTOLERANCE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SIRSTAGCMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL2_SIRSTPRSMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SIRSTCCAMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_DISSIFRAMEDET.override = 0
        phy.profile_outputs.MODEM_SICTRL2_AGCRSTUPONSI.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SUPERCHIPNUM.override = 5
        phy.profile_outputs.MODEM_SICTRL2_CORRNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SHFTWIN.override = 7
        phy.profile_outputs.MODEM_SICTRL2_NARROWPULSETHRESH.override = 31
        phy.profile_outputs.MODEM_SICTRL2_PEAKNUMADJEN.override = 51  # 0b00110011
        phy.profile_outputs.MODEM_SICTRL2_SISTARTDELAYMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SISTARTDELAY.override = 0
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH.override = 306
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHLOW.override = 50
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHUP.override = 25
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH2SYMB.override = 219

    def _BLE_fast_hopping_settings(self, phy, model):
    #     #Not yet defined in Rainier reg model
    #     pass
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        phy.profile_inputs.target_bwsel.value = 0.261  # chosen to deliver DEC0=4 and DEC1=2
        phy.profile_inputs.bandwidth_hz.value = 1225000
        phy.profile_inputs.baudrate_tol_ppm.value = 80
        phy.profile_inputs.syncword_0.value = long(0x6E8E)
        phy.profile_inputs.syncword_length.value = 16

        ## SYNC-word
        phy.profile_outputs.MODEM_SYNC2_SYNC2.override = 167
        phy.profile_outputs.MODEM_SYNC3_SYNC3.override = 47
        phy.profile_outputs.MODEM_SYNCWORDCTRL_SYNCDET2TH.override = 1
        phy.profile_outputs.MODEM_SYNCWORDCTRL_DUALSYNC2TH.override = 1
        phy.profile_outputs.MODEM_SYNCWORDCTRL_SYNCBITS2TH.override = 7
        phy.profile_outputs.MODEM_SYNCWORDCTRL_SYNCSWFEC.override = 0

        # AGC settings (consistent with 802154 fast switching AGC settings)
        fast_detection_agc_settings(phy, model)
        phy.profile_outputs.AGC_CTRL0_MODE.override = 4 #Temporary until testing is complete

        phy.profile_outputs.MODEM_AUTOCG_AUTOCGEN.override = 1

        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 1854799872
        phy.profile_outputs.MODEM_REALTIMCFE_SYNCACQWIN.override = 15
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 74
        phy.profile_outputs.MODEM_REALTIMCFE_MINCOSTTHD.override = 225
        phy.profile_outputs.MODEM_FRMSCHTIME_DSARSTSYCNEN.override = 1

        phy.profile_outputs.MODEM_DIGMIXCTRL_BLEORZB.override = 0
        phy.profile_outputs.MODEM_DIGMIXCTRL_MULTIPHYHOP.override = 0
        phy.profile_outputs.MODEM_DIGMIXCTRL_RXBRINTSHIFT.override = 0

        phy.profile_outputs.MODEM_PHDMODCTRL_PMDETEN.override = 0
        phy.profile_outputs.MODEM_PHDMODCTRL_PMDETTHD.override = 3
        phy.profile_outputs.MODEM_PRE_PREERRORS.override = 0

        phy.profile_outputs.MODEM_EHDSSSCTRL_EHDSSSEN.override = 0
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSDSATHD.override = 2
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSFRQLIM.override = 45
        phy.profile_outputs.MODEM_EHDSSSCTRL_DUALDSA.override = 0
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSTIMEACQUTHD.override = 16
        phy.profile_outputs.MODEM_EHDSSSCTRL_FOEBIAS.override = 1
        phy.profile_outputs.MODEM_EHDSSSCTRL_FREQCORREN.override = 3
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSFRQLIM.override = 45
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSPMTIMEOUT.override = 0
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSFRMTIMEOUT.override = 2

        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSEXPSYNCLEN.override = 128
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSCORRTHD.override = 400
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSDSAQTHD.override = 700
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSTIMCORRTHD.override = 600
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSFRTCORRTHD.override = 700
        phy.profile_outputs.MODEM_EHDSSSCFG2_ONESYMBOLMBDD.override = 1
        phy.profile_outputs.MODEM_EHDSSSCFG2_MAXSCHMODE.override = 0
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSTRACKINGWIN.override = 2
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSCORRSCHWIN.override = 4
        phy.profile_outputs.MODEM_EHDSSSCFG3_DSSSINITIMLEN.override = 3
        phy.profile_outputs.MODEM_EHDSSSCFG3_DSSSDASMAXTHD.override = 1400
        phy.profile_outputs.MODEM_EHDSSSCFG3_DSSSFOETRACKGEAR.override = 2
        phy.profile_outputs.MODEM_EHDSSSCFG3_OPMODE.override = 0

        ### CH power Qualification
        phy.profile_outputs.MODEM_PHDMODCTRL_CHPWRQUAL.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 3
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH0.override = 216
        phy.profile_outputs.AGC_CTRL7_SUBPERIOD.override = 1
        ### Using CH power to detect false DSA detection
        ### After DSA is detected within 2 DSSS symbol
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH1.override = 64
        ### If CH power is belower than the thrshold that is configured in COH0_COHCHPWRTH0
        ### for 8 chips, the DSA detection will be reset
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH2.override = 8

        # # AFC
        phy.profile_outputs.MODEM_AFCADJTX_AFCSCALEE.override = 0
        phy.profile_outputs.MODEM_AFCADJTX_AFCSCALEM.override = 0
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEE.override = 3
        phy.profile_outputs.MODEM_AFCADJRX_AFCSCALEM.override = 27
        phy.profile_outputs.MODEM_AFC_AFCGEAR.override = 3
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1

        # Signal Identifier
        self.BLE_1M_Signal_Identifier_Regs(phy, model)
        self.BLE_1M_Signal_Identifier_fast_hopping_overrides(phy, model)

    def _ble_agc_halfrate(self, phy, model):
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 245
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 0
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHI.override = 14
        phy.profile_outputs.AGC_AGCPERIOD1_PERIODLOW.override = 70
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHISTL.override = 1
        phy.profile_outputs.AGC_AGCPERIOD0_MAXHICNTTHD.override = 13
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 8
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 10
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 12
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 13

    def _ble_agc_fullrate(self, phy, model):
        phy.profile_outputs.AGC_CTRL0_PWRTARGET.override = 245
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 0
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHI.override = 14
        phy.profile_outputs.AGC_AGCPERIOD1_PERIODLOW.override = 70
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHISTL.override = 1
        phy.profile_outputs.AGC_AGCPERIOD0_MAXHICNTTHD.override = 13
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 8
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 10
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 12
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 13

    def PHY_Bluetooth_LE_TEST_Packet_Hop(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Test Packet',
                    phy_name=phy_name)
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.HFXOMULT
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.PH_MOD
        phy.profile_inputs.base_frequency_hz.value = long(2494630022)

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        phy.profile_inputs.target_bwsel.value = 0.261  # chosen to deliver DEC0=4 and DEC1=2
        phy.profile_inputs.bandwidth_hz.value = 1225000
        phy.profile_inputs.baudrate_tol_ppm.value = 80
        phy.profile_inputs.syncword_0.value = long(0x94826E8E)
        phy.profile_inputs.syncword_length.value = 32

        return phy

    def Bluetooth_LE_Viterbi_noDSA_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1099233

        """ Demod """
        phy.profile_inputs.dsa_enable.value = False

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = 0
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameters """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.rx_xtal_error_ppm.value = 62
        phy.profile_inputs.tx_xtal_error_ppm.value = 62

        """ Bandwidth """
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE

        """ Timing """
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0
        phy.profile_inputs.symbols_in_timing_window.value = 0

        """  """
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ Frequency comp """
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON

        """ Advanced Overrides """
        # Decimation and Demod Overrides
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 2
        phy.profile_outputs.MODEM_CTRL6_RXBRCALCDIS.override = 1
        
        # FRC Overrides
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1

        # This should be set only on DSA PHYs (PGLYNX-566), so we can clear it.
        phy.profile_outputs.FRC_CTRL_WAITEOFEN.override = 0

        # Viterbi Demod Overrides
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5

        #Half rate 1M  and full rate
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 120
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

        phy.profile_inputs.syncword_dualsync.value = False
        phy.profile_inputs.syncword_trisync.value = False

    def Bluetooth_LongRange_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_CODED(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2450000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 500000  # Net bitrate(data only)
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1230000

        """ Demod """
        phy.profile_inputs.dsa_enable.value = True

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = long(0)
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameter """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.rx_xtal_error_ppm.value = 62
        phy.profile_inputs.tx_xtal_error_ppm.value = 62

        """ Bandwidth """
        phy.profile_inputs.synth_settling_mode.value = model.vars.synth_settling_mode.var_enum.BLE_LR
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE

        """ Preamble """
        phy.profile_inputs.preamble_length.value = 20

        """ Timing """
        phy.profile_inputs.symbols_in_timing_window.value = 0
        phy.profile_inputs.errors_in_timing_window.value = 1
        phy.profile_inputs.timing_detection_threshold.value = 140
        phy.profile_inputs.timing_sample_threshold.value = 0

        """  """
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ FRC """
        phy.profile_inputs.header_addtrailtxdata_en.value = True # We need to add Trail TX Data to the header (subframe 0), which is the Coding/Rate Indication (CI/RI)
        phy.profile_inputs.header_calc_crc.value = False # The header (subframe 0) does not include a CRC
        phy.profile_inputs.header_excludesubframewcnt_en.value = True # With Dynamic frame length (DFL) mode, we need to exclude the header (subframe 0) from Word Counter
        phy.profile_inputs.header_white_en.value = False # No whitening in the header (subframe 0)

        # FIXME :MR Do we need this for LR?
        phy.profile_outputs.FRC_FECCTRL_CONVDECODEMODE.override = 0

        """ Advanced Overrides """
        #phy.profile_inputs.target_osr.value = 4
        phy.profile_inputs.src1_range_available_minimum.value = 128
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LONGRANGE
        phy.profile_inputs.frequency_comp_mode.value = model.vars.frequency_comp_mode.var_enum.INTERNAL_ALWAYS_ON
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE

        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.FRC_CTRL_RXFCDMODE.override = 0 # Override the RXFCDMODE to FCDMODE0 (FCD2 is reloaded when SCNT reaches 0)
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 0
        phy.profile_outputs.FRC_FCD0_WORDS.override = 3 # SYNCWORD size (4 bytes) - 1 = 3
        phy.profile_outputs.FRC_FCD2_ADDTRAILTXDATA.override = 0
        phy.profile_outputs.FRC_FCD2_CALCCRC.override = 1
        phy.profile_outputs.FRC_FCD2_EXCLUDESUBFRAMEWCNT.override = 0
        phy.profile_outputs.FRC_FCD2_INCLUDECRC.override = 1
        phy.profile_outputs.FRC_FCD2_WORDS.override = 2 ** phy.profile_outputs.FRC_FCD2_WORDS.var.get_bit_width() - 1
        phy.profile_outputs.FRC_AUTOCG_AUTOCGEN.override = 7
        phy.profile_outputs.FRC_CONVGENERATOR_GENERATOR0.override = 13
        phy.profile_outputs.FRC_CONVGENERATOR_GENERATOR1.override = 15
        phy.profile_outputs.FRC_FECCTRL_CONVMODE.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 1
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 1
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATACNT.override = 4
        phy.profile_outputs.MODEM_AFC_AFCRXMODE.override = 0 # Was missed
        phy.profile_outputs.MODEM_CTRL0_CODING.override = 3
        phy.profile_outputs.MODEM_CTRL0_FRAMEDETDEL.override = 1
        phy.profile_outputs.MODEM_CTRL1_COMPMODE.override = 3
        phy.profile_outputs.MODEM_CTRL1_SYNCDATA.override = 1
        phy.profile_outputs.MODEM_CTRL2_RATESELMODE.override = 2
        phy.profile_outputs.MODEM_CTRL3_TSAMPDEL.override =2 # Was missed
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 0
        phy.profile_outputs.MODEM_LONGRANGE1_CHPWRACCUDEL.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_HYSVAL.override = 3
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDPUGATE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE1_LRSS.override = 4
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 400

        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 8
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 14
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 20
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 26
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 32
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 38
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 44
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 50
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 56
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 62
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRTH11.override = 68

        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 1
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 2
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 3
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 4
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 5
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 6
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 7
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 8
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 9
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 10
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSH12.override = 11

        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 29
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 105  # MCUW-RADIO_CFG-1666: Reverting change from RC CharA2 this cleans PER Noise
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 120  # LR 125K MCUW-RADIO_CFG-1666: Reverting change from RC CharA2 this cleans PER Noise
        phy.profile_outputs.MODEM_LONGRANGE_LRBLE.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRCORRTHD.override = 1100
        phy.profile_outputs.MODEM_LONGRANGE_LRDEC.override = 3
        phy.profile_outputs.MODEM_LONGRANGE_LRTIMCORRTHD.override = 1000
        phy.profile_outputs.MODEM_LONGRANGE_LRCORRSCHWIN.override = 10

        phy.profile_outputs.MODEM_MODINDEX_FREQGAINE.override = 2
        phy.profile_outputs.MODEM_MODINDEX_FREQGAINM.override = 2

        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_DISDEMODOF.override = 1
        phy.profile_outputs.MODEM_VTTRACK_TIMEACQUTHD.override = 32


        phy.profile_inputs.syncword_dualsync.value = False
        phy.profile_inputs.syncword_trisync.value = False


    def BLE_DSA(selfs, phy, model):
        # Removed from Viper/Rainier
        pass

    def BLE_2M_DSA(self, phy, model):
        # Removed from Viper/Rainier
        pass

    def PHY_Bluetooth_LE_Viterbi_noDSA_Hop(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.FASTSW

        phy.profile_inputs.baudrate_tol_ppm.value = 80
        phy.profile_inputs.bandwidth_hz.value = 1250000
        phy.profile_inputs.syncword_0.value = long(0x6E8E)
        phy.profile_inputs.syncword_length.value = 16

        phy.profile_outputs.MODEM_FRMSCHTIME_DSARSTSYCNEN.override = 1

        phy.profile_outputs.MODEM_EHDSSSCTRL_EHDSSSEN.override = 0
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSTIMEACQUTHD.override = 16
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSEXPSYNCLEN.override = 128
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSCORRTHD.override = 600
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSDSAQTHD.override = 700
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSTIMCORRTHD.override = 32
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSFRTCORRTHD.override = 700
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSTRACKINGWIN.override = 5
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSCORRSCHWIN.override = 2
        phy.profile_outputs.MODEM_EHDSSSCFG2_ONESYMBOLMBDD.override = 1

        #### enable SID

        phy.profile_outputs.MODEM_SICTRL0_SIMODE.override = 3
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMTHRESHLW.override = 4
        phy.profile_outputs.MODEM_SICTRL0_FREQNOMINAL.override = 55
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMADJ.override = 1
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESH.override = 130
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESHADJ.override = 30
        phy.profile_outputs.MODEM_SICTRL1_TWOSYMBEN.override = 0
        phy.profile_outputs.MODEM_SICTRL1_SMALLSAMPLETHRESH.override = 20
        phy.profile_outputs.MODEM_SICTRL1_SUPERCHIPTOLERANCE.override = 33
        phy.profile_outputs.MODEM_SICTRL1_FASTMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL1_PEAKNUMP2ADJ.override = 3
        phy.profile_outputs.MODEM_SICTRL1_ZCEN.override = 1
        phy.profile_outputs.MODEM_SICTRL1_ZCSAMPLETHRESH.override = 8
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPBYPASS.override = 1
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPTHRESH.override = 28
        phy.profile_outputs.MODEM_SICTRL2_SIRSTAGCMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL2_SIRSTPRSMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SIRSTCCAMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_DISSIFRAMEDET.override = 0
        phy.profile_outputs.MODEM_SICTRL2_AGCRSTUPONSI.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SUPERCHIPNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_CORRNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SHFTWIN.override = 7
        phy.profile_outputs.MODEM_SICTRL2_NARROWPULSETHRESH.override = 31
        phy.profile_outputs.MODEM_SICTRL2_PEAKNUMADJEN.override = 51 # 0b00110011
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH.override = 306
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHLOW.override = 56
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHUP.override = 31
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH2SYMB.override = 219

        self.BLE_1M_Signal_Identifier_Regs(phy, model)
        self.BLE_1M_Signal_Identifier_fast_hopping_overrides(phy, model)

        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate_Hop(self, model, phy_name='PHY_Bluetooth_LE_Viterbi_noDSA_fullrate_Hop'):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.FASTSW

        phy.profile_inputs.hop_enable.value = model.vars.hop_enable.var_enum.ENABLED

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        phy.profile_inputs.target_bwsel.value = 0.261  # chosen to deliver DEC0=4 and DEC1=2
        phy.profile_inputs.bandwidth_hz.value = 1225000
        phy.profile_inputs.baudrate_tol_ppm.value = 80
        phy.profile_inputs.syncword_0.value = long(0x6E8E)
        phy.profile_inputs.syncword_length.value = 16
        ## force
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 3
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 5
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 8
        phy.profile_outputs.AGC_STEPDWN_STEPDWN0.override = 1
        phy.profile_outputs.AGC_STEPDWN_STEPDWN1.override = 2
        phy.profile_outputs.AGC_STEPDWN_STEPDWN2.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN3.override = 3
        phy.profile_outputs.AGC_STEPDWN_STEPDWN4.override = 5
        phy.profile_outputs.AGC_STEPDWN_STEPDWN5.override = 5
        phy.profile_outputs.AGC_CTRL0_MODE.override = 2
        phy.profile_outputs.AGC_CTRL1_RSSIPERIOD.override = 8
        phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPDEL.override = 45

        phy.profile_outputs.MODEM_AUTOCG_AUTOCGEN.override = 1

        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 1854799872
        phy.profile_outputs.MODEM_REALTIMCFE_SYNCACQWIN.override = 15
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 74
        phy.profile_outputs.MODEM_REALTIMCFE_MINCOSTTHD.override = 225
        phy.profile_outputs.MODEM_FRMSCHTIME_DSARSTSYCNEN.override = 1


        phy.profile_outputs.MODEM_PHDMODCTRL_PMDETEN.override = 0
        phy.profile_outputs.MODEM_PHDMODCTRL_PMDETTHD.override = 3
        phy.profile_outputs.MODEM_PRE_PREERRORS.override = 0

        #Force CH filter COE set-1 as same as enhanced DSSS scan phy
        #to reduce APB register reconfiguration

        phy.profile_outputs.MODEM_CHFCOE10_SET1COEFF0.override = 30
        phy.profile_outputs.MODEM_CHFCOE10_SET1COEFF1.override = 87
        phy.profile_outputs.MODEM_CHFCOE10_SET1COEFF2.override = 155
        phy.profile_outputs.MODEM_CHFCOE11_SET1COEFF3.override = 169
        phy.profile_outputs.MODEM_CHFCOE11_SET1COEFF4.override = 46
        phy.profile_outputs.MODEM_CHFCOE12_SET1COEFF5.override = 1780
        phy.profile_outputs.MODEM_CHFCOE12_SET1COEFF6.override = 3376
        phy.profile_outputs.MODEM_CHFCOE13_SET1COEFF7.override = 2984
        phy.profile_outputs.MODEM_CHFCOE13_SET1COEFF8.override = 2962
        phy.profile_outputs.MODEM_CHFCOE14_SET1COEFF10.override = 947
        phy.profile_outputs.MODEM_CHFCOE14_SET1COEFF9.override = 15896
        phy.profile_outputs.MODEM_CHFCOE15_SET1COEFF11.override = 2988
        phy.profile_outputs.MODEM_CHFCOE15_SET1COEFF12.override = 5154
        phy.profile_outputs.MODEM_CHFCOE16_SET1COEFF13.override = 6813
        phy.profile_outputs.MODEM_CHFCOE16_SET1COEFF14.override = 7436

        # phy.profile_inputs.syncword_dualsync.value = False
        # phy.profile_inputs.syncword_trisync.value = False
        # Enhanced DSSS demod settings
        phy.profile_outputs.MODEM_EHDSSSCTRL_EHDSSSEN.override = 0
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSTIMEACQUTHD.override = 16
        phy.profile_outputs.MODEM_EHDSSSCTRL_FOEBIAS.override = 1
        phy.profile_outputs.MODEM_EHDSSSCTRL_FREQCORREN.override = 1
        phy.profile_outputs.MODEM_EHDSSSCTRL_DSSSFRQLIM.override = 16

        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSEXPSYNCLEN.override = 128
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSCORRTHD.override = 400
        phy.profile_outputs.MODEM_EHDSSSCFG1_DSSSDSAQTHD.override = 700
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSTIMCORRTHD.override = 600
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSFRTCORRTHD.override = 700
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSTRACKINGWIN.override = 5
        phy.profile_outputs.MODEM_EHDSSSCFG2_DSSSCORRSCHWIN.override = 8
        phy.profile_outputs.MODEM_EHDSSSCFG2_ONESYMBOLMBDD.override = 1
        phy.profile_outputs.MODEM_FRMSCHTIME_DSARSTSYCNEN.override = 1


        #### enable SID

        phy.profile_outputs.MODEM_SICTRL0_SIMODE.override = 3
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMTHRESHLW.override = 4
        phy.profile_outputs.MODEM_SICTRL0_FREQNOMINAL.override = 55
        phy.profile_outputs.MODEM_SICTRL0_PEAKNUMADJ.override = 1
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESH.override = 150
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESHADJ.override = 30
        phy.profile_outputs.MODEM_SICTRL1_TWOSYMBEN.override = 0
        phy.profile_outputs.MODEM_SICTRL1_SMALLSAMPLETHRESH.override = 20
        phy.profile_outputs.MODEM_SICTRL1_SUPERCHIPTOLERANCE.override = 33
        phy.profile_outputs.MODEM_SICTRL1_FASTMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL1_PEAKNUMP2ADJ.override = 3
        phy.profile_outputs.MODEM_SICTRL1_ZCEN.override = 1
        phy.profile_outputs.MODEM_SICTRL1_ZCSAMPLETHRESH.override = 8
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPBYPASS.override = 1
        phy.profile_outputs.MODEM_SICTRL1_SOFTCLIPTHRESH.override = 28
        phy.profile_outputs.MODEM_SICTRL2_SIRSTAGCMODE.override = 1
        phy.profile_outputs.MODEM_SICTRL2_SIRSTPRSMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SIRSTCCAMODE.override = 0
        phy.profile_outputs.MODEM_SICTRL2_DISSIFRAMEDET.override = 0
        phy.profile_outputs.MODEM_SICTRL2_AGCRSTUPONSI.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SUPERCHIPNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_CORRNUM.override = 0
        phy.profile_outputs.MODEM_SICTRL2_SHFTWIN.override = 7
        phy.profile_outputs.MODEM_SICTRL2_NARROWPULSETHRESH.override = 31
        phy.profile_outputs.MODEM_SICTRL2_PEAKNUMADJEN.override = 51 # 0b00110011
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH.override = 306
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHLOW.override = 56
        phy.profile_outputs.MODEM_SICORR_CORRTHRESHUP.override = 31
        phy.profile_outputs.MODEM_SICORR_CORRTHRESH2SYMB.override = 219

        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT0.override = 0
        phy.profile_outputs.FRC_PUNCTCTRL_PUNCT1.override = 0

        self._BLE_fast_hopping_settings(phy, model)

        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(self, model, phy_name='PHY_Bluetooth_LE_Viterbi_noDSA_fullrate'):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA Fullrate PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M

        # default bandwidth will cause halfrate unless forced
        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        phy.profile_inputs.target_bwsel.value = 0.23  # chosen to deliver DEC0=4 and DEC1=2

        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi No DSA PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LE_Viterbi_noDSA_halfrate_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_1M
        # Increase DCCOMPGEAR to match DC compensation speed with fullrate PHYs.
        # See https://jira.silabs.com/browse/PGRAINIERVALTEST-91
        phy.profile_outputs.MODEM_DCCOMP_DCCOMPGEAR.override = 4

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 11000
        phy.profile_outputs.tx_sync_delay_ns.override = 3500
        phy.profile_outputs.tx_eof_delay_ns.override = 750

        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_iqmod(self, model, phy_name='PHY_Bluetooth_LE_Viterbi_noDSA_iqmod'):
        phy = self.PHY_Bluetooth_LE_Viterbi_noDSA(model, phy_name=phy_name)
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        self.BLE_TX_Shaping_Coeffs_IQMOD(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD

        return phy

    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE Viterbi 2M No DSA Fullrate PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.LE_2M

        phy.profile_inputs.adc_rate_mode.value = model.vars.adc_rate_mode.var_enum.FULLRATE
        # 2M Full rate
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 60

        phy.profile_outputs.rx_sync_delay_ns.override = 12492
        phy.profile_outputs.rx_eof_delay_ns.override = 6000
        phy.profile_outputs.tx_sync_delay_ns.override = 3000
        phy.profile_outputs.tx_eof_delay_ns.override = 535

        return phy

    def PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate_iqmod(self, model, phy_name='PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate_iqmod'):
        phy = self.PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(model, phy_name=phy_name)
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        self.BLE_2M_TX_Shaping_Coeffs_IQMOD(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD

        return phy

    def PHY_Bluetooth_LE_HDT(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='BLE HDT PHY', phy_name=phy_name)
        self.Bluetooth_LE_2M_Viterbi_noDSA_base(phy, model)
        model.vars.protocol_id.value_forced = model.vars.protocol_id.var_enum.HDT
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.HDT
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Root_Raised_Cosine
        phy.profile_inputs.shaping_filter_param.value = 0.4

        # IFPKD thresholds
        phy.profile_outputs.RAC_TIACTRL0_TIATHRPKDHISEL.override = 4  #250 mV
        phy.profile_outputs.RAC_TIACTRL0_TIATHRPKDLOSEL.override = 1  #100 mV

        return phy

    def PHY_Bluetooth_LE_Viterbi_noDSA_fullrate_iqmod(self, model, phy_name='PHY_Bluetooth_LE_Viterbi_noDSA_fullrate_iqmod'):
        phy = self.PHY_Bluetooth_LE_Viterbi_noDSA_fullrate(model, phy_name=phy_name)
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        self.BLE_TX_Shaping_Coeffs_IQMOD(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD

        return phy

    def PHY_Bluetooth_LongRange_dsa_125kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 125kbps PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LongRange_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K
        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 1000
        phy.profile_outputs.MODEM_LRFRC_LRDSACORRTHD.override = 1000
        return phy

    def PHY_Bluetooth_LongRange_NOdsa_125kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='PHY_luetooth_LongRange_NOdsa_125kbps',
                            phy_name=phy_name)
        self.Bluetooth_LongRange_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_125K

        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0

        phy.profile_outputs.rx_sync_delay_ns.override = 7000
        phy.profile_outputs.rx_eof_delay_ns.override = 6500
        phy.profile_outputs.tx_sync_delay_ns.override = 5375
        phy.profile_outputs.tx_eof_delay_ns.override = 500

        return phy

    def PHY_Bluetooth_LongRange_NOdsa_125kbps_iqmod(self, model, phy_name=None):
        phy = self.PHY_Bluetooth_LongRange_NOdsa_125kbps(model, "PHY_Bluetooth_LongRange_NOdsa_125kbps_iqmod")
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        self.BLE_TX_Shaping_Coeffs_IQMOD(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD

        return phy

    def PHY_Bluetooth_LongRange_dsa_500kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth LongRange DSA 500kbps PHY for Rainier',
                            phy_name=phy_name)
        self.Bluetooth_LongRange_500kbps_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K

        phy.profile_outputs.MODEM_LONGRANGE1_LRTIMEOUTTHD.override = 1000
        phy.profile_outputs.MODEM_LRFRC_LRDSACORRTHD.override = 1000
        return phy

    def PHY_Bluetooth_LongRange_NOdsa_500kbps(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Bluetooth_LongRange_NOdsa_500kbps',
                            phy_name=phy_name)
        self.Bluetooth_LongRange_500kbps_base(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CODED_500K

        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0

        phy.profile_outputs.rx_sync_delay_ns.override = 6750
        phy.profile_outputs.rx_eof_delay_ns.override = 6500
        phy.profile_outputs.tx_sync_delay_ns.override = 5375
        phy.profile_outputs.tx_eof_delay_ns.override = 500

        return phy

    def PHY_Bluetooth_LongRange_NOdsa_500kbps_iqmod(self, model, phy_name='PHY_Bluetooth_LongRange_NOdsa_500kbps_iqmod'):
        phy = self.PHY_Bluetooth_LongRange_NOdsa_500kbps(model, phy_name=phy_name)
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        self.BLE_TX_Shaping_Coeffs_IQMOD(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD

        return phy

    # BLE 1M Viterbi
    def Bluetooth_LE_Viterbi_base(self, phy, model):
        PHY_COMMON_FRAME_BLE_AOX(phy, model)
        self.BLE_AGC(phy, model)
        self.BLE_TX_Shaping_Coeffs(phy, model)
        self.BLE_DSA(phy, model)

        """ Channel and Bandwidth Settings """
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.deviation.value = 250000
        phy.profile_inputs.channel_spacing_hz.value = 2000000
        phy.profile_inputs.bitrate.value = 1000000
        phy.profile_inputs.baudrate_tol_ppm.value = 5000
        phy.profile_inputs.bandwidth_hz.value = 1099233

        """ Demod """
        phy.profile_inputs.dsa_enable.value = True

        """ Symbol Mapping and Encoding """
        phy.profile_inputs.modulation_type.value = model.vars.modulation_type.var_enum.FSK2
        phy.profile_inputs.fsk_symbol_map.value = model.vars.fsk_symbol_map.var_enum.MAP0
        phy.profile_inputs.diff_encoding_mode.value = model.vars.diff_encoding_mode.var_enum.DISABLED

        """ DSSS """
        phy.profile_inputs.dsss_chipping_code.value = 0
        phy.profile_inputs.dsss_len.value = 0
        phy.profile_inputs.dsss_spreading_factor.value = 0

        """ Shaping Filter Parameters """
        phy.profile_inputs.shaping_filter.value = model.vars.shaping_filter.var_enum.Gaussian
        phy.profile_inputs.shaping_filter_param.value = 0.5

        """ Frequency """
        phy.profile_inputs.xtal_frequency_hz.value = 38400000
        phy.profile_inputs.tx_xtal_error_ppm.value = 0
        phy.profile_inputs.rx_xtal_error_ppm.value = 20

        """ Bandwidth """
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_BLE

        """  """
        phy.profile_inputs.freq_offset_hz.value = 0
        phy.profile_inputs.if_frequency_hz.value = 1370000

        """ Advanced Overrides """
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 65535
        phy.profile_outputs.MODEM_TXBR_TXBRNUM.override = 53773
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 100

        """ Viterbi Demod Overrides """
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24

    def PHY_Bluetooth_LE(self, model, phy_name=None):
        pass


    def override_other_shaping_coeff_to_zero(self, phy, start_coeff = 0):
        # this method is to be used to override unused coefficients to zero.
        total_coeff = 64 # for Viper we have maximum of 64 filter tap coefficients
        for coeff in range(start_coeff, total_coeff):
            register_group = int(coeff / 4)
            command = "phy.profile_outputs.MODEM_SHAPING"+str(register_group)+"_COEFF"+str(coeff)+".override = 0"
            exec(command)

    def PHY_Bluetooth_1M_Concurrent(self, model, phy_name='PHY_Bluetooth_1M_Concurrent'):
        # Copied from bobcat and changed base PHY from DSA to noDSA
        # Start with the BLE LR 125k PHY
        phy = self.PHY_Bluetooth_LongRange_NOdsa_125kbps(model, phy_name=phy_name)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.CONCURRENT
        phy.profile_inputs.preamble_pattern.value = 0b10

        # The concurrent PHY needs to have:
        # Enable concurrent mode
        # FRC = BLE LR 125k, with added sub-frame description for BLE 1M uncoded
        # Demod = BLE LR 125k
        # TRECS/Viterbi = BLE 1M
        # BLE LR = BLE LR 125k
        # DSA = No DSA

        # Enable concurrent mode
        phy.profile_outputs.MODEM_COCURRMODE_CONCURRENT.override = 1

        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI1.override = 65
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI2.override = 45
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 24
        phy.profile_outputs.MODEM_VITERBIDEMOD_VTDEMODEN.override = 1
        phy.profile_outputs.MODEM_VITERBIDEMOD_SYNTHAFC.override = 1
        phy.profile_outputs.MODEM_VTBLETIMING_TIMINGDELAY.override = 220
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPECTHT.override = 5
        phy.profile_outputs.MODEM_VTCORRCFG1_EXPSYNCLEN.override = 146
        phy.profile_outputs.MODEM_VTCORRCFG1_VTFRQLIM.override = 130
        phy.profile_outputs.MODEM_VTCORRCFG0_EXPECTPATT.override = 2491575950

        # Overrides for TX mode (copied from BLE 1M Fullrate)
        phy.profile_inputs.base_frequency_hz.value = long(2402000000)
        phy.profile_inputs.channel_spacing_hz.value = 1000000
        phy.profile_outputs.FRC_FCD0_ADDTRAILTXDATA.override = 0
        phy.profile_outputs.FRC_FCD0_CALCCRC.override = 1
        phy.profile_outputs.FRC_FCD0_EXCLUDESUBFRAMEWCNT.override = 0
        phy.profile_outputs.FRC_FCD0_INCLUDECRC.override = 1
        phy.profile_outputs.FRC_FCD0_WORDS.override = 2 ** phy.profile_outputs.FRC_FCD0_WORDS.var.get_bit_width() - 1
        phy.profile_outputs.FRC_TRAILTXDATACTRL_TRAILTXDATACNT.override = 0
        phy.profile_outputs.MODEM_CTRL1_SYNCDATA.override = 0
        phy.profile_outputs.MODEM_CTRL2_RATESELMODE.override = 0
        phy.profile_outputs.MODEM_PRE_TXBASES.override = 4
        phy.profile_outputs.MODEM_CTRL0_CODING.override = 0

        phy.profile_outputs.FRC_FECCTRL_CONVMODE.override = 0

        # : Set for both Bobcat and Lynx
        phy.profile_outputs.MODEM_CGCLKSTOP_FORCEOFF.override = 56831  # : copy clock gating settings from BLE 1M
        phy.profile_outputs.MODEM_VITERBIDEMOD_VITERBIKSI3.override = 32  # Needed because BW is wider than non-concurrent 1M

        # : needed as part of IPMCUSRW-771 workaround
        # : https://jira.silabs.com/browse/IPMCUSRW-771
        phy.profile_outputs.MODEM_LONGRANGE1_LOGICBASEDLRDEMODGATE.override = 0
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 0 # MCUW_RADIO_CFG-1705, RAIL_LIB-6014 to be compatible with lynx workaround of MCUW_RADIO_CFG-1701

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 10700
        phy.profile_outputs.tx_sync_delay_ns.override = 2000
        phy.profile_outputs.tx_eof_delay_ns.override = 2000

        return phy

    def _ble_lr_dutycycle_overrides(self, phy, model):
        """For power sense algorithm:

        phy.profile_outputs.AGC_CTRL7_SUBDEN.override = 2
        phy.profile_outputs.AGC_CTRL7_SUBINT.override = 16
        phy.profile_outputs.AGC_CTRL7_SUBNUM.override = 0
        phy.profile_outputs.AGC_CTRL7_SUBPERIOD.override = 1
        phy.profile_outputs.MODEM_LONGRANGE_LRBLEDSA.override = 0
        """
        # Enable noise detector
        self.BLE_1M_Signal_Identifier_fast_hopping_overrides(phy, model)
        self.BLE_1M_Signal_Identifier_Regs(phy, model)
        # Reduce RF settling time by 5 us (cannot be used in 2 channel mode)
        model.vars.adc_clock_mode.var_value = model.vars.adc_clock_mode.var_enum.VCODIV
        phy.profile_inputs.rxdc_power_save_mode.value = model.vars.rxdc_power_save_mode.var_enum.SYNTH
        phy.profile_inputs.rxdc_power_save_time_us.value = 20
        phy.profile_outputs.MODEM_SRCCHF_CHMUTETIMER.override = 280  # Control minimum ON time

    def PHY_Bluetooth_LongRange_nodsa_125kbps_dutycycle(self, model, phy_name='PHY_Bluetooth_LongRange_nodsa_125kbps_dutycycle'):
        phy = self.PHY_Bluetooth_LongRange_NOdsa_125kbps(model, phy_name)
        self._ble_lr_dutycycle_overrides(phy, model)
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESH.override = 200  # Improve sensitivity, more false detect

        return phy

    def PHY_Bluetooth_LongRange_nodsa_500kbps_dutycycle(self, model, phy_name='PHY_Bluetooth_LongRange_nodsa_500kbps_dutycycle'):
        phy = self.PHY_Bluetooth_LongRange_NOdsa_500kbps(model, phy_name)
        self._ble_lr_dutycycle_overrides(phy, model)
        phy.profile_outputs.MODEM_SICTRL0_NOISETHRESH.override = 190  # Improve sensitivity, more false detect

        return phy



    # ##############
    # Prod PHY definition with new names
    # #############

    def PHY_Bluetooth_1M_HADM(self, model, phy_name='PHY_Bluetooth_1M_HADM'):
        phy = super().PHY_Bluetooth_1M_HADM(model)
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        phy.profile_inputs.synchronous_ifadc_clk.value = True
        phy.profile_inputs.synchronous_mixdac_clk.value = True
        self.BLE_TX_Shaping_Coeffs_IQMOD(phy, model)
        # IQMOD uses MODE1 for synth_tx_mode
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD
        phy.profile_inputs.tx_rdm_state.value = model.vars.tx_rdm_state.var_enum.TX_HADM
        phy.profile_inputs.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_HADM_RFPKD

        return phy

    def PHY_Bluetooth_2M_HADM(self, model, phy_name='PHY_Bluetooth_2M_HADM'):
        phy = super().PHY_Bluetooth_2M_HADM(model)
        phy.profile_inputs.modulator_select.value = model.vars.modulator_select.var_enum.IQ_MOD
        phy.profile_inputs.synchronous_ifadc_clk.value = True
        phy.profile_inputs.synchronous_mixdac_clk.value = True
        self.BLE_2M_TX_Shaping_Coeffs_IQMOD(phy, model)
        # IQMOD uses MODE1 for synth_tx_mode
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD
        phy.profile_inputs.tx_rdm_state.value = model.vars.tx_rdm_state.var_enum.TX_HADM
        phy.profile_inputs.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_HADM_RFPKD

        return phy

    def PHY_Bluetooth_2M_HADM_2BT(self, model, phy_name='PHY_Bluetooth_2M_HADM_2bt'):
        phy = self.PHY_Bluetooth_2M_HADM(model)
        # --------------
        # this has no effect as of now but will reflect correct shaping coeff in cfg
        phy.profile_inputs.shaping_filter_param.value = 2.0
        # ---------------
        self.BLE_2M_TX_Shaping_Coeffs_2bt_IQMOD(phy, model)
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.HADM_2M_2BT
        # IQMOD uses MODE1 for synth_tx_mode
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE_IQMOD
        phy.profile_inputs.tx_rdm_state.value = model.vars.tx_rdm_state.var_enum.TX_HADM
        phy.profile_inputs.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_HADM_RFPKD

        return phy

    def PHY_Bluetooth_1M_AOX(self, model, phy_name='PHY_Bluetooth_1M_AOX'):
        # bobcat uses half-rate PHY compared to Lynx, due to jitter and settling improvements
        # For Bobcat BLE 1M, use PHY_Bluetooth_1M_AOX_prod as there is no difference in TX SYNTH PLL BW
        phy = self.PHY_Bluetooth_LE_Viterbi_noDSA(model, phy_name=phy_name)
        phy.profile_inputs.aox_enable.value = model.vars.aox_enable.var_enum.ENABLED
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_1M
        self.BLE_RX_AOX_CHF_Coeffs(phy, model)

        # Override for AoX
        # 1 Mbps AoX SYNTH PLL should track the non-AOX PHY for whatever is needed for emissions / PA-VCO coupling.
        # Both 1 MHz BW, 1.5 MHz BW pass AoX mod char, do whatever it takes in the non-AOX PHY to make it pass mod char.

        phy.profile_outputs.rx_sync_delay_ns.override = 50000
        phy.profile_outputs.rx_eof_delay_ns.override = 11000
        phy.profile_outputs.tx_sync_delay_ns.override = 3500
        phy.profile_outputs.tx_eof_delay_ns.override = 750

        return phy

    def PHY_Bluetooth_2M_AOX(self, model, phy_name="PHY_Bluetooth_2M_AOX"):
        # always use full-rate PHY on all parts
        # For Bobcat BLE 2M, there are separate PHY_Bluetooth_2M_AOX_prod and PHY_Bluetooth_2M_prod to meet emissions
        # Both PHYs must support AoX RX, the difference is in the SYNTH PLL setting + shaping filter for TX
        phy = self.PHY_Bluetooth_LE_2M_Viterbi_noDSA_fullrate(model, phy_name)
        phy.profile_inputs.aox_enable.value = model.vars.aox_enable.var_enum.ENABLED
        model.vars.ble_feature.value_forced = model.vars.ble_feature.var_enum.AOX_2M
        self.BLE_RX_AOX_CHF_Coeffs(phy, model)

        # Override for AoX
        self.BLE_2M_AOX_TX_Shaping_Coeffs(phy, model)
        model.vars.synth_tx_mode.value_forced = model.vars.synth_tx_mode.var_enum.MODE4  # 3M PLL BW - this is important for CTE modulation characteristic
        """
        This higher PLL BW of AoX affects emissions. For low output power (< 6 dBm, i.e. Lynx) this is not a concern.
        For high output power, must consider BLE in-band emissions and FCC restricted band emissions.
        Max output lower must be limited by RAIL appropriately
        """

        phy.profile_outputs.rx_sync_delay_ns.override = 12492
        phy.profile_outputs.rx_eof_delay_ns.override = 6000
        phy.profile_outputs.tx_sync_delay_ns.override = 3000
        phy.profile_outputs.tx_eof_delay_ns.override = 535

        return phy

    def PHY_Bluetooth_1M_HADM_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE HADM 1Mbps PHY',
                            phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 40000000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.HADM_1M

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40
        return phy

    def PHY_Bluetooth_2M_HADM_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE HADM 2Mbps PHY',
                            phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 40000000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.HADM_2M

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40
        return phy

    def PHY_Bluetooth_2M_HADM_2BT_prod(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.BLE, readable_name='Production BLE HADM 2Mbps 2BT PHY',
                            phy_name=phy_name)
        phy.profile_inputs.xtal_frequency_hz.value = 40000000
        phy.profile_inputs.ble_feature.value = model.vars.ble_feature.var_enum.HADM_2M_2BT

        phy.profile_inputs.chcfg_channel_number_start.value = 0
        phy.profile_inputs.chcfg_channel_number_end.value = 39
        phy.profile_inputs.chcfg_physical_channel_offset.value = 0

        phy.profile_inputs.rail_tx_power_max.value = [-1] * 40
        return phy


    # ##############
    # Define BLE TX shaping coeffs for Rainier
    # #############

    def BLE_TX_Shaping_Coeffs(self, phy, model):
        # BLE1M Bandwidth Setting = Gear9
        # PGRAINIER-1908
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 1
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = -1
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = -4
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 21
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 28
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 56
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 88
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 113
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 127
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 126
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 121
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 108
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 91
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 64
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 36
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 9
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = -1
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = -4
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 5
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = -4
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 14
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 7
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 7
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 6
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 6
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 5
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 5
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 5
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 4
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 4
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 4
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 4
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 3
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 3
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 3
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 3
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 2
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 2
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 2
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 1

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=48)

    def BLE_2M_TX_Shaping_Coeffs(self, phy, model):
        # BLE2M Bandwidth Setting = Gear12
        # PGRAINIER-1908
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = -1
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 1
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = -3
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 19
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 25
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 52
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 81
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 107
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 123
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 127
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 125
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 117
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 103
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 79
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 52
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 28
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 15
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 9
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 12
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 3
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 17
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 10
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 10
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 9
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 8
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 7
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 6
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 6
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 5
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 5
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 4
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 4
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 3
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 3
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 3
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 3
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 2
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 1

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=48)

    def BLE_2M_AOX_TX_Shaping_Coeffs(self, phy, model):
        # not yet compensated for synth response on rainier. Currently a copy from Bobcat
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 26
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 45
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 63
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 80
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 96
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 99
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 87
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 70
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 53
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 37
        phy.profile_outputs.MODEM_SHAPING4_COEFF16.override = 21
        phy.profile_outputs.MODEM_SHAPING4_COEFF17.override = 13
        phy.profile_outputs.MODEM_SHAPING4_COEFF18.override = 9
        phy.profile_outputs.MODEM_SHAPING4_COEFF19.override = 8
        phy.profile_outputs.MODEM_SHAPING5_COEFF20.override = 7
        phy.profile_outputs.MODEM_SHAPING5_COEFF21.override = 6
        phy.profile_outputs.MODEM_SHAPING5_COEFF22.override = 5
        phy.profile_outputs.MODEM_SHAPING5_COEFF23.override = 5
        phy.profile_outputs.MODEM_SHAPING6_COEFF24.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF25.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF26.override = 4
        phy.profile_outputs.MODEM_SHAPING6_COEFF27.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF28.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF29.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF30.override = 3
        phy.profile_outputs.MODEM_SHAPING7_COEFF31.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF32.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF33.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF34.override = 2
        phy.profile_outputs.MODEM_SHAPING8_COEFF35.override = 2
        phy.profile_outputs.MODEM_SHAPING9_COEFF36.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF37.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF38.override = 1
        phy.profile_outputs.MODEM_SHAPING9_COEFF39.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF40.override = 1 # lynx goes to 0s here, xlsx uses 1s. Matching lynx
        phy.profile_outputs.MODEM_SHAPING10_COEFF41.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF42.override = 1
        phy.profile_outputs.MODEM_SHAPING10_COEFF43.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF44.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF45.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF46.override = 1
        phy.profile_outputs.MODEM_SHAPING11_COEFF47.override = 1

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=48)

    def BLE_TX_Shaping_Coeffs_IQMOD(self, phy, model):
        # to be used with IQMOD
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 2

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 6
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 15
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 30
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 52
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 76
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 97
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 112
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 120

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=8)

    def BLE_2M_TX_Shaping_Coeffs_IQMOD(self, phy, model):
        # to be used with IQMOD
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 2

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 6
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 15
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 30
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 52
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 76
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 97
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 112
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 120

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=8)

    def BLE_2M_TX_Shaping_Coeffs_2bt_IQMOD(self, phy, model):
        # to be used with IQMOD for bt = 2.0
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 2

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 16
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 111
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 127
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 127
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 127

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=8)

    def BLE_2M_AOX_TX_Shaping_Coeffs_IQMOD(self, phy, model):
        # to be used with IQMOD
        # Fb=2mbps, bw=2.5 MHz 2dB df2 boost from Optimized shaping coefficients for panther PLL_Ranga_MSW_v2.xlsx
        # This should be used with MODE4 = 3 MHz PLL BW (Column S)
        phy.profile_outputs.MODEM_CTRL0_SHAPING.override = 3

        phy.profile_outputs.MODEM_SHAPING0_COEFF0.override = 0
        phy.profile_outputs.MODEM_SHAPING0_COEFF1.override = 2
        phy.profile_outputs.MODEM_SHAPING0_COEFF2.override = 5
        phy.profile_outputs.MODEM_SHAPING0_COEFF3.override = 13
        phy.profile_outputs.MODEM_SHAPING1_COEFF4.override = 26
        phy.profile_outputs.MODEM_SHAPING1_COEFF5.override = 45
        phy.profile_outputs.MODEM_SHAPING1_COEFF6.override = 63
        phy.profile_outputs.MODEM_SHAPING1_COEFF7.override = 80
        phy.profile_outputs.MODEM_SHAPING2_COEFF8.override = 96
        phy.profile_outputs.MODEM_SHAPING2_COEFF9.override = 104
        phy.profile_outputs.MODEM_SHAPING2_COEFF10.override = 105
        phy.profile_outputs.MODEM_SHAPING2_COEFF11.override = 99
        phy.profile_outputs.MODEM_SHAPING3_COEFF12.override = 87
        phy.profile_outputs.MODEM_SHAPING3_COEFF13.override = 70
        phy.profile_outputs.MODEM_SHAPING3_COEFF14.override = 53
        phy.profile_outputs.MODEM_SHAPING3_COEFF15.override = 37

        self.override_other_shaping_coeff_to_zero(phy, start_coeff=16)
