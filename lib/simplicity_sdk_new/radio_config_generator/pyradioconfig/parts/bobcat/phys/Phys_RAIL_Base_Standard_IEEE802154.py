from pyradioconfig.parts.ocelot.phys.Phys_Internal_Base_Standard_IEEE802154 import PhysInternalBaseStandardIEEE802154Ocelot


class PhysRAILBaseStandardIEEE802154Bobcat(PhysInternalBaseStandardIEEE802154Ocelot):

    def _set_xtal_frequency(self, phy, xtal_freq=None):
        if xtal_freq is None:
            phy.profile_inputs.xtal_frequency_hz.value = 39000000
        else:
            phy.profile_inputs.xtal_frequency_hz.value = xtal_freq

    def IEEE802154_2p4GHz_base(self, phy, model):
        super().IEEE802154_2p4GHz_base(phy, model)

        # Set XO to 39M and use 2450M center freq (required for BWSEL, see MCUW_RADIO_CFG-2012)
        phy.profile_inputs.base_frequency_hz.value = int(2450000000)

        # RAIL timings
        phy.profile_outputs.rx_sync_delay_ns.override = 6625
        phy.profile_outputs.rx_eof_delay_ns.override = 6625

        #Set 802.15.4 protocol attribute
        model.vars.protocol_id.value_forced = model.vars.protocol_id.var_enum.Zigbee

    def IEEE802154_2p4GHz_cohdsa_base(self, phy, model):
        super().IEEE802154_2p4GHz_cohdsa_base(phy, model)

        # Set XO to 39M
        # phy.profile_inputs.xtal_frequency_hz.value = 39000000
        # self._set_xtal_frequency(phy)

        # RAIL timings
        phy.profile_outputs.rx_sync_delay_ns.override = 6125
        phy.profile_outputs.rx_eof_delay_ns.override = 6125

        # Set 802.15.4 protocol attribute
        model.vars.protocol_id.value_forced = model.vars.protocol_id.var_enum.Zigbee

    def PHY_IEEE802154_780MHz_OQPSK(self, model):
        pass

    def PHY_IEEE802154_868MHz_BPSK(self, model, phy_name=None):
        pass

    def PHY_IEEE802154_915MHz_BPSK_40kbps(self, model, phy_name=None):
        pass

    def PHY_IEEE802154_868MHz_BPSK_20kbps_coh(self, model):
        pass

    def PHY_IEEE802154_868MHz_OQPSK(self, model):
        pass

    #def PHY_IEEE802154_868MHz_OQPSK_coh(self, model):
        #pass

    def PHY_IEEE802154_915MHz_OQPSK(self, model):
        pass

    def PHY_IEEE802154_915MHz_OQPSK_coh(self, model):
        pass

    def PHY_IEEE802154_915MHz_BPSK_40kbps_coh(self, model):
        pass

    def PHY_IEEE802154_RSGFSK_868MHz_500kbps_mi0p76(self, model):
        pass

    def PHY_IEEE802154_2p4GHz(self, model,phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='Legacy IEEE 802.15.4 2p4GHz PHY from Jumbo',phy_name=phy_name)
        self.IEEE802154_2p4GHz_base(phy, model)
        phy.profile_inputs.demod_select.value = model.vars.demod_select.var_enum.LEGACY
        return phy

    # Owner     : Young-Joon Choi
    # Desc      : Antenna Diversity PHY with FEM (10 dB gain and 2 dB NF)
    # Jira Link : https://jira.silabs.com/browse/PGBOBCATVALTEST-207
    def PHY_IEEE802154_2p4GHz_diversity_fem(self, model, phy_name=None):
        phy = self.PHY_IEEE802154_2p4GHz_diversity(model, phy_name)
        phy.profile_inputs.antdiv_adpcsigampthr.value = 704
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.ANTDIV_FEM

    # Owner     : Young-Joon Choi
    # Desc      : Antenna Diversity PHY
    # Jira Link : https://jira.silabs.com/browse/PGBOBCATVALTEST-207
    def PHY_IEEE802154_2p4GHz_diversity(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 2p4GHz diversity', phy_name=phy_name)
        # : Start with IEEE802154 legacy demod phy. Then add antenna diversity settings
        self.IEEE802154_2p4GHz_base(phy, model)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.ANTDIV

        # : Disable phase demod path in order to improve frequency offset tolerance
        phy.profile_outputs.MODEM_CTRL1_PHASEDEMOD.override = 0

        # : Antenna diversity mode
        phy.profile_inputs.antdiv_enable_parallel_correlation.value = True
        phy.profile_inputs.antdivmode.value = model.vars.antdivmode.var_enum.ANTSELCORR
        phy.profile_inputs.antdivrepeatdis.value = model.vars.antdivrepeatdis.var_enum.NOREPEATFIRST

        # : Antenna diversity skip and delay
        phy.profile_inputs.antdiv_switch_delay_us.value = 4.0
        phy.profile_inputs.antdiv_switch_skip_us.value = 2.0

        # : Antenna diversity controls
        phy.profile_outputs.MODEM_ADPC2_ADENCORR32.override = 1
        phy.profile_outputs.MODEM_CTRL0_DUALCORROPTDIS.override = 1
        phy.profile_outputs.MODEM_ADPC2_ADPCWNDCNTRST.override = 0

        # : AGC settings
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 3 # Change PN gain step for improved WiFi blocker performance
        phy.profile_outputs.AGC_ANTDIV_GAINMODE.override = 1
        phy.profile_outputs.AGC_GAINSTEPLIM0_CFLOOPDEL.override = 40

        # : Timing and preamble thresholds
        phy.profile_outputs.MODEM_TIMING_TIMTHRESH.override = 90
        phy.profile_outputs.MODEM_CTRL6_TIMTHRESHGAIN.override = 1

        # : Antenna diversity correlation decision settings
        phy.profile_inputs.antdiv_adpcsigampthr.value = 288
        phy.profile_outputs.MODEM_CTRL5_LINCORR.override = 1
        phy.profile_outputs.MODEM_ADQUAL6_ADBACORRTHR.override = 0
        phy.profile_outputs.MODEM_ADQUAL8_ADBACORRTHR2.override = 0
        phy.profile_outputs.MODEM_ADQUAL6_ADBACORRDIFF.override = 500
        phy.profile_outputs.MODEM_ADQUAL5_ADDIRECTCORR.override = 65535

        # : Antenna diversity RSSI decision settings
        phy.profile_outputs.MODEM_ADQUAL7_ADBARSSITHR.override = 512
        phy.profile_outputs.MODEM_ADQUAL7_ADBARSSIDIFF.override = 0
        phy.profile_outputs.MODEM_ADQUAL4_ADRSSIGRTHR.override = 512

        # : Antenna diversity BBSS settings
        phy.profile_outputs.MODEM_ADPC3_ADBBSSEN.override = 0
        phy.profile_outputs.MODEM_ADPC3_ADBBSSAMPEXP.override = 11
        phy.profile_outputs.MODEM_ADPC3_ADBBSSAMPMANT.override = 0
        phy.profile_outputs.MODEM_ADPC3_ADBBSSAVGEN.override = 1
        phy.profile_outputs.MODEM_ADPC3_ADBBSSFILTLENGTH.override = 0
        phy.profile_outputs.MODEM_ADPC3_ADBBSSAVGPER.override = 2
        phy.profile_outputs.MODEM_ADPC3_ADBBSSAVGWAIT.override = 0
        phy.profile_outputs.MODEM_ADPC3_ADBBSSAVGFREEZE.override = 1

        # Antenna Diversity settings
        phy.profile_outputs.MODEM_ADQUAL4_ADAGCGRTHR.override = 63
        phy.profile_outputs.MODEM_ADQUAL8_ADBAAGCTHR.override = 0
        phy.profile_outputs.MODEM_ADQUAL4_ADGRMODE.override = 0
        phy.profile_outputs.MODEM_ADQUAL8_ADBAMODE.override = 1

        # : For debug: Statistic selection
        phy.profile_outputs.MODEM_ADFSM0_ADSTAT1SEL.override = 6

        # 1 window 48
        phy.profile_outputs.MODEM_ADPC8_ADPCOSR.override = 5
        phy.profile_outputs.MODEM_ADPC8_ADPCANTSAMPOFFSET.override = 3

        phy.profile_outputs.MODEM_CTRL6_ARW.override = 1
        phy.profile_outputs.MODEM_TIMING_ADDTIMSEQ.override = 0

        phy.profile_outputs.AGC_CTRL1_RSSIPERIOD.override = 4

        return phy

    # Owner     : Young-Joon Choi
    # Desc      : Antenna Diversity PHY RSSI based decision
    # Jira Link : https://jira.silabs.com/browse/MCUW_RADIO_CFG-2358
    def PHY_IEEE802154_2p4GHz_diversity_RSSI(self, model, phy_name=None):
        phy = self.PHY_IEEE802154_2p4GHz_diversity(model, phy_name='PHY_IEEE802154_2p4GHz_diversity_RSSI')
        phy.profile_outputs.MODEM_ADQUAL8_ADBAMODE.override = 3 # : RSSI based decision only
        # : Observing unexpected gain changes even if gain restore option is enabled.
        # : Reduce impact on RSSI measurement by reducing RSSI period and increasing debouncecntthd for better AGC convergence
        phy.profile_outputs.AGC_ANTDIV_DEBOUNCECNTTHD.override = 80
        phy.profile_outputs.AGC_CTRL1_RSSIPERIOD.override = 3
        return phy

    # Owner     : Young-Joon Choi
    # Jira Link : https://jira.silabs.com/browse/PGBOBCATVALTEST-1
    def PHY_IEEE802154_2p4GHz_cohdsa(self, model, phy_name=None):
        phy = self._makePhy(model, model.profiles.Base, readable_name='802154 2p4GHz cohdsa', phy_name=phy_name)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.COHERENT
        self.IEEE802154_2p4GHz_cohdsa_base(phy, model)

        # : Change PN gain step for improved WiFi blocker performance
        phy.profile_outputs.AGC_GAINRANGE_PNGAINSTEP.override = 3

        # : Increase dsa threshold to remove floor issues at high signal power (> -40 dBm)
        phy.profile_outputs.MODEM_LONGRANGE6_LRSPIKETHD.override = 115

        # MCUW_RADIO_CFG-1809: miwu: Minimize delay + average for dynamic threshold to avoid undesired correlation gain.
        phy.profile_outputs.MODEM_LONGRANGE1_AVGWIN.override = 0
        phy.profile_outputs.MODEM_COH2_FIXEDCDTHFORIIR.override = 125
        phy.profile_outputs.MODEM_LONGRANGE6_LRCHPWRSPIKETH.override = 35

        return phy

    def PHY_IEEE802154_2p4GHz_cohdsa_diversity(self, model, phy_name=None):
        pass

    # Owner       : Young-Joon Choi
    # Description : IEEE802154 PHY optimized for FEM with 10 dB gain and 2 dB NF
    def PHY_IEEE802154_2p4GHz_cohdsa_fem(self, model, phy_name=None):
        phy = self.PHY_IEEE802154_2p4GHz_cohdsa(model, phy_name)
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.FEM

        fem_chpwr_adjustment = 6

        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH1.override = 20 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH2.override = 26 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH3.override = 33 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE2_LRCHPWRTH4.override = 40 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH5.override = 46 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH6.override = 52 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH7.override = 59 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE3_LRCHPWRTH8.override = 66 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH9.override = 73 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRTH10.override = 80 + fem_chpwr_adjustment

        phy.profile_outputs.MODEM_COH0_COHCHPWRTH0.override = 33 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH1.override = 40 + fem_chpwr_adjustment
        phy.profile_outputs.MODEM_COH0_COHCHPWRTH2.override = 100 + fem_chpwr_adjustment

        fem_bbss_adjustment = 1

        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH1.override = 3 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH2.override = 4 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH3.override = 5 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE4_LRCHPWRSH4.override = 5 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH5.override = 6 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH6.override = 7 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH7.override = 8 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH8.override = 9 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH9.override = 10 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH10.override = 11 + fem_bbss_adjustment
        phy.profile_outputs.MODEM_LONGRANGE5_LRCHPWRSH11.override = 12 + fem_bbss_adjustment

        return phy

    # Description : IEEE802154 PHY with "fast switching" feature intended to be exposed to customer by RAIL
    def PHY_IEEE802154_2p4GHz_antdiv_fastswitch(self, model, phy_name=None):
        phy = self.PHY_IEEE802154_2p4GHz_diversity(model, phy_name='PHY_IEEE802154_2p4GHz_antdiv_fastswitch')
        model.vars.zigbee_feature.value_forced = model.vars.zigbee_feature.var_enum.FCS

        # : RAIL will set the following regs fields
        # 'RAC.RX.SYCHPBIASTRIMBUFRX' 1,
        # 'RAC.RX.SYPFDCHPLPENRX' 0,
        # 'RAC.RX.SYPFDFPWENRX': 0,

        # : Antenna diversity mode
        phy.profile_inputs.antdiv_enable_parallel_correlation.value = False
        phy.profile_inputs.antdivmode.value = model.vars.antdivmode.var_enum.ANTSELFIRST
        phy.profile_inputs.antdivrepeatdis.value = model.vars.antdivrepeatdis.var_enum.NOREPEATFIRST

        phy.profile_inputs.synth_settling_mode.value = model.vars.synth_settling_mode.var_enum.FAST

        phy.profile_outputs.MODEM_ANTDIVCTRL_ENADPRETHRESH.override = 0x1
        phy.profile_outputs.MODEM_ANTDIVCTRL_ADPRETHRESH.override = 0x12
        phy.profile_outputs.MODEM_ADCTRL1_ADCTRL1.override = 1074331649
        phy.profile_outputs.MODEM_ADCTRL2_ADCTRL2.override = 1050625
        phy.profile_outputs.MODEM_ADPC1_ADPCWNDCNT.override = 2
        phy.profile_outputs.MODEM_ADPC1_ADPCTIMINGBAUDS.override = 0x40
        phy.profile_outputs.MODEM_ADPC2_ADPCCORRSAMPLES.override = 0x140
        phy.profile_outputs.MODEM_ADPC8_ADPCANTSAMPSWITCH.override = 0xb4
        phy.profile_outputs.MODEM_ADPC8_ADPCANTSAMPWRITE.override = 0x3c
        phy.profile_outputs.MODEM_ADPC8_ADPCANTSAMPBUF.override = 0x28

        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG2.override = 4
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG1.override = 2
        phy.profile_outputs.MODEM_INTAFC_FOEPREAVG0.override = 1

        phy.profile_outputs.MODEM_CTRL6_TDREW.override = 0x20

        phy.profile_outputs.MODEM_TIMING_TIMINGBASES.override = 2

        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT0.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT1.override = 1
        phy.profile_outputs.MODEM_CTRL6_PSTIMABORT2.override = 1
        phy.profile_outputs.MODEM_CTRL5_LINCORR.override = 0

        phy.profile_outputs.MODEM_DCCOMP_DCCOMPGEAR.override = 7
        phy.profile_outputs.MODEM_RXRESTART_ANTSWRSTFLTTDIS.override = 0
        phy.profile_outputs.AGC_AGCPERIOD0_PERIODHI.override = 7
        phy.profile_outputs.AGC_AGCPERIOD1_PERIODLOW.override = 21
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION0.override = 4
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION1.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION2.override = 6
        phy.profile_outputs.AGC_HICNTREGION0_HICNTREGION3.override = 7
        phy.profile_outputs.AGC_HICNTREGION1_HICNTREGION4.override = 7

        phy.profile_outputs.RAC_SYTRIM1_SYTRIMMMDREGAMPBW.override = 0
        phy.profile_outputs.RAC_SYTRIM1_SYTRIMMMDREGAMPBIAS.override = 3

        return phy

    def PHY_Internal_IEEE802154_915MHz_cohdsa(self, model, phy_name=None):
        pass

    def PHY_Internal_IEEE802154_915MHz_cohdsa_antdiv(self, model, phy_name=None):
        pass