from enum import Enum

from pycalcmodel.core.variable import ModelVariableFormat, CreateModelVariableEnum
from pyradioconfig.parts.ocelot.calculators.calc_diversity import CALC_Diversity_Ocelot


class Calc_Diversity_Sol(CALC_Diversity_Ocelot):

    def buildVariables(self, model):
        super().buildVariables(model)
        # Internal variables
        # Must exposing the forced selection of a antenna 0 only as DISABLE enum
        var = self._addModelVariable(model, 'softmodem_antdivmode', Enum, ModelVariableFormat.DECIMAL, 'Antenna diversity mode for soft modem')
        member_data = [
            ['DISABLE', 0, 'Antenna 0 used'],
            ['ANTENNA1', 1, 'Antenna 1 is used'],
            ['ANTSELFIRST', 2, 'Select-First algorithm'],
            ['ANTSELCORR', 3, 'Select-Best algorithm based on correlation'],
            ['ANTSELRSSI', 4, 'Select-Best algorithm based on RSSI value']
        ]
        var.var_enum = CreateModelVariableEnum(
            'SFMAntDivModeEnum',
            'List of supported antenna diversity mode for soft modem',
            member_data)

    def calc_softmodem_antdiv_mode(self, model):
        model.vars.softmodem_antdivmode.value = model.vars.softmodem_antdivmode.var_enum.DISABLE
