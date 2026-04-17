from siliconlabs.slc.board_gen.util.board_gen_util import Req
import re

# Z-Wave kits have buttons on expansion board (BRD8029A).
# Treat these as another button board component, but only if in the Z-Wave range for board numbers.
# This is a temporary solution until native support for EXP boards is available.
boards_with_exp_buttons = r'^brd42[01]\d[^z]'
exp_buttons = {
    '15': 'btn2',
    '16': 'btn3',
    '10': 'slider',
}
def compatible(provides, board):
    instances = []
    if board.has_component('button') and board.get_peripheral_options(Req('gpio'), 'button', {'s': None}):
        instances.append('btn0')
    if board.has_component('button_0') and board.get_peripheral_options(Req('gpio'), 'button_0', {'s': None}):
        instances.append('btn0')
    if board.has_component('button_1') and board.get_peripheral_options(Req('gpio'), 'button_1', {'s': None}):
        instances.append('btn1')

    if any([re.search(boards_with_exp_buttons, b.name) for b in board.board_components]):
        for signal, instance in exp_buttons.items():
            if board.has_component('exp_h') and board.get_peripheral_options(Req('gpio'), 'exp_h', {signal: None}):
                instances.append(instance)

    return instances

def configure(project, board, instance):
    if any([re.search(boards_with_exp_buttons, b.name) for b in board.board_components]) and instance in exp_buttons.values():
        board_component = 'exp_h'
        for s, i in exp_buttons.items():
            if instance == i:
                signal = s
                break
        else:
            raise Exception("EXP header BTN instance mismatch")
    else:
        if board.has_component('button'):
            board_component = 'button'
            signal = 's'
        else:
            board_component = 'button_{}'.format(instance[-1])
            signal = 's'

    req = project.requirement('SL_SIMPLE_BUTTON_{}'.format(instance.upper()))

    options = board.get_peripheral_options(req, board_component, {
        signal: None
    })
    if not options:
        raise RuntimeError('Unable to find connection that match')

    project.satisfy_requirement(req, [options[0][signal]['pins'][0]])

