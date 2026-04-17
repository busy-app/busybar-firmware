import yaml
import os
import logging

logger = logging.getLogger(__name__)

board_data_file = os.path.dirname(__file__) + '/../util/board_data.yaml'
if os.path.isfile(board_data_file):
    with open(board_data_file, 'r') as f:
        board_data = yaml.safe_load(f)
else:
    raise Exception('Unable to load board data containing tuning information')

def find_hfxo(hw):
    xtals = hw.board.get_component_by_type('xtal')
    for xtal in xtals:
        if xtal.frequency > 32768:
            return xtal
    else:
        if hw.module is not None:
            xtals = hw.module.get_component_by_type('xtal')
            for xtal in xtals:
                if xtal.frequency > 32768:
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
    if hw.board.get_component_by_type('xtal', {'part_number': '7Z-38.400MBG-T'}):
        return True
    if hw.provides('device_series_3'):
        return False
    board_id = get_board_id(hw)
    if board_id in board_data:
        return True

    hfxo = find_hfxo(hw)
    if hfxo and hw.provides('device_is_module'):
        # Component to which hfxo is connected
        hfxoConnections = [signal.connections[0].component.type for signal in hfxo.signals]
        # Make sure all hfxo connection goes to the MCU/SoC
        if all(connect in ['efr32', 'efm32'] for connect in hfxoConnections):
            return True
        else:
            return False
    if hfxo and hfxo.ctune is not None:
        return True

    return False

def configure(project, hw, _):
    hfxo = find_hfxo(hw)
    board_id = get_board_id(hw)

    if hw.board.get_component_by_type('xtal', {'part_number': '7Z-38.400MBG-T'}):
        # TCXO needs HFXO peripheral to be in 'External' mode
        project.config('SL_DEVICE_INIT_HFXO_MODE').value = 'cmuOscMode_External'

    if hfxo.ctune is not None:
        # First try to find the ctune value from HwT board yaml file
        project.config('SL_DEVICE_INIT_HFXO_CTUNE').value = hfxo.ctune
    elif board_id in board_data:
        # Find previously used ctune value extracted from hardware/kit files
        d = board_data[board_id]
        if 'hfxo_ctune' in d:
            project.config('SL_DEVICE_INIT_HFXO_CTUNE').value = d['hfxo_ctune']
    else:
        logger.warning('No hfxo ctune value found for {}'.format(board_id))
    project.config('SL_DEVICE_INIT_HFXO_FREQ').value = hfxo.frequency

    # The `hardware_board` component is provided within the board component,
    # as both the module and board components contain overrides for the device_init_hfxo files.
    # Using the 'hardware_board' provide of the board component,
    # Module Overrides device_init_hfxo header files are not included for the board, it is blocked using unless 'hardware_board'
    if hw.provides('device_is_module') and not hw.provides('hardware_board'):
        project.unless.append('hardware_board')
