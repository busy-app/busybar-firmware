import re

supported_sdid = ['device_sdid_205','device_sdid_215', 'device_sdid_230']

def check_aox_config_support(provides):
    for sdid in supported_sdid:
        if sdid in provides:
            return True
    return False

def compatible(provides, hw):
    # Todo: Added this supported_sdid to avoid missing config files for error efr32xg28 which boards have rfswitch component but did not have source config files
    # make this proper by returning error/warning while checking project config files in project_config.py:36
    if not check_aox_config_support(provides):
        return False

    switches = []
    for rfswitch in ['rfswitch', 'rfswitch-sp16ts']:
        switches = hw.get_components_by_type(rfswitch)
        if switches:
            break

    # AOA boards are not described in a consitent way in t207_hardware_description
    # Different conditions applies depending on the type of board / description
    # Some boards have 5 ctrl pins for 1 switch while some have 1 ctrl pin for each switch of 5.
    # This different conditions handled in configure(), have to update them if requested by rail team.
    if len(switches) != 0 or hw.get_components_by_type('aoamatrix'):
        return True

    return False


def configure(project, hw, _):
    antenna_select = 'SL_RAIL_UTIL_AOX_ANTENNA_PIN{}'

    if hw.provides('brd4185a'):
        switches = hw.get_components_by_type('rfswitch')
        # Assume that the pinout is such that the lowest bits for round-robin switching
        # are connected to the highest-numbered RF switches
        # Example: BRD4185A
        # B0 = switch 1-4 CTL2
        # B1 = switch 1-4 CTL1
        # B2 = switch 0 CTL2
        # B3 = switch 0 CTL1
        antenna_pins = []
        for switch in reversed(switches):
            for signal in reversed(switch.signals):
                # Multiple switches are connected to the same antenna pins, only emit config once
                if signal.device_connection and signal.device_connection.id not in antenna_pins:
                    req = project.requirement(antenna_select.format(len(antenna_pins)))
                    options = hw.get_peripheral_options(req, switch.id, {signal.id: None})
                    project.satisfy_requirement(req, [options[0][signal.id]['pins'][0]])

                    antenna_pins.append(signal.device_connection.id)

        project.config('SL_RAIL_UTIL_AOX_ANTENNA_PIN_COUNT').value = str(len(antenna_pins))

    elif any([re.search(r'brd4191[a-z]', c.name) for c in hw.board_components]):
        # For BRD4191x the CTRL1-6 signals should be connected, not CTRL7
        # CTRL1-4 are connected to CHW-SP16TS (rfswitch_4 and rfswitch_5), we need to configure pins only once
        # CTRL5 is connected to rfswitch_1.ctl1
        # CTRL6 is connected to rfswitch_0.ctl1
        required_signals = [
            ('rfswitch-sp16ts_0', ['d1', 'd2', 'd3', 'd4']),
            ('aoamatrix', ['CHW', 'CPDP'])
        ]
        antenna_pins_cnt = 0

        for switch, signals in required_signals:
            for signal in signals:
                req = project.requirement(antenna_select.format(antenna_pins_cnt))
                options = hw.get_peripheral_options(req, switch, {signal: None})
                project.satisfy_requirement(req, [options[0][signal]['pins'][0]])

                antenna_pins_cnt += 1

            project.config('SL_RAIL_UTIL_AOX_ANTENNA_PIN_COUNT').value = antenna_pins_cnt
    else:
        required_signals = [
            ('BGS12WN6', ['ctl1']),
            ('CG2179M2', ['ctl1', 'ctl2']),
            ('CHW-SP16TS-B', ['d0', 'd1', 'd2', 'd3', 'd4']),
            ('SKY13377', ['ctl1', 'ctl2']),
            ('SP16T', ['d0', 'd1', 'd2', 'd3', 'd4']),
            ('SW4000-3GOPA-002-SX', ['ctl1', 'ctl2']),
            ('SW4000-SPDT-38-SX', ['ctl1']),
            ('SKY13575-639LF', ['ctl1', 'ctl2']),
            ('SKY13348-374LF', ['ctl1', 'ctl2']),
        ]

        antenna_pins = {}
        for part_number, signals in required_signals:
                switch = hw.get_components_by_part_number(part_number)
                if not switch:
                    continue
                for signal in signals:
                    req = project.requirement(antenna_select.format(0))
                    options = hw.get_peripheral_options(req, switch[0].id, {signal: None})
                    antenna_pins[options[0][signal]['pins'][0]] = ''

        if not antenna_pins:
            raise Exception('This board is not supported')

        for count, pin in enumerate(antenna_pins):
            req = project.requirement(antenna_select.format(count))
            project.satisfy_requirement(req, [pin])

        project.config('SL_RAIL_UTIL_AOX_ANTENNA_PIN_COUNT').value = len(antenna_pins)
