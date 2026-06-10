from dataclasses import dataclass, field
from typing import Dict, Optional

import gdb


# Must match furi_hal_nvm.c
NVM_MAGIC_OFFSET = 0
NVM_VERSION_OFFSET = 4
VERSION_POINTER_OFFSET = 24

NVM_BASE = 0x40036400

NVM_DATA_MAGIC = 0xB00B0005
NVM_DATA_VERSION_MIN = 0x00000002

VERSION_STRUCT_MAGIC = 0xBE40

def get_struct_field(base, offset, type):
    cstr_type = gdb.lookup_type("char").pointer()
    base_bytes = base.cast(cstr_type)
    return (base_bytes + offset).cast(type.pointer()).dereference()

class Version:
    git_hash: str
    git_branch: str
    build_date: str
    version: str
    target: int
    build_is_dirty: bool
    firmware_origin: str
    git_origin: str

    def __init__(self, version_ptr):
        cstr_type = gdb.lookup_type("char").pointer()
        uint_type = gdb.lookup_type("unsigned int")
        ushort_type = gdb.lookup_type("unsigned short")
        uchar_type = gdb.lookup_type("unsigned char")

        magic = get_struct_field(version_ptr, 0, ushort_type)
        major = get_struct_field(version_ptr, 2, uchar_type)
        minor = get_struct_field(version_ptr, 3, uchar_type)

        if magic != VERSION_STRUCT_MAGIC:
            raise ValueError("Bad struct Version magic")
        if major < 1:
            raise ValueError("struct Version major version is too old")
        self.git_hash = get_struct_field(version_ptr, 4, cstr_type).string()
        self.git_branch = get_struct_field(version_ptr, 8, cstr_type).string()
        self.build_date = get_struct_field(version_ptr, 12, cstr_type).string()
        self.version = get_struct_field(version_ptr, 16, cstr_type).string()
        self.target = int(get_struct_field(version_ptr, 20, uchar_type))
        self.build_is_dirty = bool(get_struct_field(version_ptr, 21, uchar_type))
        if major > 1 or (major == 1 and minor >= 1):
            self.firmware_origin = get_struct_field(version_ptr, 24, cstr_type).string()
            self.git_origin = get_struct_field(version_ptr, 28, cstr_type).string()

class BsbFwVersion(gdb.Command):
    """Print the version of BusyBar's firmware."""

    def __init__(self):
        super(BsbFwVersion, self).__init__("fw-version", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        void_ptr_type = gdb.lookup_type("void").pointer()
        uint_type = gdb.lookup_type("unsigned int")
        nvm = gdb.Value(NVM_BASE).cast(void_ptr_type)

        nvm_magic = get_struct_field(nvm, NVM_MAGIC_OFFSET, uint_type)
        if nvm_magic != NVM_DATA_MAGIC:
            print("Wrong NVM magic")
            return
        nvm_version = get_struct_field(nvm, NVM_VERSION_OFFSET, uint_type)
        if nvm_version < NVM_DATA_VERSION_MIN:
            print("Old NVM version")
            return

        version_ptr = get_struct_field(nvm, VERSION_POINTER_OFFSET, void_ptr_type)

        if not version_ptr:
            print("Pointer to version struct is NULL")
            return

        try:
            v = Version(version_ptr)
            print("Firmware version on attached BusyBar:")
            print(f"\tVersion:     {v.version}")
            print(f"\tBuilt on:    {v.build_date}")
            print(f"\tGit branch:  {v.git_branch}")
            print(f"\tGit commit:  {v.git_hash}")
            print(f"\tDirty:       {v.build_is_dirty}")
            print(f"\tHW Target:   {v.target}")
            if v.firmware_origin:
                print(f"\tOrigin:      {v.firmware_origin}")
            if v.git_origin:
                print(f"\tGit origin:  {v.git_origin}")
        except ValueError as e:
            print(f"{e}")


BsbFwVersion()
