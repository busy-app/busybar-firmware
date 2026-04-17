r"""Wrapper for esl_lib.h

Generated with ctypesgen.

Do not modify this file.
"""

__docformat__ = "restructuredtext"

# Begin preamble for Python

import ctypes
import sys
from ctypes import *  # noqa: F401, F403

_int_types = (ctypes.c_int16, ctypes.c_int32)
if hasattr(ctypes, "c_int64"):
    # Some builds of ctypes apparently do not have ctypes.c_int64
    # defined; it's a pretty good bet that these builds do not
    # have 64-bit pointers.
    _int_types += (ctypes.c_int64,)
for t in _int_types:
    if ctypes.sizeof(t) == ctypes.sizeof(ctypes.c_size_t):
        c_ptrdiff_t = t
del t
del _int_types



class UserString:
    def __init__(self, seq):
        if isinstance(seq, bytes):
            self.data = seq
        elif isinstance(seq, UserString):
            self.data = seq.data[:]
        else:
            self.data = str(seq).encode()

    def __bytes__(self):
        return self.data

    def __str__(self):
        return self.data.decode()

    def __repr__(self):
        return repr(self.data)

    def __int__(self):
        return int(self.data.decode())

    def __long__(self):
        return int(self.data.decode())

    def __float__(self):
        return float(self.data.decode())

    def __complex__(self):
        return complex(self.data.decode())

    def __hash__(self):
        return hash(self.data)

    def __le__(self, string):
        if isinstance(string, UserString):
            return self.data <= string.data
        else:
            return self.data <= string

    def __lt__(self, string):
        if isinstance(string, UserString):
            return self.data < string.data
        else:
            return self.data < string

    def __ge__(self, string):
        if isinstance(string, UserString):
            return self.data >= string.data
        else:
            return self.data >= string

    def __gt__(self, string):
        if isinstance(string, UserString):
            return self.data > string.data
        else:
            return self.data > string

    def __eq__(self, string):
        if isinstance(string, UserString):
            return self.data == string.data
        else:
            return self.data == string

    def __ne__(self, string):
        if isinstance(string, UserString):
            return self.data != string.data
        else:
            return self.data != string

    def __contains__(self, char):
        return char in self.data

    def __len__(self):
        return len(self.data)

    def __getitem__(self, index):
        return self.__class__(self.data[index])

    def __getslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        return self.__class__(self.data[start:end])

    def __add__(self, other):
        if isinstance(other, UserString):
            return self.__class__(self.data + other.data)
        elif isinstance(other, bytes):
            return self.__class__(self.data + other)
        else:
            return self.__class__(self.data + str(other).encode())

    def __radd__(self, other):
        if isinstance(other, bytes):
            return self.__class__(other + self.data)
        else:
            return self.__class__(str(other).encode() + self.data)

    def __mul__(self, n):
        return self.__class__(self.data * n)

    __rmul__ = __mul__

    def __mod__(self, args):
        return self.__class__(self.data % args)

    # the following methods are defined in alphabetical order:
    def capitalize(self):
        return self.__class__(self.data.capitalize())

    def center(self, width, *args):
        return self.__class__(self.data.center(width, *args))

    def count(self, sub, start=0, end=sys.maxsize):
        return self.data.count(sub, start, end)

    def decode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.decode(encoding, errors))
            else:
                return self.__class__(self.data.decode(encoding))
        else:
            return self.__class__(self.data.decode())

    def encode(self, encoding=None, errors=None):  # XXX improve this?
        if encoding:
            if errors:
                return self.__class__(self.data.encode(encoding, errors))
            else:
                return self.__class__(self.data.encode(encoding))
        else:
            return self.__class__(self.data.encode())

    def endswith(self, suffix, start=0, end=sys.maxsize):
        return self.data.endswith(suffix, start, end)

    def expandtabs(self, tabsize=8):
        return self.__class__(self.data.expandtabs(tabsize))

    def find(self, sub, start=0, end=sys.maxsize):
        return self.data.find(sub, start, end)

    def index(self, sub, start=0, end=sys.maxsize):
        return self.data.index(sub, start, end)

    def isalpha(self):
        return self.data.isalpha()

    def isalnum(self):
        return self.data.isalnum()

    def isdecimal(self):
        return self.data.isdecimal()

    def isdigit(self):
        return self.data.isdigit()

    def islower(self):
        return self.data.islower()

    def isnumeric(self):
        return self.data.isnumeric()

    def isspace(self):
        return self.data.isspace()

    def istitle(self):
        return self.data.istitle()

    def isupper(self):
        return self.data.isupper()

    def join(self, seq):
        return self.data.join(seq)

    def ljust(self, width, *args):
        return self.__class__(self.data.ljust(width, *args))

    def lower(self):
        return self.__class__(self.data.lower())

    def lstrip(self, chars=None):
        return self.__class__(self.data.lstrip(chars))

    def partition(self, sep):
        return self.data.partition(sep)

    def replace(self, old, new, maxsplit=-1):
        return self.__class__(self.data.replace(old, new, maxsplit))

    def rfind(self, sub, start=0, end=sys.maxsize):
        return self.data.rfind(sub, start, end)

    def rindex(self, sub, start=0, end=sys.maxsize):
        return self.data.rindex(sub, start, end)

    def rjust(self, width, *args):
        return self.__class__(self.data.rjust(width, *args))

    def rpartition(self, sep):
        return self.data.rpartition(sep)

    def rstrip(self, chars=None):
        return self.__class__(self.data.rstrip(chars))

    def split(self, sep=None, maxsplit=-1):
        return self.data.split(sep, maxsplit)

    def rsplit(self, sep=None, maxsplit=-1):
        return self.data.rsplit(sep, maxsplit)

    def splitlines(self, keepends=0):
        return self.data.splitlines(keepends)

    def startswith(self, prefix, start=0, end=sys.maxsize):
        return self.data.startswith(prefix, start, end)

    def strip(self, chars=None):
        return self.__class__(self.data.strip(chars))

    def swapcase(self):
        return self.__class__(self.data.swapcase())

    def title(self):
        return self.__class__(self.data.title())

    def translate(self, *args):
        return self.__class__(self.data.translate(*args))

    def upper(self):
        return self.__class__(self.data.upper())

    def zfill(self, width):
        return self.__class__(self.data.zfill(width))


class MutableString(UserString):
    """mutable string objects

    Python strings are immutable objects.  This has the advantage, that
    strings may be used as dictionary keys.  If this property isn't needed
    and you insist on changing string values in place instead, you may cheat
    and use MutableString.

    But the purpose of this class is an educational one: to prevent
    people from inventing their own mutable string class derived
    from UserString and than forget thereby to remove (override) the
    __hash__ method inherited from UserString.  This would lead to
    errors that would be very hard to track down.

    A faster and better solution is to rewrite your program using lists."""

    def __init__(self, string=""):
        self.data = string

    def __hash__(self):
        raise TypeError("unhashable type (it is mutable)")

    def __setitem__(self, index, sub):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + sub + self.data[index + 1 :]

    def __delitem__(self, index):
        if index < 0:
            index += len(self.data)
        if index < 0 or index >= len(self.data):
            raise IndexError
        self.data = self.data[:index] + self.data[index + 1 :]

    def __setslice__(self, start, end, sub):
        start = max(start, 0)
        end = max(end, 0)
        if isinstance(sub, UserString):
            self.data = self.data[:start] + sub.data + self.data[end:]
        elif isinstance(sub, bytes):
            self.data = self.data[:start] + sub + self.data[end:]
        else:
            self.data = self.data[:start] + str(sub).encode() + self.data[end:]

    def __delslice__(self, start, end):
        start = max(start, 0)
        end = max(end, 0)
        self.data = self.data[:start] + self.data[end:]

    def immutable(self):
        return UserString(self.data)

    def __iadd__(self, other):
        if isinstance(other, UserString):
            self.data += other.data
        elif isinstance(other, bytes):
            self.data += other
        else:
            self.data += str(other).encode()
        return self

    def __imul__(self, n):
        self.data *= n
        return self


class String(MutableString, ctypes.Union):

    _fields_ = [("raw", ctypes.POINTER(ctypes.c_char)), ("data", ctypes.c_char_p)]

    def __init__(self, obj=b""):
        if isinstance(obj, (bytes, UserString)):
            self.data = bytes(obj)
        else:
            self.raw = obj

    def __len__(self):
        return self.data and len(self.data) or 0

    def from_param(cls, obj):
        # Convert None or 0
        if obj is None or obj == 0:
            return cls(ctypes.POINTER(ctypes.c_char)())

        # Convert from String
        elif isinstance(obj, String):
            return obj

        # Convert from bytes
        elif isinstance(obj, bytes):
            return cls(obj)

        # Convert from str
        elif isinstance(obj, str):
            return cls(obj.encode())

        # Convert from c_char_p
        elif isinstance(obj, ctypes.c_char_p):
            return obj

        # Convert from POINTER(ctypes.c_char)
        elif isinstance(obj, ctypes.POINTER(ctypes.c_char)):
            return obj

        # Convert from raw pointer
        elif isinstance(obj, int):
            return cls(ctypes.cast(obj, ctypes.POINTER(ctypes.c_char)))

        # Convert from ctypes.c_char array
        elif isinstance(obj, ctypes.c_char * len(obj)):
            return obj

        # Convert from object
        else:
            return String.from_param(obj._as_parameter_)

    from_param = classmethod(from_param)


def ReturnString(obj, func=None, arguments=None):
    return String.from_param(obj)


# As of ctypes 1.0, ctypes does not support custom error-checking
# functions on callbacks, nor does it support custom datatypes on
# callbacks, so we must ensure that all callbacks return
# primitive datatypes.
#
# Non-primitive return values wrapped with UNCHECKED won't be
# typechecked, and will be converted to ctypes.c_void_p.
def UNCHECKED(type):
    if hasattr(type, "_type_") and isinstance(type._type_, str) and type._type_ != "P":
        return type
    else:
        return ctypes.c_void_p


# ctypes doesn't have direct support for variadic functions, so we have to write
# our own wrapper class
class _variadic_function(object):
    def __init__(self, func, restype, argtypes, errcheck):
        self.func = func
        self.func.restype = restype
        self.argtypes = argtypes
        if errcheck:
            self.func.errcheck = errcheck

    def _as_parameter_(self):
        # So we can pass this variadic function as a function pointer
        return self.func

    def __call__(self, *args):
        fixed_args = []
        i = 0
        for argtype in self.argtypes:
            # Typecheck what we can
            fixed_args.append(argtype.from_param(args[i]))
            i += 1
        return self.func(*fixed_args + list(args[i:]))


def ord_if_char(value):
    """
    Simple helper used for casts to simple builtin types:  if the argument is a
    string type, it will be converted to it's ordinal value.

    This function will raise an exception if the argument is string with more
    than one characters.
    """
    return ord(value) if (isinstance(value, bytes) or isinstance(value, str)) else value

# End preamble

_libs = {}
_libdirs = []

# Begin loader

"""
Load libraries - appropriately for all our supported platforms
"""
# ----------------------------------------------------------------------------
# Copyright (c) 2008 David James
# Copyright (c) 2006-2008 Alex Holkner
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
#  * Neither the name of pyglet nor the names of its
#    contributors may be used to endorse or promote products
#    derived from this software without specific prior written
#    permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

import ctypes
import ctypes.util
import glob
import os.path
import platform
import re
import sys


def _environ_path(name):
    """Split an environment variable into a path-like list elements"""
    if name in os.environ:
        return os.environ[name].split(":")
    return []


class LibraryLoader:
    """
    A base class For loading of libraries ;-)
    Subclasses load libraries for specific platforms.
    """

    # library names formatted specifically for platforms
    name_formats = ["%s"]

    class Lookup:
        """Looking up calling conventions for a platform"""

        mode = ctypes.DEFAULT_MODE

        def __init__(self, path):
            super(LibraryLoader.Lookup, self).__init__()
            self.access = dict(cdecl=ctypes.CDLL(path, self.mode))

        def get(self, name, calling_convention="cdecl"):
            """Return the given name according to the selected calling convention"""
            if calling_convention not in self.access:
                raise LookupError(
                    "Unknown calling convention '{}' for function '{}'".format(
                        calling_convention, name
                    )
                )
            return getattr(self.access[calling_convention], name)

        def has(self, name, calling_convention="cdecl"):
            """Return True if this given calling convention finds the given 'name'"""
            if calling_convention not in self.access:
                return False
            return hasattr(self.access[calling_convention], name)

        def __getattr__(self, name):
            return getattr(self.access["cdecl"], name)

    def __init__(self):
        self.other_dirs = []

    def __call__(self, libname):
        """Given the name of a library, load it."""
        paths = self.getpaths(libname)

        for path in paths:
            # noinspection PyBroadException
            try:
                return self.Lookup(path)
            except Exception:  # pylint: disable=broad-except
                pass

        raise ImportError("Could not load %s." % libname)

    def getpaths(self, libname):
        """Return a list of paths where the library might be found."""
        if os.path.isabs(libname):
            yield libname
        else:
            # search through a prioritized series of locations for the library

            # we first search any specific directories identified by user
            for dir_i in self.other_dirs:
                for fmt in self.name_formats:
                    # dir_i should be absolute already
                    yield os.path.join(dir_i, fmt % libname)

            # check if this code is even stored in a physical file
            try:
                this_file = __file__
            except NameError:
                this_file = None

            # then we search the directory where the generated python interface is stored
            if this_file is not None:
                for fmt in self.name_formats:
                    yield os.path.abspath(os.path.join(os.path.dirname(__file__), fmt % libname))

            # now, use the ctypes tools to try to find the library
            for fmt in self.name_formats:
                path = ctypes.util.find_library(fmt % libname)
                if path:
                    yield path

            # then we search all paths identified as platform-specific lib paths
            for path in self.getplatformpaths(libname):
                yield path

            # Finally, we'll try the users current working directory
            for fmt in self.name_formats:
                yield os.path.abspath(os.path.join(os.path.curdir, fmt % libname))

    def getplatformpaths(self, _libname):  # pylint: disable=no-self-use
        """Return all the library paths available in this platform"""
        return []


# Darwin (Mac OS X)


class DarwinLibraryLoader(LibraryLoader):
    """Library loader for MacOS"""

    name_formats = [
        "lib%s.dylib",
        "lib%s.so",
        "lib%s.bundle",
        "%s.dylib",
        "%s.so",
        "%s.bundle",
        "%s",
    ]

    class Lookup(LibraryLoader.Lookup):
        """
        Looking up library files for this platform (Darwin aka MacOS)
        """

        # Darwin requires dlopen to be called with mode RTLD_GLOBAL instead
        # of the default RTLD_LOCAL.  Without this, you end up with
        # libraries not being loadable, resulting in "Symbol not found"
        # errors
        mode = ctypes.RTLD_GLOBAL

    def getplatformpaths(self, libname):
        if os.path.pathsep in libname:
            names = [libname]
        else:
            names = [fmt % libname for fmt in self.name_formats]

        for directory in self.getdirs(libname):
            for name in names:
                yield os.path.join(directory, name)

    @staticmethod
    def getdirs(libname):
        """Implements the dylib search as specified in Apple documentation:

        http://developer.apple.com/documentation/DeveloperTools/Conceptual/
            DynamicLibraries/Articles/DynamicLibraryUsageGuidelines.html

        Before commencing the standard search, the method first checks
        the bundle's ``Frameworks`` directory if the application is running
        within a bundle (OS X .app).
        """

        dyld_fallback_library_path = _environ_path("DYLD_FALLBACK_LIBRARY_PATH")
        if not dyld_fallback_library_path:
            dyld_fallback_library_path = [
                os.path.expanduser("~/lib"),
                "/usr/local/lib",
                "/usr/lib",
            ]

        dirs = []

        if "/" in libname:
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
        else:
            dirs.extend(_environ_path("LD_LIBRARY_PATH"))
            dirs.extend(_environ_path("DYLD_LIBRARY_PATH"))
            dirs.extend(_environ_path("LD_RUN_PATH"))

        if hasattr(sys, "frozen") and getattr(sys, "frozen") == "macosx_app":
            dirs.append(os.path.join(os.environ["RESOURCEPATH"], "..", "Frameworks"))

        dirs.extend(dyld_fallback_library_path)

        return dirs


# Posix


class PosixLibraryLoader(LibraryLoader):
    """Library loader for POSIX-like systems (including Linux)"""

    _ld_so_cache = None

    _include = re.compile(r"^\s*include\s+(?P<pattern>.*)")

    name_formats = ["lib%s.so", "%s.so", "%s"]

    class _Directories(dict):
        """Deal with directories"""

        def __init__(self):
            dict.__init__(self)
            self.order = 0

        def add(self, directory):
            """Add a directory to our current set of directories"""
            if len(directory) > 1:
                directory = directory.rstrip(os.path.sep)
            # only adds and updates order if exists and not already in set
            if not os.path.exists(directory):
                return
            order = self.setdefault(directory, self.order)
            if order == self.order:
                self.order += 1

        def extend(self, directories):
            """Add a list of directories to our set"""
            for a_dir in directories:
                self.add(a_dir)

        def ordered(self):
            """Sort the list of directories"""
            return (i[0] for i in sorted(self.items(), key=lambda d: d[1]))

    def _get_ld_so_conf_dirs(self, conf, dirs):
        """
        Recursive function to help parse all ld.so.conf files, including proper
        handling of the `include` directive.
        """

        try:
            with open(conf) as fileobj:
                for dirname in fileobj:
                    dirname = dirname.strip()
                    if not dirname:
                        continue

                    match = self._include.match(dirname)
                    if not match:
                        dirs.add(dirname)
                    else:
                        for dir2 in glob.glob(match.group("pattern")):
                            self._get_ld_so_conf_dirs(dir2, dirs)
        except IOError:
            pass

    def _create_ld_so_cache(self):
        # Recreate search path followed by ld.so.  This is going to be
        # slow to build, and incorrect (ld.so uses ld.so.cache, which may
        # not be up-to-date).  Used only as fallback for distros without
        # /sbin/ldconfig.
        #
        # We assume the DT_RPATH and DT_RUNPATH binary sections are omitted.

        directories = self._Directories()
        for name in (
            "LD_LIBRARY_PATH",
            "SHLIB_PATH",  # HP-UX
            "LIBPATH",  # OS/2, AIX
            "LIBRARY_PATH",  # BE/OS
        ):
            if name in os.environ:
                directories.extend(os.environ[name].split(os.pathsep))

        self._get_ld_so_conf_dirs("/etc/ld.so.conf", directories)

        bitage = platform.architecture()[0]

        unix_lib_dirs_list = []
        if bitage.startswith("64"):
            # prefer 64 bit if that is our arch
            unix_lib_dirs_list += ["/lib64", "/usr/lib64"]

        # must include standard libs, since those paths are also used by 64 bit
        # installs
        unix_lib_dirs_list += ["/lib", "/usr/lib"]
        if sys.platform.startswith("linux"):
            # Try and support multiarch work in Ubuntu
            # https://wiki.ubuntu.com/MultiarchSpec
            if bitage.startswith("32"):
                # Assume Intel/AMD x86 compat
                unix_lib_dirs_list += ["/lib/i386-linux-gnu", "/usr/lib/i386-linux-gnu"]
            elif bitage.startswith("64"):
                # Assume Intel/AMD x86 compatible
                unix_lib_dirs_list += [
                    "/lib/x86_64-linux-gnu",
                    "/usr/lib/x86_64-linux-gnu",
                ]
            else:
                # guess...
                unix_lib_dirs_list += glob.glob("/lib/*linux-gnu")
        directories.extend(unix_lib_dirs_list)

        cache = {}
        lib_re = re.compile(r"lib(.*)\.s[ol]")
        # ext_re = re.compile(r"\.s[ol]$")
        for our_dir in directories.ordered():
            try:
                for path in glob.glob("%s/*.s[ol]*" % our_dir):
                    file = os.path.basename(path)

                    # Index by filename
                    cache_i = cache.setdefault(file, set())
                    cache_i.add(path)

                    # Index by library name
                    match = lib_re.match(file)
                    if match:
                        library = match.group(1)
                        cache_i = cache.setdefault(library, set())
                        cache_i.add(path)
            except OSError:
                pass

        self._ld_so_cache = cache

    def getplatformpaths(self, libname):
        if self._ld_so_cache is None:
            self._create_ld_so_cache()

        result = self._ld_so_cache.get(libname, set())
        for i in result:
            # we iterate through all found paths for library, since we may have
            # actually found multiple architectures or other library types that
            # may not load
            yield i


# Windows


class WindowsLibraryLoader(LibraryLoader):
    """Library loader for Microsoft Windows"""

    name_formats = ["%s.dll", "lib%s.dll", "%slib.dll", "%s"]

    class Lookup(LibraryLoader.Lookup):
        """Lookup class for Windows libraries..."""

        def __init__(self, path):
            super(WindowsLibraryLoader.Lookup, self).__init__(path)
            self.access["stdcall"] = ctypes.windll.LoadLibrary(path)


# Platform switching

# If your value of sys.platform does not appear in this dict, please contact
# the Ctypesgen maintainers.

loaderclass = {
    "darwin": DarwinLibraryLoader,
    "cygwin": WindowsLibraryLoader,
    "win32": WindowsLibraryLoader,
    "msys": WindowsLibraryLoader,
}

load_library = loaderclass.get(sys.platform, PosixLibraryLoader)()


def add_library_search_dirs(other_dirs):
    """
    Add libraries to search paths.
    If library paths are relative, convert them to absolute with respect to this
    file's directory
    """
    for path in other_dirs:
        if not os.path.isabs(path):
            path = os.path.abspath(os.path.join(os.path.dirname(__file__), path))
        load_library.other_dirs.append(path)


del loaderclass

# End loader

add_library_search_dirs(['./lib'])

# Begin libraries

n = os.path.splitext(os.path.basename(__file__))[0]
if n.endswith("_wrapper"):
    n = n[:-len("_wrapper")]
p = sys.platform
if p == "darwin":
    _libs["{:s}.dylib".format(n)] = load_library(n)
elif (p == "cygwin" or p == "win32" or p == "msys"):
    _libs["{:s}.dll".format(n)] = load_library(n)
# Posix
else:
    _libs["{:s}.so".format(n)] = load_library(n)

# End libraries

# No modules

__uint8_t = c_ubyte# types.h: 38

__uint16_t = c_ushort# types.h: 40

__uint32_t = c_uint# types.h: 42

__uint64_t = c_ulong# types.h: 45

uint8_t = __uint8_t# stdint-uintn.h: 24

uint16_t = __uint16_t# stdint-uintn.h: 25

uint32_t = __uint32_t# stdint-uintn.h: 26

uint64_t = __uint64_t# stdint-uintn.h: 27

sl_status_t = uint32_t# sl_status.h: 504

enum_esl_lib_bool_e = c_int# esl_lib.h: 137

ESL_LIB_FALSE = 0# esl_lib.h: 137

ESL_LIB_TRUE = 1# esl_lib.h: 137

esl_lib_bool_t = enum_esl_lib_bool_e# esl_lib.h: 137

enum_esl_lib_node_id_type_e = c_int# esl_lib.h: 145

ESL_LIB_NODE_ID_TYPE_NONE = 0# esl_lib.h: 145

ESL_LIB_NODE_ID_TYPE_ADDRESS = (ESL_LIB_NODE_ID_TYPE_NONE + 1)# esl_lib.h: 145

ESL_LIB_NODE_ID_TYPE_CONNECTION = (ESL_LIB_NODE_ID_TYPE_ADDRESS + 1)# esl_lib.h: 145

ESL_LIB_NODE_ID_TYPE_PAWR = (ESL_LIB_NODE_ID_TYPE_CONNECTION + 1)# esl_lib.h: 145

esl_lib_node_id_type_t = enum_esl_lib_node_id_type_e# esl_lib.h: 145

enum_esl_lib_connection_mode_e = c_int# esl_lib.h: 151

ESL_LIB_CONNECTION_MODE_SINGLE = 0# esl_lib.h: 151

ESL_LIB_CONNECTION_MODE_LIST = (ESL_LIB_CONNECTION_MODE_SINGLE + 1)# esl_lib.h: 151

esl_lib_connection_mode_t = enum_esl_lib_connection_mode_e# esl_lib.h: 151

enum_esl_lib_connection_state_e = c_int# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_OFF = 0# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_CONNECTING = (ESL_LIB_CONNECTION_STATE_OFF + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_CONNECTION_OPENED = (ESL_LIB_CONNECTION_STATE_CONNECTING + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_APPLYING_LTK = (ESL_LIB_CONNECTION_STATE_CONNECTION_OPENED + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_NEW_BOND_REQUIRED = (ESL_LIB_CONNECTION_STATE_APPLYING_LTK + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_BONDING = (ESL_LIB_CONNECTION_STATE_NEW_BOND_REQUIRED + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_BONDING_RECOVERY = (ESL_LIB_CONNECTION_STATE_BONDING + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY = (ESL_LIB_CONNECTION_STATE_BONDING_RECOVERY + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_DIS_DISCOVERY = (ESL_LIB_CONNECTION_STATE_SERVICE_DISCOVERY + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY = (ESL_LIB_CONNECTION_STATE_DIS_DISCOVERY + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE = (ESL_LIB_CONNECTION_STATE_ESL_DISCOVERY + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_OTS_INIT = (ESL_LIB_CONNECTION_STATE_ESL_SUBSCRIBE + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_CONNECTED = (ESL_LIB_CONNECTION_STATE_OTS_INIT + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_GET_TAG_INFO = (ESL_LIB_CONNECTION_STATE_CONNECTED + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_OTS_IMAGE_TRANSFER = (ESL_LIB_CONNECTION_STATE_GET_TAG_INFO + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_OTS_GET_TYPE = (ESL_LIB_CONNECTION_STATE_OTS_IMAGE_TRANSFER + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_PAST_INIT = (ESL_LIB_CONNECTION_STATE_OTS_GET_TYPE + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION = (ESL_LIB_CONNECTION_STATE_PAST_INIT + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_WRITE_CONTROL_POINT = (ESL_LIB_CONNECTION_STATE_PAST_CLOSE_CONNECTION + 1)# esl_lib.h: 175

ESL_LIB_CONNECTION_STATE_CONFIGURE_TAG = (ESL_LIB_CONNECTION_STATE_WRITE_CONTROL_POINT + 1)# esl_lib.h: 175

esl_lib_connection_state_t = enum_esl_lib_connection_state_e# esl_lib.h: 175

enum_esl_lib_pawr_state_e = c_int# esl_lib.h: 183

ESL_LIB_PAWR_STATE_INIT = 0# esl_lib.h: 183

ESL_LIB_PAWR_STATE_IDLE = (ESL_LIB_PAWR_STATE_INIT + 1)# esl_lib.h: 183

ESL_LIB_PAWR_STATE_RUNNING = (ESL_LIB_PAWR_STATE_IDLE + 1)# esl_lib.h: 183

ESL_LIB_PAWR_STATE_RUNNING_ADVERTISING = (ESL_LIB_PAWR_STATE_RUNNING + 1)# esl_lib.h: 183

esl_lib_pawr_state_t = enum_esl_lib_pawr_state_e# esl_lib.h: 183

enum_esl_lib_core_state_e = c_int# esl_lib.h: 189

ESL_LIB_CORE_STATE_IDLE = 0# esl_lib.h: 189

ESL_LIB_CORE_STATE_CONNECTING = (ESL_LIB_CORE_STATE_IDLE + 1)# esl_lib.h: 189

esl_lib_core_state_t = enum_esl_lib_core_state_e# esl_lib.h: 189

esl_lib_pawr_handle_t = POINTER(None)# esl_lib.h: 192

esl_lib_connection_handle_t = POINTER(None)# esl_lib.h: 195

# esl_lib.h: 202
class struct_anon_19(Structure):
    pass

struct_anon_19.__slots__ = [
    'addr',
]
struct_anon_19._fields_ = [
    ('addr', uint8_t * int(6)),
]

# esl_lib.h: 201
class union_anon_20(Union):
    pass

union_anon_20.__slots__ = [
    'address',
    'addr',
]
union_anon_20._fields_ = [
    ('address', struct_anon_19),
    ('addr', uint8_t * int(6)),
]

# esl_lib.h: 200
class struct_anon_21(Structure):
    pass

struct_anon_21.__slots__ = [
    'unnamed_1',
    'address_type',
]
struct_anon_21._anonymous_ = [
    'unnamed_1',
]
struct_anon_21._fields_ = [
    ('unnamed_1', union_anon_20),
    ('address_type', uint8_t),
]

# esl_lib.h: 199
class union_anon_22(Union):
    pass

union_anon_22.__slots__ = [
    'unnamed_1',
    'u64',
    'bytes',
]
union_anon_22._anonymous_ = [
    'unnamed_1',
]
union_anon_22._fields_ = [
    ('unnamed_1', struct_anon_21),
    ('u64', uint64_t),
    ('bytes', uint8_t * int(sizeof(uint64_t))),
]

# esl_lib.h: 212
class struct_esl_lib_address_s(Structure):
    pass

struct_esl_lib_address_s.__slots__ = [
    'unnamed_1',
]
struct_esl_lib_address_s._anonymous_ = [
    'unnamed_1',
]
struct_esl_lib_address_s._fields_ = [
    ('unnamed_1', union_anon_22),
]

esl_lib_address_t = struct_esl_lib_address_s# esl_lib.h: 212

# esl_lib.h: 217
class struct_esl_lib_pawr_subevent_s(Structure):
    pass

struct_esl_lib_pawr_subevent_s.__slots__ = [
    'handle',
    'subevent',
]
struct_esl_lib_pawr_subevent_s._fields_ = [
    ('handle', esl_lib_pawr_handle_t),
    ('subevent', uint8_t),
]

esl_lib_pawr_subevent_t = struct_esl_lib_pawr_subevent_s# esl_lib.h: 217

# esl_lib.h: 222
class union_esl_lib_node_id_id_u(Union):
    pass

union_esl_lib_node_id_id_u.__slots__ = [
    'connection_handle',
    'pawr',
    'address',
]
union_esl_lib_node_id_id_u._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('pawr', esl_lib_pawr_subevent_t),
    ('address', esl_lib_address_t),
]

# esl_lib.h: 227
class struct_esl_lib_node_id_s(Structure):
    pass

struct_esl_lib_node_id_s.__slots__ = [
    'type',
    'id',
]
struct_esl_lib_node_id_s._fields_ = [
    ('type', esl_lib_node_id_type_t),
    ('id', union_esl_lib_node_id_id_u),
]

esl_lib_node_id_t = struct_esl_lib_node_id_s# esl_lib.h: 227

# esl_lib.h: 233
class struct_esl_lib_array_s(Structure):
    pass

struct_esl_lib_array_s.__slots__ = [
    'len',
    'data',
]
struct_esl_lib_array_s._fields_ = [
    ('len', uint8_t),
    ('data', uint8_t * int(0)),
]

esl_lib_array_t = struct_esl_lib_array_s# esl_lib.h: 233

# esl_lib.h: 239
class struct_esl_lib_long_array_s(Structure):
    pass

struct_esl_lib_long_array_s.__slots__ = [
    'len',
    'data',
]
struct_esl_lib_long_array_s._fields_ = [
    ('len', uint32_t),
    ('data', uint8_t * int(0)),
]

esl_lib_long_array_t = struct_esl_lib_long_array_s# esl_lib.h: 239

# esl_lib.h: 243
class struct_esl_lib_pawr_config_adv_interval_s(Structure):
    pass

struct_esl_lib_pawr_config_adv_interval_s.__slots__ = [
    'min',
    'max',
]
struct_esl_lib_pawr_config_adv_interval_s._fields_ = [
    ('min', uint16_t),
    ('max', uint16_t),
]

# esl_lib.h: 247
class struct_esl_lib_pawr_config_subevent_s(Structure):
    pass

struct_esl_lib_pawr_config_subevent_s.__slots__ = [
    'count',
    'interval',
]
struct_esl_lib_pawr_config_subevent_s._fields_ = [
    ('count', uint8_t),
    ('interval', uint8_t),
]

# esl_lib.h: 251
class struct_esl_lib_pawr_config_response_slot_s(Structure):
    pass

struct_esl_lib_pawr_config_response_slot_s.__slots__ = [
    'delay',
    'spacing',
    'count',
]
struct_esl_lib_pawr_config_response_slot_s._fields_ = [
    ('delay', uint8_t),
    ('spacing', uint8_t),
    ('count', uint8_t),
]

# esl_lib.h: 257
class struct_esl_lib_pawr_config_s(Structure):
    pass

struct_esl_lib_pawr_config_s.__slots__ = [
    'adv_interval',
    'subevent',
    'response_slot',
    'advertise',
]
struct_esl_lib_pawr_config_s._fields_ = [
    ('adv_interval', struct_esl_lib_pawr_config_adv_interval_s),
    ('subevent', struct_esl_lib_pawr_config_subevent_s),
    ('response_slot', struct_esl_lib_pawr_config_response_slot_s),
    ('advertise', esl_lib_bool_t),
]

esl_lib_pawr_config_t = struct_esl_lib_pawr_config_s# esl_lib.h: 257

# esl_lib.h: 275
class struct_esl_lib_scan_parameters_s(Structure):
    pass

struct_esl_lib_scan_parameters_s.__slots__ = [
    'mode',
    'interval',
    'window',
    'scanning_phy',
    'discover_mode',
]
struct_esl_lib_scan_parameters_s._fields_ = [
    ('mode', uint8_t),
    ('interval', uint16_t),
    ('window', uint16_t),
    ('scanning_phy', uint8_t),
    ('discover_mode', uint8_t),
]

esl_lib_scan_parameters_t = struct_esl_lib_scan_parameters_s# esl_lib.h: 275

enum_esl_lib_evt_type_e = c_int# esl_lib.h: 301

ESL_LIB_EVT_SYSTEM_BOOT = 0# esl_lib.h: 301

ESL_LIB_EVT_CONNECTION_MODE = (ESL_LIB_EVT_SYSTEM_BOOT + 1)# esl_lib.h: 301

ESL_LIB_EVT_SCAN_STATUS = (ESL_LIB_EVT_CONNECTION_MODE + 1)# esl_lib.h: 301

ESL_LIB_EVT_TAG_FOUND = (ESL_LIB_EVT_SCAN_STATUS + 1)# esl_lib.h: 301

ESL_LIB_EVT_TAG_INFO = (ESL_LIB_EVT_TAG_FOUND + 1)# esl_lib.h: 301

ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE = (ESL_LIB_EVT_TAG_INFO + 1)# esl_lib.h: 301

ESL_LIB_EVT_CONNECTION_RETRY = (ESL_LIB_EVT_CONFIGURE_TAG_RESPONSE + 1)# esl_lib.h: 301

ESL_LIB_EVT_CONNECTION_CLOSED = (ESL_LIB_EVT_CONNECTION_RETRY + 1)# esl_lib.h: 301

ESL_LIB_EVT_CONNECTION_OPENED = (ESL_LIB_EVT_CONNECTION_CLOSED + 1)# esl_lib.h: 301

ESL_LIB_EVT_BONDING_DATA = (ESL_LIB_EVT_CONNECTION_OPENED + 1)# esl_lib.h: 301

ESL_LIB_EVT_BONDING_FINISHED = (ESL_LIB_EVT_BONDING_DATA + 1)# esl_lib.h: 301

ESL_LIB_EVT_PAWR_CONFIG = (ESL_LIB_EVT_BONDING_FINISHED + 1)# esl_lib.h: 301

ESL_LIB_EVT_PAWR_STATUS = (ESL_LIB_EVT_PAWR_CONFIG + 1)# esl_lib.h: 301

ESL_LIB_EVT_PAWR_RESPONSE = (ESL_LIB_EVT_PAWR_STATUS + 1)# esl_lib.h: 301

ESL_LIB_EVT_PAWR_DATA_REQUEST = (ESL_LIB_EVT_PAWR_RESPONSE + 1)# esl_lib.h: 301

ESL_LIB_EVT_SHUTDOWN_READY = (ESL_LIB_EVT_PAWR_DATA_REQUEST + 1)# esl_lib.h: 301

ESL_LIB_EVT_ERROR = (ESL_LIB_EVT_SHUTDOWN_READY + 1)# esl_lib.h: 301

ESL_LIB_EVT_IMAGE_TRANSFER_FINISHED = (ESL_LIB_EVT_ERROR + 1)# esl_lib.h: 301

ESL_LIB_EVT_IMAGE_TYPE = (ESL_LIB_EVT_IMAGE_TRANSFER_FINISHED + 1)# esl_lib.h: 301

ESL_LIB_EVT_CONTROL_POINT_RESPONSE = (ESL_LIB_EVT_IMAGE_TYPE + 1)# esl_lib.h: 301

ESL_LIB_EVT_CONTROL_POINT_NOTIFICATION = (ESL_LIB_EVT_CONTROL_POINT_RESPONSE + 1)# esl_lib.h: 301

ESL_LIB_EVT_GENERAL = (ESL_LIB_EVT_CONTROL_POINT_NOTIFICATION + 1)# esl_lib.h: 301

esl_lib_evt_type_t = enum_esl_lib_evt_type_e# esl_lib.h: 301

enum_esl_lib_status_e = c_int# esl_lib.h: 352

ESL_LIB_STATUS_NO_ERROR = 0# esl_lib.h: 352

ESL_LIB_STATUS_UNSPECIFIED_ERROR = (ESL_LIB_STATUS_NO_ERROR + 1)# esl_lib.h: 352

ESL_LIB_STATUS_UNASSOCIATED = (ESL_LIB_STATUS_UNSPECIFIED_ERROR + 1)# esl_lib.h: 352

ESL_LIB_STATUS_GATT_TIMEOUT = (ESL_LIB_STATUS_UNASSOCIATED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_BONDING_FAILED = (ESL_LIB_STATUS_GATT_TIMEOUT + 1)# esl_lib.h: 352

ESL_LIB_STATUS_FEATURE_NOT_SUPPORTED = (ESL_LIB_STATUS_BONDING_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_INVALID_STATE = (ESL_LIB_STATUS_FEATURE_NOT_SUPPORTED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_INVALID_PARAMETERS = (ESL_LIB_STATUS_INVALID_STATE + 1)# esl_lib.h: 352

ESL_LIB_STATUS_UNEXPECTED_RESET = (ESL_LIB_STATUS_INVALID_PARAMETERS + 1)# esl_lib.h: 352

ESL_LIB_STATUS_GATT_ERROR = (ESL_LIB_STATUS_UNEXPECTED_RESET + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_TIMEOUT = (ESL_LIB_STATUS_GATT_ERROR + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_LOST = (ESL_LIB_STATUS_CONN_TIMEOUT + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_FAILED = (ESL_LIB_STATUS_CONN_LOST + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_CLOSE_FAILED = (ESL_LIB_STATUS_CONN_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_DISCOVERY_FAILED = (ESL_LIB_STATUS_CONN_CLOSE_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_ESL_SERVICE_VIOLATION = (ESL_LIB_STATUS_CONN_DISCOVERY_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_SUBSCRIBE_FAILED = (ESL_LIB_STATUS_CONN_ESL_SERVICE_VIOLATION + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_READ_FAILED = (ESL_LIB_STATUS_CONN_SUBSCRIBE_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_CONFIG_FAILED = (ESL_LIB_STATUS_CONN_READ_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_SET_MODE_FAILED = (ESL_LIB_STATUS_CONN_CONFIG_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_LINK_LAYER = (ESL_LIB_STATUS_CONN_SET_MODE_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_NO_PAWR_SYNC = (ESL_LIB_STATUS_LINK_LAYER + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_ERROR = (ESL_LIB_STATUS_NO_PAWR_SYNC + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_GOTO_FAILED = (ESL_LIB_STATUS_OTS_ERROR + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_INIT_FAILED = (ESL_LIB_STATUS_OTS_GOTO_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_WRITE_RESP_FAILED = (ESL_LIB_STATUS_OTS_INIT_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_UNEXPECTED_OFFSET = (ESL_LIB_STATUS_OTS_WRITE_RESP_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_TRANSFER_FAILED = (ESL_LIB_STATUS_OTS_UNEXPECTED_OFFSET + 1)# esl_lib.h: 352

ESL_LIB_STATUS_OTS_META_READ_FAILED = (ESL_LIB_STATUS_OTS_TRANSFER_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_SCAN_START_FAILED = (ESL_LIB_STATUS_OTS_META_READ_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_SCAN_STOP_FAILED = (ESL_LIB_STATUS_SCAN_START_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_SCAN_CONFIG_FAILED = (ESL_LIB_STATUS_SCAN_STOP_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_INIT_FAILED = (ESL_LIB_STATUS_SCAN_CONFIG_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_PAWR_START_FAILED = (ESL_LIB_STATUS_INIT_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_PAWR_STOP_FAILED = (ESL_LIB_STATUS_PAWR_START_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_PAWR_CONFIG_FAILED = (ESL_LIB_STATUS_PAWR_STOP_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_PAWR_SET_DATA_FAILED = (ESL_LIB_STATUS_PAWR_CONFIG_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_RESOURCE_EXCEEDED = (ESL_LIB_STATUS_PAWR_SET_DATA_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_PAST_INIT_FAILED = (ESL_LIB_STATUS_RESOURCE_EXCEEDED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_WRITE_CP_FAILED = (ESL_LIB_STATUS_PAST_INIT_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_TAG_CONFIG_FAILED = (ESL_LIB_STATUS_CONN_WRITE_CP_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONN_INIT_POLICY_FAILED = (ESL_LIB_STATUS_CONN_TAG_CONFIG_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_CONTROL_FAILED = (ESL_LIB_STATUS_CONN_INIT_POLICY_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_UNKNOWN_COMMAND = (ESL_LIB_STATUS_CONTROL_FAILED + 1)# esl_lib.h: 352

ESL_LIB_STATUS_SYSTEM_ERROR = (ESL_LIB_STATUS_UNKNOWN_COMMAND + 1)# esl_lib.h: 352

ESL_LIB_STATUS_DATABASE_ERROR = (ESL_LIB_STATUS_SYSTEM_ERROR + 1)# esl_lib.h: 352

ESL_LIB_STATUS_DATABASE_PERMISSION = (ESL_LIB_STATUS_DATABASE_ERROR + 1)# esl_lib.h: 352

esl_lib_status_t = enum_esl_lib_status_e# esl_lib.h: 352

enum_esl_lib_data_type_e = c_int# esl_lib.h: 380

ESL_LIB_DATA_TYPE_UNINITIALIZED = 0# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS = (ESL_LIB_DATA_TYPE_UNINITIALIZED + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY = (ESL_LIB_DATA_TYPE_GATT_ESL_ADDRESS + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY = (ESL_LIB_DATA_TYPE_GATT_AP_SYNC_KEY + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT = (ESL_LIB_DATA_TYPE_GATT_RESPONSE_KEY + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME = (ESL_LIB_DATA_TYPE_GATT_CONTROL_POINT + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_DISPLAY_INFO = (ESL_LIB_DATA_TYPE_GATT_CURRENT_TIME + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO = (ESL_LIB_DATA_TYPE_GATT_DISPLAY_INFO + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_SENSOR_INFO = (ESL_LIB_DATA_TYPE_GATT_IMAGE_INFO + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_LED_INFO = (ESL_LIB_DATA_TYPE_GATT_SENSOR_INFO + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_PNP_ID = (ESL_LIB_DATA_TYPE_GATT_LED_INFO + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER = (ESL_LIB_DATA_TYPE_GATT_PNP_ID + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_OTS_FEATURE = (ESL_LIB_DATA_TYPE_GATT_SERIAL_NUMBER + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_GATT_OTS_OBJECT_TYPE = (ESL_LIB_DATA_TYPE_GATT_OTS_FEATURE + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_PAWR_DATA = (ESL_LIB_DATA_TYPE_GATT_OTS_OBJECT_TYPE + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_PAWR_RESP_SLOT = (ESL_LIB_DATA_TYPE_PAWR_DATA + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_PAWR_SUBEVENT = (ESL_LIB_DATA_TYPE_PAWR_RESP_SLOT + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_SUBEVENT_START = (ESL_LIB_DATA_TYPE_PAWR_SUBEVENT + 1)# esl_lib.h: 380

ESL_LIB_DATA_TYPE_SUBEVENT_DATA_COUNT = (ESL_LIB_DATA_TYPE_SUBEVENT_START + 1)# esl_lib.h: 380

esl_lib_data_type_t = enum_esl_lib_data_type_e# esl_lib.h: 380

# esl_lib.h: 386
class struct_esl_lib_tlv_s(Structure):
    pass

struct_esl_lib_tlv_s.__slots__ = [
    'type',
    'data',
]
struct_esl_lib_tlv_s._fields_ = [
    ('type', esl_lib_data_type_t),
    ('data', esl_lib_long_array_t),
]

esl_lib_tlv_t = struct_esl_lib_tlv_s# esl_lib.h: 386

# esl_lib.h: 392
class struct_esl_lib_ots_features_s(Structure):
    pass

struct_esl_lib_ots_features_s.__slots__ = [
    'oacp_features',
    'olcp_features',
]
struct_esl_lib_ots_features_s._fields_ = [
    ('oacp_features', uint32_t),
    ('olcp_features', uint32_t),
]

esl_lib_ots_features_t = struct_esl_lib_ots_features_s# esl_lib.h: 392

enum_esl_lib_characteristic_index_e = c_int# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_ADDRESS = 0# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_AP_SYNC_KEY_MATERIAL = 1# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_RESPONSE_KEY_MATERIAL = 2# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_CURRENT_ABSOLUTE_TIME = 3# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_DISPLAY_INFORMATION = 4# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_IMAGE_INFORMATION = 5# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_SENSOR_INFORMATION = 6# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_LED_INFORMATION = 7# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_ESL_CONTROL_POINT = 8# esl_lib.h: 406

ESL_LIB_CHARACTERISTIC_INDEX_SIZE = 9# esl_lib.h: 406

esl_lib_characteristic_index_t = enum_esl_lib_characteristic_index_e# esl_lib.h: 406

enum_esl_lib_characteristic_index_dis_e = c_int# esl_lib.h: 412

ESL_LIB_CHARACTERISTIC_INDEX_DIS_PNP_ID = 0# esl_lib.h: 412

ESL_LIB_CHARACTERISTIC_INDEX_DIS_SN = 1# esl_lib.h: 412

ESL_LIB_CHARACTERISTIC_INDEX_DIS_SIZE = 2# esl_lib.h: 412

esl_lib_characteristic_index_dis_t = enum_esl_lib_characteristic_index_dis_e# esl_lib.h: 412

esl_lib_ots_gattdb_handles_t = uint16_t * int(12)# esl_lib.h: 415

# esl_lib.h: 419
class struct_esl_lib_gattdb_handles_services_s(Structure):
    pass

struct_esl_lib_gattdb_handles_services_s.__slots__ = [
    'esl',
    'ots',
    'dis',
]
struct_esl_lib_gattdb_handles_services_s._fields_ = [
    ('esl', uint32_t),
    ('ots', uint32_t),
    ('dis', uint32_t),
]

# esl_lib.h: 427
class struct_esl_lib_gattdb_handles_s(Structure):
    pass

struct_esl_lib_gattdb_handles_s.__slots__ = [
    'services',
    'esl_characteristics',
    'dis_characteristics',
    'ots_characteristics',
]
struct_esl_lib_gattdb_handles_s._fields_ = [
    ('services', struct_esl_lib_gattdb_handles_services_s),
    ('esl_characteristics', uint16_t * int(ESL_LIB_CHARACTERISTIC_INDEX_SIZE)),
    ('dis_characteristics', uint16_t * int(ESL_LIB_CHARACTERISTIC_INDEX_DIS_SIZE)),
    ('ots_characteristics', esl_lib_ots_gattdb_handles_t),
]

esl_lib_gattdb_handles_t = struct_esl_lib_gattdb_handles_s# esl_lib.h: 427

# esl_lib.h: 437
class struct_esl_lib_evt_system_boot_s(Structure):
    pass

struct_esl_lib_evt_system_boot_s.__slots__ = [
    'address',
    'radio_id',
    'status',
]
struct_esl_lib_evt_system_boot_s._fields_ = [
    ('address', esl_lib_address_t),
    ('radio_id', uint32_t),
    ('status', sl_status_t),
]

esl_lib_evt_system_boot_t = struct_esl_lib_evt_system_boot_s# esl_lib.h: 437

# esl_lib.h: 445
class struct_esl_lib_evt_connection_mode_s(Structure):
    pass

struct_esl_lib_evt_connection_mode_s.__slots__ = [
    'mode',
    'core_state',
    'filter_size',
    'connections',
]
struct_esl_lib_evt_connection_mode_s._fields_ = [
    ('mode', esl_lib_connection_mode_t),
    ('core_state', esl_lib_core_state_t),
    ('filter_size', c_size_t),
    ('connections', uint8_t),
]

esl_lib_evt_connection_mode_t = struct_esl_lib_evt_connection_mode_s# esl_lib.h: 445

# esl_lib.h: 452
class struct_esl_lib_evt_scan_status_s(Structure):
    pass

struct_esl_lib_evt_scan_status_s.__slots__ = [
    'enabled',
    'configured',
    'parameters',
]
struct_esl_lib_evt_scan_status_s._fields_ = [
    ('enabled', esl_lib_bool_t),
    ('configured', esl_lib_bool_t),
    ('parameters', esl_lib_scan_parameters_t),
]

esl_lib_evt_scan_status_t = struct_esl_lib_evt_scan_status_s# esl_lib.h: 452

# esl_lib.h: 458
class struct_esl_lib_evt_tag_found_s(Structure):
    pass

struct_esl_lib_evt_tag_found_s.__slots__ = [
    'address',
    'rssi',
]
struct_esl_lib_evt_tag_found_s._fields_ = [
    ('address', esl_lib_address_t),
    ('rssi', c_int8),
]

esl_lib_evt_tag_found_t = struct_esl_lib_evt_tag_found_s# esl_lib.h: 458

# esl_lib.h: 464
class struct_esl_lib_evt_tag_info_s(Structure):
    pass

struct_esl_lib_evt_tag_info_s.__slots__ = [
    'connection_handle',
    'tlv_data',
]
struct_esl_lib_evt_tag_info_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('tlv_data', esl_lib_long_array_t),
]

esl_lib_evt_tag_info_t = struct_esl_lib_evt_tag_info_s# esl_lib.h: 464

# esl_lib.h: 471
class struct_esl_lib_evt_tag_config_response_s(Structure):
    pass

struct_esl_lib_evt_tag_config_response_s.__slots__ = [
    'connection_handle',
    'type',
    'status',
]
struct_esl_lib_evt_tag_config_response_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('type', esl_lib_data_type_t),
    ('status', sl_status_t),
]

esl_lib_evt_configure_tag_response_t = struct_esl_lib_evt_tag_config_response_s# esl_lib.h: 471

# esl_lib.h: 478
class struct_esl_lib_evt_control_point_response_s(Structure):
    pass

struct_esl_lib_evt_control_point_response_s.__slots__ = [
    'connection_handle',
    'status',
    'data_sent',
]
struct_esl_lib_evt_control_point_response_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('status', sl_status_t),
    ('data_sent', esl_lib_array_t),
]

esl_lib_evt_control_point_response_t = struct_esl_lib_evt_control_point_response_s# esl_lib.h: 478

# esl_lib.h: 484
class struct_esl_lib_evt_control_point_notification_s(Structure):
    pass

struct_esl_lib_evt_control_point_notification_s.__slots__ = [
    'connection_handle',
    'data',
]
struct_esl_lib_evt_control_point_notification_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('data', esl_lib_array_t),
]

esl_lib_evt_control_point_notification_t = struct_esl_lib_evt_control_point_notification_s# esl_lib.h: 484

# esl_lib.h: 493
class struct_esl_lib_evt_connection_retry_s(Structure):
    pass

struct_esl_lib_evt_connection_retry_s.__slots__ = [
    'connection_handle',
    'reason',
    'connection_state',
    'address',
    'retries_left',
]
struct_esl_lib_evt_connection_retry_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('reason', sl_status_t),
    ('connection_state', esl_lib_connection_state_t),
    ('address', esl_lib_address_t),
    ('retries_left', uint8_t),
]

esl_lib_evt_connection_retry_t = struct_esl_lib_evt_connection_retry_s# esl_lib.h: 493

# esl_lib.h: 500
class struct_esl_lib_evt_connection_closed_s(Structure):
    pass

struct_esl_lib_evt_connection_closed_s.__slots__ = [
    'connection_handle',
    'address',
    'reason',
]
struct_esl_lib_evt_connection_closed_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('address', esl_lib_address_t),
    ('reason', sl_status_t),
]

esl_lib_evt_connection_closed_t = struct_esl_lib_evt_connection_closed_s# esl_lib.h: 500

# esl_lib.h: 508
class struct_esl_lib_evt_connection_opened_s(Structure):
    pass

struct_esl_lib_evt_connection_opened_s.__slots__ = [
    'connection_handle',
    'address',
    'gattdb_handles',
    'status',
]
struct_esl_lib_evt_connection_opened_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('address', esl_lib_address_t),
    ('gattdb_handles', esl_lib_gattdb_handles_t),
    ('status', sl_status_t),
]

esl_lib_evt_connection_opened_t = struct_esl_lib_evt_connection_opened_s# esl_lib.h: 508

# esl_lib.h: 515
class struct_esl_lib_evt_bonding_data_s(Structure):
    pass

struct_esl_lib_evt_bonding_data_s.__slots__ = [
    'connection_handle',
    'address',
    'ltk',
]
struct_esl_lib_evt_bonding_data_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('address', esl_lib_address_t),
    ('ltk', uint8_t * int(16)),
]

esl_lib_evt_bonding_data_t = struct_esl_lib_evt_bonding_data_s# esl_lib.h: 515

# esl_lib.h: 521
class struct_esl_lib_evt_bonding_finished_s(Structure):
    pass

struct_esl_lib_evt_bonding_finished_s.__slots__ = [
    'connection_handle',
    'address',
]
struct_esl_lib_evt_bonding_finished_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('address', esl_lib_address_t),
]

esl_lib_evt_bonding_finished_t = struct_esl_lib_evt_bonding_finished_s# esl_lib.h: 521

# esl_lib.h: 529
class struct_esl_lib_evt_pawr_status_s(Structure):
    pass

struct_esl_lib_evt_pawr_status_s.__slots__ = [
    'pawr_handle',
    'status',
    'configured',
    'config',
]
struct_esl_lib_evt_pawr_status_s._fields_ = [
    ('pawr_handle', esl_lib_pawr_handle_t),
    ('status', esl_lib_pawr_state_t),
    ('configured', esl_lib_bool_t),
    ('config', esl_lib_pawr_config_t),
]

esl_lib_evt_pawr_status_t = struct_esl_lib_evt_pawr_status_s# esl_lib.h: 529

# esl_lib.h: 536
class struct_esl_lib_evt_image_transfer_finished_s(Structure):
    pass

struct_esl_lib_evt_image_transfer_finished_s.__slots__ = [
    'connection_handle',
    'img_index',
    'status',
]
struct_esl_lib_evt_image_transfer_finished_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('img_index', uint8_t),
    ('status', sl_status_t),
]

esl_lib_evt_image_transfer_finished_t = struct_esl_lib_evt_image_transfer_finished_s# esl_lib.h: 536

# esl_lib.h: 543
class struct_esl_lib_evt_image_type_s(Structure):
    pass

struct_esl_lib_evt_image_type_s.__slots__ = [
    'connection_handle',
    'img_index',
    'type_data',
]
struct_esl_lib_evt_image_type_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('img_index', uint8_t),
    ('type_data', esl_lib_long_array_t),
]

esl_lib_evt_image_type_t = struct_esl_lib_evt_image_type_s# esl_lib.h: 543

# esl_lib.h: 551
class struct_esl_lib_evt_pawr_response_s(Structure):
    pass

struct_esl_lib_evt_pawr_response_s.__slots__ = [
    'pawr_handle',
    'subevent',
    'response_slot',
    'data',
]
struct_esl_lib_evt_pawr_response_s._fields_ = [
    ('pawr_handle', esl_lib_pawr_handle_t),
    ('subevent', uint8_t),
    ('response_slot', uint8_t),
    ('data', esl_lib_long_array_t),
]

esl_lib_evt_pawr_response_t = struct_esl_lib_evt_pawr_response_s# esl_lib.h: 551

# esl_lib.h: 558
class struct_esl_lib_evt_pawr_data_request_s(Structure):
    pass

struct_esl_lib_evt_pawr_data_request_s.__slots__ = [
    'pawr_handle',
    'subevent_start',
    'subevent_data_count',
]
struct_esl_lib_evt_pawr_data_request_s._fields_ = [
    ('pawr_handle', esl_lib_pawr_handle_t),
    ('subevent_start', uint8_t),
    ('subevent_data_count', uint8_t),
]

esl_lib_evt_pawr_data_request_t = struct_esl_lib_evt_pawr_data_request_s# esl_lib.h: 558

# esl_lib.h: 566
class struct_esl_lib_evt_ots_return_type_s(Structure):
    pass

struct_esl_lib_evt_ots_return_type_s.__slots__ = [
    'connection_handle',
    'img_index',
    'object_type',
    'gatt_status',
]
struct_esl_lib_evt_ots_return_type_s._fields_ = [
    ('connection_handle', esl_lib_connection_handle_t),
    ('img_index', uint8_t),
    ('object_type', uint8_t),
    ('gatt_status', uint16_t),
]

esl_lib_evt_ots_return_type_t = struct_esl_lib_evt_ots_return_type_s# esl_lib.h: 566

# esl_lib.h: 572
class union_esl_lib_status_data_u(Union):
    pass

union_esl_lib_status_data_u.__slots__ = [
    'core_state',
    'connection_state',
    'pawr_state',
]
union_esl_lib_status_data_u._fields_ = [
    ('core_state', esl_lib_core_state_t),
    ('connection_state', esl_lib_connection_state_t),
    ('pawr_state', esl_lib_pawr_state_t),
]

esl_lib_status_data_t = union_esl_lib_status_data_u# esl_lib.h: 572

# esl_lib.h: 580
class struct_esl_lib_evt_error_s(Structure):
    pass

struct_esl_lib_evt_error_s.__slots__ = [
    'node_id',
    'lib_status',
    'sl_status',
    'data',
]
struct_esl_lib_evt_error_s._fields_ = [
    ('node_id', esl_lib_node_id_t),
    ('lib_status', esl_lib_status_t),
    ('sl_status', sl_status_t),
    ('data', esl_lib_status_data_t),
]

esl_lib_evt_error_t = struct_esl_lib_evt_error_s# esl_lib.h: 580

# esl_lib.h: 604
class union_esl_lib_evt_data_u(Union):
    pass

union_esl_lib_evt_data_u.__slots__ = [
    'evt_boot',
    'evt_connection_mode',
    'evt_scan_status',
    'evt_tag_found',
    'evt_tag_info',
    'evt_configure_tag_response',
    'evt_control_point_response',
    'evt_control_point_notification',
    'evt_connection_retry',
    'evt_connection_closed',
    'evt_connection_opened',
    'evt_bonding_data',
    'evt_bonding_finished',
    'evt_image_transfer_finished',
    'evt_image_type',
    'evt_pawr_status',
    'evt_pawr_response',
    'evt_pawr_data_request',
    'evt_error',
    'evt_general',
]
union_esl_lib_evt_data_u._fields_ = [
    ('evt_boot', esl_lib_evt_system_boot_t),
    ('evt_connection_mode', esl_lib_evt_connection_mode_t),
    ('evt_scan_status', esl_lib_evt_scan_status_t),
    ('evt_tag_found', esl_lib_evt_tag_found_t),
    ('evt_tag_info', esl_lib_evt_tag_info_t),
    ('evt_configure_tag_response', esl_lib_evt_configure_tag_response_t),
    ('evt_control_point_response', esl_lib_evt_control_point_response_t),
    ('evt_control_point_notification', esl_lib_evt_control_point_notification_t),
    ('evt_connection_retry', esl_lib_evt_connection_retry_t),
    ('evt_connection_closed', esl_lib_evt_connection_closed_t),
    ('evt_connection_opened', esl_lib_evt_connection_opened_t),
    ('evt_bonding_data', esl_lib_evt_bonding_data_t),
    ('evt_bonding_finished', esl_lib_evt_bonding_finished_t),
    ('evt_image_transfer_finished', esl_lib_evt_image_transfer_finished_t),
    ('evt_image_type', esl_lib_evt_image_type_t),
    ('evt_pawr_status', esl_lib_evt_pawr_status_t),
    ('evt_pawr_response', esl_lib_evt_pawr_response_t),
    ('evt_pawr_data_request', esl_lib_evt_pawr_data_request_t),
    ('evt_error', esl_lib_evt_error_t),
    ('evt_general', esl_lib_long_array_t),
]

esl_lib_evt_data_t = union_esl_lib_evt_data_u# esl_lib.h: 604

enum_esl_lib_key_type_e = c_int# esl_lib.h: 612

ESL_LIB_KEY_TYPE_NO_KEY = 0# esl_lib.h: 612

ESL_LIB_KEY_TYPE_LTK = (ESL_LIB_KEY_TYPE_NO_KEY + 1)# esl_lib.h: 612

ESL_LIB_KEY_TYPE_PASSKEY = (ESL_LIB_KEY_TYPE_LTK + 1)# esl_lib.h: 612

ESL_LIB_KEY_TYPE_OOB_DATA = (ESL_LIB_KEY_TYPE_PASSKEY + 1)# esl_lib.h: 612

esl_lib_key_type_t = enum_esl_lib_key_type_e# esl_lib.h: 612

esl_lib_on_event_t = CFUNCTYPE(UNCHECKED(None), esl_lib_evt_type_t, POINTER(esl_lib_evt_data_t))# esl_lib.h: 622

enum_esl_lib_log_level_e = c_int# esl_lib.h: 635

ESL_LIB_LOG_LEVEL_DEBUG = 0# esl_lib.h: 635

ESL_LIB_LOG_LEVEL_INFO = (ESL_LIB_LOG_LEVEL_DEBUG + 1)# esl_lib.h: 635

ESL_LIB_LOG_LEVEL_WARNING = (ESL_LIB_LOG_LEVEL_INFO + 1)# esl_lib.h: 635

ESL_LIB_LOG_LEVEL_ERROR = (ESL_LIB_LOG_LEVEL_WARNING + 1)# esl_lib.h: 635

ESL_LIB_LOG_LEVEL_CRITICAL = (ESL_LIB_LOG_LEVEL_ERROR + 1)# esl_lib.h: 635

esl_lib_log_level_t = enum_esl_lib_log_level_e# esl_lib.h: 635

esl_lib_log_callback_t = CFUNCTYPE(UNCHECKED(None), esl_lib_log_level_t, String, String, String, c_int, String)# esl_lib.h: 646

# esl_lib.h: 670
for _lib in _libs.values():
    if not _lib.has("esl_lib_start", "cdecl"):
        continue
    esl_lib_start = _lib.get("esl_lib_start", "cdecl")
    esl_lib_start.argtypes = [String, esl_lib_on_event_t, esl_lib_log_callback_t]
    esl_lib_start.restype = sl_status_t
    break

# esl_lib.h: 679
for _lib in _libs.values():
    if not _lib.has("esl_lib_stop", "cdecl"):
        continue
    esl_lib_stop = _lib.get("esl_lib_stop", "cdecl")
    esl_lib_stop.argtypes = []
    esl_lib_stop.restype = sl_status_t
    break

# esl_lib.h: 700
for _lib in _libs.values():
    if not _lib.has("esl_lib_connect", "cdecl"):
        continue
    esl_lib_connect = _lib.get("esl_lib_connect", "cdecl")
    esl_lib_connect.argtypes = [esl_lib_address_t, POINTER(esl_lib_pawr_subevent_t), POINTER(esl_lib_address_t), esl_lib_key_type_t, POINTER(esl_lib_array_t), POINTER(esl_lib_gattdb_handles_t)]
    esl_lib_connect.restype = sl_status_t
    break

# esl_lib.h: 713
for _lib in _libs.values():
    if not _lib.has("esl_lib_close_connection", "cdecl"):
        continue
    esl_lib_close_connection = _lib.get("esl_lib_close_connection", "cdecl")
    esl_lib_close_connection.argtypes = [esl_lib_connection_handle_t]
    esl_lib_close_connection.restype = sl_status_t
    break

# esl_lib.h: 722
for _lib in _libs.values():
    if not _lib.has("esl_lib_set_connection_mode", "cdecl"):
        continue
    esl_lib_set_connection_mode = _lib.get("esl_lib_set_connection_mode", "cdecl")
    esl_lib_set_connection_mode.argtypes = [esl_lib_connection_mode_t]
    esl_lib_set_connection_mode.restype = sl_status_t
    break

# esl_lib.h: 729
for _lib in _libs.values():
    if not _lib.has("esl_lib_get_connection_mode", "cdecl"):
        continue
    esl_lib_get_connection_mode = _lib.get("esl_lib_get_connection_mode", "cdecl")
    esl_lib_get_connection_mode.argtypes = []
    esl_lib_get_connection_mode.restype = sl_status_t
    break

# esl_lib.h: 740
for _lib in _libs.values():
    if not _lib.has("esl_lib_get_tag_info", "cdecl"):
        continue
    esl_lib_get_tag_info = _lib.get("esl_lib_get_tag_info", "cdecl")
    esl_lib_get_tag_info.argtypes = [esl_lib_connection_handle_t]
    esl_lib_get_tag_info.restype = sl_status_t
    break

# esl_lib.h: 749
for _lib in _libs.values():
    if not _lib.has("esl_lib_configure_tag", "cdecl"):
        continue
    esl_lib_configure_tag = _lib.get("esl_lib_configure_tag", "cdecl")
    esl_lib_configure_tag.argtypes = [esl_lib_connection_handle_t, POINTER(esl_lib_long_array_t)]
    esl_lib_configure_tag.restype = sl_status_t
    break

# esl_lib.h: 760
for _lib in _libs.values():
    if not _lib.has("esl_lib_write_control_point", "cdecl"):
        continue
    esl_lib_write_control_point = _lib.get("esl_lib_write_control_point", "cdecl")
    esl_lib_write_control_point.argtypes = [esl_lib_connection_handle_t, POINTER(esl_lib_array_t), esl_lib_bool_t]
    esl_lib_write_control_point.restype = sl_status_t
    break

# esl_lib.h: 773
for _lib in _libs.values():
    if not _lib.has("esl_lib_pawr_create", "cdecl"):
        continue
    esl_lib_pawr_create = _lib.get("esl_lib_pawr_create", "cdecl")
    esl_lib_pawr_create.argtypes = [POINTER(esl_lib_pawr_handle_t)]
    esl_lib_pawr_create.restype = sl_status_t
    break

# esl_lib.h: 782
for _lib in _libs.values():
    if not _lib.has("esl_lib_pawr_remove", "cdecl"):
        continue
    esl_lib_pawr_remove = _lib.get("esl_lib_pawr_remove", "cdecl")
    esl_lib_pawr_remove.argtypes = [esl_lib_pawr_handle_t]
    esl_lib_pawr_remove.restype = sl_status_t
    break

# esl_lib.h: 793
for _lib in _libs.values():
    if not _lib.has("esl_lib_pawr_enable", "cdecl"):
        continue
    esl_lib_pawr_enable = _lib.get("esl_lib_pawr_enable", "cdecl")
    esl_lib_pawr_enable.argtypes = [esl_lib_pawr_handle_t, esl_lib_bool_t, esl_lib_bool_t]
    esl_lib_pawr_enable.restype = sl_status_t
    break

# esl_lib.h: 805
for _lib in _libs.values():
    if not _lib.has("esl_lib_pawr_set_data", "cdecl"):
        continue
    esl_lib_pawr_set_data = _lib.get("esl_lib_pawr_set_data", "cdecl")
    esl_lib_pawr_set_data.argtypes = [esl_lib_pawr_handle_t, uint8_t, uint8_t, POINTER(esl_lib_array_t)]
    esl_lib_pawr_set_data.restype = sl_status_t
    break

# esl_lib.h: 819
for _lib in _libs.values():
    if not _lib.has("esl_lib_pawr_configure", "cdecl"):
        continue
    esl_lib_pawr_configure = _lib.get("esl_lib_pawr_configure", "cdecl")
    esl_lib_pawr_configure.argtypes = [esl_lib_pawr_handle_t, POINTER(esl_lib_pawr_config_t)]
    esl_lib_pawr_configure.restype = sl_status_t
    break

# esl_lib.h: 828
for _lib in _libs.values():
    if not _lib.has("esl_lib_get_pawr_status", "cdecl"):
        continue
    esl_lib_get_pawr_status = _lib.get("esl_lib_get_pawr_status", "cdecl")
    esl_lib_get_pawr_status.argtypes = [esl_lib_pawr_handle_t]
    esl_lib_get_pawr_status.restype = sl_status_t
    break

# esl_lib.h: 837
for _lib in _libs.values():
    if not _lib.has("esl_lib_initiate_past", "cdecl"):
        continue
    esl_lib_initiate_past = _lib.get("esl_lib_initiate_past", "cdecl")
    esl_lib_initiate_past.argtypes = [esl_lib_connection_handle_t, esl_lib_pawr_handle_t]
    esl_lib_initiate_past.restype = sl_status_t
    break

# esl_lib.h: 852
for _lib in _libs.values():
    if not _lib.has("esl_lib_write_image", "cdecl"):
        continue
    esl_lib_write_image = _lib.get("esl_lib_write_image", "cdecl")
    esl_lib_write_image.argtypes = [esl_lib_connection_handle_t, uint8_t, c_size_t, POINTER(None)]
    esl_lib_write_image.restype = sl_status_t
    break

# esl_lib.h: 864
for _lib in _libs.values():
    if not _lib.has("esl_lib_get_image_type", "cdecl"):
        continue
    esl_lib_get_image_type = _lib.get("esl_lib_get_image_type", "cdecl")
    esl_lib_get_image_type.argtypes = [esl_lib_connection_handle_t, uint8_t]
    esl_lib_get_image_type.restype = sl_status_t
    break

# esl_lib.h: 876
for _lib in _libs.values():
    if not _lib.has("esl_lib_scan_configure", "cdecl"):
        continue
    esl_lib_scan_configure = _lib.get("esl_lib_scan_configure", "cdecl")
    esl_lib_scan_configure.argtypes = [POINTER(esl_lib_scan_parameters_t)]
    esl_lib_scan_configure.restype = sl_status_t
    break

# esl_lib.h: 884
for _lib in _libs.values():
    if not _lib.has("esl_lib_scan_enable", "cdecl"):
        continue
    esl_lib_scan_enable = _lib.get("esl_lib_scan_enable", "cdecl")
    esl_lib_scan_enable.argtypes = [esl_lib_bool_t]
    esl_lib_scan_enable.restype = sl_status_t
    break

# esl_lib.h: 891
for _lib in _libs.values():
    if not _lib.has("esl_lib_get_scan_status", "cdecl"):
        continue
    esl_lib_get_scan_status = _lib.get("esl_lib_get_scan_status", "cdecl")
    esl_lib_get_scan_status.argtypes = []
    esl_lib_get_scan_status.restype = sl_status_t
    break

# esl_lib.h: 902
for _lib in _libs.values():
    if not _lib.has("esl_lib_general_cmd", "cdecl"):
        continue
    esl_lib_general_cmd = _lib.get("esl_lib_general_cmd", "cdecl")
    esl_lib_general_cmd.argtypes = [uint8_t, POINTER(esl_lib_long_array_t)]
    esl_lib_general_cmd.restype = sl_status_t
    break

# stdint.h: 128
try:
    UINT16_MAX = 65535
except:
    pass

# stdint.h: 129
try:
    UINT32_MAX = 4294967295
except:
    pass

# sl_status.h: 48
try:
    SL_STATUS_SPACE_MASK = (sl_status_t (ord_if_char(0xFF00))).value
except:
    pass

# sl_status.h: 50
try:
    SL_STATUS_GENERIC_SPACE = (sl_status_t (ord_if_char(0x0000))).value
except:
    pass

# sl_status.h: 52
try:
    SL_STATUS_PLATFORM_1_SPACE = (sl_status_t (ord_if_char(0x0100))).value
except:
    pass

# sl_status.h: 53
try:
    SL_STATUS_PLATFORM_2_SPACE = (sl_status_t (ord_if_char(0x0200))).value
except:
    pass

# sl_status.h: 54
try:
    SL_STATUS_HARDWARE_SPACE = (sl_status_t (ord_if_char(0x0300))).value
except:
    pass

# sl_status.h: 56
try:
    SL_STATUS_BLUETOOTH_SPACE = (sl_status_t (ord_if_char(0x0400))).value
except:
    pass

# sl_status.h: 57
try:
    SL_STATUS_BLUETOOTH_MESH_SPACE = (sl_status_t (ord_if_char(0x0500))).value
except:
    pass

# sl_status.h: 58
try:
    SL_STATUS_CAN_CANOPEN_SPACE = (sl_status_t (ord_if_char(0x0600))).value
except:
    pass

# sl_status.h: 59
try:
    SL_STATUS_CONNECT_SPACE = (sl_status_t (ord_if_char(0x0700))).value
except:
    pass

# sl_status.h: 60
try:
    SL_STATUS_NET_SUITE_SPACE = (sl_status_t (ord_if_char(0x0800))).value
except:
    pass

# sl_status.h: 61
try:
    SL_STATUS_THREAD_SPACE = (sl_status_t (ord_if_char(0x0900))).value
except:
    pass

# sl_status.h: 62
try:
    SL_STATUS_USB_SPACE = (sl_status_t (ord_if_char(0x0A00))).value
except:
    pass

# sl_status.h: 63
try:
    SL_STATUS_WIFI_SPACE = (sl_status_t (ord_if_char(0x0B00))).value
except:
    pass

# sl_status.h: 64
try:
    SL_STATUS_ZIGBEE_SPACE = (sl_status_t (ord_if_char(0x0C00))).value
except:
    pass

# sl_status.h: 65
try:
    SL_STATUS_Z_WAVE_SPACE = (sl_status_t (ord_if_char(0x0D00))).value
except:
    pass

# sl_status.h: 67
try:
    SL_STATUS_GECKO_OS_1_SPACE = (sl_status_t (ord_if_char(0x0E00))).value
except:
    pass

# sl_status.h: 68
try:
    SL_STATUS_GECKO_OS_2_SPACE = (sl_status_t (ord_if_char(0x0F00))).value
except:
    pass

# sl_status.h: 70
try:
    SL_STATUS_BLUETOOTH_CTRL_SPACE = (sl_status_t (ord_if_char(0x1000))).value
except:
    pass

# sl_status.h: 71
try:
    SL_STATUS_BLUETOOTH_ATT_SPACE = (sl_status_t (ord_if_char(0x1100))).value
except:
    pass

# sl_status.h: 72
try:
    SL_STATUS_BLUETOOTH_SMP_SPACE = (sl_status_t (ord_if_char(0x1200))).value
except:
    pass

# sl_status.h: 73
try:
    SL_STATUS_BLUETOOTH_MESH_FOUNDATION_SPACE = (sl_status_t (ord_if_char(0x1300))).value
except:
    pass

# sl_status.h: 75
try:
    SL_STATUS_WISUN_SPACE = (sl_status_t (ord_if_char(0x1400))).value
except:
    pass

# sl_status.h: 77
try:
    SL_STATUS_COMPUTE_SPACE = (sl_status_t (ord_if_char(0x1500))).value
except:
    pass

# sl_status.h: 85
try:
    SL_STATUS_OK = (sl_status_t (ord_if_char(0x0000))).value
except:
    pass

# sl_status.h: 86
try:
    SL_STATUS_FAIL = (sl_status_t (ord_if_char(0x0001))).value
except:
    pass

# sl_status.h: 89
try:
    SL_STATUS_INVALID_STATE = (sl_status_t (ord_if_char(0x0002))).value
except:
    pass

# sl_status.h: 90
try:
    SL_STATUS_NOT_READY = (sl_status_t (ord_if_char(0x0003))).value
except:
    pass

# sl_status.h: 91
try:
    SL_STATUS_BUSY = (sl_status_t (ord_if_char(0x0004))).value
except:
    pass

# sl_status.h: 92
try:
    SL_STATUS_IN_PROGRESS = (sl_status_t (ord_if_char(0x0005))).value
except:
    pass

# sl_status.h: 93
try:
    SL_STATUS_ABORT = (sl_status_t (ord_if_char(0x0006))).value
except:
    pass

# sl_status.h: 94
try:
    SL_STATUS_TIMEOUT = (sl_status_t (ord_if_char(0x0007))).value
except:
    pass

# sl_status.h: 95
try:
    SL_STATUS_PERMISSION = (sl_status_t (ord_if_char(0x0008))).value
except:
    pass

# sl_status.h: 96
try:
    SL_STATUS_WOULD_BLOCK = (sl_status_t (ord_if_char(0x0009))).value
except:
    pass

# sl_status.h: 97
try:
    SL_STATUS_IDLE = (sl_status_t (ord_if_char(0x000A))).value
except:
    pass

# sl_status.h: 98
try:
    SL_STATUS_IS_WAITING = (sl_status_t (ord_if_char(0x000B))).value
except:
    pass

# sl_status.h: 99
try:
    SL_STATUS_NONE_WAITING = (sl_status_t (ord_if_char(0x000C))).value
except:
    pass

# sl_status.h: 100
try:
    SL_STATUS_SUSPENDED = (sl_status_t (ord_if_char(0x000D))).value
except:
    pass

# sl_status.h: 101
try:
    SL_STATUS_NOT_AVAILABLE = (sl_status_t (ord_if_char(0x000E))).value
except:
    pass

# sl_status.h: 102
try:
    SL_STATUS_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x000F))).value
except:
    pass

# sl_status.h: 103
try:
    SL_STATUS_INITIALIZATION = (sl_status_t (ord_if_char(0x0010))).value
except:
    pass

# sl_status.h: 104
try:
    SL_STATUS_NOT_INITIALIZED = (sl_status_t (ord_if_char(0x0011))).value
except:
    pass

# sl_status.h: 105
try:
    SL_STATUS_ALREADY_INITIALIZED = (sl_status_t (ord_if_char(0x0012))).value
except:
    pass

# sl_status.h: 106
try:
    SL_STATUS_DELETED = (sl_status_t (ord_if_char(0x0013))).value
except:
    pass

# sl_status.h: 107
try:
    SL_STATUS_ISR = (sl_status_t (ord_if_char(0x0014))).value
except:
    pass

# sl_status.h: 108
try:
    SL_STATUS_NETWORK_UP = (sl_status_t (ord_if_char(0x0015))).value
except:
    pass

# sl_status.h: 109
try:
    SL_STATUS_NETWORK_DOWN = (sl_status_t (ord_if_char(0x0016))).value
except:
    pass

# sl_status.h: 110
try:
    SL_STATUS_NOT_JOINED = (sl_status_t (ord_if_char(0x0017))).value
except:
    pass

# sl_status.h: 111
try:
    SL_STATUS_NO_BEACONS = (sl_status_t (ord_if_char(0x0018))).value
except:
    pass

# sl_status.h: 114
try:
    SL_STATUS_ALLOCATION_FAILED = (sl_status_t (ord_if_char(0x0019))).value
except:
    pass

# sl_status.h: 115
try:
    SL_STATUS_NO_MORE_RESOURCE = (sl_status_t (ord_if_char(0x001A))).value
except:
    pass

# sl_status.h: 116
try:
    SL_STATUS_EMPTY = (sl_status_t (ord_if_char(0x001B))).value
except:
    pass

# sl_status.h: 117
try:
    SL_STATUS_FULL = (sl_status_t (ord_if_char(0x001C))).value
except:
    pass

# sl_status.h: 118
try:
    SL_STATUS_WOULD_OVERFLOW = (sl_status_t (ord_if_char(0x001D))).value
except:
    pass

# sl_status.h: 119
try:
    SL_STATUS_HAS_OVERFLOWED = (sl_status_t (ord_if_char(0x001E))).value
except:
    pass

# sl_status.h: 120
try:
    SL_STATUS_OWNERSHIP = (sl_status_t (ord_if_char(0x001F))).value
except:
    pass

# sl_status.h: 121
try:
    SL_STATUS_IS_OWNER = (sl_status_t (ord_if_char(0x0020))).value
except:
    pass

# sl_status.h: 124
try:
    SL_STATUS_INVALID_PARAMETER = (sl_status_t (ord_if_char(0x0021))).value
except:
    pass

# sl_status.h: 125
try:
    SL_STATUS_NULL_POINTER = (sl_status_t (ord_if_char(0x0022))).value
except:
    pass

# sl_status.h: 126
try:
    SL_STATUS_INVALID_CONFIGURATION = (sl_status_t (ord_if_char(0x0023))).value
except:
    pass

# sl_status.h: 127
try:
    SL_STATUS_INVALID_MODE = (sl_status_t (ord_if_char(0x0024))).value
except:
    pass

# sl_status.h: 128
try:
    SL_STATUS_INVALID_HANDLE = (sl_status_t (ord_if_char(0x0025))).value
except:
    pass

# sl_status.h: 129
try:
    SL_STATUS_INVALID_TYPE = (sl_status_t (ord_if_char(0x0026))).value
except:
    pass

# sl_status.h: 130
try:
    SL_STATUS_INVALID_INDEX = (sl_status_t (ord_if_char(0x0027))).value
except:
    pass

# sl_status.h: 131
try:
    SL_STATUS_INVALID_RANGE = (sl_status_t (ord_if_char(0x0028))).value
except:
    pass

# sl_status.h: 132
try:
    SL_STATUS_INVALID_KEY = (sl_status_t (ord_if_char(0x0029))).value
except:
    pass

# sl_status.h: 133
try:
    SL_STATUS_INVALID_CREDENTIALS = (sl_status_t (ord_if_char(0x002A))).value
except:
    pass

# sl_status.h: 134
try:
    SL_STATUS_INVALID_COUNT = (sl_status_t (ord_if_char(0x002B))).value
except:
    pass

# sl_status.h: 135
try:
    SL_STATUS_INVALID_SIGNATURE = (sl_status_t (ord_if_char(0x002C))).value
except:
    pass

# sl_status.h: 136
try:
    SL_STATUS_NOT_FOUND = (sl_status_t (ord_if_char(0x002D))).value
except:
    pass

# sl_status.h: 137
try:
    SL_STATUS_ALREADY_EXISTS = (sl_status_t (ord_if_char(0x002E))).value
except:
    pass

# sl_status.h: 140
try:
    SL_STATUS_IO = (sl_status_t (ord_if_char(0x002F))).value
except:
    pass

# sl_status.h: 141
try:
    SL_STATUS_IO_TIMEOUT = (sl_status_t (ord_if_char(0x0030))).value
except:
    pass

# sl_status.h: 142
try:
    SL_STATUS_TRANSMIT = (sl_status_t (ord_if_char(0x0031))).value
except:
    pass

# sl_status.h: 143
try:
    SL_STATUS_TRANSMIT_UNDERFLOW = (sl_status_t (ord_if_char(0x0032))).value
except:
    pass

# sl_status.h: 144
try:
    SL_STATUS_TRANSMIT_INCOMPLETE = (sl_status_t (ord_if_char(0x0033))).value
except:
    pass

# sl_status.h: 145
try:
    SL_STATUS_TRANSMIT_BUSY = (sl_status_t (ord_if_char(0x0034))).value
except:
    pass

# sl_status.h: 146
try:
    SL_STATUS_RECEIVE = (sl_status_t (ord_if_char(0x0035))).value
except:
    pass

# sl_status.h: 147
try:
    SL_STATUS_OBJECT_READ = (sl_status_t (ord_if_char(0x0036))).value
except:
    pass

# sl_status.h: 148
try:
    SL_STATUS_OBJECT_WRITE = (sl_status_t (ord_if_char(0x0037))).value
except:
    pass

# sl_status.h: 149
try:
    SL_STATUS_MESSAGE_TOO_LONG = (sl_status_t (ord_if_char(0x0038))).value
except:
    pass

# sl_status.h: 152
try:
    SL_STATUS_EEPROM_MFG_VERSION_MISMATCH = (sl_status_t (ord_if_char(0x0039))).value
except:
    pass

# sl_status.h: 153
try:
    SL_STATUS_EEPROM_STACK_VERSION_MISMATCH = (sl_status_t (ord_if_char(0x003A))).value
except:
    pass

# sl_status.h: 154
try:
    SL_STATUS_FLASH_WRITE_INHIBITED = (sl_status_t (ord_if_char(0x003B))).value
except:
    pass

# sl_status.h: 155
try:
    SL_STATUS_FLASH_VERIFY_FAILED = (sl_status_t (ord_if_char(0x003C))).value
except:
    pass

# sl_status.h: 156
try:
    SL_STATUS_FLASH_PROGRAM_FAILED = (sl_status_t (ord_if_char(0x003D))).value
except:
    pass

# sl_status.h: 157
try:
    SL_STATUS_FLASH_ERASE_FAILED = (sl_status_t (ord_if_char(0x003E))).value
except:
    pass

# sl_status.h: 160
try:
    SL_STATUS_MAC_NO_DATA = (sl_status_t (ord_if_char(0x003F))).value
except:
    pass

# sl_status.h: 161
try:
    SL_STATUS_MAC_NO_ACK_RECEIVED = (sl_status_t (ord_if_char(0x0040))).value
except:
    pass

# sl_status.h: 162
try:
    SL_STATUS_MAC_INDIRECT_TIMEOUT = (sl_status_t (ord_if_char(0x0041))).value
except:
    pass

# sl_status.h: 163
try:
    SL_STATUS_MAC_UNKNOWN_HEADER_TYPE = (sl_status_t (ord_if_char(0x0042))).value
except:
    pass

# sl_status.h: 164
try:
    SL_STATUS_MAC_ACK_HEADER_TYPE = (sl_status_t (ord_if_char(0x0043))).value
except:
    pass

# sl_status.h: 165
try:
    SL_STATUS_MAC_COMMAND_TRANSMIT_FAILURE = (sl_status_t (ord_if_char(0x0044))).value
except:
    pass

# sl_status.h: 168
try:
    SL_STATUS_CLI_STORAGE_NVM_OPEN_ERROR = (sl_status_t (ord_if_char(0x0045))).value
except:
    pass

# sl_status.h: 171
try:
    SL_STATUS_SECURITY_IMAGE_CHECKSUM_ERROR = (sl_status_t (ord_if_char(0x0046))).value
except:
    pass

# sl_status.h: 172
try:
    SL_STATUS_SECURITY_DECRYPT_ERROR = (sl_status_t (ord_if_char(0x0047))).value
except:
    pass

# sl_status.h: 175
try:
    SL_STATUS_COMMAND_IS_INVALID = (sl_status_t (ord_if_char(0x0048))).value
except:
    pass

# sl_status.h: 176
try:
    SL_STATUS_COMMAND_TOO_LONG = (sl_status_t (ord_if_char(0x0049))).value
except:
    pass

# sl_status.h: 177
try:
    SL_STATUS_COMMAND_INCOMPLETE = (sl_status_t (ord_if_char(0x004A))).value
except:
    pass

# sl_status.h: 180
try:
    SL_STATUS_BUS_ERROR = (sl_status_t (ord_if_char(0x004B))).value
except:
    pass

# sl_status.h: 183
try:
    SL_STATUS_CCA_FAILURE = (sl_status_t (ord_if_char(0x004C))).value
except:
    pass

# sl_status.h: 186
try:
    SL_STATUS_MAC_SCANNING = (sl_status_t (ord_if_char(0x004D))).value
except:
    pass

# sl_status.h: 187
try:
    SL_STATUS_MAC_INCORRECT_SCAN_TYPE = (sl_status_t (ord_if_char(0x004E))).value
except:
    pass

# sl_status.h: 188
try:
    SL_STATUS_INVALID_CHANNEL_MASK = (sl_status_t (ord_if_char(0x004F))).value
except:
    pass

# sl_status.h: 189
try:
    SL_STATUS_BAD_SCAN_DURATION = (sl_status_t (ord_if_char(0x0050))).value
except:
    pass

# sl_status.h: 192
try:
    SL_STATUS_MAC_TRANSMIT_QUEUE_FULL = (sl_status_t (ord_if_char(0x0053))).value
except:
    pass

# sl_status.h: 193
try:
    SL_STATUS_TRANSMIT_SCHEDULER_FAIL = (sl_status_t (ord_if_char(0x0054))).value
except:
    pass

# sl_status.h: 194
try:
    SL_STATUS_TRANSMIT_INVALID_CHANNEL = (sl_status_t (ord_if_char(0x0055))).value
except:
    pass

# sl_status.h: 195
try:
    SL_STATUS_TRANSMIT_INVALID_POWER = (sl_status_t (ord_if_char(0x0056))).value
except:
    pass

# sl_status.h: 196
try:
    SL_STATUS_TRANSMIT_ACK_RECEIVED = (sl_status_t (ord_if_char(0x0057))).value
except:
    pass

# sl_status.h: 197
try:
    SL_STATUS_TRANSMIT_BLOCKED = (sl_status_t (ord_if_char(0x0058))).value
except:
    pass

# sl_status.h: 200
try:
    SL_STATUS_NVM3_ALIGNMENT_INVALID = (sl_status_t (ord_if_char(0x0059))).value
except:
    pass

# sl_status.h: 201
try:
    SL_STATUS_NVM3_SIZE_TOO_SMALL = (sl_status_t (ord_if_char(0x005A))).value
except:
    pass

# sl_status.h: 202
try:
    SL_STATUS_NVM3_PAGE_SIZE_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x005B))).value
except:
    pass

# sl_status.h: 203
try:
    SL_STATUS_NVM3_TOKEN_INIT_FAILED = (sl_status_t (ord_if_char(0x005C))).value
except:
    pass

# sl_status.h: 204
try:
    SL_STATUS_NVM3_OPENED_WITH_OTHER_PARAMETERS = (sl_status_t (ord_if_char(0x005D))).value
except:
    pass

# sl_status.h: 205
try:
    SL_STATUS_NVM3_NO_VALID_PAGES = (sl_status_t (ord_if_char(0x005E))).value
except:
    pass

# sl_status.h: 206
try:
    SL_STATUS_NVM3_OBJECT_SIZE_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x005F))).value
except:
    pass

# sl_status.h: 207
try:
    SL_STATUS_NVM3_OBJECT_IS_NOT_DATA = (sl_status_t (ord_if_char(0x0060))).value
except:
    pass

# sl_status.h: 208
try:
    SL_STATUS_NVM3_OBJECT_IS_NOT_A_COUNTER = (sl_status_t (ord_if_char(0x0061))).value
except:
    pass

# sl_status.h: 209
try:
    SL_STATUS_NVM3_WRITE_DATA_SIZE = (sl_status_t (ord_if_char(0x0062))).value
except:
    pass

# sl_status.h: 210
try:
    SL_STATUS_NVM3_READ_DATA_SIZE = (sl_status_t (ord_if_char(0x0063))).value
except:
    pass

# sl_status.h: 211
try:
    SL_STATUS_NVM3_INIT_WITH_FULL_NVM = (sl_status_t (ord_if_char(0x0064))).value
except:
    pass

# sl_status.h: 212
try:
    SL_STATUS_NVM3_RESIZE_PARAMETER = (sl_status_t (ord_if_char(0x0065))).value
except:
    pass

# sl_status.h: 213
try:
    SL_STATUS_NVM3_RESIZE_NOT_ENOUGH_SPACE = (sl_status_t (ord_if_char(0x0066))).value
except:
    pass

# sl_status.h: 214
try:
    SL_STATUS_NVM3_ERASE_COUNT_ERROR = (sl_status_t (ord_if_char(0x0067))).value
except:
    pass

# sl_status.h: 215
try:
    SL_STATUS_NVM3_NVM_ACCESS = (sl_status_t (ord_if_char(0x0068))).value
except:
    pass

# sl_status.h: 216
try:
    SL_STATUS_NVM3_WRITE_TO_NOT_ERASED = (sl_status_t (ord_if_char(0x006D))).value
except:
    pass

# sl_status.h: 217
try:
    SL_STATUS_NVM3_INVALID_ADDR = (sl_status_t (ord_if_char(0x006E))).value
except:
    pass

# sl_status.h: 218
try:
    SL_STATUS_NVM3_KEY_MISMATCH = (sl_status_t (ord_if_char(0x006F))).value
except:
    pass

# sl_status.h: 219
try:
    SL_STATUS_NVM3_SIZE_ERROR = (sl_status_t (ord_if_char(0x0070))).value
except:
    pass

# sl_status.h: 220
try:
    SL_STATUS_NVM3_EMULATOR = (sl_status_t (ord_if_char(0x0071))).value
except:
    pass

# sl_status.h: 223
try:
    SL_STATUS_SECURITY_ENCRYPT_ERROR = (sl_status_t (ord_if_char(0x0072))).value
except:
    pass

# sl_status.h: 224
try:
    SL_STATUS_SECURITY_KEY_ERROR = (sl_status_t (ord_if_char(0x0073))).value
except:
    pass

# sl_status.h: 225
try:
    SL_STATUS_SECURITY_RANDOM_NUM_GEN_ERROR = (sl_status_t (ord_if_char(0x0074))).value
except:
    pass

# sl_status.h: 228
try:
    SL_STATUS_BT_OUT_OF_BONDS = (sl_status_t (ord_if_char(0x0402))).value
except:
    pass

# sl_status.h: 229
try:
    SL_STATUS_BT_UNSPECIFIED = (sl_status_t (ord_if_char(0x0403))).value
except:
    pass

# sl_status.h: 230
try:
    SL_STATUS_BT_HARDWARE = (sl_status_t (ord_if_char(0x0404))).value
except:
    pass

# sl_status.h: 231
try:
    SL_STATUS_BT_NO_BONDING = (sl_status_t (ord_if_char(0x0406))).value
except:
    pass

# sl_status.h: 232
try:
    SL_STATUS_BT_CRYPTO = (sl_status_t (ord_if_char(0x0407))).value
except:
    pass

# sl_status.h: 233
try:
    SL_STATUS_BT_DATA_CORRUPTED = (sl_status_t (ord_if_char(0x0408))).value
except:
    pass

# sl_status.h: 234
try:
    SL_STATUS_BT_INVALID_SYNC_HANDLE = (sl_status_t (ord_if_char(0x040A))).value
except:
    pass

# sl_status.h: 235
try:
    SL_STATUS_BT_INVALID_MODULE_ACTION = (sl_status_t (ord_if_char(0x040B))).value
except:
    pass

# sl_status.h: 236
try:
    SL_STATUS_BT_RADIO = (sl_status_t (ord_if_char(0x040C))).value
except:
    pass

# sl_status.h: 237
try:
    SL_STATUS_BT_L2CAP_REMOTE_DISCONNECTED = (sl_status_t (ord_if_char(0x040D))).value
except:
    pass

# sl_status.h: 238
try:
    SL_STATUS_BT_L2CAP_LOCAL_DISCONNECTED = (sl_status_t (ord_if_char(0x040E))).value
except:
    pass

# sl_status.h: 239
try:
    SL_STATUS_BT_L2CAP_CID_NOT_EXIST = (sl_status_t (ord_if_char(0x040F))).value
except:
    pass

# sl_status.h: 240
try:
    SL_STATUS_BT_L2CAP_LE_DISCONNECTED = (sl_status_t (ord_if_char(0x0410))).value
except:
    pass

# sl_status.h: 241
try:
    SL_STATUS_BT_L2CAP_FLOW_CONTROL_VIOLATED = (sl_status_t (ord_if_char(0x0412))).value
except:
    pass

# sl_status.h: 242
try:
    SL_STATUS_BT_L2CAP_FLOW_CONTROL_CREDIT_OVERFLOWED = (sl_status_t (ord_if_char(0x0413))).value
except:
    pass

# sl_status.h: 243
try:
    SL_STATUS_BT_L2CAP_NO_FLOW_CONTROL_CREDIT = (sl_status_t (ord_if_char(0x0414))).value
except:
    pass

# sl_status.h: 244
try:
    SL_STATUS_BT_L2CAP_CONNECTION_REQUEST_TIMEOUT = (sl_status_t (ord_if_char(0x0415))).value
except:
    pass

# sl_status.h: 245
try:
    SL_STATUS_BT_L2CAP_INVALID_CID = (sl_status_t (ord_if_char(0x0416))).value
except:
    pass

# sl_status.h: 246
try:
    SL_STATUS_BT_L2CAP_WRONG_STATE = (sl_status_t (ord_if_char(0x0417))).value
except:
    pass

# sl_status.h: 247
try:
    SL_STATUS_BT_PS_STORE_FULL = (sl_status_t (ord_if_char(0x041B))).value
except:
    pass

# sl_status.h: 248
try:
    SL_STATUS_BT_PS_KEY_NOT_FOUND = (sl_status_t (ord_if_char(0x041C))).value
except:
    pass

# sl_status.h: 249
try:
    SL_STATUS_BT_APPLICATION_MISMATCHED_OR_INSUFFICIENT_SECURITY = (sl_status_t (ord_if_char(0x041D))).value
except:
    pass

# sl_status.h: 250
try:
    SL_STATUS_BT_APPLICATION_ENCRYPTION_DECRYPTION_ERROR = (sl_status_t (ord_if_char(0x041E))).value
except:
    pass

# sl_status.h: 253
try:
    SL_STATUS_BT_CTRL_UNKNOWN_HCI_COMMAND = (sl_status_t (ord_if_char(0x1001))).value
except:
    pass

# sl_status.h: 254
try:
    SL_STATUS_BT_CTRL_UNKNOWN_CONNECTION_IDENTIFIER = (sl_status_t (ord_if_char(0x1002))).value
except:
    pass

# sl_status.h: 255
try:
    SL_STATUS_BT_CTRL_HARDWARE_FAILURE = (sl_status_t (ord_if_char(0x1003))).value
except:
    pass

# sl_status.h: 256
try:
    SL_STATUS_BT_CTRL_PAGE_TIMEOUT = (sl_status_t (ord_if_char(0x1004))).value
except:
    pass

# sl_status.h: 257
try:
    SL_STATUS_BT_CTRL_AUTHENTICATION_FAILURE = (sl_status_t (ord_if_char(0x1005))).value
except:
    pass

# sl_status.h: 258
try:
    SL_STATUS_BT_CTRL_PIN_OR_KEY_MISSING = (sl_status_t (ord_if_char(0x1006))).value
except:
    pass

# sl_status.h: 259
try:
    SL_STATUS_BT_CTRL_MEMORY_CAPACITY_EXCEEDED = (sl_status_t (ord_if_char(0x1007))).value
except:
    pass

# sl_status.h: 260
try:
    SL_STATUS_BT_CTRL_CONNECTION_TIMEOUT = (sl_status_t (ord_if_char(0x1008))).value
except:
    pass

# sl_status.h: 261
try:
    SL_STATUS_BT_CTRL_CONNECTION_LIMIT_EXCEEDED = (sl_status_t (ord_if_char(0x1009))).value
except:
    pass

# sl_status.h: 262
try:
    SL_STATUS_BT_CTRL_SYNCHRONOUS_CONNECTION_LIMIT_EXCEEDED = (sl_status_t (ord_if_char(0x100A))).value
except:
    pass

# sl_status.h: 263
try:
    SL_STATUS_BT_CTRL_ACL_CONNECTION_ALREADY_EXISTS = (sl_status_t (ord_if_char(0x100B))).value
except:
    pass

# sl_status.h: 264
try:
    SL_STATUS_BT_CTRL_COMMAND_DISALLOWED = (sl_status_t (ord_if_char(0x100C))).value
except:
    pass

# sl_status.h: 265
try:
    SL_STATUS_BT_CTRL_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES = (sl_status_t (ord_if_char(0x100D))).value
except:
    pass

# sl_status.h: 266
try:
    SL_STATUS_BT_CTRL_CONNECTION_REJECTED_DUE_TO_SECURITY_REASONS = (sl_status_t (ord_if_char(0x100E))).value
except:
    pass

# sl_status.h: 267
try:
    SL_STATUS_BT_CTRL_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR = (sl_status_t (ord_if_char(0x100F))).value
except:
    pass

# sl_status.h: 268
try:
    SL_STATUS_BT_CTRL_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED = (sl_status_t (ord_if_char(0x1010))).value
except:
    pass

# sl_status.h: 269
try:
    SL_STATUS_BT_CTRL_UNSUPPORTED_FEATURE_OR_PARAMETER_VALUE = (sl_status_t (ord_if_char(0x1011))).value
except:
    pass

# sl_status.h: 270
try:
    SL_STATUS_BT_CTRL_INVALID_COMMAND_PARAMETERS = (sl_status_t (ord_if_char(0x1012))).value
except:
    pass

# sl_status.h: 271
try:
    SL_STATUS_BT_CTRL_REMOTE_USER_TERMINATED = (sl_status_t (ord_if_char(0x1013))).value
except:
    pass

# sl_status.h: 272
try:
    SL_STATUS_BT_CTRL_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES = (sl_status_t (ord_if_char(0x1014))).value
except:
    pass

# sl_status.h: 273
try:
    SL_STATUS_BT_CTRL_REMOTE_POWERING_OFF = (sl_status_t (ord_if_char(0x1015))).value
except:
    pass

# sl_status.h: 274
try:
    SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_BY_LOCAL_HOST = (sl_status_t (ord_if_char(0x1016))).value
except:
    pass

# sl_status.h: 275
try:
    SL_STATUS_BT_CTRL_REPEATED_ATTEMPTS = (sl_status_t (ord_if_char(0x1017))).value
except:
    pass

# sl_status.h: 276
try:
    SL_STATUS_BT_CTRL_PAIRING_NOT_ALLOWED = (sl_status_t (ord_if_char(0x1018))).value
except:
    pass

# sl_status.h: 277
try:
    SL_STATUS_BT_CTRL_UNKNOWN_LMP_PDU = (sl_status_t (ord_if_char(0x1019))).value
except:
    pass

# sl_status.h: 278
try:
    SL_STATUS_BT_CTRL_UNSUPPORTED_REMOTE_FEATURE = (sl_status_t (ord_if_char(0x101A))).value
except:
    pass

# sl_status.h: 279
try:
    SL_STATUS_BT_CTRL_SCO_OFFSET_REJECTED = (sl_status_t (ord_if_char(0x101B))).value
except:
    pass

# sl_status.h: 280
try:
    SL_STATUS_BT_CTRL_SCO_INTERVAL_REJECTED = (sl_status_t (ord_if_char(0x101C))).value
except:
    pass

# sl_status.h: 281
try:
    SL_STATUS_BT_CTRL_SCO_AIR_MODE_REJECTED = (sl_status_t (ord_if_char(0x101D))).value
except:
    pass

# sl_status.h: 282
try:
    SL_STATUS_BT_CTRL_INVALID_LL_PARAMETERS = (sl_status_t (ord_if_char(0x101E))).value
except:
    pass

# sl_status.h: 283
try:
    SL_STATUS_BT_CTRL_UNSPECIFIED_ERROR = (sl_status_t (ord_if_char(0x101F))).value
except:
    pass

# sl_status.h: 284
try:
    SL_STATUS_BT_CTRL_UNSUPPORTED_LL_PARAMETER_VALUE = (sl_status_t (ord_if_char(0x1020))).value
except:
    pass

# sl_status.h: 285
try:
    SL_STATUS_BT_CTRL_ROLE_CHANGE_NOT_ALLOWED = (sl_status_t (ord_if_char(0x1021))).value
except:
    pass

# sl_status.h: 286
try:
    SL_STATUS_BT_CTRL_LL_RESPONSE_TIMEOUT = (sl_status_t (ord_if_char(0x1022))).value
except:
    pass

# sl_status.h: 287
try:
    SL_STATUS_BT_CTRL_LL_PROCEDURE_COLLISION = (sl_status_t (ord_if_char(0x1023))).value
except:
    pass

# sl_status.h: 288
try:
    SL_STATUS_BT_CTRL_LMP_PDU_NOT_ALLOWED = (sl_status_t (ord_if_char(0x1024))).value
except:
    pass

# sl_status.h: 290
try:
    SL_STATUS_BT_CTRL_ENCRYPTION_MODE_NOT_ACCEPTABLE = (sl_status_t (ord_if_char(0x1025))).value
except:
    pass

# sl_status.h: 291
try:
    SL_STATUS_BT_CTRL_LINK_KEY_CANNOT_BE_CHANGED = (sl_status_t (ord_if_char(0x1026))).value
except:
    pass

# sl_status.h: 292
try:
    SL_STATUS_BT_CTRL_REQUESTED_QOS_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x1027))).value
except:
    pass

# sl_status.h: 293
try:
    SL_STATUS_BT_CTRL_INSTANT_PASSED = (sl_status_t (ord_if_char(0x1028))).value
except:
    pass

# sl_status.h: 294
try:
    SL_STATUS_BT_CTRL_PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x1029))).value
except:
    pass

# sl_status.h: 295
try:
    SL_STATUS_BT_CTRL_DIFFERENT_TRANSACTION_COLLISION = (sl_status_t (ord_if_char(0x102A))).value
except:
    pass

# sl_status.h: 297
try:
    SL_STATUS_BT_CTRL_QOS_UNACCEPTABLE_PARAMETER = (sl_status_t (ord_if_char(0x102C))).value
except:
    pass

# sl_status.h: 298
try:
    SL_STATUS_BT_CTRL_QOS_REJECTED = (sl_status_t (ord_if_char(0x102D))).value
except:
    pass

# sl_status.h: 299
try:
    SL_STATUS_BT_CTRL_CHANNEL_ASSESSMENT_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x102E))).value
except:
    pass

# sl_status.h: 300
try:
    SL_STATUS_BT_CTRL_INSUFFICIENT_SECURITY = (sl_status_t (ord_if_char(0x102F))).value
except:
    pass

# sl_status.h: 301
try:
    SL_STATUS_BT_CTRL_PARAMETER_OUT_OF_MANDATORY_RANGE = (sl_status_t (ord_if_char(0x1030))).value
except:
    pass

# sl_status.h: 303
try:
    SL_STATUS_BT_CTRL_ROLE_SWITCH_PENDING = (sl_status_t (ord_if_char(0x1032))).value
except:
    pass

# sl_status.h: 305
try:
    SL_STATUS_BT_CTRL_RESERVED_SLOT_VIOLATION = (sl_status_t (ord_if_char(0x1034))).value
except:
    pass

# sl_status.h: 306
try:
    SL_STATUS_BT_CTRL_ROLE_SWITCH_FAILED = (sl_status_t (ord_if_char(0x1035))).value
except:
    pass

# sl_status.h: 307
try:
    SL_STATUS_BT_CTRL_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE = (sl_status_t (ord_if_char(0x1036))).value
except:
    pass

# sl_status.h: 309
try:
    SL_STATUS_BT_CTRL_SIMPLE_PAIRING_NOT_SUPPORTED_BY_HOST = (sl_status_t (ord_if_char(0x1037))).value
except:
    pass

# sl_status.h: 310
try:
    SL_STATUS_BT_CTRL_HOST_BUSY_PAIRING = (sl_status_t (ord_if_char(0x1038))).value
except:
    pass

# sl_status.h: 311
try:
    SL_STATUS_BT_CTRL_CONNECTION_REJECTED_DUE_TO_NO_SUITABLE_CHANNEL_FOUND = (sl_status_t (ord_if_char(0x1039))).value
except:
    pass

# sl_status.h: 312
try:
    SL_STATUS_BT_CTRL_CONTROLLER_BUSY = (sl_status_t (ord_if_char(0x103A))).value
except:
    pass

# sl_status.h: 313
try:
    SL_STATUS_BT_CTRL_UNACCEPTABLE_CONNECTION_INTERVAL = (sl_status_t (ord_if_char(0x103B))).value
except:
    pass

# sl_status.h: 314
try:
    SL_STATUS_BT_CTRL_ADVERTISING_TIMEOUT = (sl_status_t (ord_if_char(0x103C))).value
except:
    pass

# sl_status.h: 315
try:
    SL_STATUS_BT_CTRL_CONNECTION_TERMINATED_DUE_TO_MIC_FAILURE = (sl_status_t (ord_if_char(0x103D))).value
except:
    pass

# sl_status.h: 316
try:
    SL_STATUS_BT_CTRL_CONNECTION_FAILED_TO_BE_ESTABLISHED = (sl_status_t (ord_if_char(0x103E))).value
except:
    pass

# sl_status.h: 317
try:
    SL_STATUS_BT_CTRL_MAC_CONNECTION_FAILED = (sl_status_t (ord_if_char(0x103F))).value
except:
    pass

# sl_status.h: 318
try:
    SL_STATUS_BT_CTRL_COARSE_CLOCK_ADJUSTMENT_REJECTED_BUT_WILL_TRY_TO_ADJUST_USING_CLOCK_DRAGGING = (sl_status_t (ord_if_char(0x1040))).value
except:
    pass

# sl_status.h: 319
try:
    SL_STATUS_BT_CTRL_TYPE0_SUBMAP_NOT_DEFINED = (sl_status_t (ord_if_char(0x1041))).value
except:
    pass

# sl_status.h: 320
try:
    SL_STATUS_BT_CTRL_UNKNOWN_ADVERTISING_IDENTIFIER = (sl_status_t (ord_if_char(0x1042))).value
except:
    pass

# sl_status.h: 321
try:
    SL_STATUS_BT_CTRL_LIMIT_REACHED = (sl_status_t (ord_if_char(0x1043))).value
except:
    pass

# sl_status.h: 322
try:
    SL_STATUS_BT_CTRL_OPERATION_CANCELLED_BY_HOST = (sl_status_t (ord_if_char(0x1044))).value
except:
    pass

# sl_status.h: 323
try:
    SL_STATUS_BT_CTRL_PACKET_TOO_LONG = (sl_status_t (ord_if_char(0x1045))).value
except:
    pass

# sl_status.h: 324
try:
    SL_STATUS_BT_CTRL_TOO_LATE = (sl_status_t (ord_if_char(0x1046))).value
except:
    pass

# sl_status.h: 325
try:
    SL_STATUS_BT_CTRL_TOO_EARLY = (sl_status_t (ord_if_char(0x1047))).value
except:
    pass

# sl_status.h: 326
try:
    SL_STATUS_BT_CTRL_INSUFFICIENT_CHANNELS = (sl_status_t (ord_if_char(0x1048))).value
except:
    pass

# sl_status.h: 329
try:
    SL_STATUS_BT_ATT_INVALID_HANDLE = (sl_status_t (ord_if_char(0x1101))).value
except:
    pass

# sl_status.h: 330
try:
    SL_STATUS_BT_ATT_READ_NOT_PERMITTED = (sl_status_t (ord_if_char(0x1102))).value
except:
    pass

# sl_status.h: 331
try:
    SL_STATUS_BT_ATT_WRITE_NOT_PERMITTED = (sl_status_t (ord_if_char(0x1103))).value
except:
    pass

# sl_status.h: 332
try:
    SL_STATUS_BT_ATT_INVALID_PDU = (sl_status_t (ord_if_char(0x1104))).value
except:
    pass

# sl_status.h: 333
try:
    SL_STATUS_BT_ATT_INSUFFICIENT_AUTHENTICATION = (sl_status_t (ord_if_char(0x1105))).value
except:
    pass

# sl_status.h: 334
try:
    SL_STATUS_BT_ATT_REQUEST_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x1106))).value
except:
    pass

# sl_status.h: 335
try:
    SL_STATUS_BT_ATT_INVALID_OFFSET = (sl_status_t (ord_if_char(0x1107))).value
except:
    pass

# sl_status.h: 336
try:
    SL_STATUS_BT_ATT_INSUFFICIENT_AUTHORIZATION = (sl_status_t (ord_if_char(0x1108))).value
except:
    pass

# sl_status.h: 337
try:
    SL_STATUS_BT_ATT_PREPARE_QUEUE_FULL = (sl_status_t (ord_if_char(0x1109))).value
except:
    pass

# sl_status.h: 338
try:
    SL_STATUS_BT_ATT_ATT_NOT_FOUND = (sl_status_t (ord_if_char(0x110A))).value
except:
    pass

# sl_status.h: 339
try:
    SL_STATUS_BT_ATT_ATT_NOT_LONG = (sl_status_t (ord_if_char(0x110B))).value
except:
    pass

# sl_status.h: 340
try:
    SL_STATUS_BT_ATT_ENCRYPTION_KEY_SIZE_TOO_SHORT = (sl_status_t (ord_if_char(0x110C))).value
except:
    pass

# sl_status.h: 341
try:
    SL_STATUS_BT_ATT_INSUFFICIENT_ENC_KEY_SIZE = SL_STATUS_BT_ATT_ENCRYPTION_KEY_SIZE_TOO_SHORT
except:
    pass

# sl_status.h: 342
try:
    SL_STATUS_BT_ATT_INVALID_ATT_LENGTH = (sl_status_t (ord_if_char(0x110D))).value
except:
    pass

# sl_status.h: 343
try:
    SL_STATUS_BT_ATT_UNLIKELY_ERROR = (sl_status_t (ord_if_char(0x110E))).value
except:
    pass

# sl_status.h: 344
try:
    SL_STATUS_BT_ATT_INSUFFICIENT_ENCRYPTION = (sl_status_t (ord_if_char(0x110F))).value
except:
    pass

# sl_status.h: 345
try:
    SL_STATUS_BT_ATT_UNSUPPORTED_GROUP_TYPE = (sl_status_t (ord_if_char(0x1110))).value
except:
    pass

# sl_status.h: 346
try:
    SL_STATUS_BT_ATT_INSUFFICIENT_RESOURCES = (sl_status_t (ord_if_char(0x1111))).value
except:
    pass

# sl_status.h: 347
try:
    SL_STATUS_BT_ATT_OUT_OF_SYNC = (sl_status_t (ord_if_char(0x1112))).value
except:
    pass

# sl_status.h: 348
try:
    SL_STATUS_BT_ATT_VALUE_NOT_ALLOWED = (sl_status_t (ord_if_char(0x1113))).value
except:
    pass

# sl_status.h: 349
try:
    SL_STATUS_BT_ATT_APPLICATION = (sl_status_t (ord_if_char(0x1180))).value
except:
    pass

# sl_status.h: 350
try:
    SL_STATUS_BT_ATT_WRITE_REQUEST_REJECTED = (sl_status_t (ord_if_char(0x11FC))).value
except:
    pass

# sl_status.h: 351
try:
    SL_STATUS_BT_ATT_CLIENT_CHARACTERISTIC_CONFIGURATION_DESCRIPTOR_IMPROPERLY_CONFIGURED = (sl_status_t (ord_if_char(0x11FD))).value
except:
    pass

# sl_status.h: 352
try:
    SL_STATUS_BT_ATT_PROCEDURE_ALREADY_IN_PROGRESS = (sl_status_t (ord_if_char(0x11FE))).value
except:
    pass

# sl_status.h: 353
try:
    SL_STATUS_BT_ATT_OUT_OF_RANGE = (sl_status_t (ord_if_char(0x11FF))).value
except:
    pass

# sl_status.h: 356
try:
    SL_STATUS_BT_SMP_PASSKEY_ENTRY_FAILED = (sl_status_t (ord_if_char(0x1201))).value
except:
    pass

# sl_status.h: 357
try:
    SL_STATUS_BT_SMP_OOB_NOT_AVAILABLE = (sl_status_t (ord_if_char(0x1202))).value
except:
    pass

# sl_status.h: 358
try:
    SL_STATUS_BT_SMP_AUTHENTICATION_REQUIREMENTS = (sl_status_t (ord_if_char(0x1203))).value
except:
    pass

# sl_status.h: 359
try:
    SL_STATUS_BT_SMP_CONFIRM_VALUE_FAILED = (sl_status_t (ord_if_char(0x1204))).value
except:
    pass

# sl_status.h: 360
try:
    SL_STATUS_BT_SMP_PAIRING_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x1205))).value
except:
    pass

# sl_status.h: 361
try:
    SL_STATUS_BT_SMP_ENCRYPTION_KEY_SIZE = (sl_status_t (ord_if_char(0x1206))).value
except:
    pass

# sl_status.h: 362
try:
    SL_STATUS_BT_SMP_COMMAND_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x1207))).value
except:
    pass

# sl_status.h: 363
try:
    SL_STATUS_BT_SMP_UNSPECIFIED_REASON = (sl_status_t (ord_if_char(0x1208))).value
except:
    pass

# sl_status.h: 364
try:
    SL_STATUS_BT_SMP_REPEATED_ATTEMPTS = (sl_status_t (ord_if_char(0x1209))).value
except:
    pass

# sl_status.h: 365
try:
    SL_STATUS_BT_SMP_INVALID_PARAMETERS = (sl_status_t (ord_if_char(0x120A))).value
except:
    pass

# sl_status.h: 366
try:
    SL_STATUS_BT_SMP_DHKEY_CHECK_FAILED = (sl_status_t (ord_if_char(0x120B))).value
except:
    pass

# sl_status.h: 367
try:
    SL_STATUS_BT_SMP_NUMERIC_COMPARISON_FAILED = (sl_status_t (ord_if_char(0x120C))).value
except:
    pass

# sl_status.h: 368
try:
    SL_STATUS_BT_SMP_BREDR_PAIRING_IN_PROGRESS = (sl_status_t (ord_if_char(0x120D))).value
except:
    pass

# sl_status.h: 369
try:
    SL_STATUS_BT_SMP_CROSS_TRANSPORT_KEY_DERIVATION_GENERATION_NOT_ALLOWED = (sl_status_t (ord_if_char(0x120E))).value
except:
    pass

# sl_status.h: 370
try:
    SL_STATUS_BT_SMP_KEY_REJECTED = (sl_status_t (ord_if_char(0x120F))).value
except:
    pass

# sl_status.h: 371
try:
    SL_STATUS_BT_SMP_BUSY = (sl_status_t (ord_if_char(0x1210))).value
except:
    pass

# sl_status.h: 374
try:
    SL_STATUS_BT_MESH_ALREADY_EXISTS = (sl_status_t (ord_if_char(0x0501))).value
except:
    pass

# sl_status.h: 375
try:
    SL_STATUS_BT_MESH_DOES_NOT_EXIST = (sl_status_t (ord_if_char(0x0502))).value
except:
    pass

# sl_status.h: 376
try:
    SL_STATUS_BT_MESH_LIMIT_REACHED = (sl_status_t (ord_if_char(0x0503))).value
except:
    pass

# sl_status.h: 377
try:
    SL_STATUS_BT_MESH_INVALID_ADDRESS = (sl_status_t (ord_if_char(0x0504))).value
except:
    pass

# sl_status.h: 378
try:
    SL_STATUS_BT_MESH_MALFORMED_DATA = (sl_status_t (ord_if_char(0x0505))).value
except:
    pass

# sl_status.h: 379
try:
    SL_STATUS_BT_MESH_ALREADY_INITIALIZED = (sl_status_t (ord_if_char(0x0506))).value
except:
    pass

# sl_status.h: 380
try:
    SL_STATUS_BT_MESH_NOT_INITIALIZED = (sl_status_t (ord_if_char(0x0507))).value
except:
    pass

# sl_status.h: 381
try:
    SL_STATUS_BT_MESH_NO_FRIEND_OFFER = (sl_status_t (ord_if_char(0x0508))).value
except:
    pass

# sl_status.h: 382
try:
    SL_STATUS_BT_MESH_PROV_LINK_CLOSED = (sl_status_t (ord_if_char(0x0509))).value
except:
    pass

# sl_status.h: 383
try:
    SL_STATUS_BT_MESH_PROV_INVALID_PDU = (sl_status_t (ord_if_char(0x050A))).value
except:
    pass

# sl_status.h: 384
try:
    SL_STATUS_BT_MESH_PROV_INVALID_PDU_FORMAT = (sl_status_t (ord_if_char(0x050B))).value
except:
    pass

# sl_status.h: 385
try:
    SL_STATUS_BT_MESH_PROV_UNEXPECTED_PDU = (sl_status_t (ord_if_char(0x050C))).value
except:
    pass

# sl_status.h: 386
try:
    SL_STATUS_BT_MESH_PROV_CONFIRMATION_FAILED = (sl_status_t (ord_if_char(0x050D))).value
except:
    pass

# sl_status.h: 387
try:
    SL_STATUS_BT_MESH_PROV_OUT_OF_RESOURCES = (sl_status_t (ord_if_char(0x050E))).value
except:
    pass

# sl_status.h: 388
try:
    SL_STATUS_BT_MESH_PROV_DECRYPTION_FAILED = (sl_status_t (ord_if_char(0x050F))).value
except:
    pass

# sl_status.h: 389
try:
    SL_STATUS_BT_MESH_PROV_UNEXPECTED_ERROR = (sl_status_t (ord_if_char(0x0510))).value
except:
    pass

# sl_status.h: 390
try:
    SL_STATUS_BT_MESH_PROV_CANNOT_ASSIGN_ADDR = (sl_status_t (ord_if_char(0x0511))).value
except:
    pass

# sl_status.h: 391
try:
    SL_STATUS_BT_MESH_ADDRESS_TEMPORARILY_UNAVAILABLE = (sl_status_t (ord_if_char(0x0512))).value
except:
    pass

# sl_status.h: 392
try:
    SL_STATUS_BT_MESH_ADDRESS_ALREADY_USED = (sl_status_t (ord_if_char(0x0513))).value
except:
    pass

# sl_status.h: 393
try:
    SL_STATUS_BT_MESH_PUBLISH_NOT_CONFIGURED = (sl_status_t (ord_if_char(0x0514))).value
except:
    pass

# sl_status.h: 394
try:
    SL_STATUS_BT_MESH_APP_KEY_NOT_BOUND = (sl_status_t (ord_if_char(0x0515))).value
except:
    pass

# sl_status.h: 397
try:
    SL_STATUS_BT_MESH_FOUNDATION_INVALID_ADDRESS = (sl_status_t (ord_if_char(0x1301))).value
except:
    pass

# sl_status.h: 398
try:
    SL_STATUS_BT_MESH_FOUNDATION_INVALID_MODEL = (sl_status_t (ord_if_char(0x1302))).value
except:
    pass

# sl_status.h: 399
try:
    SL_STATUS_BT_MESH_FOUNDATION_INVALID_APP_KEY = (sl_status_t (ord_if_char(0x1303))).value
except:
    pass

# sl_status.h: 400
try:
    SL_STATUS_BT_MESH_FOUNDATION_INVALID_NET_KEY = (sl_status_t (ord_if_char(0x1304))).value
except:
    pass

# sl_status.h: 401
try:
    SL_STATUS_BT_MESH_FOUNDATION_INSUFFICIENT_RESOURCES = (sl_status_t (ord_if_char(0x1305))).value
except:
    pass

# sl_status.h: 402
try:
    SL_STATUS_BT_MESH_FOUNDATION_KEY_INDEX_EXISTS = (sl_status_t (ord_if_char(0x1306))).value
except:
    pass

# sl_status.h: 403
try:
    SL_STATUS_BT_MESH_FOUNDATION_INVALID_PUBLISH_PARAMS = (sl_status_t (ord_if_char(0x1307))).value
except:
    pass

# sl_status.h: 404
try:
    SL_STATUS_BT_MESH_FOUNDATION_NOT_SUBSCRIBE_MODEL = (sl_status_t (ord_if_char(0x1308))).value
except:
    pass

# sl_status.h: 405
try:
    SL_STATUS_BT_MESH_FOUNDATION_STORAGE_FAILURE = (sl_status_t (ord_if_char(0x1309))).value
except:
    pass

# sl_status.h: 406
try:
    SL_STATUS_BT_MESH_FOUNDATION_NOT_SUPPORTED = (sl_status_t (ord_if_char(0x130A))).value
except:
    pass

# sl_status.h: 407
try:
    SL_STATUS_BT_MESH_FOUNDATION_CANNOT_UPDATE = (sl_status_t (ord_if_char(0x130B))).value
except:
    pass

# sl_status.h: 408
try:
    SL_STATUS_BT_MESH_FOUNDATION_CANNOT_REMOVE = (sl_status_t (ord_if_char(0x130C))).value
except:
    pass

# sl_status.h: 409
try:
    SL_STATUS_BT_MESH_FOUNDATION_CANNOT_BIND = (sl_status_t (ord_if_char(0x130D))).value
except:
    pass

# sl_status.h: 410
try:
    SL_STATUS_BT_MESH_FOUNDATION_TEMPORARILY_UNABLE = (sl_status_t (ord_if_char(0x130E))).value
except:
    pass

# sl_status.h: 411
try:
    SL_STATUS_BT_MESH_FOUNDATION_CANNOT_SET = (sl_status_t (ord_if_char(0x130F))).value
except:
    pass

# sl_status.h: 412
try:
    SL_STATUS_BT_MESH_FOUNDATION_UNSPECIFIED = (sl_status_t (ord_if_char(0x1310))).value
except:
    pass

# sl_status.h: 413
try:
    SL_STATUS_BT_MESH_FOUNDATION_INVALID_BINDING = (sl_status_t (ord_if_char(0x1311))).value
except:
    pass

# sl_status.h: 418
try:
    SL_STATUS_WIFI_INVALID_KEY = (sl_status_t (ord_if_char(0x0B01))).value
except:
    pass

# sl_status.h: 419
try:
    SL_STATUS_WIFI_FIRMWARE_DOWNLOAD_TIMEOUT = (sl_status_t (ord_if_char(0x0B02))).value
except:
    pass

# sl_status.h: 420
try:
    SL_STATUS_WIFI_UNSUPPORTED_MESSAGE_ID = (sl_status_t (ord_if_char(0x0B03))).value
except:
    pass

# sl_status.h: 421
try:
    SL_STATUS_WIFI_WARNING = (sl_status_t (ord_if_char(0x0B04))).value
except:
    pass

# sl_status.h: 422
try:
    SL_STATUS_WIFI_NO_PACKET_TO_RECEIVE = (sl_status_t (ord_if_char(0x0B05))).value
except:
    pass

# sl_status.h: 423
try:
    SL_STATUS_WIFI_SLEEP_GRANTED = (sl_status_t (ord_if_char(0x0B08))).value
except:
    pass

# sl_status.h: 424
try:
    SL_STATUS_WIFI_SLEEP_NOT_GRANTED = (sl_status_t (ord_if_char(0x0B09))).value
except:
    pass

# sl_status.h: 425
try:
    SL_STATUS_WIFI_SECURE_LINK_MAC_KEY_ERROR = (sl_status_t (ord_if_char(0x0B10))).value
except:
    pass

# sl_status.h: 426
try:
    SL_STATUS_WIFI_SECURE_LINK_MAC_KEY_ALREADY_BURNED = (sl_status_t (ord_if_char(0x0B11))).value
except:
    pass

# sl_status.h: 427
try:
    SL_STATUS_WIFI_SECURE_LINK_RAM_MODE_NOT_ALLOWED = (sl_status_t (ord_if_char(0x0B12))).value
except:
    pass

# sl_status.h: 428
try:
    SL_STATUS_WIFI_SECURE_LINK_FAILED_UNKNOWN_MODE = (sl_status_t (ord_if_char(0x0B13))).value
except:
    pass

# sl_status.h: 429
try:
    SL_STATUS_WIFI_SECURE_LINK_EXCHANGE_FAILED = (sl_status_t (ord_if_char(0x0B14))).value
except:
    pass

# sl_status.h: 430
try:
    SL_STATUS_WIFI_WRONG_STATE = (sl_status_t (ord_if_char(0x0B18))).value
except:
    pass

# sl_status.h: 431
try:
    SL_STATUS_WIFI_CHANNEL_NOT_ALLOWED = (sl_status_t (ord_if_char(0x0B19))).value
except:
    pass

# sl_status.h: 432
try:
    SL_STATUS_WIFI_NO_MATCHING_AP = (sl_status_t (ord_if_char(0x0B1A))).value
except:
    pass

# sl_status.h: 433
try:
    SL_STATUS_WIFI_CONNECTION_ABORTED = (sl_status_t (ord_if_char(0x0B1B))).value
except:
    pass

# sl_status.h: 434
try:
    SL_STATUS_WIFI_CONNECTION_TIMEOUT = (sl_status_t (ord_if_char(0x0B1C))).value
except:
    pass

# sl_status.h: 435
try:
    SL_STATUS_WIFI_CONNECTION_REJECTED_BY_AP = (sl_status_t (ord_if_char(0x0B1D))).value
except:
    pass

# sl_status.h: 436
try:
    SL_STATUS_WIFI_CONNECTION_AUTH_FAILURE = (sl_status_t (ord_if_char(0x0B1E))).value
except:
    pass

# sl_status.h: 437
try:
    SL_STATUS_WIFI_RETRY_EXCEEDED = (sl_status_t (ord_if_char(0x0B1F))).value
except:
    pass

# sl_status.h: 438
try:
    SL_STATUS_WIFI_TX_LIFETIME_EXCEEDED = (sl_status_t (ord_if_char(0x0B20))).value
except:
    pass

# sl_status.h: 443
try:
    SL_STATUS_COMPUTE_DRIVER_FAULT = (sl_status_t (ord_if_char(0x1501))).value
except:
    pass

# sl_status.h: 444
try:
    SL_STATUS_COMPUTE_DRIVER_ALU_NAN = (sl_status_t (ord_if_char(0x1502))).value
except:
    pass

# sl_status.h: 445
try:
    SL_STATUS_COMPUTE_DRIVER_ALU_OVERFLOW = (sl_status_t (ord_if_char(0x1503))).value
except:
    pass

# sl_status.h: 446
try:
    SL_STATUS_COMPUTE_DRIVER_ALU_UNDERFLOW = (sl_status_t (ord_if_char(0x1504))).value
except:
    pass

# sl_status.h: 447
try:
    SL_STATUS_COMPUTE_DRIVER_STORE_CONVERSION_OVERFLOW = (sl_status_t (ord_if_char(0x1505))).value
except:
    pass

# sl_status.h: 448
try:
    SL_STATUS_COMPUTE_DRIVER_STORE_CONVERSION_UNDERFLOW = (sl_status_t (ord_if_char(0x1506))).value
except:
    pass

# sl_status.h: 449
try:
    SL_STATUS_COMPUTE_DRIVER_STORE_CONVERSION_INFINITY = (sl_status_t (ord_if_char(0x1507))).value
except:
    pass

# sl_status.h: 450
try:
    SL_STATUS_COMPUTE_DRIVER_STORE_CONVERSION_NAN = (sl_status_t (ord_if_char(0x1508))).value
except:
    pass

# sl_status.h: 452
try:
    SL_STATUS_COMPUTE_MATH_NAN = (sl_status_t (ord_if_char(0x1512))).value
except:
    pass

# sl_status.h: 453
try:
    SL_STATUS_COMPUTE_MATH_INFINITY = (sl_status_t (ord_if_char(0x1513))).value
except:
    pass

# sl_status.h: 454
try:
    SL_STATUS_COMPUTE_MATH_OVERFLOW = (sl_status_t (ord_if_char(0x1514))).value
except:
    pass

# sl_status.h: 455
try:
    SL_STATUS_COMPUTE_MATH_UNDERFLOW = (sl_status_t (ord_if_char(0x1515))).value
except:
    pass

# sl_status.h: 459
try:
    SL_STATUS_ZIGBEE_PACKET_HANDOFF_DROPPED = (sl_status_t (ord_if_char(0x0C01))).value
except:
    pass

# sl_status.h: 460
try:
    SL_STATUS_ZIGBEE_DELIVERY_FAILED = (sl_status_t (ord_if_char(0x0C02))).value
except:
    pass

# sl_status.h: 461
try:
    SL_STATUS_ZIGBEE_MAX_MESSAGE_LIMIT_REACHED = (sl_status_t (ord_if_char(0x0C03))).value
except:
    pass

# sl_status.h: 462
try:
    SL_STATUS_ZIGBEE_BINDING_IS_ACTIVE = (sl_status_t (ord_if_char(0x0C04))).value
except:
    pass

# sl_status.h: 463
try:
    SL_STATUS_ZIGBEE_ADDRESS_TABLE_ENTRY_IS_ACTIVE = (sl_status_t (ord_if_char(0x0C05))).value
except:
    pass

# sl_status.h: 464
try:
    SL_STATUS_ZIGBEE_MOVE_FAILED = (sl_status_t (ord_if_char(0x0C06))).value
except:
    pass

# sl_status.h: 465
try:
    SL_STATUS_ZIGBEE_NODE_ID_CHANGED = (sl_status_t (ord_if_char(0x0C07))).value
except:
    pass

# sl_status.h: 466
try:
    SL_STATUS_ZIGBEE_INVALID_SECURITY_LEVEL = (sl_status_t (ord_if_char(0x0C08))).value
except:
    pass

# sl_status.h: 467
try:
    SL_STATUS_ZIGBEE_IEEE_ADDRESS_DISCOVERY_IN_PROGRESS = (sl_status_t (ord_if_char(0x0C09))).value
except:
    pass

# sl_status.h: 468
try:
    SL_STATUS_ZIGBEE_APS_ENCRYPTION_ERROR = (sl_status_t (ord_if_char(0x0C0A))).value
except:
    pass

# sl_status.h: 469
try:
    SL_STATUS_ZIGBEE_SECURITY_STATE_NOT_SET = (sl_status_t (ord_if_char(0x0C0B))).value
except:
    pass

# sl_status.h: 470
try:
    SL_STATUS_ZIGBEE_TOO_SOON_FOR_SWITCH_KEY = (sl_status_t (ord_if_char(0x0C0C))).value
except:
    pass

# sl_status.h: 471
try:
    SL_STATUS_ZIGBEE_SIGNATURE_VERIFY_FAILURE = (sl_status_t (ord_if_char(0x0C0D))).value
except:
    pass

# sl_status.h: 472
try:
    SL_STATUS_ZIGBEE_KEY_NOT_AUTHORIZED = (sl_status_t (ord_if_char(0x0C0E))).value
except:
    pass

# sl_status.h: 473
try:
    SL_STATUS_ZIGBEE_BINDING_HAS_CHANGED = (sl_status_t (ord_if_char(0x0C0F))).value
except:
    pass

# sl_status.h: 474
try:
    SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_CHANGED = (sl_status_t (ord_if_char(0x0C10))).value
except:
    pass

# sl_status.h: 475
try:
    SL_STATUS_ZIGBEE_TRUST_CENTER_SWAP_EUI_HAS_NOT_CHANGED = (sl_status_t (ord_if_char(0x0C11))).value
except:
    pass

# sl_status.h: 476
try:
    SL_STATUS_ZIGBEE_INSUFFICIENT_RANDOM_DATA = (sl_status_t (ord_if_char(0x0C12))).value
except:
    pass

# sl_status.h: 477
try:
    SL_STATUS_ZIGBEE_SOURCE_ROUTE_FAILURE = (sl_status_t (ord_if_char(0x0C13))).value
except:
    pass

# sl_status.h: 478
try:
    SL_STATUS_ZIGBEE_MANY_TO_ONE_ROUTE_FAILURE = (sl_status_t (ord_if_char(0x0C14))).value
except:
    pass

# sl_status.h: 479
try:
    SL_STATUS_ZIGBEE_STACK_AND_HARDWARE_MISMATCH = (sl_status_t (ord_if_char(0x0C15))).value
except:
    pass

# sl_status.h: 480
try:
    SL_STATUS_ZIGBEE_PAN_ID_CHANGED = (sl_status_t (ord_if_char(0x0C16))).value
except:
    pass

# sl_status.h: 481
try:
    SL_STATUS_ZIGBEE_CHANNEL_CHANGED = (sl_status_t (ord_if_char(0x0C17))).value
except:
    pass

# sl_status.h: 482
try:
    SL_STATUS_ZIGBEE_NETWORK_OPENED = (sl_status_t (ord_if_char(0x0C18))).value
except:
    pass

# sl_status.h: 483
try:
    SL_STATUS_ZIGBEE_NETWORK_CLOSED = (sl_status_t (ord_if_char(0x0C19))).value
except:
    pass

# sl_status.h: 484
try:
    SL_STATUS_ZIGBEE_RECEIVED_KEY_IN_THE_CLEAR = (sl_status_t (ord_if_char(0x0C1A))).value
except:
    pass

# sl_status.h: 485
try:
    SL_STATUS_ZIGBEE_NO_NETWORK_KEY_RECEIVED = (sl_status_t (ord_if_char(0x0C1B))).value
except:
    pass

# sl_status.h: 486
try:
    SL_STATUS_ZIGBEE_NO_LINK_KEY_RECEIVED = (sl_status_t (ord_if_char(0x0C1C))).value
except:
    pass

# sl_status.h: 487
try:
    SL_STATUS_ZIGBEE_PRECONFIGURED_KEY_REQUIRED = (sl_status_t (ord_if_char(0x0C1D))).value
except:
    pass

# sl_status.h: 488
try:
    SL_STATUS_ZIGBEE_EZSP_ERROR = (sl_status_t (ord_if_char(0x0C1E))).value
except:
    pass

# sl_status.h: 489
try:
    SL_STATUS_ZIGBEE_ID_DISCOVERY_FAILED = (sl_status_t (ord_if_char(0x0C1F))).value
except:
    pass

# sl_status.h: 490
try:
    SL_STATUS_ZIGBEE_NO_APS_ACK = (sl_status_t (ord_if_char(0x0C20))).value
except:
    pass

# sl_status.h: 491
try:
    SL_STATUS_ZIGBEE_APS_MESSAGE_CANCELED = (sl_status_t (ord_if_char(0x0C21))).value
except:
    pass

# sl_status.h: 492
try:
    SL_STATUS_ZIGBEE_ID_DISCOVERY_NOT_ENABLED = (sl_status_t (ord_if_char(0x0C22))).value
except:
    pass

# sl_status.h: 493
try:
    SL_STATUS_ZIGBEE_ID_DISCOVERY_UNDERWAY = (sl_status_t (ord_if_char(0x0C23))).value
except:
    pass

# sl_status.h: 494
try:
    SL_STATUS_ZIGBEE_SEND_UNICAST_ROUTE_DISCOVERY_UNDERWAY = (sl_status_t (ord_if_char(0x0C24))).value
except:
    pass

# sl_status.h: 495
try:
    SL_STATUS_ZIGBEE_SEND_UNICAST_FAILURE = (sl_status_t (ord_if_char(0x0C25))).value
except:
    pass

# sl_status.h: 496
try:
    SL_STATUS_ZIGBEE_SEND_UNICAST_NO_ROUTE = (sl_status_t (ord_if_char(0x0C26))).value
except:
    pass

# sl_status.h: 497
try:
    SL_STATUS_ZIGBEE_BROADCAST_TO_SLEEPY_CHILDREN_TIMEOUT = (sl_status_t (ord_if_char(0x0C27))).value
except:
    pass

# sl_status.h: 498
try:
    SL_STATUS_ZIGBEE_BROADCAST_RELAY_FAILED = (sl_status_t (ord_if_char(0x0C28))).value
except:
    pass

# sl_status.h: 499
try:
    SL_STATUS_ZIGBEE_REJOIN_FAILED_BUT_NETWORK_RESTORED = (sl_status_t (ord_if_char(0x0C29))).value
except:
    pass

# esl_lib.h: 48
try:
    ESL_LIB_INVALID_SERVICE_HANDLE = UINT32_MAX
except:
    pass

# esl_lib.h: 49
try:
    ESL_LIB_INVALID_CHARACTERISTIC_HANDLE = UINT16_MAX
except:
    pass

# esl_lib.h: 52
try:
    ESL_LIB_LTK_SIZE = 16
except:
    pass

# esl_lib.h: 53
try:
    ESL_LIB_OOB_SIZE = 32
except:
    pass

# esl_lib.h: 54
try:
    ESL_LIB_PASSKEY_SIZE = 4
except:
    pass

# esl_lib.h: 56
try:
    ESL_LIB_ARRAY_HEADER_SIZE = sizeof(uint8_t)
except:
    pass

# esl_lib.h: 57
try:
    ESL_LIB_LONG_ARRAY_HEADER_SIZE = sizeof(uint32_t)
except:
    pass

# esl_lib.h: 59
try:
    ESL_LIB_TLV_HEADER_LEN = sizeof(esl_lib_tlv_t)
except:
    pass

# esl_lib.h: 66
try:
    ESL_LIB_PAWR_MIN_INTERVAL_DEFAULT = 1197
except:
    pass

# esl_lib.h: 69
try:
    ESL_LIB_PAWR_MAX_INTERVAL_DEFAULT = ESL_LIB_PAWR_MIN_INTERVAL_DEFAULT
except:
    pass

# esl_lib.h: 71
try:
    ESL_LIB_PAWR_SUBEVENT_COUNT_DEFAULT = 57
except:
    pass

# esl_lib.h: 73
try:
    ESL_LIB_PAWR_SUBEVENT_INTERVAL_DEFAULT = 21
except:
    pass

# esl_lib.h: 76
try:
    ESL_LIB_PAWR_RESPONSE_SLOT_DELAY_DEFAULT = 7
except:
    pass

# esl_lib.h: 80
try:
    ESL_LIB_PAWR_RESPONSE_SLOT_SPACING_DEFAULT = 6
except:
    pass

# esl_lib.h: 83
try:
    ESL_LIB_PAWR_RESPONSE_SLOT_COUNT_DEFAULT = 23
except:
    pass

# esl_lib.h: 87
try:
    ESL_LIB_CONN_INTERVAL_MIN = 0x0006
except:
    pass

# esl_lib.h: 88
try:
    ESL_LIB_CONN_INTERVAL_MAX = 0x0c80
except:
    pass

# esl_lib.h: 89
try:
    ESL_LIB_CONN_MIN_TIMEOUT = 0x000a
except:
    pass

# esl_lib.h: 90
try:
    ESL_LIB_CONN_MAX_TIMEOUT = 0x0c80
except:
    pass

# esl_lib.h: 91
try:
    ESL_LIB_CONN_PERIPHERAL_MIN_LATENCY = 0
except:
    pass

# esl_lib.h: 92
try:
    ESL_LIB_CONN_PERIPHERAL_MAX_LATENCY = 0x1f3
except:
    pass

# esl_lib.h: 93
try:
    ESL_LIB_CONN_MIN_CE_LENGTH = 0
except:
    pass

# esl_lib.h: 94
try:
    ESL_LIB_CONN_MAX_CE_LENGTH = 0xffff
except:
    pass

# esl_lib.h: 98
try:
    ESL_LIB_PAWR_MIN_PA_INTERVAL = 0x6
except:
    pass

# esl_lib.h: 99
try:
    ESL_LIB_PAWR_MAX_PA_INTERVAL = 0xffff
except:
    pass

# esl_lib.h: 100
try:
    ESL_LIB_PAWR_MIN_NUM_SUBEVENTS = 0x01
except:
    pass

# esl_lib.h: 101
try:
    ESL_LIB_PAWR_MAX_NUM_SUBEVENTS = 0x80
except:
    pass

# esl_lib.h: 102
try:
    ESL_LIB_PAWR_MIN_SUBEVENT_INTERVAL = 0x6
except:
    pass

# esl_lib.h: 103
try:
    ESL_LIB_PAWR_MAX_SUBEVENT_INTERVAL = 0xff
except:
    pass

# esl_lib.h: 104
try:
    ESL_LIB_PAWR_MIN_RESPONSE_SLOT_DELAY = 0x1
except:
    pass

# esl_lib.h: 105
try:
    ESL_LIB_PAWR_MAX_RESPONSE_SLOT_DELAY = 0xfe
except:
    pass

# esl_lib.h: 106
try:
    ESL_LIB_PAWR_MIN_RESPONSE_SLOT_SPACING = 0x2
except:
    pass

# esl_lib.h: 107
try:
    ESL_LIB_PAWR_MAX_RESPONSE_SLOT_SPACING = 0xff
except:
    pass

# esl_lib.h: 108
try:
    ESL_LIB_PAWR_MIN_NUM_RESPONSE_SLOTS = 0x01
except:
    pass

# esl_lib.h: 109
try:
    ESL_LIB_PAWR_MAX_NUM_RESPONSE_SLOTS = 0xff
except:
    pass

# esl_lib.h: 112
try:
    ESL_LIB_PAWR_UNSPECIFIED_SUBEVENT = 0xff
except:
    pass

# esl_lib.h: 119
try:
    ESL_LIB_CONN_INTERVAL_MIN_DEFAULT = (ESL_LIB_PAWR_SUBEVENT_INTERVAL_DEFAULT / 3)
except:
    pass

# esl_lib.h: 121
try:
    ESL_LIB_CONN_INTERVAL_MAX_DEFAULT = ESL_LIB_CONN_INTERVAL_MIN_DEFAULT
except:
    pass

# esl_lib.h: 125
try:
    ESL_LIB_CONN_PERIPHERAL_LATENCY_DEFAULT = 0
except:
    pass

# esl_lib.h: 128
try:
    ESL_LIB_CONN_TIMEOUT_DEFAULT = (((1 + ESL_LIB_CONN_PERIPHERAL_LATENCY_DEFAULT) * ESL_LIB_CONN_INTERVAL_MAX_DEFAULT) * 8)
except:
    pass

# otf_compressor_config.h: 45
try:
    OTF_CFG_COMPRESSION_LEVEL = 12
except:
    pass

# otf_compressor_config.h: 54
try:
    OTF_CFG_BYTE_WIDTH = 8
except:
    pass

# otf_compressor_config.h: 56
try:
    OTF_CFG_WORD_WIDTH = (2 * OTF_CFG_BYTE_WIDTH)
except:
    pass

# otf_compressor_config.h: 62
try:
    OTF_CFG_PEER_BITS = (OTF_CFG_BYTE_WIDTH - 3)
except:
    pass

# otf_compressor_config.h: 63
try:
    OTF_CFG_PEER_MIN = (OTF_CFG_BYTE_WIDTH - 5)
except:
    pass

# otf_compressor_config.h: 64
try:
    OTF_CFG_PEER_MAX = ((1 << OTF_CFG_PEER_BITS) + (OTF_CFG_PEER_MIN - 1))
except:
    pass

# otf_compressor_config.h: 65
try:
    OTF_CFG_OFFSET_MASK = ((1 << (OTF_CFG_WORD_WIDTH - OTF_CFG_PEER_BITS)) - 1)
except:
    pass

# otf_compressor_config.h: 66
try:
    OTF_CFG_PATTERN_MASK_INIT = (1 << (OTF_CFG_BYTE_WIDTH - 1))
except:
    pass

# otf_compressor_config.h: 67
try:
    OTF_CFG_PATTERN_MASK_LIMIT = (1 << OTF_CFG_BYTE_WIDTH)
except:
    pass

# otf_compressor_config.h: 68
try:
    OTF_CFG_PATTERN_SHIFT_COUNT = (OTF_CFG_BYTE_WIDTH - OTF_CFG_PEER_BITS)
except:
    pass

# otf_compressor_config.h: 69
try:
    OTF_CFG_HASH_WINDOW_SIZE = (1 << OTF_CFG_COMPRESSION_LEVEL)
except:
    pass

esl_lib_address_s = struct_esl_lib_address_s# esl_lib.h: 212

esl_lib_pawr_subevent_s = struct_esl_lib_pawr_subevent_s# esl_lib.h: 217

esl_lib_node_id_id_u = union_esl_lib_node_id_id_u# esl_lib.h: 222

esl_lib_node_id_s = struct_esl_lib_node_id_s# esl_lib.h: 227

esl_lib_array_s = struct_esl_lib_array_s# esl_lib.h: 233

esl_lib_long_array_s = struct_esl_lib_long_array_s# esl_lib.h: 239

esl_lib_pawr_config_adv_interval_s = struct_esl_lib_pawr_config_adv_interval_s# esl_lib.h: 243

esl_lib_pawr_config_subevent_s = struct_esl_lib_pawr_config_subevent_s# esl_lib.h: 247

esl_lib_pawr_config_response_slot_s = struct_esl_lib_pawr_config_response_slot_s# esl_lib.h: 251

esl_lib_pawr_config_s = struct_esl_lib_pawr_config_s# esl_lib.h: 257

esl_lib_scan_parameters_s = struct_esl_lib_scan_parameters_s# esl_lib.h: 275

esl_lib_tlv_s = struct_esl_lib_tlv_s# esl_lib.h: 386

esl_lib_ots_features_s = struct_esl_lib_ots_features_s# esl_lib.h: 392

esl_lib_gattdb_handles_services_s = struct_esl_lib_gattdb_handles_services_s# esl_lib.h: 419

esl_lib_gattdb_handles_s = struct_esl_lib_gattdb_handles_s# esl_lib.h: 427

esl_lib_evt_system_boot_s = struct_esl_lib_evt_system_boot_s# esl_lib.h: 437

esl_lib_evt_connection_mode_s = struct_esl_lib_evt_connection_mode_s# esl_lib.h: 445

esl_lib_evt_scan_status_s = struct_esl_lib_evt_scan_status_s# esl_lib.h: 452

esl_lib_evt_tag_found_s = struct_esl_lib_evt_tag_found_s# esl_lib.h: 458

esl_lib_evt_tag_info_s = struct_esl_lib_evt_tag_info_s# esl_lib.h: 464

esl_lib_evt_tag_config_response_s = struct_esl_lib_evt_tag_config_response_s# esl_lib.h: 471

esl_lib_evt_control_point_response_s = struct_esl_lib_evt_control_point_response_s# esl_lib.h: 478

esl_lib_evt_control_point_notification_s = struct_esl_lib_evt_control_point_notification_s# esl_lib.h: 484

esl_lib_evt_connection_retry_s = struct_esl_lib_evt_connection_retry_s# esl_lib.h: 493

esl_lib_evt_connection_closed_s = struct_esl_lib_evt_connection_closed_s# esl_lib.h: 500

esl_lib_evt_connection_opened_s = struct_esl_lib_evt_connection_opened_s# esl_lib.h: 508

esl_lib_evt_bonding_data_s = struct_esl_lib_evt_bonding_data_s# esl_lib.h: 515

esl_lib_evt_bonding_finished_s = struct_esl_lib_evt_bonding_finished_s# esl_lib.h: 521

esl_lib_evt_pawr_status_s = struct_esl_lib_evt_pawr_status_s# esl_lib.h: 529

esl_lib_evt_image_transfer_finished_s = struct_esl_lib_evt_image_transfer_finished_s# esl_lib.h: 536

esl_lib_evt_image_type_s = struct_esl_lib_evt_image_type_s# esl_lib.h: 543

esl_lib_evt_pawr_response_s = struct_esl_lib_evt_pawr_response_s# esl_lib.h: 551

esl_lib_evt_pawr_data_request_s = struct_esl_lib_evt_pawr_data_request_s# esl_lib.h: 558

esl_lib_evt_ots_return_type_s = struct_esl_lib_evt_ots_return_type_s# esl_lib.h: 566

esl_lib_status_data_u = union_esl_lib_status_data_u# esl_lib.h: 572

esl_lib_evt_error_s = struct_esl_lib_evt_error_s# esl_lib.h: 580

esl_lib_evt_data_u = union_esl_lib_evt_data_u# esl_lib.h: 604

# No inserted files

# No prefix-stripping
