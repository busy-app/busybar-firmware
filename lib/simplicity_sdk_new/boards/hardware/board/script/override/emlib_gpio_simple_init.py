from siliconlabs.slc.board_gen.util.board_gen_util import Instance, Req
from siliconlabs.slc.board_gen.hardware import Hardware
from siliconlabs.slc.board_gen.project_config import ProjectConfig
from typing import Set, List


def compatible(provides: Set[str], board: Hardware) -> List[str]:
  if not board.has_component('exp_h'):
    return []

  instances = []
  for signal in range(1, 21):
    if board.get_peripheral_options(Req('gpio'), 'exp_h', {str(signal): None}):
      instances.append(f'exp_{signal}')
  return instances


def configure(project: ProjectConfig, board: Hardware, instance_name: str):
  pin_number = instance_name.replace('exp_', '')

  req = project.requirement('SL_EMLIB_GPIO_INIT_{}'.format(instance_name.upper()))

  options = board.get_peripheral_options(req, 'exp_h', {pin_number: None})
  project.satisfy_requirement(req, [options[0][pin_number]['pins'][0]])
