from siliconlabs.slc.board_gen.project_config import ProjectConfig
from siliconlabs.slc.board_gen.hardware import Hardware

import yaml
from pathlib import Path
from typing import Set

antenna_offset_info_file = Path(__file__).parent / '../../../../platform/scripts/board_generation/generation_files/rail_util_cs_antenna_offset.yaml'
if not antenna_offset_info_file.exists():
    print(antenna_offset_info_file.resolve())
    raise Exception('rail_util_cs_antenna_offset.yaml not found. Make sure that the file exists!')

with open(antenna_offset_info_file) as file_handle:
    antenna_offset_info = yaml.safe_load(file_handle)


def get_board_id(board: Hardware) -> str:
    # Crude way of getting the actual board id that we are trying to generate for
    # some boards like brd4182a have multiple revisions which we don't care about
    # so we strip off the last part of the name which is separate by underscore.
    board_id = ''
    for bc in board.board_components:
        if bc.name != 'brd4001a' and bc.name != 'brd4002a':
            board_id = bc.name
    return board_id.split('_')[0]

def compatible(provides: Set[str], board: Hardware) -> bool:
    if get_board_id(board) in antenna_offset_info['cs_antenna_offset']:
        return True
    return False

def configure(project: ProjectConfig, board: Hardware, _):
    board_id = get_board_id(board)
    antenna_offset_info_board = antenna_offset_info['cs_antenna_offset'][board_id]

    project.config('SL_RAIL_UTIL_CS_ANTENNA_COUNT').value = antenna_offset_info_board['count']
    project.config('SL_RAIL_UTIL_CS_ANTENNA_OFFSET_WIRELESS_CM').set_array_values(antenna_offset_info_board['offset_wireless'])
    project.config('SL_RAIL_UTIL_CS_ANTENNA_OFFSET_WIRED_CM').set_array_values(antenna_offset_info_board['offset_wired'])

    project.config('SL_RAIL_UTIL_CS_ANTENNA_COUNT').set_user_default_value(antenna_offset_info_board['count'])
    project.config('SL_RAIL_UTIL_CS_ANTENNA_OFFSET_WIRELESS_CM').set_user_default_value("{ " + ", ".join(map(str, antenna_offset_info_board['offset_wireless'])) + " }")
    project.config('SL_RAIL_UTIL_CS_ANTENNA_OFFSET_WIRED_CM').set_user_default_value("{ " + ", ".join(map(str, antenna_offset_info_board['offset_wired'])) + " }")

