special_case = {
    # Update configuration for BRD4726A as mentioned in RDMAP-6269
    "BRD4276A": {
        "voltage_mv": '3300',
        "power_deci_dbm": '250',
        "ramp_time_us": '44',
        "pa_curve_header": 'efr32xg25/sl_rail_util_pa_curves_brd4276a.h'
    }
}

def compatible(provides, board):
  if board.board_components and board.provides('device_is_module'):
    # There are board components that are not device components; this is a
    # <board>_config component. Modules should get their config file from the
    # <module>_config component instead.
    return False

  if board.provides('device_is_module') and board.provides('device_has_radio'):
    # Module with radio should be overridden
    return True
  if board.board.radio_config:
    # Board with radio should be overridden
    return True

  return False

def configure(project, board, _):
  # PAVDD configuration
  if board.board.radio_config.get('paconfig') == 'DCDC':
    project.config('SL_RAIL_UTIL_PA_VOLTAGE_MV').value = '1800'
  elif board.board.radio_config.get('paconfig') == 'VLDO':
    project.config('SL_RAIL_UTIL_PA_VOLTAGE_MV').value = '3600'
  else:
    project.config('SL_RAIL_UTIL_PA_VOLTAGE_MV').value = '3300'

  # For special cases, the configuration should override
  board_opn = board.board.board_no
  if board.board.board_no in special_case:
      if 'voltage_mv' in special_case[board_opn]:
          project.config('SL_RAIL_UTIL_PA_VOLTAGE_MV').value = special_case[board_opn]['voltage_mv']
      if 'power_deci_dbm' in special_case[board_opn]:
          project.config('SL_RAIL_UTIL_PA_POWER_DECI_DBM').value = special_case[board_opn]['power_deci_dbm']
      if 'ramp_time_us' in special_case[board_opn]:
          project.config('SL_RAIL_UTIL_PA_RAMP_TIME_US').value = special_case[board_opn]['ramp_time_us']
      if 'pa_curve_header' in special_case[board_opn]:
          pa_curve_header_path = special_case[board_opn]['pa_curve_header']
          project.config('SL_RAIL_UTIL_PA_CURVE_HEADER').value = f'"{pa_curve_header_path}"'

  # PA selection
  support_2p4 = False
  support_subgig = False
  for port in board.board.radio_config.get('rf-ports', []):
    if 2400 in port:
      support_2p4 = True
    if any(freq < 1000 for freq in port):
      support_subgig = True

  if not support_2p4:
    project.config('SL_RAIL_UTIL_PA_SELECTION_2P4GHZ').value = 'RAIL_TX_POWER_MODE_NONE'
    # SL_RAIL_UTIL_PA_SELECTION_2P4GHZ
        # RAIL_TX_POWER_MODE_2P4_HP
        # RAIL_TX_POWER_MODE_2P4_MP (Panther only)
        # RAIL_TX_POWER_MODE_2P4_LP
        # RAIL_TX_POWER_MODE_NONE

  if not support_subgig:
    project.config('SL_RAIL_UTIL_PA_SELECTION_SUBGHZ').value = 'RAIL_TX_POWER_MODE_NONE'
    # SL_RAIL_UTIL_PA_SELECTION_SUBGHZ
        # RAIL_TX_POWER_MODE_SUBGIG
        # RAIL_TX_POWER_MODE_NONE
