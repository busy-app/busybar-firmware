
switch_control_signal = 'ctl1'
switch_radio_active_signal = 'VDD'

def compatible(provides, hw):
    if hw.provides('device_sdid_235'):
        if hw.has_component('rfswitch'):
            return True

    return False

def configure(project, hw, _):
    # Add switch control pin and radio active pin from rfswitch component
    # A and B variants of margay boards have only switch control pin
    # C variant of margay boards have both switch control pin and radio active pin
    switches = hw.get_components_by_type('rfswitch')
    switch_control_req = project.requirement('SL_RAIL_UTIL_RF_PATH_SWITCH_CONTROL')
    switch_radio_active_req = project.requirement('SL_RAIL_UTIL_RF_PATH_SWITCH_RADIO_ACTIVE')

    switch_control_options = hw.get_peripheral_options(switch_control_req, 'rfswitch', {switch_control_signal: None})
    if len(switch_control_options) != 0:
        switch_control_pin = [switch_control_options[0][switch_control_signal]['pins'][0]]
        project.satisfy_requirement(switch_control_req, switch_control_pin)

    switch_radio_active_options = hw.get_peripheral_options(switch_radio_active_req, 'rfswitch', {switch_radio_active_signal: None})
    if len(switch_radio_active_options) != 0:
        switch_radio_active_pin = [switch_radio_active_options[0][switch_radio_active_signal]['pins'][0]]
        project.satisfy_requirement(switch_radio_active_req, switch_radio_active_pin)
