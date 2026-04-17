#!/usr/bin/env python3

# Copyright 2025 Silicon Laboratories Inc. www.silabs.com
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

'''Certificate Authority

Handles signing Certificate Signing Requests for connected embedded devices.

Prerequisites:
Python 3.9 or higher.
Python packages contained in `requirements.txt`.
  Run `pip install -r requirements.txt` to install all requirements for the application.
Simplicity Commander added to path.
Certificate Authority created using the `create_authority_certificate.py` script.
A connected device with the prepared certificate signing request with the
  `SoC - Certificate Signing Request Generator` example.
'''
# Metadata
__author__ = 'Silicon Laboratories, Inc'
__copyright__ = 'Copyright 2025, Silicon Laboratories, Inc.'

import os
import sys
import pathlib
import datetime
import re
import binascii, struct
import argparse
import subprocess
import yaml
from cryptography import x509
from cryptography.hazmat import primitives

RAM_DATA_START            = 0x20007C00
RAM_DATA_LEN_MAX          = 1024
STATIC_AUTH_DATA_LEN      = 32

NVM3_CONTROL_BLOCK_KEY    = { 'ble': 0x40400, 'btmesh': 0x60400 }
NVM3_CONTROL_BLOCK_SIZE   = 17

# Control Block bit fields
CERTIFICATE_ON_DEVICE_BIT = 0
DEVICE_EC_KEY_BIT         = 1
STATIC_AUTH_DATA_BIT      = 2

MAX_ITERATIONS            = 10

def main(level, validity, serial, ip, device, protocol):
    # Check the presence of Simplicity Commander.
    try:
        subprocess.run(['commander', '-v'], stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT) # Supress output
    except FileNotFoundError:
        raise FileNotFoundError('Cannot find Simplicity Commander.')

    # Check arguments.
    if level < 0:
        raise Exception('Authority level should be greater than or equal to zero!')
    if validity < 1:
        raise Exception('Validity should be greater than or equal to 1 [days]!')
    if serial is not None and ip is not None:
        raise Exception('At most one of [J-Link serial number] or [IP address] shall be defined.')
    if protocol not in NVM3_CONTROL_BLOCK_KEY:
        raise Exception('Invalid protocol: ' + protocol + ' (choose from: ' + str(NVM3_CONTROL_BLOCK_KEY.keys()) + ')')

    # Set paths and check certificate authority.
    if level == 0:
        path_auth_dir = os.path.join(os.path.abspath(os.path.join((__file__), '..')), 'central_authority')
    else:
        path_auth_dir = os.path.join(os.path.abspath(os.path.join((__file__), '..')), 'intermediate_' + str(level) + '_authority')
    path_auth_key = os.path.join(path_auth_dir, 'private_key.pem')
    path_auth_cert = os.path.join(path_auth_dir, 'certificate.pem')
    path_auth_database = os.path.join(path_auth_dir, 'issued_certificates.yaml')

    if not os.path.exists(path_auth_dir) or not os.path.exists(path_auth_key) or not os.path.exists(path_auth_cert):
        raise FileNotFoundError(path_auth_dir + ' authority is incomplete.')

    # Retrieve RAM data.
    ram_data = read_ram(RAM_DATA_START, RAM_DATA_LEN_MAX)
    print(f'RAM data retrieved from the device ({hex(RAM_DATA_START)} - {hex(RAM_DATA_START + RAM_DATA_LEN_MAX)}).')

    # Get Certificate Signing Request from RAM data.
    csr = get_csr(ram_data)
    print('Certificate Signing Request retrieved from RAM data.', os.linesep)

    # Retrieve Control Block from NVM3.
    path_nvm3_dump = dump_nvm3(path_auth_dir)
    control_block  = ControlBlock(path_nvm3_dump)

    if control_block.static_auth_present == True:
        # Get Static Authentication Data from RAM dump.
        static_auth = ram_data[1:STATIC_AUTH_DATA_LEN]
        print("Static Authentication Data is ready for further processing.")

    if control_block.device_ec_key_present == False:
        print('EC key pair is missing from the device. It is needed to generate a CSR. Exiting.')
        os.remove(path_nvm3_dump)
        sys.exit(1)

    # Load private key and certificate of the authority.
    with open(path_auth_key, 'rb') as f:
        auth_key = primitives.serialization.load_pem_private_key(f.read(), password=None)
    with open(path_auth_cert, 'rb') as f:
        auth_cert = x509.load_pem_x509_certificate(f.read())

    # Sign the request and create the Device Certificate.
    cert = sign_csr(auth_key, auth_cert, path_auth_database, csr, validity)

    # Exit if the device is not configured to hold the certificate.
    if control_block.certificate_on_device == False:
        print('The device is not configured to hold the certificate. The certificate file is ready for further processing. Exiting.')
        os.remove(path_nvm3_dump)
        sys.exit(0)

    # Add the signed certificate to the NVM3 image according to the Control Block.
    patch_nvm3(path_nvm3_dump,
               cert.public_bytes(primitives.serialization.Encoding.DER), # Export in binary (DER) format
               control_block.nvm3_key[CERTIFICATE_ON_DEVICE_BIT],
               control_block.max_link_data_len)

    # Flash back the modified NVM3 content onto the device.
    subprocess_call(('commander flash ' + path_nvm3_dump + ' ' + device_argument()).split())
    print('Certificate flashed onto the device.')
    os.remove(path_nvm3_dump) # Clean up

def read_ram(start_address, memory_size):
    '''Get data from the RAM in the given range.

    Keyword arguments:
    start_address -- Address to start reading from.
    memory_size -- Memory size to read.
    Return values:
    The retrieved data from the RAM.
    '''
    cmd = ('commander readmem '
           + device_argument()
           + ' --range ' + hex(start_address) + ':+' + hex(memory_size)).split()
    output = subprocess_call(cmd).stdout

    # Grab raw bytes from the output.
    output = re.findall(r'^([0-9A-Fa-f]{8}):(\s--)*((\s[0-9A-Fa-f]{2}){1,16})(\s--)*',
                        output.decode('utf-8'),
                        re.MULTILINE)
    s = ''

    for item in output:
      s = s + item[2].replace(' ', '')

    return binascii.unhexlify(s)

def get_csr(ram_data):
    '''Get Certificate Signing Request.

    Keyword arguments:
    ram_data -- Data retrieved from the RAM using read_ram
    Return values:
    The Certificate Signing Request
    '''
    # Select the certificate signing request.
    is_complete = ram_data[0] == 1 # Completion bit
    tuple = struct.unpack('<H', ram_data[STATIC_AUTH_DATA_LEN + 1 : STATIC_AUTH_DATA_LEN + 3])
    csr_len = tuple[0]

    if is_complete == False or csr_len == 0:
        raise Exception('Certificate signing request generation is incomplete!')

    csr_bytes = ram_data[STATIC_AUTH_DATA_LEN + 3 : STATIC_AUTH_DATA_LEN + 3 + csr_len]
    return x509.load_der_x509_csr(csr_bytes)

def sign_csr(authority_key, authority_cert, path_auth_database, csr, validity):
    '''Create a device certificate by signing a Certificate Signing Request and modifying the necessary fields.
    Update the list of the issued certificates.

    Keyword arguments:
    authority_key -- The private key of the certificate authority that shall sign the request.
    authority_cert -- The certificate authority that shall sign the request.
    path_auth_database -- Path to a file containing the database of the issued certificates.
    csr -- The device certificate signing request.
    validity -- The valid period of the certificate in days starting
                from the moment of signing. (Default: 365)
    Return values:
    The signed device certificate.
    '''

    # Check the validity of the higher authority.
    # Note: if there is a revocation list, it should be also checked if the certificate is revoked or not.
    now = datetime.datetime.now(datetime.timezone.utc)
    if now < authority_cert.not_valid_before_utc or authority_cert.not_valid_after_utc < now:
        raise Exception('The validity period of the CA has expired.')

    # Check if the CSR signature is valid
    if not csr.is_signature_valid:
        raise Exception('Invalid CSR signature!')

    # Create a new certificate from the CSR.
    cert = (x509.CertificateBuilder()
        .subject_name(csr.subject)
        .issuer_name(authority_cert.subject)
        .public_key(csr.public_key())
        .serial_number(generate_serial_number(path_auth_database))
        .not_valid_before(now)
        .not_valid_after(now + datetime.timedelta(days=validity)))

    # Device certificates should not act as intermediate certificates (authorities)
    cert = cert.add_extension(x509.BasicConstraints(ca=False, path_length=None), True)

    # Add other extensions from the CSR
    for ext in csr.extensions:
        if ext.value.oid != x509.BasicConstraints.oid:
            cert = cert.add_extension(ext.value, ext.critical)

    # Sign certificate with the private key of the authority.
    cert = cert.sign(authority_key, csr.signature_hash_algorithm)

    # Verify signature.
    authority_cert.public_key().verify(cert.signature,
                                       cert.tbs_certificate_bytes,
                                       primitives.asymmetric.ec.ECDSA(csr.signature_hash_algorithm))

    # Check if the public key can be used for key agreement (needed for secure pairing).
    if cert.extensions.get_extension_for_class(x509.KeyUsage).value.key_agreement != True:
        raise Exception("KeyUsage key_agreement should be set to True!")

    add_certificate_to_database(cert, path_auth_database)
    return cert

def generate_serial_number(path_database):
    ''' Generate a unique serial number which is not present in the database.

    Keyword arguments
    path_database -- Path to the database file.
    Return values:
    The serial number.
    '''
    try:
        # Load file if present.
        with open(path_database, mode="r", encoding="utf-8") as stream:
            issued = yaml.safe_load(stream)
    except:
        # No database, return random.
        return x509.random_serial_number()

    for _ in range(MAX_ITERATIONS):
        serial = x509.random_serial_number()
        if serial not in issued:
            return serial

    raise RuntimeError("Failed to generate serial number. Max iteration reached:", MAX_ITERATIONS)

def add_certificate_to_database(certificate, path_database):
    ''' Update the list of the issued certificates.

    Keyword arguments
    certificate -- The certificate to add.
    path_database -- Path to the database file.
    '''
    try:
        # Load file if present.
        with open(path_database, mode="r", encoding="utf-8") as stream:
            issued = yaml.safe_load(stream)
    except:
        issued = {}

    # Check if entry with the same serial exists.
    if certificate.serial_number in issued:
        raise ValueError("Database already contains a certificate with the same serial number:", certificate.serial_number)

    # Create new entry.
    issued[certificate.serial_number] = \
        [{"common_name": certificate.subject.get_attributes_for_oid(x509.oid.NameOID.COMMON_NAME)[0].value},
         {"public_key": certificate.public_key().public_bytes(primitives.serialization.Encoding.DER, primitives.serialization.PublicFormat.SubjectPublicKeyInfo)},
         {"CA": certificate.extensions.get_extension_for_class(x509.BasicConstraints).value.ca}]

    with open(path_database, mode = "w", encoding="utf8") as stream:
        # Write back the updated database to file.
        yaml.dump(issued, stream, default_flow_style=False, allow_unicode=True)

    # Save the signed certificate to file in PEM format.
    path_cert_pem = os.path.abspath(os.path.join(os.path.join(path_database, '..'), str(certificate.serial_number) + '.crt'))
    with open(path_cert_pem, 'wb') as f:
        f.write(certificate.public_bytes(primitives.serialization.Encoding.PEM))
    print('Signing completed. Certificate is stored in:', path_cert_pem)

def dump_nvm3(path_dir):
    '''Retrieve NVM3 content in s37 format.

    Keyword arguments:
    path_dir -- Path to working directory where the NVM3 image file shall be created.
    Return values:
    Path to the NVM3 image file.
    '''
    path_nvm3_dump = os.path.join(path_dir, 'nvm3_dump.s37')

    # Remove temp file if exist
    if os.path.exists(path_nvm3_dump):
        print(path_nvm3_dump, 'already exists. Removing.')
        os.remove(path_nvm3_dump)

    # Search for the NVM3 area in the device's flash and dump the content to file.
    subprocess_call(('commander nvm3 read ' + device_argument() + ' -o ' + path_nvm3_dump).split())
    return path_nvm3_dump

def patch_nvm3(path_nvm3_dump, data, nvm3_start_key, nvm3_object_size):
    '''Create an NVM3 patch file containing the given data and apply on the NVM3 image.

    Keyword arguments:
    path_nvm3_dump -- Path to the NVM3 image.
    data -- The data to add to the NVM3 image.
    nvm3_start_key -- NVM3 objects will be created or overwritten starting from this key.
    nvm3_object_size -- The size of each NVM3 object data.
    '''

    # Temporary file to help patching the NVM3 dump file.
    path_nvm3_patch = str(pathlib.Path(os.path.splitext(path_nvm3_dump)[0] + '.patch'))

    # Create patch file
    with open(path_nvm3_patch, 'w') as f:
        nvm3_key = nvm3_start_key

        while nvm3_key != 0:
            # Select next chunk.
            if len(data) >= nvm3_object_size:
                length = nvm3_object_size
            else:
                length = len(data)

            chunk = data[:length]
            data = data[length:]

            if len(data) > 0:
                next_nvm3_key = nvm3_key + 1
                header = 0x0001 | 2 | next_nvm3_key << 2
            else: # The last chunk is being processed.
                next_nvm3_key = 0
                header = 0x0001

            # Construct next line and write to file.
            key = NVM3_CONTROL_BLOCK_KEY[args.protocol] | nvm3_key # Add key prefix.
            data_chunk = struct.pack(f'<HHH', header, 0x0001, length) + chunk # Pack data chunk.
            f.write(hex(key) + ' : OBJ : ' + binascii.hexlify(data_chunk).decode('utf-8') + '\n') # <key>:<type>:<data>

            nvm3_key = next_nvm3_key

    # Apply patch
    subprocess_call(['commander', 'nvm3', 'set', path_nvm3_dump, '--nvm3file', path_nvm3_patch, '--outfile', path_nvm3_dump])
    os.remove(path_nvm3_patch)

class ControlBlock():
    def __init__(self, path_nvm3_dump):
        '''Get Control Block from an NVM3 image file.

        Keyword arguments:
        path_nvm3_dump -- Path to the NVM3 image.
        Return values:
        ControlBlock -- The retrieved and parsed Control Block.
        '''
        nvm3_object = self.get_nvm3_object(path_nvm3_dump)
        self.parse_data(nvm3_object)
        self.validate()

    def get_nvm3_object(self, path_nvm3_dump):
        # Read NVM3 object from the NVM3 image that belongs to the control block key.
        cmd = ('commander nvm3 parse ' + path_nvm3_dump + ' --key ' + str(NVM3_CONTROL_BLOCK_KEY[args.protocol])).split()
        nvm3_object = subprocess_call(cmd).stdout.decode('utf-8')

        # Check Control Block key.
        key = re.findall(r'^Key\s+:\s+(0x\d+).*$', nvm3_object, re.MULTILINE)

        if len(re.findall(r'^Found NVM3 range:', nvm3_object, re.MULTILINE)) == 0 or len(key) == 0:
            raise Exception('NVM3 range cannot be found.')

        self.key = key[0]
        return nvm3_object

    def parse_data(self, nvm3_object):
        # Get raw bytes of the Control Block.
        data = ''

        for item in re.findall(r'^([0-9A-Fa-f]{8}):((\s[0-9A-Fa-f]{2}){1,16})', nvm3_object, re.MULTILINE):
            data = data + item[1].replace(' ', '')

        data = binascii.unhexlify(data)

        if data == None or len(data) != NVM3_CONTROL_BLOCK_SIZE:
            raise Exception('Invalid Control Block!')

        # Unpack the data of the NVM3 object.
        self.nvm3_key = [0, 0, 0]
        (self.header,
         self.next_control_block,
         bitmap,
         self.nvm3_key[0], self.nvm3_key[1], self.nvm3_key[2],
         self.max_link_data_len) = struct.unpack('<HHQbbbH', data)
        self.version = self.header & 0xf000 # Upper 4 bits of the header contain version number. Other bits are reserved for future use.
        self.next_control_block |= NVM3_CONTROL_BLOCK_KEY[args.protocol] & 0xF0000 # Only stored on two bytes. Adding prefix.

        print(f'Control Block retrieved from device NVM3 ({self.key}).')
        print('\tVersion:', self.version)
        print('\tHeader:', hex(self.header))
        print('\tMaximum link data length:', self.max_link_data_len)
        print('\tNext Control Block:', hex(self.next_control_block))

        # Inspect bitmap.
        print(f'\tConfiguration: {format(bitmap, "#06b")}')

        self.certificate_on_device = bool(bitmap & (1 << CERTIFICATE_ON_DEVICE_BIT))
        print('\t\tCertificate on device required:', self.certificate_on_device)

        self.device_ec_key_present = bool(bitmap & (1 << DEVICE_EC_KEY_BIT))
        print('\t\tEC key pair present:', self.device_ec_key_present)

        self.static_auth_present = bool(bitmap & (1 << STATIC_AUTH_DATA_BIT))
        print('\t\tStatic Authentication data present:', self.static_auth_present, os.linesep)

    def validate(self):
        # Expecting zero as the version number.
        if self.version != 0:
            raise Exception('Invalid Control Block! Version: ', self.version, '. "0" is expected.')

        # Expecting only one control block. Therefore it should point onto itself.
        if self.next_control_block != NVM3_CONTROL_BLOCK_KEY[args.protocol]:
            raise Exception('Invalid Control Block! Next Control Block NVM3 key: ',
                            hex(self.next_control_block), '. ',
                            hex(NVM3_CONTROL_BLOCK_KEY[args.protocol]), ' is expected.')

        # Check the security of NVM3 ITS data.
        if self.device_ec_key_present and self.nvm3_key[DEVICE_EC_KEY_BIT] != 0:
            raise Exception('EC key pair is exposed. Any data stored in ITS should not be accessible!')

        if self.static_auth_present and self.nvm3_key[STATIC_AUTH_DATA_BIT] != 0:
            raise Exception('Static Authentication data is exposed. Any data stored in ITS should not be accessible!')

def subprocess_call(command):
    '''Handle subprocess calls.

    Keyword arguments:
    command -- The CLI command to be called.
    Return values:
    process -- The CompletedProcess instance of the subprocess.
    '''
    try:
        process = subprocess.run(command,
                                 check=True,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT) # Merge stderr into stdout.
    except subprocess.CalledProcessError as e:
        raise Exception('[E: ', e.returncode, '] ', e.stdout)
    except:
        raise Exception('[E: ', e.returncode, '] ', e.stdout)
    return process

def device_argument():
    '''Returns the device argument of a Simplicity Commander command.
    '''
    arg = ''

    if args.serial is not None and args.ip is not None:
        raise Exception('At most one of [J-Link serial number] or [IP address] shall be defined.')
    elif args.serial is not None:
        arg += '--serialno ' + args.serial
    elif args.ip is not None:
        arg += '--ip ' + args.ip

    if args.device is not None:
        arg += ' --device ' + args.device

    return arg

class CustomFormatter(argparse.ArgumentDefaultsHelpFormatter, argparse.RawDescriptionHelpFormatter):
    pass

def load_args():
    '''Parse command line arguments.'''
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=CustomFormatter)

    parser.epilog = (f'examples:{os.linesep}'
                     f'\t%(prog)s                     Try to autodetect device.{os.linesep}'
                     f'\t%(prog)s --serial 440192051  Connect to device with the given J-Link serial.{os.linesep}'
                     f'\t%(prog)s --ip 192.168.0.143  Connect to device with the given IP address.')

    parser.add_argument('-l', '--level',
                        default='0',
                        type=int,
                        help='The level authority that shall sign the request.')
    parser.add_argument('-v', '--validity',
                        default='365',
                        type=int,
                        help='The valid period of the certificate in days starting '\
                             'from the moment of signing.')
    parser.add_argument('-s', '--serial',
                        help='J-Link serial number. Should not be given together with the IP address.')
    parser.add_argument('-i', '--ip',
                        help='IP Address. Should not be given together with the J-Link serial number.')
    parser.add_argument('-d', '--device',
                        help='The device, device family or platform to target. Only needed for custom hardware.')
    parser.add_argument('-p', '--protocol',
                        default='btmesh',
                        type=str.lower,
                        choices=['ble', 'btmesh'],
                        help='Determines which NVM3 region to use.')

    args = parser.parse_args()
    return args

if __name__ == '__main__':
    args = load_args()
    main(args.level, args.validity, args.serial, args.ip, args.device, args.protocol)
