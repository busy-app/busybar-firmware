#!/usr/bin/env python3
"""
Dynamic Hardware Configuration data classes
"""

# Copyright 2025 Silicon Laboratories Inc. www.silabs.com
#
# SPDX-License-Identifier: Zlib
#
# The licensor of this software is Silicon Laboratories Inc.
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

import os
import struct
from ctypes import *
from typing import Union
from dataclasses import dataclass
import json

# PA Curve constants as defined in 'sl_rail_util_pa_nvm_configs.h'
SL_RAIL_NVM_PA_CONFIGS_VERSION = 1
SL_RAIL_NVM_PA_CURVE_SEGMENTS  = 9
SL_RAIL_NVM_PA_TABLE_ENTRIES   = 16

@dataclass
class Version:
    """Data class containing version"""
    version: c_uint8
    _format = '<B'
    _id = 0x00
    # Supported configuration version. Configurations with other values indicate that the configuration structure
    # has changed, therefore it cannot be interpreted.
    _SUPPORTED_VERSION = 1

    @classmethod
    def key(_):
        return "version"

    @classmethod
    def id(cls):
        return cls._id

    def __len__(self) -> int:
        return struct.calcsize(self._format)

    def validate(self) -> bool:
        return self.version == self._SUPPORTED_VERSION

    def to_bytes(self) -> bytes:
        return struct.pack(self._format, self.version)

    @classmethod
    def from_bytes(cls, stream: bytes):
        return cls(struct.unpack(cls._format, stream)[0])

    def to_json(self):
        return self.version

    @classmethod
    def from_json(cls, obj):
        return cls(obj)

@dataclass
class RssiOffset:
    """Data class containing rssi_offset"""
    rssi_offset: c_uint8
    _format = '<B'
    _id = 0x01

    @classmethod
    def key(_):
        return "rssi_offset"

    @classmethod
    def id(cls):
        return cls._id

    def __len__(self) -> int:
        return struct.calcsize(self._format)

    def to_bytes(self) -> bytes:
        return struct.pack(self._format, self.rssi_offset)

    @classmethod
    def from_bytes(cls, stream: bytes):
        return cls(struct.unpack(cls._format, stream)[0])

    def to_json(self):
        return self.rssi_offset

    @classmethod
    def from_json(cls, obj):
        return cls(obj)

@dataclass
class PaDescriptor:
    """Data class representing the sl_rail_nvm_pa_descriptor structure"""
    algorithm: c_uint8
    num_segments_or_entries: c_uint8
    min: c_uint8
    max: c_uint8
    _format = '<BBBB'

    def __len__(self) -> int:
        return struct.calcsize(self._format)

    def validate(self) -> bool:
        if self.algorithm == 0:
            return self.num_segments_or_entries == SL_RAIL_NVM_PA_CURVE_SEGMENTS
        else:
            return self.num_segments_or_entries == SL_RAIL_NVM_PA_TABLE_ENTRIES

    def to_bytes(self) -> bytes:
        return struct.pack(self._format,
                           self.algorithm, self.num_segments_or_entries, self.min, self.max)

    @classmethod
    def from_bytes(cls, stream: bytes):
        unpacked = struct.unpack(cls._format, stream)
        obj = cls(unpacked[0], unpacked[1], unpacked[2], unpacked[3])
        # Validate instance
        if not obj.validate():
            raise ValueError()
        return obj

    def to_json(self):
        obj = {}
        obj["algorithm"] = self.algorithm
        obj["num_segments_or_entries"] = self.num_segments_or_entries
        obj["min"] = self.min
        obj["max"] = self.max
        return obj

    @classmethod
    def from_json(cls, json_obj):
        obj = cls(json_obj["algorithm"], json_obj["num_segments_or_entries"], json_obj["min"], json_obj["max"])
        # Validate instance
        if not obj.validate():
            raise ValueError()
        return obj

@dataclass
class PaCurveSegment:
    """Data class representing the RAIL_TxPowerCurveSegment structure"""
    maxPowerLevel: c_uint16
    slope: c_int16
    intercept: c_int32
    _format = '<Hhi'

    def __len__(self) -> int:
        return struct.calcsize(self._format)

    def to_bytes(self) -> bytes:
        return struct.pack(self._format,
                           self.maxPowerLevel, self.slope, self.intercept)

    @classmethod
    def from_bytes(cls, stream: bytes):
        unpacked = struct.unpack(cls._format, stream)
        return cls(unpacked[0], unpacked[1], unpacked[2])

    def to_json(self):
        obj = {}
        obj["maxPowerLevel"] = self.maxPowerLevel
        obj["slope"] = self.slope
        obj["intercept"] = self.intercept
        return obj

    @classmethod
    def from_json(cls, obj):
        return cls(obj["maxPowerLevel"], obj["slope"], obj["intercept"])

@dataclass
class PaCurveAlgorithmCurve:
    """Data class representing the sl_rail_nvm_pa_curve_t structure"""
    curve_min_ddbm: c_int16
    curve_max_ddbm: c_int16
    curve_segments: list[PaCurveSegment]
    _format = '<hh'

    def __len__(self) -> int:
        length = struct.calcsize(self._format)
        for segment in self.curve_segments:
            length += len(segment)
        return length

    def to_bytes(self) -> bytes:
        data = bytearray(struct.pack(self._format,
                                     self.curve_min_ddbm, self.curve_max_ddbm))
        for segment in self.curve_segments:
            data.extend(segment.to_bytes())
        return bytes(data)

    @classmethod
    def from_bytes(cls, stream: bytes):
        i = struct.calcsize(cls._format)
        ddbm_min_max = struct.unpack(cls._format, stream[:i])

        # curve_segments
        segments = []
        segment_len = struct.calcsize(PaCurveSegment._format)
        while i < len(stream):
            segments.append(PaCurveSegment.from_bytes(stream[i:i + segment_len]))
            i += segment_len
        # Check the length of the remaining stream
        if i != len(stream):
            raise BufferError()
        return cls(ddbm_min_max[0], ddbm_min_max[1], segments)

    def to_json(self):
        obj = {}
        obj["curve_min_ddbm"] = self.curve_min_ddbm
        obj["curve_max_ddbm"] = self.curve_max_ddbm
        obj["curve_segments"] = []
        for segment in self.curve_segments:
            obj["curve_segments"].append(segment.to_json())
        return obj

    @classmethod
    def from_json(cls, obj):
        segments = []
        for segment in obj["curve_segments"]:
            segments.append(PaCurveSegment.from_json(segment))
        return cls(obj["curve_min_ddbm"], obj["curve_max_ddbm"], segments)

@dataclass
class PaCurveAlgorithmTable:
    """Data class representing the sl_rail_nvm_pa_table_t structure"""
    ddbm_values: list[c_int16]
    _format = '<h'

    def __len__(self) -> int:
        return len(self.ddbm_values) * struct.calcsize(self._format)

    def to_bytes(self) -> bytes:
        data = bytearray()
        for ddbm in self.ddbm_values:
            data.extend(struct.pack(self._format, ddbm))
        return bytes(data)

    @classmethod
    def from_bytes(cls, stream: bytes):
        i = 0
        ddbm_values = []
        ddbm_len = struct.calcsize(cls._format)
        while i < len(stream):
            ddbm_values.append(struct.unpack(cls._format, stream[i:i + ddbm_len])[0])
            i += ddbm_len
        # Check the length of the remaining stream
        if i != len(stream):
            raise BufferError()
        return cls(ddbm_values)

    def to_json(self):
        obj = {}
        obj["ddbm_values"] = []
        for ddbm in self.ddbm_values:
            obj["ddbm_values"].append(ddbm)
        return obj

    @classmethod
    def from_json(cls, obj):
        ddbm_values = []
        for ddbm in obj["ddbm_values"]:
            ddbm_values.append(ddbm)
        return cls(ddbm_values)

@dataclass
class PaCurve:
    """Data class representing the sl_rail_nvm_pa_config_t structure"""
    version: c_uint8
    num_descriptors: c_uint8
    pa_voltage: c_uint16
    signature: c_uint32
    # Size: num_descriptors
    pa_descriptors: list[PaDescriptor]
    # Size: num_descriptors. Should be interpreted according to pa_descriptors[i].algorithm
    pa_curve_or_table: list[Union[PaCurveAlgorithmCurve, PaCurveAlgorithmTable]]
    _pa_curve_or_table_union_size = max(
        struct.calcsize(PaCurveAlgorithmCurve._format) + SL_RAIL_NVM_PA_CURVE_SEGMENTS * struct.calcsize(PaCurveSegment._format),
        SL_RAIL_NVM_PA_TABLE_ENTRIES * struct.calcsize(PaCurveAlgorithmTable._format)
    )
    _format = '<BBHI'
    _id = 0x02
    # Supported version. Configurations with other values indicate that the structure has changed,
    # therefore it cannot be interpreted.
    _SUPPORTED_VERSION = 1

    @classmethod
    def key(_):
        return "pa_curves"

    @classmethod
    def id(cls):
        return cls._id

    def __len__(self) -> int:
        length = struct.calcsize(self._format)
        # pa_descriptors
        for i in range(0, self.num_descriptors):
            length += len(self.pa_descriptors[i])
        # pa_curve_or_table
        for i in range(0, self.num_descriptors):
            length += len(self.pa_curve_or_table[i])
        return length

    def validate(self) -> bool:
        if self._SUPPORTED_VERSION != SL_RAIL_NVM_PA_CONFIGS_VERSION:
            return False

        if not (self.num_descriptors == len(self.pa_descriptors) == len(self.pa_curve_or_table)):
            return False

        for i in range(0, self.num_descriptors):
            if self.pa_descriptors[i].algorithm == 0:
                segments = len(self.pa_curve_or_table[i].curve_segments)
                if segments != self.pa_descriptors[i].num_segments_or_entries:
                    return False
            else:
                entries = len(self.pa_curve_or_table[i].ddbm_values)
                if entries != self.pa_descriptors[i].num_segments_or_entries:
                    return False
        return True

    def to_bytes(self) -> bytes:
        data = bytearray(struct.pack(self._format,
                         self.version, self.num_descriptors, self.pa_voltage, self.signature))
        # pa_descriptors
        for i in range(0, self.num_descriptors):
            data.extend(self.pa_descriptors[i].to_bytes())

        # pa_curve_or_table
        for i in range(0, self.num_descriptors):
            data.extend(self.pa_curve_or_table[i].to_bytes())
            if len(self.pa_curve_or_table[i]) < self._pa_curve_or_table_union_size:
                # Add padding
                data.extend(b'\x00' * (self._pa_curve_or_table_union_size - len(self.pa_curve_or_table[i])))

        return bytes(data)

    @classmethod
    def from_bytes(cls, stream: bytes):
        i = struct.calcsize(cls._format)
        unpacked = struct.unpack(cls._format, stream[:i])

        # pa_descriptors
        num_descriptors = unpacked[1]
        pa_descriptors = []
        pa_descriptor_len = struct.calcsize(PaDescriptor._format)
        for _ in range(0, num_descriptors):
            pa_descriptors.append(PaDescriptor.from_bytes(stream[i:i + pa_descriptor_len]))
            i += pa_descriptor_len

        # pa_curve_or_table
        pa_curve_or_table = []
        for j in range(0, num_descriptors):
            if pa_descriptors[j].algorithm == 0:
                length = struct.calcsize(PaCurveAlgorithmCurve._format)
                length += pa_descriptors[j].num_segments_or_entries * struct.calcsize(PaCurveSegment._format)
                obj = PaCurveAlgorithmCurve.from_bytes(stream[i:i + length])
            else:
                length = pa_descriptors[j].num_segments_or_entries * struct.calcsize(PaCurveAlgorithmTable._format)
                obj = PaCurveAlgorithmTable.from_bytes(stream[i:i + length])
            pa_curve_or_table.append(obj)
            i += cls._pa_curve_or_table_union_size

        # Check the length of the remaining stream
        if i != len(stream):
            raise BufferError()
        obj = cls(unpacked[0], unpacked[1], unpacked[2], unpacked[3], pa_descriptors, pa_curve_or_table)
        # Validate instance
        if not obj.validate():
            raise ValueError()
        return obj

    def to_json(self):
        obj = {}
        obj["version"] = self.version
        obj["num_descriptors"] = self.num_descriptors
        obj["pa_voltage"] = self.pa_voltage
        obj["signature"] = self.signature
        # pa_descriptors
        obj["pa_descriptors"] = []
        for i in range(0, self.num_descriptors):
            obj["pa_descriptors"].append(self.pa_descriptors[i].to_json())
        # pa_curve_or_table
        obj["pa_curve_or_table"] = []
        for i in range(0, self.num_descriptors):
            obj["pa_curve_or_table"].append(self.pa_curve_or_table[i].to_json())
        return obj

    @classmethod
    def from_json(cls, json_obj):
        # pa_descriptors
        pa_descriptors = []
        for pa_descriptor in json_obj["pa_descriptors"]:
            pa_descriptors.append(PaDescriptor.from_json(pa_descriptor))
        # pa_curve_or_table
        pa_curve_or_table = []
        i = 0
        for item in json_obj["pa_curve_or_table"]:
            if pa_descriptors[i].algorithm == 0:
                pa_curve_or_table.append(PaCurveAlgorithmCurve.from_json(item))
            else:
                pa_curve_or_table.append(PaCurveAlgorithmTable.from_json(item))
            i += 1

        obj = cls(json_obj["version"],
                  json_obj["num_descriptors"],
                  json_obj["pa_voltage"],
                  json_obj["signature"],
                  pa_descriptors,
                  pa_curve_or_table)
        # Validate instance
        if not obj.validate():
            raise ValueError()
        return obj

@dataclass
class PaMode:
    """Data class containing pa_mode"""
    pa_mode: c_uint8
    _format = '<B'
    _id = 0x03
    # These PA mode values are evaluated to some other value, therefore should be ignored while verifying.
    # For more information, please refer to the 'RAIL_TxPowerMode_t' enum in 'rail_types.h'
    _PA_MODE_IGNORE = [ 4, 5, 10, 11 ]

    @classmethod
    def key(_):
        return "pa_mode"

    @classmethod
    def id(cls):
        return cls._id

    def __len__(self) -> int:
        return struct.calcsize(self._format)

    def __eq__(self, other):
        if (self.pa_mode in self._PA_MODE_IGNORE) or (other.pa_mode in other._PA_MODE_IGNORE):
            print("PaMode value exception. Skipping comparison.")
            return True
        return self.pa_mode == other.pa_mode

    def to_bytes(self) -> bytes:
        return struct.pack(self._format, self.pa_mode)

    @classmethod
    def from_bytes(cls, stream: bytes):
        return cls(struct.unpack(cls._format, stream)[0])

    def to_json(self):
        return self.pa_mode

    @classmethod
    def from_json(cls, obj):
        return cls(obj)

@dataclass
class Ctune:
    """Data class containing ctune"""
    ctune: c_uint32
    _format = '<I'
    _id = 0x04

    @classmethod
    def key(_):
        return "ctune"

    @classmethod
    def id(cls):
        return cls._id

    def __len__(self) -> int:
        return struct.calcsize(self._format)

    def to_bytes(self) -> bytes:
        return struct.pack(self._format, self.ctune)

    @classmethod
    def from_bytes(cls, stream: bytes):
        return cls(struct.unpack(cls._format, stream)[0])

    def to_json(self):
        return self.ctune

    @classmethod
    def from_json(cls, obj):
        return cls(obj)

@dataclass
class Configuration:
    """Data class containing the whole configuration structure"""
    version: Version
    rssi_offset: RssiOffset
    pa_curves: PaCurve
    pa_mode: PaMode
    ctune: Ctune

if __name__ == "__main__":
    print(__doc__, "Testing...", os.linesep)
    json_1 = json.load(open(os.path.abspath(os.path.join((__file__), '../configuration_sample.json'))))["silabs_dhc"]
    print("Input:", os.linesep, json_1, os.linesep)

    print("Instantiating.")
    if Version.key() in json_1:
        version = Version(json_1[Version.key()])
        print(Version.key(), Version.id(), version, sep='\t')
    if RssiOffset.key() in json_1:
        rssi_offset = RssiOffset(json_1[RssiOffset.key()])
        print(RssiOffset.key(), RssiOffset.id(), rssi_offset, sep='\t')
    if PaCurve.key() in json_1:
        pa_curves_json_1 = json_1[PaCurve.key()]
    if PaMode.key() in json_1:
        pa_mode = PaMode(json_1[PaMode.key()])
        print(PaMode.key(), PaMode.id(), pa_mode, sep='\t')
    if Ctune.key() in json_1:
        ctune = Ctune(json_1[Ctune.key()])
        print(Ctune.key(), Ctune.id(), ctune, sep='\t')

    # Test pa_curves
    pa_curves_1 = PaCurve.from_json(pa_curves_json_1)
    pa_curves_bytes_1 = pa_curves_1.to_bytes()
    pa_curves_2 = PaCurve.from_bytes(pa_curves_bytes_1)
    pa_curves_json_2 = pa_curves_2.to_json()
    pa_curves_bytes_2 = pa_curves_2.to_bytes()
    print(PaCurve.key(), PaCurve.id(), sep='\t')
    print(pa_curves_json_2, os.linesep)

    if pa_curves_bytes_1 != pa_curves_bytes_2: raise Exception()
    if pa_curves_json_1 != pa_curves_json_2: raise Exception()
    print("Bytes:", os.linesep, pa_curves_bytes_1)
    print("Len:", os.linesep, len(pa_curves_bytes_1), os.linesep)
    print("Success!")
