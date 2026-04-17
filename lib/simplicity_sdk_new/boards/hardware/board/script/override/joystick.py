from siliconlabs.slc.board_gen.util.board_gen_util import Req
import re

def compatible(provides, board):
    # Verify the board has the joystick component and 'comm' signal for joystick is connected
    if board.has_component('joystick') and board.get_peripheral_options(Req('gpio'), 'joystick', {'comm': None}):
        return True

    return False

def configure(project, board, instance):
    # Get requirement object from the Joystick driver config file // <gpio> SL_JOYSTICK
    requirement = project.requirement('SL_JOYSTICK')

    # Get peripheral/pinout options for Joystick Pin
    signal_options = board.get_peripheral_options(requirement, 'joystick', {'comm': None})

    # Satisfy the requirement using the single pin comm
    project.satisfy_requirement(requirement, [signal_options[0]['comm']['pins'][0]])
