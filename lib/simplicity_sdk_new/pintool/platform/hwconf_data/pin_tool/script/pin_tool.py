#!/usr/bin/env python3

import Studio
import collections
import sys
import os

import sourceload

DEBUG_CHANGESET = False
DEBUG_REQUIREMENT = False

# Utilities for Python 3
if sys.version_info[0] == 3:
  basestring = (str, bytes)

# Aliases for the signals
ALIASES = {
  'COPI': 'TX',
  'CIPO': 'RX',
}

def on_load(func):
  def call(event, device):
    changeset = []
    func(device, changeset)
    return changeset

  object_filter = Studio.halConfig.newObjectNameEventFilter('.*')
  Studio.halConfig.addEventHandler(Studio.halConfig.DOCUMENT_LOAD, object_filter, call)

  return func


def on_requirement_change(func):
  def call(event, changed_requirement):
    changeset = []
    func(changed_requirement, changeset)
    if DEBUG_CHANGESET:
      print("changeset for req {}".format(changed_requirement.getName()), [str(c) for c in changeset])
    return changeset

  Studio.halConfig.addEventHandler(Studio.halConfig.REQUIREMENT_CHANGE, None, call)

  return func

def on_requirement_peripheral_selection(func):
  def call(event, changed_requirement):
    changeset = []
    func(changed_requirement, changeset)
    if DEBUG_CHANGESET:
      print("changeset for req psel {}".format(changed_requirement.getName()), [str(c) for c in changeset])
    return changeset

  Studio.halConfig.addEventHandler(Studio.halConfig.REQUIREMENT_MODULE_SELECTION, None, call)

  return func


def peripheral(mode, name):
    for p in mode.getPeripherals():
        if p.name.lower() == name.lower():
            return p
    return None


def add_change_handler(module_name, property_name, callback):
  def call(event, property):
    changeset = []
    callback(property, changeset)
    if DEBUG_CHANGESET:
      print("changeset for prop {}".format(property.id), [str(c) for c in changeset])
    return changeset

  event_filter = Studio.halConfig.newObjectPropertyEventFilter(module_name, property_name)

  Studio.halConfig.addEventHandler(Studio.halConfig.PROPERTY_CHANGE, event_filter, call)

  return call

class PinRoute:
  def __init__(self, pin=None, routes=None):
    self.pin = pin
    if routes:
      self.routes = routes
    else:
      self.routes = []

  @property
  def route_label(self):
    if len(self.routes) == 0:
      routes = ''
    else:
      labels = []
      for route in self.routes:
        labels += route.labels
      routes = ' | '.join(labels)  

    return routes

  def route_list(self, exclude=None):
    labels = []
    for route in self.routes:
      for label in route.labels:
        if exclude and label in exclude:
          continue
        else:
          labels.append(label)

    return ', '.join(labels)

  @property
  def label(self):
    custom_name = self.pin.ref('pin.name').get()

    if not custom_name and self.reserved:
      custom_name = 'GPIO'

    routes = self.route_label

    if custom_name and routes:
      return custom_name + ' | ' + routes
    elif custom_name:
      return custom_name
    elif routes:
      return routes
    else:
      return ''

  @property
  def color(self):
    # None => blue if label is non-empty
    # 'warning' => orange
    # 'conflict' => red
    if len(self.routes) > 1 or (self.reserved and len(self.routes) > 0):
      return 'conflict'

    return None

  @property
  def signal(self):
    new_value = [self.label, self.color]
    if new_value == ['', None]:
      new_value = None
    return new_value

  @property
  def reserved(self):
    return self.pin.ref('pin.reserve').enumIndex != 0
  
  def errors(self):
    if self.reserved and len(self.routes) > 0:
      yield (self.pin.ref('pin.reserve'),
             '{0} is reserved for use as GPIO, and can not be used for {1}'.format(self.pin.name, self.route_list()))
      for route in self.routes:
        yield (route.route.enablePropertyReference,
               '{0} can not be used for {1}, as it is reserved for use as GPIO'.format(self.pin.name, route.labels[0]))
    elif len(self.routes) > 1:
      for route in self.routes:
        yield (route.route.enablePropertyReference,
               '{0} on {1} conflicts with {2}'.format(route.labels[0], self.pin.name, self.route_list(exclude=[route.labels[0]])))


class Route:
  def __init__(self, selector=None, route=None, location=None):
    self.selector = selector
    self.route = route
    self.location = location

    self.module_names = [self.selector.module.name.split('.')[0]]
    custom_names = self.selector.module.ref('peripheral.name')
    if custom_names:
        custom_names = custom_names.get()
    if custom_names:
      self.module_names += custom_names.split(',')

  @property
  def labels(self):
    labels = []
    for name in self.module_names:
      if '_' in self.selector.module.name:
        # Underscode in module name means this is a sub-module, and the module name already includes the route name
        assert len(self.selector.routes) == 1
        labels.append(name)
      else:
        labels.append('{}_{}'.format(name, self.route.name))

    return labels

  @property
  def pin(self):
    return self.location.pin

  @property
  def pin_id(self):
    return self.location.pin.id
  

def active_routes(portio):
  for selector in portio.selectors:
    # Not all selectors have a location property
    if selector.locationPropertyReference:
      selector_location = selector.locationPropertyReference.get()
    else:
      selector_location = None

    for route in selector.routes:
      # If the route is enabled
      if route.enablePropertyReference.enumIndex == 1:
        for location in route.locations:
          # Select the correct location
          if (not selector_location) or (str(location.locationNumber) == selector_location):
            yield Route(selector, route, location)
            break


def validate_pin_names(portio, changeset):
  changeset.append(Studio.halConfig.newProblemGroup(portio.getMode(), 'pin_names'))
  pin_names = {}
  for portbank in portio.portBanks:
    for pin in portbank.pins:
      name = pin.getProperty('pin.name')
      if name:
        if name not in pin_names:
          pin_names[name] = []
        pin_names[name].append(pin)
  
  for name, pins in pin_names.items():
    if len(pins) > 1:
      for pin in pins:
        changeset.append(Studio.halConfig.newWarning(pin.ref('pin.name'), '{} name ({}) conflicts with {}'.format(pin.name, name, ', '.join([p.name for p in pins if p != pin]))))


def validate_peripheral_names(mode, changeset):
  changeset.append(Studio.halConfig.newProblemGroup(mode, 'peripheral_names'))
  peripheral_names = {}
  for peripheral in mode.getPeripherals():
    name_ref = peripheral.ref('peripheral.name')
    if name_ref:
      name = name_ref.get()
      if name:
        if name not in peripheral_names:
          peripheral_names[name] = []
        peripheral_names[name].append(peripheral)
  
  for name, peripherals in peripheral_names.items():
    if len(peripherals) > 1:
      for peripheral in peripherals:
        changeset.append(Studio.halConfig.newWarning(peripheral.ref('peripheral.name'), '{} name ({}) conflicts with {}'.format(peripheral.name, name, ', '.join([p.name for p in peripherals if p != peripheral]))))


def validate_routes(portio, changeset):

  # Stores the active abus routes and their properties
  abus_routes = {}
  # Stores the active routes and properties for Si91x device
  si91x_bus_routes = {}

  changeset.append(Studio.halConfig.newProblemGroup(portio.getMode(), 'validate_routes'))

  pin_assignments = collections.OrderedDict()
  signal_changes = []

  # Create a flat list of all pins
  for portbank in portio.portBanks:
    for pin in portbank.pins:
      pin_assignments[pin.id] = PinRoute(pin, [])

  # List all routes connected to the pin
  for route in active_routes(portio):
    pin_assignments[route.pin_id].pin = route.pin
    pin_assignments[route.pin_id].routes.append(route)
    
    # Accumulating all the active routes and properties for Si91x device. All Si91x pin names contains string GPIO
    if "siwg917" in portio.getMode().getDevice().getName():
      si91x_bus_routes[route.selector.getName()] = {}
      si91x_bus_routes[route.selector.getName()]['Route'] = route.route
      si91x_bus_routes[route.selector.getName()]['ModuleName'] = route.selector.getModule().getName()
      si91x_bus_routes[route.selector.getName()]['PortBankIndex'] = route.location.getPin().getPortBank().getIndex()
      si91x_bus_routes[route.selector.getName()]['Pin'] = route.location.getPin()
      si91x_bus_routes[route.selector.getName()]['PinName'] = route.location.getPin().getName()
      si91x_bus_routes[route.selector.getName()]['PinIndex'] = route.location.getPin().getIndex()
      si91x_bus_routes[route.selector.getName()]['RouteLabel'] = route.location.getRouteLabel() 

    # Accumulating all the active abus routes and route properties
    if route.route.getAportName() == "ABUS":
      abus_routes[route.selector.getName()] = {}
      abus_routes[route.selector.getName()]['Route'] = route.route
      abus_routes[route.selector.getName()]['ModuleName'] = route.selector.getModule().getName()
      abus_routes[route.selector.getName()]['PortBankIndex'] = route.location.getPin().getPortBank().getIndex()
      abus_routes[route.selector.getName()]['Pin'] = route.location.getPin()
      abus_routes[route.selector.getName()]['PinName'] = route.location.getPin().getName()
      abus_routes[route.selector.getName()]['PinIndex'] = route.location.getPin().getIndex()
      abus_routes[route.selector.getName()]['RouteLabel'] = route.location.getRouteLabel()


  validate_abus_routes(abus_routes, changeset)
  if "siwg917" in portio.getMode().getDevice().getName():
    # Device is Si91x
    validate_si91x_routes(si91x_bus_routes, changeset)

    # TODO: Check if route has special requirement
    #  - same   other route (Series 0 PRS channels)
    #  - exclusivity (Series 1 APORT)

  # Update the pin tool with assignments for all the pins
  for assignment in pin_assignments.values():
    new_value = assignment.signal
    if new_value != assignment.pin.assignmentSignal.get():
      # Value is different from current
      changeset.append(Studio.halConfig.newSignalSetting(assignment.pin.assignmentSignal, new_value))

    for (property, message) in assignment.errors():
      changeset.append(Studio.halConfig.newError(property, message))

# Checks for BUS over-subscription error, Polarity error and produces warnings
def validate_abus_routes(abus_routes, changeset):
  # Holds ABUS<->Client map information

  abus_active_clients = {'ABUSA_E': set([]), 'ABUSA_O': set([]), 'ABUSB_E': set([]), 'ABUSB_O': set([]), 'ABUSCD_E': set([]), 'ABUSCD_O': set([])}

  for analog_signal, route_properties in abus_routes.items():
    # Finding the polarity of the ABUS i.e. EVEN/ODD
    if route_properties['PinIndex'] % 2 == 0:
      bus_polarity = "E"
    else:
      bus_polarity = "O"

    # Finding the ABUS i.e. ABUSA/ABUSB/ABUSCD
    if route_properties["PortBankIndex"] == 0:
      bus_name = "A"
    elif route_properties["PortBankIndex"] == 1:
      bus_name = "B"
    elif route_properties["PortBankIndex"] == 2 or route_properties["PortBankIndex"] == 3:
      bus_name = "CD"

    # Appending the active analog peripheral to relevant ABUS, to which it is mapped to
    abus_active_clients["ABUS"+bus_name+"_"+bus_polarity].add(route_properties['ModuleName'])

    # Bus Over-subscription Error Check: No more than two analog peripherals should request access to EVEN/ODD BUS
    if len(abus_active_clients["ABUS"+bus_name+"_"+bus_polarity]) > 2:
      changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'ABUS{} OVER-SUBSCRIBED : not more than two peripherals allowed for an EVEN/ODD ABUS'.format(bus_name)))

    if "CEXT" in analog_signal:
      analog_signal_prefix = "CSEN_"
      if "CSEN_SENSE" in abus_routes:
        if 'X' in abus_routes[analog_signal_prefix + "SENSE"]["RouteLabel"] and 'X' in route_properties["RouteLabel"]:
          changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'POLARITY ERROR : cannot map both CSEN_SENSE and {} to even Buses'.format(analog_signal)))
        if 'Y' in abus_routes[analog_signal_prefix + "SENSE"]["RouteLabel"] and 'Y' in route_properties["RouteLabel"]:
          changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'POLARITY ERROR : cannot map both CSEN_SENSE and {} to odd Buses'.format(analog_signal)))

    # Polarity Error Check:  POS/NEG should not be connected to same BUS (EVEN/ODD)
    if "NEG" in analog_signal:

      # Slicing negative signal to fetch analog signal prefix and searching for positive signal
      analog_signal_prefix = analog_signal[:-3]
      if analog_signal_prefix + "POS" in abus_routes:
        if 'RouteLabel' not in abus_routes[analog_signal_prefix + "POS"] and abus_routes[analog_signal_prefix + "POS"]['PinIndex'] % 2 == 0 and route_properties['PinIndex'] % 2 == 0:
          changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'POLARITY ERROR : cannot map both {} and {} to even pins'.format(analog_signal_prefix+"POS",analog_signal)))
        if 'RouteLabel' not in abus_routes[analog_signal_prefix + "POS"] and abus_routes[analog_signal_prefix + "POS"]['PinIndex'] % 2 != 0 and route_properties['PinIndex'] % 2 != 0:
          changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'POLARITY ERROR : cannot map both {} and {} to odd pins'.format(analog_signal_prefix+"POS",analog_signal)))
        if 'X' in abus_routes[analog_signal_prefix + "POS"]['RouteLabel'] and 'X' in route_properties['RouteLabel']:
          changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'POLARITY ERROR : cannot map both {} and {} to even (Bus_X) Buses'.format(analog_signal_prefix+"POS",analog_signal)))
        if 'Y' in abus_routes[analog_signal_prefix + "POS"]['RouteLabel'] and 'Y' in route_properties['RouteLabel']:
          changeset.append(Studio.halConfig.newWarning(route_properties['Route'].enablePropertyReference,'POLARITY ERROR : cannot map both {} and {} to odd (Bus_Y) Buses'.format(analog_signal_prefix+"POS",analog_signal)))

# Checks for Si91x device module validations for using same power domain pins
def validate_si91x_routes(si91x_bus_routes, changeset):
  ulp_module_names = []
  #Fetch all the modules containing ULP pins active routes and store them in ulp_module_names
  for si91x_active_module, route_properties in si91x_bus_routes.items():
    if "ULP" in route_properties['PinName']:
      ulp_module_names.append(route_properties['ModuleName'])
  ulp_module_names = list(set(ulp_module_names))
  for si91x_active_module, route_properties in si91x_bus_routes.items():
    for i in ulp_module_names:
      if route_properties['ModuleName'] == i:
        if "ULP" not in route_properties['PinName']:
          # Module is having pins from different power domains (i.e) SoC and ULP domains
          changeset.append(Studio.halConfig.newWarning(route_properties['Pin'], '{} module should have pins from same power domain'.format(route_properties['ModuleName'])))

def prs_producer_update(prs_channel, changeset):
  producer = prs_channel.getProperty('producer')

  for property_id in prs_channel.getPropertyIds():
    if property_id.startswith('signal.'):
      changeset.append(Studio.halConfig.newIsHiddenSetting(
        prs_channel.ref(property_id),
        producer != property_id.strip('signal.')
      ))


def get_peripheral_by_type(device, required_type, is_pin):
  if is_pin:
    pins = []
    for port in device.getModes()[0].getPortIO().getPortBanks():
      pins += port.getPins()
    return pins
  else:
    peripherals = []
    for peripheral in device.getModes()[0].getPeripherals():
      if peripheral.getProperty('type') == required_type:
        peripherals.append(peripheral)
    return peripherals


def filter_peripherals(candidates, filters):
  # By default, all candidates are supported
  peripherals = candidates

  if 'capability' in filters:
    capabilities = filters['capability']
    if isinstance(capabilities, basestring):
      capabilities = [capabilities]
    else:
      capabilities = list(capabilities)

    compatible_peripherals = []
    for candidate in candidates:
      compatible = True
      for capability in capabilities:
        if candidate.getProperty('capability.{}'.format(capability)):
          # Capability is satisfied
          pass
        else:
          compatible = False

      if compatible:
        compatible_peripherals.append(candidate)

    # Only compatible peripherals are supported
    peripherals = compatible_peripherals

  if 'channel' in filters:
    channels = filters['channel']
    if isinstance(channels, basestring):
      channels = [channels]
    else:
      channels = list(channels)

    required_channel_count = len(channels)

    compatible_peripherals = []
    for candidate in peripherals:
      channel_count = candidate.getProperty('channel.count')
      if channel_count is not None:
        if int(channel_count) >= required_channel_count:
          # Candidate has sufficient number of channels
          compatible_peripherals.append(candidate)

    # Only compatible peripherals are supported
    peripherals = compatible_peripherals

  if 'signal' in filters:
    signals = filters['signal']
    if isinstance(signals, basestring):
      signals = [signals]
    else:
      signals = list(signals)

    required_signals = set()
    for signal in signals:
      if not signal.startswith('('):
        if signal in ALIASES:
          signal = ALIASES[signal]
        required_signals.add(signal)

    compatible_peripherals = []
    for candidate in candidates:
      if required_signals.issubset(get_signals(candidate)):
        compatible_peripherals.append(candidate)

    # Only compatible peripherals are supported
    peripherals = compatible_peripherals

  return peripherals


def process_added_requirement(requirement, changeset):
  # Set peripheral selection dropdown
  is_pin = requirement.type == 'gpio' and 'signal' not in requirement.properties
  # Get peripherals of the correct type
  candidates = get_peripheral_by_type(requirement.container.device, requirement.type, is_pin)

  # Filter using properties
  candidates = filter_peripherals(candidates, requirement.properties)
  requirement.setAllowedModules(candidates)

  # Load changes from source file (if any)
  options = sourceload.load_options_from_requirement(requirement)
  sourceload.load_requirement(requirement, candidates, options, changeset)

  if 'channel' in requirement.properties:
    update_channel_requirement(requirement, changeset)
  update_requirement_validation(requirement, changeset, True)


def process_removed_requirement(requirement, changeset):
  # Add empty problem group to erase errors
  changeset.append(Studio.halConfig.newProblemGroup(requirement.container.device.getModes()[0], 'req_{}_{}'.format(requirement.type, requirement.getName())))
  if requirement.getModule() is not None:
    if requirement.getModule().getProperty('channel.count') is not None:
      reset_channels(requirement.getModule(), changeset)
  # Remove changes from source file (if any)
  is_pin = requirement.type == 'gpio' and 'signal' not in requirement.properties
  candidates = get_peripheral_by_type(requirement.container.device, requirement.type, is_pin)
  options = sourceload.load_options_from_requirement(requirement)
  sourceload.remove_requirement(requirement, candidates, options, changeset)

def update_all_requirements_validation(mode, changeset):
  for req in mode.getDevice().getRequirements().requirements:
    update_requirement_validation(req, changeset)


def reset_channels(module, changeset):
  channel_count = int(module.getProperty('channel.count'))

  for i in range(channel_count):
    requirement_id = 'requirement.{}.channel'.format(i)
    changeset.append(Studio.halConfig.newPropertySetting(
      module.ref(requirement_id),
      '-1'
    ))
    changeset.append(Studio.halConfig.newIsHiddenSetting(
      module.ref(requirement_id),
      True
    ))


def update_channels(prop, changeset, name_changed):

  module = prop.object

  req_channels = []
  for req in module.getMode().getDevice().getRequirements().requirements:
    if not req.getModule() == module:
      continue
    
    if 'channel' in req.properties:
      req_channels = req.properties['channel']
    else:
      req_channels = []

    if isinstance(req_channels, basestring):
      req_channels = [req_channels]
    else:
      req_channels = list(req_channels)
    break
  else:
    pass

  if name_changed:
    # map name to requirement
    channel_name = prop.get()
    try:
      requirement_index = req_channels.index(channel_name)
      requirement = 'requirement.{}.channel'.format(requirement_index)
      channel_index = prop.getPropertyId().split('.')[1]

      if module.getProperty(requirement) != channel_index:
        changeset.append(Studio.halConfig.newPropertySetting(
          module.ref(requirement),
          channel_index
        ))
    except ValueError as e:
      # Nothing to do, new channel name does not match a requirement
      pass
  else:
    # map requirement to name
    requirement_name = prop.getLabel().split(' ')[0]
    channel_no = int(prop.get())
    channel_id = 'channel.{}.name'.format(channel_no)
    if module.getProperty(channel_id) != requirement_name:
      channel_count = module.getProperty('channel.count')
      for i in range(int(channel_count)):
        current_channel_id = 'channel.{}.name'.format(i)
        if channel_no == i:
          # Set the selected channel name to requirement name
          channel_name = requirement_name
        elif module.getProperty(current_channel_id) == requirement_name:
          # Remove requirement name from other channels
          channel_name = ""
        else:
          # Do not update unrelated channels
          continue
        changeset.append(Studio.halConfig.newPropertySetting(
          module.ref(current_channel_id),
          channel_name
        ))

  # TODO: Filter to only update requirements that are associated with the
  #       peripheral `property.object`
  update_all_requirements_validation(prop.object.mode, changeset)


def update_requirement_validation(req, changeset, added=False):
  # Add problem group for requirement
  changeset.append(
    Studio.halConfig.newProblemGroup(
      req.container.device.getModes()[0],
      'req_{}_{}'.format(req.type, req.getName())
    )
  )

  if req.getModule() is None:
    if 'optional' not in req.getProperties() and 'readonly' not in req.getProperties():
      changeset.append(
        Studio.halConfig.newWarning(
          req.container.device,
          'Requirement {} is unsatisified (needs {} instance) [from {}]'.format(
            req.name,
            req.type.upper(),
            os.path.basename(str(req.getSourceFile()))
          )
        )
    )
  else:
    # Requirement is satisfied, perform validation on contents
    if 'readonly' in req.properties:
      # Requirement is readonly, don't produce any errors/warnings about the content, as it isn't necessarily understood by the tool
      return

    # channel names
    if 'channel' in req.properties:
      req_channels = req.properties['channel']
      if isinstance(req_channels, basestring):
        req_channels = [req_channels]
      else:
        req_channels = list(req_channels)
      have_channels = []

      channel_count = req.getModule().getProperty('channel.count')
      if channel_count is None:
        channel_count = 0
      for i in range(int(channel_count)):
        have_channels.append(req.getModule().getProperty('channel.{}.name'.format(i)))

      for channel in req_channels:
        if channel not in have_channels:
          changeset.append(Studio.halConfig.newWarning(
            req.getModule(),
            '{} must have a channel named {} [from {}]'.format(req.getModule().label, channel, os.path.basename(str(req.getSourceFile())))
          ))

    # follow
    if 'follow' in req.properties:
      follow = req.properties['follow']
      for other in req.getContainer().requirements:
        if other.name == follow:
          other_module = other.getModule()
          if not other_module:
            changeset.append(Studio.halConfig.newWarning(
              req.container.device,
              '{} for {} must immediately follow the peripheral configured for {} [from {}]'.format(str(req.type).upper(), req.name, other.name, os.path.basename(str(req.getSourceFile())))
            ))
          else:
            other_instance = get_module_instance(other_module.name)
            this_instance = get_module_instance(req.getModule().name)

            if (int(other_instance) + 1) != int(this_instance):
              changeset.append(Studio.halConfig.newWarning(
                req.getModule(),
                '{} for {} must immediately follow {} [from {}]'.format(str(req.type).upper(), req.name, other_module.label, os.path.basename(str(req.getSourceFile())))
              ))

    # producer
    # TODO

    # signal enable
    if 'signal' in req.properties or (req.type == 'prs' and 'gpio' in req.properties):
      if req.type == 'prs':
        # For PRS with GPIO output, all signals must be implicitly enabled
        signals = get_signals(req.getModule())
      else:
        signals = req.properties['signal']
        if isinstance(signals, basestring):
          signals = [signals]
        else:
          signals = list(signals)

      ABSig_list = []
      ABSignal = []
      for i, val in enumerate(req.getModule().getSignals()):
        ABSignal.append((req.getModule().getSignals()[i].toString().split('.')[-1])[0:-1])
        for signal in signals:
          if signal in ALIASES:
            signal = ALIASES[signal]
          if ABSignal[i] in signal:
            ABSig_list.append(req.getModule().getSignals()[i])
      req.setVisibleSignals(ABSig_list)

      for signal in signals:
        if signal in ALIASES:
          signal = ALIASES[signal]
        if signal.startswith('('):
          continue
        prop, enabled = get_signal_enabled(req.getModule(), signal)
        if not enabled:
          changeset.append(Studio.halConfig.newWarning(
            prop,
            '{} must be enabled for {} [from {}]'.format(signal, req.getModule().label, os.path.basename(str(req.getSourceFile())))))


def update_channel_requirement(req, changeset):
  req_channels = req.properties['channel']
  if isinstance(req_channels, basestring):
    req_channels = [req_channels]
  else:
    req_channels = list(req_channels)
  have_channels = []

  if req.getModule() is None:
    return

  channel_count = req.getModule().getProperty('channel.count')
  if channel_count is None:
    channel_count = 0

  # Show requirement selectors
  for i, channel in enumerate(req_channels):
    req_channel_select = req.getModule().getPropertyReference('requirement.{}.channel'.format(i))
    if req_channel_select.isHidden():
      changeset.append(Studio.halConfig.newIsHiddenSetting(
        req_channel_select,
        False # not hidden
      ))

    # TODO Figure out how to do this without poking internal state of the ABProperty directly
    req_channel_select.getProperty().setLabel("{} channel".format(channel))

  # Hide unused requirement selectors
  for i in range(len(req_channels), int(channel_count)):
    req_channel_select = req.getModule().getPropertyReference('requirement.{}.channel'.format(i))
    if not req_channel_select.isHidden():
      changeset.append(Studio.halConfig.newIsHiddenSetting(
        req_channel_select,
        True # hidden
      ))

  for i in range(int(channel_count)):
    # FIXME Take differences already in changeset into consideration
    have_channels.append(req.getModule().getProperty('channel.{}.name'.format(i)))

  for i, channel in enumerate(req_channels):
    if channel in have_channels:
      channel_index = str(have_channels.index(channel))
      requirement_id = 'requirement.{}.channel'.format(i)

      if req.getModule().getProperty(requirement_id) != channel_index:
        changeset.append(Studio.halConfig.newPropertySetting(
          req.getModule().ref(requirement_id),
          channel_index
        ))


def get_signals(module):
  signals = []
  for selector in module.getMode().getPortIO().selectors:
    if selector.getModule().getName() != module.getName():
      # Not the desired module
      continue

    for route in selector.routes:
      signals.append(route.getName())

  return signals


def get_signal_enabled(module, signal):
  for selector in module.getMode().getPortIO().selectors:
    if selector.getModule().getName() != module.getName():
      # Not the desired module
      continue

    if selector.locationPropertyReference:
      location = selector.locationPropertyReference.get()
    else:
      location = None

    for route in selector.routes:
      if route.getName() != signal:
        # Not the desired route
        continue

      if route.enablePropertyReference.enumIndex == 1:
        return route.enablePropertyReference, True
      else:
        return route.enablePropertyReference, False

  return None, False


def get_module_instance(name):
  instance = name[len(name.rstrip('0123456789')):]

  return instance
