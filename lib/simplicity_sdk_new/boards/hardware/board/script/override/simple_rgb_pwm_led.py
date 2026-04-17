from siliconlabs.slc.board_gen.util.board_gen_util import Req
import re
from itertools import permutations

rgb_led_parts = [
    'RGB',
    'SML-LX0404SIUPGUSB'
]

def compatible(provides, board):
    instances = []
    rgb_components = []
    for rgb_led_part in rgb_led_parts:
        if (rgb_components_by_part := board.get_components_by_part_number(rgb_led_part)):
            rgb_components.extend(rgb_components_by_part)

    for inst_number in range(len(rgb_components)):
        instances.append('rgb_led{}'.format(inst_number))
    else:
        return instances


def configure(project, board, instance):
    config_prefix = 'SL_SIMPLE_RGB_PWM_LED_'
    req = project.requirement('SL_SIMPLE_RGB_PWM_LED_{}'.format(instance.upper()))

    rgb_components = []
    for rgb_led_part in rgb_led_parts:
        if (rgb_components_by_part := board.get_components_by_part_number(rgb_led_part)):
            rgb_components.extend(rgb_components_by_part)

    # Get component based on instance name last caracter
    rgb_component = rgb_components[int(instance[7])]

    # Get all permutations of channels
    channels_candidates = list(permutations(['CC0', 'CC1', 'CC2', 'CC3'], 3))
    for channels_candidate in channels_candidates:
        rgb_signal_map = {
            's0': channels_candidate[0],
            's1': channels_candidate[1],
            's2': channels_candidate[2]
        }
        options = board.get_peripheral_options(req, rgb_component.id, rgb_signal_map)
        # Stop iterating when there is a solution
        if len(options) != 0:
            break

    # Take first locations solution
    locations = options[0]
    # This is done to avoid selecting TIMER0 always.
    for option in options:
        if option.get('peripheral', "").upper() != "TIMER0":
            locations = option
            break

    locs = []
    for b_signal in rgb_signal_map.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(req, locs)
    i = 0
    for signal in rgb_component.signals:
        # Process just colors signals
        if signal.id not in rgb_signal_map.keys():
            continue

        color = signal.properties.get('color')
        polarity = signal.properties.get('polarity')

        # Override channel name to be able to set configuration properly
        # TODO: This need to be updated in pin tool generator script
        locs[i].route.peripheral.properties['channel.{}.name'.format(rgb_signal_map[signal.id][-1])].value = color.upper()
        i = i + 1

        # Set polarity
        if polarity == 'negative':
            project.config(config_prefix + '{}_{}_POLARITY'.format(instance.upper(), color.upper())).value = config_prefix + 'POLARITY_ACTIVE_LOW'
        else:
            project.config(config_prefix + '{}_{}_POLARITY'.format(instance.upper(), color.upper())).value = config_prefix + 'POLARITY_ACTIVE_HIGH'
