import siliconlabs.slc.board_gen.board.board as Board
import siliconlabs.slc.board_gen.project_config as Project_Config
import siliconlabs.slc.board_gen.hardware as Hardware

from siliconlabs.slc.board_gen.util.board_gen_util import *
from typing import Union, Set

def compatible(provides: Set[str], board: Hardware) -> bool:
    return board.has_component("spiflash") and board.provides('device_has_usart') and board.provides('hardware_board_has_spiflash')

def configure_ext_flash(project: Project_Config, board: Hardware):
    requirement = project.requirement("SL_USART_EXTFLASH")

    board_map = {
        'copi': 'TX',
        'cipo': 'RX',
        'clk': 'CLK',
        'cs_n': '(CS)',
    }

    component_name = "spiflash"

    if board.has_tag("board:pn:BRD4301"):
        component_name = "exp_h"

        board_map = {
            '4': 'TX',
            '6': 'RX',
            '8': 'CLK',
            '10': '(CS)',
        }

    signal_options = board.get_peripheral_options(requirement, component_name, board_map)

    locations = select_peripheral_instance(board, signal_options)
    locs = []

    for b_signal in board_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(requirement, locs)

    # List present due to backward compatibility regarding the values provided inside hardware/kit (s024_kits)
    # for some specific boards.
    board_with_specific_frequency = [
        "BRD4151A", "BRD4159A", "BRD4165B", "BRD4171A", "BRD4154A", "BRD4257B",
        "BRD4259A", "BRD4263A", "BRD4263B", "BRD4182A", "BRD2503A", "BRD4186A",
        "BRD4187A", "BRD4186B", "BRD4187B",
        ]

    for specific_board in board_with_specific_frequency:
        if board.has_label(specific_board):
            project.config('SL_USART_EXTFLASH_FREQUENCY').value = "2000000U"

def select_peripheral_instance(board, options):
    # Default to the first match
    return options[0]

def configure(project: Project_Config, board: Hardware, _):
    configure_ext_flash(project, board)
