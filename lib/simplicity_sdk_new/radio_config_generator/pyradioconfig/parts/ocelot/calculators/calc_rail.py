from pyradioconfig.parts.lynx.calculators.calc_rail import CalcRailLynx
from pycalcmodel.core.variable import ModelVariableFormat


class CalcRailOcelot(CalcRailLynx):

    def buildVariables(self, model):
        super().buildVariables(model)
        self._addModelVariable(model, 'ircal_index', int, ModelVariableFormat.DECIMAL, 'IR calibration ID')

    def calc_rail_delays(self, model):
        """calc_rail_delays

        Args:
            model (ModelRoot) : Data model to read and write variables from
        """
        # Ocelot+ has a built-in RX chain delay value
        rx_delay_ns = int(round(model.vars.rx_grp_delay_us.value * 1000))
        sync_delay_ns = int((model.vars.MODEM_CTRL0_FRAMEDETDEL.value * 8 * 1e9) \
                            / model.vars.baudrate.value)

        startTx_delay_ns = int(round((model.vars.tx_grp_delay_us.value) * 1000)) # + on air delta ?
        endTx_delay_ns = 0

        # With no FRAMEDETDEL these values will be identical
        model.vars.rx_sync_delay_ns.value = rx_delay_ns + sync_delay_ns
        model.vars.rx_eof_delay_ns.value = rx_delay_ns

        model.vars.tx_sync_delay_ns.value = startTx_delay_ns
        model.vars.tx_eof_delay_ns.value = endTx_delay_ns

    def calc_ircal_index(self, model):
        # https://jira.silabs.com/browse/MCUW_RADIO_CFG-2687

        loside_reg = model.vars.SYNTH_IFFREQ_LOSIDE.value
        pgabwmode_reg = model.vars.RAC_PGACTRL_PGABWMODE.value
        lodiv_actual = model.vars.lodiv_actual.value

        model.vars.ircal_index.value = lodiv_actual << 5 + pgabwmode_reg << 1 + loside_reg
