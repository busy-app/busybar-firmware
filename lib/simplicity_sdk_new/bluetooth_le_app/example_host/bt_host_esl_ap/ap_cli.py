"""
ESL AP CLI.
"""

# Copyright 2023 Silicon Laboratories Inc. www.silabs.com
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

import cmd
from shlex import split as lexical_split
import queue
import re
import threading
import time
import argparse
import struct
import textwrap
from datetime import datetime as dt
from ap_logger import getLogger, log, setLogLevel, LEVELS, logLevelName
from ap_core import AccessPoint
from ap_config import IOP_TEST, BLOCKING_WAIT_TIMEOUT
from ap_constants import (
    LED_PATTERN_LENGTH,
    LED_DEFAULT_GAMUT,
    LED_DEFAULT_PATTERN,
    LED_DEFAULT_PERIOD,
    LED_DEFAULT_DURATION,
    PA_INTERVAL_ABS_MIN,
    PA_INTERVAL_ABS_MAX,
    PA_RESPONSE_SLOT_NUMBERS_MIN,
    PA_RESPONSE_SLOT_NUMBERS_MAX,
    PA_RESPONSE_SLOT_SPACING_MIN,
    PA_RESPONSE_SLOT_SPACING_MAX,
    PA_RESPONSE_SLOT_DELAY_MIN,
    PA_RESPONSE_SLOT_DELAY_MAX,
    PA_SUBEVENT_INTERVAL_MIN,
    PA_SUBEVENT_INTERVAL_MAX,
    PA_SUBEVENT_MIN,
    PA_SUBEVENT_MAX,
    ADDRESS_TYPE_PUBLIC_ADDRESS,
    ADDRESS_TYPE_STATIC_ADDRESS,
    VALID_ESL_ID_NUMBER_REGEX,
    VALID_GROUP_ID_NUMBER_REGEX,
    VALID_BD_ADDRESS_REGEX,
)
from esl_lib import Address, EventType, EVENT_PREFIX
from esl_lib_wrapper import (
    ESL_LIB_CONNECTION_MODE_SINGLE,
    ESL_LIB_CONNECTION_MODE_LIST,
)

# Known event names for script wait (lowercase, no prefix; matches EventDispatcher.notify)
KNOWN_EVENT_NAMES = {et.name for et in EventType.all()}

def clamp(n, minn, maxn):
    return max(min(maxn, n), minn)


def ble_address_type(arg_value):
    pat = re.compile(r"(" + VALID_BD_ADDRESS_REGEX + ")")
    if not pat.match(arg_value):
        raise argparse.ArgumentTypeError("Invalid Bluetooth address type.")
    return arg_value


def ble_address_type_all(arg_value):
    pat = re.compile(r"(" + VALID_BD_ADDRESS_REGEX + "|all)")
    if not pat.match(arg_value):
        raise argparse.ArgumentTypeError("Not a valid Bluetooth address.")
    return arg_value


def address_type(arg_value):
    pat = re.compile(
        r"^("
            + VALID_ESL_ID_NUMBER_REGEX
            + r"|"
            + VALID_BD_ADDRESS_REGEX
            + r"|all"
            + r"|\s*"
        + r")$"
    )
    if not pat.match(arg_value):
        raise argparse.ArgumentTypeError("Invalid address type.")
    return arg_value


def esl_id_type(arg_value):
    pat = re.compile(r"(" + VALID_ESL_ID_NUMBER_REGEX + "|all)")
    if not pat.match(arg_value):
        raise argparse.ArgumentTypeError("Invalid ESL ID value. Please select from the allowed range of 0 to 254, or you may use 'all' as a substitute for 255 in some contexts.")
    return arg_value


def esl_group_id_type(arg_value):
    re_str = VALID_GROUP_ID_NUMBER_REGEX if not IOP_TEST else VALID_ESL_ID_NUMBER_REGEX # IOP_TEST mode allows full <u8> range for RFU bit tests 
    pat = re.compile(r"(" + re_str + ")")
    if not pat.match(arg_value):
        raise argparse.ArgumentTypeError("Invalid ESL Group ID. Please select from the allowed range of 0 to 127!")
    return int(arg_value)


def event_type(arg_value):
    """Validate and normalize event name for script wait (e.g. connection_opened, tag_found).
    Accepts any case and optional EVENT_PREFIX (e.g. ESL_LIB_EVT_ or esl_lib_evt_).
    """
    if not arg_value or not str(arg_value).strip():
        raise argparse.ArgumentTypeError("Event name must not be empty.")
    normalized = str(arg_value).strip().lower()
    prefix_lower = EVENT_PREFIX.lower()
    if normalized.startswith(prefix_lower):
        normalized = normalized[len(prefix_lower):]
    if normalized not in KNOWN_EVENT_NAMES:
        raise argparse.ArgumentTypeError(
            "Invalid event name: '%s'. Known events: %s."
            % (arg_value, ", ".join(sorted(KNOWN_EVENT_NAMES)))
        )
    return normalized


def time_type(arg_value):
    try:
        if "." in arg_value:
            arg_value = dt.strptime(arg_value, "%H:%M:%S.%f")
        else:
            arg_value = dt.strptime(arg_value, "%H:%M:%S")
        return arg_value
    except (
        TypeError,
        ValueError,
    ) as exc:  # strptime can cause type or value error - exception chaining
        raise argparse.ArgumentTypeError(
            "Invalid argument [time=<hh:mm:ss[.f]>]: the execution time of the command must be in hour:min:sec[.fraction] format."
        ) from exc


def date_type(arg_value):
    try:
        arg_value = dt.strptime(arg_value, "%Y-%m-%d")
        return arg_value
    except (
        ValueError,
        TypeError,
    ) as exc:  # strptime can cause type or value error - exception chaining
        raise argparse.ArgumentTypeError(
            "Invalid argument [date=<YYYY-MM-DD>]: the execution date of the command in ISO-8601 format."
        ) from exc


def data_type(arg_value):
    pat = re.compile(r"((0(?i)[x])?(?i)[0-9a-f]{1,32})")
    if not pat.match(arg_value):
        raise argparse.ArgumentTypeError("Invalid data type for vendor opcode command.")
    return arg_value


def split_sequence(sequence, sep):
    chunk = []
    for val in sequence:
        if val == sep:
            yield chunk
            chunk = []
        else:
            chunk.append(val)
    yield chunk


class ArgumentParser(argparse.ArgumentParser):
    """ArgumentParser with custom help message."""

    def __init__(self, *args, add_help=None, **kwargs):
        super().__init__(*args, add_help=add_help, **kwargs)
        if add_help is None:
            self.add_argument(
                "-h",
                "--help",
                action="help",
                default=argparse.SUPPRESS,
                help="Show this help message.",
            )


class CliProcessor(cmd.Cmd):
    """CLI Processor"""

    def __init__(self, ap: AccessPoint):
        super().__init__()
        self.queue = queue.Queue()
        self.ap = ap
        self.record_file = None

        # Parsers
        self.command_parser = ArgumentParser(usage=argparse.SUPPRESS)
        self.subparsers = self.command_parser.add_subparsers()
        self.parser_exit = self.subparsers.add_parser(
            "exit",
            description=self.do_exit.__doc__,
        )
        self.parser_foreach = self.subparsers.add_parser(
            "foreach_group",
            description=self.do_foreach_group.__doc__,
            formatter_class=lambda prog: argparse.RawTextHelpFormatter(
                prog, max_help_position=30
            ),
            add_help=False,
            usage=" foreach_group <command> [; commands]",
            epilog="""
        Disclaimer: This is an experimental feature, so please use with caution!
                    Please do not use it with commands that don't take the '--group' optional argument.
                """,
        )
        self.parser_help = self.subparsers.add_parser(
            "help",
            formatter_class=lambda prog: argparse.RawTextHelpFormatter(
                prog, max_help_position=30
            ),
            description="""
        List available commands with 'help' or detailed help with 'help <command>'.
            """,
            add_help=False,
            usage=" help [cmd]",
        )
        self.arg_demo()
        self.arg_mode()
        self.arg_network()
        self.arg_scan()
        self.arg_config()
        self.arg_disconnect()
        self.arg_connect()
        self.arg_sync()
        self.arg_led()
        self.arg_list()
        self.arg_image_update()
        self.arg_unassociate()
        self.arg_read_sensor()
        self.arg_display_image()
        self.arg_factory_reset()
        self.arg_delete_timed()
        self.arg_refresh_display()
        self.arg_ping()
        self.arg_vendor_opcode()
        self.arg_service_reset()
        self.arg_set_rssi_threshold()
        self.arg_script()
        self.arg_update_complete()
        self.arg_verbosity()

        # LED control defaults
        self.led_pattern = bytearray(LED_DEFAULT_PATTERN)
        self.led_gamut = LED_DEFAULT_GAMUT
        self.led_period = bytearray(LED_DEFAULT_PERIOD)
        self.led_repeats = bytearray(LED_DEFAULT_DURATION)
        self.led_index = 0
        self.prompt = ""

    # Logger
    @property
    def log(self):
        return getLogger("CLI")

    def onecmd(self, line):
        """Interpret the argument as though it had been typed in response
        to the prompt.

        Overwrite this function to parse the whole line.
        The line includes the command itself, which will be interpreted by subparsers.

        """
        cmd, arg, line = self.parseline(line)
        if not line:
            return self.emptyline()
        if cmd is None:
            return self.default(line)
        self.lastcmd = line
        if line == "EOF":
            self.lastcmd = ""
        if cmd == "":
            return self.default(line)
        if cmd == self.parser_help.prog.strip():
            return self.subparser_help(arg)
        else:
            try:
                func = getattr(self, "do_" + cmd)
                lexical_splitted_command = lexical_split(line)
                args, unknown = self.command_parser.parse_known_args(
                    lexical_splitted_command
                )
                if unknown:
                    self.log.error("Unknown argument '%s' for command %s", unknown, cmd)
                else:
                    return func(args)
            except AttributeError as e:
                self.log.error("Command error: %s", cmd)
                self.log.debug(e)
            except Exception as e:
                self.log.error(e)
            except SystemExit:
                # Argparse error generates SystemExit for missing positional arguments
                pass

    def poll_input(self):
        """Provide input to the command queue from the stdin"""
        while True:
            try:
                line = input(self.prompt)
            except EOFError:
                continue
            self.queue.put(line)

    def loop(self):
        """Thread safe queue based version original command loop"""
        self.preloop()
        try:
            import readline

            readline.set_completer(self.complete)
            readline.parse_and_bind(self.completekey + ": complete")
        except ImportError:
            self.log.debug("Auto complete not supported.")
        threading.Thread(target=self.poll_input, daemon=True).start()
        stop = None
        foreach = None
        while stop != True:
            try:
                line = self.queue.get(timeout=BLOCKING_WAIT_TIMEOUT)
                foreach_cmd = self.parser_foreach.prog.strip() + " "
                if line.startswith(foreach_cmd):
                    foreach = self.do_foreach_group()
                    line = line.partition(foreach_cmd)[2]
                else:
                    foreach = None
            except queue.Empty:
                continue
            # Following preprocessing steps make the command chaining available by detecting semicolon as a command separator
            try:
                lexical_splitted_input = lexical_split(line)
            except Exception as e:
                self.log.error(e)
                continue

            input_groups = list(split_sequence(lexical_splitted_input, ";"))
            # Now we have a list of lists - each element (sub-list) in this list is a single command with its arguments
            for single_input in input_groups:
                # Check for empty commands in the chain, ignore if any - also stop on exit command
                if len(single_input) and not stop:
                    # Original cmd class expects string input instead of list - our inherited class expects the same
                    stringline = ""
                    # So now we convert each sublist to an appropriate string...
                    for e in single_input:
                        # ...where 'appropriate' means that if a list element contains space or any escape sequence then we need to put into quotes!
                        if e.count("'"):
                            stringline += '"{0}"'.format(
                                e if not e.count('"') else e.replace('"', '\\"')
                            )
                        else:
                            stringline += (
                                e
                                if not any(c in e for c in ' \\"')
                                else "'{0}'".format(e)
                            )
                        stringline += " "
                    if foreach is None:
                        stringline = self.precmd(stringline)
                        stop = self.onecmd(stringline)
                        stop = self.postcmd(stop, stringline)
                    elif len(foreach) != 0:
                        for i in foreach:
                            group = f"-g {i}"
                            gstringline = self.precmd(stringline + group)
                            print(gstringline)
                            stop = self.onecmd(gstringline)
                            stop = self.postcmd(stop, gstringline)
                    else:
                        self.log.warning(
                            "There's no group to send commands to, command ignored."
                        )

        self.postloop()

    def arg_demo(self):
        parser_demo = self.subparsers.add_parser(
            "demo",
            description=self.do_demo.__doc__,
            epilog="""
        Note: You can obtain the current status of the demo mode by omitting the choice.""",
        )
        parser_demo.add_argument(
            "choice",
            nargs="?",
            choices=["on", "off"],
            help="Turn AP advertising on or off for ESL Demo in Simplicity Connect mobile app",
        )

    def do_demo(self, arg):
        """
        Control the built-in advertising feature of the ESL NCP AP target for the ESL demo in the Simplicity Connect mobile application.
        """
        if arg.choice == "on":
            self.ap.ap_adv_start()
        elif arg.choice == "off":
            self.ap.ap_adv_stop()
        else:
            self.ap.ap_demo_status()

    def arg_scan(self):
        parser_scan = self.subparsers.add_parser(
            "scan",
            description=self.do_scan.__doc__,
            epilog="""
        Note: You can obtain the current status of the scanning by omitting the choice.""",
        )
        parser_scan.add_argument(
            "choice",
            nargs="?",
            choices=["start", "stop"],
            help="Control AP scanning to detect or ignore nearby advertiser ESL devices.",
        )
        parser_scan.add_argument(
            "--active",
            "-a",
            action="store_true",
            help="Start active scan instead of default passive type.",
        )

    def do_scan(self, arg):
        """
        Start or stop scanning for advertising ESL devices.
        """
        active_scan = False
        scan_enable = None
        if arg.choice == "start":
            if arg.active:
                active_scan = True
            scan_enable = True
        elif arg.choice == "stop":
            if arg.active:
                self.log.info("The active scan option will be ignored during stop.")
            scan_enable = False

        self.ap.ap_scan(scan_enable, active_scan)

    def arg_connect(self):
        parser_connect = self.subparsers.add_parser(
            "connect",
            formatter_class=lambda prog: argparse.RawTextHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_connect.__doc__,
            epilog="""
        Notes: <esl_id> and <group_id> can be used instead of <bt_addr> if ESL is already configured. <address_type>
               will be taken into account only if the given <bt_addr> is unknown - otherwise the proper type reported
               by the remote device will be used. If the group ID is not given after the ESL ID then the default value
               group zero is used. This applies to many commands expecting the group ID as optional parameter.

               The 'all' keyword can be used with a special meaning with 'connect' command: it will try to connect to
               all advertiser ESLs (within the 'group_id' if it is given or to any advertisers if it isn't) up to the
               the maximum number of simultaneous connections supported by the current build of the ESL library and
               the attached Network Co-Processor embedded controller.
               """,
        )
        parser_connect.add_argument(
            "address",
            nargs="?",
            type=address_type,
            help="Bluetooth address (e.g. 'AA:BB:CC:DD:EE:22') in case insensitive format or ESL ID of the tag.",
        )
        parser_connect.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )
        parser_connect.add_argument(
            "--addr_type",
            "-t",
            metavar="",
            choices=["public", "static"],
            help=textwrap.dedent(
                """[address_type]: ESL address type (optional), possible values:
        - public:      Public device address (default assumption)
        - static:      Random static device address"""
            ),
        )

    def do_connect(self, arg):
        """
        Connect to one or more ESL devices.
        """
        group_id = arg.group_id
        bt_addr = None
        esl_id = None
        address_type = None
        if arg.addr_type == "public":
            address_type = ADDRESS_TYPE_PUBLIC_ADDRESS
        elif arg.addr_type == "static":
            address_type = ADDRESS_TYPE_STATIC_ADDRESS

        if arg.address is not None:
            if arg.address.isnumeric():
                esl_id = int(arg.address)
                if arg.addr_type is not None:
                    address_type = None
                    self.log.warning(
                        "Explicit address type ignored for already configured ESLs - correct type must be known and will be used instead!"
                    )
            else:
                try:
                    bt_addr = ble_address_type(arg.address.lower())
                except:
                    esl_id = str(arg.address)  # it must be 'all', then

        self.ap.ap_connect(esl_id, bt_addr, group_id, address_type)

    def arg_disconnect(self):
        parser_disconnect = self.subparsers.add_parser(
            "disconnect",
            formatter_class=lambda prog: argparse.RawTextHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_disconnect.__doc__,
            epilog="""
        Notes: If no address is specified, the default active connection is closed - if only one exists.

               To close more existing connections at once, you can use the 'disconnect all' command.
               """,
        )
        parser_disconnect.add_argument(
            "address",
            nargs="?",
            metavar="<addr>",
            type=address_type,
            help="""Bluetooth address (e.g. 'AA:BB:CC:DD:EE:22') in case insensitive format or ESL ID of the tag.""",
        )
        parser_disconnect.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_disconnect(self, arg):
        """
        Initiate the Periodic Advertisement Sync Transfer process if PAwR train is
        available then disconnect from an ESL device with the specified address.
        """
        group_id = arg.group_id
        bt_addr = None
        esl_id = None
        if arg.address is not None:
            if arg.address.isnumeric():
                esl_id = int(arg.address)
            else:
                try:
                    bt_addr = ble_address_type(arg.address.lower())
                except:
                    esl_id = str(arg.address)  # it must be 'all', then
        self.ap.ap_disconnect(esl_id, bt_addr, group_id)

    def arg_list(self):
        parser_list = self.subparsers.add_parser(
            "list",
            formatter_class=lambda prog: argparse.RawTextHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_list.__doc__,
            epilog="""
        Examples: list a --verbose
                  list synchronized -v
        Note: To reset the content of advertising and blocked lists you may want to issue a <scan start>
        command at any time.""",
        )
        exclusive_group = parser_list.add_mutually_exclusive_group()
        parser_list.add_argument(
            "state",
            nargs="+",
            metavar="state",
            choices=[
                "advertising",
                "a",
                "blocked",
                "b",
                "connected",
                "c",
                "initiating",
                "i",
                "synchronized",
                "s",
                "unsynchronized",
                "u",
            ],
            help=textwrap.dedent(
                """
        <advertising, a>:    List devices that are advertising ESL Service UUID
        <blocked, b>:        List blocked devices, see reasoning by adding -v
        <connected, c>:      List connected ESL information
        <initiating, i>:     List devices that are in connection initiation queue
        <synchronized, s>:   List synchronized ESL information
        <unsynchronized, u>: List unsynchronized ESL information"""
            ),
        )
        exclusive_group.add_argument(
            "--verbose",
            "-v",
            action="store_true",
            help="List more detailed information, mutually exclusive with 'number' param.",
        )
        exclusive_group.add_argument(
            "--number",
            "-n",
            action="store_true",
            help="Show only the number of devices in given state(s), mutually exclusive with 'verbose' param.",
        )
        parser_list.add_argument(
            "--group_id", "-g", metavar="<u7>", type=int, help="ESL group ID (optional)"
        )

    def do_list(self, arg):
        """
        List details about known devices by state.
        """
        param_list = []
        verbosity_level = None  # default, will print a list of addresses
        if arg.verbose:
            verbosity_level = (
                True  # increased verbosity, prints some useful additional information
            )
        elif arg.number:
            verbosity_level = (
                False  # decreased verbosity, prints element count in the list, only
            )
        group_id = arg.group_id
        if any(i in arg.state for i in ["advertising", "a"]):
            param_list.append("advertising")
        if any(i in arg.state for i in ["blocked", "b"]):
            param_list.append("blocked")
        if any(i in arg.state for i in ["connected", "c"]):
            param_list.append("connected")
        if any(i in arg.state for i in ["initiating", "i"]):
            param_list.append("initiating")
        if any(i in arg.state for i in ["synchronized", "s"]):
            param_list.append("synchronized")
        if any(i in arg.state for i in ["unsynchronized", "u"]):
            param_list.append("unsynchronized")
        self.ap.ap_list(param_list, verbosity_level, group_id)

    def arg_led(self):
        parser_led = self.subparsers.add_parser(
            "led",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=45
            ),
            description=self.do_led.__doc__,
            epilog="""
        Notes: Almost all of the optional led control parameters are "sticky", meaning that the last values are
               preserved by the AP internally and will be re-used next time, if the given parameter is omitted in the
               argument list. This doesn't apply on the delay, time and absolute parameters, though.""",
        )
        group_led_delay_absolute = parser_led.add_mutually_exclusive_group()
        group_led_repeats_duration = parser_led.add_mutually_exclusive_group()
        parser_led.add_argument(
            "choice", choices=["on", "off", "flash"]
        ).help = "Turn LED on/off or flash the LED"
        parser_led.add_argument("esl_id", type=esl_id_type, help="ESL ID or all")
        parser_led.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )
        parser_led.add_argument(
            "--default",
            "-d",
            action="store_true",
            help="Restore the default flashing pattern built-in with AP",
        )
        parser_led.add_argument(
            "--pattern",
            "-p",
            help="A string containing either '1's or '0's, max length: 40",
            metavar="<bits>",
        )
        parser_led.add_argument(
            "--on_period",
            "-on",
            type=int,
            help="""Integer value from 1 to 255, meaning 'delay *2ms' for on state bits of the pattern. '0' is prohibited""",
            metavar="<u8>",
        )
        parser_led.add_argument(
            "--off_period",
            "-of",
            type=int,
            help="""Integer value from 1 to 255, meaning 'delay *2ms' for off state bits of the pattern. '0' is prohibited""",
            metavar="<u8>",
        )
        parser_led.add_argument(
            "--brightness",
            "-b",
            type=int,
            help="Integer value, 4 step brightness from 0 to 3",
            metavar="<int[0,3]>",
        )
        parser_led.add_argument(
            "--color",
            "-c",
            help="Red, green and blue values - only applies to LED with sRGB type",
            metavar="<int[0,3]>",
        )
        group_led_repeats_duration.add_argument(
            "--repeats",
            "-r",
            metavar="<u15>",
            type=int,
            help="""How many times the pattern shall be repeated. Mutually exclusive with 'duration=' param. Value set [1-32767]""",
        )
        group_led_repeats_duration.add_argument(
            "--duration",
            "-dn",
            metavar="<u15>",
            type=int,
            help="""How many seconds the pattern shall be repeated. Mutually exclusive with 'repeats=' param. Value set [1-32767]""",
        )
        parser_led.add_argument(
            "--index",
            "-i",
            metavar="<u8>",
            type=int,
            help="Index of the LED (optional, 0 by default)",
        )
        group_led_delay_absolute.add_argument(
            "--time",
            "-t",
            type=time_type,
            help="""Execution time of the command in hour:min:sec format. Note: If <delay_ms> is specified then it is also added  to the calculated value as an additional delay""",
            metavar="<hh:mm:ss>",
        )
        group_led_delay_absolute.add_argument(
            "--absolute",
            "-a",
            metavar="<u32>",
            type=int,
            help="""Execution time of the command in ESL Absolute Time epoch value. Mutually exclusive with timed delay.""",
        )
        parser_led.add_argument(
            "--date",
            "-dt",
            type=date_type,
            help="Execution date of the command in ISO-8601 format (optional to time, only).",
            metavar="YYYY-MM-DD",
        )
        parser_led.add_argument(
            "--delay", "-dy", metavar="<u32>", type=int, help="Delay in milliseconds"
        )

    def do_led(self, arg):
        """
        Turn on / off or flash an LED utilizing the LED control command.
        """
        now = dt.now()
        delay_ms = 0
        absolute_base = pattern = None
        input_error = False
        absolute_now = self.ap.get_absolute_time()
        index = self.led_index
        repeat_field = bytearray(2)  # initialised to "LED Off"
        period = self.led_period.copy()
        gamut = self.led_gamut
        if arg.choice in ["on", "off"]:
            if any(
                opt
                for opt in [
                    arg.on_period,
                    arg.off_period,
                    arg.pattern,
                    arg.default,
                    arg.repeats,
                    arg.duration,
                ]
            ):
                self.log.warning(
                    "Arguments controlling flashing parameters are ignored for 'on' and 'off' commands!"
                )
            if arg.choice == "on":
                repeat_field[0] |= 1
            elif any(opt for opt in [arg.brightness, arg.color]):
                self.log.warning(
                    "Color and brightness control parameters are useless for 'off' command!"
                )
        elif arg.choice == "flash":
            pattern = self.led_pattern.copy()
        esl_id = arg.esl_id
        group_id = arg.group_id
        if arg.default:
            pattern = bytearray(LED_DEFAULT_PATTERN)
            gamut = LED_DEFAULT_GAMUT
            period = bytearray(LED_DEFAULT_PERIOD)
            repeat_field = bytearray(LED_DEFAULT_DURATION)
        if arg.index is not None:
            index = arg.index
        if arg.brightness is not None:
            brightness = arg.brightness
            if brightness not in range(0, 4):
                self.log.error("Brightness has to be between [0,3], aborting")
                input_error = True
            else:
                gamut = (
                    (int.from_bytes(gamut, byteorder="little") & 0x3F)
                    | (brightness << 6)
                ).to_bytes(1, "little")
        if arg.color is not None:
            r, g, b = struct.unpack("ccc", str(arg.color).encode())
            if any(c < 0 or c > 3 for c in [int(r), int(g), int(b)]):
                self.log.error("Color has to be between [0,3], aborting")
                input_error = True
            else:
                color = (int(b) << 4) | (int(g) << 2) | int(r)
                gamut = (
                    (int.from_bytes(gamut, byteorder="little") & 0xC0) | color
                ).to_bytes(1, "little")
        if arg.delay is not None:
            delay_ms += arg.delay
            absolute_base = absolute_now
        if arg.time is not None:
            set_date_input_time = arg.time
            if not arg.date:
                set_date = dt.combine(dt.today(), set_date_input_time.time())
            else:
                set_date = dt.combine(arg.date, set_date_input_time.time())
            try:
                delay_ms += self.ap.calculate_exec_time(
                    now,
                    set_date.hour,
                    set_date.minute,
                    set_date.second,
                    set_date.microsecond,
                    arg.date,
                )
                absolute_base = absolute_now
            except:
                self.log.error("Requested delay can't be set, command ignored!")
                input_error = True
        if arg.date is not None and not arg.time is not None:
            self.log.error("Date without time cannot be set.")
            input_error = True
        if arg.absolute is not None:
            absolute_base = arg.absolute
            if absolute_base == 0:
                delay_ms = None
        if pattern is not None:
            repeat_field = self.led_repeats
            if arg.on_period is not None:
                on_period = arg.on_period
                low_limit = 1 if not IOP_TEST else 0
                if on_period < low_limit or on_period > 255:
                    self.log.error("LED on period has to be between [1,255], aborting")
                    input_error = True
                else:
                    period[1] = on_period
            if arg.off_period is not None:
                off_period = arg.off_period
                low_limit = 1 if not IOP_TEST else 0
                if off_period < low_limit or off_period > 255:
                    self.log.error("LED off period has to be between [1,255], aborting")
                    input_error = True
                else:
                    period[0] = off_period
            if arg.duration is not None:
                duration = arg.duration
                if (
                    duration <= 0x7FFF
                ):  # and duration in range(1,32767) and len(duration)<=15): # only 15 bits long!
                    repeat_field = bytearray(
                        int((duration << 1) | 1).to_bytes(2, "little")
                    )
                else:
                    self.log.warning(
                        "Duration field value of %u out of bounds, ignored!", duration
                    )
                    input_error = True
            if arg.repeats is not None:
                repeats = arg.repeats
                if repeats <= 0x7FFF:  # only 15 bits long!
                    repeat_field = bytearray(int((repeats << 1)).to_bytes(2, "little"))
                else:
                    self.log.warning(
                        "Repeats field value of %u out of bounds, ignored!", repeats
                    )
                    input_error = True
            if arg.pattern is not None:
                bitstream = arg.pattern
                if (len(bitstream) > LED_PATTERN_LENGTH * 8) or (
                    bitstream.find("1") == -1 and not IOP_TEST
                ):
                    self.log.error(
                        "Pattern length shall not exceed 40 bits and shall comprise of at least one bit set to 1."
                    )
                    input_error = True
                else:
                    pattern = bytearray(
                        int(bitstream, 2).to_bytes(LED_PATTERN_LENGTH, "little")
                    )
            low_limit = 1 if not IOP_TEST else 0
            self.led_pattern = pattern.copy()
            self.led_period = period.copy()
            self.led_repeats = repeat_field.copy()
            pattern += period
        elif (
            IOP_TEST
        ):  # IOP tests requires the pattern and period fields to be able to set for On and Off commands too (off standard behavior!)
            pattern = self.led_pattern.copy()
            pattern += period
            input_error = True

        self.led_gamut = gamut
        self.led_index = index

        if not input_error or IOP_TEST:  # allow erroneous values only in IOP_TEST mode!
            absolute_value = None
            if delay_ms is None:
                absolute_value = absolute_base
            elif absolute_base is not None:
                absolute_value = int(absolute_base + delay_ms) & 0xFFFFFFFF
                if (
                    absolute_value == 0
                ):  # do not send any unsolicited delete on overflow!!
                    absolute_value += 1
            self.ap.ap_led(
                esl_id,
                group_id,
                repeat_field,
                index,
                absolute_value,
                int.from_bytes(gamut, byteorder="little"),
                pattern,
            )
        else:
            self.log.warning("Implausible input error, LED command ignored!")

    def arg_config(self):
        parser_config = self.subparsers.add_parser(
            "config",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_config.__doc__,
            epilog="""
        Notes: Either the option '--full' or at least one of the other optional parameters shall be given.

               The 'all' keyword can be used to configure a number of connected ESLs, but the ESL ID can't be specified
               in turn, as this would make the command ambiguous.
               However, the same ESL group ID can be specified for multiple connected devices - but use this with care,
               as this command doesn't check against existing ESL configurations, so the network may end up broken!
        """,
        )
        parser_config.add_argument(
            "device",
            nargs="?",
            type=ble_address_type_all,
            help="Bluetooth address of the target device (e.g. 'AA:BB:CC:DD:EE:22') in case insensitive format or 'all'",
        )
        parser_config.add_argument(
            "--full",
            "-f",
            action="store_true",
            help="Configure everything in one step. ESL ID and group can be specified to override default values - see notes.",
        )
        parser_config.add_argument(
            "--esl_id",
            "-i",
            metavar="<u8>",
            type=esl_id_type,
            help="New ESL ID of the connected tag.",
        )
        parser_config.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=esl_group_id_type,
            help="ESL group ID (optional, default is group 0)",
        )
        parser_config.add_argument(
            "--sync_key",
            "-sk",
            action="store_true",
            help="Set current Access Point Sync Key Material.",
        )
        parser_config.add_argument(
            "--response_key",
            "-rk",
            action="store_true",
            help="Generate then set new Response Key Material.",
        )
        time_group_parser_config = parser_config.add_mutually_exclusive_group()
        time_group_parser_config.add_argument(
            "--time",
            "-t",
            action="store_true",
            help="Set current Absolute Time of the ESL Access Point.",
        )
        time_group_parser_config.add_argument(
            "--absolute",
            "-a",
            metavar="<u32>",
            type=int,
            help="""Set custom Absolute Time epoch value - use with care! Mutually exclusive with the 'time' parameter.""",
        )

    def do_config(self, arg):
        """
        Configure the writable mandatory GATT characteristics of the ESL tag.
        """
        params = {}
        if arg.device == "all" and arg.esl_id is not None:
            self.log.error(
                "ESL ID can't be specified for 'all' in one step, command ignored!"
            )
            return
        else:
            if arg.full:
                params["full"] = True
            if arg.esl_id is not None:
                params["esl_addr"] = int(arg.esl_id)
            if arg.group_id is not None:
                params["group_id"] = int(arg.group_id)
            if arg.sync_key:
                params["sync_key"] = True
            if arg.response_key:
                params["response_key"] = True
            if arg.time:
                params["time"] = True
            if arg.absolute is not None:
                params["absolute_time"] = arg.absolute

        if params == {}:
            self.log.error(
                "Either the option '--full' or at least one of the optional parameters shall be given!"
            )
        else:
            self.ap.ap_config(params, arg.device)

    def arg_image_update(self):
        parser_image_update = self.subparsers.add_parser(
            "image_update",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=33
            ),
            description=self.do_image_update.__doc__,
            epilog="""
        Notes:    To use space or backslash in the filename or other special characters, such as line break escape
                  sequences in the text caption, please enclose these strings in quotes.
                  The modifiers like rotation, fitting and and labeling are mutually exclusive with raw data input.
                  If the group is specified along with the keyword `all`, then only connected devices in the group will be affected.

        Examples: image_update 0 ./image/banana.png --label=\"Line 1\\nLine 2\"
                  Send an image to index 0 on the single connected ESL with two lines of label.

                  image_update 1 "/user/home/path with space/img.jpg" all
                  Use the 'all' keyword as special address to send the same image to slot 1 on all connected ESLs.

                  image_update 0 *qrcode all
                  To send unique QR codes to all connected tags for use with ESL Demo, enter "*qrcode" instead of a valid image file path.
                  """,
        )
        parser_image_update.add_argument(
            "image_index",
            type=int,
            help="Image storage index of the ESL tag to be updated.",
        )
        parser_image_update.add_argument(
            "imagefile_path",
            type=str,
            help="Relative or full path to the selected image file. Use quotation marks if the path contains spaces.",
        )
        parser_image_update.add_argument(
            "address",
            nargs="?",
            metavar="[address]",
            type=address_type,
            help="Bluetooth address of the target device or ESL ID or 'all' if there are more ESLs connected.",
        )
        parser_image_update.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )
        parser_image_update.add_argument(
            "--label",
            "-l",
            metavar="<str>",
            type=str,
            help="Caption to be written over the image. Use quotation marks if it includes spaces or line breaks.",
        )
        parser_image_update.add_argument(
            "--cropfit",
            "-c",
            action="store_true",
            help="Fit the image to the display proportions by cropping.",
        )
        parser_image_update_converter_group = (
            parser_image_update.add_mutually_exclusive_group()
        )
        parser_image_update_converter_group.add_argument(
            "--raw",
            "-r",
            action="store_true",
            help="Upload raw image file without any conversion.",
        )
        parser_image_update_converter_group.add_argument(
            "--display_index",
            "-d",
            metavar="<u8>",
            type=int,
            help="Try auto-conversion image for this display. Mutually exclusive with '--raw' argument.",
        )
        parser_image_update_rotator_group = (
            parser_image_update.add_mutually_exclusive_group()
        )
        parser_image_update_rotator_group.add_argument(
            "--cw", "-rr", action="store_true", help="Clockwise (right) rotation."
        )
        parser_image_update_rotator_group.add_argument(
            "--ccw",
            "-rl",
            action="store_true",
            help="Counter clockwise (left) rotation.",
        )
        parser_image_update_rotator_group.add_argument(
            "--flip", "-f", action="store_true", help="Turn the image upside down."
        )

    def do_image_update(self, arg):
        """
        Update single image on one or more connected Tags.
        """
        raw_img = input_error = False
        display_index = None
        label = None
        rotation = None
        cropfit = False
        filename = None

        if arg.image_index in range(0, 256):
            image_index = arg.image_index
        else:
            self.log.error("Image index must be between 0 and 255")
            input_error = True
        if (
            arg.imagefile_path != "*qrcode"
        ):  # Check if this is not the specific use case for the ESL demo
            filename = arg.imagefile_path
        if arg.raw:
            raw_img = True
        if arg.display_index is not None:
            display_index = arg.display_index
        if arg.raw and (arg.cw or arg.ccw or arg.flip or arg.label or arg.cropfit):
            self.log.error(
                "Raw input can't be rotated, fitted or labelled - command ignored!"
            )
            input_error = True
        if arg.cw:
            rotation = 270
        if arg.ccw:
            rotation = 90
        if arg.flip:
            rotation = 180
        if arg.cropfit:
            cropfit = True
        if arg.label:
            # arg.label is a raw string: decode escapes before passing to ap_imageupdate!
            label = arg.label.encode().decode("unicode-escape")

        if not input_error:
            self.ap.ap_imageupdate(
                image_index,
                filename,
                raw_img,
                display_index,
                label,
                rotation,
                cropfit,
                arg.address,
                arg.group_id,
            )

    def arg_unassociate(self):
        parser_unassociate = self.subparsers.add_parser(
            "unassociate",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_unassociate.__doc__,
            epilog="Note: the keyword 'all' can be used as a substitute for the ESL broadcast address (0xff)",
        )
        parser_unassociate.add_argument(
            "address",
            type=address_type,
            help="Bluetooth address (e.g. 'AA:BB:CC:DD:EE:22') in case insensitive format or ESL ID of the tag.",
        )
        parser_unassociate.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_unassociate(self, arg):
        """
        Unassociate ESL(s) from AP.
        """
        if arg.address.isnumeric():
            address = arg.address
        else:
            address = arg.address.lower()
        group_id = arg.group_id
        self.ap.ap_unassociate(address, group_id)

    def arg_mode(self):
        parser_mode = self.subparsers.add_parser(
            "mode",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=20
            ),
            description=self.do_mode.__doc__,
            epilog="""
        Note:       To check current mode you can issue the command without argument.
        Disclaimer: Please also note that manual mode gives you full control over the devices on your network, allowing you to
                    easily violate the Profile rules (e.g. setting the same ESL ID and Group ID on two different devices at
                    the same time), so use this option with caution!
        """,
        )

        parser_mode.add_argument(
            "ap_mode",
            nargs="?",
            choices=["auto", "manual"],
            help="Toggle between automatic and manual mode of AP operation. ",
        )

        parser_mode.add_argument(
            "lib_mode",
            nargs="?",
            choices=["single", "list"],
            help="Toggle ESL library connection initiation behavior between single or list based.",
        )

    def do_mode(self, arg):
        """
        Changes the operation mode of the ESL Access Point and the connection initiation method of the underlying ESL library.
        Automatic mode essentially refers to a set of actions that are performed automatically according to the ESL Profile
        specification, such as automatically configuring and synchronizing newly discovered ESLs with the network.
        The connection initiation method of the library selects whether a new connection is initiated to a single target
        at a time, or to one of many devices that are first added to the Filter Accept List, then initiated using the
        Initiator Filter Policy Core feature. (This doesn't apply to the connection requests initiated through PAwR.)
        """
        if arg.ap_mode == "auto":
            arg.ap_mode = True
        elif arg.ap_mode == "manual":
            arg.ap_mode = False
        if arg.lib_mode == "single":
            arg.lib_mode = ESL_LIB_CONNECTION_MODE_SINGLE
        elif arg.lib_mode == "list":
            arg.lib_mode = ESL_LIB_CONNECTION_MODE_LIST
        self.ap.ap_mode(arg.ap_mode, arg.lib_mode)

    def arg_network(self):
        parser_config = self.subparsers.add_parser(
            "network",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_network.__doc__,
            epilog="""
        Notes:      To check current network exclusivity mode, issue the command without arguments.
                    If both '--save' and '--load' requests are issued in the same command, the file export operation is done first, regardless of the sequence of arguments.
        Disclaimer: Please also note that importing a pre-defined network configuration from a JSON file gives you full control over the devices on your network in auto (aka ESL profile) mode, potentially overriding your previous / running network configurations, so use this option with caution!
        """,
        )
        parser_config.add_argument(
            "--save",
            "-s",
            nargs="?",
            type=str,
            metavar="FILE",
            help="Export the current network configuration (Bluetooth and ESL addresses, grouped) to a JSON formatted file. Use quotation marks if the path contains spaces.",
        )
        parser_config.add_argument(
            "--load",
            "-l",
            nargs="?",
            type=str,
            metavar="FILE",
            help="Import network configuration in JSON format (e.g. from a previously exported backup)",
        )
        parser_config.add_argument(
            "--exclusive",
            "-e",
            choices=["no", "yes"],
            help="Exclusive mode allows only preconfigured devices to synchronize (otherwise any ESL detected within RSSI limits)",
        )

    def do_network(self, arg):
        """
        Execute commands related to the network control.
        """
        exclusive = None
        if arg.exclusive is not None:
            exclusive = True if arg.exclusive == "yes" else False
        self.ap.ap_network(arg.save, arg.load, exclusive)
        pass

    def arg_read_sensor(self):
        parser_read_sensor = self.subparsers.add_parser(
            "read_sensor",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=35
            ),
            description=self.do_read_sensor.__doc__,
        )
        parser_read_sensor.add_argument(
            "esl_id", type=int, help="ESL ID"
        )  # esl_id is int because all is not accepted
        parser_read_sensor.add_argument("sensor_index", type=int, help="Sensor index.")
        parser_read_sensor.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_read_sensor(self, arg):
        """
        Read sensor information.
        """
        esl_id = arg.esl_id
        group_id = arg.group_id
        if arg.sensor_index in range(0, 256):
            sensor_idx = arg.sensor_index
            self.ap.ap_read_sensor(esl_id, group_id, sensor_idx)
        else:
            self.log.warning("Sensor index must be between 0 and 255")

    def arg_factory_reset(self):
        parser_factory_reset = self.subparsers.add_parser(
            "factory_reset",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_factory_reset.__doc__,
            epilog="""
        Note: the keyword 'all' can be used as a substitute for the ESL broadcast address (0xff)""",
        )
        parser_factory_reset.add_argument(
            "address",
            type=address_type,
            help="Bluetooth address (e.g. 'AA:BB:CC:DD:EE:22') in case insensitive format or ESL ID of the tag.",
        )
        parser_factory_reset.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )
        parser_factory_reset.add_argument(
            "--pawr",
            "-p",
            action="store_true",
            help="""Force command through PAwR sync train even if the addressed ESL is currently connected""",
        )

    def do_factory_reset(self, arg):
        """
        Execute factory reset on tag.
        """
        force_pawr = arg.pawr
        group_id = arg.group_id
        address = None

        if arg.address.isnumeric():
            address = int(arg.address)
        else:
            address = arg.address.lower()
        self.ap.ap_factory_reset(address, group_id, force_pawr)

    def arg_delete_timed(self):
        parser_delete_timed = self.subparsers.add_parser(
            "delete_timed",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_delete_timed.__doc__,
        )
        parser_delete_timed.add_argument(
            "led_display",
            choices=["led", "display"],
            help="Delete timed led or display_image command.",
        )
        parser_delete_timed.add_argument(
            "esl_id", type=esl_id_type, help="ESL ID or all"
        )
        parser_delete_timed.add_argument(
            "index", type=int, help="Index of the LED or the display"
        )
        parser_delete_timed.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_delete_timed(self, arg):
        """
        Delete a delayed command of an ESL Tag peripheral with the selected index.
        """
        esl_id = arg.esl_id
        device_index = arg.index
        group_id = arg.group_id
        if arg.led_display == "led":
            self.ap.ap_led(esl_id, group_id, bytearray(2), device_index, 0)
        else:
            self.ap.ap_display_image(esl_id, group_id, 0, device_index, 0)

    def arg_refresh_display(self):
        parser_refresh_display = self.subparsers.add_parser(
            "refresh_display",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=35
            ),
            description=self.do_refresh_display.__doc__,
        )
        parser_refresh_display.add_argument(
            "esl_id", type=esl_id_type, help="ESL ID or all"
        )
        parser_refresh_display.add_argument(
            "display_index", type=int, help="Display index"
        )
        parser_refresh_display.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_refresh_display(self, arg):
        """
        Refresh tag display.
        """
        esl_id = arg.esl_id
        group_id = arg.group_id
        if arg.display_index in range(0, 256):
            display_index = arg.display_index
            self.ap.ap_refresh_display(esl_id, group_id, display_index)
        else:
            self.log.warning("Display index must be between 0 and 255")

    def arg_display_image(self):
        parser_display_image = self.subparsers.add_parser(
            "display_image",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=40
            ),
            description=self.do_display_image.__doc__,
            epilog="""
        Note: Timed display commands with a delay shorter than the actual periodic advertisement interval may be
              rejected on receive by Implausible Absolute Time (0x0C) ESL error response.""",
        )
        group_display_image_delay_absolute = (
            parser_display_image.add_mutually_exclusive_group()
        )
        parser_display_image.add_argument(
            "esl_id", type=esl_id_type, help="ESL ID or all"
        )
        parser_display_image.add_argument(
            "image_index", type=int, help="Image index to update."
        )
        parser_display_image.add_argument(
            "display_index", type=int, help="Display index"
        )
        parser_display_image.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )
        group_display_image_delay_absolute.add_argument(
            "--time",
            "-t",
            metavar="<hh:mm:ss>",
            type=time_type,
            help="""Execution time of the command in hour:min:sec format. (optional) Note: If <--delay> is specified then it is also added to the calculated value as an additional delay.""",
        )
        group_display_image_delay_absolute.add_argument(
            "--absolute",
            "-a",
            metavar="<u32>",
            type=int,
            help="""Execution time of the command in ESL Absolute Time epoch value. Mutually exclusive with timed delay.""",
        )
        parser_display_image.add_argument(
            "--delay",
            "-dy",
            metavar="<u32>",
            type=int,
            help="Delay in milliseconds (optional)",
        )
        parser_display_image.add_argument(
            "--date",
            "-d",
            metavar="<YYYY-MM-DD>",
            type=date_type,
            help="Execution date of the command in ISO-8601 format (optional to time, only).",
        )

    def do_display_image(self, arg):
        """
        Display tag image.
        """
        now = dt.now()
        absolute_now = self.ap.get_absolute_time()
        esl_id = image_idx = display_idx = None
        group_id = delay_ms = 0
        absolute_base = absolute_value = None
        input_error = False

        esl_id = arg.esl_id
        if arg.image_index in range(0, 256):
            image_idx = arg.image_index
        else:
            self.log.warning("Image index must be between 0 and 255")
        if arg.image_index in range(0, 256):
            display_idx = arg.display_index
        else:
            self.log.warning("Display index must be between 0 and 255")
        group_id = arg.group_id
        if arg.time:
            set_date_input_time = arg.time
            if not arg.date:
                set_date = dt.combine(dt.today(), set_date_input_time.time())
            else:
                set_date = dt.combine(arg.date, set_date_input_time.time())
            try:
                delay_ms += self.ap.calculate_exec_time(
                    now,
                    set_date.hour,
                    set_date.minute,
                    set_date.second,
                    set_date.microsecond,
                    arg.date,
                )
                absolute_base = absolute_now
            except:
                self.log.error("Requested delay can't be set, command ignored!")
                input_error = True
        if arg.delay is not None:
            try:
                delay_ms += arg.delay
                absolute_base = absolute_now
            except:
                self.log.error(
                    "Invalid argument [delay=<delay_ms>]: Delay in milliseconds (optional)"
                )
                input_error = True
        if arg.absolute is not None:
            try:
                absolute_base = arg.absolute
                if absolute_base == 0:
                    delay_ms = None
            except:
                self.log.error(
                    "Invalid argument [absolute=<abs_time>]: Execution time of the command in ESL Absolute Time epoch value. Mutually exclusive with other delays."
                )
                input_error = True
        if delay_ms is None:
            absolute_value = absolute_base
        elif absolute_base is not None:
            absolute_value = int(absolute_base + delay_ms) & 0xFFFFFFFF
            if absolute_value == 0:  # do not send any unsolicited delete on overflow!!
                absolute_value += 1

        if not input_error:
            self.ap.ap_display_image(
                esl_id, group_id, image_idx, display_idx, absolute_value
            )

    def arg_ping(self):
        parser_ping = self.subparsers.add_parser(
            "ping",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_ping.__doc__,
            epilog="""
        Note: 'all' also can be used as a broadcast address (0xff) if IOP_TEST config True
              (Although it still makes no sense as broadcast messages doesn't solicit any
              response by the specification!)""",
        )
        parser_ping.add_argument("esl_id", type=esl_id_type, help="ESL ID or all")
        parser_ping.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_ping(self, arg):
        """
        Get the Basic State response of the addressed ESL.
        """
        group_id = arg.group_id
        esl_id = arg.esl_id
        self.ap.ap_ping(esl_id, group_id)

    def arg_vendor_opcode(self):
        parser_vendor_opcode = self.subparsers.add_parser(
            "vendor_opcode",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_vendor_opcode.__doc__,
            epilog="""
        Examples: vendor_opcode 0 -g 1 - there will be no extra payload, the resulting ESL TLV is 0F00 for group 1
                  vendor_opcode 3 --data 0x0004 - 2 bytes payload, the resulting ESL TLV is 2F030004 for default group 0
                  vendor_opcode 1 --data 12233 - 3 bytes payload, the resulting ESL TLV is 3F01012233
                  vendor_opcode 5 -d 0012233 - 4 bytes payload, the resulting ESL TLV is 4F0500012233

        Please note that the payload is always interpreted as an ASCII hex string, regardless of the presence or absence
        of the '0x' prefix, and if an odd number of bytes is entered, a leading zero will be added.""",
        )
        parser_vendor_opcode.add_argument(
            "esl_id", type=esl_id_type, help="ESL ID or all"
        )
        parser_vendor_opcode.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )
        parser_vendor_opcode.add_argument(
            "--data",
            "-d",
            metavar="<hex>",
            type=data_type,
            help="ASCII hexadecimal data stream up to 16 bytes overall - an appropriate TLV to the given length will be built automatically.",
        )

    def do_vendor_opcode(self, arg):
        """
        Send generic ESL vendor specific command.
        """
        group_id = arg.group_id
        data = None
        esl_id = arg.esl_id
        if arg.data is not None:
            prefix_pos = arg.data.casefold().find("x")
            if prefix_pos == -1:
                prefix_pos = 0
            else:
                prefix_pos += 1
            payload = int(arg.data, base=16)
            length = (len(arg.data[prefix_pos:]) + 1) // 2
            data = payload.to_bytes(length, byteorder="big")
        self.ap.ap_vendor_opcode(esl_id, group_id, data)

    def arg_service_reset(self):
        parser_service_reset = self.subparsers.add_parser(
            "service_reset",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=35
            ),
            description=self.do_service_reset.__doc__,
        )
        parser_service_reset.add_argument(
            "esl_id",
            type=esl_id_type,
            help="ESL ID or all can be used as a broadcast address (0xff)",
        )
        parser_service_reset.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_service_reset(self, arg):
        """
        Send Service Reset command.
        """
        group_id = arg.group_id
        esl_id = arg.esl_id
        self.ap.ap_service_reset(esl_id, group_id)

    def sync_config(self, arg_list=[]):
        """
        Overwrite default PAWR parameters. Some parameters have a resolution of 1.25msec or 0.125 msec
        If this requirement is not met, the nearest available value will be used
        """
        if arg_list[0] == "start" or arg_list[0] == "empty":
            self.log.info("Current sync parameters:")
            params = self.ap.get_pawr_params()
            del arg_list[0]

            if len(arg_list) > 0:
                params[0] = arg_list[0]
                params[1] = arg_list[-1]

            log(
                "Minimum Periodic Interval: "
                + str((1.25 * params[0]))
                + " ms ("
                + str(params[0])
                + ")"
            )
            log(
                "Maximum Periodic Interval: "
                + str((1.25 * params[1]))
                + " ms ("
                + str(params[1])
                + ")"
            )
            log("Subevent Count:            " + str(params[2]))
            log(
                "Subevent Interval:         "
                + str((1.25 * params[3]))
                + " ms ("
                + str(params[3])
                + ")"
            )
            log(
                "Response Slot Delay:       "
                + str((1.25 * params[4]))
                + " ms ("
                + str(params[4])
                + ")"
            )
            log(
                "Response Slot Duration:    "
                + str((0.125 * params[5]))
                + " ms ("
                + str(params[5])
                + ")"
            )
            log("Response Slot Count:       " + str(params[6]))
            return
        elif arg_list[0] == "config":
            if len(arg_list) == 1:
                self.log.error("One or more parameters missing!")
                return
            else:
                del arg_list[0]
                interval_min = int(arg_list[0])
                interval_max = int(arg_list[1])
                subevent_count = int(arg_list[2])
                subevent_interval = int(arg_list[3])
                response_slot_delay = int(arg_list[4])
                response_slot_spacing = int(arg_list[5])
                response_slot_count = int(arg_list[6])

        if interval_min > interval_max:
            self.log.error("The maximum interval can't be less than the minimal!")
        elif (
            (1.25 * interval_min < PA_INTERVAL_ABS_MIN)
            or (1.25 * interval_min > PA_INTERVAL_ABS_MAX)
            or (1.25 * interval_max < PA_INTERVAL_ABS_MIN)
            or (1.25 * interval_max > PA_INTERVAL_ABS_MAX)
        ):
            self.log.error("Interval settings are out of bounds!")
        elif (subevent_count < PA_SUBEVENT_MIN) or (subevent_count > PA_SUBEVENT_MAX):
            self.log.error("The subevent count is out of bounds!")
        elif (subevent_interval < PA_SUBEVENT_INTERVAL_MIN) or (
            subevent_interval > PA_SUBEVENT_INTERVAL_MAX
        ):
            self.log.error("The subevent interval is out of bounds!")
        elif (response_slot_delay < PA_RESPONSE_SLOT_DELAY_MIN) or (
            response_slot_delay > PA_RESPONSE_SLOT_DELAY_MAX
        ):
            self.log.error("The response slot delay is out of bounds!")
        elif (response_slot_spacing < PA_RESPONSE_SLOT_SPACING_MIN) or (
            response_slot_spacing > PA_RESPONSE_SLOT_SPACING_MAX
        ):
            self.log.error("The response slot spacing is out of reasonable bounds!")
        elif (response_slot_count < PA_RESPONSE_SLOT_NUMBERS_MIN) or (
            response_slot_count > PA_RESPONSE_SLOT_NUMBERS_MAX
        ):
            self.log.error("The response slot count is out of bounds!")
        else:
            self.ap.set_pawr_interval([interval_min, interval_max])
            self.ap.pawr_configure(
                interval_min,
                interval_max,
                subevent_count,
                subevent_interval,
                response_slot_delay,
                response_slot_spacing,
                response_slot_count,
            )

    def arg_sync(self):
        parser_sync = self.subparsers.add_parser(
            "sync",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=40
            ),
            description=self.do_sync.__doc__,
            epilog="""
        Notes: Using the optional '-ms' argument with the 'config' subcommand allows you to specify timing parameters in
               milliseconds instead of their natural units, but this may introduce rounding errors. Please also note
               that with this option the fractional milliseconds can't be specified precisely.
               You can ask for the current status of the PAwR train by omitting the choice.
        Disclaimer: Extended advertisement of PAwR train parameters, when initiated with a '--advertise' request, does
                    not start immediately when the sync signal is activated. Instead, the parameter advertisement is
                    started only after the first PAST procedure is SUCCESSFULLY initiated on a connected device. This
                    delay ensures that if the PAwR train is intentionally restarted, the ESLs will enter the
                    Unsynchronized state and advertise according to the ESL specification, rather than just silently
                    resynchronizing to another sync signal.
            """,
        )
        parser_sync.add_argument(
            "choice",
            nargs="?",
            choices=["start", "stop", "config"],
            help="Start/stop sending or config periodic synchronization packets",
        )
        parser_sync.add_argument(
            "--millis",
            "-ms",
            action="store_true",
            help="Specify timing parameters in milliseconds",
        )
        parser_sync.add_argument(
            "--advertise",
            "-a",
            action="store_true",
            help="Enable extended advertising of PAwR train parameters for sync scanners",
        )
        parser_sync.add_argument(
            "--in_max",
            "-max",
            metavar="<int>",
            type=int,
            help="Maximum periodic advertising interval in units of 1.25ms.",
        )
        parser_sync.add_argument(
            "--in_min",
            "-min",
            metavar="<int>",
            type=int,
            help="Minimum periodic advertising interval in units of 1.25ms.",
        )
        parser_sync.add_argument(
            "--se_count", "-sc", metavar="<int>", type=int, help="Number of subevents"
        )
        parser_sync.add_argument(
            "--se_interval",
            "-si",
            metavar="<int>",
            type=int,
            help="Subevent interval in units of 1.25ms",
        )
        parser_sync.add_argument(
            "--rs_delay",
            "-rd",
            metavar="<int>",
            type=int,
            help="Response slot delay in units of 1.25ms",
        )
        parser_sync.add_argument(
            "--rs_spacing",
            "-rs",
            metavar="<int>",
            type=int,
            help="Response slot spacing in units of 0.125ms",
        )
        parser_sync.add_argument(
            "--rs_count", "-rc", metavar="<int>", type=int, help="Response slot count"
        )

    def do_sync(self, arg):
        """
        Start / stop sending synchronization packets.
        """
        start = None
        arg_list = None
        if arg.choice == "config":
            arg_list = ["config"]
            arg_list[1:] = [
                arg.in_min,
                arg.in_max,
                arg.se_count,
                arg.se_interval,
                arg.rs_delay,
                arg.rs_spacing,
                arg.rs_count,
            ]
            if arg.advertise:
                self.log.warning(
                    "Advertising request ignored for configuration - please specify the argument with 'sync start' command if you want to enable advertising!"
                )
            if arg.millis:
                self.log.warning(
                    "The milliseconds time unit parameter is specified, but is not required for the configuration query."
                )
            if all(v is None for v in arg_list[1:]):
                arg_list.clear()
                arg_list.append("empty")
                self.sync_config(arg_list)
            elif None in arg_list:
                self.log.error("One or more parameters missing!")
            else:
                if arg.millis:
                    self.log.warning(
                        "Parameters in milliseconds may result in inaccuracies due to rounding errors!"
                    )
                    dividers = [1.25, 1.25, 1, 1.25, 1.25, 0.125, 1]
                    ms_values = [
                        round(float(value) / div)
                        for value, div in zip(arg_list[1:], dividers)
                    ]
                    ms_values.insert(0, "config")
                    self.sync_config(ms_values)
                else:
                    self.sync_config(arg_list)
        else:
            if arg.choice == "stop":
                d = vars(arg)
                d.pop("choice")
                if any(v for v in d.values()):
                    self.log.warning(
                        "Any additional parameters passed with the 'sync stop' command are ignored!"
                    )
                start = False
            elif arg.choice == "start":
                start = True
                arg_list = ["start"]
                if arg.in_min is not None:
                    arg_list.append(round(float(arg.in_min) / 1.25))
                if arg.in_max is not None:
                    arg_list.append(round(float(arg.in_max) / 1.25))
                if (
                    arg.se_count is not None
                    or arg.se_interval is not None
                    or arg.rs_delay is not None
                    or arg.rs_spacing is not None
                    or arg.rs_count is not None
                ):
                    self.log.warning(
                        "Only the interval can be specified with the 'sync start' command, the rest is ignored."
                    )
                if not arg.millis and (
                    arg.in_min is not None or arg.in_max is not None
                ):
                    self.log.warning(
                        "The interval given with the 'sync start' command is always interpreted in milliseconds and as such may introduce small rounding errors!"
                    )
                if len(arg_list) > 1:
                    # Check interval limits
                    if len(arg_list) >= 3:
                        if arg_list[1] > arg_list[2]:
                            self.log.error(
                                "Wrong periodic advertising interval values!"
                            )
                            return
                    for val in arg_list[1:]:
                        if (1.25 * val > PA_INTERVAL_ABS_MAX) or (
                            1.25 * val < PA_INTERVAL_ABS_MIN
                        ):
                            self.log.error(
                                "Wrong periodic advertising interval values!"
                                " Time range: 7.5 ms to 81.92 s"
                            )
                            return
                self.log.info(
                    "Request Periodic Synchronization Transfer start as follows:"
                )
                self.sync_config(arg_list)
                if len(arg_list) == 0:
                    arg_list = None
            self.ap.ap_sync(start, arg_list, arg.advertise)

    def arg_set_rssi_threshold(self):
        parser_set_rssi_threshold = self.subparsers.add_parser(
            "set_rssi_threshold",
            description=self.do_set_rssi_threshold.__doc__,
            epilog="Note: Negative values are accepted, only!",
        )
        parser_set_rssi_threshold.add_argument("rssi", type=int, help="RSSI value")

    def do_set_rssi_threshold(self, arg):
        """
        Set RSSI filter threshold value.
        """
        if arg.rssi < 0:
            self.ap.ap_set_rssi_threshold(int(arg.rssi))
        else:
            self.log.error("Invalid RSSI parameter, only negative integers are allowed")

    def precmd(self, command):
        """Optionally log commands to record_file"""
        if self.record_file and "script" and "record" not in command:
            print(command, file=self.record_file)
            self.record_file.flush()
        return command

    def arg_script(self):
        parser_script = self.subparsers.add_parser(
            "script",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=30
            ),
            description=self.do_script.__doc__,
            epilog="""
        Notes: Scripting is an experimental feature only – it also supports basic waiting with timeout and
               optional device filtering for events, but it lacks any configuration‑dependent and/or conditional
               execution capabilities. Recorded script files may contain script commands as well, even recursively.
               However, it is strongly advised to avoid it, as recursive execution cannot be interrupted and may
               lead to uncontrolled behavior.""",
        )

        # create subparsers for record / run / wait
        sub = parser_script.add_subparsers(dest="record_run_wait", required=True)

        # subcommand: record
        p_record = sub.add_parser(
            "record",
            help="Record commands to an output file. Issue 'script record stop' to stop recording and close the file."
        )
        p_record.add_argument(
            "filename",
            help="Filename to write AP commands to. Note: the word 'stop' is reserved, can't be used as a valid file name."
        )

        # subcommand: run
        p_run = sub.add_parser(
            "run",
            help="Run commands from an input file."
        )
        p_run.add_argument(
            "filename",
            help="Filename to read AP commands from."
        )

        # subcommand: wait
        p_wait = sub.add_parser(
            "wait",
            help="Wait before running the next command. "
        )
        p_wait.add_argument(
            "seconds",
            type=int,
            help="Seconds to wait"
        )
        p_wait.add_argument(
            "event",
            nargs="?",
            type=event_type,
            help="Event name (e.g. connection_opened, ESL_LIB_EVT_TAG_FOUND)"
        )
        p_wait.add_argument(
            "address",
            nargs="?",
            type=address_type,
            help="ESL ID (0-254), BLE address, or 'all'"
        )
        p_wait.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=esl_group_id_type,
            help="ESL group ID (optional, default 0 if an address is given, None otherwise); with address 'all', wait for first event from this group"
        )

    def do_script(self, arg):
        """
        Record commands to an output file or execute them from an input file.
        """
        if arg.record_run_wait == "record":
            filename = arg.filename
            self.record_commands(filename)

        elif arg.record_run_wait == "run":
            filename = arg.filename
            self.playback_commands(filename)

        elif arg.record_run_wait == "wait":
            seconds = arg.seconds
            event_name = arg.event
            address = arg.address
            wait_group_id = arg.group_id

            device_tag = None
            if address is None:
                # no device filtering
                pass
            elif address == "all":
                # group filtering only
                wait_group_id = arg.group_id # May be None if not given, integer in range 0..127, otherwise
            else:
                # address is guaranteed valid by address_type()
                if ":" in address:
                    # arg.address is BLE address → create Address object
                    node_id = Address.from_str(address)
                else:
                    # arg.address is an ESL ID
                    esl_id = int(address)
                    group_id = arg.group_id if arg.group_id is not None else 0
                    node_id = (esl_id, group_id)

                device_tag = self.ap.tag_db.find(node_id)

                if device_tag is None:
                    self.log.error(
                        "Unknown device: %s. Device must exist in tag database (use 'list' to see known devices).",
                        address,
                    )
                    return
            self.ap_wait(
                seconds,
                event_name=event_name,
                device_tag=device_tag,
                group_id=wait_group_id if device_tag is None else device_tag.group_id,
            )

    def arg_update_complete(self):
        parser_update_complete = self.subparsers.add_parser(
            "update_complete",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=35
            ),
            description=self.do_update_complete.__doc__,
            epilog="""
        Notes: This command used only for testing purposes in IOP test mode. 
               If the group is specified along with the keyword `all`, then only devices in the group will be affected.
               """,
        )
        parser_update_complete.add_argument(
            "address",
            type=address_type,
            nargs="?",
            default="",
            help="Bluetooth address (e.g. 'AA:BB:CC:DD:EE:22') in case insensitive format or ESL ID of the tag or 'all'.",
        )
        parser_update_complete.add_argument(
            "--group_id",
            "-g",
            metavar="<u7>",
            type=int,
            help="ESL group ID (optional, default is group 0)",
        )

    def do_update_complete(self, arg):
        """
        Send update complete command.
        """
        if IOP_TEST:
            self.ap.ap_update_complete(arg.address.lower(), arg.group_id)
        else:
            self.log.warning("The update_complete command works only in IOP test mode!")

    def arg_verbosity(self):
        parser_verbosity = self.subparsers.add_parser(
            "verbosity",
            formatter_class=lambda prog: argparse.RawDescriptionHelpFormatter(
                prog, max_help_position=17
            ),
            description=self.do_verbosity.__doc__,
            epilog="""
        Notes:   To check current verbosity level you can issue the command without argument.
                 NOTSET can be used to display debugging messages not only for AP code, but also for all python modules that may utilze logging.
        """,
        )
        parser_verbosity.add_argument(
            "choice",
            nargs="?",
            type=str.upper,
            choices=list(LEVELS),
            help="Level to apply",
        )

    def do_verbosity(self, arg):
        """
        Set Access Point logging verbosity level at runtime
        """
        if arg.choice is None:
            log("Current logging level:", logLevelName())
        else:
            setLogLevel(LEVELS[arg.choice])

    def all_help(self):
        """
        Print detailed help messages for all commands.
        """
        subparsers_actions = [
            action
            for action in self.command_parser._actions
            if isinstance(action, argparse._SubParsersAction)
        ]
        # there will probably only be one subparser_action,
        # but better safe than sorry
        for subparsers_action in subparsers_actions:
            # get all subparsers and print help
            for choice, subparser in subparsers_action.choices.items():
                print("Subparser '{}'".format(choice))
                print(subparser.format_help())

    def subparser_help(self, subparser_given):
        """
        Print detailed help message for the given subparser.
        """
        subp = []
        if not subparser_given:
            return self.do_help(subparser_given)
        # hidden argumentum of help - this will print the help message of all commands
        elif subparser_given == "--generate":
            self.all_help()
        else:
            subparsers_actions = [
                action
                for action in self.command_parser._actions
                if isinstance(action, argparse._SubParsersAction)
            ]
            for subparsers_action in subparsers_actions:
                for choice, subparser in subparsers_action.choices.items():
                    if subparser_given == choice:
                        subparser.print_help()
                    subp.append(choice)
                if subparser_given not in subp:
                    self.log.error(
                        "Help not available for unknown command: " + subparser_given
                    )

    def ap_wait(self, w_time, event_name=None, device_tag=None, group_id=None):
        """
        Wait <w_time> seconds before running the next command.
        If event_name is given, wait until that event occurs or timeout.
        If device_tag is given with event_name, only resume when the event
        originates from that device. If group_id is given (and device_tag is None),
        resume on the first event from any device in that group.

        Note: The wait blocks the CLI pipeline until the event or timeout occurs.
        However, internal system operations not handled by the command interpreter
        will proceed concurrently.
        """
        if event_name is None:
            self.log.info("Waiting " + str(w_time) + " seconds")
            time.sleep(int(w_time))
            return
        # Wait for event with optional device filter
        condition = threading.Condition()
        event_received = [False]  # list to allow closure to mutate

        def _tag_from_event(event):
            """Resolve tag from event (same logic as ap_core)."""
            if hasattr(event, "node_id"):
                return self.ap.tag_db.find(event.node_id)
            if hasattr(event, "address"):
                return self.ap.tag_db.find(event.address)
            if hasattr(event, "connection_handle"):
                return self.ap.tag_db.find(event.connection_handle)
            return None

        def _on_event(event):
            tag = _tag_from_event(event)
            if device_tag is not None:
                if tag is not device_tag:
                    return
            elif group_id is not None:
                if tag is None or getattr(tag, "group_id", None) != group_id:
                    return
            with condition:
                event_received[0] = True
                condition.notify()

        prefix = "script_wait_" + str(threading.get_ident())
        event_display = EVENT_PREFIX + event_name.upper() if event_name else ""
        try:
            self.ap.evt_dispatcher.subscribe(event_name, _on_event, prefix=prefix)
            if device_tag is not None:
                device_desc = " from ESL ID " + str(device_tag)
            elif group_id is not None:
                device_desc = " from group " + str(group_id)
            else:
                device_desc = ""
            self.log.info(
                "Waiting up to %s s for event %s%s",
                w_time,
                event_display,
                device_desc,
            )
            with condition:
                condition.wait(timeout=float(w_time))
            if event_received[0]:
                self.log.info("Event %s received%s", event_display, device_desc)
            else:
                self.log.info("Timeout waiting for event %s%s", event_display, device_desc)
        finally:
            self.ap.evt_dispatcher.unsubscribe_prefix(prefix)

    def playback_commands(self, fname):
        """Playback commands from an input file"""
        try:
            with open(fname) as f:
                self.log.info("Executing commands from file: " + fname)
                # Remove lines starting with '#' comment character
                command_list = [
                    i for i in f.read().splitlines() if not i.startswith("#")
                ]
                for line in command_list:
                    self.onecmd(line)
        except FileNotFoundError:
            self.log.warning("File not found: " + fname)

    def record_commands(self, fname):
        """Record commands to a file"""
        if fname != "stop":
            try:
                self.log.info("Recording commands to file: " + fname)
                self.record_file = open(fname, "w")
            except OSError:
                self.log.warning("Cannot open file: " + fname)
        else:
            if self.record_file:
                self.log.info("Recording of commands stopped")
                self.record_file.close()
                self.record_file = None
            else:
                self.log.info("There's no recording to stop!")

    def do_exit(self, arg):
        """
        Exit from application
        """
        return True

    def do_foreach_group(self, arg=None):
        """
        Command factory that creates and executes the command(s) following the expression for all configured ESL groups.
        """
        return list(self.ap.tag_db.list_all_groups().keys())

    def emptyline(self):
        # Do nothing in case of empty command input
        pass
