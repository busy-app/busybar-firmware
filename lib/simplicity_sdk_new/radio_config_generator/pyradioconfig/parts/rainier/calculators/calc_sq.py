from pyradioconfig.parts.bobcat.calculators.calc_sq import Calc_SQ_Bobcat
from math import ceil


class CalcSqRainier(Calc_SQ_Bobcat):

    def calc_sq_timeout1_timeout2(self, model):
        #This function calculates the PSM timeout1 and timeout2 depending on demod configuration

        #Read in model variables
        demod_select = model.vars.demod_select.value
        sq_enabled = (model.vars.MODEM_SQ_SQEN.value == 1)
        bits_per_symbol = model.vars.bits_per_symbol_actual.value
        delay_adc_to_demod_symbols = model.vars.delay_adc_to_demod_symbols.value
        delay_adc_to_demod_bits = delay_adc_to_demod_symbols * bits_per_symbol
        pmcostvalthd = model.vars.MODEM_TRECPMDET_PMCOSTVALTHD.value
        baudrate = model.vars.baudrate.value


        #Only calculate timeouts if fast detect is enabled
        if sq_enabled:
            if demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI or demod_select == model.vars.demod_select.var_enum.TRECS_SLICER:
                trecs_pmacqingwin_actual = model.vars.pmacquingwin_actual.value
                trecs_pmdet_enabled = model.vars.MODEM_PHDMODCTRL_PMDETEN.value

                #CFE used for DSA detect
                min_preamble_bits = trecs_pmacqingwin_actual

                # : delay due to number of valid preamble counts required before dsa detect
                if pmcostvalthd > 1:
                    detection_delay = 2 * pmcostvalthd
                else:  # : use at least 4 delay symbols based on sweep of MI and datarates
                    detection_delay = 4

                # : sqtimout_buffer based on experimental sweep of MI and datarates. Detection time is delayed
                # : if the signal level is near sensitivity level.
                sqtimout_buffer = 5
                sqtimout = int(ceil(sqtimout_buffer + detection_delay + delay_adc_to_demod_bits + min_preamble_bits))

                if trecs_pmdet_enabled:
                    pmdetthd = model.vars.MODEM_PHDMODCTRL_PMDETTHD.value
                    # Between timeout1 and timeout2 we need pmdetthd bits
                    # Always use a value of at least 2 as 0 means "wait forever"
                    sqstg2timout = max(2,pmdetthd-min_preamble_bits)
                else:
                    sqstg2timout = 0 #No separate preamble detect step

                # : RX Duty cycle on time ideally should be linear to SQTIMEOUT values but has an offset.
                # Also, there is a minimum possible value for RX Duty cycle on time because of
                # rxsearch_irqhandler completion + isr latency
                # : Above can be seen in SQTIMEOUTmin.png is https://jira.silabs.com/browse/MCUW_RADIO_CFG-1994
                # NOTE: at the time since this experiment was done only for TRECS, updating TRECS calculation only

                min_rx_duty_cucle_ontime_us = 20
                sqtimeoutmin = ceil(int(min_rx_duty_cucle_ontime_us * 1e-6 * baudrate))

                sqtimout = max(sqtimeoutmin, sqtimout)

            elif demod_select == model.vars.demod_select.var_enum.LEGACY:
                sqtimout = 0
                sqstg2timout = 0

            # For Coherent Demod, only specific Longrange PHYs were tested as per MCUW_RADIO_CFG-1858
            # So doing sqtimout and sqstg2timout overrides in Longrange_mode profile (_lookup_from_longrange_mode)
            # Not yet calculating for other demods
            else:
                sqtimout = 0
                sqstg2timout = 0
        else:
            #Fast detect not enabled
            sqtimout = 0
            sqstg2timout = 0

        #Write the registers
        self._reg_write(model.vars.MODEM_SQ_SQTIMOUT, sqtimout, check_saturation=True)
        self._reg_write(model.vars.MODEM_SQEXT_SQSTG2TIMOUT, sqstg2timout, check_saturation=True)

    def calc_sqen_reg(self, model):
        # Overridden to accomodate duty cycle (with noise detector)

        #Read in model variables
        fast_detect_enable = (model.vars.fast_detect_enable.value == model.vars.fast_detect_enable.var_enum.ENABLED)
        duty_cycled = model.vars.rxdc_power_save_mode.value != model.vars.rxdc_power_save_mode.var_enum.DISABLED
        symbol_encoding = model.vars.symbol_encoding.value
        demod_select = model.vars.demod_select.value
        hop_enabled = model.vars.hop_enable.value

        supported_demod = (demod_select == model.vars.demod_select.var_enum.TRECS_VITERBI) or \
                          (demod_select == model.vars.demod_select.var_enum.TRECS_SLICER) or \
                          (demod_select == model.vars.demod_select.var_enum.ENHANCED_DSSS)
        if duty_cycled and not hop_enabled:
            # We need to disable SQ for duty cycling (no hopping), else NOISEDET interrupt will trigger upon SQTIMEOUT
            sqen_reg = 0
        elif hop_enabled:
            # If hopping is enabled, we want SQ timeout to prevent overstay on one channel
            sqen_reg = 1
        elif fast_detect_enable:
            # Enable the signal qualifier when fast timing detection is enabled and we are using a supported demod
            if supported_demod:
                sqen_reg = 1
            else:
                LogMgr.Warning("Fast detect is currently only supported with 2FSK PHYs in the Base Profile and DSSS PHYs in LongRange Profile")
                sqen_reg = 0
        else:
            sqen_reg = 0

        #Write the register
        self._reg_write(model.vars.MODEM_SQ_SQEN, sqen_reg)
