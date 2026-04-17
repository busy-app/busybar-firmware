from pyradioconfig.parts.common.calculators.calc_utilities import CALC_Utilities
from enum import Enum
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from pyradioconfig.parts.bobcat.calculators.calc_modulator import Calc_Modulator_Bobcat
from pyradioconfig.calculator_model_framework.Utils.CustomExceptions import CalculationException
from pyradioconfig.calculator_model_framework.Utils.LogMgr import LogMgr

class CalcModulatorRainier(Calc_Modulator_Bobcat):

    def buildVariables(self, model):
        super().buildVariables(model)

        self._addModelVariable(model, 'modulator_select', Enum, ModelVariableFormat.DECIMAL,
                               desc='determines modulator path')
        self._addModelVariable(model, 'br2m', int, ModelVariableFormat.DECIMAL)
        self._addModelVariable(model, 'dac_clock_mode', Enum, ModelVariableFormat.DECIMAL,
                               desc='determines dac clocking mode')
        model.vars.modulator_select.var_enum = CreateModelVariableEnum(
            'ModSelEnum',
            'List of supported modulator paths',
            [['PH_MOD', 0, 'Phase modulator'],
             ['IQ_MOD', 1, 'IQ modulator (direct upconversion)']])

        model.vars.dac_clock_mode.var_enum = CreateModelVariableEnum(
            'ModSelEnum',
            'List of supported dac clock modes',
            [['DISABLED', 0, 'CLKMULT not used in TX mode'],
             ['HFXOBYP', 1, 'DAC runs at HFXO frequency'],
             ['HFXOMULT', 2, 'DAC runs at multiple of HFXO frequency']])

        self._addModelVariable(model, 'synchronous_mixdac_clk', bool, ModelVariableFormat.DECIMAL,
                               desc='Flag used treat clk_dac as synchronous to clk_mod and bypass afifo')

    def calc_tx_mode(self, model):
        # for rainier we will be using PHMOD by default
        # https://confluence.silabs.com/display/PGRAINIER/PGRAINIERVALTEST+-+2023-08-09+-+Meeting+Note+-+IQMOD+vs.+PHMOD+in+Production
        model.vars.modulator_select.value = model.vars.modulator_select.var_enum.PH_MOD

    def calc_txmod_reg(self, model):
        modulator_select = model.vars.modulator_select.value
        self._reg_write(model.vars.MODEM_TXCTRL_TXMOD, int(modulator_select))

    def calc_dac_clock_mode(self, model):
        modulator_select = model.vars.modulator_select.value
        if modulator_select == model.vars.modulator_select.var_enum.IQ_MOD:
            model.vars.dac_clock_mode.value = model.vars.dac_clock_mode.var_enum.HFXOBYP
        else:
            model.vars.dac_clock_mode.value = model.vars.dac_clock_mode.var_enum.DISABLED

    def calc_tx_baud_rate_actual(self, model):
        """
        calculate actual TX baud rate from register settings

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        fxo = model.vars.xtal_frequency.value
        txbr_ratio = model.vars.txbr_ratio_actual.value
        interp_factor = 2.0 if model.vars.br2m.value else 4.0

        tx_baud_rate = fxo / interp_factor / 8.0 * txbr_ratio

        model.vars.tx_baud_rate_actual.value = tx_baud_rate

    def calc_tx_br2m(self, model):
        """
        controls the interpolator ratio and SRC outout_rate
        interpolator ratio is 2X when br2m = 1, 4X when br2m = 0
        SRC outout_rate is f_xo/2 when br2m = 1, f_xo/4 when br2m = 0

        SRC_ratio = input_rate / output_rate
        input_rate = baudrate * 8
        output_rate = f_xo/2 if br2m else f_xo/4
        given baud_rate and fxo we should choose BR2M so that SRC is as close to 1.0 as possible to get best performance

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """
        baudrate = model.vars.baudrate.value
        fxo = model.vars.xtal_frequency.value * 1.0

        # given baud_rate and fxo we should choose BR2M so that SRC is as close to 1.0
        # as possible to get best performance
        src_distance_to_1 = float("inf")  # set
        br2m = 0  # set default value
        for br2m_value in range(2 ** model.vars.MODEM_TXCTRL_BR2M.get_bit_width()):
            src = (fxo / (2 + 2*(1 - br2m_value))) / (8*baudrate)
            if abs(1 - src) < src_distance_to_1:
                src_distance_to_1 = abs(1 - src)
                br2m = br2m_value

        model.vars.br2m.value = br2m
        self._reg_write(model.vars.MODEM_TXCTRL_BR2M, int(br2m))

    def get_mod_sample_rate_factor(self, model):
        """
        Return modulator sample multiplier factor based on modulator data path selection

        :param modulator_select:
        :return:
        """
        modulator_select = model.vars.modulator_select.value
        br2m = model.vars.br2m.value

        # If IQMOD data path is selected
        if modulator_select == model.vars.modulator_select.var_enum.IQ_MOD:
            if br2m:
                # modulation multiplier is running at (f_xo/2)
                mod_samp_rate_factor = 2
            else:
                # modulation multiplier is running at (f_xo/4)
                mod_samp_rate_factor = 4

        # If PHMOD data path is selected
        else:
            # modulation multiplier is running at f_xo
            mod_samp_rate_factor = 1

        return mod_samp_rate_factor

    def calc_modindex_value(self, model):
        """
        calculate MODINDEX value
        Modulation scaling for PHMOD happens after src and interpolator
        Modulation scaling for IQMOD happens after src but before interpolator, therefore modulation scaling needs to be
        adjusted as per br2m bit

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        fxo = model.vars.xtal_frequency.value * 1.0
        modformat = model.vars.modulation_type.value
        freq_dev_hz = model.vars.deviation.value * 1.0
        shaping_filter_gain = model.vars.shaping_filter_gain_actual.value
        br2m = model.vars.br2m.value
        baudrate = model.vars.baudrate.value

        # setup inputs for modindex calculations
        mod_samp_rate_factor = self.get_mod_sample_rate_factor(model)

        # calculate modindex
        if modformat == model.vars.modulation_type.var_enum.OQPSK:
            src_output_rate = fxo / 2 if br2m else fxo / 4
            # conv_gain = shaping_filter_output_rate/ src_output_rate
            # conv_gain = (T_fxo * 2 / T_shape), freq --> phase --> freq
            conv_gain = 8 * baudrate / src_output_rate
            modindex = 0.5 * mod_samp_rate_factor * baudrate / (2 * shaping_filter_gain * conv_gain * fxo)

        elif (modformat == model.vars.modulation_type.var_enum.FSK2 or
              modformat == model.vars.modulation_type.var_enum.FSK4 or
              modformat == model.vars.modulation_type.var_enum.MSK):
            modindex = freq_dev_hz / (shaping_filter_gain * (fxo / mod_samp_rate_factor))
        else:
            raise CalculationException("ERROR: %s modulation not yet supported!" % modformat)

        model.vars.modindex.value = modindex


    def calc_modindex_field(self, model):
        """
        convert desired modindex fractional value to MODINDEXM * 2^MODINDEXE

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        modindex = model.vars.modindex.value
        max_m = (2 ** model.vars.MODEM_MODINDEX_MODINDEXM.get_bit_width()) - 1
        max_e = (2 ** model.vars.MODEM_MODINDEX_MODINDEXE.get_bit_width()) - 1

        # convert fractional modindex into m * 2^e format
        m, e = CALC_Utilities().frac2exp(max_m, modindex, up=True)

        # ############
        # In series 3, RTL implementation of the modindex multiplier is as follows
        # input -> 16bit fractional value (fixed point implementation, signed 5.11, sign bit included)
        # output -> 20bit fractional value (fixed point implementation, signed 1.19, sign bit included)
        # mod_index -> mantissa * 2^exp
        # for any fractional value 'a' with fixed point binary implementation of lets say 5.11
        # a = (value of 16 bit assuming no implied binary point) * 2^-11
        # therefore,
        # output = input * 2^-11 * (mantissa * 2^exp) = input * (mantissa * 2^exp) * 2^-11
        # since required output is needed with 19 fractional bits, output can be re-written as
        # output = input * (mantissa * 2^exp * 2^8) * 2^-19 = input * (mantissa * 2^exp+8) * 2^-19
        # therefore we need to make this adjustment in exp(e)
        # ############
        e = e + 8

        # MODEINDEXE is a signed value, therefore wrap if value is negative
        if e < 0:
            e += (2 ** model.vars.MODEM_MODINDEX_MODINDEXE.get_bit_width())

        # verify number fits into register
        if m > max_m:
            m = max_m
        if e > max_e:
            e = max_e
        if m < 0:
            m = 0

        self._reg_write(model.vars.MODEM_MODINDEX_MODINDEXM, int(m))
        self._reg_write(model.vars.MODEM_MODINDEX_MODINDEXE, int(e))

    def calc_modindex_virtual_reg_field(self, model):
        modindex_e = model.vars.MODEM_MODINDEX_MODINDEXE.value
        modindex_m = model.vars.MODEM_MODINDEX_MODINDEXM.value
        self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE, int(modindex_e))
        self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXM, int(modindex_m))
        if model.target.upper() == 'IC':
            if modindex_e < pow(2, model.vars.SEQ_MODINDEX_CALC_MODINDEXE.get_bit_width()) - 1:
                self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXE, int(modindex_e+1))
                self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXM, int(modindex_m))
            else:
                # check if we can double Mantisaa
                if modindex_m * 2 < pow(2, model.vars.SEQ_MODINDEX_CALC_MODINDEXM.get_bit_width()) - 1:
                    self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXE, int(modindex_e))
                    self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXM, int(modindex_m*2))
                else:
                    LogMgr.Error('Can not calculate DOUBLE MODINDEXE and MODEINDEXM due to Overflow')
        else:
            self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXE, int(modindex_e))
            self._reg_write(model.vars.SEQ_MODINDEX_CALC_MODINDEXE_DOUBLED_MODINDEXM, int(modindex_m))


    def calc_modindex_actual(self, model):
        """
        given register settings return actual MODINDEX as fraction

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        m = model.vars.MODEM_MODINDEX_MODINDEXM.value
        e = model.vars.MODEM_MODINDEX_MODINDEXE.value

        # MODEINDEXE is a signed value
        if e > 15:
            e -= 32

        # removing the adjustment from exp(e) done for bit fixed point implementation adjustment
        e = e - 8
        model.vars.modindex_actual.value = 1.0 * m * 2**e

    def get_modulator_sample_rate(self, model):
        fxo = model.vars.xtal_frequency.value * 1.0
        br2m = model.vars.br2m.value
        modulator_select = model.vars.modulator_select.value

        if modulator_select == model.vars.modulator_select.var_enum.IQ_MOD:
            mod_samp_rate = fxo / 2 if br2m else fxo / 4  # IQMOD
        else:
            mod_samp_rate = fxo  # PHMOD

        return mod_samp_rate

    def calc_tx_freq_dev_actual(self, model):
        """
        given register setting return actual frequency deviation used in the modulator
        Using Equations in Table 5.25 of EFR32 Reference Manual (internal.pdf)

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """
        modformat = model.vars.modulation_type.value
        modindex = model.vars.modindex_actual.value
        shaping_filter_gain = model.vars.shaping_filter_gain_actual.value

        mod_samp_rate = self.get_modulator_sample_rate(model)

        if modformat == model.vars.modulation_type.var_enum.FSK2 or \
                modformat == model.vars.modulation_type.var_enum.FSK4:
            freq_dev_hz = modindex * (mod_samp_rate * shaping_filter_gain)
        else:
            freq_dev_hz = 0.0

        model.vars.tx_deviation_actual.value = freq_dev_hz

    def calc_txbr_value(self, model):
        """
        TXBRNUM is the register used to set SRC ratio. Unlike series 2 where interpolator controlled the datarate,
        an SRC (after shaping) in the modulator datapath indirectly controls the baudrate.

        SRC_ratio = TXBRNUM = input_rate / output_rate
        input_rate = baudrate * 8
        output_rate = f_xo/2 if br2m else f_xo/4

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        # Load model values into local variables
        xtal_frequency_hz = model.vars.xtal_frequency_hz.value
        baudrate = model.vars.baudrate.value

        # calculate baudrate to fxo ratio
        interp_factor = 2.0 if model.vars.br2m.value else 4.0
        ratio = (8.0 * baudrate) / (xtal_frequency_hz / interp_factor)
        # Load local variables back into model variables
        model.vars.txbr_ratio.value = ratio

    def calc_txbr_reg(self, model):

        ratio = model.vars.txbr_ratio.value
        # TXBRNUM is a 16bits fractional number
        self._reg_write(model.vars.MODEM_TXBR_TXBRNUM, int(round(ratio * (2 ** 16))))

    def calc_txbr_actual(self, model):
        """
        given register values calculate actual TXBR ratio implemented

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """

        num = model.vars.MODEM_TXBR_TXBRNUM.value * 1.0
        # TXBRNUM is a 16bits fractional number
        ratio = num / (2 ** 16)
        model.vars.txbr_ratio_actual.value = ratio

    def calc_synchronous_mixdac_clk(self, model):
        # default to disabled, only HADM PHYs will set the profile input
        model.vars.synchronous_mixdac_clk.value = False

    def calc_synchronous_mixdac_clk_regs(self, model):
        if not model.vars.synchronous_mixdac_clk.value:
            # normal async front filtering
            self._reg_write(model.vars.MODEM_TXCTRL_TXAFIFOBYP, 0)
        else:
            self._reg_write(model.vars.MODEM_TXCTRL_TXAFIFOBYP, 1)