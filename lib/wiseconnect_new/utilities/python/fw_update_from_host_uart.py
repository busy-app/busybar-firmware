import serial
import time
import os
import sys
from print_color import print  # For Debug prints

# Constants
COM_PORT = 'COM3'  # Update the actual COM port number
BAUD_RATE = 115200  # Replace with your desired baud rate
MAX_BYTES_TO_READ = 1024
HEADER_SIZE = 64

# Function to perform handshake with MCU
def perform_handshake(ser):
    
    # Send acknowledgment to MCU
    ser.write(b"Python Ready for firmware update process")
    print(" >>>> Sent 'Python Ready for firmware update process' to MCU")

    while True:
        # Read any data sent by MCU
        received_data = ser.readline().strip()
        if received_data == b"Host is ready for Firmware update process":
            print(" <<<< Received 'Host is ready for Firmware update process' from MCU")           
            break
        else:
            print("Waiting for 'MCUReady' from MCU ...")

# Function to send the firmware file in big-endian hex format
def send_firmware(file_path, chunk_size_kb):
    i = 0
    sent_data = 0
    remaining_data = 0
    start_time = 0
    firmware_transmission_complete = False

    try:
        # Calculate the firmware file size
        firmware_size = os.path.getsize(file_path)

        with open(file_path, 'rb') as firmware:
            # Create a serial connection with Micocontroller
            with serial.Serial(COM_PORT, BAUD_RATE, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, xonxoff=False, rtscts=False, dsrdtr=False, timeout=1) as ser:
                ser.flushInput()  # Clear input buffer
                print(f"Serial port {COM_PORT} opened at {BAUD_RATE} baud with no parity, 1 stop bit, and hardware flow control (CTS/RTS).")

                # Perform handshake with MCU
                perform_handshake(ser)

                # Convert chunk size from kilobytes to bytes
                chunk_size = chunk_size_kb * 1024

                while True:
                    received_data = ser.read(MAX_BYTES_TO_READ)
                    
                    print(f" >>>>Rx Data:",received_data)
                    
                    if received_data:
                        if received_data == b"header":
                            start_time = time.time()  # Record the start time
                            print(" <<<< Received 'header' command from MCU", tag='success', tag_color='green', color='cyan')
                            header = firmware.read(HEADER_SIZE)
                            ser.write(header)
                            print("Sent 64-byte header to MCU")
                        elif received_data == b"data":
                            print(" <<<< Received 'data' command from MCU. Sending firmware data.", tag='success', tag_color='green', color='cyan')
                            try:
                                firmware.seek(i)  # Update file pointer
                                remaining_data = firmware_size - sent_data
                                current_chunk_size = min(remaining_data, chunk_size)
                                data = firmware.read(current_chunk_size)
                                ser.write(data)
                                sent_data += current_chunk_size
                                i += chunk_size
                                print(f" >>>> Sent {sent_data} out of {firmware_size} bytes of firmware data to MCU ({sent_data / firmware_size:.1%}):")
                                #,data[:16].hex(' ')
                                if sent_data >= firmware_size:
                                    end_time = time.time()  # Record the end time
                                    elapsed_time = end_time - start_time
                                    print(f"Firmware update took {elapsed_time:.2f} seconds.")
                                    print("Firmware transmission complete.")
                                    firmware_transmission_complete = True

                            except IOError as e:
                                print(f"Error: I/O error while reading firmware file: {e}")
                            except Exception as e:
                                print(f"Error: An unexpected error occurred: {e}")
                        
                        elif received_data == b"done":
                                print(" <<<< Received 'done' command from MCU. Firmware transmission complete. Terminating Script", tag='success', tag_color='green', color='cyan')
                                break
                        else:
                            print(" <<<< Received data from MCU:", received_data.decode('utf-8', errors='replace'),tag='MCU LOG PRINT', tag_color='red', color='red')

                    else:
                        if not firmware_transmission_complete:
                            print("Waiting for data from MCU ...")

    except FileNotFoundError:
        print(f"Error: Firmware file not found: {file_path}")
    except serial.serialutil.SerialException as e:
        print(f"Error: Failed to open the serial port: {e}")
    except Exception as e:
        print(e)

if __name__ == "__main__":
    # User Input for Firmware File
    if len(sys.argv) != 2:
        print("Usage: python script.py <firmware_file_path>")
        sys.exit(1)

    firmware_file = sys.argv[1]
    chunk_size_kb = 1

    send_firmware(firmware_file, chunk_size_kb)
