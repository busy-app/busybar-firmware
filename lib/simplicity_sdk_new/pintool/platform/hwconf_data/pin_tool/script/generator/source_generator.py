#!/usr/bin/env python3

from Studio import halConfig

import os
import sys

import sourcegen
import output_generator

def generate(context):
  print("Generate source")

  device = context.scope.getValue("object")
  config_dir = os.path.join(os.path.split(device.getDocument().getPath())[0], 'config')

  files_requirements = {}

  # Group requirements by source file
  for req in device.requirements.requirements:
    if req.getSourceFile() not in files_requirements:
      files_requirements[req.getSourceFile()] = [req]
    else:
      files_requirements[req.getSourceFile()].append(req)

  for filename in files_requirements:
    # Construct location accessor
    path = os.path.relpath(str(filename), config_dir).split(os.path.sep)
    location_id = 'dir(config)/'
    for d in path[:-1]:
      location_id += 'dir({})/'.format(d)
    location_id += 'file({})'.format(path[-1])

    with output_generator.File(context, location_id) as f:
      # Generate requirements into file
      for req in files_requirements[filename]:
        region_id = '{}_{}'.format(req.getType().upper(), req.getName())
        if req.properties.get('readonly'):
          print("Preserve read-only region {}".format(region_id))
          with output_generator.Region(context, region_id, owned=False) as region:
            pass
          continue
        module = req.getModule()
        if module is not None:
          print("Generate {} into region {}".format(module.getName(), region_id))

          with output_generator.Region(context, region_id) as region:
            sourcegen.generate_requirement(region, req, module)
        elif req.getPreviousModule() is not None:
          print("Erase region {}".format(region_id))
          with output_generator.Region(context, region_id) as region:
            pass
        else:
          print("Preserve region {}".format(region_id))
          with output_generator.Region(context, region_id, owned=False) as region:
            pass

  # General pin_config.h
  with output_generator.File(context, 'dir(config)/file(pin_config.h)') as f:
    f.write('#ifndef PIN_CONFIG_H')
    f.write('#define PIN_CONFIG_H')
    f.write('')

    for module in device.getModes()[0].getPeripherals():
      signals = sourcegen.get_signals(module)
      if signals:
        with output_generator.Region(context, module.getName()) as region:
          for signal in signals:
            output = sourcegen.generate_signal(region, module, signal, '{}_{}'.format(module.getName().split(' ')[0].split('.')[0], signal))
            if output:
              region.write('')

    # Output renamed GPIO pins
    with output_generator.Region(context, "CUSTOM_PIN_NAME") as region:
      for portbank in device.getModes()[0].getPortIO().getPortBanks():
        for pin in portbank.pins:
          pin_name = pin.getProperty('pin.name')
          if pin_name is not None and pin_name != "":
            sourcegen.generate_gpio(region, pin, pin_name)
            region.write('')

    f.write('')
    f.write('#endif // PIN_CONFIG_H')
    f.write('')
