import re
import siliconlabs.slc.board_gen.hardware as Hardware
import siliconlabs.slc.board_gen.project_config as Project_Config
from typing import Tuple

special_cases = {
    "brd4276a": {
        "requirements": {
            "SL_FEM_UTIL_RX": {
                "component": "breakout_1",
                "signal": "5"
            }
        }
    }
}

def compatible(_, hw):
    if hw.has_component('fem'):
        return True
    return False


def is_special_case(board: Hardware) -> Tuple[bool, str]:
    for board_pn in special_cases:
        if board.provides(board_pn) or board.provides("{}_revision".format(board_pn)):
            return (True, board_pn)

    return (False, "")


def configure_pin(project: Project_Config, hw: Hardware, requirement: str, component: str, pin_signal_name: str):
    req = project.requirement(requirement)

    expansion_option = hw.get_peripheral_options(req, component, {pin_signal_name: 'ASYNC'})

    options = {}
    for option in expansion_option:
        channel = re.search(r'[A-Z.](\d+)$', option['peripheral']).group(1)
        options[int(channel)] = option[pin_signal_name]['locations'][0]
    # Default to channel 8 for CRX
    project.satisfy_requirement(req, [options[8]])
    req.set_readonly()


def configure(project, hw, _):
    ctx_req = project.requirement('SL_FEM_UTIL_TX')
    cps_req = project.requirement('SL_FEM_UTIL_SLEEP')

    ctx_options = hw.get_peripheral_options(ctx_req, 'fem', {'ctx': 'ASYNCH'})
    cps_options = hw.get_peripheral_options(cps_req, 'fem', {'cps': 'ASYNCH'})

    options = {
        'ctx':  {},
        'cps': {}
    }
    for option in ctx_options:
        channel = int(re.search(r'[A-Z.](\d+)$', option['peripheral']).group(1))
        options['ctx'][channel] = option['ctx']['locations'][0]
    for option in cps_options:
        channel = re.search(r'[A-Z.](\d+)$', option['peripheral']).group(1)
        options['cps'][int(channel)] = option['cps']['locations'][0]

    if 9 in options['cps'] and 10 in options['ctx']:
        # Default to channel 9 and 10 for CPS and CTX
        project.satisfy_requirement(cps_req, [options['cps'][9]])
        project.satisfy_requirement(ctx_req, [options['ctx'][10]])

    else:
        # Try every possible triple combination of channels
        for i in range(21):
            if i+1 in options['cps'] and i+2 in options['ctx']:
                project.satisfy_requirement(cps_req, [options['cps'][i+1]])
                project.satisfy_requirement(ctx_req, [options['ctx'][i+2]])
                break
        else:
            raise ValueError("Failed to set PRS channels for FEM")

    is_special = is_special_case(hw)

    if is_special[0]:
        requirements = special_cases[is_special[1]]["requirements"]

        for requirement_name in requirements:
            component_name = requirements[requirement_name]["component"]
            signal_name = requirements[requirement_name]["signal"]

            configure_pin(project, hw, requirement_name, component_name, signal_name)

    project.config('SL_FEM_UTIL_TX_ENABLE').value = '1'
    project.config('SL_FEM_UTIL_RX_ENABLE').value = '1'
    ctx_req.set_readonly()
    cps_req.set_readonly()
