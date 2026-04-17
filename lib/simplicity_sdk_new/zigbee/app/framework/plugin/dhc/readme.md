# Zigbee Dynamic Hardware Configuration (DHC)

The Zigbee Dynamic Hardware Configuration (DHC) feature lets a Zigbee host provision or audit power amplifier calibration data on a running Network Co-Processor (NCP). Configuration data travels over EZSP using CLI commands or a JSON document.

## Audience

This document targets engineers integrating the DHC component into Zigbee gateways, manufacturing tools, or bring-up harnesses. It assumes familiarity with EZSP host builds, PA calibration concepts, and Silicon Labs tooling.

## Components at a Glance

- `dhc-cli.c`: user-facing EZSP CLI commands that read, write, or export calibration data.
- `dhc-parser.c`: JSON ingestion layer that detects the legacy schema, validates fields, and calls the host API.
- `dhc-ncp.c`: NCP-side wrappers that translate API calls into RAIL/NVM3 operations.
- `configuration_sample.json`: reference document that matches the accepted schema.

## Prerequisites

1. Build or flash an NCP image that enables the Zigbee DHC component.
2. Build the host application with `zigbee_dhc` so that the DHC CLI, parser, and EZSP commands are linked.
3. If you plan to seed configuration automatically at startup, provide the JSON file path through the host option parser (see below).

## Host Startup Options

Host applications built using the Zigbee DHC component can take a -j <file> to apply a JSON configuration file before network bring-up.

Typical usage:

```
./ezsp-host-app -p /dev/ttyUSB0 -b 115200 -j configuration_sample.json
```

During `sli_zigbee_af_init_cb`, `af-host.c` calls `sl_zigbee_dhc_init_from_json(path, dry_run=false, stop_on_error=true)`. Any non-success status is logged with the file path to aid debugging.

## Supported Calibration Elements

| Category | Details |
|----------|---------|
| Metadata | version, num_descriptors, pa_voltage, signature |
| PA descriptors | algorithm, segment/table count, min_ddbm, max_ddbm |
| PA curves | per-segment maxPowerLevel, slope, intercept |
| PA tables | 16-entry ddbm array (per index) |
| Scalars | rssi_offset, pa_mode, ctune |
| Versions | dhc_version (volatile) and pa_version |

## Typical Workflow

1. Provide or capture a calibration JSON file (see `configuration_sample.json`).
2. Launch the host with `-j <file>` or run the CLI command `plugin dhc apply file <file>`.
3. Verify the result:
   - `plugin dhc read all` for a full dump.
   - `plugin dhc export` for a metadata summary.
4. Save or adjust individual values with the `plugin dhc set ...` commands.

### Apply versus Validate

`plugin dhc apply file <json>` writes values to the NCP. `plugin dhc validate file <json>` parses the document with `DHC_PARSE_FLAG_DRY_RUN`, confirming schema correctness without touching hardware.

## CLI Reference

The following commands are registered in `dhc-cli.c`. All commands return a status code where `0x00000000` (SL_STATUS_OK) indicates success. Invalid argument counts display a usage banner.

### File Operations

#### `plugin dhc apply file <json_path>`
Parse and write a JSON configuration document to the NCP.

**Example:**
```bash
plugin dhc apply file /path/to/configuration_sample.json
# Output: apply /path/to/configuration_sample.json -> 0x00000000
```

#### `plugin dhc validate file <json_path>`
Parse and validate a JSON document without writing to hardware (dry-run mode).

**Example:**
```bash
plugin dhc validate file /path/to/configuration_sample.json
# Output: validate /path/to/configuration_sample.json -> 0x00000000
```

#### `plugin dhc export`
Print a compact JSON summary with current metadata values.

**Example:**
```bash
plugin dhc export
# Output: {"silabs_dhc":{"version":5,"metadata":{"num_descriptors":2,"pa_voltage":3300,"signature":305419896}}}
```

### Read Operations

#### `plugin dhc read-metadata`
Display PA metadata including version, number of descriptors, voltage, and signature.

**Example:**
```bash
plugin dhc read-metadata
# Output: metadata: status=0x00000000 version=5 num_desc=2 pa_voltage=3300 signature=0x12345678
```

#### `plugin dhc read-scalars`
Display all scalar values: RSSI offset, PA mode, CTUNE, voltage, signature, and versions.

**Example:**
```bash
plugin dhc read-scalars
# Output: scalars: rssi_offset=0 pa_mode=4 ctune=87 pa_voltage=3300 pa_signature=0x12345678 pa_version=1 dhc_version=5
```

#### `pluign dhc read-versions`
Display DHC and PA version information (alias for version fields from `read-scalars`).

**Example:**
```bash
plugin dhc read-versions
# Output: versions: pa_version=1 dhc_version=5
```

#### `plugin dhc read-all`
Comprehensive dump of all DHC data: metadata, all descriptors, curves/tables, and scalars.

**Example:**
```bash
plugin dhc read-all
# Output: [Full dump of all DHC configuration]
```

#### `plugin dhc read-descriptor <index>`
Display a single PA descriptor by index.

**Example:**
```bash
plugin dhc read-descriptor 0
# Output: descriptor[0]: status=0x00000000 algo=0 n=9 min_ddbm=0 max_ddbm=90
```

#### `plugin dhc read-curve <index>`
Display curve segments for a curve-based descriptor (algorithm 0).

**Example:**
```bash
plugin dhc read-curve 0
# Output: curve[0]: status=0x00000000 min=-300 max=100
#         seg[0]: mpl=255 slope=100 intercept=22
#         seg[1]: mpl=90 slope=1960 intercept=-116460
#         ...
```

#### `plugin dhc read-table <index>`
Display table entries for a table-based descriptor (algorithm 1).

**Example:**
```bash
plugin dhc read-table 1
# Output: table[1]: status=0x00000000 -252 -149 -97 -69 -52 -40 -32 -26 -22 -18 -15 -13 -11 -9 -8 -7
```

#### `plugin dhc read-segment <pa_index> <segment_index>`
Display a single curve segment from a specific PA descriptor.

**Example:**
```bash
plugin dhc read-segment 0 2
# Output: segment[0][2]: status=0x00000000 mpl=36 slope=567 intercept=-7935
```

### Write Operations

#### `plugin dhc set-descriptor <index> <algorithm> <count> <min_ddbm> <max_ddbm>`
Update a PA descriptor's parameters.

**Parameters:**
- `index`: Descriptor index (0-based)
- `algorithm`: 0 for curve, 1 for table
- `count`: Number of segments (curve) or entries (table)
- `min_ddbm`: Minimum ddBm value
- `max_ddbm`: Maximum ddBm value

**Example:**
```bash
plugin dhc set-descriptor 0 0 9 0 90
# Output: set-descriptor -> 0x00000000
```

#### `plugin dhc set-segment <pa_index> <segment_index> <mpl> <slope> <intercept>`
Update a single curve segment.

**Parameters:**
- `pa_index`: PA descriptor index
- `segment_index`: Segment index within the curve
- `mpl`: Maximum power level (0-255)
- `slope`: Slope value
- `intercept`: Intercept value

**Example:**
```bash
plugin dhc set-segment 0 2 36 567 -7935
# Output: set-segment -> 0x00000000
```

#### `plugin dhc set-table <pa_index> <entry_index> <ddbm>`
Update a single table entry.

**Parameters:**
- `pa_index`: PA descriptor index
- `entry_index`: Entry index (0-15)
- `ddbm`: ddBm value for this entry

**Example:**
```bash
plugin dhc set-table 1 0 -252
# Output: set-table -> 0x00000000
```

#### `plugin dhc set-scalar <type> <value>`
Update a scalar value. Valid types: `rssi_offset`, `pa_mode`, or `ctune`.

**Example:**
```bash
plugin dhc set-scalar pa_mode 5
# Output: set-scalar -> 0x00000000

plugin dhc set-scalar rssi_offset -2
# Output: set-scalar -> 0x00000000

plugin dhc set-scalar ctune 175
# Output: set-scalar -> 0x00000000
```

#### `plugin dhc set-voltage <millivolts>`
Update PA voltage in millivolts.

**Example:**
```bash
plugin dhc set-voltage 3300
# Output: set-voltage -> 0x00000000
```

#### `plugin dhc set-signature <hex32>`
Update PA signature (32-bit hex value).

**Example:**
```bash
plugin dhc set-signature 0x12345678
# Output: set-signature -> 0x00000000
```

#### `plugin dhc set-dhc-version <version>`
Update DHC protocol version (volatile, not persisted).

**Example:**
```bash
plugin dhc set-dhc-version 5
# Output: set-dhc-version -> 0x00000000
```

#### `plugin dhc recompute signature`
Regenerate the PA signature by computing a checksum of all descriptor and curve/table data.

**Example:**
```bash
plugin dhc recompute signature
# Output: recompute signature -> 0x00000000
```

### Raw Binary Protocol

#### `plugin dhc raw <bytes...>`
Send a raw binary frame to the DHC protocol. This low-level interface provides direct access to the binary protocol for debugging, testing, or integration with external tools.

**Frame Format:**
```
Byte 0: 0xDC (header)
Byte 1: Command (0x00 = READ, 0x01 = WRITE)
Byte 2: Setting ID
Bytes 3+: Payload (WRITE only)
```

**Setting IDs:**
- `0x00` = VERSION (metadata.version)
- `0x01` = RSSI_OFFSET
- `0x02` = PA_MODE
- `0x03` = CTUNE (4 bytes, little-endian)
- `0x04` = VOLTAGE (2 bytes, little-endian)
- `0x05` = SIGNATURE (4 bytes, little-endian)
- `0x06` = METADATA (8 bytes: version, num_desc, voltage, signature)
- `0x07` = DHC_VERSION
- `0x08` = PA_VERSION

**Response Format:**
```
RESP: DC <status> <setting_id> [value_bytes...]
```

The response includes both hex output and a human-readable interpretation.

**Read Examples:**
```bash
# Read PA mode
plugin dhc raw "DC 00 02"
# Output: RESP: DC 00 02 04
#         PA Mode: 4

# Read CTUNE (4-byte value)
plugin dhc raw "DC 00 03"
# Output: RESP: DC 00 03 57 00 00 00
#         CTUNE: 87 (0x00000057)

# Read voltage (2-byte value, little-endian)
plugin dhc raw "DC 00 04"
# Output: RESP: DC 00 04 E4 0C
#         Voltage: 3300 mV

# Read full metadata
plugin dhc raw "DC 00 06"
# Output: RESP: DC 00 06 05 02 D0 0C 12 34 56 78
#         Metadata: version=5 num_desc=2 voltage=3280 mV signature=0x78563412
```

**Write Examples:**
```bash
# Write PA mode to 5
plugin dhc raw "DC 01 02 05"
# Output: RESP: DC 00 02 00
#         PA Mode: 0

# Write CTUNE = 175 (0xAF) as 4 bytes little-endian
plugin dhc raw "DC 01 03 AF 00 00 00"
# Output: RESP: DC 00 03 00

# Write voltage = 3300mV (0x0CE4) as 2 bytes little-endian
plugin dhc raw "DC 01 04 E4 0C"
# Output: RESP: DC 00 04 00
```

**Status Codes:**
- `0x00` = Success (SL_STATUS_OK)
- `0x21` = Invalid parameter (SL_STATUS_INVALID_PARAMETER)
- `0x2B` = Invalid count (SL_STATUS_INVALID_COUNT)
- `0x0E` = Not available (SL_STATUS_NOT_AVAILABLE)
- `0x0F` = Not supported (SL_STATUS_NOT_SUPPORTED)
- `0x22` = Null pointer (SL_STATUS_NULL_POINTER)
- `0x27` = Invalid index (SL_STATUS_INVALID_INDEX)


**Note:** The raw command accepts hex bytes in multiple formats:
- Space-separated: `plugin dhc raw DC 00 02`
- With 0x prefix: `plugin dhc raw 0xDC 0x00 0x02`
- Single quoted string: `plugin dhc raw "DC 00 02"`

## JSON Schema

The parser accepts the schema of which the minimal example is shown below; consult `configuration_sample.json` for a complete document.

```json
{
  "silabs_dhc": {
    "version": 5,
    "metadata": {
      "num_descriptors": 2,
      "pa_voltage": 3300,
      "signature": 305419896
    },
    "pa_curves": [
      {
        "index": 0,
        "curve_min_ddbm": -100,
        "curve_max_ddbm": 100,
        "segments": [
          {"segment_index": 0, "maxPowerLevel": 10, "slope": 1, "intercept": 0}
        ]
      }
    ],
    "scalars": {
      "rssi_offset": -2,
      "pa_mode": 3,
      "ctune": 175
    }
  }
}
```

Key validation rules enforced by `dhc-parser.c`:

- Metadata fields are optional, but when present must fit within documented ranges (for example `pa_voltage` 0 to 65535).
- Descriptor counts and indexes must be consistent with `num_descriptors`.
- Curve segment arrays must not exceed `SL_ZIGBEE_DHC_CURVE_SEGMENT_COUNT`.
- Table entries must contain exactly `SL_ZIGBEE_DHC_TABLE_ENTRY_COUNT` integers.
- Scalars must fit within the target type (int8 for RSSI offset, uint32 for ctune, etc.).

Dry-run and stop-on-error flags let manufacturing flows catch malformed data quickly. `DHC_PARSE_FLAG_STOP_ON_ERROR` aborts on the first invalid field to avoid partial writes.

## Understanding Responses

### Text Responses (Regular CLI Commands)

Most CLI commands return human-readable text with status codes:

**Success Response:**
```
metadata: status=0x00000000 version=5 num_desc=2 pa_voltage=3300 signature=0x12345678
```

**Error Response:**
```
set-scalar -> 0x00000021
```
(0x00000021 = SL_STATUS_INVALID_PARAMETER)

### Hex Responses (Raw Command)

The `dhc raw` command returns both hex and interpreted output:

**Example:**
```bash
plugin dhc raw "DC 00 02"
RESP: DC 00 02 04
PA Mode: 4
```

**Response Structure:**
- `DC` = Protocol header (0xDC)
- `00` = Status byte (0x00 = success)
- `02` = Setting ID (0x02 = PA_MODE)
- `04` = Value byte(s)

**Multi-byte Values (Little-Endian):**
- CTUNE (4 bytes): `57 00 00 00` = 0x00000057 = 87
- Voltage (2 bytes): `E4 0C` = 0x0CE4 = 3300
- Signature (4 bytes): `12 34 56 78` = 0x78563412

**Error Responses:**
When an error occurs, the status byte is non-zero and no value bytes are returned:
```bash
plugin dhc raw "DC 00 99"
RESP: DC 21 99
Status: 0x21 (ERROR)
```
Note: The status byte in raw responses is the low byte of the full status code. Common error status bytes:
- `0x21` = SL_STATUS_INVALID_PARAMETER (full code: 0x00000021)
- `0x2B` = SL_STATUS_INVALID_COUNT (full code: 0x0000002B)
- `0x0F` = SL_STATUS_NOT_SUPPORTED (full code: 0x0000000F)

## Troubleshooting

| Symptom | Suggested Action |
|---------|------------------|
| JSON apply returns `SL_STATUS_INVALID_PARAMETER` | Check for missing keys, negative values where unsigned fields are expected, or segment counts beyond the allowed maximum. Verify JSON syntax with `dhc validate file <json>`. |
| JSON apply returns `SL_STATUS_NOT_SUPPORTED` | Verify the top-level version field matches `SL_ZIGBEE_DHC_VERSION`. |
| CLI read commands return zeros | Confirm the JSON file was applied and the NCP build includes the DHC EZSP server. Try `dhc read-all` to see if any data is present. |
| Signature remains unchanged | Run `dhc recompute signature` or write a value explicitly with `dhc set signature <hex32>`. |
| Raw command status byte is non-zero | Ensure the payload length matches the selected setting (1 byte for PA_MODE, 4 bytes for CTUNE, 2 bytes for VOLTAGE, etc.). Check the response interpretation for details. |
| PA mode reads as 0 | Verify with `dhc read-scalars` to see the actual value. If it's 0, it may be uninitialized. Use `dhc set-scalar pa_mode <value>` to set it. |
| CTUNE not persistent after gateway restart or NCP reset | DHC persists ctune to NVM3 when set. Ensure the NCP image includes the Clock Manager Oscillator Calibration Override** component (`clock_manager_oscillator_calibration_override`) so the stored value is applied at platform init. Add the component to the NCP project (.slcp) and rebuild if ctune still reverts. |
| Response shows hex but no interpretation | Ensure you're using the latest build with enhanced raw command output. The interpretation appears on the line following the hex response. |

Enable verbose host logging or EZSP tracing if calibration writes appear to succeed but values revert on reboot. Inspect NVM3 health on the NCP side when repeated writes fail.

## File Inventory

| File | Purpose |
|------|---------|
| `dhc-cli.c` | Implements the CLI shell commands listed above. |
| `dhc-parser.c` | JSON loader with validation and dry-run support. |
| `dhc-ncp.c` | Translation layer to the RAIL PA storage APIs. |
| `dhc-parser.h`, `dhc-ncp.h` | Shared type definitions. |
| `configuration_sample.json` | Ready-to-edit template that matches the parser schema. |

## Quick Reference

### Common Operations

**Apply configuration from file:**
```bash
plugin dhc apply file configuration_sample.json
```

**Read all current values:**
```bash
plugin dhc read-all
```

**Read specific scalar values:**
```bash
plugin dhc read-scalars
```

**Update PA mode:**
```bash
plugin dhc set-scalar pa_mode 4
```

**Update CTUNE:**
```bash
plugin dhc set-scalar ctune 87
```

**Read using raw protocol:**
```bash
plugin dhc raw "DC 00 02"    # Read PA mode
plugin dhc raw "DC 00 03"    # Read CTUNE
plugin dhc raw "DC 00 04"    # Read voltage
```

**Write using raw protocol:**
```bash
plugin dhc raw "DC 01 02 04"           # Write PA mode = 4
plugin dhc raw "DC 01 03 57 00 00 00"  # Write CTUNE = 87
plugin dhc raw "DC 01 04 E4 0C"        # Write voltage = 3300mV
```

### Status Code Reference

| Status Code | Meaning |
|-------------|---------|
| `0x00000000` | Success (SL_STATUS_OK) |
| `0x00000001` | General failure (SL_STATUS_FAIL) |
| `0x0000000E` | Not available (SL_STATUS_NOT_AVAILABLE) |
| `0x0000000F` | Not supported (SL_STATUS_NOT_SUPPORTED) |
| `0x00000021` | Invalid parameter (SL_STATUS_INVALID_PARAMETER) |
| `0x00000022` | Null pointer (SL_STATUS_NULL_POINTER) |
| `0x00000027` | Invalid index (SL_STATUS_INVALID_INDEX) |
| `0x0000002B` | Invalid count (SL_STATUS_INVALID_COUNT) |

**Note:** In raw command responses, only the low byte of the status code is returned. For example, `SL_STATUS_INVALID_PARAMETER` (0x00000021) appears as `0x21` in the raw response.

## Support

For assistance, capture the failing JSON payload, the CLI command sequence, and the host console output (including status codes). File an internal ticket or post on the Silicon Labs Community with the metadata version, descriptor count, and any raw responses observed.