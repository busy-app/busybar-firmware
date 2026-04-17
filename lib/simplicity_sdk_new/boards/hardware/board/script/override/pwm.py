#!/usr/bin/env python3

class Req():
    def __init__(self, t, p=None):
        self.type = t
        self.properties = {}
        if p:
            self.properties.update(p)
        self.name = ''

def compatible(provides, board):
    if any([board.provides(brd) for brd in ('brd2601a', 'brd2601b')]):
        # Handle the RGB led as 3 pwm instances:
        return ['led0', 'led1', 'led2']

    instances = []

    if board.has_component('led') and not board.has_component('led_0') and \
            board.get_peripheral_options(Req('gpio'), 'led', {'s0': None}):
        instances.append('led0')
    if board.has_component('led_0') and board.get_peripheral_options(Req('timer', p={'channel': 'OUTPUT'}), 'led_0', {'s0': 'CC'}):
        instances.append('led0')
    if board.has_component('led_1') and board.get_peripheral_options(Req('timer', p={'channel': 'OUTPUT'}), 'led_1', {'s0': 'CC'}):
        instances.append('led1')
    if board.has_component('mikroe') and board.get_peripheral_options(Req('timer', p={'channel': 'OUTPUT'}), 'mikroe', {'pwm': 'CC'}):
        instances.append('mikroe')

    return instances

def configure(project, board, instance_name):
    # Convert SW instance name to board component name
    if any([board.provides(brd) for brd in ('brd2601a', 'brd2601b')]):
        board_component_name = 'led_0'
        signal = 's{}'.format(instance_name[-1])
    elif board.provides('brd4178a'):
        board_component_name = 'led'
        signal = 's{}'.format(instance_name[-1])
    elif instance_name == 'mikroe':
        board_component_name = 'mikroe'
        signal = 'pwm'
    else:
        if board.has_component('led') and not board.has_component('led_0'):
            board_component_name = 'led'
            signal = 's0'
        else:
            board_component_name = 'led_{}'.format(instance_name[-1])
            signal = 's0'
    led_component = board.board.get_component(board_component_name)

    config_prefix = 'SL_PWM_{}'.format(instance_name.upper())
    requirement = project.requirement(config_prefix)

    active_high = True
    if led_component.get_signal(signal).properties.get('polarity') == 'negative':
        active_high = False

    # Find all device peripherals that fulfil the following:
    #  - Fulfil the SIMPLE_PWM_LEDn requirement
    #  - Have a CC* signal connected to the <signal> signal of the led_n board component
    signal_options = board.get_peripheral_options(
        requirement,
        board_component_name,
        {signal: 'CC'}
    )

    # Select different timers for the different LEDs
    # This is done to avoid selecting TIMER0 CC0 for every LED instance
    try:
        instance_index = int(instance_name[-1])
    except ValueError:
        instance_index = -1
    if instance_index < len(signal_options):
        signal_map = signal_options[instance_index]
    else:
        signal_map = signal_options[0]

    # Select the first matching location for the <signal> signal (LED pin)
    location = signal_map[signal]['locations'][0]

    if 'inverted' in signal_map[signal]['properties']:
        active_high = not active_high

    # Register the set of options as fulfilling the requirement
    project.satisfy_requirement(requirement, [location])

    # Set the channel name
    # TODO Refactor when channel selection is done differently
    channel = location.route.name[-1]
    location.route.peripheral.properties['channel.{}.name'.format(channel)].value = 'OUTPUT'

    # If LED is wired with ground side to device, reconfigure polarity
    if not active_high:
        project.config(config_prefix + '_POLARITY').value = 'PWM_ACTIVE_LOW'
