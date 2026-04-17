#!/usr/bin/env python3

import Studio
import pin_tool
import time

DEBUG = False
DEBUG_REQUIREMENT = False


def timeit(func):
  def call(*args, **kwargs):
    if not DEBUG:
      func(*args, **kwargs)
    else:
      print("{} :: {}".format(func.__name__, args[0] if len(args) else ""))
      start = time.time()
      func(*args, **kwargs)
      stop = time.time()
      print(" >> {:.3f} ms".format((stop - start) * 1000))

  return call

@timeit
def initial_route_setup(device, changeset):
  pin_tool.validate_routes(device.modes[0].portIO, changeset)
  pin_tool.validate_pin_names(device.modes[0].portIO, changeset)
  pin_tool.validate_peripheral_names(device.modes[0], changeset)


@timeit
def route_enable_changed(property, changeset):
  pin_tool.validate_routes(property.object, changeset)
  # TODO: Filter to only update requirements that are associated with the
  #       peripheral connected to this route
  pin_tool.update_all_requirements_validation(property.object.mode, changeset)


@timeit
def route_location_changed(property, changeset):
  pin_tool.validate_routes(property.object, changeset)


@timeit
def route_name_changed(property, changeset):
  pin_tool.validate_routes(property.object.mode.portIO, changeset)
  pin_tool.validate_peripheral_names(property.object.mode, changeset)


@timeit
def pin_changed(property, changeset):
  pin_tool.validate_routes(property.object.mode.portIO, changeset)
  pin_tool.validate_pin_names(property.object.mode.portIO, changeset)


@timeit
def prs_producer_changed(property, changeset):
  pin_tool.prs_producer_update(property.object, changeset)


@timeit
def channel_name_changed(property, changeset):
  pin_tool.update_channels(property, changeset, True)


@timeit
def channel_requirement_changed(property, changeset):
  pin_tool.update_channels(property, changeset, False)


@pin_tool.on_load
def on_load(device, changeset):
  print("Document load")

  import sys
  print(sys.version)

  # Route change
  pin_tool.add_change_handler('PORTIO', '.*enable.*', route_enable_changed)
  pin_tool.add_change_handler('PORTIO', '.*location.*', route_location_changed)

  # Pin names change
  pin_tool.add_change_handler('P[A-Z][0-9][0-5]?', 'pin.name', pin_changed)
  pin_tool.add_change_handler('P[A-Z][0-9][0-5]?', 'pin.reserve', pin_changed)

  # Route names change when peripheral name changes
  pin_tool.add_change_handler('.*', 'peripheral.name', route_name_changed)

  # PRS signals change visibility when producer changes
  pin_tool.add_change_handler('.*', 'producer', prs_producer_changed)

  # Channel name change
  pin_tool.add_change_handler('.*', 'channel.*name', channel_name_changed)
  pin_tool.add_change_handler('.*', 'requirement.*channel', channel_requirement_changed)


  # Sync pin state with route selections. Needed to make pins that are on by default visible in the UI.
  initial_route_setup(device, changeset)

  print("Document load complete")


@pin_tool.on_requirement_change
def on_requirement_change(requirement, changeset):
  if requirement in requirement.container.requirements:
    if DEBUG_REQUIREMENT:
      print("process_added_requirement {}".format(requirement.name))
    pin_tool.process_added_requirement(requirement, changeset)
  else:
    if DEBUG_REQUIREMENT:
      print("process_removed_requirement {}".format(requirement.name))
    pin_tool.process_removed_requirement(requirement, changeset)

  return changeset

@pin_tool.on_requirement_peripheral_selection
def on_peripheral_selection(requirement, changeset):
  if DEBUG_REQUIREMENT:
    if requirement.getModule() is None:
      print("Requirement {}: No peripheral selected".format(requirement.getName()))
    else:
      print("Requirement {}: {} selected".format(requirement.getName(), requirement.getModule().getName()))
  if 'channel' in requirement.properties:
    pin_tool.update_channel_requirement(requirement, changeset)
  pin_tool.update_requirement_validation(requirement, changeset)
  if requirement.getModule() is not None:
    # Update settings for selected module
    module = requirement.getModule()
    # Pin reservation
    pin_reserve = module.ref('pin.reserve')
    if pin_reserve is not None:
      if DEBUG_REQUIREMENT:
        print("  Add GPIO mode to {}".format(module.name))
      changeset.append(Studio.halConfig.newPropertySetting(
        pin_reserve,
        'Reserved'
      ))
  if requirement.getPreviousModule() is not None:
    # Update settings for deselected module
    module = requirement.getPreviousModule()
    # Reset channel settings
    if module.getProperty('channel.count') is not None:
      pin_tool.reset_channels(module, changeset)
    # Pin reservation
    pin_reserve = module.ref('pin.reserve')
    if pin_reserve is not None:
      for req in requirement.container.requirements:
        if req.getModule() == module:
          if DEBUG_REQUIREMENT:
            print("  Module {} still in use by {}".format(module.name, req.getName()))
          return
      if DEBUG_REQUIREMENT:
        print("  Remove GPIO mode from {}".format(module.name))
      changeset.append(Studio.halConfig.newPropertySetting(
        pin_reserve,
        'Not reserved'
      ))

