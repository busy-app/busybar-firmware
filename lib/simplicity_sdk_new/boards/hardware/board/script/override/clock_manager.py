import yaml
import os
import logging
from typing import Set
from siliconlabs.slc.board_gen.util.uart_util import *
from siliconlabs.slc.board_gen.util.clock_util import find_hfxo, find_lfxo, get_board_id, configure_hfxo, configure_lfxo

logger = logging.getLogger(__name__)

# Load board data
board_data_file = os.path.dirname(__file__) + '/../util/board_data.yaml'
if os.path.isfile(board_data_file):
    with open(board_data_file, 'r') as f:
        board_data = yaml.safe_load(f)
else:
    raise Exception('Unable to load board data containing tuning information')


def compatible(provides: Set[str], hw: Hardware) -> bool:

    if hw.board.get_component_by_type('xtal', {'part_number': '7Z-38.400MBG-T'}):
        return True

    board_id = get_board_id(hw)
    if board_id in board_data:
            return True

    hfxo = find_hfxo(hw)
    if hfxo and hw.provides('device_is_module'):
        # Component to which hfxo is connected
        hfxoConnections = [signal.connections[0].component.type for signal in hfxo.signals]
        # Make sure all hfxo connection goes to the MCU/SoC
        if all(connect in ['efr32', 'efm32'] for connect in hfxoConnections):
            return True
        else:
            return False
    if hfxo:
        return True

    lfxo = find_lfxo(hw)
    if lfxo:
        return True

    return False


def configure(project: Project_Config, hw: Hardware, _):
    board_id = get_board_id(hw)

    hfxo = find_hfxo(hw)
    if hfxo:
        configure_hfxo(project, hw, board_id, hfxo, board_data)

    lfxo = find_lfxo(hw)
    if lfxo:
        configure_lfxo(project, hw, board_id, lfxo, board_data)

    if hw.provides('device_series_3'):
        project.config('SL_CLOCK_MANAGER_SOCPLL_REFCLK').value = 'SOCPLL_CTRL_REFCLKSEL_REF_HFXO'
        project.config('SL_CLOCK_MANAGER_SOCPLL_FREQ').value = '150000000'
        project.config('SL_CLOCK_MANAGER_HFXO_FREQ').value = '38400000'
        project.config('SL_CLOCK_MANAGER_HFXO_EN').value = 'SL_CLOCK_MANAGER_HFXO_EN_ENABLE'
        # divn_value = int(project.config('SL_CLOCK_MANAGER_SOCPLL_DIVN').value)
        # project.config('SL_CLOCK_MANAGER_SOCPLL_DIVF').value = int((((socpll_freq * 6) / hfxo_freq) - (divn_value + 2)) * 1024)

    # The `hardware_board` component is provided within the board component,
    # as both the module and board components contain overrides for the clock manager files.
    # Using the 'hardware_board' provide of the board component,
    # Module Overrides clock manager header files are not included for the board, it is blocked using unless 'hardware_board'
    if hw.provides('device_is_module') and not hw.provides('hardware_board'):
        project.unless.append('hardware_board')

    if hw.provides('device_has_ext_mem'):
        qspiflash = hw.board.get_component_by_type('qspiflash')

        for qspiflash in qspiflash:
            for clk in qspiflash.signals:
                if clk.id == 'clk':
                    freq = clk.properties['max_freq']

        project.config('SL_CLOCK_MANAGER_EXT_FLASH_MAX_FREQ').value = int(freq.replace("MHz", "000000"))
