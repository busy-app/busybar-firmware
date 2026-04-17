#!/usr/bin/env python

try:
  from Studio import halConfig as hc
  from Studio import project, stack
except:
  pass

import traceback
import argparse
import shutil
import glob
import sys
import os
import xml.etree.ElementTree as ET
import subprocess
import re

def _locate_studio_script():
  """
  Locate the Studio installation and its scripting interface entry point

  @return Path to scripting interface entry point
  """
  studio_loc = os.environ.get('STUDIO_LOCATION')

  import platform

  if not studio_loc:
    # Default to default install path
    if platform.system() == 'Windows':
      studio_loc = 'C:/SiliconLabs/SimplicityStudio/v4/'
    elif platform.system() == 'Darwin':
      studio_loc = '/Applications/'

  print(platform.system())
  if platform.system() == 'Windows' or 'CYGWIN' in platform.system():
    studio_script = os.path.join(studio_loc, 'developer/scripting/runScript.bat')
  elif platform.system() == 'Linux':
    studio_script = os.path.join(studio_loc, 'developer/scripting/runScript.sh')
  elif platform.system() == 'Darwin':
    studio_script = os.path.join(studio_loc, 'Simplicity Studio.app/Contents/Eclipse/developer/scripting/runScript.sh')

  if not os.path.exists(studio_script):
    raise ValueError('[ERROR] Studio scripting interface not found at {} -- is STUDIO_LOCATION set correctly?'.format(studio_script))

  return studio_script

def cygpath(path):
    """
    Convert a cygwin path to a windows path. The conversion is done using the
    cygpath command.

    Parameters:
    path -- The cygwin path to convert

    Returns: the windows path of the input parameter.
    """
    command = ['cygpath', '-m', path]
    output = subprocess.check_output(command)
    winpath = output.splitlines()[0]
    return winpath

def create_project(sdk):
  """
  Create a Studio project to use as the parent for all hwconf file regeneration

  @param sdk: The SDK to use for regeneration
  """
  slsproj_template = os.path.join(os.path.dirname(__file__), 'template.slsproj')
  options = {
    project.OPTION_SDK_ID: sdk.getId()
  }
  p = project.importSlsProject(slsproj_template, options, None)

  return p


def enable_all_peripherals(path):
  """
  Go through a .hwconf file and emit enable properties for all modules that have configuration

  @param path: path to the .hwconf file
  """
  ET.register_namespace('xmi', "http://www.omg.org/XMI")
  ET.register_namespace('device', "http://www.silabs.com/ss/hwconfig/document/device.ecore")
  tree = ET.parse(path)
  root = tree.getroot()
  mode = root.find("mode")

  already_enabled_modules = []
  # Force enable PA so that it will generate BSP_PA_VOLTAGE even when default
  enable_modules = ['PA']

  for prop in mode.iter('property'):
    module = prop.attrib.get('object')
    setting = prop.attrib.get('propertyId')

    if module in ['PORTIO'] or setting.startswith('ports.settings.'):
      # Skip PORTIO and pin settings
      continue

    if setting == 'ABPeripheral.included':
      already_enabled_modules.append(module)
    if module not in enable_modules:
      enable_modules.append(module)

  for module in enable_modules:
    if module not in already_enabled_modules:
      e = ET.SubElement(mode, 'property')
      e.set('object', module)
      e.set('propertyId', 'ABPeripheral.included')
      e.set('value', 'true')

  tree.write(path, xml_declaration=True, encoding='ASCII')


def generate_halconf_header(f, proj):
  """
  Generate HAL config header from a single .hwconf file

  @param hwconf_original: Path to the .hwconf file to generate from
  @param proj:            Studio project to perform generation in
  """
  # Get location of project on disk
  project_uri = proj.getLocationURI()
  project_dir = project_uri.getPath()
  # for some reason the location uri on windows does not look like a correct
  # uri, it's missing a '/'. The format ends up beeing "file:/C:/whatever"
  # instead of file://C:/whatever, so we need to parse it manually.
  match = re.match(r'file:/([A-Za-z]:.*)', str(project_uri))
  if match:
    project_dir = match.group(1)

  print("Project: " + project_dir)
  out_dir, basename = os.path.split(f)

  # Copy .hwconf file into project
  shutil.copy(f, project_dir)

  hwconf_file = os.path.join(project_dir, basename)
  out_dir = os.path.normpath(os.path.join(out_dir, '..'))
  print("Output: " + out_dir)

  # Enable all peripherals in .hwconf file
  enable_all_peripherals(hwconf_file)

  # Remove existing header file
  try:
    os.remove(os.path.join(out_dir, 'config', 'hal-config-board.h'))
  except OSError:
    pass

  # Load hwconf file in project
  dev = hc.loadDocument(hwconf_file, True)
  options = {
    'output_dir': 'config',
    'output_file': 'hal-config-board.h',
    'skip_prefix': ['HAL_']
  }

  hc.generateSource(out_dir, options)


def generate_halconf_headers(prefix=None):
  """
  Generate HAL config headers for all .hwconf files on the path with a given prefix

  @param prefix: Path prefix (e.g. EFR32MG1_ or SLSTK)
  """
  # Load SDK
  sdk_path = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..', '..'))
  try:
    sdk = stack.getSDKForStack(sdk_path)
    if not sdk:
      sdk = stack.scanStack(sdk_path, None)
  except:
    print("[ERROR] Could not load SDK from {}".format(sdk_path))
    traceback.print_stack()
    sys.exit(1)

  proj = create_project(sdk)

  root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
  if prefix:
    kit_pattern = '{}*'.format(prefix)
  else:
    kit_pattern = '*'
  hwconf_files = glob.glob("{}/{}/config/*.hwconf".format(root_dir, kit_pattern))
  for f in hwconf_files:
    print("===== {} =====".format(os.path.basename(f)))
    generate_halconf_header(f, proj)


if __name__ == '__main__':
  # Deduce whether we're running in Studio or not
  in_studio = False
  if 'jython' in os.__file__:
    in_studio = True

  parser = argparse.ArgumentParser()
  parser.add_argument('-p', '--prefix', help='Device prefix to use to filter regenerated files')
  args = parser.parse_args()

  if in_studio:
    generate_halconf_headers(prefix=args.prefix)
  else:
    # Check that Studio is available
    studio_script = _locate_studio_script()

    # Create temporary workspace
    import tempfile
    workspace = tempfile.mkdtemp()
    file_path = os.path.abspath(__file__)

    # Convert cygwin filepaths like /cygdrive/c/work to windows path like C:/work
    if os.getcwd().startswith('/cygdrive'):
      workspace = cygpath(workspace)
      file_path = cygpath(file_path)

    cmd = [
      studio_script,
      '-data',
      workspace,
      file_path
    ]

    if args.prefix:
      cmd += ['-p', args.prefix]

    import subprocess
    process = subprocess.Popen(cmd)
    process.communicate()

    # Delete temporary workspace
    shutil.rmtree(workspace)

    sys.exit(process.returncode)
