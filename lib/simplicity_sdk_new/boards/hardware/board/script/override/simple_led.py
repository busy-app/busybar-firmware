from siliconlabs.slc.board_gen.util.board_gen_util import Req
from siliconlabs.slc.board_gen.hardware import Hardware
from siliconlabs.slc.board_gen.board.board import Component, Signal
from siliconlabs.slc.board_gen.project_config import ProjectConfig
import re
import logging
from typing import Set, List, Dict, TypedDict

logger = logging.getLogger(__name__)

# Z-Wave kits have LEDs on expansion board (BRD8029A).
# Treat these as another LED board component, but only if in the Z-Wave range for board numbers.
# This is a temporary solution until native support for EXP boards is available.
boards_with_exp_leds = r'^brd42[01]\d[^z]'
exp_leds = {
    '3': 'led2',
    '5': 'led3',
}


class Context(TypedDict):
    led: Component
    signal: Signal


# Use a dictionary to save the led component between compatible and configure calls
context: Dict[str, Context] = {}


def compatible(provides: Set[str], board: Hardware):
    global context

    led_components: List[Component] = board.get_components_by_type('led')
    instances: List[str] = []
    rgb_components = board.get_components_by_part_number('RGB')

    for count, led_component in enumerate(led_components):
        led_count = count
        for signal in led_component.signals:
            if not signal.id.startswith('s'):
                # Skip signals such as G_CA and R_CA which are not LED-related
                continue
            # Check if the LED is connected to the Device
            if board.get_peripheral_options(Req('gpio'), led_component.id, {signal.id: None}):
                # For boards with more than 1 RGB LED, append instances alternatively
                # RGB_0      |      RGB_1
                # s0-led0    |     s0-led1
                # s1-led2    |     s1-led3
                # s2-led4    |     s2-led5
                if len(rgb_components) > 1:
                    context[f'led{led_count}'] = {'led': led_component, 'signal': signal}
                    instances.append(f'led{led_count}')
                    led_count = led_count + len(rgb_components)
                else:
                    context[f'led{len(instances)}'] = {'led': led_component, 'signal': signal}
                    instances.append(f'led{len(instances)}')

    if any([re.search(boards_with_exp_leds, b.name) for b in board.board_components]):
        # Add Z-Wave expansion header LEDs
        for signal, instance in exp_leds.items():
            if board.has_component('exp_h') and board.get_peripheral_options(Req('gpio'), 'exp_h', {signal: None}):
                led_component = board.board.get_component('exp_h')
                if instance in context:
                    # If the LED instance already exists, move it down to a lower instance
                    context[f'led{len(instances)}'] = context[instance]
                    instances.append(f'led{len(instances)}')
                context[instance] = {'led': led_component, 'signal': led_component.get_signal(signal)}
                if instance not in instances:
                    instances.append(instance)

    return instances


def configure(project: ProjectConfig, board: Hardware, instance: str):
    logger.info("Configure instance {}".format(instance))

    global context

    signal = context[instance]['signal']
    led_component = context[instance]['led']
    board_component = led_component.id

    active_high = True
    if signal.properties.get('polarity', 'positive') == 'negative':
        active_high = False

    req = project.requirement('SL_SIMPLE_LED_{}'.format(instance.upper()))

    options = board.get_peripheral_options(req, board_component, {
        signal.id: None
    })
    project.satisfy_requirement(req, [options[0][signal.id]['pins'][0]])

    if 'inverted' in options[0][signal.id]['properties']:
        active_high = not active_high

    if not active_high:
        project.config('SL_SIMPLE_LED_{}_POLARITY'.format(instance.upper())).value = 'SL_SIMPLE_LED_POLARITY_ACTIVE_LOW'

