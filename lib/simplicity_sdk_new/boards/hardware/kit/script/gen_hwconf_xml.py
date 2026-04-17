#!/usr/bin/env python
import os
import glob
from lxml import etree

class HwconfFile(object):
	def __init__(self, part=None, board=None, path=None):
		self.part = part
		self.board = board
		self.path = path

	def __repr__(self):
		return "{} - {} - {}".format(self.part, self.board, self.path)

def part_from_hwconf_file(f):
	return etree.parse(f).getroot().get('partId')

def create_hwconf_database(kit_dir):
	"""
	Traverse all config folders and find .hwconf files
	Interpret file name as board name/compatibility
	Capture compatibility and path to file in HwconfFile objects
	"""
	hwconf_files = glob.glob("{}/*/config/*.hwconf".format(kit_dir))

	objects = []
	for f in sorted(hwconf_files):
		board_file = os.path.splitext(os.path.basename(f))[0]
		board = board_file.replace("rev", ".*")
		if board == 'brd4308a.*A01':
			board = 'brd4308a.*A0[01]'
		if board == 'brd4308b.*A01':
			board = 'brd4308b.*A0[01]'
		path = './' + os.path.relpath(f, kit_dir)
		part = part_from_hwconf_file(f)
		if not part:
			# Special case Pearl Gecko boards that don't have part defined
			# These are compatible with all Pearl and Jade Geckos
			if "EFM32PG" in path:
				family = path[2:].split('/')[0].split('_')[1][5:].lower()
				if family == 'pg':
					family = 'pg1'
				part = 'mcu.arm.efm32.({}|{})\..*'.format(family, family.replace('pg', 'jg'))
			else:
				raise ValueError("Invalid part ID from {}".format(path))

		objects.append(HwconfFile(part=part, board=board, path=path))

	return objects

def write_hwconf_database(kit_dir, hwconf_files):
	"""
	Create hwconf.xml in kit_dir based on compatibility information
	captured in HwconfFile objects
	"""
	root = etree.Element('contentItems')
	for obj in hwconf_files:
		data = etree.Element('contentItemData')
		prop = etree.Element('property')
		prop.attrib['key'] = 'partCompatibility'
		prop.attrib['value'] = obj.part
		data.append(prop)
		prop = etree.Element('property')
		prop.attrib['key'] = 'boardCompatibility'
		prop.attrib['value'] = obj.board
		data.append(prop)
		prop = etree.Element('property')
		prop.attrib['key'] = 'installationPath'
		prop.attrib['value'] = obj.path
		data.append(prop)
		root.append(data)

	doc = etree.ElementTree(root)
	doc.write(os.path.join(kit_dir, 'hwconf.xml'),
						xml_declaration=True,
						pretty_print=True,
						encoding='UTF-8')

if __name__ == '__main__':
	kit_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
	hwconf_files = create_hwconf_database(kit_dir)
	write_hwconf_database(kit_dir, hwconf_files)
