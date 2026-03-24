#!/usr/bin/env python3
"""This script has moved. Use the new provisioning workflow instead."""

import sys

print(
    "This script has been replaced by the unified provisioning targets.\n"
    "\n"
    "Use one of the following:\n"
    "  ./fbt crypto_provision   - provision both Matter and MQTT credentials\n"
    "  ./fbt matter_provision   - provision Matter attestation only\n"
    "  ./fbt mqtt_provision     - provision MQTT TLS credentials only\n"
    "  ./fbt crypto_wipe        - wipe MQTT key storage\n"
    "\n"
    "Or run the scripts directly:\n"
    "  python3 scripts/matter_provision.py [--insecure-crypto]\n"
    "  python3 scripts/mqtt_provision.py provision [--insecure-crypto]\n",
    file=sys.stderr,
)
sys.exit(1)
