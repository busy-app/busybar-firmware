from pyradioconfig.calculator_model_framework.interfaces.itarget import ITarget
from os.path import join

class TargetFpgaRainier(ITarget):

    _targetName = "FPGA"
    _description = "Supports the OTA FPGA"
    _store_config_output = False
    _cfg_location = join('target_fpga','rainier')
    _tag = "FPGA"

    # https://confluence.silabs.com/display/TEAMPHYMAC/PHYMAC+-+RF+and+Digital+IQ+Swaps
    # TX IQ swap is currently not under control of Radio Calculator
    # RX IQ swap should not be forced, as it is enabled elsewhere
    # For sanity check, one should confirm TX IQ swap and RX IQ swap are both true on the bench
    # MODEM->TXCORRSTATIC→TXIQSWAP = 1
    # MODEM->MIXCTRL→DIGIQSWAPEN = 1
    def target_calculate(self, model):

        #FPGA can only run in full rate mode
        model.vars.adc_rate_mode.value_forced = model.vars.adc_rate_mode.var_enum.FULLRATE
        model.vars.adc_clock_mode.value_forced = model.vars.adc_clock_mode.var_enum.HFXOMULT

        #38.4 MHz XO Frequency
        model.vars.xtal_frequency_hz.value_forced = int(38.4e6)