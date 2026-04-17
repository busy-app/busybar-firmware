from functools import cmp_to_key

from pyradioconfig.calculator_model_framework.Utils.CalcStatus import CalcStatus
from rail_scripts.rail_adapter import *
from rail_scripts._version import __version__

from rail_scripts import config

from enum import IntEnum

import itertools

# Update kRAILVersion to be used in phyInfoData.
kRAILVersion = 20

class ConcPhyEnum(IntEnum):
  """
  These enumerations must match those in sl_rail_channel_config_entry_type_t
  https://stash.silabs.com/projects/GSDK/repos/gsdk/browse/platform/radio/rail_lib/common/sl_rail_types.h
  """


  CONC_PHY_NONE = 0
  CONC_PHY_BASE = 1
  CONC_PHY_VT = 2
  CONC_PHY_9_6_NON_HOP = 3      # DEPRECATED - Leave for regression purpose
  CONC_PHY_FAST_SWITCH = 4

class ProtocolIDEnum(IntEnum):
  CUSTOM = 0
  EMBERPHY = 1
  THREAD = 2
  BLE = 3
  CONNECT = 4
  ZIGBEE = 5
  ZWAVE = 6
  WISUN = 7

class RAIL_OptArgInput(object):
  channel_timing_name = "channel_timing_name"
  conc_phy_opt_hop = "conc_phy_opt_hop"
  viterbidemod = "MODEM.VITERBIDEMOD"

class RAIL_ConcPhy:
  @staticmethod
  def RAIL_IsConcPhyVt(opt_arguments):
    for optional_argument in opt_arguments:
      if (optional_argument.key == RAIL_OptArgInput.conc_phy_opt_hop) and (int(optional_argument.value) == ConcPhyEnum.CONC_PHY_VT):
        # if virtual concurrent PHY
        return True

    # virtual concurrnet PHY not found
    return False

  @staticmethod
  def RAIL_IsConcPhyFastSw(opt_arguments):
    for optional_argument in opt_arguments:
      if (optional_argument.key == RAIL_OptArgInput.conc_phy_opt_hop) and (int(optional_argument.value) == ConcPhyEnum.CONC_PHY_FAST_SWITCH):
        return True

    return False

def parsePower(value):
  # Just use a big number for maxpower so that comparisons work even if future
  # chips have different max power defines
  if type(value) == str and value == "RAIL_TX_POWER_MAX":
    return 100000

  # Otherwise assume that we were given an int and do some double checking
  assert type(value) == int, "Power must be specified as an integer value"
  return value

class RAILAdapter_MultiPhy(RAILAdapter):

  def __init__(self, **kwargs):
    self.rc_version = kwargs["rc_version"] if "rc_version" in kwargs else None
    setattr(self, "version", __version__)
    self.mphyConfig = kwargs["mphyConfig"]
    self.pte_script = kwargs["pte_script"] if "pte_script" in kwargs else False
    self.wifi_script = kwargs["wifi_script"] if "wifi_script" in kwargs else False
    self.wifi_script_seqacc = kwargs["wifi_script_seqacc"] if "wifi_script_seqacc" in kwargs else False
    regex = re.compile(r'rail_api_(\d+).x')
    # Captures RAIL major version into an int
    # We use "get", to avoid raising "KeyError"; default to "rail_api_2.x"
    adapter_name = kwargs.get("adapter_name", "rail_api_2.x")
    re_match = regex.match(adapter_name)
    if re_match:
      self.rail_version= int(re_match.group(1))
    else:
      raise Exception("Unknown RAIL Adapter name: {}".format(adapter_name))
    rail_model_file = "rail_model_multi_phy_{}x.yml".format(self.rail_version)
    with open(os.path.join(RAILAdapter.current_dir, rail_model_file)) as f:
      self.yamlobject = None
      if hasattr(yaml, 'FullLoader'):
        self.yamlobject = yaml.load(f.read(), Loader=yaml.FullLoader)
      else:
        self.yamlobject = yaml.load(f.read())
    self.railModel = RAILModel(self.yamlobject)
    self._railModelPopulated = False

    self.series = self._getSeriesFromFamily(kwargs['mphyConfig'].get_part_family().lower())
    # Store the register bases in self._regBases
    self._getRegBasesFromFamily(kwargs['mphyConfig'].get_part_family().lower())

  def _encodeAction(self, modemConfig, address, length, values=[], names=[], debug_print=False):
    # Get address for protected field regs on the current chip
    PROTECTED_FIELD_REG_VALUES = {}
    for register in config.PROTECTED_FIELDS[self.partFamily.lower()].keys():
      baseName = register.split('.')[0]
      fieldName = register.split('.')[1]
      try:
        protectedAddress = self._getRegAddressWithPolarity(baseName, fieldName)
      except AttributeError:
        continue
      registerObject = getattr(self.rm, baseName, None)
      if registerObject == None:
        continue
      registerObject = getattr(registerObject, fieldName, None)
      if registerObject == None:
        continue
      protectionMask = 0
      for field in config.PROTECTED_FIELDS[self.partFamily.lower()][register]:
        fieldObject = getattr(registerObject, field, None)
        if fieldObject == None:
          continue
        bitWidth = fieldObject.bitWidth
        bitOffset = fieldObject.bitOffset
        protectionMask |= ((2 ** bitWidth - 1) << bitOffset)
      PROTECTED_FIELD_REG_VALUES[protectedAddress] = { 'protectionMask' : protectionMask, 'name': register }

    while length > 0:
      currentLength = length
      currentValues = values
      currentNames = names
      currentAddress = address
      addressConflict = None
      for protectedAddress in PROTECTED_FIELD_REG_VALUES.keys():
        if (protectedAddress >= currentAddress) and (protectedAddress < (currentAddress + length * 4)):
          if (addressConflict == None) or (protectedAddress < addressConflict):
            addressConflict = protectedAddress

      if addressConflict != None:
        currentLength = int((addressConflict - address) / 4)
        currentValues = values[:currentLength]
        currentNames = names[:currentLength]

      if currentLength > 0:
        newModemConfigEntry = modemConfig.addNewElement()
        try:
          encodedAddress = self._encodeWriteAddress(currentAddress, currentLength)
        except:
          print("Unable to encode {} at address 0x{:8X}.  Check mapping!".format(currentNames[0], currentAddress))
          exit(1)

        if self.rail_version >= 3:
          newModemConfigEntry.length.value = 1 + len(currentValues)

        firstValue = currentValues[0]
        firstName = currentNames[0]
        newModemConfigEntry.encodedAction.value = encodedAddress
        newModemConfigEntry.encodedValues.value = [firstValue]
        newModemConfigEntry.encodedRegNames.value = ["Write: " + firstName]
        # Remember the 'end' in ARRAY[start:end] is EXclusive, while the start is INclusive
        for val in currentValues[1:]:
          newModemConfigEntry.encodedValues.value.append(val)

        for name in currentNames[1:]:
          newModemConfigEntry.encodedRegNames.value.append(name)

      if addressConflict != None:
        if debug_print:
          print("Conflict found on address %08x, trying to set to %08x." % (addressConflict, values[currentLength]))
          print("This is %s, with the following protected fields:" % PROTECTED_FIELD_REG_VALUES[addressConflict]['name'])
          print(config.PROTECTED_FIELDS[self.partFamily.lower()][PROTECTED_FIELD_REG_VALUES[addressConflict]['name']])
          print("Splitting to:")

        protectionMask = PROTECTED_FIELD_REG_VALUES[addressConflict]['protectionMask']

        if self.pte_script is False:
          # First clear out unprotected fields
          if ((values[currentLength] & ~protectionMask) != ~protectionMask) and (protectionMask != (2 ** 32 - 1)):
            encodedAddress = self._encodeWriteAddress(addressConflict, radio_action=config.RADIO_ACTION_AND)
            newModemConfigEntry = modemConfig.addNewElement()
            newModemConfigEntry.encodedAction.value = encodedAddress
            newModemConfigEntry.encodedValues.value = [protectionMask]
            newModemConfigEntry.encodedRegNames.value = ["AND: " + names[currentLength]]
            if self.rail_version >= 3:
              newModemConfigEntry.length.value = 2
            if debug_print:
              print("AND with %08x" % newModemConfigEntry.encodedValues.value[0])

          # We only need to OR things in if the value isn't 0
          if ((values[currentLength] & ~protectionMask) != 0) and (protectionMask != (2 ** 32 - 1)):
            encodedAddress = self._encodeWriteAddress(addressConflict, radio_action=config.RADIO_ACTION_OR)
            newModemConfigEntry = modemConfig.addNewElement()
            newModemConfigEntry.encodedAction.value = encodedAddress
            newModemConfigEntry.encodedValues.value = [values[currentLength] & ~protectionMask]
            newModemConfigEntry.encodedRegNames.value = ["OR: " + names[currentLength]]
            if self.rail_version >= 3:
              newModemConfigEntry.length.value = 2
            if debug_print:
              print("OR with %08x" % newModemConfigEntry.encodedValues.value[0])

        else:
          # For register with protected fields, we first CLR the non-protected fields
          if ((values[currentLength] & ~protectionMask) != ~protectionMask) and (protectionMask != (2 ** 32 - 1)):
            encodedAddress = self._encodeWriteAddress(addressConflict, radio_action=config.RADIO_ACTION_PTE_CLR)
            newModemConfigEntry = modemConfig.addNewElement()
            newModemConfigEntry.encodedAction.value = encodedAddress
            newModemConfigEntry.encodedValues.value = [0xFFFFFFFF & ~protectionMask]
            newModemConfigEntry.encodedRegNames.value = ["CLR: " + names[currentLength]]
            if debug_print:
              print("AND with %08x" % newModemConfigEntry.encodedValues.value[0])

          # Then only SET desried phy value for the non-protected fields
          if ((values[currentLength] & ~protectionMask) != 0) and (protectionMask != (2 ** 32 - 1)):
            encodedAddress = self._encodeWriteAddress(addressConflict, radio_action=config.RADIO_ACTION_PTE_SET)
            newModemConfigEntry = modemConfig.addNewElement()
            newModemConfigEntry.encodedAction.value = encodedAddress
            newModemConfigEntry.encodedValues.value = [values[currentLength] & (0xFFFFFFFF & ~protectionMask)]
            newModemConfigEntry.encodedRegNames.value = ["SET: " + names[currentLength]]
            if debug_print:
              print("OR with %08x" % newModemConfigEntry.encodedValues.value[0])

      # Add one here, to account for the conflicted address. Keep in mind though, we don't
      # actually have to check if we found a conflicted address here though. If we didn't,
      # we're done forming this array, and so we'll fail the while check at the top of this
      # loop with or without checking for a conflicted address.
      length -= (currentLength + 1)
      values = values[currentLength + 1 :]
      address = currentAddress + (currentLength + 1) * 4
      names = names[currentLength + 1 :]

  def optimizeRadioConfig(self, base_info):
    def _optimizeWrite(base_streak, regs_channels, expensive, cheap):
      # Compute cost to move write in word units. The write costs 1 + len words.
      # This results in a net cost based on the number of places it's added vs.
      # the one place it's removed.
      net_cost_to_add = (len(regs_channels) - 1) * (1 + len(base_streak))
      # Now lower our cost. For every add that has a neighboring write on the
      # high or low side, we subtract one word due to continuous write synergy
      for add in regs_channels:
        above = False
        below = False
        for address, _, __ in add:
          above |= (address == base_streak[0][0] + 4 * len(base_streak))
          below |= (address == base_streak[0][0] - 4)
        net_cost_to_add -= 1 if above else 0
        net_cost_to_add -= 1 if below else 0
      # If the net cost is 0, it probably saves time due to write synergy
      if (net_cost_to_add <= 0):
        cheap.extend(base_streak)
      else:
        expensive.extend(base_streak)

    # This algorithm handles a single add poorly. Leave everything in the base
    if len(base_info["add"]) <= 1:
      return

    regs_channels = [x[0] for x in base_info["add"]]
    new_base = []
    add_to_entries = []
    # Chunk base into it's continuous writes, and see
    base_streak = [base_info["base"][0]]
    for registerAddress, registerValue, registerName in base_info["base"][1:]:
      if base_streak == [] or registerAddress == (base_streak[0][0] + 4 * len(base_streak)):
        base_streak.append((registerAddress, registerValue, registerName))
      else:
        _optimizeWrite(base_streak, regs_channels, new_base, add_to_entries)
        base_streak = [(registerAddress, registerValue, registerName)]
    _optimizeWrite(base_streak, regs_channels, new_base, add_to_entries)

    for add in regs_channels:
      add.extend(add_to_entries)
      add.sort()
    base_info["base"] = new_base

    # Move PHYINFO (and associated registers in a continuous write) to the start
    address = self._getRegAddressWithPolarity("SEQ","PHYINFO")
    allWrites = [new_base]
    allWrites.extend(regs_channels)
    for add in allWrites:
      current = address
      writeToMove = []
      for i, register in enumerate(add):
        if register[0] == current:
          writeToMove.append(add.pop(i))
          current += 4
      add[0:0] = writeToMove

    do_print = False # change to True to enable extra debug print
    # Make sure there is no duplicate address
    allWrites = [new_base]
    allWrites.extend(regs_channels)
    if (self.partFamily in ["ocelot", "margay", "serval"]):
      bcrdemoctrlReg = self.rm.MODEM.BCRDEMODCTRL
      viterbidemodReg = self.rm.MODEM.VITERBIDEMOD
    for j, chunkWrite in enumerate(allWrites):
      if (do_print):
        if (j == 0):
          print("Total number of register on 'base configuration' is {}".format(len(chunkWrite)))
        else:
          print("Total number of register on {} is {}".format(base_info["add"][j - 1][1][3].name, len(chunkWrite)))
        print("chunkWrite len {}".format(len(chunkWrite)))
      if (len(chunkWrite) == 0):
        if (do_print):
          print("chunkWrite is empty")
      else:
        if (do_print):
          print("chunkWrite[0]: {}".format(chunkWrite[0]))
        prevAddress = chunkWrite[0]
        for i, register in enumerate(chunkWrite):
          if (i != 0):
            if (prevAddress[0] == register[0]):
              print("Found duplicate address {} @ {}".format(register, i))
              if (prevAddress[1] != register[1]):
                print("Duplicate address value mismatch, {} != {}".format(prevAddress[1], register[1]))
                raise
          prevAddress = register

    if self.partFamily.lower() in ["ocelot", "margay", "serval"]:
      # Conc. PHY optimization
      # for virtual concurrent PHY (i.e. Zwave 9.6K concurrent PHY), this sequence has to be followed when writing the deltaAdd register set:
      #    --- MODEM_SRCCHF, MODEM_BCRDEMODCTRL, MODEM_VITERBIDEMOD has to be written first
      #    --- MODEM_TRECPMPATT has to be written last
      regNameLists = ["SRCCHF", "BCRDEMODCTRL", "VITERBIDEMOD", "CTRL0", "SYNC0", "FRMSCHTIME", "TRECPMPATT"]
      regLists = []
      for regNameList in regNameLists:
        regLists.append(self._getRegAddressWithPolarity("MODEM", regNameList))
      if (do_print):
        for regIdx, reg in enumerate(regLists):
          for add in allWrites:
            for i, register in enumerate(add):
              if register[0] == reg:
                print("Name: {}, Addr: {}, Value: {}, index {}".format(regNameLists[regIdx], register[0], register[1], i))
      regs_channels = [x[0] for x in base_info["add"]]
      # order of the first few register write
      firstWriteLists = ["SRCCHF", "BCRDEMODCTRL", "VITERBIDEMOD"]
      # order of the last few register write
      lastWriteLists = ["TRECPMPATT"]
      # the middle register write will be in ascending address as before (CTRL0, CTRL1, SYNC0, FRMSCHTIME, SQEXT, BCRDEMODCTRL)
      for add_channel in base_info["add"]:
        if (do_print):
          print("Optimizing conc. PHY: {}".format(add_channel[1][3].name))
          #multiPhyConfigEntry.optional_arguments.argument.conc_phy_opt_hop.var_value
        if (add_channel[1][1].entryType.value == ConcPhyEnum.CONC_PHY_VT):
          # if virtual concurrent PHY
          reg_lists = add_channel[0]
          firstWrites = []
          lastWrites = []
          # Find the registers that has to be executed first
          for list in firstWriteLists:
            list_addr = self._getRegAddressWithPolarity("MODEM", list)
            regFound = False
            for i, reg in enumerate(reg_lists):
              if reg[0] == list_addr:
                if (do_print):
                  print("Found Name: {}, Addr: {}, Value: {}".format(list, list_addr, hex(reg[1])))

                firstWrites.append(reg_lists.pop(i))
                regFound = True
            if (regFound != True):
              print("Error {} is not found".format(list))
              raise

          # Find the registers that has to be executed last
          for list in lastWriteLists:
            regFound = False
            list_addr = self._getRegAddressWithPolarity("MODEM", list)
            for i, reg in enumerate(reg_lists):
              if reg[0] == list_addr:
                if (do_print):
                  print("Found Name: {}, Addr: {}, Value: {}".format(list, list_addr, hex(reg[1])))
                lastWrites.append(reg_lists.pop(i))
                regFound = True
            if (regFound != True):
              print("Error: {} is not found".format(list))
              raise

          # Relocate the register update
          reg_lists[1:1] = firstWrites # move the first set of register @ the beginning of deltaAdd list
          for x in lastWrites:
            reg_lists.append(x) # move the last set of register @ the end of deltaAdd list

  def formatModemConfigEntries(self, configName, phyConfigEntry, registerEntries, base=False, subtract=False, commonBase=False, reg_group_name=None):
    if self.rail_version == 1:
      # RAIL 1.x requires an action for every write
      for registerAddress, registerValue in registerEntries:
        self._encodeAction(phyConfigEntry, registerAddress, 1, [registerValue])
    else:
      # RAIL 2.x supports packed radio configurations
      currentAddress = 0
      length = 0
      values = []
      names = []

      # Create a new modemConfig element, and grab appropriate references based on
      # the mode.
      if base == True:
        if self.wifi_script is False and self.wifi_script_seqacc is False:
          if self.rail_version >= 3:
            newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesBase.newElement(configName + "_modem_config_base")
          else:
            newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesBase.newElement(configName + "_modemConfigBase")
        else :
          if self.rail_version >= 3:
            newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesBase.newElement(configName)
          else:
            newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesBase.newElement(configName)
        currentModemConfigs = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesBase
        currentPhyConfigEntryModemConfigEntry = phyConfigEntry.modemConfigEntryBase
      elif subtract == True:
        if self.rail_version < 3:
          newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesSubtract.newElement(configName + "_modemConfigSubtract")
          currentModemConfigs = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesSubtract
          currentPhyConfigEntryModemConfigEntry = phyConfigEntry.modemConfigEntrySubtract
        else:
          raise Exception("phy_config_delta_subtract is not supported in RAIL 3.x")
      elif commonBase == True:
        if self.rail_version >= 3:
          newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesCommonBase.newElement(configName + "_modem_config_common_base")
          currentModemConfigs = self.railModel.multiPhyConfig.commonStructures.modemConfigEntriesCommonBase
          currentPhyConfigEntryModemConfigEntry = phyConfigEntry.modemConfigEntryCommonBase
      elif reg_group_name is not None:
        newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigGroupEntries.newElement(phyConfigEntry.name + f"_modem_config_group_{reg_group_name}")
        currentModemConfigs = self.railModel.multiPhyConfig.commonStructures.modemConfigGroupEntries
        currentPhyConfigEntryModemConfigEntry = phyConfigEntry.modemConfigGroupEntry
      else:
        if self.rail_version >= 3:
          newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntries.newElement(phyConfigEntry.name + "_modem_config")
        else:
          newModemConfig = self.railModel.multiPhyConfig.commonStructures.modemConfigEntries.newElement(phyConfigEntry.name + "_modemConfig")
        currentModemConfigs = self.railModel.multiPhyConfig.commonStructures.modemConfigEntries
        currentPhyConfigEntryModemConfigEntry = phyConfigEntry.modemConfigEntry

      for registerAddress, registerValue, registerName in registerEntries:

        if self.pte_script is False and self.wifi_script is False:
          if (registerAddress - currentAddress) == (4 * length):
            length += 1
            values.append(registerValue)
            names.append(registerName)
          else:
            self._encodeAction(newModemConfig, currentAddress, length, values, names)
            currentAddress = registerAddress
            length = 1
            values = [registerValue]
            names = [registerName]
        else:
          # for PTE output
          # All address will be printed out after removing duplication
          if registerAddress == currentAddress:
            # Do not print out the duplicated address
            pass
          else:
            if (registerAddress > 0xB0000000) and (registerValue == 0) and self.wifi_script is False:
              # For dummy register that ZERO is written into RAM, we will not print out entries
              # ATE program needs to take care of memory intialization before loading phy
              pass
            else:
              self._encodeAction(newModemConfig, currentAddress, length, values, names)
              currentAddress = registerAddress
              length = 1
              values = [registerValue]
              names = [registerName]

      # One final write for whatever is left
      self._encodeAction(newModemConfig, currentAddress, length, values, names)

      # Check the size of the newModemConfig is greater than 0 before adding it
      # to the currentModemConfigs structure
      if len(newModemConfig._elements) > 0:

        # Traverse existing modemConfigEntries and check for duplicates
        entryFound = False
        for modemConfig in currentModemConfigs._elements:
          if modemConfig == newModemConfig:
            # Register the entry with the current phyConfigEntry
            currentPhyConfigEntryModemConfigEntry.value = modemConfig
            entryFound = True
            break

        if not entryFound:
          # Found no duplicates, so add the newModemConfig to the phyInfoEntries object
          currentModemConfigs.addElement(newModemConfig)
          # Register the entry with the current phyConfigEntry
          currentPhyConfigEntryModemConfigEntry.value = newModemConfig

  def _writeBRA(self, phyConfigEntry, model, regs):
    address = self._getRegAddressWithPolarity("FRC", "BLOCKRAMADDR")
    if phyConfigEntry.bchArray:
      # Write BLOCKRAMADDR
      self._loadBchLookupTable(model, regs, address, phyConfigEntry.bchArray)
    else:
      # Write the address of the block decoding table to BLOCKRAMADDR when
      # BLOCKWHITEMODE == BLOCKLOOKUP. Include the register even when the table
      # is absent for speed with continuous write, and make the potential
      # future implementation of RAIL_LIB-1138 easier
      regs.append((address, phyConfigEntry.frameCodingTableEntry.value or 0, "FRC.BLOCKRAMADDR"))
    regs.sort()
    return regs

  def _writeCRA(self, regs, fecEnabled):
    # On Panther, we decided to ALWAYS write the FRC_CONVRAMADDR to HIGH RAM offset 0
    # On rest of series-2 after Panther, we decided to ALWAYS write the FRC_CONVRAMADDR to FRCRAM offset 0
    # Since this varies per part it is now owned in the Radio Configurator in series 2+
    if self.series == 1:
      # Series - 1 point to allocated buffer, or just leave at 0 if not needed.
      if fecEnabled:
        convDecodeBuffer = "convDecodeBuffer"
      else:
        convDecodeBuffer = 0

      # Write the address of the convDecodeBuffer to CONVRAMADDR when
      # fecEnabled, include even when absent for speed
      address = self._getRegAddressWithPolarity("FRC", "CONVRAMADDR")
      regs.append((address, convDecodeBuffer, "FRC.CONVRAMADDR"))
    regs.sort()
    return regs

  def _addRailOwnedRegsToConfigEntries(self, refConfigName, phyConfigEntry, model, regs_base, regs_channel):
    refBaseFecEnabled = False
    bchArrayChanged = False
    frameCodeArrayChanged = False
    # Loop through all base channel configurations that are related to phyConfigEntry via refConfigName
    for baseChannelConfig in self.mphyConfig.base_channel_configurations.base_channel_configuration:
      # Check if refConfigName is referenced by or references iterated baseChannelConfig
      if refConfigName in [baseChannelConfig.name, baseChannelConfig.base_channel_reference]:
        for channelConfigEntry in baseChannelConfig.channel_config_entries.channel_config_entry:
          radioConfigModel = channelConfigEntry.radio_configurator_output_model

          # Grab iterated channel config's FEC & BCH/Frame coding parameters
          chFecEnabled = bool(radioConfigModel.profile.outputs.get_output('fec_enabled').var_value)
          chBchArray = None
          if getattr(radioConfigModel.profile.outputs, 'bch_lut_data', None):
            chBchArray = radioConfigModel.profile.outputs.get_output('bch_lut_data').var_value

          chFrameCodingArray = None
          generic_lut = getattr(model.profile.outputs, 'generic_lookup_tables', None)

          try:
            var_value = getattr(generic_lut, 'var_value', None)
            if var_value and 'frameCodingTable' in var_value:
              chFrameCodingArray = generic_lut.var_value['frameCodingTable']
          except(AttributeError, KeyError):
            pass

          # Check if FEC is enabled on any channel config entry related to refConfigName
          if chFecEnabled:
            refBaseFecEnabled = True
          # Check if BCH array is present & different on any channel config entry related to refConfigName
          if chBchArray and phyConfigEntry.bchArray and chBchArray != phyConfigEntry.bchArray:
            bchArrayChanged = True
          # Check if frame coding array is present & different on any channel config entry related to refConfigName
          if chFrameCodingArray and phyConfigEntry.arrayTable and chFrameCodingArray != phyConfigEntry.arrayTable:
            frameCodeArrayChanged = True

    # FRC.CONVRAMADDR is always written to base since it doesn't really need to change phy to phy
    regs_base = self._writeCRA(regs_base, refBaseFecEnabled)
    # FRC.BLOCKRAMADDR written to base or channel config depending on whether any change is seen
    # across related channel configs
    if bchArrayChanged or frameCodeArrayChanged:
      regs_channel = self._writeBRA(phyConfigEntry, radioConfigModel, regs_channel)
    else:
      regs_base = self._writeBRA(phyConfigEntry, radioConfigModel, regs_base)
    return regs_base, regs_channel

  def _generateModemConfigEntries(self, phyConfigEntry, model, regs):
    if len(regs) > 0:

      if self.pte_script is False and self.wifi_script is False and self.wifi_script_seqacc is False:
        if self.rail_version < 3:
          # Write the address of the phyInfo structure to SEQ.PHYINFO.ADDRESS
          address = self._getRegAddressWithPolarity("SEQ","PHYINFO")
          regs.append((address, phyConfigEntry.phyInfoEntry.value, "SEQ.PHYINFO"))

      # Write the address of the last Dynamic Slicer Configuration link
      # if there are any present, exclude the write if false
      dynamicSlicerTableEntry = phyConfigEntry.dynamicSlicerTableEntry.value
      if dynamicSlicerTableEntry and len(dynamicSlicerTableEntry._elements) > 0 and \
         self.partFamily in ["dumbo", "jumbo", "nerio", "nixi", "bobcat","caracal"]:
          address = self._getRegAddressWithPolarity("SEQ","DYNAMIC_CHPWR_TABLE")
          regs.append((address, phyConfigEntry.dynamicSlicerTableEntry.value.lastElement, "SEQ.DYNAMIC_CHPWR_TABLE"))

      regs.sort() # Put the registers in the right order again
    return regs

  # Takes a model instance and writes it to a rm object
  def _writeModelToRmDevice(self, isBaseConfig, baseChannelConfig, channelConfigEntry, phyConfigEntry, rm, debug_print=False):

    if isBaseConfig:

      if baseChannelConfig.base_channel_reference:
        # If "base_channel_reference" feature is being used, (it's not 'None')
        # we need to make sure the phy_config_base is indeed empty
        assert len(baseChannelConfig.phy_config_base) == 0, \
          "Length of baseChannelConfig.phy_config_base ({}) is not 0".format(len(baseChannelConfig.phy_config_base))
        try:
          # Attempt to get the phy_config_base from the base_channel_configuration
          # in mphyConfig that matches the name of base_channel_reference;
          # use the first element returned by the filter (index 0)
          baseChannelConfig.phy_config_base = \
          list(filter(lambda x: x.name == baseChannelConfig.base_channel_reference, \
                 self.mphyConfig.base_channel_configurations.base_channel_configuration))[0].phy_config_base
        except:
          raise Exception("Invalid configuration for base_channel_configuration '{}'".format(baseChannelConfig.name))

    registers_base = baseChannelConfig.phy_config_base
    registers_channel = channelConfigEntry.phy_config_delta_add
    if self.rail_version >= 3:
      # Setting it to empty until calculator can provide phy_config_common_base registers
      registers_common = {}
    else:
      registers_subtract = baseChannelConfig.phy_config_delta_subtract

    if self.rail_version >= 3:
      register_groups_dict = dict()
      for register_group in channelConfigEntry.register_groups.register_group:
        register_groups_dict[register_group.name] = register_group.phy_config_delta_grouped_add

    # Don't let the configurator dictate channel information to us. We use the
    # RAIL_ChannelConfig_t via SYNTH_Config to determine the below registers_channel,
    # so removing them saves space and potential headache.
    # Also, BLOCKRAMADDR and CONVRAMADDR should only be written by us (RAIL)
    # We use pop(X, None) to avoid raising KeyError in case the register is not
    # in the dictionary
    registers_sets_prune_list = [registers_base, registers_channel]
    if self.rail_version >= 3:
      registers_sets_prune_list += [register_groups_dict[phy_config_delta_grouped_add] for phy_config_delta_grouped_add in register_groups_dict]
    for register in config.PROTECTED_REGS:
      for register_set in registers_sets_prune_list:
        x = register_set.pop(register, None)
        if (x != None) and debug_print:
          print("Protected register found in register set, removing.")
          print(x)

    # : remove register if the value from calculator is same as reset
    if self.wifi_script is True or self.wifi_script_seqacc is True:
      for register_set in [registers_base, registers_channel]:
        remove_list = []
        for register_name in register_set.keys():
          reset_val = register_set[register_name].resetValue
          io_val = register_set[register_name].io
          if reset_val == io_val:
            remove_list.append(register_name)
        for register_name in remove_list:
          # Let's keep LUT values even though value is the same as reset
          if 'WIFILUTGAINLNA' not in register_name:
            register_set.pop(register_name, None)

    self.registers_base = registers_base
    if self.rail_version >= 3:
      self.registers_common = registers_common
      self.register_groups_dict = register_groups_dict
    else:
      self.registers_subtract = registers_subtract
    self.registers_channel = registers_channel

  def _generatePhyInfoStructure(self, phyConfigEntry, baseConfigOptions, channelConfigOptions, model):
    # Get a local reference to model.profile.outputs to use here
    outputs = model.profile.outputs

    # For antenna diversity, first grab the relevant outputs from the calculator
    antDivMode = outputs.get_output('div_antdivmode').var_value
    antDivRepeatDisable = outputs.get_output('div_antdivrepeatdis').var_value

    # Then, generate the register value than maps to the fields we care about
    antDivConfiguration = 0x0
    antDivConfiguration |= (antDivMode << self.rm.MODEM.CTRL3.ANTDIVMODE.bitOffset)
    antDivConfiguration |= (antDivRepeatDisable << self.rm.MODEM.CTRL3.ANTDIVREPEATDIS.bitOffset)

    # FSW Rx antenna diversity config is stored on the 16 LSB
    # OFDM Rx antenna diversity config is stored on the 16 MSB
    if hasattr(model.vars, 'softmodem_antdivmode'):
      assert self.rm.MODEM.CTRL3.ANTDIVREPEATDIS.bitOffset < 16, ('ANTDIVREPEATDIS shift is too big to store ofdm ant div'
                                                                  ' in the same word')
      # Store ofdm antdiv mode in the 16 upper bits in the phyInfo and keeping virtual register mapping
      antDivConfiguration |= model.vars.softmodem_antdivmode.value << (self.rm.SUNOFDM.ANT.ANTSEL.bitOffset + 16)

    # Create a new phyInfo element
    if self.rail_version >= 3:
      newPhyInfoEntry = self.railModel.multiPhyConfig.commonStructures.phyInfoEntries.newElement("phy_info")
    else:
      newPhyInfoEntry = self.railModel.multiPhyConfig.commonStructures.phyInfoEntries.newElement("phyInfo")
    data = newPhyInfoEntry.phyInfoData

    data.version.value = 0 if self.rail_version == 1 else kRAILVersion
    data.freqOffsetFactor.value = outputs.get_output('frequency_offset_factor').var_value
    data.freqOffsetFactor_fxp.value = (outputs.get_output('frequency_offset_factor_fxp').var_value or 0)
    data.frameTypeConfig.value = phyConfigEntry.frameTypeEntry.value
    data.irCalConfig.value = phyConfigEntry.irCalConfigEntry.value
    data.timingConfig.value = phyConfigEntry.timingConfigEntry.value
    if phyConfigEntry.rffpllConfigEntry.value is not None:
      data.rffpllConfig.value = phyConfigEntry.rffpllConfigEntry.value
    if phyConfigEntry.dcdcRetimingConfigEntry.value is not None:
      data.dcdcRetimingConfig.value = phyConfigEntry.dcdcRetimingConfigEntry.value
    if self.partFamily.lower() not in ["dumbo", "jumbo", "nerio", "nixi", "panther"]:
      data.hfxoRetimingConfig.value = phyConfigEntry.hfxoRetimingTableEntry.value
    if phyConfigEntry.txIrCalConfigEntry.value is not None:
      data.txIrCalConfig.value = phyConfigEntry.txIrCalConfigEntry.value
    if phyConfigEntry.modemTxCompensation.value is not None:
      data.modemTxCompensation.value = phyConfigEntry.modemTxCompensation.value
    if getattr(outputs, 'rssi_adjust_db', None) != None:
      rssiAdjustDb = round(outputs.get_output('rssi_adjust_db').var_value)
    else:
      rssiAdjustDb = 0

    data.antDivRxAutoConfig.value = antDivConfiguration
    if self.partFamily.lower() in [ "ocelot", "margay", "serval" ]:
      # The ADCDIV will take the place of the deprecated SRC1 field for Ocelot and Margay to resolve
      # the bug causing RAIL_LIB-9898.
      data.src1Denominator.value = model.vars.adc_vco_div_actual.value
    else:
      data.src1Denominator.value = (outputs.get_output('src1_calcDenominator').var_value or 0)
    data.src2Denominator.value = (outputs.get_output('src2_calcDenominator').var_value or 0)

    if hasattr(outputs, 'trecs_pre_bits_to_syncword'):
      data.trecsPreBitsToSync.value = int(outputs.get_output('trecs_pre_bits_to_syncword').var_value or 0)
    else:
      data.trecsPreBitsToSync.value = 0

    modType = model.vars.modulation_type.value
    if hasattr(model.vars.modulation_type.var_enum, 'OFDM') and modType == model.vars.modulation_type.var_enum.OFDM:
      # In OFDM txBaudRate contains the symbol rate
      data.txBaudRate.value = round(outputs.get_output('ofdm_symbol_rate').var_value)
    else:
      data.txBaudRate.value = round(outputs.get_output('tx_baud_rate_actual').var_value)

    data.rxBaudRate.value = round(model.vars.rx_baud_rate_actual.value)
    data.baudPerSymbol.value = outputs.get_output('baud_per_symbol_actual').var_value
    data.bitsPerSymbol.value = outputs.get_output('bits_per_symbol_actual').var_value
    data.synthCache.value = (model.vars.SYNTH_IFFREQ_IFFREQ.value or 0) \
                            | (model.vars.lodiv_actual.value << 25)
    data.zWaveChannelHopTiming.value = (outputs.get_output('rx_ch_hopping_delay_usec').var_value or 0)
    data.rateInfo.value = (rssiAdjustDb & 0xFF) << 16 | data.baudPerSymbol.value << 8 | data.bitsPerSymbol.value

    if self.series != 1:
      # Cap DEC0 at 3, since the decimation value for all values above 3 is 8.
      # Also don't use value 2, in case that's useful in the future
      DEC0_MAP = [0, 1, 1, 3, 3, 3, 3, 3]
      cache40 = (model.vars.SYNTH_IFFREQ_LOSIDE.value << 20)

      if hasattr(model.vars, "MODEM_CF_DEC0") and (model.vars.MODEM_CF_DEC0.value is not None):
        cache40 |= (DEC0_MAP[int(model.vars.MODEM_CF_DEC0.value or 0)] << 22)
      elif hasattr(model.vars, "FEFILT_CF_DEC0") and (model.vars.FEFILT_CF_DEC0.value is not None):
        cache40 |= (DEC0_MAP[int(model.vars.FEFILT_CF_DEC0.value or 0)] << 22)
      else:
        pass

      if hasattr(model.vars, "MODEM_SRCCHF_BWSEL") and (model.vars.MODEM_SRCCHF_BWSEL.value is not None):
        cache40 |= (int(model.vars.MODEM_SRCCHF_BWSEL.value >= 2) << 21)

      if hasattr(model.vars, "RAC_ADCCTRL1_ADCENHALFMODE") and (model.vars.RAC_ADCCTRL1_ADCENHALFMODE.value is not None):
        cache40 |= model.vars.RAC_ADCCTRL1_ADCENHALFMODE.value << 24
      elif hasattr(model.vars, "RAC_IFADCTRIM0_IFADCENHALFMODE") and (model.vars.RAC_IFADCTRIM0_IFADCENHALFMODE.value is not None):
        cache40 |= model.vars.RAC_IFADCTRIM0_IFADCENHALFMODE.value << 24
      else:
        pass
      data.synthCache.value |= cache40

    am_low_ramplev = getattr(outputs, 'am_low_ramplev', None)
    if am_low_ramplev is None:
      data.paParams.value = 0
    else:
      data.paParams.value = am_low_ramplev.var_value

    psm_max_sleep_us = getattr(outputs, 'psm_max_sleep_us', None)
    if psm_max_sleep_us is None:
      data.psmMaxSleepUs.value = 0
    else:
      data.psmMaxSleepUs.value = psm_max_sleep_us.var_value

    # Traverse existing irCalConfigEntries and check for duplicates
    entryFound = False
    for phyInfoEntry in self.railModel.multiPhyConfig.commonStructures.phyInfoEntries._elements:
      if phyInfoEntry == newPhyInfoEntry:
        # Register the entry with the current phyConfigEntry
        phyConfigEntry.phyInfoEntry.value = phyInfoEntry
        entryFound = True
        break

    if not entryFound:
      # Found no duplicates, so add the newPhyInfoEntry to the phyInfoEntries object
      self.railModel.multiPhyConfig.commonStructures.phyInfoEntries.addElement(newPhyInfoEntry)
      # Register the entry with the current phyConfigEntry
      phyConfigEntry.phyInfoEntry.value = newPhyInfoEntry

  def _generateDynamicSlicerConfiguration(self, phyConfigEntry, baseConfigOptions, model):
    # Check the feature is enabled in the model output
    if model.profile.outputs.get_output('dynamic_slicer_enabled').var_value == True:
      # Get the two arrays that must be defined
      dynamicSlicerThresholdValues = model.profile.outputs.get_output('dynamic_slicer_threshold_values').var_value
      dynamicSlicerLevelValues = model.profile.outputs.get_output('dynamic_slicer_level_values').var_value

      # Create a new dynamicSlicerTableEntry object where to add the actual configs
      dynamicSlicerTableEntries = self.railModel.multiPhyConfig.commonStructures.dynamicSlicerTableEntries
      dynSlicerTableEntriesLen = len(dynamicSlicerTableEntries._elements)
      newDynamicSlicerTableEntryName = baseConfigOptions.get("dynamic_slicer_name", phyConfigEntry.name) \
                                       + "_dynamicSlicerTable" + str(dynSlicerTableEntriesLen)
      newDynamicSlicerTableEntry = dynamicSlicerTableEntries.newElement(newDynamicSlicerTableEntryName)

      # Use dynamicSlicerLevelValues to traverse the loop since
      # it has one more item than dynamic_slicer_threshold_values
      for i, level in enumerate(dynamicSlicerLevelValues):

        # We need to define the links in reverse order because they need to
        # reference the previous one (linked list), by doing this we avoid the
        # need for forward declarations in the C code.
        dynSlicerConfigNum = len(dynamicSlicerLevelValues)-i-1
        newDynamicSlicerConfigName = newDynamicSlicerTableEntryName+"Config"+str(dynSlicerConfigNum)

        # The threshold is stored using 2's complement in the LSB, thus the short mask (0xFF)
        # The value is stored in a uint16_t to optimize packing.
        threshold = (dynamicSlicerThresholdValues[-1] if i == 0 else dynamicSlicerThresholdValues[-i]) & 0xFF

        # The level is stored as a uint16_t, properly aligned to make the sequencer
        # code more efficient; thus the shiftValue-16.
        level = (dynamicSlicerLevelValues[-i-1] << self.rm.MODEM.CTRL1.FREQOFFESTLIM.bitOffset-16) & 0xFFFF

        # Create a new Dynamic Slicer Config
        newDynamicSlicerConfig = newDynamicSlicerTableEntry.addNewElement(newDynamicSlicerConfigName)
        newDynamicSlicerConfig.threshold.value = threshold
        newDynamicSlicerConfig.level.value = level
        newDynamicSlicerConfig.next.value = None if i == 0 else newDynamicSlicerTableEntry._elements[i-1]

      # Traverse the existing Dynamic Slicer Table Entries, and check this is not
      # a duplicate (same set of dynamicSlicerConfig entries)
      entryFound = False
      for i, dynamicSlicerTableEntry in enumerate(dynamicSlicerTableEntries._elements):
        if dynamicSlicerTableEntry._elements == newDynamicSlicerTableEntry._elements:
          # Register the entry with the current phyConfigEntry
          phyConfigEntry.dynamicSlicerTableEntry.value = dynamicSlicerTableEntry
          entryFound = True
          break

      if not entryFound:
        # Add newDynamicSlicerTableEntry entry in common structures
        dynamicSlicerTableEntries.addElement(newDynamicSlicerTableEntry)
        # Register the new entry with the current phyConfigEntry
        phyConfigEntry.dynamicSlicerTableEntry.value = newDynamicSlicerTableEntry

  # For a particular PHY get the frame config
  def _generateFrameTypeStructures(self, phyConfigEntry, model):

    newFrameLength = model.vars.frame_type_lengths.value
    if newFrameLength:
      # Traverse existing frameLengthEntries and check for duplicates
      entryFound = False
      commonStructures = self.railModel.multiPhyConfig.commonStructures
      for i, frameLengthEntry in enumerate(commonStructures.frameLengthEntries._elements):
        if frameLengthEntry.values == newFrameLength:
          # Register the entry with the current phyConfigEntry
          phyConfigEntry.frameLengthEntry.value = frameLengthEntry
          entryFound = True
          break
      if not entryFound:
        # Create a new frameLength entry in common structures
        if self.rail_version >= 3:
          newFrameLengthEntry = commonStructures.frameLengthEntries.addNewElement("frame_length_list")
        else:
          newFrameLengthEntry = commonStructures.frameLengthEntries.addNewElement("frameLengthList")
        newFrameLengthEntry.values = newFrameLength
        # Register the new entries with the current phyConfigEntry
        phyConfigEntry.frameLengthEntry.value = newFrameLengthEntry

      # Create new frameTypeConfig entry
      if self.rail_version >= 3:
        newFrameTypeEntry = commonStructures.frameTypeEntries.newElement("frame_type_config")
      else:
        newFrameTypeEntry = commonStructures.frameTypeEntries.newElement("frameTypeConfig")
      newFrameTypeEntry.offset.value = model.vars.frame_type_loc.value
      newFrameTypeEntry.mask.value = model.vars.frame_type_mask.value
      newFrameTypeEntry.frameLen.value = phyConfigEntry.frameLengthEntry
      newFrameTypeEntry.isValid.value = model.vars.frame_type_valid.value
      newFrameTypeEntry.addressFilter.value = model.vars.frame_type_filter.value
      newFrameTypeEntry.variableAddrLoc.value = False

      # Traverse existing frameTypeEntries and check for duplicates
      entryFound = False
      for i, frameTypeEntry in enumerate(commonStructures.frameTypeEntries._elements):
        if frameTypeEntry == newFrameTypeEntry:
          # Register the entry with the current phyConfigEntry
          phyConfigEntry.frameTypeEntry.value = frameTypeEntry
          entryFound = True
          break
      if not entryFound:
        # Create a new frameType entry in common structures
        commonStructures.frameTypeEntries.addElement(newFrameTypeEntry)
        # Register the new entry with the current phyConfigEntry
        phyConfigEntry.frameTypeEntry.value = newFrameTypeEntry

  def _generateIrCalStructure(self, phyConfigEntry, model, isBaseConfig, baseConfigAttr):
    # Get a local reference to model.profile.outputs to use here
    outputs = model.profile.outputs

    legacyIrConfig = True
    if (self.partFamily.lower() in ["ocelot", "sol", "margay", "serval"]):
      # For the subG chip,
      # if there is a 2.4Ghz PHY being built, it should use the same ircal coefficient
      # as all the 2.4GHz PHYs should get the coefficient from DEVINFO
      if (model.vars.lodiv_actual.value != 1):
        legacyIrConfig = False
      agcMangainReg = self.rm.AGC.MANGAIN
      agcMangainReg.io = 0
      agcMangainReg.MANGAINEN.io = 1
      agcMangainReg.MANGAINIFPGA.io = 4
      agcMangainReg.MANGAINLNA.io = 1
      agcMangainReg.MANGAINPN.io = 1

      #Find the correct IRCAL register to use
      if (self.partFamily.lower() in ["sol"]):
        modemIrcalReg = self.rm.FEFILT1.IRCAL if self.rm.SEQ.MODEMINFO.SOFTMODEM_DEMOD_EN else self.rm.FEFILT0.IRCAL
      else:
        modemIrcalReg = self.rm.MODEM.IRCAL

      #Get MURSHF and MUISHF values from Radio Configurator Profile Outputs
      modemIrcalReg.io = 0
      modemIrcalReg.MURSHF.io = outputs.get_output('ircal_murshf').var_value
      modemIrcalReg.MUISHF.io = outputs.get_output('ircal_muishf').var_value

      modemIrcalReg.IRCALEN.io = 1

      newIrCalConfig = [
        agcMangainReg.io & 0xFF,
        (agcMangainReg.io >> 8) & 0xFF,
        (agcMangainReg.io >> 16) & 0xFF,
        (agcMangainReg.io >> 24) & 0xFF,
        modemIrcalReg.io & 0xFF,
        (modemIrcalReg.io >> 8) & 0xFF,
        (modemIrcalReg.io >> 16) & 0xFF,
        (modemIrcalReg.io >> 24) & 0xFF,
        False, #pteConfigIsValid
        True, #pllLoopbackConfigIsValid
        False, #paLoopbackConfigIsValid
        2, #bestConfig = IRCAL_TYPE_PLL_LOOPBACK
        100 & 0xFF,  #delayMsResult, 100ms
        (100 >> 8) & 0xFF,
        1, #divx
        1, #divr
        47, #divn
        0, #bw
        0, #freq
        7, #slice
      ]
    else:
      newIrCalConfig = [
        outputs.get_output('ircal_auxndiv').var_value,
        outputs.get_output('ircal_auxlodiv').var_value,
        outputs.get_output('ircal_rampval').var_value,
        outputs.get_output('ircal_rxamppll').var_value,
        outputs.get_output('ircal_rxamppa').var_value,
        outputs.get_output('ircal_manufconfigvalid').var_value,
        outputs.get_output('ircal_pllconfigvalid').var_value,
        outputs.get_output('ircal_paconfigvalid').var_value,
        outputs.get_output('ircal_bestconfig').var_value,
        outputs.get_output('ircal_useswrssiaveraging').var_value,
        outputs.get_output('ircal_numrssitoavg').var_value,
        outputs.get_output('ircal_throwawaybeforerssi').var_value,
        outputs.get_output('ircal_delayusbeforerssi').var_value % 256,
        outputs.get_output('ircal_delayusbeforerssi').var_value / 256,
        outputs.get_output('ircal_delayusbetweenswrssi').var_value % 256,
        outputs.get_output('ircal_delayusbetweenswrssi').var_value / 256,
        outputs.get_output('ircal_agcrssiperiod').var_value,

        # recreate these Jumbo settings at the end for backwards compatibility with RAIL 1.x
        outputs.get_output('ircal_useswrssiaveraging2').var_value,
        outputs.get_output('ircal_numrssitoavg2').var_value,
        outputs.get_output('ircal_throwawaybeforerssi2').var_value,
        outputs.get_output('ircal_delayusbeforerssi2').var_value % 256,
        outputs.get_output('ircal_delayusbeforerssi2').var_value / 256,
        outputs.get_output('ircal_delayusbetweenswrssi2').var_value % 256,
        outputs.get_output('ircal_delayusbetweenswrssi2').var_value / 256,

        # Allow changing of default PA output power during IR cal (specifically for split TX/RX paths).
        outputs.get_output('ircal_power_level').var_value,
      ]

    # Traverse existing irCalConfigEntries and check for duplicates
    entryFound = False
    commonStructures = self.railModel.multiPhyConfig.commonStructures

    if legacyIrConfig is False:
      if (isBaseConfig):
        # for non-2.4Ghz, Create a new RAIL_ChannelConfigEntryAttr_t entry in
        # common structures, so each PHY has its own ircal coefficient
        if self.rail_version >= 3:
          newRailChannelConfigEntryAttr = commonStructures.railChannelConfigEntryAttrEntries.addNewElement("channel_config_entry_attr")
        else:
          newRailChannelConfigEntryAttr = commonStructures.railChannelConfigEntryAttrEntries.addNewElement("channelConfigEntryAttr")
        newRailChannelConfigEntryAttr.calValues.value = 2
        phyConfigEntry.channelConfigEntryAttr.value = newRailChannelConfigEntryAttr
      else:
        phyConfigEntry.channelConfigEntryAttr.value = baseConfigAttr

    for i, irCalConfigEntry in enumerate(commonStructures.irCalConfigEntries._elements):
      if irCalConfigEntry.values == newIrCalConfig:
        # Register the entry with the current phyConfigEntry
        phyConfigEntry.irCalConfigEntry.value = irCalConfigEntry
        if legacyIrConfig is True:
          phyConfigEntry.channelConfigEntryAttr.value = commonStructures.railChannelConfigEntryAttrEntries._elements[i]
        #print("irCalConfigEntry {}".format(irCalConfigEntry.name))
        entryFound = True
        break

    if not entryFound:
      # Create a new irCalConfig entry in common structures
      if self.rail_version >= 3:
        newIrCalConfigEntry = commonStructures.irCalConfigEntries.addNewElement("ir_cal_config")
      else:
        newIrCalConfigEntry = commonStructures.irCalConfigEntries.addNewElement("irCalConfig")
      newIrCalConfigEntry.values = newIrCalConfig
      if legacyIrConfig is True:
        # Create a new RAIL_ChannelConfigEntryAttr_t entry in common structures
        if self.rail_version >= 3:
          newRailChannelConfigEntryAttr = commonStructures.railChannelConfigEntryAttrEntries.addNewElement("channel_config_entry_attr")
        else:
          newRailChannelConfigEntryAttr = commonStructures.railChannelConfigEntryAttrEntries.addNewElement("channelConfigEntryAttr")
        if (self.partFamily.lower() in ["dumbo","jumbo","nerio","nixi", "lynx", "leopard", "lion", "curl"]):
          newRailChannelConfigEntryAttr.calValues.value = 1
        else:
          newRailChannelConfigEntryAttr.calValues.value = 2 #panther has 2 RF paths
        phyConfigEntry.channelConfigEntryAttr.value = newRailChannelConfigEntryAttr
      # Register the new entries with the current phyConfigEntry
      phyConfigEntry.irCalConfigEntry.value = newIrCalConfigEntry

    return phyConfigEntry.channelConfigEntryAttr.value

  def _generateTxIrCalStructure(self, phyConfigEntry, model):
    outputs = model.profile.outputs

    newtxIrCalConfig = None
    if (hasattr(outputs, 'softmodem_txircal_params')):
      softmodem_txircal_params = outputs.get_output('softmodem_txircal_params').var_value
      softmodem_txircal_freq = outputs.get_output('softmodem_txircal_freq').var_value

      if softmodem_txircal_params is not None:
        # Create a list containing all info
        newtxIrCalConfig = [softmodem_txircal_freq]
        newtxIrCalConfig.extend(softmodem_txircal_params)

    commonStructures = self.railModel.multiPhyConfig.commonStructures

    newTxIrCalBand = False
    # Check for duplicates in the commonStructures containing all phy configurations
    for i, txIrCalConfigEntry in enumerate(commonStructures.txIrCalConfigEntries._elements):
      # if parameters changed, then another array must be created
      if txIrCalConfigEntry.values != newtxIrCalConfig:
        newTxIrCalBand = True

      phyConfigEntry.txIrCalConfigEntry.value = txIrCalConfigEntry

    if newtxIrCalConfig is not None:
      # If there is no txIRCAL config yet, then create one
      if phyConfigEntry.txIrCalConfigEntry.value == None or newTxIrCalBand:
        if self.rail_version >= 3:
          newTxIrCalConfigEntry = commonStructures.txIrCalConfigEntries.addNewElement("tx_ir_cal_config")
        else:
          newTxIrCalConfigEntry = commonStructures.txIrCalConfigEntries.addNewElement("txIrCalConfig")
        newTxIrCalConfigEntry.values = newtxIrCalConfig
        phyConfigEntry.txIrCalConfigEntry.value = newTxIrCalConfigEntry

  def _generateHardModemTxCompensation(self, radioConfigModel, phyConfigEntry):
    hardmodem_txbr_compensation = getattr(radioConfigModel.profile.outputs, 'hardmodem_txbr_compensation', None)

    if hardmodem_txbr_compensation is not None:
      newTxCompensation = hardmodem_txbr_compensation.var_value
      commonStructures = self.railModel.multiPhyConfig.commonStructures

      if newTxCompensation is not None and len(newTxCompensation) > 0:
        newCompensationArray = True
        # Check for duplicates in the commonStructures containing all phy configurations
        for i, txBrCompensationEntry in enumerate(commonStructures.modemTxCompensationEntries._elements):
          # if parameters changed, then another array must be created
          if txBrCompensationEntry.values == newTxCompensation:
            phyConfigEntry.modemTxCompensation.value = txBrCompensationEntry
            newCompensationArray = False
            break

        # If there is no compensation recorded yet, then create one
        if newCompensationArray:
          if self.rail_version >= 3:
            newCompensationEntry = commonStructures.modemTxCompensationEntries.addNewElement("modem_tx_compensation")
          else:
            newCompensationEntry = commonStructures.modemTxCompensationEntries.addNewElement("modemTxCompensation")
          newCompensationEntry.values = newTxCompensation
          phyConfigEntry.modemTxCompensation.value = newCompensationEntry

  def _generateStackInfo(self, phyConfigEntry, model):
    outputs = model.profile.outputs

    if (hasattr(outputs, 'stack_info')):
      stack_info = outputs.get_output('stack_info').var_value
      # Only reference non default values
      if stack_info != [0, 0] or model.profile.name.upper() == "BASE":
        newStackInfo = stack_info
      else:
        newStackInfo = None
        phyConfigEntry.stackInfo.value = None

      if newStackInfo is not None:
        commonStructures = self.railModel.multiPhyConfig.commonStructures

        # Create a new config in commonStructures by default
        newStackInfoConfig = True
        # Check for duplicates in the commonStructures containing all phy configurations
        for i, stackInfoConfigEntry in enumerate(commonStructures.stackInfoEntries._elements):
          phyConfigEntry.stackInfo.value = stackInfoConfigEntry

          if stackInfoConfigEntry.values == newStackInfo:
            # Duplicate found: no new config will be added to commonStructures
            newStackInfoConfig = False
            break

        if phyConfigEntry.stackInfo.value == None or newStackInfoConfig:
          length = 0

          # Get commonStructures length for name uniqueness purposes
          if commonStructures.stackInfoEntries.lastElement is not None:
            length = len(commonStructures.stackInfoEntries._elements)

          # Create a new config in commonStructures
          if self.rail_version >= 3:
            newStackInfoConfigEntry = commonStructures.stackInfoEntries.addNewElement("stack_info_%s" % length)
          else:
            newStackInfoConfigEntry = commonStructures.stackInfoEntries.addNewElement("stackInfo_%s" % length)
          newStackInfoConfigEntry.values = newStackInfo
          # Store that config in the linked phyConfigEntry
          phyConfigEntry.stackInfo.value = newStackInfoConfigEntry

          # Configure structure settings for rail_config.h formatting.
          # Only structure having the field enabled to True will be printed in the header.
          # If enabled, all fields in structure under stackInfoProtocolTableEntries in yaml file
          # except "enable" will be printed.
          # If new structs were to be added, the field "enable" must be present in each one of it.
          if newStackInfo[0] == model.vars.protocol_id.var_enum.WiSUN:
            # Enable WiSUN protocol struct printing in header
            if self.rail_version >=3 :
              commonStructures.stackInfoProtocolTableEntries.sl_rail_stack_info_wisun.enable.value = True
            else:
              commonStructures.stackInfoProtocolTableEntries.RAIL_StackInfoWisun.enable.value = True

  def _generateAlternatePhy(self, phyConfigEntry, model, channelConfigEntry):
    base_frequency = channelConfigEntry.alternate_phy.base_frequency
    channel_spacing = channelConfigEntry.alternate_phy.channel_spacing
    number_of_channels = channelConfigEntry.alternate_phy.number_of_channels
    alt_min_if_var = getattr(model.profile.outputs, "alt_min_if_hz", None)
    base_min_if_var = getattr(model.profile.outputs, "min_if_hz", None)
    alt_softmodem_used_var = getattr(model.profile.outputs, "alt_softmodem_used", None)
    alt_rssi_adjust_db_var = getattr(model.profile.outputs, "alt_rssi_adjust_db", None)
    alt_agc_ctrl1_pwrperiod_var = getattr(model.profile.outputs, "alt_agc_ctrl1_pwrperiod", None)
    alt_agc_ctrl1_rssiperiod_var = getattr(model.profile.outputs, "alt_agc_ctrl1_rssiperiod", None)

    try:
      alt_hwModemAgcCtrl1 = self.rm.AGC.CTRL1.io & (
                self.rm.AGC.CTRL1.PWRPERIOD.mask ^ 0xFFFFFFFF & self.rm.AGC.CTRL1.RSSIPERIOD.mask ^ 0xFFFFFFFF) | (
                                      alt_agc_ctrl1_pwrperiod_var.var_value << self.rm.AGC.CTRL1.PWRPERIOD.bitOffset) | (
                                      alt_agc_ctrl1_rssiperiod_var.var_value << self.rm.AGC.CTRL1.RSSIPERIOD.bitOffset)
      alt_min_if_hz = alt_min_if_var.var_value
      base_min_if_hz = base_min_if_var.var_value
      alt_softmodem_used = alt_softmodem_used_var.var_value
      alt_rssi_adjust_db = alt_rssi_adjust_db_var.var_value

      # Convert unit Hz ==> kHz
      alt_min_if_kHz = int(alt_min_if_hz / 1000)
      base_min_if_kHz = int(base_min_if_hz / 1000)

      alt_rateInfo = (int(alt_rssi_adjust_db) & 0xFF) << 16

      newAlternatePhy = True
      commonStructures = self.railModel.multiPhyConfig.commonStructures

      newAlternatePhyValues = [base_frequency,
                               channel_spacing,
                               number_of_channels,
                               alt_min_if_kHz,
                               base_min_if_kHz,
                               alt_softmodem_used,
                               alt_rateInfo,
                               alt_hwModemAgcCtrl1]

      for i, alternatePhyEntry in enumerate(commonStructures.railAlternatePhyEntries._elements):
        alternatePhyValues = [alternatePhyEntry.baseFrequency.value,
                              alternatePhyEntry.channelSpacing.value,
                              alternatePhyEntry.numberOfChannels.value,
                              alternatePhyEntry.minIf_kHz.value,
                              alternatePhyEntry.minBaseIf_kHz.value,
                              alternatePhyEntry.isOfdmModem.value,
                              alternatePhyEntry.rateInfo.value,
                              alternatePhyEntry.hwModemAgcCtrl1.value,]
        if alternatePhyValues == newAlternatePhyValues:
          phyConfigEntry.alternatePhy.value = alternatePhyEntry
          newAlternatePhy = False
          break

      if newAlternatePhy:
        length = 0
        if commonStructures.railAlternatePhyEntries.lastElement is not None:
          length = len(commonStructures.railAlternatePhyEntries._elements)

        if self.rail_version >= 3:
          newAlternatePhyEntry = commonStructures.railAlternatePhyEntries.addNewElement("alternate_phy_%s" % length)
        else:
          newAlternatePhyEntry = commonStructures.railAlternatePhyEntries.addNewElement("alternatePhy_%s" % length)
        newAlternatePhyEntry.baseFrequency.value = base_frequency
        newAlternatePhyEntry.channelSpacing.value = channel_spacing
        newAlternatePhyEntry.numberOfChannels.value = number_of_channels
        newAlternatePhyEntry.minIf_kHz.value = alt_min_if_kHz
        newAlternatePhyEntry.minBaseIf_kHz.value = base_min_if_kHz
        newAlternatePhyEntry.isOfdmModem.value = alt_softmodem_used
        newAlternatePhyEntry.rateInfo.value = alt_rateInfo
        newAlternatePhyEntry.hwModemAgcCtrl1.value = alt_hwModemAgcCtrl1
        phyConfigEntry.alternatePhy.value = newAlternatePhyEntry
    except:
      # At least one of variables below is None
      # [base_frequency, channel_spacing, number_of_channels, base_min_if, alt_min_if, alt_softmodem_used, alt_rssi_adjust_db_var]
      return


  def _generateDcdcRetimingStructure(self, phyConfigEntry, model):
    if (self.partFamily in ["ocelot", "sol", "margay", "serval"]):
      # Get a local reference to model.profile.outputs to use here
      outputs = model.profile.outputs

      if (outputs.get_output('ppnd_0').var_value is not None) and (outputs.get_output('ppnd_0').var_value != 0):
        newDcdcRetimingConfig = [
          outputs.get_output('ppnd_0').var_value,
          outputs.get_output('ppnd_1').var_value,
          outputs.get_output('ppnd_2').var_value,
          outputs.get_output('ppnd_3').var_value
        ]

        # Traverse existing dcdcRetimingConfigEntries and check for duplicates
        entryFound = False
        commonStructures = self.railModel.multiPhyConfig.commonStructures

        for i, dcdcRetimingConfigEntry in enumerate(commonStructures.dcdcRetimingConfigEntries._elements):
          if dcdcRetimingConfigEntry.values == newDcdcRetimingConfig:
            # Register the entry with the current phyConfigEntry
            phyConfigEntry.dcdcRetimingConfigEntry.value = dcdcRetimingConfigEntry
            entryFound = True
            break

        if not entryFound:
          # Create a new dcdcRetimingConfig entry in common structures
          if self.rail_version >= 3:
            newDcdcRetimingConfigEntry = commonStructures.dcdcRetimingConfigEntries.addNewElement("dcdc_retiming_config")
          else:
            newDcdcRetimingConfigEntry = commonStructures.dcdcRetimingConfigEntries.addNewElement("dcdcRetimingConfig")
          newDcdcRetimingConfigEntry.values = newDcdcRetimingConfig
          # Register the new entries with the current phyConfigEntry
          phyConfigEntry.dcdcRetimingConfigEntry.value = newDcdcRetimingConfigEntry

  def _generateTimingStructure(self, phyConfigEntry, model):
    # Get a local reference to model.profile.outputs to use here
    outputs = model.profile.outputs

    if outputs.get_output('rx_sync_delay_ns').var_value is not None:
      if self.partFamily in ["jumbo", "nerio"]:
        newTimingConfig = [
          outputs.get_output('rx_sync_delay_ns').var_value,
          outputs.get_output('tx_sync_delay_ns').var_value,
          outputs.get_output('tx_eof_delay_ns').var_value
        ]
      else:
        newTimingConfig = [
          outputs.get_output('rx_sync_delay_ns').var_value,
          outputs.get_output('rx_eof_delay_ns').var_value,
          outputs.get_output('tx_sync_delay_ns').var_value,
          outputs.get_output('tx_eof_delay_ns').var_value
        ]

      # Traverse existing dcdcRetimingConfigEntries and check for duplicates
      entryFound = False
      commonStructures = self.railModel.multiPhyConfig.commonStructures

      for i, timingConfigEntry in enumerate(commonStructures.timingConfigEntries._elements):
        if timingConfigEntry.values == newTimingConfig:
          # Register the entry with the current phyConfigEntry
          phyConfigEntry.timingConfigEntry.value = timingConfigEntry
          entryFound = True
          break

      if not entryFound:
        # Create a new timingConfig entry in common structures
        if self.rail_version >= 3:
          newTimingConfigEntry = commonStructures.timingConfigEntries.addNewElement("timing_config")
        else:
          newTimingConfigEntry = commonStructures.timingConfigEntries.addNewElement("timingConfig")
        newTimingConfigEntry.values = newTimingConfig
        # Register the new entries with the current phyConfigEntry
        phyConfigEntry.timingConfigEntry.value = newTimingConfigEntry

  def _generateHfxoRetimingStructure(self, phyConfigEntry, model):
    if self.partFamily.lower() not in ["dumbo", "jumbo", "nerio", "nixi", "panther"]:
      commonStructures = self.railModel.multiPhyConfig.commonStructures
      if self.rail_version >= 3:
        newHfxoRetimingConfigEntry = commonStructures.hfxoRetimingTableEntries.newElement("hfxo_retiming_config_entries")
      else:
        newHfxoRetimingConfigEntry = commonStructures.hfxoRetimingTableEntries.newElement("hfxoRetimingConfigEntries")

      # Get the arrays
      hfxoRetimingFreqValues = model.vars.lut_freq.value
      hfxoRetimingFreqUpperValues = model.vars.lut_freq_upper.value
      hfxoRetimingValidValues = model.vars.lut_valid.value
      hfxoRetimingSmuxDivValues = model.vars.lut_smuxdiv.value
      hfxoRetimingLimitLValues = model.vars.lut_limitl.value
      hfxoRetimingLimitHValues = model.vars.lut_limith.value
      hfxoRetimingDpllValues = model.vars.lut_dpll_freq_hz.value
      hfxoRetimingTblIdxValues = model.vars.lut_table_index.value
      hfxoRetimingIdxStart = 0
      byteOffset = 4 + (8 * len(hfxoRetimingTblIdxValues))

      # Populate the rest of the table
      for tbl_idx in range(len(hfxoRetimingTblIdxValues)):
        # insert lowest range
        if self.rail_version >= 3:
          newHfxoRetimingConfigStruct = newHfxoRetimingConfigEntry.hfxoRetimingBandConfigEntries.addNewElement("hfxo_retiming_band_config_struct")
        else:
          newHfxoRetimingConfigStruct = newHfxoRetimingConfigEntry.hfxoRetimingBandConfigEntries.addNewElement("hfxoRetimingBandConfigStruct")
        newHfxoRetimingConfigStruct.loFreqUpperRange.value = hfxoRetimingFreqValues[hfxoRetimingIdxStart]
        newHfxoRetimingConfigStruct.valid.value = 0
        newHfxoRetimingConfigStruct.sMuxDiv.value = 0
        newHfxoRetimingConfigStruct.hfxoLimitL.value = 0
        newHfxoRetimingConfigStruct.hfxoLimitH.value = 0

        for i in range(hfxoRetimingTblIdxValues[tbl_idx]):
          if self.rail_version >= 3:
            newHfxoRetimingConfigStruct = newHfxoRetimingConfigEntry.hfxoRetimingBandConfigEntries.addNewElement("hfxo_retiming_band_config_struct")
          else:
            newHfxoRetimingConfigStruct = newHfxoRetimingConfigEntry.hfxoRetimingBandConfigEntries.addNewElement("hfxoRetimingBandConfigStruct")
          newHfxoRetimingConfigStruct.loFreqUpperRange.value = hfxoRetimingFreqUpperValues[hfxoRetimingIdxStart + i]
          newHfxoRetimingConfigStruct.valid.value = hfxoRetimingValidValues[hfxoRetimingIdxStart + i]
          if (hfxoRetimingSmuxDivValues[hfxoRetimingIdxStart + i] > 0):
            newHfxoRetimingConfigStruct.sMuxDiv.value = hfxoRetimingSmuxDivValues[hfxoRetimingIdxStart + i]
          else:
            newHfxoRetimingConfigStruct.sMuxDiv.value = 0
          if (hfxoRetimingLimitLValues[hfxoRetimingIdxStart + i] > 0):
            newHfxoRetimingConfigStruct.hfxoLimitL.value = hfxoRetimingLimitLValues[hfxoRetimingIdxStart + i]
          else:
            newHfxoRetimingConfigStruct.hfxoLimitL.value = 0
          if (newHfxoRetimingConfigStruct.hfxoLimitL.value > 7):
            newHfxoRetimingConfigStruct.valid.value = 0
          if (hfxoRetimingLimitHValues[hfxoRetimingIdxStart + i] > 0):
            newHfxoRetimingConfigStruct.hfxoLimitH.value = hfxoRetimingLimitHValues[hfxoRetimingIdxStart + i]
          else:
            newHfxoRetimingConfigStruct.hfxoLimitH.value = 0
          if (newHfxoRetimingConfigStruct.hfxoLimitH.value > 7):
            newHfxoRetimingConfigStruct.valid.value = 0

        hfxoRetimingIdxStart = hfxoRetimingIdxStart + hfxoRetimingTblIdxValues[tbl_idx]
        if self.rail_version >= 3:
          newHfxoRetimingBandConfigStruct = newHfxoRetimingConfigEntry.hfxoRetimingInfoConfigEntries.addNewElement("hfxo_retiming_band_config_struct")
        else:
          newHfxoRetimingBandConfigStruct = newHfxoRetimingConfigEntry.hfxoRetimingInfoConfigEntries.addNewElement("hfxoRetimingBandConfigStruct")
        newHfxoRetimingBandConfigStruct.numBand.value = hfxoRetimingTblIdxValues[tbl_idx] + 1
        newHfxoRetimingBandConfigStruct.dpllClock.value = hfxoRetimingDpllValues[tbl_idx]
        newHfxoRetimingBandConfigStruct.offset.value = byteOffset
        byteOffset = byteOffset + (6 * (hfxoRetimingTblIdxValues[tbl_idx] + 1))

      # Traverse existing hfxoRetimingConfigEntries and check for duplicates
      entryFound = False
      for i, hfxoRetimingConfigEntry in enumerate(commonStructures.hfxoRetimingTableEntries._elements):
        if hfxoRetimingConfigEntry == newHfxoRetimingConfigEntry:
          # Register the entry with the current phyConfigEntry
          phyConfigEntry.hfxoRetimingTableEntry.value = hfxoRetimingConfigEntry
          entryFound = True
          break

      if not entryFound:
        # Create a new newHfxoRetimingConfig entry in common structures
        commonStructures.hfxoRetimingTableEntries.addElement(newHfxoRetimingConfigEntry)
        # Register the new entries with the current phyConfigEntry
        phyConfigEntry.hfxoRetimingTableEntry.value = newHfxoRetimingConfigEntry

  def _generateRffpllStructure(self, phyConfigEntry, model):
    # Get a local reference to model.profile.outputs to use here
    outputs = model.profile.outputs

    if (hasattr(outputs, 'fpll_div_array')):
      fpll_div_array = outputs.get_output('fpll_div_array').var_value
      fpll_divx = fpll_div_array[0]
      fpll_divy = fpll_div_array[1]
      fpll_divn = fpll_div_array[2]

      newRffpllConfig = [
        fpll_divx | (fpll_divy << 8) | (fpll_divn << 16),
        int(outputs.get_output('fpll_divx_freq').var_value),
        int(outputs.get_output('fpll_divy_freq').var_value),
      ]

      # Traverse existing rffpllConfigEntries and check for duplicates
      entryFound = False
      commonStructures = self.railModel.multiPhyConfig.commonStructures
      for i, rffpllConfigEntry in enumerate(commonStructures.rffpllConfigEntries._elements):
        if rffpllConfigEntry.values == newRffpllConfig:
          # Register the entry with the current phyConfigEntry
          phyConfigEntry.rffpllConfigEntry.value = rffpllConfigEntry
          entryFound = True
          break

      if not entryFound:
        # If there is already an entry in the list of RFFPLL configurations, add
        # the new entry and print a warning message.
        if len(commonStructures.rffpllConfigEntries._elements):
          print("Warning: The configuration contains multiple RFFPLL settings. "
                "Only PHYs which match the system RFFPLL configuration will load.")
        # Create a new rffpllConfig entry in common structures
        if self.rail_version >= 3:
          newRffpllConfigEntry = commonStructures.rffpllConfigEntries.addNewElement("rffpll_config")
        else:
          newRffpllConfigEntry = commonStructures.rffpllConfigEntries.addNewElement("rffpllConfig")
        newRffpllConfigEntry.values = newRffpllConfig
        phyConfigEntry.rffpllConfigEntry.value = newRffpllConfigEntry

  def _generateFrameCodingTable(self, phyConfigEntry, model):

    generic_lut = getattr(model.profile.outputs, 'generic_lookup_tables', None)

    try:
      var_value = getattr(generic_lut, 'var_value', None)
      if var_value and 'frameCodingTable' in var_value:
        codingArray = generic_lut.var_value['frameCodingTable']
        # Traverse existing frameCodingTableEntries and check for duplicates
        entryFound = False
        commonStructures = self.railModel.multiPhyConfig.commonStructures
        for i, frameCodingTableEntry in enumerate(commonStructures.frameCodingTableEntries._elements):
          if frameCodingTableEntry.values == codingArray:
            # Register the entry with the current phyConfigEntry
            phyConfigEntry.frameCodingTableEntry.value = frameCodingTableEntry
            entryFound = True
            break

        if not entryFound:
          # Create a new frameCodingTable entry in common structures
          if self.rail_version >= 3:
            newframeCodingTableEntry = commonStructures.frameCodingTableEntries.addNewElement("frame_coding_table")
            # Update lookup table key since this is used to generate the variable on the C side
            model.vars.generic_lookup_tables.value['frame_coding_table'] = model.vars.generic_lookup_tables.value[
              'frameCodingTable']
            del model.vars.generic_lookup_tables.value['frameCodingTable']

          else:
            newframeCodingTableEntry = commonStructures.frameCodingTableEntries.addNewElement("frameCodingTable")

          newframeCodingTableEntry.values = codingArray
          # Register the new entry with the current phyConfigEntry
          phyConfigEntry.frameCodingTableEntry.value = newframeCodingTableEntry
    except (AttributeError, TypeError):
      pass

  def _generateLookupTable(self, model):
    # Check if the model has the generic lookup tables variable
    if hasattr(model.vars, 'generic_lookup_tables') and model.vars.generic_lookup_tables.value:
      lookupTablesDict = model.vars.generic_lookup_tables.value

      # Ensure lookupTablesDict is a dictionary
      if lookupTablesDict and isinstance(lookupTablesDict, dict):
        commonStructures = self.railModel.multiPhyConfig.commonStructures

        # Iterate through each lookup table in the dictionary
        for tableName, tableValues in lookupTablesDict.items():
          if tableValues and isinstance(tableValues, list):
            # Ensure all values are integers
            lookupValues = [int(x) for x in tableValues]

            # Traverse existing lookupTableEntries and check for duplicates
            entryFound = False
            for i, lookupTableEntry in enumerate(commonStructures.lookupTableEntries._elements):
              if (hasattr(lookupTableEntry, 'tableName') and
                      hasattr(lookupTableEntry, 'values') and
                      lookupTableEntry.tableName.value == tableName and
                      lookupTableEntry.values.values == lookupValues):
                entryFound = True
                break

            if not entryFound:
              # Create a new lookupTable entry in common structures
              newLookupTableEntry = commonStructures.lookupTableEntries.addNewElement(
                "genericLookupTable")
              newLookupTableEntry.tableName.value = tableName
              newLookupTableEntry.values.values = lookupValues
              newLookupTableEntry._uniqueName = tableName

  # This method is used to extract ordered indices from a data dictionary for the purpose
  # of generating comments for the lookup table entries in the RAIL configuration.
  def _extract_ordered_indices(self, data_dict, register_name):
    meta_key = next(
        (k for k in data_dict if register_name in k and '_metadata' in k),
        None
    )
    if not meta_key:
        return []

    metadata = data_dict[meta_key]
    if 'table_names' not in metadata or register_name not in metadata['table_names']:
        return []

    # Extract and sort _index_* keys numerically
    indices = [
        (int(k.split('_')[-1]), k, v)
        for k, v in metadata.items()
        if k.startswith('_index_')
    ]
    indices.sort(key=lambda x: x[0])

    # Build the result list
    result = [f"index_{idx}: {values}" for idx, key, values in indices]
    return result

  def _generateLookupTable(self, model):
    # Check if the model has the generic lookup tables variable
    if hasattr(model.vars, 'generic_lookup_tables') and model.vars.generic_lookup_tables.value:
      lookupTablesDict = model.vars.generic_lookup_tables.value

      # Ensure lookupTablesDict is a dictionary
      if lookupTablesDict and isinstance(lookupTablesDict, dict):
        commonStructures = self.railModel.multiPhyConfig.commonStructures

        # Iterate through each lookup table in the dictionary
        for tableName, tableValues in lookupTablesDict.items():
          if tableValues and isinstance(tableValues, list):
            # Ensure all values are integers
            lookupValues = [int(x) for x in tableValues]

            # Traverse existing lookupTableEntries and check for duplicates
            entryFound = False
            for i, lookupTableEntry in enumerate(commonStructures.lookupTableEntries._elements):
              if (hasattr(lookupTableEntry, 'tableName') and
                      hasattr(lookupTableEntry, 'values') and
                      lookupTableEntry.tableName.value == tableName and
                      lookupTableEntry.values.values == lookupValues):
                entryFound = True
                break

            if not entryFound:
              # Create a new lookupTable entry in common structures
              newLookupTableEntry = commonStructures.lookupTableEntries.addNewElement(
                "genericLookupTable")
              newLookupTableEntry.tableName.value = tableName
              newLookupTableEntry.values.values = lookupValues
              result = self._extract_ordered_indices(lookupTablesDict, tableName)
              newLookupTableEntry.tableInputs.values = result
              newLookupTableEntry._uniqueName = tableName



  def _loadBchLookupTable(self, model, regs, regAddress, codingArray):

    convRamAddress = model.vars.FRC_CONVRAMADDR_CONVRAMADDR.value << 2

    if codingArray is not None and convRamAddress != 0:
      # The block decoder Lookup table starts 64 bytes after the convolutional block address pointed in RAM
      blockRamAddr = convRamAddress + 64
      # Write the address in RAM of the block decoding lookup table to BLOCKRAMADDR
      regs.append((regAddress, blockRamAddr, "FRC.BLOCKRAMADDR"))

      # Write the BCH lookup table in RAM at the address pointed by BLOCKRAMADDR
      # Store each halfword of the LUT in RAM
      ctr = 0
      for index in range(1, len(codingArray), 2):
        lsbValue = codingArray[index - 1]
        msbValue = codingArray[index]
        word = (msbValue << 16) | lsbValue
        regs.append((0xB0000000 + blockRamAddr + (4*ctr), word, "FRC.BLOCKRAMADDR" + str(ctr)))
        ctr += 1
    else:
      regs.append((regAddress, 0, "FRC.BLOCKRAMADDR"))

  def _generateChannelConfigGroups(self, phyConfigEntry):
    newChannelConfigGroup = phyConfigEntry.channelConfigGroups.newElement()
    newChannelConfigGroup.modemConfigGroup.value = phyConfigEntry.modemConfigGroupEntry.value
    modemConfigGroupLength = 0
    if newChannelConfigGroup.modemConfigGroup.value is not None:
      for element in phyConfigEntry.modemConfigGroupEntry.value._elements:
        modemConfigGroupLength = modemConfigGroupLength + element.length.value
    newChannelConfigGroup.modemConfigGroupLength.value = modemConfigGroupLength
    phyConfigEntry.channelConfigGroups.addElement(newChannelConfigGroup)

  def _generateChannelStructures(self, multiPhyConfigEntry, phyConfigEntry, channelConfigEntry):

    # Create a new Channel Config Entry
    newChannelConfigEntry = multiPhyConfigEntry.channelConfigEntries.newElement()
    newChannelConfigEntry.modemConfigDeltaAdd.value = phyConfigEntry.modemConfigEntry.value if len(channelConfigEntry.phy_config_delta_add) > 0 else None
    newChannelConfigEntry.baseFrequency.value = channelConfigEntry.base_frequency
    newChannelConfigEntry.channelSpacing.value = channelConfigEntry.channel_spacing
    newChannelConfigEntry.physicalChannelOffset.value = channelConfigEntry.physical_channel_offset
    newChannelConfigEntry.channelNumberStart.value = channelConfigEntry.channel_number_start
    newChannelConfigEntry.channelNumberEnd.value = channelConfigEntry.channel_number_end
    newChannelConfigEntry.maxPower.value = channelConfigEntry.max_power #"RAIL_TX_POWER_MAX"
    newChannelConfigEntry.attr.value = phyConfigEntry.channelConfigEntryAttr.value
    newChannelConfigEntry.entryType.value = phyConfigEntry.entryType.value
    newChannelConfigEntry.stackInfo.value = phyConfigEntry.stackInfo.value #Not serialized in multiphy XML
    newChannelConfigEntry.alternatePhy.value = phyConfigEntry.alternatePhy.value
    if self.rail_version >= 3:
      newChannelConfigEntry.phyInfoEntry.value = phyConfigEntry.phyInfoEntry.value
      modemConfigDeltaAddLength = 0
      if newChannelConfigEntry.modemConfigDeltaAdd.value is not None:
        for element in phyConfigEntry.modemConfigEntry.value._elements:
          modemConfigDeltaAddLength = modemConfigDeltaAddLength + element.length.value
      newChannelConfigEntry.modemConfigDeltaAddLength.value = modemConfigDeltaAddLength
      newChannelConfigEntry.channelConfigGroups.value = phyConfigEntry.channelConfigGroups
      newChannelConfigEntry.channelConfigGroupsLength.value = len(phyConfigEntry.channelConfigGroups._elements)

    # Traverse existing channelConfigEntries and check for duplicates
    entryFound = False
    for channelConfigEntry in multiPhyConfigEntry.channelConfigEntries._elements:
      if channelConfigEntry == newChannelConfigEntry:
        entryFound = True
        break

    if not entryFound:
      # Found no duplicates, so add the newChannelConfigEntry to the channelConfigEntries object
      multiPhyConfigEntry.channelConfigEntries.addElement(newChannelConfigEntry)

  def _generateChannelConfigs(self, railModel):

    for multiPhyConfigEntry in railModel.multiPhyConfig.multiPhyConfigEntries._elements:
      # Populate the channelConfig object
      channelConfig = multiPhyConfigEntry.channelConfig
      channelConfig.modemConfigBase.value = multiPhyConfigEntry.phyConfigEntries._elements[0].modemConfigEntryBase.value
      if self.rail_version >= 3:
        channelConfig.modemConfigCommonBase.value = multiPhyConfigEntry.phyConfigEntries._elements[0].modemConfigEntryCommonBase.value
      else:
        channelConfig.modemConfigDeltaSubtract.value = multiPhyConfigEntry.phyConfigEntries._elements[0].modemConfigEntrySubtract.value
      channelConfig.channelConfigEntries.value = multiPhyConfigEntry.channelConfigEntries
      channelConfig.length.value = len(multiPhyConfigEntry.channelConfigEntries._elements)
      channelConfig.signature.value = 0
      channelConfig.xtalFrequency.value = multiPhyConfigEntry.phyConfigEntries._elements[0].xtalFrequency.value

      if self.rail_version >= 3:
        modemConfigBaseLength = 0
        modemConfigCommonBaseLength = 0

        if channelConfig.modemConfigBase.value is not None:
          for element in multiPhyConfigEntry.phyConfigEntries._elements[0].modemConfigEntryBase.value._elements:
            modemConfigBaseLength = modemConfigBaseLength + element.length.value
        channelConfig.modemConfigBaseLength.value = modemConfigBaseLength

        if channelConfig.modemConfigCommonBase.value is not None:
          for element in multiPhyConfigEntry.phyConfigEntries._elements[0].modemConfigEntryCommonBase.value._elements:
            modemConfigCommonBaseLength = modemConfigCommonBaseLength + element.length.value
        channelConfig.modemConfigCommonBaseLength.value = modemConfigCommonBaseLength

  def _orderChannelConfigEntries(self, railModel):

    # The algorithm to order the channels is as follows:
    if (self.rail_version >= 3):
      # Sort all channel entries using the channelNumberStart first and then by
      # channelNumberEnd.
      # First step, sort the channelConfigEntries, define a custom compare function
      def compareChannelConfigEntries(a, b):
        if a.channelNumberStart.value < b.channelNumberStart.value:
          return -1
        elif a.channelNumberStart.value > b.channelNumberStart.value:
          return 1
        else:
          return 0
      # This function is used to find overlapping and duplicate entries.
      def findOverlappingAndDuplicateChannelConfigEntries(entries):
        for i in range(len(entries) - 1):
          if entries[i].channelNumberEnd.value >= entries[i+1].channelNumberStart.value:
            raise Exception("Channel config entries with duplicate or overlapping channel_number_start and channel_number_end found!")

      # This function is used to validate channel_number_end >= channel_number_start
      def validateChannelStartAndEnd(entries):
        for i in range(len(entries)):
          if (entries[i].channelNumberEnd.value < entries[i].channelNumberStart.value):
            raise Exception("channel_number_end < channel_number_start!")
    else:
      # Sort all channel entries using the channelNumberStart first, by maxPower
      # second, and by channelNumberEnd last.

      # First step, sort the channelConfigEntries, define a custom compare function
      def compareChannelConfigEntries(a, b):
        if a.channelNumberStart.value < b.channelNumberStart.value:
          return -1
        elif a.channelNumberStart.value > b.channelNumberStart.value:
          return 1
        else:
          # Now look at maxPower
          if parsePower(a.maxPower.value) < parsePower(b.maxPower.value):
            return -1
          elif parsePower(a.maxPower.value) > parsePower(b.maxPower.value):
            return 1
          else:
            if a.channelNumberEnd.value < b.channelNumberEnd.value:
                return -1
            elif a.channelNumberEnd.value > b.channelNumberEnd.value:
                return 1
            else:
              return 0

    for multiPhyConfigEntry in railModel.multiPhyConfig.multiPhyConfigEntries._elements:

      # Now, do the actual sort, using the compare function defined above
      channelConfigEntries = multiPhyConfigEntry.channelConfigEntries._elements
      channelConfigEntriesSorted = sorted(channelConfigEntries, key=cmp_to_key(compareChannelConfigEntries))

      if (self.rail_version >= 3):
        # Raise an exception if channelNumberEnd < channelNumberStart
        validateChannelStartAndEnd(channelConfigEntriesSorted)
        # Raise an exception if we find duplicate or overlapping entries.
        findOverlappingAndDuplicateChannelConfigEntries(channelConfigEntriesSorted)
      else:
        # Next, set the reference entry to the first element of the newly sorted
        # array, and call this the reference channel config entry. Iterate through
        # the remaining channel entries and check those that have maxPower <= than
        # the current reference; if the channelNumberEnd is <= to the one in the
        # reference, AND the the channelNumberStart is > the one in the reference
        # entry, move the current channel entry to the position currently occupied
        # by the reference config entry. When an item is moved, start over the loop
        # with the moved item now becoming the reference channel config entry.
        # If we traverse all the remaining items, and no changes in position occur,
        # we need to move the reference to the next channel config entry.

        # We need a variable to kick us out of this loop
        changed = False
        reference = 0
        while reference < len(channelConfigEntriesSorted):

          # Grab the maxPower for the current reference entry
          referenceChannelEntry = channelConfigEntriesSorted[reference]

          for idx, channelConfigEntry in enumerate(channelConfigEntriesSorted[reference+1:]):
            # Set this variable to False to kick us out in case nothing gets updated
            changed = False
            if parsePower(channelConfigEntry.maxPower.value) <= parsePower(referenceChannelEntry.maxPower.value):
              if channelConfigEntry.channelNumberEnd.value <= referenceChannelEntry.channelNumberEnd.value and \
                channelConfigEntry.channelNumberStart.value > referenceChannelEntry.channelNumberStart.value:
                itemToMove = channelConfigEntriesSorted.pop(idx + reference+1)
                channelConfigEntriesSorted.insert(reference, itemToMove)
                changed = True
                break
          if not changed:
            reference += 1

      # Now that all the entries are correctly sorted, replace the elements in the
      # multiPhyConfigEntry.channelConfigEntries._elements list
      multiPhyConfigEntry.channelConfigEntries._elements = channelConfigEntriesSorted

  def _resolveConvDecoderBuffer(self, railModel):

    maxConvDecodeBufferSize = 0
    # On Panther, we decided to ALWAYS write the FRC_CONVRAMADDR to HIGH RAM offset 0
    # On Lynx+, we decided to ALWAYS write the FRC_CONVRAMADDR to FRCRAM offset 0
    if self.series == 1:
      for multiPhyConfigEntry in railModel.multiPhyConfig.multiPhyConfigEntries._elements:
        for phyConfigEntry in multiPhyConfigEntry.phyConfigEntries._elements:
          if phyConfigEntry.convDecodeBufferSize.value > maxConvDecodeBufferSize:
            maxConvDecodeBufferSize = phyConfigEntry.convDecodeBufferSize.value

    railModel.multiPhyConfig.commonStructures.convDecodeBufferSize.value = maxConvDecodeBufferSize


  def _convertRmToRegisterList(self, registers):
    regs = []
    if registers is not None:
      for registerName in registers.keys():
        (block, register) = registerName.split(".")
        if block not in EXCLUDE_BLOCK_LIST:
          # registers[registerName].access == 'read-only'
          # if registers[registerName] != "":
          #   # This is a workaround for write only registers
          #   regs.append(self._regOutput(block, register, registers[registerName]))
          # else:
            eval_string = "registers[registerName].io"
            # value = eval(eval_string)
            value = registers[registerName].io
            regs.append(self._regOutput(block, register, value, registerName))

      # Check for duplicates since we won't know what to do with them
      regs = sorted(regs)

      prevAddr = None
      prevValue = None
      for reg in regs:
        if prevAddr != None and reg[0] == prevAddr:
          if prevValue != reg[1]:
            # raise Exception("Duplicate non-identical set of register 0x%.8x!" % reg[0])
            print("Error: Conflicting definition for register at 0x%.8x" % reg[0])
          else:
            print("Warning: Duplicate definition of register at 0x%.8x" % reg[0])
        prevAddr = reg[0]
        prevValue = reg[1]

    return regs

  def _genModeSwitchPhrs(self, radioConfigModel, phyConfigEntry):
    if radioConfigModel.part_family.lower() in ['sol', 'margay'] and hasattr(radioConfigModel.profile.outputs, 'wisun_phy_mode_id'):
      wisun_phy_mode_ids = getattr(radioConfigModel.profile.outputs, 'wisun_phy_mode_id', None)
      wisun_mode_switch_phrs = getattr(radioConfigModel.profile.outputs, 'wisun_mode_switch_phr', None)
      mode_switch_dict = self.railModel.multiPhyConfig.commonStructures.modeSwitchPhyModeIds.value

      # Make a copy of mode switch variables
      phyModeIdsIter = copy.deepcopy(wisun_phy_mode_ids)
      phrIter = copy.deepcopy(wisun_mode_switch_phrs)

      if wisun_phy_mode_ids is not None and wisun_mode_switch_phrs is not None:
        # Verify that these mode switch infos are not already saved in mode_switch_dict
        for index, phyModeId in enumerate(wisun_phy_mode_ids.var_value):
          if phyModeId in mode_switch_dict.keys():
            # If phyModeId is already in the array: it's a duplicate, then remove it and the PHR from lists
            phyModeIdsIter.var_value.remove(phyModeId)
            phrIter.var_value.remove(mode_switch_dict[phyModeId])
          else:
            # It's a new element: save it and its associated PHR
            mode_switch_dict[phyModeId] = wisun_mode_switch_phrs.var_value[index]

        # Verify that modified list are not empty
        if phyModeIdsIter.var_value and phrIter.var_value:
          # Extract elements left from the modified list: those are new elements
          phyConfigEntry.modeSwitchPhr.value = zip(phyModeIdsIter.var_value, phrIter.var_value)

        # Update dict
        self.railModel.multiPhyConfig.commonStructures.modeSwitchPhyModeIds.value = mode_switch_dict
      else:
        phyConfigEntry.modeSwitchPhr.value = None

  # -------- External ---------------------------------------------------------
  def setInstanceDict(self, mphyConfig):
    self.mphyConfig = mphyConfig

  def setSignatures(self, signatures):
    mPhyEntries = self.railModel.multiPhyConfig.multiPhyConfigEntries._elements
    for sig, multiPhyConfigEntry in zip(signatures, mPhyEntries):
      multiPhyConfigEntry.channelConfig.signature.value = sig

  def populateModel(self):

    #Check if we have an mphyConfig
    if self.mphyConfig == None:
      print("No mphyConfig configured. Please call the setInstanceDict method.")

    # In case populateModel gets called multiple times, start the internal
    # railModel object from scratch
    if self.railModel or self._railModelPopulated:
      self.railModel = RAILModel(self.yamlobject)
      self._railModelPopulated = False

    self.partFamily = self.mphyConfig.part_family

    # Create a proper rm object depending on partFamily
    if (self.mphyConfig.part_revision == 'ANY'
            or self.partFamily.upper() in RM_S1_PART_FAMILY_NAMES
            or self.partFamily.upper() in RM_S2_PART_FAMILY_NAMES
            or self.partFamily.upper() in ["RAINIER"]):
      rm_factory = RM_Factory(self.partFamily.upper())
    else:
      rm_factory = RM_Factory(self.partFamily.upper(), self.mphyConfig.part_revision)
    self.rm = rm_factory()

    # Create a structure that will be added to on a per-base channel reference basis.
    # We use this to move writes between base and add to optimize keeping
    # continuous writes as a block
    radio_configs = OrderedDict()

    # Create a dict that will be used to avoid duplicates in wisun_modeSwitchPhrs struct
    self.railModel.multiPhyConfig.commonStructures.modeSwitchPhyModeIds.value = OrderedDict()

    # Go through all base channel configurations (NOTE!, ask Rick to rename!)
    baseConfigAttr = 0
    for baseChannelConfig in self.mphyConfig.base_channel_configurations.base_channel_configuration:
      configName = baseChannelConfig.name
      baseConfigOptions = {}
      # Extract the optional arguments for this baseChannelConfig
      for argument in baseChannelConfig.optional_arguments.argument:
        baseConfigOptions[argument.key] = argument.value

      # Start off by creating a new instance of multiPhyConfigEntry, use the baseChannelConfig name
      multiPhyConfigEntry = self.railModel.multiPhyConfig.multiPhyConfigEntries.addNewElement(configName)
      # multiPhyConfigEntry.signature.signature.value = 0

      # Now, iterate through all the channel configs and mark the "base" (right now, it's always the first entry)
      for index, channelConfigEntry in enumerate(baseChannelConfig.channel_config_entries.channel_config_entry):
        isBaseConfig = index == 0
        channelConfigOptions = {}
        # Extract the optional arguments for this channelConfigEntry
        for argument in channelConfigEntry.optional_arguments.argument:
          channelConfigOptions[argument.key] = argument.value

        # Create a new phy configEntry config
        phyConfigEntry = multiPhyConfigEntry.phyConfigEntries.addNewElement(channelConfigEntry.name)
        radioConfigModel = channelConfigEntry.radio_configurator_output_model

        # Guard against PHYs which may not have a name or a profile. The
        # template omits a define for falsy values (None or "")
        if radioConfigModel and hasattr(radioConfigModel, "phy") \
           and hasattr(radioConfigModel.phy, "name"):
          multiPhyConfigEntry.phyName.value = radioConfigModel.phy.name
        else:
          multiPhyConfigEntry.phyName.value = ""
        if radioConfigModel and hasattr(radioConfigModel, "profile") \
           and hasattr(radioConfigModel.profile, "name"):
          multiPhyConfigEntry.profile.value = radioConfigModel.profile.name
        else:
          multiPhyConfigEntry.profile.value = ""

        if radioConfigModel.result_code != CalcStatus.Failure.value:
          if self.rc_version is None:
            self.rc_version = radioConfigModel.calc_version

          # Write model instance to RM device
          self._writeModelToRmDevice(isBaseConfig, baseChannelConfig, channelConfigEntry, phyConfigEntry, self.rm)

          # Handle Generic Lookup Tables
          self._generateLookupTable(radioConfigModel)

          if self.wifi_script is False and self.wifi_script_seqacc is False:
            #Handle Frame Type Configurations
            self._generateFrameTypeStructures(phyConfigEntry, radioConfigModel)

            #Handle IR Cal Settings
            baseConfigAttr = self._generateIrCalStructure(phyConfigEntry, radioConfigModel, isBaseConfig, baseConfigAttr)

            #Handle DCDC Retiming Settings
            self._generateDcdcRetimingStructure(phyConfigEntry, radioConfigModel)

            #Handle timing structures
            self._generateTimingStructure(phyConfigEntry, radioConfigModel)

            #Handle HFXO Retiming Settings
            self._generateHfxoRetimingStructure(phyConfigEntry, radioConfigModel)

            #Handle RFFPLL Settings
            self._generateRffpllStructure(phyConfigEntry, radioConfigModel)

            # Handle TX IR Cal Settings
            self._generateTxIrCalStructure(phyConfigEntry, radioConfigModel)

            # Handle modem compensation
            self._generateHardModemTxCompensation(radioConfigModel, phyConfigEntry)

            self._generatePhyInfoStructure(phyConfigEntry, baseConfigOptions, channelConfigOptions, radioConfigModel)

            #Handle Frame Coding tables
            self._generateFrameCodingTable(phyConfigEntry, radioConfigModel)

            # Handle generic lookup tables
            self._generateLookupTable(radioConfigModel)

            # Handle Dynamic Slicer Configuration (for OOK PHYs)
            self._generateDynamicSlicerConfiguration(phyConfigEntry, baseConfigOptions, radioConfigModel)

            # Generic Model Info
            phyConfigEntry.xtalFrequency.value = radioConfigModel.vars.xtal_frequency_hz.value
            phyConfigEntry.baseFrequency.value = radioConfigModel.vars.base_frequency_hz.value
            phyConfigEntry.bitrate.value = radioConfigModel.vars.bitrate.value
            phyConfigEntry.modType.value = radioConfigModel.vars.modulation_type.value
            phyConfigEntry.deviation.value = radioConfigModel.vars.deviation.value
            phyConfigEntry.synthResolution.value = radioConfigModel.vars.synth_res_actual.value
            phyConfigEntry.fecEnabled.value = bool(radioConfigModel.profile.outputs.get_output('fec_enabled').var_value)
            phyConfigEntry.convDecodeBufferSize.value = radioConfigModel.profile.outputs.get_output('frc_conv_decoder_buffer_size').var_value
            phyConfigEntry.arrayTable = None


            generic_lut = getattr(radioConfigModel.profile.outputs, 'generic_lookup_tables', None)

            try:
              var_value = getattr(generic_lut, 'var_value', None)
              if var_value and 'frameCodingTable' in var_value:
                phyConfigEntry.arrayTable = generic_lut.var_value['frameCodingTable']
            except (AttributeError, TypeError):
              pass
            phyConfigEntry.bchArray = None
            if getattr(radioConfigModel.profile.outputs, 'bch_lut_data', None):
              phyConfigEntry.bchArray = radioConfigModel.profile.outputs.get_output('bch_lut_data').var_value

            self._generateStackInfo(phyConfigEntry, radioConfigModel)

            # Handle alternate phy for Concurrent mode
            self._generateAlternatePhy(phyConfigEntry, radioConfigModel, channelConfigEntry)

            # Populate modeSwitchPhyModeIds
            self._genModeSwitchPhrs(radioConfigModel, phyConfigEntry)

            # Extract the concurrent PHY optional argument marker
            phyConfigEntry.entryType.value = 0 # initialize as non-concurrent PHY
            for optional_argument in baseChannelConfig.optional_arguments.argument:
              if (optional_argument.key == 'conc_phy_opt_hop'):
                phyConfigEntry.entryType.value = int(optional_argument.value)
                break

            # Check fecEnabled flag and convDecodeBufferSize are correctly configured
            if phyConfigEntry.fecEnabled.value:
              assert phyConfigEntry.convDecodeBufferSize.value > 0, "Incorrect configuration for FEC Enabled"

          regs_channel = self._convertRmToRegisterList(self.registers_channel)
          regs_base = self._convertRmToRegisterList(self.registers_base)
          if self.rail_version >= 3:
            regs_common = self._convertRmToRegisterList(self.registers_common)
            regs_group_dict = dict()
            for reg_group in self.register_groups_dict:
              regs_group_dict[reg_group] = self._convertRmToRegisterList(self.register_groups_dict[reg_group])
          else:
            regs_subtract = self._convertRmToRegisterList(self.registers_subtract)
          regs_channel = self._generateModemConfigEntries(phyConfigEntry, radioConfigModel, regs_channel)

          # NOTE! Special case: If regs_channel is empty, and the base is not, it means we need to include
          # the registers normally in the channel specific modemConfig in the base
          if not regs_channel:
            regs_base = self._generateModemConfigEntries(phyConfigEntry, radioConfigModel, regs_base)
          if self.rail_version >= 3:
            regs_common = self._generateModemConfigEntries(phyConfigEntry, radioConfigModel, regs_common)
          else:
            regs_subtract = self._generateModemConfigEntries(phyConfigEntry, radioConfigModel, regs_subtract)

          # Package metadata in a struct for unpacking after optimization
          meta = (configName, phyConfigEntry, multiPhyConfigEntry, channelConfigEntry)
          reference = baseChannelConfig.base_channel_reference
          reference = configName if reference is None else reference

          if self.wifi_script is False and self.wifi_script_seqacc is False:
            # Update regs for RAIL owned registers like FRC.CONVRAMADDR (series 1) & FRC.BLOCKRAMADDR
            regs_base, regs_channel = self._addRailOwnedRegsToConfigEntries(reference, phyConfigEntry, radioConfigModel, regs_base, regs_channel)
            if self.rail_version >= 3:
              for reg_group in regs_group_dict:
                _, regs_group_dict[reg_group] = self._addRailOwnedRegsToConfigEntries(reference, phyConfigEntry, radioConfigModel, regs_base, regs_group_dict[reg_group])

          if not reference in radio_configs:
            if self.rail_version >= 3:
              radio_configs[reference] = {
                "base": regs_base,
                "common": regs_common,
                "add": [(regs_channel, meta, regs_group_dict)],
              }
            else:
              radio_configs[reference] = {
                "base": regs_base,
                "subtract": regs_subtract,
                "add": [(regs_channel, meta)],
              }
          else:
            if self.rail_version >= 3:
              radio_configs[reference]["add"].append((regs_channel, meta, regs_group_dict))
            else:
              radio_configs[reference]["add"].append((regs_channel, meta))
        else:
          self._railModelPopulated = False
          print('Radio configurator had a failure, exiting rail scripts.')
          return

    # Optimize and write radio configs
    for _, radio_config in radio_configs.items():
      self.optimizeRadioConfig(radio_config)

      regs_base = radio_config["base"]
      if self.rail_version >= 3:
        regs_common = radio_config["common"]
      else:
        regs_subtract = radio_config["subtract"]

      if self.rail_version >= 3:
        for regs_channel, meta, regs_group_dict in radio_config["add"]:
          configName = meta[0]
          phyConfigEntry = meta[1]
          multiPhyConfigEntry = meta[2]
          channelConfigEntry = meta[3]
          self.formatModemConfigEntries(configName, phyConfigEntry, regs_base, True)
          self.formatModemConfigEntries(configName, phyConfigEntry, regs_channel)
          self.formatModemConfigEntries(configName, phyConfigEntry, regs_common, False, False)
          for reg_group in regs_group_dict:
            self.formatModemConfigEntries(configName, phyConfigEntry, regs_group_dict[reg_group], False, False, reg_group_name=reg_group)
            self._generateChannelConfigGroups(phyConfigEntry)
          # Handle Channel Lists
          self._generateChannelStructures(multiPhyConfigEntry, phyConfigEntry, channelConfigEntry)
      else:
        for regs_channel, meta in radio_config["add"]:
          configName = meta[0]
          phyConfigEntry = meta[1]
          multiPhyConfigEntry = meta[2]
          channelConfigEntry = meta[3]
          self.formatModemConfigEntries(configName, phyConfigEntry, regs_base, True)
          self.formatModemConfigEntries(configName, phyConfigEntry, regs_channel)
          self.formatModemConfigEntries(configName, phyConfigEntry, regs_subtract, False, True)
          # Handle Channel Lists
          self._generateChannelStructures(multiPhyConfigEntry, phyConfigEntry, channelConfigEntry)

    # Populate the channelConfigs objects
    self._generateChannelConfigs(self.railModel)

    # Sort all the channel config entries in the model
    self._orderChannelConfigEntries(self.railModel)

    # Resolve convDecoderBuffer
    self._resolveConvDecoderBuffer(self.railModel)

    # We can now mark the _railModelPopulated flag as True
    self._railModelPopulated = True
