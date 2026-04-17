
def compatible(provides, hw):
  if hw.board.radio_config is None:
    # No radio config
    return False

  if hw.provides('device_is_module'):
    return False

  if hw.has_component('rfswitch'):
    return True

  rf_paths = hw.board.radio_config.get('rf-paths')
  if rf_paths is None:
    # No RF path information
    return False

  if 2400 in rf_paths:
    default_path = rf_paths[2400][0]
    if default_path not in ['RF2G4_IO2', 'RF2G4_IO']:
      # Lynx default is RF2G4_IO
      # Rest of Series 2 default is RF2G4_IO2
      return True

  return False

def configure(project, hw, _):

  if hw.provides('device_generic_family_efr32xg28'):
    #The switch on RFPATH0 in margay OPNs is not an antenna diversity switch, it is intended to assist with SUBG emissions.
    return

  if hw.has_component('rfswitch'):
    signals = {
      'ctl1': '(ANT0)',
      'ctl2': '(ANT1)',
    }
    req = project.requirement('SL_RAIL_UTIL_ANT_DIV')
    opts = hw.get_peripheral_options(req, 'rfswitch', signals)

    # Select first matching Peripheral
    locations = opts[0]
    locs = []

    for b_signal in signals.keys():
        if b_signal in locations.keys():
            locs.append(locations[b_signal]['locations'][0])

    project.satisfy_requirement(req, locs)

  else:
    # Configure MODEM antenna routing
    project.requirement('SL_RAIL_UTIL_ANT_DIV').peripheral = hw.device.peripherals.get('MODEM')
