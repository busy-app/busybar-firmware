
def compatible(provides, hw):
  if hw.board.radio_config is None:
    # No radio config
    return False

  rf_paths = hw.board.radio_config.get('rf-paths')
  if rf_paths is None:
    # No RF path information
    return False

  # BRD4188A has rfswitch but no rf-paths, so keep this condition
  # below rf-paths to prevent a KeyError in configure()
  if hw.has_component('rfswitch'):
    return True


  if 2400 in rf_paths:
    default_path = rf_paths[2400][0]
    if default_path not in ['RF2G4_IO2', 'RF2G4_IO']:
      # Lynx default is RF2G4_IO
      # Rest of Series 2 default is RF2G4_IO2
      return True

  return False

def configure(project, hw, _):
    # Configure internal RF path
    antenna = 'RAIL_ANTENNA_1'

    rf_path = hw.board.radio_config['rf-paths'][2400][0]
    if rf_path == 'RF2G4_IO1':
      antenna = 'RAIL_ANTENNA_0'
    elif rf_path == 'RF2G4_IO2':
      antenna = 'RAIL_ANTENNA_1'

    project.config('SL_RAIL_UTIL_RF_PATH_INT_RF_PATH_MODE').value = antenna
