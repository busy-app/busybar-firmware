#!/usr/bin/env python3

import os
import re
import yaml
import xml.etree.ElementTree as ET

blacklist = [
'EFR32BG13_BRD4305A',
'EFR32BG13_BRD4305C',
'EFR32BG13_BRD4306A',
'EFR32BG13_BRD4306B',
'EFR32BG1_BRD4300A',
'EFR32BG1_BRD4301A',
'EFR32BG1_BRD4302A',
'EFR32BG1_BRD4303A',
'EFR32MG1_BRD4300B',
'EFR32MG12_BRD4304A',
'EFR32MG13_BRD4305D',
'EFR32MG13_BRD4305E',
'EFR32MG13_BRD4306C',
'EFR32MG13_BRD4306D',
'BGM21_BRD4999A'
]

class BoardConfig:
    def __init__(self):
        self.defines = {}
        self.hwconf_data = {}
        self.board_id = None
        self.dir_name = None
        self.hfxo_ctune = None
        self.hfxo_freq = None
        self.lfxo_ctune = None

    def __str__(self):
        return 'kit_dir={}\n'.format(self.dir_name) \
             + 'board={}\n'.format(self.board_id) \
             + 'hfxo_ctune={}\n'.format(self.hfxo_ctune) \
             + 'lfxo_ctune={}\n'.format(self.lfxo_ctune)

def script_path():
    return os.path.dirname(os.path.realpath(__file__))

def parse_value(s):
    s = s.strip()
    if s[-1] == '\\':
        return s

    while s[0] == '(':
        if s[-1] != ')':
            raise Exception('Unmatched parenthesis')
        s = s[1:-1]

    if s[0].isdigit():
        while not s[-1].isdigit():
            s = s[:-1]

        if s.startswith('0x'):
            return int(s[2:], 16)
        try:
            return int(s)
        except Exception as e:
            pass

    return s

def get_defines(file):
    defines = {}
    with open(file, 'r') as f:
        for line in f:
            # first try to match value with parenthesis
            match = re.search('#define (\w+)\s+\((.+)\)', line)
            if match:
                key = match.group(1)
                value = parse_value(match.group(2))
                defines[key] = value
                continue

            # define with a value
            match = re.search('#define (\w+)\s+(\S+)', line)
            if match:
                key = match.group(1)
                value = parse_value(match.group(2))
                defines[key] = value
                continue

            # define without any value
            match = re.search('#define (\w+)', line)
            if match:
                key = match.group(1)
                defines[key] = None
    return defines

def load_defines(d, file):
    if not os.path.isfile(file):
        return
    d.update(get_defines(file))

def load_hwconf(c, path):
    hwconf_file = None
    for file in os.listdir(path):
        if file.endswith('.hwconf'):
            hwconf_file = path + '/' + file

    if hwconf_file:
        tree = ET.parse(hwconf_file)
        root = tree.getroot()
        if 'name' in root.attrib:
            c.device_name = root.attrib['name']
        if 'partId' in root.attrib:
            c.part_id = root.attrib['partId']

        mode = root.find('mode')
        for p in mode.iter('property'):
            property_id = p.attrib['propertyId']
            if property_id.startswith('ABP') or property_id.startswith('mode.'):
                continue
            key = property_id.split('.')[1]
            value = p.attrib['value']
            c.hwconf_data[key] = parse_value(value)

def find_values(c):
    # Try to guess board id based on folder
    match = re.search('(BRD.*)', c.dir_name)
    if match:
        c.board_id = match.group(1).lower()

    # Try to find board id from macro
    if not c.board_id:
        for key in c.defines.keys():
            if 'BRD' in key:
                match = re.search('(BRD.*)', key)
                if match:
                    c.board_id = match.group(1).lower()

    # Get HFXO Frequency from either header file or hwconf
    if 'BSP_CLK_HFXO_FREQ' in c.defines:
        c.hfxo_freq = c.defines['BSP_CLK_HFXO_FREQ']
    if 'BSP_CLK_HFXO_FREQ' in c.hwconf_data and not c.hfxo_freq:
        c.hfxo_freq = c.hwconf_data['BSP_CLK_HFXO_FREQ']
    # Get HFXO ctune value
    if 'BSP_CLK_HFXO_CTUNE' in c.defines:
        c.hfxo_ctune =c.defines['BSP_CLK_HFXO_CTUNE']
    if 'BSP_HFXO_CTUNE' in c.defines and not c.hfxo_ctune:
        c.hfxo_ctune = c.defines['BSP_HFXO_CTUNE']
    # Get LFXO ctune value
    if 'BSP_CLK_LFXO_CTUNE' in c.defines:
        c.lfxo_ctune = c.defines['BSP_CLK_LFXO_CTUNE']
    if 'BSP_LFXO_CTUNE' in c.defines and not c.lfxo_ctune:
        c.lfxo_ctune = c.defines['BSP_LFXO_CTUNE']
    if 'BSP_CLK_LFXO_PRESENT' in c.defines and c.defines['BSP_CLK_LFXO_PRESENT'] == 0:
        c.lfxo_ctune = None


if __name__ == '__main__':
    output_file = 'board_data.yaml'
    print('Writing board data to {}'.format(output_file))

    boards = []

    for d in os.scandir(script_path() + '/../../../kit'):
        if d.is_file() or d.name[0].islower() or d.name.startswith('EMDK') or d.name.startswith('EFM32') \
           or d.name.startswith('SLWSTK') or d.name.endswith('EFM32G') or d.name.endswith('EFM32HG') \
           or d.name.startswith('MGM') or d.name.startswith('ZGM') or d.name.startswith('BGM') \
           or d.name in blacklist:
            continue

        c = BoardConfig()
        c.dir_name = d.name
        load_defines(c.defines, d.path + '/config/bspconfig.h')
        load_defines(c.defines, d.path + '/config/hal-config-board.h')
        load_hwconf(c, d.path + '/config')

        find_values(c)
        if not c.board_id:
            raise Exception('Missing board id for {}'.format(c.dir_name))

        for b in boards:
            if b.board_id == c.board_id:
                raise Exception('Duplicate board id found {}, {}=={}'.format(c.board_id, b.dir_name, c.dir_name))

        boards.append(c)

    data = {}
    for board in boards:
        values = {}
        values['hfxo_ctune'] = board.hfxo_ctune
        if board.lfxo_ctune:
            values['lfxo_ctune'] = board.lfxo_ctune
        if board.hfxo_freq:
            values['hfxo_freq'] = board.hfxo_freq
        data[board.board_id] = values

    with open(output_file, 'w') as f:
        yaml.dump(data, f)
