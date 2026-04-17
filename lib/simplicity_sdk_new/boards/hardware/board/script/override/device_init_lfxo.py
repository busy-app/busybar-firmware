import yaml
import os

board_data_file = os.path.dirname(__file__) + '/../util/board_data.yaml'
if os.path.isfile(board_data_file):
    with open(board_data_file, 'r') as f:
        board_data = yaml.safe_load(f)
else:
    raise Exception('Unable to load board data containing tuning information')

def find_lfxo(hw):
    xtals = hw.board.get_component_by_type('xtal')
    for xtal in xtals:
        if xtal.frequency == 32768 and xtal.mounted:
            return xtal
    else:
        if hw.module is not None:
            xtals = hw.module.get_component_by_type('xtal')
            for xtal in xtals:
                if xtal.frequency == 32768:
                    return xtal
    return None

def get_board_id(hw):
    # Crude way of getting the actual board id that we are trying to generate for
    # some boards like brd4182a have multiple revisions which we don't care about
    # so we strip off the last part of the name which is separate by underscore.
    board_id = ''
    for bc in hw.board_components:
        if bc.name != 'brd4001a' and bc.name != 'brd4002a':
            board_id = bc.name
    return board_id.split('_')[0]

def compatible(provides, hw):
    board_id = get_board_id(hw)
    # if board_id in board_data and 'lfxo_ctune' in board_data[board_id]:
    #     return True

    if hw.provides('device_series_3'):
        return False
    lfxo = find_lfxo(hw)
    if lfxo:
        return True

    return False

def configure(project, hw, _):
    board_id = get_board_id(hw)
    lfxo = find_lfxo(hw)
    if lfxo is not None and lfxo.ctune is not None:
        project.config('SL_DEVICE_INIT_LFXO_CTUNE').value = lfxo.ctune
    elif board_id in board_data:
        d = board_data[board_id]
        if 'lfxo_ctune' in d:
            project.config('SL_DEVICE_INIT_LFXO_CTUNE').value = d['lfxo_ctune']

    # The `hardware_board` component is provided within the board component,
    # as both the module and board components contain overrides for the device_init_lfxo files.
    # Using the 'hardware_board' provide of the board component,
    # Module Overrides device_init_lfxo header files are not included for the board, it is blocked using unless 'hardware_board'
    if hw.provides('device_is_module') and not hw.provides('hardware_board'):
        project.unless.append('hardware_board')
