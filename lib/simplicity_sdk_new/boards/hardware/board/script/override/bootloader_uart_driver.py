import siliconlabs.slc.board_gen.board.board as Board
import siliconlabs.slc.board_gen.project_config as Project_Config
import siliconlabs.slc.board_gen.hardware as Hardware
import siliconlabs.slc.board_gen.util.board_gen_util as board_gen_util

from siliconlabs.slc.board_gen.util.board_gen_util import *

from typing import List, Dict, Union, Set

instance_name_to_board_component = {
    'vcom': 'vcom',
    'exp': 'exp_h',
    'mikroe': 'mikroe',
}

specific_boards = {
    'BRD2204A': 'exp'
}

def requirement_exist(board: Hardware, requirement: Req, board_component_name: str, board_map_list: List[Dict[str, str]]) -> bool:
    for board_signal_map in board_map_list:
        if board.get_peripheral_options(requirement, board_component_name, board_signal_map):
            return True
    return False

def get_best_communication_component(board: Board) -> Union[bool, str]:
    preferred_instance = False

    for specific_board in specific_boards:
        if board.has_label(specific_board):
            return specific_boards[specific_board]

    for instance_name, board_component_name in instance_name_to_board_component.items():
        if board.has_component(board_component_name):
            required_signals = []

            if instance_name == 'exp':
                required_signals = [{
                    # Not available on 12/14, try on 4/6
                    '12': 'TX',
                    '14': 'RX',
                }, {
                    '4': 'TX',
                    '6': 'RX',
                }]
            elif instance_name == 'vcom':
                required_signals = [{
                    'rx_cipo': 'RX',
                    'tx_copi': 'TX',
                }]
            elif instance_name == 'mikroe':
                required_signals = [{
                    'uart_tx': 'TX',
                    'uart_rx': 'RX',
                }]

            if requirement_exist(board, Req('usart'), board_component_name, required_signals):
                preferred_instance = instance_name
                break
            continue

    return preferred_instance

def compatible(provides: Set[str], board: Hardware) -> Union[bool, str]:
    return get_best_communication_component(board)

def configure(project: Project_Config, board: Hardware, _):
    communicationPeripheral = get_best_communication_component(board)

    board_component_name = instance_name_to_board_component[communicationPeripheral]
    requirement = project.requirement("SL_SERIAL_UART")

    # board_signal : device_signal_prefix
    if communicationPeripheral == 'vcom':
        # BC VCOM
        board_map = {
            'cts_n_clk': '(CTS)',
            'rts_n_cs_n': '(RTS)',
            'rx_cipo': 'RX',
            'tx_copi': 'TX',
        }
    elif communicationPeripheral == 'mikroe':
        board_map = {
            'uart_tx': 'TX',
            'uart_rx': 'RX',
        }
    else:
        # EXP header
        board_map = {
            '12': 'TX',
            '14': 'RX',
        }
    signal_options = board.get_peripheral_options(requirement, board_component_name, board_map)

    if communicationPeripheral == 'exp' and not signal_options:
        board_map = {
            '4': 'TX',
            '6': 'RX',
        }
        signal_options = board.get_peripheral_options(requirement, board_component_name, board_map)

    # Select first matching Peripheral
    locations = signal_options[0]
    locs = []
    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(requirement, locs)

    # Must be enforced
    project.config('SL_SERIAL_UART_FLOW_CONTROL').value = '0'

    connect_gpio(project, board, 'SL_VCOM_ENABLE', 'vcom', 'enable')

