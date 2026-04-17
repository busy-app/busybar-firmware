#!/usr/bin/env python3
from contextlib import contextmanager

class LocationManager():
  def __init__(self, context, location):
    self.context = context
    self.location = location
    self.emitted_defines = []

  def write(self, content):
    self.context.addContribution(self.location, content + '\n')

  def define(self, key, val, args=None):
    if args is not None:
      args = '({})'.format(','.join(args))
    else:
      args = ''

    macro = '{}{}'.format(key, args)
    text1 = '#ifndef {macro: <40}'.format(macro=macro)
    text2 = '#define {macro: <40} {value}'.format(macro=macro, value=val)

    if macro not in self.emitted_defines:
      self.write(text1)
      self.write(text2)
      self.write('#endif')
      self.emitted_defines.append(macro)

  def comment(self, comment):
    text = '// {}'.format(comment)
    self.write(text)


class Region():
  def __init__(self, context, name, owned=True):
    self.context = context
    self.name = name
    self.owned = owned

  def __enter__(self):
    parent = self.context.scope.getValue('location')
    attrs = {
      'parent': parent,
      'path': 'region({})'.format(self.name),
      'name': self.name,
      'owned': self.owned,
      'define': True,
      'packed': False,
      'scope': self.context.scope
    }

    region = self.context.declareLocation(attrs)
    self.manager = LocationManager(self.context, region)

    self.context.pushScope({'location': region})

    return self.manager

  def __exit__(self, *args):
    self.context.popScope()


class File():
  def __init__(self, context, path):
    self.context = context
    self.path = path

  def __enter__(self):
    root = self.context.scope.findLocation("root")
    attrs = {
      'parent': root,
      'path': self.path,
      'name': self.path.split('/')[-1][5:-1].replace('.','_'),
      'owned': False,
      'define': False,
      'packed': False,
      'scope': self.context.scope
    }

    file = self.context.declareLocation(attrs)
    manager = LocationManager(self.context, file)

    self.context.pushScope({'location': file})

    return manager

  def __exit__(self, *args):
      self.context.popScope()
