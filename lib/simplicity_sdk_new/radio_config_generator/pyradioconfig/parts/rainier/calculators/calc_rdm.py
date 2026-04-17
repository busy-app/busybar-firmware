from pycalcmodel.core.variable import ModelVariableFormat
from enum import Enum
from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from pyradioconfig.calculator_model_framework.interfaces.icalculator import ICalculator


class CalcRdmRainier(ICalculator):

    def buildVariables(self, model):
        """Populates a list of needed variables for this calculator
        Args:
           model (ModelRoot) : Builds the variables specific to this calculator
        """

        self._addModelVariable(model, 'rx_rdm_state', Enum, ModelVariableFormat.DECIMAL, 'RX RDM State')
        self._add_rx_rdm_state_variable_enums(model)

        self._addModelVariable(model, 'tx_rdm_state', Enum, ModelVariableFormat.DECIMAL, 'TX RDM State')
        self._add_tx_rdm_state_variable_enums(model)

    def _add_rx_rdm_state_variable_enums(self, model):
        member_data = [
            ['RX_FR', 0, 'RX Full-Rate'],
            ['RX_HR', 1, 'RX Half-Rate'],
            ['RX_FR_RFPKD', 2, 'RX Full-Rate with RFPKD'],
            ['RX_HR_RFPKD', 3, 'RX Half-Rate with RFPKD'],
            ['RX_HADM', 4, 'RX HADM'],
            ['RX_HADM_RFPKD', 5, 'RX HADM with RFPKD'],
            ['RX_ENHANCED_RFPKD', 6, 'RX with RFPKD and HFXOMULT as adc_clk'],
        ]
        model.vars.rx_rdm_state.var_enum = CreateModelVariableEnum(
            'RxRDMStateEnum',
            'List of RX RDM States',
            member_data)

    def _add_tx_rdm_state_variable_enums(self, model):
        member_data = [
            ['TX_SY', 0, 'TX Synth'],
            ['TX_HADM', 1, 'TX HADM'],
        ]
        model.vars.tx_rdm_state.var_enum = CreateModelVariableEnum(
            'TxRDMStateEnum',
            'List of TX RDM States',
            member_data)

    def calc_rx_rdm_state(self, model):
        adc_rate_mode = model.vars.adc_rate_mode.value
        rfpkd_enabled = model.vars.AGC_CTRL2_DISRFPKD.value == 0
        adc_clock_mode = model.vars.adc_clock_mode.value

        if adc_clock_mode == model.vars.adc_clock_mode.var_enum.VCODIV:
            if adc_rate_mode == model.vars.adc_rate_mode.var_enum.HALFRATE:
                if rfpkd_enabled:
                    model.vars.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_HR_RFPKD
                else:
                    model.vars.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_HR
            else:
                if rfpkd_enabled:
                    model.vars.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_FR_RFPKD
                else:
                    model.vars.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_FR

        else:
            model.vars.rx_rdm_state.value = model.vars.rx_rdm_state.var_enum.RX_ENHANCED_RFPKD

    def calc_tx_rdm_state(self, model):
        model.vars.tx_rdm_state.value = model.vars.tx_rdm_state.var_enum.TX_SY
