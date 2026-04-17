from siliconlabs.slc.board_gen.util.board_gen_util import Req
import re


def compatible(provides, board):
    instances = []

    if board.has_component('button_0') and board.get_peripheral_options(Req('gpio'), 'button_0', {'s': None}):
        instances.append('btn0')
    if board.has_component('button_1') and board.get_peripheral_options(Req('gpio'), 'button_1', {'s': None}):
        instances.append('btn1')

    # we need two entries to get it to work
    if len(instances) != 2:
        return []

    return instances

def configure(project, board, instance):
    if instance is None:
        return

    board_component = 'button_{}'.format(instance[-1])
    signal = 's'

    project.config(
            'SL_CPC_GPIO_EXPANDER_GPIO_{}_NAME'.format(instance.upper())).value = '"' + instance.upper() + '"'
    req = project.requirement('SL_CPC_GPIO_EXPANDER_GPIO_{}'.format(instance.upper()))

    options = board.get_peripheral_options(req, board_component, {
        signal: None
    })
    if not options:
        raise RuntimeError('Unable to find connection that match')

    project.satisfy_requirement(req, [options[0][signal]['pins'][0]])

