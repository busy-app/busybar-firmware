# Libraries

The libraries can be divided into two main categories: internal and third-party.

Noteworthy internal libraries include:

- `anim_file` - Support for animation files (see [assets/animations](/assets/animations) for more info)
- `cli` - Bespoke command line interface support
- `setting_provider` - JSON-based settings library (standard for BSB firmware)
- `toolbox` - A collection of various small helper libraries

Third-party libraries in this project are most often (but not always) included as submodules:

- `cjson` - JSON parsing
- `heatshrink` - File/data compression
- `lvgl` - Graphical user interface
- `lwip` - TCP/IP stack
- `mongoose` - HTTP/MQTT stack
- `microtar` - `.tar` file format support
- `mbedtls` - SSL/TLS encryption 
- `nanopb` - Protocol buffers (protobuf) parsing
- `thorvg` - Vector graphics
- `tinyusb` - USB stack
- `matter_ext` - Smart home (Matter) protocol 
- `simplicity_sdk` - Vendor SDK for the `SiWG917` wireless chip
- `wiseconnect` - Vendor firmware for the `SiWG917` wireless chip
- `zlib` - File/data compression
