from typing import Set
import siliconlabs.slc.board_gen.hardware as Hardware
import siliconlabs.slc.board_gen.project_config as ProjectConfig
from siliconlabs.slc.board_gen.util.board_gen_util import Req

ccs811_name = 'CCS811'
wake_signal = 'wake'

def compatible(provides: Set[str], board: Hardware) -> bool:
    """
    Determine if the board passed as argument is compatible with the ccs811_driver component
    @param Set[str] provides: Set of 'provides' defined within the board and its components
    @param Hardware board: Board descption object
    @return: a boolean inidicating if the board is compatible with the ccs881_driver component
    """

    if not board.has_tag('hardware:has:sensor:ccs811'):
        return False

    # Retrieve the component using it's part number (as its type is sensor)
    ccs811_comp = board.get_components_by_part_number(ccs811_name)[0]
    return board.get_peripheral_options(Req('gpio'), ccs811_comp.id, {wake_signal: None, })

def configure(project: ProjectConfig, board: Hardware, instance_name: str):
    """
    Generate the ccs881_driver configuration header for a given board
    @param ProjectConfig project: Project containing the requirements to be satisfied
    @param Hardware board: Board description object
    @param str instance_name: instance name - None in this case as the component is not instanciable
    @return:
    """
    req = project.requirement('SL_CCS811_WAKE')
    # Retrieve the component using it's part number (as its type is sensor)
    ccs811_comp = board.get_components_by_part_number(ccs811_name)[0]
    opts = board.get_peripheral_options(req, ccs811_comp.id, {wake_signal: None, })
    if opts:
        pin = opts[0][wake_signal]["pins"][0]
        project.satisfy_requirement(req, [pin])