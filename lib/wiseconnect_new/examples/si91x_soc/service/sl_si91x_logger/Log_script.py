#!/usr/bin/env python3
"""
UART Console - Real-time log parser for sl_log_event_t records.

This console application receives raw 25-byte log messages from a UART port
(e.g., JLink CDC Uart Port) and decodes them into human-readable messages
using format strings extracted from a .out/.axf/.elf file.

Features:
- Auto-detection of JLink CDC UART port
- Auto-search for .out files in build directories
- Real-time log message decoding with timestamps
- Color-coded log levels (DEBUG, INFO, WARN, ERROR)
- Overflow detection and reporting
- Optional device reset via Simplicity Commander

Usage:
    python Log_script.py --out firmware.out
    python Log_script.py --port COM3 --baud 115200
    python Log_script.py --reset  # Reset device before starting

Special behavior:
- If event_id == 0xFFFFFFFF (4294967295), print:
    [OVERFLOW] <timestamp>: overflow count : <arg0>
"""

import argparse
import struct
import sys
import time
import subprocess
import os
import re
from pathlib import Path
from typing import Dict, Optional, List
from datetime import datetime

try:
    from elftools.elf.elffile import ELFFile
except ImportError:
    print("ERROR: pyelftools is required. Install with: pip install pyelftools")
    sys.exit(1)

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("ERROR: pyserial is required. Install with: pip install pyserial")
    sys.exit(1)

# Try to import colorama for colored output (optional)
try:
    from colorama import init, Fore, Style
    init(autoreset=True)
    COLORS_AVAILABLE = True
except ImportError:
    COLORS_AVAILABLE = False

SL_LOG_CONFIG_ARG = 3
RECORD_SIZE = 25
LEVEL_MAP = {1: "DEBUG", 2: "INFO", 3: "WARN", 4: "ERROR"}

# Color mapping for log levels
LEVEL_COLORS = {
    "DEBUG": Fore.CYAN if COLORS_AVAILABLE else "",
    "INFO": Fore.GREEN if COLORS_AVAILABLE else "",
    "WARN": Fore.YELLOW if COLORS_AVAILABLE else "",
    "ERROR": Fore.RED if COLORS_AVAILABLE else "",
    "OVERFLOW": Fore.MAGENTA if COLORS_AVAILABLE else "",
}
RESET_COLOR = Style.RESET_ALL if COLORS_AVAILABLE else ""


# -------------------------------------------------------------------------
# Console Banner and Utilities
# -------------------------------------------------------------------------
def print_banner():
    """Print the UART console banner."""
    banner = """
╔══════════════════════════════════════════════════════════════════════════════╗
║                         UART LOG CONSOLE                                     ║
║                    Real-time Log Message Decoder                             ║
╚══════════════════════════════════════════════════════════════════════════════╝
"""
    print(banner)


def print_config(port: str, baud: int, out_file: str, fmt_count: int):
    """Print the current configuration."""
    print("┌─ Configuration ─────────────────────────────────────────────────────────────┐")
    print(f"│  Serial Port  : {port:<60} │")
    print(f"│  Baud Rate    : {baud:<60} │")
    print(f"│  OUT File     : {Path(out_file).name:<60} │")
    print(f"│  Format Strs  : {fmt_count:<60} │")
    print("└──────────────────────────────────────────────────────────────────────────────┘")
    print()
    print("Press Ctrl+C to stop the console.\n")
    print("─" * 80)
    print(f"{'TIME':<12} {'LEVEL':<8} {'TIMESTAMP':<12} MESSAGE")
    print("─" * 80)


def get_current_time() -> str:
    """Get current time as HH:MM:SS.mmm"""
    now = datetime.now()
    return now.strftime("%H:%M:%S.") + f"{now.microsecond // 1000:03d}"


def colorize(text: str, level: str) -> str:
    """Apply color to text based on log level."""
    if not COLORS_AVAILABLE:
        return text
    color = LEVEL_COLORS.get(level, "")
    return f"{color}{text}{RESET_COLOR}"


# -------------------------------------------------------------------------
# Commander reset
# -------------------------------------------------------------------------
def reset_device():
    try:
        print("Resetting device using commander...")
        subprocess.run(["commander", "device", "reset"], check=True)
        print("Device reset complete.")
    except Exception as e:
        print(f"ERROR: Failed to reset device: {e}")


def parse_args():
    p = argparse.ArgumentParser(
        description="UART Console - Real-time log message decoder using .out file.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s --out firmware.out              # Use specific .out file
  %(prog)s --port COM3 --baud 115200       # Specify port and baud rate
  %(prog)s --reset                         # Reset device before starting
  %(prog)s --no-color                      # Disable colored output
        """
    )

    p.add_argument("--port", help="Serial port name (override auto-detection).")
    p.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200).")
    p.add_argument(
        "--out", "--axf",
        dest="out_file",
        help="Path to .out/.axf/.elf file containing log format strings. If omitted, auto-search in build directories.",
    )
    p.add_argument(
        "--big-endian",
        action="store_true",
        help="Use big-endian for records (default: little-endian).",
    )
    p.add_argument(
        "--no-arg-format",
        action="store_true",
        help="Skip applying arguments to format strings (faster).",
    )
    p.add_argument(
        "--no-color",
        action="store_true",
        help="Disable colored output.",
    )
    p.add_argument(
        "--no-banner",
        action="store_true",
        help="Skip printing the console banner.",
    )
    p.add_argument(
        "--reset",
        action="store_true",
        help="Reset device using 'commander device reset' before starting.",
    )
    p.add_argument(
        "--raw",
        action="store_true",
        help="Also show raw hex data for each record (debug mode).",
    )
    return p.parse_args()


def mk_struct_fmt(big_endian: bool) -> str:
    endian = ">" if big_endian else "<"
    # timestamp(4) + event_id(4) + 3 args(12) + arg_count(2) + core(1) + flags(1) + version(1)
    return endian + ("II" + "I" * SL_LOG_CONFIG_ARG + "HBBB")


def auto_find_out_file() -> Optional[str]:
    """Auto-search for .out/.axf files in common build directories."""
    search_dirs = [
        Path.cwd() / "GNU ARM v12.2.1 - Default",
        Path.cwd() / "build",
        Path.cwd() / "Debug",
        Path.cwd() / "Release",
        Path.cwd(),
    ]
    
    out_files = []
    for base_dir in search_dirs:
        if base_dir.exists():
            out_files.extend(base_dir.rglob("*.out"))
            out_files.extend(base_dir.rglob("*.axf"))
            out_files.extend(base_dir.rglob("*.elf"))
    
    if not out_files:
        print("Warning: No .out/.axf/.elf files found in search directories")
        return None

    # Sort by modification time (newest first)
    out_files.sort(key=lambda p: p.stat().st_mtime, reverse=True)
    chosen = out_files[0]
    print(f"Auto-detected OUT file: {chosen}")
    return str(chosen)


def load_log_fmt_map(out_path: str) -> Dict[int, str]:
    """
    Load log format strings from the .log_fmt section of an ELF/OUT file.
    
    The .out file contains a special section called .log_fmt which stores
    all the format strings used for logging. Each string is mapped to its
    memory address (event_id) for runtime decoding.
    
    Args:
        out_path: Path to the .out/.axf/.elf file
        
    Returns:
        Dictionary mapping event_id (address) to format string
    """
    try:
        with open(out_path, "rb") as f:
            elf = ELFFile(f)
            sec = elf.get_section_by_name(".log_fmt")
            if sec is None:
                print("Warning: .log_fmt section not found in OUT file.")
                print("         Log messages will show as raw event IDs.")
                return {}

            base_addr = sec["sh_addr"]
            data = sec.data()
            n = len(data)

        fmt_map: Dict[int, str] = {}
        i = 0
        while i < n:
            if data[i] == 0:
                i += 1
                continue
            start = i
            while i < n and data[i] != 0:
                i += 1
            s = data[start:i].decode("utf-8", errors="replace")
            addr = base_addr + start
            fmt_map[addr] = s
            i += 1

        return fmt_map
        
    except Exception as e:
        print(f"ERROR: Failed to load format strings from {out_path}: {e}")
        return {}


def find_jlink_port() -> Optional[str]:
    """Find JLink CDC UART port automatically."""
    desired_ports = ["JLink CDC Uart Port", "JLink CDC UART", "J-Link"]
    for p in list_ports.comports():
        desc = p.description or ""
        for desired in desired_ports:
            if desired.lower() in desc.lower():
                return p.device
    return None


def list_available_ports():
    """List all available serial ports."""
    ports = list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return
    print("\nAvailable serial ports:")
    for p in ports:
        print(f"  {p.device}: {p.description}")


def level_from_flags(flags: int) -> str:
    """Extract log level from flags byte."""
    lvl_code = (flags >> 1) & 0x07
    return LEVEL_MAP.get(lvl_code, f"LEVEL{lvl_code}")


def parse_record(chunk: bytes, fmt: str) -> tuple:
    """Parse a 25-byte log record into its components."""
    ts, event_id, *rest = struct.unpack(fmt, chunk)
    args = rest[:SL_LOG_CONFIG_ARG]
    arg_count, core_id, flags, version = rest[SL_LOG_CONFIG_ARG:SL_LOG_CONFIG_ARG + 4]
    return ts, event_id, args, arg_count, core_id, flags, version


def format_raw_hex(chunk: bytes) -> str:
    """Format raw bytes as hex string for debug output."""
    return " ".join(f"{b:02X}" for b in chunk)


def apply_args_to_format(fmt_str: str, args_list: List[int], arg_count: int) -> str:
    """
    Replace format placeholders in the format string with actual argument values.
    
    Supports common C format specifiers: %lu, %u, %d, %x, %X, %08x, %s, etc.
    """
    if arg_count <= 0 or not fmt_str:
        return fmt_str
    
    s = fmt_str
    # Extended list of common C format specifiers
    placeholders = [
        "%lu", "%ld", "%lx", "%lX",
        "%u", "%d", "%x", "%X",
        "%08x", "%08X", "%04x", "%04X",
        "%02x", "%02X",
        "%i", "%p"
    ]
    max_args = min(arg_count, len(args_list))
    arg_idx = 0

    for _ in range(max_args):
        replaced = False
        best_pos = len(s)
        best_ph = None
        
        # Find the earliest placeholder
        for ph in placeholders:
            pos = s.find(ph)
            if pos != -1 and pos < best_pos:
                best_pos = pos
                best_ph = ph
        
        if best_ph is not None:
            # Format based on placeholder type
            val = args_list[arg_idx]
            if 'x' in best_ph or 'X' in best_ph:
                # Hex format
                if '08' in best_ph:
                    formatted = f"{val:08x}" if 'x' in best_ph else f"{val:08X}"
                elif '04' in best_ph:
                    formatted = f"{val:04x}" if 'x' in best_ph else f"{val:04X}"
                elif '02' in best_ph:
                    formatted = f"{val:02x}" if 'x' in best_ph else f"{val:02X}"
                else:
                    formatted = f"{val:x}" if 'x' in best_ph else f"{val:X}"
            elif 'p' in best_ph:
                formatted = f"0x{val:08x}"
            else:
                formatted = str(val)
            
            s = s[:best_pos] + formatted + s[best_pos + len(best_ph):]
            arg_idx += 1
        else:
            break
            
    return s


def open_serial(port: str, baud: int) -> serial.Serial:
    """Open serial port with specified settings."""
    ser = serial.Serial(
        port=port,
        baudrate=baud,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.0,
        write_timeout=0.0,
    )
    return ser


def decode_message(event_id: int, args_val: tuple, arg_count: int, flags: int, 
                   fmt_map: Dict[int, str], no_arg_format: bool, core_id: int, version: int) -> tuple:
    """
    Decode a log event into a human-readable message.
    
    Args:
        event_id: The event identifier (address of format string or numeric ID)
        args_val: Tuple of argument values
        arg_count: Number of valid arguments
        flags: Log flags byte
        fmt_map: Dictionary mapping addresses to format strings
        no_arg_format: If True, skip argument substitution
        core_id: Core ID from the record
        version: Version from the record
        
    Returns:
        Tuple of (level_str, message)
    """
    # Check for overflow event
    if event_id == 0xFFFFFFFF:
        overflow_count = args_val[0] if arg_count > 0 else 0
        return "OVERFLOW", f"Buffer overflow detected! Count: {overflow_count}"
    
    is_fmt = (flags & 1) == 0
    level_str = level_from_flags(flags)
    arg_count = min(arg_count, SL_LOG_CONFIG_ARG)
    
    if is_fmt and fmt_map:
        base_msg = fmt_map.get(event_id)
        if base_msg:
            if not no_arg_format and arg_count > 0:
                msg = apply_args_to_format(base_msg, list(args_val), arg_count)
            else:
                msg = base_msg
        else:
            # Event ID not found in format map
            msg = f"<Unknown Event 0x{event_id:08X}> Args={list(args_val)[:arg_count]}"
    else:
        # Numeric event or no format map
        msg = f"Event=0x{event_id:08X} Core={core_id} Ver={version} Args={list(args_val)[:arg_count]}"
    
    return level_str, msg


def main():
    global COLORS_AVAILABLE
    
    args = parse_args()
    
    # Handle color option
    if args.no_color:
        COLORS_AVAILABLE = False

    # Print banner
    if not args.no_banner:
        print_banner()

    # Optional device reset first
    if args.reset:
        reset_device()
        time.sleep(0.5)  # delay for CDC re-enumeration

    # Find OUT file
    if args.out_file:
        out_path = args.out_file
        if not os.path.exists(out_path):
            print(f"ERROR: OUT file not found: {out_path}")
            sys.exit(1)
    else:
        out_path = auto_find_out_file()
        if not out_path:
            print("ERROR: Could not find .out file automatically.")
            print("       Use --out <path> to specify the firmware .out file.")
            sys.exit(1)

    # Load format strings from OUT file
    fmt_map = load_log_fmt_map(out_path)
    print(f"Loaded {len(fmt_map)} format strings from .log_fmt section")

    # Serial port
    if args.port:
        port_name = args.port
    else:
        port_name = find_jlink_port()
        if not port_name:
            print("ERROR: JLink CDC UART port not found.")
            list_available_ports()
            print("\nUse --port <port> to specify the serial port manually.")
            sys.exit(1)
        print(f"Auto-detected serial port: {port_name}")

    # Struct format
    fmt = mk_struct_fmt(args.big_endian)
    if struct.calcsize(fmt) != RECORD_SIZE:
        print(f"ERROR: Struct size mismatch (got {struct.calcsize(fmt)}, expected {RECORD_SIZE}).")
        sys.exit(1)

    try:
        ser = open_serial(port_name, args.baud)
    except Exception as e:
        print(f"ERROR: Failed to open serial port {port_name}: {e}")
        list_available_ports()
        sys.exit(1)

    # Print configuration and start header
    print()
    print_config(port_name, args.baud, out_path, len(fmt_map))

    buffer = bytearray()
    read_size = 4096
    record_count = 0
    error_count = 0

    try:
        while True:
            try:
                data = ser.read(read_size)
            except Exception as e:
                sys.stdout.write(f"\n[ERROR] Serial read failed: {e}\n")
                sys.stdout.flush()
                break

            if data:
                buffer.extend(data)

                while len(buffer) >= RECORD_SIZE:
                    chunk = buffer[:RECORD_SIZE]
                    del buffer[:RECORD_SIZE]

                    try:
                        ts, event_id, args_val, arg_count, core_id, flags, version = parse_record(chunk, fmt)
                    except Exception as e:
                        error_count += 1
                        if buffer:
                            buffer.pop(0)
                        continue

                    record_count += 1
                    
                    # Decode the message
                    level_str, msg = decode_message(
                        event_id, args_val, arg_count, flags,
                        fmt_map, args.no_arg_format, core_id, version
                    )
                    
                    # Format output
                    current_time = get_current_time()
                    
                    if args.raw:
                        # Debug mode: show raw hex
                        raw_hex = format_raw_hex(chunk)
                        output = f"{current_time} [{colorize(level_str, level_str):>8}] {ts:>10}: {msg}\n"
                        output += f"           RAW: {raw_hex}\n"
                    else:
                        output = f"{current_time} [{colorize(level_str, level_str):>8}] {ts:>10}: {msg}\n"
                    
                    sys.stdout.write(output)
                    sys.stdout.flush()
            else:
                time.sleep(0.001)

    except KeyboardInterrupt:
        print("\n" + "─" * 80)
        print(f"Console stopped. Records processed: {record_count}, Errors: {error_count}")
    finally:
        try:
            ser.close()
        except Exception:
            pass


if __name__ == "__main__":
    main()

