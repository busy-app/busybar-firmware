from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from enum import Enum
from pyradioconfig.parts.bobcat.calculators.calc_synth import Calc_Synth_Bobcat
from enum import Enum
from pyradioconfig.calculator_model_framework.Utils.CustomExceptions import CalculationException
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
class CalcSynthRainier(Calc_Synth_Bobcat):

    def buildVariables(self, model):
        """Populates a list of needed variables for this calculator
        Args:
            model (ModelRoot) : Builds the variables specific to this calculator
        """

        super().buildVariables(model)

        self._addModelVariable(model, 'adc_constrain_xomult', bool, ModelVariableFormat.DECIMAL,
                               desc='Flag used internally to constrain ADC clock to multiple of HFXO')

        # : Modify tx synth modes from Bobcat
        model.vars.synth_tx_mode.var_enum = CreateModelVariableEnum(
            enum_name='SynthTxModeEnum',
            enum_desc='Defined Synth TX Mode',
            member_data=[
                ['MODE1', 0, 'TX Mode 1'],           # 750KHz one side
                ['MODE2', 1, 'TX Mode 2'],           # 1MHZ, one side
                ['MODE3', 2, 'TX Mode 3'],           # 1.5MHZ, one side
                ['MODE4', 3, 'TX Mode 4'],           # 2.5MHz, one side
                ['MODE_IQMOD', 0, 'TX IQMOD Mode'],   # IQMOD
                ['MODE_BLE', 0, 'TX BLE Mode'],      # BLE1M
                ['MODE_BLE_FULLRATE', 4, 'TX BLE Fullrate Mode'], # BLE2M
                ['MODE_IEEE802154', 2, 'TX IEEE802154 Mode'], # Zigbee
            ])
        model.vars.synth_tx_mode_actual.var_enum = model.vars.synth_tx_mode.var_enum

        # : Modify synth RX modes from Ocelot
        model.vars.synth_rx_mode.var_enum = CreateModelVariableEnum(
            enum_name='SynthRxModeEnum',
            enum_desc='Defined Synth RX Mode',
            member_data=[
                ['MODE1', 0, 'RX Mode 1'],
                ['MODE2', 1, 'RX Mode 2'],
                ['MODE_HOP', 2, 'RX Mode Hopping'],
            ])
        model.vars.synth_rx_mode_actual.var_enum = model.vars.synth_rx_mode.var_enum

        var = self._addModelVariable(model, 'hop_enable', Enum, ModelVariableFormat.DECIMAL, units='',
                                     desc='Enables Hopping')

        member_data = [
            ['DISABLED', 0, 'Hopping Disabled'],
            ['ENABLED', 1, 'Hopping Enabled'],
        ]

        var.var_enum = CreateModelVariableEnum(
            'HopEnableEnum',
            'HOP Enable/Disable Selection',
            member_data)

    def calc_ifadcenhalfmode_reg(self, model):

        adc_rate_mode = model.vars.adc_rate_mode.value

        if adc_rate_mode == model.vars.adc_rate_mode.var_enum.HALFRATE:
            reg = 1
        else:
            reg = 0

        self._reg_write(model.vars.RAC_ADCCTRL1_ADCENHALFMODE, reg)

    def calc_synth_settling_mode(self, model):
        hop_enable = model.vars.hop_enable.value        # Disable by default

        if hop_enable == model.vars.hop_enable.var_enum.ENABLED:
            model.vars.synth_settling_mode.value = model.vars.synth_settling_mode.var_enum.FAST
        else:       # FastSw Disabled
            model.vars.synth_settling_mode.value = model.vars.synth_settling_mode.var_enum.NORMAL

    def calc_adc_clockmode_reg(self, model):
        adc_clock_mode_actual = model.vars.adc_clock_mode.value

        if adc_clock_mode_actual == model.vars.adc_clock_mode.var_enum.VCODIV:
            reg = 0
        else:
            reg = 1
        self._reg_write(model.vars.RAC_ADCCTRL0_ADCCLKSEL, reg)

    def calc_adc_clockmode_actual(self, model):
        #This function calculates the actual value of the adc clock mode based on the register value used
        ifadc_clk_sel = model.vars.RAC_ADCCTRL0_ADCCLKSEL.value

        if( 1 == ifadc_clk_sel ):
            model.vars.adc_clock_mode_actual.value = model.vars.adc_clock_mode.var_enum.HFXOMULT
        else:
            model.vars.adc_clock_mode_actual.value = model.vars.adc_clock_mode.var_enum.VCODIV

    def calc_adc_freq_actual(self,model):
        #This function calculates the actual ADC sample frequency and error based on the registers

        #Load model variables into local variables
        adc_clock_mode_actual = model.vars.adc_clock_mode_actual.value
        adc_vco_div_actual = model.vars.adc_vco_div_actual.value
        xtal_frequency_hz = model.vars.xtal_frequency_hz.value
        fsynth = model.vars.rx_synth_freq_actual.value
        fadc_target = model.vars.adc_target_freq.value #The target frequency is used for computing error
        ifadc_halfrate = model.vars.RAC_ADCCTRL1_ADCENHALFMODE.value

        if adc_clock_mode_actual == model.vars.adc_clock_mode.var_enum.HFXOMULT:
            #the ordering of the if statements is important - keep ifadcpll_en_xo_bypass on top
            if 1 == ifadc_halfrate:
                adc_freq_actual = xtal_frequency_hz * 4
            else:
                adc_freq_actual = xtal_frequency_hz * 8
        else:
            adc_freq_actual = int(fsynth / adc_vco_div_actual)

        # Compute the final ADC frequency percent error
        ferror = 100 * (fadc_target - adc_freq_actual) / float(fadc_target)

        #Load local variables back into model variables
        model.vars.adc_freq_actual.value = adc_freq_actual
        model.vars.adc_freq_error.value = ferror

    def calc_clkmult_div_reg(self, model):
        adc_clock_mode_actual = model.vars.adc_clock_mode_actual.value
        ifadc_halfrate = model.vars.RAC_ADCCTRL1_ADCENHALFMODE.value

        if adc_clock_mode_actual == model.vars.adc_clock_mode.var_enum.HFXOMULT:
            if ifadc_halfrate == 0:
                # adc_full_speed from dpll_utils.py (xo * 8); 8 = 48 / (3 * 2)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTDIVR, 1)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTDIVN, 48)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTDIVX, 3)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTFREQCAL, 1)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTBWCAL, 1)
            else:
                # adc_half_speed from dpll_utils.py (xo * 4); 4 = 40 / (5 * 2)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTDIVR, 1)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTDIVN, 40)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTDIVX, 5)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTFREQCAL, 0)
                self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTBWCAL, 0)
        else:
            # reset values
            self._reg_write_default(model.vars.RAC_CLKMULTEN1_CLKMULTDIVR)
            self._reg_write_default(model.vars.RAC_CLKMULTEN1_CLKMULTDIVN)
            self._reg_write_default(model.vars.RAC_CLKMULTEN1_CLKMULTDIVX)
            self._reg_write_default(model.vars.RAC_CLKMULTEN1_CLKMULTFREQCAL)
            self._reg_write_default(model.vars.RAC_CLKMULTEN1_CLKMULTBWCAL)

    def calc_clkmulten_tx_reg(self, model):
        # issue here is that the dpll does not have TX/RX registers for all settings (en_bbdet, etc)
        # HFXOBYP still has the DPLL loop running, but just bypasses in the final R divider, causing
        # higher current. Acceptable for Rainier, but needs to be optimized in Everest
        dac_clock_mode_actual = model.vars.dac_clock_mode.value

        if dac_clock_mode_actual == model.vars.dac_clock_mode.var_enum.DISABLED:
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTENBYPASS40MHZTX, 0)
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTDIVNTX, 64)
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTDIVRTX, 0)
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTDIVXTX, 0)
        elif dac_clock_mode_actual == model.vars.dac_clock_mode.var_enum.HFXOBYP:
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTENBYPASS40MHZTX, 1)
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTDIVNTX, 64)
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTDIVRTX, 1)
            self._reg_write(model.vars.RAC_CLKMULTEN2_CLKMULTDIVXTX, 0)
        elif dac_clock_mode_actual == model.vars.dac_clock_mode.var_enum.HFXOMULT:
            raise NotImplementedError("CLKMULT TX in non-bypass mode is not yet supported")
        else:
            raise RuntimeError(f"Invalid dac clock mode {dac_clock_mode_actual}")

    def calc_clkmulten_reg(self, model):
        # TODO: not sure if these are to be set by us or rail as they are state dependent,
        #  check values from the RDM output file

        adc_clock_mode_actual = model.vars.adc_clock_mode_actual.value

        # To enable clkmult the following additional registers must also be set, but are handled in RAIL
        # SYXO0.INTERNALCTRL.ENCLKMULTANA = 1 # enable XO output to CLKMULT
        # RAC.SYLOEN.SYLODIVRLO2P4GENEN = 0 # disable LODIV output buffer from SYLODIV (power saving)

        if adc_clock_mode_actual == model.vars.adc_clock_mode.var_enum.HFXOMULT:
            # unless otherwise specified, the values are taken from dualbclk_mult_validation_20190516_lynx_revA0.pptx > dpll_utils.py > dualbclk_mult spec sheet
            # based on the common settings for adc_full_speed, adc_full_speed_lp, adc_half_speed, adc_half_speed_lp
            # in dpll_utils.py
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENDITHER, 0)
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVN, 0)
            # self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVRX2P4G, 0)
            # self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVRXSUBG, 0)
            # self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVTXDUALB, 0)
            # self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENREG3, 1)
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTENBYPASS40MHZ, 0)

            self._reg_write(model.vars.RAC_CLKMULTEN1_CLKMULTINNIBBLE, 8)
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTLDFNIB, 0)
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTLDMNIB, 0)
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTRDNIBBLE, 3)
            self._reg_write(model.vars.RAC_CLKMULTEN0_CLKMULTLDCNIB, 0)

            self._reg_write(model.vars.RAC_CLKMULTCTRL_CLKMULTENRESYNC, 0)
            self._reg_write(model.vars.RAC_CLKMULTCTRL_CLKMULTVALID, 0)
        else:
            # when using lodiv, turn off dualbclk_mult to reset values
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENDITHER)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVN)
            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVRX2P4G)
            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVRXSUBG)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENDRVTXDUALB)
            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENREG3)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTENBYPASS40MHZ)

            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTREG1ADJV)
            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTREG2ADJV)
            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTREG2ADJI)
            # self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTREG3ADJV)

            self._reg_write_default(model.vars.RAC_CLKMULTEN1_CLKMULTINNIBBLE)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTLDFNIB)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTLDMNIB)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTRDNIBBLE)
            self._reg_write_default(model.vars.RAC_CLKMULTEN0_CLKMULTLDCNIB)

            self._reg_write_default(model.vars.RAC_CLKMULTCTRL_CLKMULTENRESYNC)
            self._reg_write_default(model.vars.RAC_CLKMULTCTRL_CLKMULTVALID)

    def calc_rx_mode_reg(self, model):

        rx_mode = model.vars.synth_rx_mode.value
        ind = rx_mode.value

        # Synth settings https://jira.silabs.com/browse/MCUW_RADIO_CFG-1529
        # Settings copied over from Lynx Assert
        # {workspace}\shared_files\lynx\radio_validation\ASSERTS
        # BLK_SYNTH_RX_LP_BW_200KHZ.csv (BLE_LR mode)
        # BLK_SYNTH_RX_LP_BW_250KHZ.csv (NORMAL mode)

        rx_mode_settings = {
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR2': [15, 15, 15],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR3': [11, 11, 12],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR4': [11, 11, 12],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR5': [11, 11, 12],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR6': [7, 7, 10],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR7': [7, 7, 10],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR8': [7, 7, 10],
            'SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR9': [6, 6, 8],
            'SYNTH.DSMCTRLRX.LSBFORCERX': [1, 1, 1],
            'SYNTH.DSMCTRLRX.DEMMODERX': [1, 1, 1],
            'SYNTH.DSMCTRLRX.QNCMODERX': [0, 0, 1],
            'SYNTH.DSMCTRLRX.GLMSOVERRIDEVALRX': [840, 840, 840],

           # 'RAC.SYMMDCTRL.SYMMDMODERX': [4, 4],
           #'RAC.SYTRIM1.SYLODIVLDOTRIMNDIORX': [1, 1],
        }
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR2,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR2'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR3,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR3'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR4,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR4'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR5,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR5'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR6,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR6'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR7,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR7'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR8,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR8'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLRX_RXLOCKLPFBWGEAR9,
                        rx_mode_settings['SYNTH.DLFCTRLRX.RXLOCKLPFBWGEAR9'][ind])
        self._reg_write(model.vars.SYNTH_DSMCTRLRX_QNCMODERX,
                        rx_mode_settings['SYNTH.DSMCTRLRX.QNCMODERX'][ind])
        self._reg_write(model.vars.SYNTH_DSMCTRLRX_GLMSOVERRIDEVALRX,
                        rx_mode_settings['SYNTH.DSMCTRLRX.GLMSOVERRIDEVALRX'][ind])

        # : Following registers are PTE Set & Forget but needs to be set by RC since they are different from reset value
        # : See https://jira.silabs.com/browse/MCUW_RADIO_CFG-1610
        self._reg_write(model.vars.SYNTH_DSMCTRLRX_LSBFORCERX, rx_mode_settings['SYNTH.DSMCTRLRX.LSBFORCERX'][ind])
        self._reg_write(model.vars.SYNTH_DSMCTRLRX_DEMMODERX, rx_mode_settings['SYNTH.DSMCTRLRX.DEMMODERX'][ind])

    def calc_tx_mode(self, model):
        modulator_select = model.vars.modulator_select.value
        baudrate = model.vars.baudrate.value
        modulation_index = model.vars.modulation_index.value

        if modulator_select == model.vars.modulator_select.var_enum.IQ_MOD:
            model.vars.synth_tx_mode.value = model.vars.synth_tx_mode.var_enum.MODE1      #750KHz,one side
        # Set FSK and OQPSK settings based on baudrate
        else:
            if baudrate > 1500e3 and modulation_index > 0.5:
                # to fix eye diagram for 2mbps1Mhz datasheet PHY - https://jira.silabs.com/browse/MCUW_RADIO_CFG-2548
                model.vars.synth_tx_mode.value = model.vars.synth_tx_mode.var_enum.MODE4  # 2.5MHz, one side
            elif baudrate > 1000e3:
                model.vars.synth_tx_mode.value = model.vars.synth_tx_mode.var_enum.MODE3  # 1.5MHz, one side
            elif baudrate > 500e3:
                model.vars.synth_tx_mode.value = model.vars.synth_tx_mode.var_enum.MODE2  # 1 MHz, one side
            else:
                model.vars.synth_tx_mode.value = model.vars.synth_tx_mode.var_enum.MODE1  # 750KHz, one side

    def calc_tx_mode_reg(self, model):
        ind = model.vars.synth_tx_mode.value

        # Synth settings coming from
        # file://silabs.com/design/projects/ip_em_22nm/tsmc22ull/modules/blesy_all/docs/specs/blesy_all.pdf
        # 750KHz one side (MODE 1), to be used for IQMOD and BLE1M PHY inband requirements
        # 1MHz one side (MODE 2)
        # 1.5MHz one side (MODE 3)
        # 2.5MHz one side (MODE 4)
        # 2 MHz one side (MODE 5), to be used for BLE2M PHY due to inband requirements

        tx_mode_settings = {
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR2': [15, 15, 15, 15, 15],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR3': [12, 12, 14, 14, 14],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR4': [12, 12, 14, 14, 14],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR5': [12, 12, 14, 14, 14],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR6': [10, 10, 13, 13, 13],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR7': [10, 10, 13, 13, 13],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR8': [10, 10, 13, 13, 13],
            'SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR9': [9, 10, 11, 13, 12],
            'SYNTH.DLFCTRL.LOCKLPFBWGEARSLOT': [1, 1, 1, 1, 1],
            'SYNTH.DLFCTRL.LPFBWLOADDEL': [1, 1, 1, 1, 1],

            'SYNTH.DSMCTRLTX.LSBFORCETX': [1, 1, 1, 1, 1],
            'SYNTH.DSMCTRLTX.DEMMODETX': [1, 1, 1, 1, 1],

         #   'RAC_SYCTRL1_SYLODIVSELFP4G82G4TX': [1, 0, 0, 0],   #fp select, depend on TX power, move to rail code
         #   'RAC_SYMMDCTRL_SYMMDSEL56STGTX': [1, 0, 0, 0],   #TX IQMOD select 6 stage
        }

        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR2,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR2'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR3,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR3'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR4,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR4'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR5,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR5'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR6,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR6'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR7,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR7'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR8,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR8'][ind])
        self._reg_write(model.vars.SYNTH_DLFCTRLTX_TXLOCKLPFBWGEAR9,
                        tx_mode_settings['SYNTH.DLFCTRLTX.TXLOCKLPFBWGEAR9'][ind])

        # : Following registers are PTE Set & Forget but needs to be set by RC since they are different from reset value
        # : See https://jira.silabs.com/browse/MCUW_RADIO_CFG-1610
        self._reg_write(model.vars.SYNTH_DSMCTRLTX_LSBFORCETX, tx_mode_settings['SYNTH.DSMCTRLTX.LSBFORCETX'][ind])
        self._reg_write(model.vars.SYNTH_DSMCTRLTX_DEMMODETX, tx_mode_settings['SYNTH.DSMCTRLTX.DEMMODETX'][ind])



    def calc_dlf_ctrl(self, model):
        self._reg_write(model.vars.SYNTH_DLFCTRL_LOCKLPFBWGEARSLOT, 3)
        self._reg_write(model.vars.SYNTH_DLFCTRL_LPFBWLOADDEL, 1)
        self._reg_write(model.vars.SYNTH_DLFCTRL_LPFBWDURINGHOP, 15)
        self._reg_write(model.vars.SYNTH_DLFCTRL_LPFBWAFTERHOP, 8)

    def calc_fcal_reg(self, model):
        self._reg_write(model.vars.SYNTH_FCALCTRL_FCALMODE, 1)
        ## NUMCYCLE
        self._reg_write(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE, 5)
        self._reg_write(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE1, 5)
        self._reg_write(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE2, 5)
        self._reg_write(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE3, 4)
        self._reg_write(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE4, 4)
        self._reg_write(model.vars.SYNTH_LOCNTCTRL_NUMCYCLE5, 4)
        self._reg_write(model.vars.SYNTH_FCALCTRL_NUMCYCLE6, 4)
        self._reg_write(model.vars.SYNTH_FCALCTRL_NUMCYCLE7, 4)
        self._reg_write(model.vars.SYNTH_FCALCTRL_NUMCYCLE8, 3)
        self._reg_write(model.vars.SYNTH_FCALCTRL_NUMCYCLE9, 3)
        self._reg_write(model.vars.SYNTH_FCALCTRL_NUMCYCLE10, 2)
        ## COMPANION
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION0, 1)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION1, 1)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION2, 2)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION3, 2)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION4, 2)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION5, 4)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION6, 4)
        self._reg_write(model.vars.SYNTH_COMPANION_COMPANION7, 5)
        ## STEPWAIT
        self._reg_write(model.vars.SYNTH_FCALCTRL_STEPWAIT, 0)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT1, 0)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT2, 0)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT3, 1)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT4, 1)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT5, 2)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT6, 2)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT7, 3)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT8, 3)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT9, 4)
        self._reg_write(model.vars.SYNTH_FCALSTEPWAIT_STEPWAIT10, 4)

    def calc_lms_reg(self, model):
        self._reg_write(model.vars.SYNTH_GLMS_GLMSENABLEDELAY, 7)
        self._reg_write(model.vars.SYNTH_GLMS_GLMSGEAR0, 10)
        self._reg_write(model.vars.SYNTH_GLMS_GLMSGEAR1, 10)
        self._reg_write(model.vars.SYNTH_GLMS_GLMSGEAR2, 9)
        self._reg_write(model.vars.SYNTH_GLMS_GLMSGEAR3, 7)
        self._reg_write(model.vars.SYNTH_GLMS_GLMSGEAR4, 6)
        self._reg_write(model.vars.SYNTH_GLMS_GLMSGEARSLOT, 0)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSENABLEDELAY, 7)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSGEAR0, 11)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSGEAR1, 10)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSGEAR2, 9)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSGEAR3, 8)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSGEAR4, 6)
        self._reg_write(model.vars.SYNTH_PLMS_PLMSGEARSLOT, 0)

    def calc_s3_reg(self, model):
        denominit0 = 63
        denominit1 = 64
        self._reg_write(model.vars.SYNTH_MMDDENOMINIT_DENOMINIT0, denominit0)
        self._reg_write(model.vars.SYNTH_MMDDENOMINIT_DENOMINIT1, denominit1)
        self._reg_write(model.vars.SYNTH_QNCCTRL_ENABLEDQNCTIME, 5)

    def calc_s3_synth_virtual_reg(self, model):
        denominit0 = model.vars.SYNTH_MMDDENOMINIT_DENOMINIT0.value
        denominit1 = model.vars.SYNTH_MMDDENOMINIT_DENOMINIT1.value
        self._reg_write(model.vars.SEQ_MMDDENOMINIT_CALC_DENOMINIT0, denominit0)
        self._reg_write(model.vars.SEQ_MMDDENOMINIT_CALC_DENOMINIT1, denominit1)
        if model.target.upper() == 'IC':
            self._reg_write(model.vars.SEQ_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT0, int(denominit0 * 2))
            self._reg_write(model.vars.SEQ_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT1, int(denominit1 * 2))
        else:
            self._reg_write(model.vars.SEQ_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT0, int(denominit0))
            self._reg_write(model.vars.SEQ_MMDDENOMINIT_CALC_DOUBLED_DENOMINIT1, int(denominit1))

        # Force TX10D mode by default
        self._reg_write(model.vars.SEQ_MISC_SYNTH_MODE_TX00D_EN, 0)

    def calc_synth_misc(self, model):
       # self._reg_write(model.vars.RAC_SYCTRL1_SYLODIVSELFP4G82G4, 0)  #RX always 2.4G, default REG value, remove from calculator
       # self._reg_write(model.vars.RAC_SYMMDCTRL_SYMMDSEL56STG, 0)     #RX always 2.4G

        self._reg_write(model.vars.RAC_SYLOCTRL0_SYLODIVDSMDACCLKDIVRATIO, 0)     #RX 150MHz
        self._reg_write(model.vars.RAC_SYLOCTRLTX0_SYLODIVDSMDACCLKDIVRATIOTX, 1) #TX 300MHz
        self._reg_write(model.vars.RAC_SYLOCTRL0_SYLODIVREGTRIMVREG, 1) # MCUW_RADIO_CFG-2622

    def calc_pga_lna_bw_reg(self, model):
        # PGA BW is controlled by capfb registers
        # for rainier there will be only one BW supported (the PHYs only need 5MHz
        # check agc.py -> calc_tia_capfb_reg
        pass

    def calc_boostlna_reg(self, model):
        # no more extra gain present in LNA
        pass

    def calc_sylodivrloadcclk_reg(self, model):
        # RAC_SYLOEN2_SYENLODIVADCCLK (new register in Rainier) has txrx label, is controlled by RAIL
        pass

    def calc_adc_rate_mode_actual(self, model):
        # This function calculates the actual value of the adc rate mode based on the reg value used
        ifadcenhalfmode = model.vars.RAC_ADCCTRL1_ADCENHALFMODE.value

        if ifadcenhalfmode == 1:
            adc_rate_mode_actual = model.vars.adc_rate_mode.var_enum.HALFRATE
        else:
            adc_rate_mode_actual = model.vars.adc_rate_mode.var_enum.FULLRATE

        # Write the variable
        model.vars.adc_rate_mode_actual.value = adc_rate_mode_actual

    def calc_adc_sampling_rate_reg(self, model):
        # https://jira.silabs.com/browse/MCUW_RADIO_CFG-2309
        adc_rate_mode = model.vars.adc_rate_mode.value

        if adc_rate_mode == model.vars.adc_rate_mode.var_enum.HALFRATE:
            reg = 0
        else:
            reg = 1
        self._reg_write(model.vars.RAC_SYLOCTRL0_SYLODIVADCDIVRATIO, reg)

    def calc_hop_enable(self, model):
        # Disable by default
        model.vars.hop_enable.value = model.vars.hop_enable.var_enum.DISABLED

    def calc_rx_mode(self, model):
        synth_settling_mode = model.vars.synth_settling_mode.value

        if synth_settling_mode == model.vars.synth_settling_mode.var_enum.FAST:
            model.vars.synth_rx_mode.value = model.vars.synth_rx_mode.var_enum.MODE_HOP
        elif synth_settling_mode == model.vars.synth_settling_mode.var_enum.BLE_LR:
            model.vars.synth_rx_mode.value = model.vars.synth_rx_mode.var_enum.MODE1
        else:
            model.vars.synth_rx_mode.value = model.vars.synth_rx_mode.var_enum.MODE2

    def calc_sytrim_misc(self, model):
        pass

    def calc_ifadctrim1_reg(self, model):
        self._reg_write(model.vars.RAC_ADCEN0_ADCENNEGRES, 0)

    def calc_syctrl_reg(self, model):
        modulator = model.vars.modulator_select.value

        if modulator == model.vars.modulator_select.var_enum.IQ_MOD:
            syvcotrimbiastx_val = 5
        else:
            syvcotrimbiastx_val = 4

        self._reg_write(model.vars.RAC_SYCTRL0_SYVCOTRIMIPTAT, 13)
        self._reg_write(model.vars.RAC_SYCTRL0_SYVCOTRIMIBIAS, 4)
        self._reg_write(model.vars.RAC_SYCTRL0_SYDSMDACTRIMLOADBALDLF, 4)
        self._reg_write(model.vars.RAC_SYCTRL0_SYDSMDACTRIMLOADBALDSM, 2)
        self._reg_write(model.vars.RAC_SYCTRL0_SYENMMDREGREPLICA, 0)

        self._reg_write(model.vars.RAC_SYCTRLTX0_SYVCOTRIMIPTATTX, 13)
        self._reg_write(model.vars.RAC_SYCTRLTX0_SYVCOTRIMIBIASTX,  syvcotrimbiastx_val)
        self._reg_write(model.vars.RAC_SYCTRLTX0_SYDSMDACTRIMLOADBALDLFTX, 4)
        self._reg_write(model.vars.RAC_SYCTRLTX0_SYENMMDREGREPLICATX, 1)

    def calc_physeltx_physelrx_reg(self, model):

        # https://jira.silabs.com/browse/MCUW_RADIO_CFG-2608
        modulator_select = model.vars.modulator_select.value
        adc_clock_mode = model.vars.adc_clock_mode.value

        if (modulator_select == model.vars.modulator_select.var_enum.IQ_MOD
                and adc_clock_mode == model.vars.adc_clock_mode.var_enum.HFXOMULT):
            # it's HADM PHY
            self._reg_write(model.vars.SYNTH_DSMCTRLTX_PHISELTX, 1)
            self._reg_write(model.vars.SYNTH_DSMCTRLRX_PHISELRX, 1)
        else:
            self._reg_write(model.vars.SYNTH_DSMCTRLTX_PHISELTX, 0)
            self._reg_write(model.vars.SYNTH_DSMCTRLRX_PHISELRX, 0)
