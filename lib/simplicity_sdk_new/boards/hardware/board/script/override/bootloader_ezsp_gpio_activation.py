import siliconlabs.slc.board_gen.board.board as Board
import siliconlabs.slc.board_gen.project_config as Project_Config
import siliconlabs.slc.board_gen.hardware as Hardware
from siliconlabs.slc.board_gen.device.module import ABPin

from siliconlabs.slc.board_gen.util.board_gen_util import *

from typing import List, Dict, Any, Tuple, Set, Union

special_cases = {
    "brd2503a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "exp_h",
                "signal": "11"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "exp_h",
                "signal": "13"
            }
        }
    },
    "brd2601a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "exp_h",
                "signal": "11"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "exp_h",
                "signal": "13"
            }
        }
    },
    "brd2601b": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "exp_h",
                "signal": "11"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "exp_h",
                "signal": "13"
            }
        }
    },
    "brd2608a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "breakout_0",
                "signal": "2"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "breakout_0",
                "signal": "3"
            }
        }
    },
    "brd4104a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "breakout_1",
                "signal": "15"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "breakout_1",
                "signal": "17"
            }
        }
    },
    "brd4300a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "led_0",
                "signal": "s0"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "led_1",
                "signal": "s0"
            }
        }
    },
    "brd4300b": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "led_0",
                "signal": "s0"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "led_1",
                "signal": "s0"
            }
        }
    },
    "brd4179b": {
        "compatible": False,
    },
    "brd4183a": {
        "compatible": False,
    },
    "brd4183b": {
        "compatible": False,
    },
    "brd4183c": {
        "compatible": False,
    },
    "brd4270b": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "exp_h",
                "signal": "7"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "exp_h",
                "signal": "11"
            }
        }
    },
    "brd4271a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "exp_h",
                "signal": "7"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "exp_h",
                "signal": "11"
            }
        }
    },
    "brd4272a": {
        "compatible": True,
        "requirements": {
            "SL_EZSPSPI_HOST_INT": {
                "component": "exp_h",
                "signal": "7"
            },
            "SL_EZSPSPI_WAKE_INT": {
                "component": "exp_h",
                "signal": "11"
            }
        }
    }
}

def is_special_case(board: Hardware) -> Tuple[bool, str]:
    for board_pn in special_cases:
        if board.provides(board_pn) or board.provides("{}_revision".format(board_pn)):
            return (True, board_pn)

    return (False, "")

def specal_cases_is_compatible(board_pn: str) -> bool:
    return special_cases[board_pn]["compatible"]

def compatible(provides: Set[str], board: Hardware) -> Union[bool, str]:
    compatible = False
    is_special = is_special_case(board)

    if is_special[0]:
        compatible = special_cases[is_special[1]]["compatible"]
    elif board.has_component("exp_h"):
        exp_header_signals = {
            '7',
            '9'
        }

        for signal in exp_header_signals:
            expansion_option = board.get_peripheral_options(Req('gpio'), "exp_h", {signal: None, })

            if not expansion_option:
                return False

        compatible = True

    return compatible

def configure_pin(project: Project_Config, hw: Hardware, requirement: str, component: str, pin_signal_name: str):
    req = project.requirement(requirement)

    expansion_option = hw.get_peripheral_options(req, component, {pin_signal_name: None})

    pin = expansion_option[0][pin_signal_name]["pins"][0]

    project.satisfy_requirement(req, [pin])

def configure(project: Project_Config, board: Hardware, _):
    is_special = is_special_case(board)

    if is_special[0] and specal_cases_is_compatible(is_special[1]):
        requirements = special_cases[is_special[1]]["requirements"]

        for requirement_name in requirements:
            component_name = requirements[requirement_name]["component"]
            signal_name = requirements[requirement_name]["signal"]

            configure_pin(project, board, requirement_name, component_name, signal_name)
    else:
        configure_pin(project, board, "SL_EZSPSPI_HOST_INT", "exp_h", "7")
        configure_pin(project, board, "SL_EZSPSPI_WAKE_INT", "exp_h", "9")
