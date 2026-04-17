# Host voice

## Features

This application demonstrates voice data transfer from boards with integrated
microphone. Samples are written to a file, or to the standard output.

## Requirements

- Target board with microphone running the Bluetooth - SoC voice example application

## Usage

- Generate and build the application
- Connect the target with the Bluetooth - SoC voice example
- Start the host example application

To start recording and streaming audio data over the BLE link, press and hold BTN0. Once the button is pressed, you should see activity on the terminal window that summarizes the transmission progress. All the audio data will be saved to a file defined by the output filename -parameter (audio_file.ima in this example). If a file with a same name already exists, the audio data is appended to the end of that file.

When BTN0 is released, the transmission is paused and can be resumed by pressing BTN0 again. If the SoC board is reset, the connection will be terminated and the application will be closed.

### Typical invocation

```bash
bt_host_voice -u /dev/tty.usbmodem0004403482281 -o <file_name>
```

Replace <file_name> with your output file.

To convert the audio samples to WAV format, consult the Readme.md file in the scripts
folder.
