#!/usr/bin/python

import os
import re
import argparse
import yaml

from rail_scripts.rail_model import RAILModel
from rail_scripts.rail_model_types import *
from rail_scripts import rail_model_types

from host_py_rm_studio_internal import RM_Factory, RM_S1_PART_FAMILY_NAMES, RM_S2_PART_FAMILY_NAMES
from pyradioconfig.calculator_model_framework.CalcManager import CalcManager
from pycalcmodel import *

EXCLUDE_BLOCK_LIST = [
]

WRITE_ONLY_REGISTERS = [
]

class RAILAdapter(object):

  _REG_BASES = (
    0x40080000,
    0x21000000,
  )
  _REG_BASES_EFR32XG2x = (
    0xA8000000,
    0xA8010000,
    0xA8020000,
    0xB0000000,
  )
  _REG_BASES_EFR32XG2B = (
    0xA8000000,
    0xA8010000,
    0xA8020000,
    0xA8030000,
    0xB0000000,
  )
  _REG_BASES_EFR32XG25 = (
    0x400C0000,
    0xA8000000,
    0xA8010000,
    0xA8020000,
    0xA8030000,
    0xA8040000,
    0xB0000000,
    0xB5000000,
    0xA6040000
  )

  # NOT AN OPTIMAL SOLUTION:
  # Placeholder for sequence cfg setting in rail_scripts for wifi generation.
  # This is duplicated from YAML sequences.
  _SEQUENCE_CFG_DEFAULT = {
    "MOVSWAP": 0,
    "DISABSRST": 0,
    "HWSTTRIG": 0,
    "HWSTSEL": 0,
    "CNTWRPOS": 16
  }

  _BASEADDR_CFG_DEFAULT = {
    "Name": "BaseAddrCfgDefault",
    "Access": "BaseAddrCfgDefault"
  }

  import os
  current_dir =  os.path.dirname(os.path.abspath(__file__))

  _RADIO_ACTION_WRITE_TMPL = """  0x{0:08X}UL, 0x{1:08X}UL,\n"""

  # ------------------------------------------------------------------------
  def __new__(cls, **kwargs):
    if cls is not RAILAdapter:
      return super(RAILAdapter, cls).__new__(cls)
    if "mphyConfig" in kwargs:
      import rail_scripts.rail_adapter_multi_phy
      return rail_scripts.rail_adapter_multi_phy.RAILAdapter_MultiPhy(**kwargs)
    else:
      raise NotImplementedError("Single phy adapter no longer supported!!!")

  @property
  def modelPopulated(self):
    return self._railModelPopulated

  def pte_script_flag(self):
    return self.pte_script

  def wifi_script_seqacc_flag(self):
    return self.wifi_script_seqacc

  def wifi_script_flag(self):
    return self.wifi_script

  def generateRailModelContext(self):
    modelContext = {}
    for element in self.railModel._elements:
      if type(getattr(self.railModel, element)).__name__ in rail_model_types.supportedClassType:
        getattr(self.railModel, element).resolve(modelContext)
      else:
        modelContext[element] = getattr(self.railModel, element)
    return modelContext

  # -------- Internal ---------------------------------------------------------
  # // TODO: Rename if we ever support anything other than writes
  def _encodeWriteAddress(self, reg_address, write_length=1, radio_action=0):

    if self.pte_script is False and self.wifi_script is False:
      try:
        reg_base = self._regBases[int(reg_address) & self._baseAddrMask]
      except:
        # Toggle the address in the event of trustzone mismatch.
        reg_base = self._regBases[int((reg_address ^ 0x10000000)) & self._baseAddrMask]
      reg_offset = int(reg_address) & self._offsetAddrMask

      encodedAddress = (radio_action << 28) \
                       | (reg_base << 24) \
                       | (write_length << self._contWrPos) \
                       | reg_offset
    else:
      # Don't encrypt register address for PTE output
      # only update address value based on opearation(Write/SET/CLR)
      encodedAddress =  reg_address + radio_action

    return encodedAddress

  def _getRegAddress(self, block, register, suffix=''):
    # Use the RM_Device object to get the absolute register address
    baseAddrString = "self.rm.{}{}.{}.baseAddress".format(block, suffix, register)
    regOffsetString = "self.rm.{}{}.{}.addressOffset".format(block, suffix, register)
    regAddr = eval(baseAddrString) + eval(regOffsetString)
    return regAddr

  def _getRegAddressWithPolarity(self, block, register):
    if self.series == 3:
      try:
        regAddr = self._getRegAddress(block, register, suffix='_NS')
      except (KeyError, AttributeError):
        try:
          regAddr = self._getRegAddress(block, register, suffix='_S')
        except (KeyError, AttributeError):
          regAddr = self._getRegAddress(block, register, suffix='')
    else:
      regAddr = self._getRegAddress(block, register)
    return regAddr

  def _regOutput(self, block, register, value, name=None):
    # Use the RM_Device object to get register address value pairs
    regAddr = self._getRegAddressWithPolarity(block, register)

    if (name is None):
      return (regAddr, value)
    else:
      return (regAddr, value, name)

  def _getRegNameFromFieldName(self, fieldname):
    (block, reg, field) = fieldname.split('.')
    return block + '.' + reg

  def _getRegBasesFromFamily(self, family):
    # Bit positions for base address and opcode
    basePos = 25
    opCodePos = 28
    contWrPos = 16

    if self.series == 1:
      regBases = self._REG_BASES

    elif self.series == 2:
      regBases = self._REG_BASES_EFR32XG2x

      if family in ["sol"]:
        # Sol has 16 base addresses because of its virtual registers defined at the same address.
        # So let's consider basePos is 24 which is not true since bit position 24 is for the VERIFY bit
        # but it's not used anyway
        basePos = 24
        regBases = self._REG_BASES_EFR32XG25
      elif family in ["curl"]:
        regBases = self._REG_BASES_EFR32XG2B
      else:
        pass

    else:
      # Series 3
      basePos = 24
      ymlSource = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'generators/seqacc_regbases.yml')

      try:
        ymlRegBase = yaml.safe_load(open(ymlSource))
      except FileNotFoundError:
        raise Exception("YAML source file {} not found.".format(ymlSource))

      try:
        yml_family = ymlRegBase[family.upper()]
      except KeyError:
        raise Exception("Family part {} not found in {}. Please verify that this part is mentioned in it and it"
                        "contains base addresses.".format(family.upper(), ymlSource))

      try:
        basePos = yml_family['BaseAddrCfgDefault'].get('BasePos', 24)  # get default (24) if parameter not found
        contWrPos = yml_family['BaseAddrCfgDefault'].get('ContWrPos', 16)  # get default (16) if parameter not found
        regBases = yml_family['BaseAddrCfgDefault']['BaseAddr']
      except KeyError:
        raise Exception("Missing base addresses for family part {} in {}.".format(family.upper(), ymlSource))

      if self.wifi_script_seqacc is True:
        config = self._SEQUENCE_CFG_DEFAULT
        self.sequenceCfg = (config.get('MOVSWAP', 0) << 14) | (config.get('DISABSRST', 0) << 13) | (config.get('HWSTTRIG', 0) << 10)\
                | (config.get('HWSTSEL', 0)) << 5 | config.get('CNTWRPOS', 16)
        config = self._BASEADDR_CFG_DEFAULT
        self.baseAddrAccess = config.get('Access', config)
      else:
        self.sequenceCfg = None
        self.baseAddrAccess = None

    maxNumRegBases = 2 ** (opCodePos - basePos)

    if self.series < 3:
      if len(regBases) > maxNumRegBases:
        raise Exception(("Number of register bases ({}) exceeds maximum allowed "
                         "value ({}) for {}").format(len(regBases), maxNumRegBases, family))
      regBases = {key: val for val, key in enumerate(regBases)}
    else:
      if (len(regBases) - 1) > maxNumRegBases:  # subtracting 1 to get rid of 0xFFFFFFFF key
        raise Exception(("Number of register bases ({}) exceeds maximum allowed "
                         "value ({}) for {}").format(len(regBases) - 1, maxNumRegBases, family))

    self._regBases = regBases
    self._contWrPos = contWrPos
    self._offsetAddrMask = (2 ** (self._contWrPos) - 1)
    self._baseAddrMask = self._offsetAddrMask ^ 0xFFFFFFFF

  @staticmethod
  def _getSeriesFromFamily(family):
    if family in ["dumbo", "jumbo", "nerio", "nixi"]:
      series = 1
    elif family in ["panther", "lynx", "ocelot", "bobcat", "leopard", "margay", "caracal", "lion", "sol", "serval", "curl"]:
      series = 2
    else:
      series = 3
    return series

def getParserArgs():
  """
  Build argparse parser and return arguments from sys.argv.
  """
  parser = argparse.ArgumentParser(description="Parse radio configurator output into header file.")
  parser.add_argument('-o', '--output_dir', type=str, default=None, help="The output dir.")
  parser.add_argument('-p', '--phy_name', type=str, default="PHY_RAIL", help="Build configuration for a specific PHY")
  args = parser.parse_args()
  return args

def main():

  args = getParserArgs()
  basePath = os.path.abspath(os.getcwd())

  # Check output information
  outDir = os.path.abspath(args.output_dir)

  radio_configurator = CalcManager("dumbo", "A0")

  # phy_name = args.phy_name
  phy_name = "PHY_Internal_915M_OOK_100kbps"
  adapter='rail_api_2.x'

  print("phy_name is: {}".format(phy_name))
  print("adapter is: {}".format(adapter))

  if phy_name not in radio_configurator.getPhyNames():
    print ("Phy Name {} is not a valid phy. Using PHY_RAIL.".format(phy_name))
    phy_name = "PHY_RAIL"

  instanceDict = {}
  instanceDict["PHY_generated"] = radio_configurator.calculate_phy(phy_name)
  railAdapter = RAILAdapter(dictionary=instanceDict, adapter_name=adapter)
  railAdapter.populateModel()

  outPath = os.path.join(outDir, 'rail_model_out.yml')
  outputDir = os.path.dirname(outPath)

  try:
    os.makedirs(outputDir)
  except OSError:
    if not os.path.isdir(outputDir):
      raise

  railModelContext = railAdapter.generateRailModelContext()
  rail_model_out = yaml.dump(railModelContext)

  with open(outPath, 'w') as fc:
    print ("Creating '{}'...".format(outPath))
    fc.write(rail_model_out)
    fc.close()

  # Script Finished
  return 0

# Call main if necessary
if __name__ == '__main__':
  main()
