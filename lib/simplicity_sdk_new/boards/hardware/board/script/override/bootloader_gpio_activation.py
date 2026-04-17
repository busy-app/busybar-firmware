import siliconlabs.slc.board_gen.board.board as Board
import siliconlabs.slc.board_gen.project_config as Project_Config
import siliconlabs.slc.board_gen.hardware as Hardware

from siliconlabs.slc.board_gen.util.board_gen_util import *
from typing import Tuple, Union, Set

board_components = [
    'button',
    'button_0',
    'button_1',
]

# To make use of a GPIO in case button is not present/usable
special_cases = {
    # using thingplus pinout a4 signal which is connected to pc00
    "brd2704a": {
        "enabled": True,
        "component_name": "thingplus",
        "signal": {'a4': None}
    },
    # use breakout pin 7 signal which is connected to pc00
    "brd4191a": {
        "enabled": True,
        "component_name": "breakout",
        "signal": {'7': None}
    },
    # use the signal which is connected to pc00
    "brd4406a": {
        "enabled": True,
        "component_name": "rfswitch",
        "signal": {'ctl1': None}
    },
}


def is_special_case(board: Hardware) -> Tuple[bool, str]:
    for board_pn in special_cases:
        if board.provides(board_pn) or board.provides("{}_revision".format(board_pn)):
            return (True, board_pn)
    return (False, "")


def special_case_is_enabled(board_pn: str) -> bool:
    return special_cases[board_pn]["enabled"]


def get_best_button(board: Hardware) -> Union[bool, str]:
    prefered_instance = False
    for component in board_components:
        if board.has_component(component):
            if not board.get_peripheral_options(Req('gpio'), component, {'s': None, }):
                continue

            prefered_instance = component
            break

    return prefered_instance


def compatible(provides: Set[str], board: Hardware) -> Union[bool, str]:
    is_special = is_special_case(board)
    if is_special[0] and special_case_is_enabled(is_special[1]):
        return True
    return get_best_button(board)


def configure(project: Project_Config, board: Hardware, _):
    req = project.requirement('SL_BTL_BUTTON')
    is_special = is_special_case(board)

    if is_special[0] and special_case_is_enabled(is_special[1]):
        InstanceName = special_cases[is_special[1]]["component_name"]
        signal = special_cases[is_special[1]]["signal"]

    else:
        InstanceName = get_best_button(board)
        signal = {
            's': None
        }

    options = board.get_peripheral_options(req, InstanceName, signal)

    locations = options[0]
    locs = []
    for b_signal in signal.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['pins'][0])

    project.satisfy_requirement(req, locs)
    button_component = board.board.get_component(InstanceName)

    if not is_special[0]:
        if button_component.get_signal('s').properties.get('polarity') == 'negative':
            project.config('SL_GPIO_ACTIVATION_POLARITY').value = 'LOW'
        else:
            project.config('SL_GPIO_ACTIVATION_POLARITY').value = 'HIGH'
