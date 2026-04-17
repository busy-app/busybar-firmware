import siliconlabs.slc.board_gen.board.board as Board
import siliconlabs.slc.board_gen.project_config as Project_Config
import siliconlabs.slc.board_gen.hardware as Hardware

from typing import List, Union, Set

from siliconlabs.slc.board_gen.util.board_gen_util import *

def communication_peripheral_connected_to_exp_header(board: Hardware, required_signals_list: List[str]) -> bool:
    found = False

    for signal in required_signals_list:
        if board.get_peripheral_options(Req('eusart'), 'exp_h', signal):
            # Eusart available on exp 4/6/8/10
            found = True
        elif board.provides('brd2608a'):
            if board.get_peripheral_options(Req('eusart'), 'breakout_1', signal):
                # Eusart can be connected through breakout
                found = True

    return found

def compatible(provides: Set[str], board: Hardware) -> bool:
    found = False

    if board.has_component('exp_h'):
        required_signals = [{
                                '4': 'TX',
                                '6': 'RX',
                                '8': 'SCLK',
                                '10': 'CS',
                            },
        ]

        found = communication_peripheral_connected_to_exp_header(board, required_signals)

    elif board.provides('brd2608a') and board.has_component('breakout_1'):
        required_signals = [{
                                '5': 'TX',
                                '4': 'RX',
                                '3': 'SCLK',
                                '2': 'CS',
                            },
        ]

        found = communication_peripheral_connected_to_exp_header(board, required_signals)

    return found

def configure(project: Project_Config, board: Hardware, _):
    req = project.requirement('SL_EUSART_SPINCP')

    if board.provides('brd2608a'):
        board_map_list = [{
            '5': 'TX',
            '4': 'RX',
            '6': 'SCLK',
            '7': 'CS',
        },
        ]
        locs = []

        for map in board_map_list:
            signal_options = board.get_peripheral_options(req, 'breakout_1', map)

            if not signal_options:
                continue

            locations = signal_options[0]

            for b_signal in map.keys():
                if b_signal in locations.keys():
                    locs.append(locations[b_signal]['locations'][0])
            break

    else:
        board_map_list = [{
            '4': 'TX',
            '6': 'RX',
            '8': 'SCLK',
            '10': 'CS',
        },
        ]
        locs = []

        for map in board_map_list:
            signal_options = board.get_peripheral_options(req, 'exp_h', map)

            if not signal_options:
                continue

            locations = signal_options[0]

            for b_signal in map.keys():
                if b_signal in locations.keys():
                    locs.append(locations[b_signal]['locations'][0])
            break

    project.satisfy_requirement(req, locs)
