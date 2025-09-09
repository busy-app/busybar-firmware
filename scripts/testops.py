#!/usr/bin/env python3
"""
BusyBar Operations Tool
Usage: python testops.py wait -t TIMEOUT -h HOST -p PORT
"""
import socket
import subprocess
import time
import os
import argparse
import logging
import sys


class BusyBarWaiter:
    """Handles waiting for BusyBar service to come online"""

    def __init__(self):
        self.logger = self._setup_logger()

    def _setup_logger(self):
        """Setup logging with timestamps"""
        logger = logging.getLogger('busybar_waiter')
        logger.setLevel(logging.INFO)

        handler = logging.StreamHandler()
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        handler.setFormatter(formatter)

        if not logger.handlers:
            logger.addHandler(handler)

        return logger

    def _ping_host(self, host: str, timeout: int = 1) -> bool:
        """Ping a host to check if it's reachable"""
        try:
            # Use ping command with timeout
            result = subprocess.run(
                ['ping', '-c', '1', '-W', str(timeout), host],
                capture_output=True,
                timeout=timeout + 1
            )
            return result.returncode == 0
        except (subprocess.TimeoutExpired, subprocess.SubprocessError):
            return False

    def _check_port(self, host: str, port: int, timeout: int = 1) -> bool:
        """Check if a specific port is open on the host"""
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
                sock.settimeout(timeout)
                result = sock.connect_ex((host, port))
                return result == 0
        except (socket.error, socket.timeout):
            return False

    def wait_for_busybar(self, host: str, port: int = 80, timeout: int = 60) -> bool:
        """
        Wait for BusyBar to come online

        Args:
            host: BusyBar IP address or hostname
            port: Port to check (default 80)
            timeout: Timeout in seconds (default 60)

        Returns:
            True if BusyBar comes online, False if timeout
        """
        start_time = time.time()

        self.logger.info(f"Waiting for BusyBar to come online at {host}:{port} (timeout: {timeout}s)...")

        while True:
            current_time = time.time()
            elapsed_time = int(current_time - start_time)

            # Check if timeout exceeded
            if elapsed_time >= timeout:
                self.logger.error(f"ERROR: BusyBar did not come online after {timeout} seconds")
                return False

            # Check if host is pingable
            if self._ping_host(host):
                self.logger.info("BusyBar is online!")

                # Extra check: try to connect to the specified port
                if self._check_port(host, port):
                    elapsed_time = int(time.time() - start_time)
                    self.logger.info(f"BusyBar web service is ready! (elapsed time: {elapsed_time}s)")
                    return True
                else:
                    self.logger.info(f"BusyBar is pingable but web service not ready yet... (elapsed: {elapsed_time}s)")
            else:
                self.logger.info(f"BusyBar not responding yet... (elapsed: {elapsed_time}s)")

            # Wait before next attempt
            time.sleep(2)


def cmd_wait(args):
    """Handler for wait subcommand"""
    waiter = BusyBarWaiter()
    success = waiter.wait_for_busybar(
        host=args.host,
        port=args.port,
        timeout=args.timeout
    )
    return 0 if success else 1


def main():
    """Main entry point"""
    # Create main parser
    parser = argparse.ArgumentParser(
        description="BusyBar Operations Tool",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    # Create subparsers for different commands
    subparsers = parser.add_subparsers(
        title='commands',
        description='Available commands',
        help='Command help',
        dest='command',
        required=True
    )

    # Add 'wait' subcommand
    parser_wait = subparsers.add_parser(
        'wait',
        help='Wait for BusyBar to come online',
        description='Wait for BusyBar service to become available'
    )

    # Add arguments specific to wait command
    parser_wait.add_argument(
        '--host',
        help='BusyBar IP address or hostname',
        default=os.getenv('BUSYBAR_IP', '10.0.4.20'),
        dest='host'
    )

    parser_wait.add_argument(
        '--port',
        help='Port to check (default: 80)',
        type=int,
        default=80
    )

    parser_wait.add_argument(
        '-t', '--timeout',
        help='Timeout in seconds (default: 60)',
        type=int,
        default=60
    )

    # Set the function to call for wait command
    parser_wait.set_defaults(func=cmd_wait)

    # Parse arguments
    args = parser.parse_args()

    # Execute the appropriate function
    try:
        return args.func(args)
    except KeyboardInterrupt:
        print("\nInterrupted by user", file=sys.stderr)
        return 130  # Standard exit code for SIGINT
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())