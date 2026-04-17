import yaml
import os
import re
import logging

logger = logging.getLogger(__name__)

boards = [
  'brd4165b', # Lumen 1 board does not wire up the DCDC
  'brd4319a', # Bobcat lightning board does not wire up the DCDC
  'brd4179b', # This board is using the EFP
  'brd2506a', # This board does not wire up the DCDC
]

modules = [
    'mgm240l022rnf', # Bobcat lightning module does not have DCDC
    'mgm240l022vnf'  # Bobcat lightning module does not have DCDC
]

module_data_file = os.path.dirname(__file__) + '/../util/module_data.yaml'
if os.path.isfile(module_data_file):
    with open(module_data_file, 'r') as f:
        module_data = yaml.safe_load(f)
    logger.slc('Sucuessfully opened the module data file')
else:
    logger.warning('Unable to load module data file containing dcdc information')
    module_data = {}

def dcdc_power_off_boards(hw):
    for b in boards:
        if hw.provides(b) or hw.provides("{}_revision".format(b)):
            return True
    return False

def dcdc_power_off_modules(hw):
    return any([hw.device_id == module_opn for module_opn in modules])

def dcdc_get_pfmx_ipkavl(hw):
    # This function uses module data gathered from ../util/module_data.yaml file
    # And returns matched module's IPKVAL for DCDC_PFMXCTRL register
    device_id = hw.device_id
    for opn, dcdc_data  in module_data.items():
        if re.search(opn, hw.device_id) != None:
            try:
                return dcdc_data['ipkval']
            except KeyError:
                return False
    return False

def is_boost_board(hw):
    """ Return true if the IC on the board has a Boost DCDC and if it's BOOST_EN signal is connected"""
    if hw.provides('device_dcdc_boost'):
        # Look for a signal connected to BOOST_EN, signal will be None if BOOST_EN is connected to VSS
        signal, prop = hw.board.get_device_connection('efr32', 'BOOST_EN')
        if signal:
            return True
    return False

def compatible(provides, hw):
    if dcdc_power_off_boards(hw) or dcdc_power_off_modules(hw) or is_boost_board(hw):
        return True
    else:
        for opn, dcdc_data in module_data.items():
            if re.search(opn, hw.device_id) != None:
                return True
    return False

def configure(project, hw, _):
    ipkval = dcdc_get_pfmx_ipkavl(hw)
    if ipkval != False:
        project.config('SL_DEVICE_INIT_DCDC_ENABLE').value = '1'
        project.config('SL_DEVICE_INIT_DCDC_PFMX_IPKVAL_OVERRIDE').value = '1'
        project.config('SL_DEVICE_INIT_DCDC_PFMX_IPKVAL').value = ipkval

    # The `hardware_board` component is provided within the board component,
    # as both the module and board components contain overrides for the device_init_dcdc files.
    # Using the 'hardware_board' provide of the board component,
    # Module Overrides device_init_dcdc header files are not included for the board, it is blocked using unless 'hardware_board'
    if hw.provides('device_is_module') and not hw.provides('hardware_board'):
        project.unless.append('hardware_board')
