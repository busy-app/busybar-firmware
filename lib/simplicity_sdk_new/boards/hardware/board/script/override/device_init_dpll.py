import yaml
import os
import logging
from typing import Set
from typing import Optional
from siliconlabs.slc.board_gen.board.board import XtalComponent
from siliconlabs.slc.board_gen.util.uart_util import *

logger = logging.getLogger(__name__)

# Key value pairs of sdid and default frequencies
sdid_freq = {
    '200': 38400000,
    '205': 38400000,
    '210': 39000000,
    '215': 39000000,
    '220': 39000000,
    '225': 39000000,
    '230': 38400000,
    '235': 39000000,
    '240': 38400000
}


def find_hfxo(hw: Hardware) -> Optional[XtalComponent]:
    xtals = hw.board.get_component_by_type('xtal')
    for xtal in xtals:
        if xtal.frequency > 32768:
            return xtal
    else:
        if hw.module is not None:
            xtals = hw.module.get_component_by_type('xtal')
            for xtal in xtals:
                if xtal.frequency > 32768:
                    return xtal
    return None


def compatible(provides: Set[str], hw: Hardware) -> bool:
    if hw.provides('device_series_3'):
        return False

    hfxo = find_hfxo(hw)
    # board having no hfxo value
    if hfxo is None:
        return False

    if hw.sdid not in sdid_freq:
        logger.warning('Skipping this as the Default frequency for the new family sdid is not found')
    if hw.sdid in sdid_freq and sdid_freq[hw.sdid] != hfxo.frequency:
        return True
    return False


def configure(project: Project_Config, hw: Hardware, _):
    hfxo = find_hfxo(hw)
    #  Fetch Numerator (N),Denominator (M) from default dpll config file
    N = int(project.config('SL_DEVICE_INIT_DPLL_N').value) + 1
    M = int(project.config('SL_DEVICE_INIT_DPLL_M').value) + 1
    project.config('SL_DEVICE_INIT_DPLL_FREQ').value = int(hfxo.frequency * N / M)

    # The `hardware_board` component is provided within the board component,
    # as both the module and board components contain overrides for the device_init_dpll files.
    # Using the 'hardware_board' provide of the board component,
    # Module Overrides device_init_dpll header files are not included for the board, it is blocked using unless 'hardware_board'
    if hw.provides('device_is_module') and not hw.provides('hardware_board'):
        project.unless.append('hardware_board')
