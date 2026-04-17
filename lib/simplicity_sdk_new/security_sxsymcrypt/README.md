# Secure-IC Symmetric Crypto Offload C Library

Software for symmetric cryptography offload to Secure-IC hardware.

## Introduction

[Secure-IC](https://www.secure-ic.com) develops, validates and
licenses a whole set of hardware blocks to off-load symmetric
cryptography operations.

This software library provides a simple programming interface to offload
encryption, decryption or hashing to hardware offload modules. It works
well in a baremetal environment or similarly simple setups.

Currently the following algorithms and cipher modes are supported :

* AES GCM encryption and decryption with the Secure-IC CryptoSoc Accelerator
  including a scalable AES-GCM (BA415) IP core
* AES XTS encryption and decryption with the Secure-IC CryptoSoc Accelerator
  including a scalable AES-XTS (BA416) IP core
* AES ECB, CBC, OFB, CFB, CTR, XTS, GCM/GMAC, CCM/CCM* encryption and decryption with
  the Secure-IC CryptoSoc Accelerator including the scalable AES (BA411) IP core
* AES CMAC MAC generation and validation with the Secure-IC CryptoSoc Accelerator
  including the scalable AES (BA411) IP core
* SM4 ECB, CBC, OFB, CFB, CTR, XTS, GCM, CCM encryption and decryption with
  the Secure-IC CryptoSoc Accelerator including the scalable SM4 (BA419) IP core
* SM4 CMAC MAC generation with the Secure-IC CryptoSoc Accelerator
  including the scalable SM4 (BA419) IP core
* SHA-1, SHA-2, SM3 hashing and SHA-1, SHA-224, SHA2-256, SHA2-384 and SHA2-512 based HMAC
  with the Secure-IC CryptoSoc Accelerator including the Hash (BA413) IP core
* Chacha20-Poly1305 AEAD and ChaCha20 stream cipher with the Secure-IC CryptoSoc Accelerator
  including the ChaCha20-Poly1305 (BA417) Crypto Engine
* Triple DES (also called TDES) with the Secure-IC CryptoSoc Accelerator
  including the 3DES (BA412) Crypto Engine
* ZUC 3GPP confidentiality (EEA3) and integrity (EIA3) algorithms with the Secure-IC CryptoSoc Accelerator
  including the ZUC (BA421) Crypto Engine
* SNOW 3G 3GPP confidentiality (UEA2) and integrity (UIA2) algorithms with the Secure-IC CryptoSoc Accelerator
  including the SNOW3G (BA423) Crypto Engine
* Kasumi 3GPP confidentiality (UEA1) and integrity (UIA1) algorithms and keystream
  generator functions (GSM A5/3, ECSD A5/3, GEA3, GSM A5/4, ECSD A5/4, GEA4)
  with the Secure-IC CryptoSoc Accelerator including the KASUMI (BA422) Crypto Engine
* ARIA ECB, CBC, OFB, CFB, CTR, GCM, CCM encryption and decryption with
  the Secure-IC CryptoSoc Accelerator including the scalable ARIA (BA424) IP core
* ARIA CMAC MAC generation the Secure-IC CryptoSoc Accelerator
  including the scalable ARIA (BA424) IP core
* Data transfer with the the Secure-IC CryptoSoc Accelerator

This library also supports Non-deterministic Random Number Generator TRNG (BA431)

The API has a native asynchronous non-blocking interface. But it can
also be used very easily in a blocking synchronous way.

To ease development, the library can also work with full software
emulation. That way, a developer can write, run and debug applications
directly on his development computer.


## Platforms and Build

To interface with the platform the code runs on, the library uses the
concept of platforms. A platform implements basic functionalities like
read and write registers. Software emulation is another example of how
platforms are used.

The header files "src/hw.h" defines the functions expected from a
platform implementation.


### Baremetal Platform

The implementation is located in "src/platform/baremetal". To build
with this implementation, call make with "PLATFORM=baremetal".

It expects the hardware registers memory mapped at SX_CM_REGS_ADDR.
Multiple hardware instances are supported. The constant SX_CM_REGS_STRIDE tells
the registers address offset between each instance of the hardware. For each
instance of the hardware, an element needs to be added to the "hwregs" array.
The default implementation expects one instance.
Example, "hwregs" with 2 instances:

    static const struct sx_regs hwregs[] = {
        {
            .devmem = SX_CM_REGS_ADDR,
        },
        {
            .devmem = SX_CM_REGS_ADDR + 1 * SX_CM_REGS_STRIDE,
        }
    };

The constant SX_ADDR2BUS defines the offset to apply to CPU addresses to convert
them to bus addresses for the DMA engines.

This platform serves as an example to write one's custom baremetal
platform.

Support for availability checks can be controlled by preprocessor define
SX_BAREMETAL_AVAILABILITY_CHECK.
If SX_BAREMETAL_AVAILABILITY_CHECK is undefined or nonzero, the support for
availability checks will be included.
If SX_BAREMETAL_AVAILABILITY_CHECK is 0, the hardware will be checked if busy,
no internal memory will be used.

Support for DMA reserved memory can be controlled by preprocessor define:
"SX_DMAMEM_RESERVE_MEM_SZ". When used, this define must have a numeric value.
If "SX_DMAMEM_RESERVE_MEM_SZ" is not defined, global dmamem in baremetal.c will
be used with default value of 2048.
If value is 0, global memory defined in baremetal.c is removed the user must
allocate memory for dmamem.
If value is nonzero, global memory in baremetal.c will be used with
size given by "SX_DMAMEM_RESERVE_MEM_SZ".

In order to build baremetal platform without using DMA global memory:

    make PLATFORM=baremetal CFLAGS="-Werror -Wextra -Os" CPPFLAGS="-DSX_DMAMEM_RESERVE_MEM_SZ=0 -DSX_BAREMETAL_AVAILABILITY_CHECK=0"

Depending on the target architecture and platform, the baremetal
platform can be further customized by the integrator. Inline comments
in the platform implementation tagged with the word "CUSTOMIZATION"
explain the most likely adaptations. When using a specific CPU
architecture, the memory barriers for that CPU should be added in
"target/include/membarriers.h". If needed, the integrator can also
create a new platform based on the baremetal one.


### Software Emulation Platform: emulcifra

To build, call make:

~~~
  make
~~~

#### Limitations
The emulator only supports SM4 ECB, CBC, CTR, GCM and CCM modes. SM4 CFB, OFB,
XTS and CMAC are not implemented, though available in the BA419 hardware.
The emulator does not support ARIA.

### Linux Baremetal Emulation Platform: lnxslxidmem

With the help of the included 'slxidmem' kernel driver, this platform
is used to emulate a baremetal environment on top of Linux. It is a very
convenient platform to develop, debug or run programs based on sxsymcrypt
on FPGA SoCs like the Xilinx Zedboard or ZCU102.  The programs are executed
on the processing system (microprocessor running a Linux distribution) while
having the Secure-IC CryptoMaster(s) loaded in the programmable logic.

If cross compiling, the cross compiler should be defined in the
environment variable CROSS_COMPILE.

This platform has defines which need to match the hardware:

 * SX_TOTAL_HW: How many CryptoMaster instances are included in the hardware.
 * SX_CM_REGS_STRIDE: The offset between the start address of the registers of cryptomaster instances.

The default values for these platform defines can be overridden via
CPPFLAGS.

To build the kernel driver, the library and the samples :

~~~
    export CROSS_COMPILE=<cross_compiler>
    cd src/platform/lnxslxidmem/driver
    make KDIR=<linux_kernel_dir> ARCH=<arch>
    cd ../../../..
    make PLATFORM=lnxslxidmem CPPFLAGS="-DSX_TOTAL_HW=2 -DSX_CM_REGS_STRIDE=0x1000"
~~~

On the target board, the device tree should define the hardware to use.
That can be written in the main device tree loaded at start up by the
bootloader or at runtime if the Linux kernel ships with device tree
overlays. Example device-tree section :

~~~
/dts-v1/;
/plugin/;

/ {
    fragment@0 {
        target = <&amba>;
        __overlay__ {
                #address-cells = <0x2>;
		#size-cells = <0x2>;
		cryptomasters: cryptomasters@A40C0000 {
                	compatible = "slxidmem";
                	interrupt-parent = <&gic>;
                	reg = <0x00 0xA40C0000 0x00 0x1000>;
     	       };
        };
   };
};
~~~

Which can be compiled into a binary device tree overlay with the 'dtc'
of the target Linux system :

~~~
 dtc -O dtb -o slxidmem.dtbo -b 0 -@  slxidmem.dts
~~~

A device tree overlay can be loaded with the configfs filesystem before
loading the kernel driver, for example :

~~~
 mkdir -p /config
 mount -t configfs none /config/
 mkdir -p /config/device-tree/overlays/fpga
 echo slxidmem.dtbo > /config/device-tree/overlays/fpga/path
 insmod slxidmem.ko
~~~

# Integrations

## OpenSSL engine

Secure-IC has developed an OpenSSL engine on top of sxsymcrypt library that allows
OpenSSL to offload the symmetric crypto operations on the BA450 Crypto Master.

The engine, *sic_ossl*, is developed to support the OpenSSL V1.1w version.

### Supported algorithms

The *sic_ossl* engine can offload the following cryptographic operations:

* AES GCM 128 and 256 bit key with single shot
* AES XTS 128 and 256 bit key with single shot
* SHA3 224, 256, 384, and 512 bit with single shot and context saving

### How to build

The OpenSSL *sic_ossl* engine can be built using the dedicated script "*openssl_v1.sh*".
This script needs to be called from the root of the library.
The script will download the OpenSSL archive, untar it in the root directory of the library and
will start compiling the OpenSSL using the compiler specified by *CROSS_COMPILE* prefix.
When compiling OpenSSL, *OPENSSL_TARGET*, needs to be specified(if ommitted, the default value
will be used).
After OpenSSL is compiled, the sxsymcrypt library and *sic_ossl* engines are compiled for
the PLATFORM specified.

The result can be found in ossl/staging/tmp/openssl11/:

* sic_ossl - lib/engines-1.1/
* openssl executable - bin/

Environment variables needed for building:

* CROSS_COMPILE - prefix for the compiler to be used, default will be the system compiler
* OPENSSL_TARGET - OpenSSL target to be used, default platform  is *linux-x86_64*
* PLATFORM - sxsymcrypt platform to compile for, default platform is *emulcifra*

### How to use

After the *openssl_v1.sh* script was used, one can run a simple SHA3 test by loading
the engine using the CLI.
As a precondition, the lib folder of OpenSSL must be set in environment variables

~~~
export LD_LIBRARY_PATH="path_to_sxsymcrypt_dir"/ossl/staging/tmp/openssl11/lib:$LD_LIBRARY_PATH
~~~

To run a simple test using the *sic_ossl* engine:

~~~
cd ossl/staging/tmp/openssl11/bin/
echo whatever | ./openssl dgst -engine ../lib/engines-1.1/sic_ossl.so -sha3-512
~~~

### Samples

We are providing a sample app that shows how to use the engine without using the CLI.
In this sample we test all digests, AES GCM 256-bit key encrypt and decrypt, and
AES XTS 128-bit key encrypt and decrypt.

The sample can be found in integrations/ossl/engine/samples/sic_ossl_engine.c
This sample is automatically built with the *openssl_v1.sh* script.

As a prerequisite, the user must set *OPENSSL_ENGINES* environment variable. If not set,
the *sic_ossl* engine will not be loaded.

~~~
export OPENSSL_ENGINES="path_to_sxsymcrypt_dir"/sxsymcrypt_fork/ossl/staging/tmp/openssl11/lib/engines-1.1/
~~~


### Known limitations

* AES GCM(BA415) and AES XTS(BA416) single shot caused by HW not compatible with
context saving.
* AES GCM, when passing AAD and data, the pointers to these buffers must be different
and not overlap. This is because the operation is done single shot and all the data
must be available with the final call.
* AES XTS, the key buffer must contain both keys involved in XTS operation, one
after the other.
* OpenSSL does not allow AES GCM and XTS to be executed in CLI
* All the other library limitations are applicable here


# API


## General Principles

The operations are grouped by category of how they are used. The categories
are listed in the next chapter.

Each operation is started asynchronously. The pending operation is represented
by an object typed according to the category of the operation. Such object is
created using operation-specific functions named following the pattern
sx\_*category*\_create\_*mode*().
The process concludes either on error or upon receiving completion for the
operation with sx\_*category*\_wait() or sx\_*category*\_status().
Users can employ AES generic functions, sx\_*category*\_create\_aes\_generic(), to create operation objects. These functions provide the flexibility to pass configuration parameters, enabling advanced features like AES temporal redundancy.

The specific algorithm/mode of operation is defined by a (single) creation
function. That function obtains all resources needed. Only the creation
function is specific to the algorithm. The other functions are general for all
operations of the category.

For maximum efficiency, the sxsymcrypt library avoids copying memory.
Buffers given to sxsymcrypt functions must remain valid until a sx_*_status()
or a sx*_wait() function returned a completion status code (all besides
SX_ERR_HW_PROCESSING).

## Categories

The operations supported by sxsymcrypt can be divided into the following
main categories:

* Simple block cipher modes, abbreviated here as 'blkcipher'. Those
  modes start with a key and an IV. After that a user can encrypt a
  plaintext or decrypt a ciphertext.
  The header files
  "include/sxsymcrypt/blkcipher.h",
  "include/sxsymcrypt/sm4.h",
  "include/sxsymcrypt/tdes.h",
  "include/sxsymcrypt/aria.h" and
  "include/sxsymcrypt/chachapoly.h"
  define the API for those modes.
* Authenticated encryption with associated data, also called
  [AEAD](https://en.wikipedia.org/wiki/Authenticated_encryption).
  In addition to keys, IV plaintext or ciphertext, AEAD encryption and
  decryption have authentication tags and additional data (sometimes
  called headers). The authentication tag covers the additional data
  and the (encrypted) plaintext.
  The header files
  "include/sxsymcrypt/aead.h",
  "include/sxsymcrypt/chachapoly.h" and
  "include/sxsymcrypt/aria.h"
  define the API for those modes.
* Keyed message authentication code generation without encryption.
  The header files
  "include/sxsymcrypt/mac.h",
  "include/sxsymcrypt/hmac.h",
  "include/sxsymcrypt/cmac.h",
  "include/sxsymcrypt/aria.h"
  define the API.
* Hashing, create a cryptographic hash (also called digest) of a message.
  The header files
  "include/sxsymcrypt/hash.h" and
  "include/sxsymcrypt/sha3.h"
  define the API.
* TRNG, Hardware true random number generation.
  The header file
  "include/sxsymcrypt/trng.h" defines the API.
* Channel feature.
  The header file
  "include/sxsymcrypt/channel.h" defines the API.
* Transfer of data(based on channel).
  The header file
  "include/sxsymcrypt/transfer.h" defines the API.

## Hardware interaction

The functions that interact with the hardware can be split into multiple
categories:

* Functions that start the cryptographic operation, non-blocking  (return
immediately after starting the operation):
    - sx_aead_produce_tag(), sx_aead_verify_tag(), sx_aead_resume_state() or sx_aead_save_state()
    - sx_blkcipher_run(), sx_blkcipher_resume_state() or sx_blkcipher_save_state()
    - sx_hash_digest(), sx_hash_resume_state() or sx_hash_save_state()
    - sx_cm_load_mask()
    - sx_mac_generate(), sx_mac_resume_state() or sx_mac_save_state()
    - sx_channel_run()

* Functions that check operation status, non-blocking  (return immediately after
checking the operation status):
    - sx_aead_status()
    - sx_blkcipher_status()
    - sx_hash_status()
    - sx_cm_mask_status()
    - sx_mac_status()
    - sx_channel_status()

* Functions that check operation status, blocking until operation ended (with
success, or with error):
    - sx_aead_wait()
    - sx_blkcipher_wait()
    - sx_hash_wait()
    - sx_cm_mask_wait()
    - sx_mac_wait()
    - sx_wait_status()

* Functions that read or write configuration registers for specific modules,
non-blocking :
    - sx_interrupts_enable() or sx_interrupts_disable()

* Functions to initialize and configure the TRNG as well as get data in a non-blocking way:
   - sx_trng_init()
   - sx_trng_get()

All other functions that are not listed above do not interact with the hardware
as they are used for preparing the cryptographic operation.

## Overview


An operation needs an instance of the structure corresponding to the
main mode, for example sxblkcipher for a blkcipher operation :

~~~
 struct sxblkcipher c = {};
~~~

For the block cipher(or AEAD) operations, the user needs to provide the
reference to the key/keys. For more detailes see "Key reference".
A key reference instance of the structure must be created:

~~~
 struct sxkeyref key;
~~~

Next, the key reference structure needs to be initialized by using
sx_keyref_load_material(), for the case of AES XTS the function needs to be
called for each of the two keys.

~~~
 struct sxkeyref key1 = sx_keyref_load_material(keysz1, keymaterial1);
 struct sxkeyref key2 = sx_keyref_load_material(keysz2, keymaterial2);
~~~

Next, call the create function that matches the operation to perform, in our
example, for AES XTS encryption:

~~~
 r = sx_blkcipher_create_aesxts_enc(&c, &key1, &key2, iv);
~~~

Each mode in block cipher has a create function for encryption and one for
decryption. The next function calls are the same for all blkcipher operations.

The functions return a status code. SX_OK means that the function call
completed without any issue. Other return values indicate an issue the
caller must handle. That can range from corrupt signature detection to
fatal programming errors. The status codes are defined and documented
in "include/sxsymcrypt/statuscodes.h".

The success status code SX_OK is by default defined as 0. Users can specify at build time a different (non-zero) value for SX_OK. A custom SX_OK value must be chosen such that it does not collide with the other predefined error codes in sxsymcrypt (and in the higher level sicrypto library, if used).

To add data to be encrypted or decrypted, the user calls sx_blkcipher_crypt():

~~~
 r = sx_blkcipher_crypt(&c, datain, sz, dataout);
~~~

To start an encryption, the user calls sx_blkcipher_run():

~~~
 r = sx_blkcipher_run(&c);
~~~

After that call, the hardware is in charge. The user can check if the
operation is still ongoing (SX_ERR_PROCESSING) or has finished successfully
or with an error by calling sx_blkcipher_status().

To wait for the completion by the hardware and get the result of the
operation, call sx_blkcipher_wait() :

~~~
 r = sx_blkcipher_wait(&c);
~~~

The same principles are applied to hashing below:

1. Create an instance for the hashing operation:

~~~
struct sxhash c = {};
~~~

2. Call the create for the particular hash operation (mode):

~~~
r = sx_hash_create(&c, sxhashalg_sha2_256, sizeof(c));
~~~

3. Provide the data:

~~~
sx_hash_feed(&c, data, data_size);
~~~

Several calls to sx_hash_feed() can be issued allowing hashing of
data scattered in memory.

4. Trigger the hardware to compute the digest:

~~~
r = sx_hash_digest(&c, digest);
~~~

5. Check if the operation has finished:

~~~
r = sx_hash_status(&c, digest);
~~~

or, simply, wait for the result:

~~~
r = sx_hash_wait(&c);
~~~


## Concurrent Usage

sxsymcrypt is a pure asynchronous lightweight library. It is not
concurrent. The functions of sxsymcrypt must be called sequentially.
For example the sequence to start an AES GCM encryption may not be
interrupted by other calls :

1. sx_aead_create_aesgcm_enc()
2. sx_aead_feed_aad()
3. sx_aead_crypt()
4. sx_aead_produce_tag()

Calls to other functions like sx_aead_status() may not occur
while running any other sxsymcrypt function. Thus they should not be
called in interrupt handlers or parallel threads. Not respecting those
conditions can result in corruption of the internal state of sxsymcrypt
and cause undefined behaviour.

When working in a concurrent environment, the user shall use
synchronization primitives to protect access to sxsymcrypt.


## DMA Memory

By default, DMA memory can be accessed without the need to know
the DMA memory range.

For platforms with constraints on which memory is accessible over DMA,
sxsymcrypt includes "sx_alloc_global_dmamem()".

sx_alloc_global_dmamem() can be called only once and returns a pointer
to internal memory suitable for DMA. If the requested size is too big
it will return NULL. It will also return NULL when the platform doesn't
support sx_alloc_global_dmamem().

On platforms without constraints on memory accessible by DMA, applications
can skip sx_alloc_global_dmamem().


## Counter-measures

Counter-measures have been implemented according to the paper:
"Secure and Efficient Masking of AES - A Mission Impossible?", June 2004.

The counter-measures are available only for AES, BA411 engine, if they are
enabled in hardware.

With sx_cm_load_mask, the user can add a random value of 4 bytes to the
hardware engine randomness pool for masking. When called multiple
times, the random value is added to the entropy inside the hardware.
For good security, it's recommended to call it twice to provide at
least 64 bits of masking randomness. More is better and 128 bits should
work well in all cases.

To keep the random mask unpredictable, it's recommended to provide new
random values regularly, for example based on how much data has been
processed.

For effective masking counter-measures, the random values should come
from a cryptographically secure random source, for example from a true
random number generator (TRNG) or a DRBG seeded by a TRNG.

After calling sx_cm_load_mask() the user needs to call sx_cm_wait() in order to
wait for the completion by the hardware. The operation does not have an output.

It is under the user responsibility to call sx_cm_load_mask() after system boot
and before any other operation is started. This function is not automatically called.

The sample "cmmask_sample" shows how to load the mask values for countermeasures.


## Key reference

Block cipher, AEAD operations and HMAC operations need keys as inputs for the
create functions.
There are two types of keys: provided by the user or selected from a predefined
set.
In the case of user provided key, the sxkeyref structure holds the reference to
key meterial and its size. The function that returns a key reference in this
case is:

~~~
  sx_keyref_load_material(keysz, keymaterial);
~~~

In the case of the predefined keys, the user can select one of the keys by using
the next function, where keyindex represents the index of the predefined key:

~~~
  sx_keyref_load_by_id(keyindex);
~~~

WARNING: when using "sx_keyref_load_material" the key material buffer must be
kept unchanged until the end of the operation.

## Hardware interrupts

In the context of "sxsymcrypt" library, an interrupt will be generated when the
DMA finished with success or with an error.
Hardware interrupts can be used with AEAD, block ciphers, and HASH.

The user can enable the interrupts by calling "sx_interrupts_enable()". This
function should be called before starting any operation.

~~~
  sx_interrupts_enable();
~~~

The interrupts can be disabled by calling "sx_interrupts_disable()".
This function should be called only after all DMA transfer operations
are finished.

~~~
  sx_interrupts_disable();
~~~

When using interrupts, the implementation of the wait functions can be
optimized to use fewer CPU cycles. For example, the CPU can go in sleep mode
while waiting for an interrupt. On baremetal, integrators can make a customized
version of the baremetal platform code to add support for interrupts.
They can also customize "sx_cmdma_wait()" to wait for an interrupt and put
the CPU in a sleep mode.

When interrupts are enabled, the same rules and mechanisms are applied as
described in the "Overview" paragraph. For more details see "Overview" and
sample "largesha256_irq".

WARNING: "cmmask" does not support interrupts because the operation does not
have a result that is sent.

## Context saving
Context saving is used when the message is too large to store and it needs to
be split into multiple chunks. Context saving can be used with AEAD, block
ciphers, hash and MAC.

### AEAD context saving
AEAD supports context saving for plaintext/ciphertext with AES GCM and CCM and
ChaCha20Poly1305. AAD can be fed also using context saving, supported modes are
AES GCM/GMAC and ChaCha20Poly1305.
Besides the create functions and the generic functions, the functions used for
doing a context saving are:

~~~
  sx_aead_resume_state(c);
  sx_aead_save_state(c);
~~~

For more details about how to use context saving for plaintext/ciphertexthow,
check "aead_ctx" sample.
For more details about how to feed AAD using context saving, check "aead_gmac" sample.

Below examples for AAD fed using context saving:
Remark: encryption and decryption work in the same manner, only the create
functions are different and in the last step sx_aead_produce_tag() should be
used when encrypting and sx_aead_verify_tag() when decrypting.

~~~
  Encryption
    First round AAD:
      sx_aead_create_aesgcm_enc(ctx)
      sx_aead_feed_aad(ctx, 'first AAD chunk')
      sx_aead_save_state(ctx)
      sx_aead_wait(ctx)
    Intermediary rounds AAD:
      sx_aead_resume_state(ctx)
      sx_aead_feed_aad(ctx, 'n-th AAD chunk')
      sx_aead_save_state(ctx)
      sx_aead_wait(ctx)
    Last round AAD / First round plaintext:
      sx_aead_resume_state(ctx)
      sx_aead_feed_aad(ctx, 'last AAD chunk')
      sx_aead_crypt(ctx, 'first chunk')
      sx_aead_save_state(ctx)
      sx_aead_wait(ctx)
    Intermediary rounds plaintext:
      sx_aead_resume_state(ctx)
      sx_aead_crypt(ctx, 'n-th chunk')
      sx_aead_save_state(ctx)
      sx_aead_wait(ctx)
    Last round plaintext:
      sx_aead_resume_state(ctx)
      sx_aead_crypt(ctx, 'last chunk')
      sx_aead_produce_tag(ctx, tag)
      sx_aead_wait(ctx)
~~~

Below examples for AAD fed using context saving, particular case AES GMAC where
no plaintext is used:

~~~
  Encryption
    First round AAD:
      sx_aead_create_aesgcm_enc(ctx)
      sx_aead_feed_aad(ctx, 'first AAD chunk')
      sx_aead_save_state(ctx)
      sx_aead_wait(ctx)
    Intermediary rounds AAD:
      sx_aead_resume_state(ctx)
      sx_aead_feed_aad(ctx, 'n-th AAD chunk')
      sx_aead_save_state(ctx)
      sx_aead_wait(ctx)
    Last round AAD:
      sx_aead_resume_state(ctx)
      sx_aead_feed_aad(ctx, 'last AAD chunk')
      sx_aead_produce_tag(ctx, tag)
      sx_aead_wait(ctx)
~~~

### MAC context saving
Context saving is supported by MAC(message authentication code).
Supported MAC modes: AES CMAC.
Remark: HMAC does not support context saving.
Besides the create functions and the generic functions, the functions used for
doing a context saving are:

~~~
  sx_mac_resume_state(c);
  sx_mac_save_state(c);
~~~

For more details check "cmac_ctx" sample.
Below examples for generating a MAC using AES CMAC:

~~~
    First round:
      sx_mac_create_aescmac(ctx)
      sx_mac_crypt(ctx, 'first chunk')
      sx_mac_save_state(ctx)
      sx_mac_wait(ctx)
    Intermediary rounds:
      sx_mac_resume_state(ctx)
      sx_mac_crypt(ctx, 'n-th chunk')
      sx_mac_save_state(ctx)
      sx_mac_wait(ctx)
    Last round:
      sx_mac_resume_state(ctx)
      sx_mac_crypt(ctx, 'last chunk')
      sx_mac_generate(ctx, mac)
      sx_mac_wait(ctx)
~~~

### Block cipher context saving
Block cipher supports context saving with AES CBC, CFB, CTR, OFB and XTS.
Besides the create functions and the generic functions, the functions used for
doing a context saving are:


~~~
  sx_blkcipher_resume_state(c);
  sx_blkcipher_save_state(c);
~~~

For more details check "blkcipher_ctx" sample.
Below examples for context saving encryption and decryption using:
Remark: all the other supported modes work in the same manner

~~~
  Encryption
    First round:
      sx_blkcipher_create_aesctr_enc(ctx)
      sx_blkcipher_crypt(ctx, 'first chunk')
      sx_blkcipher_save_state(ctx)
      sx_blkcipher_wait(ctx)
    Intermediary rounds:
      sx_blkcipher_resume_state(ctx)
      sx_blkcipher_crypt(ctx, 'n-th chunk')
      sx_blkcipher_save_state(ctx)
      sx_blkcipher_wait(ctx)
    Last round:
      sx_blkcipher_resume_state(ctx)
      sx_blkcipher_crypt(ctx, 'last chunk')
      sx_blkcipher_run(ctx, tag)
      sx_blkcipher_wait(ctx)
  Decryption
    First round:
      sx_blkcipher_create_aesctr_dec(ctx)
      sx_blkcipher_crypt(ctx, 'first chunk')
      sx_blkcipher_save_state(ctx)
      sx_blkcipher_wait(ctx)
    Intermediary rounds:
      sx_blkcipher_resume_state(ctx)
      sx_blkcipher_crypt(ctx, 'n-th chunk')
      sx_blkcipher_save_state(ctx)
      sx_blkcipher_wait(ctx)
    Last round:
      sx_blkcipher_resume_state(ctx)
      sx_blkcipher_crypt(ctx, 'last chunk')
      sx_blkcipher_run(ctx, tag)
      sx_blkcipher_wait(ctx)
~~~

### Hash context saving
Hash supports context saving with SHA1, SHA224, SHA256, SHA384, SHA512 and SM3.
Besides the create functions and the generic functions, the functions used for
doing a context saving are:

~~~
  sx_hash_resume_state(c);
  sx_hash_save_state(c);
~~~

For more details check "streamhash" sample.
Below examples for computing a digest using context saving:

~~~
  First round:
    r = sx_hash_create(ctx, sxhashalg_sha2_256, ctxsz);
    sx_hash_feed(ctx, 'first chunk')
    sx_hash_save_state(ctx)
    sx_hash_wait(ctx)
  Intermediary rounds:
    sx_hash_resume_state(ctx)
    sx_hash_feed(ctx, 'n-th chunk')
    sx_hash_save_state(ctx)
    sx_hash_wait(ctx)
  Last round:
    sx_hash_resume_state(ctx)
    sx_hash_feed(ctx, 'last chunk')
    sx_hash_digest(ctx, tag)
    sx_hash_wait(ctx)
~~~

## AES generic functions

The AES generic functions were designed to accept configurations as parameters.
These functions ensure backward compatibility with previous
versions of the library. There are three generic functions:

~~~
sx_aead_create_aes_generic()
sx_blkcipher_create_aes_generic()
sx_mac_create_aes_generic()
~~~

Configurations are passed using the struct:

~~~
struct sxaesparams {
    uint32_t config;
};
~~~

The sample *aes_tempo_red* demonstrates the usage of all these APIs.
Below is a code snippet from this sample, which uses
sx_aead_create_aes_generic().

~~~
struct sxaead c;
struct sxaesparams params = {.config = 0};
SX_AES_SET_TEMPO_REDUNDANCY(params.config);
r = sx_aead_create_aes_generic(&c, &key, nonce_ccm, sizeof(nonce_ccm), 16,
      sizeof(aad), sizeof(reference_plaintext), 1, AEAD_MODEID_CCM, &params);
~~~


## BA411 AES temporal redundancy

To use AES temporal redundancy one must use the AES generic functions.
To activate this feature, the first bit of the config parameter
has to be set to one.

SX_AES_SET_TEMPO_REDUNDANCY() macros can be used for that.

Note: The feature needs to be active also in the hardware.
If the feature is requested, but the HW does not have it enabled,
a specific error will be triggered.

## Reference Documentation

The public API header files in "include/sxsymcrypt" include the API
reference documentation. That documentation can be extracted with
doxygen :

~~~
  make doc
~~~


# Compatibility

This library works with 32 bit and 64 bit little endian CPUs. The
hardware accelerators have to use the same addresses and endianess as
the CPU.

The software has been built with GCC version 7.3.0. It has been tested on
32-bit ARMv7 (dual-core Cortex-A9) using the linux baremetal emulation.

Some samples and platforms rely on POSIX features. Those are built with
with the "gnu99" language standard of GCC.

Platform independent code should build with the "C99" language
standard. It builds cleanly with GCC 7.3.0 and '-Wall -Wextra' warning
levels. Using other compiler versions or settings is possible but not
directly supported. In those cases, users should fully validate the
library themselves.


# Samples

This project includes a few samples. They're all in the "samples/"
directory.


## Sample "streamcrypt"

This sample reads a stream of input data from stdin and writes the
encrypted or decrypted result on stdout.

This sample works on systems which support standard C input and output
streams.

For the block ciphers and AEAD block ciphers, the input data is streamed via
the standard input in the following order:

 1. Key material
 2. IV
 3. Additional authentication data (for AES GCM only)
 4. Text to encrypt or decrypt
 5. Input authentication tag (for AES GCM decryption only)

The standard output will contain in order :

 1. Encrypted or decrypted text
 2. Authentication tag (for AES GCM encryption only)

For AES GCM decryption, if the authentication tag does not validate,
the decrypted text will not be sent to the standard output.

For hashing, the input data is streamed via the standard input and output
contains the digest.


### AES GCM with streamcrypt

~~~
 $ AESKEY="0123456789abcdef0123456789abcdef"
 $ AAD="authenticated but not encrypted payload"
 $ AADSZ=39
 $ IV="\0\0\0\0\0\0\0\0\0\0\0\0"

 $ printf ${AESKEY}${IV}"${AAD}""a secret message!" | ./streamcrypt 2 $AADSZ > crypted
 $ hd
 00000000  af 53 d2 64 71 71 3a 2c  cf 40 e4 fb 6b f8 37 37  |.S.dqq:,.@..k.77|
 00000010  77 9b c8 63 c0 64 32 3a  77 44 62 1a 86 6d 6c 6d  |w..c.d2:wDb..mlm|
 00000020  e1                                                |.|
 00000021

 $ printf ${AESKEY}${IV}"${AAD}" | cat - crypted | ./streamcrypt 3 $AADSZ
 a secret message!
~~~


## Sample "runpatterns"

The "runpatterns" sample reads patterns from standard input, runs the
associated operation and checks if the results match the expected
outputs. For example :

~~~
    ./runpatterns < validation/patterns/aes_ccm.ptns
    processed: 1920
    failures: 0
~~~

A pattern is a lightweight and very easy to parse binary representation
of an operation to run. It contains the sizes, the inputs, the
expected status code and the expected outputs.

The package "slxicvt" specifies the format of patterns. That package can
also import reference test vectors and write them as patterns files. See
"Validation with Reference Test Vectors" for explanations on that.

### Running "runpatterns" with embedded patterns

There are some cases where the patterns must be embedded in the executable binary.
One case is running on baremetal platform where data cannot be input.
To embed the patterns in the executable, the following steps are needed:
    - add "rawdata.c" file in the samples folder
        - "slxicvt" project can be used to generate this file, example of
        generating "rawpatterns.c" containing the AES CTR patterns:
        ./validation/slxicvt/convertpatterns.py validation/patterns/aes_ctr.ptns --render validation/slxicvt/templates/carray.tmpl > samples/rawdata.c
    - build platform using "IOSRCS" set to samples/env/io_embedded.c
        export IOSRCS=samples/env/io_embedded.c
        make PLATFORM=baremetal CFLAGS="-Werror -Wextra -Os" CPPFLAGS="-DSX_DMAMEM_RESERVE_MEM_SZ=0 -DSX_BAREMETAL_AVAILABILITY_CHECK=0"

## Sample "many"

This sample runs encryption-decryption cycles with internal patterns.

It first starts multiple encryptions asynchronously. When the
encryptions complete, a corresponding decryption is started
asynchronously. When the decryptions finish, the decrypted text is
compared with the original plaintext validating the
encryption-decryption cycle.

This sample has minimal dependencies and software requirements. It's
quite easy to run in a baremetal environment. For baremetal, you'll
probably want to set the preprocessor variable CFG_REPORT_LVL to 0
in order to disable reporting on standard error output.

When the preprocessor variable CFG_WRITE_RAW_ON_STDOUT is defined (by
default it's undefined), the encrypted (with eventually the
authentication tag) or decrypted text will be written on the standard
output.

The preprocessor variable CFG_MAX_JOBS defines how many asynchronous
operations to start in parallel. The sample defaults to 2. The system
must have at least as many hardware accelerators.


## Sample "cmmask_sample"

This sample will load the counter-measures mask into the AES engine.
This sample takes a value from the command line and sends it to the AES engine.

## Sample "sha256hmac"

This sample computes the HMAC with SHA-256 for a text given in the command line.

## Sample "sha256hmac_hwkey"

This sample is identical to the above but uses the IP block attached key
when given the -hw option.

## Sample "largesha256_irq"

The sample computes, using interrupts, 1024 times the hash of a 60KB message
filled with 0xAB. The total size hashed is 60MB.
When the sample is executed on the Linux Baremetal Emulation Platform, the CPU
load decreases compared to a sample without interrupts.
The sample can be launched with the "time" command to measure the CPU load.
In this case, the "user" time is very small compared to "real"(total elapsed
clock time), showing that the interrupts offloaded the CPU.

~~~
    real    0m 2.92s
    user    0m 0.01s
    sys     0m 0.01s
~~~

## Sample "blkcipher_ctx"

This sample shows how to use block cipher context saving.
In this sample, an AES CTR encryption and decryption is executed and the result
is checked against the reference.
All block cipher modes work in the same manner as AES CTR.

## Sample "aead_ctx"

This sample shows how to use AEAD context saving.
In this sample, an AES CCM and GCM and ChaCha20Poly1305 encryption and
decryption is executed and the result is checked against the reference.

## Sample "cmac_ctx"

This sample shows how to use AES CMAC with context saving.
This sample computes the AES CMAC message authentication code for one input
text string. The generated code is compared against the expected CMAC code.

## Sample "aead_gmac"

This sample shows how to use AES GMAC with AAD fed using context saving.
In this sample, an AES GMAC tag computation is executed and the result is
checked against the reference.

## Sample "transfer"

This sample shows how to use the transfer feature which is based on the channel
operation. In this sample, a transfer of data is executed, source to destination
and the data transfered is checked against its source.

## Sample "streammac"

This sample reads a stream of input data from stdin and writes the MAC result on
stdout.

This sample works on systems which support standard C input and output
streams.

The input data is streamed via the standard input in the following order:

 1. Key Material
 2. Message

The output will contain the MAC.

## Sample "truerandom"

Uses the TRNG API to generate a number of random bytes as requested by the argument.

## Sample "aes_tempo_red"

This sample illustrates how to utilize AES generic APIs to enable AES temporal redundancy.

## Sample "aead_ccm_star"

This sample shows how to use AEAD CCM*.
In this sample, an AES CCM* operation is triggered. The sample verifies that expected
ciphertext is generated and that no tag is outputted.

# Validation tools

## "rawrandom" - validation tool for TRNG characterization and certification

Configures and uses the TRNG API to generate a set of samples as defined by
[NIST SP800-90B](https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-90B.pdf)
Chapter 3 for validation.

The tool uses three arguments:
  - mode
  - length, in bytes, of the samples requested.
  - sampling period, number of clock cycles to wait between sampling moments (optional).

The tool uses two modes:
  - 'c' - continuous: samples gathered up to length argument
  - 'r' - reset: samples are gathered up to length argument with a TRNG reset after 1000 samples

The sampling period is HW implementation-specific. It depends on a set of specific
system particularities, for example, but not limited to, bus frequency, ring
oscillators frequencies, etc. For more details please check the BA431 NDRNG application note.

During automated testing "rawrandom" is used to generate data that gets verified
by the publicly available [NIST SP800-90B entropy test suite](https://github.com/usnistgov/SP800-90B_EntropyAssessment).
The app was used to generate values for "non-IID" test and for "restart" test(Chapter 3.1.4).

Example:

~~~
  ./rawrandom c 1000000 > continuous_samples.bin
  ./rawrandom r 1000000 > restart_samples.bin
~~~

WARNING: do not use the raw mode in production. This sample is intended to be used
ONLY for pre-production testing, characterization, and/or certification.

# Validation with Reference Test Vectors

The "slxicvt" package, located in "validation/", can import test vectors
from official standards and references to help validate and qualify the
product. The sample runpatterns can run those test vectors directly.

The Makefile uses "slxicvt" to obtain all reference test vectors and
filter for supported AES key sizes. The pattern files to use are put in
"validation/patterns/".

For example :

    cd validation/slxicvt
    make patterns
    cd ../../
    for i in validation/patterns/*.ptns
    do
       echo $i
       ./runpatterns < $i
    done

# License

This library is copyrighted by Secure-IC. Use of it is governed by
the agreements with Secure-IC.

Files in the software emulator may contain third party open source
code. Those files contain comments explaining it, typically at the top
of the file. The texts of the open source licenses are in the LICENSES
subdirectory.

Sub-projects have their own licences.

# Known limitations

   - The BA415 hardware engine for AES GCM does not support:
      * context saving
      * both AAD and plaintext/ciphertext empty (sizes of zero bytes)

   - The BA416 hardware engine for AES XTS does not support:
      * context saving

# Versions and Compatibility

The macros SXSYMCRYPT_API_MAJOR and SXSYMCRYPT_API_MINOR in
"include/sxsymcrypt/versions.h" define the version of the sxsymcrypt API.
Applications can use it to make sure they use a compatible version.

When SXSYMCRYPT_API_MINOR increases, the API was extended with new functionality
without changing previous API.
When SXSYMCRYPT_API_MAJOR increases, the new API is not backwards compatible
with the previous one. For example, functions have other arguments or behave
differently.

The macro SXSYMCRYPT_API_ASSERT_COMPATIBLE() can check the compatibility. If
not compatible, it triggers a compile time error. In that case, the
application should be updated to work with the latest API of sxsymcrypt.

Current API versions of sxsymcrypt and the changes with respect to the previous
version:

1.0
   - First version of API

1.1
   - Added TDES functionality

2.0
   - Added keyref(key reference) for key management, used in block ciphers and
   AEAD opearations
   - Simplified hashing when using context saving
   - Hash create functions take the size of the hash operation context

3.0
   - Updated AES, SM4 and TDES block cipher functions:
        - moved the operation direction from sx_blkcipher_encrypt() and
        sx_blkcipher_decrypt() to the create functions.
        Ex: sx_blkcipher_create_aesctr_enc(), sx_blkcipher_create_aesctr_dec()
        - added sx_blkcipher_crypt() and sx_blkcipher_run() functions replace
        the functionality of sx_blkcipher_encrypt() and sx_blkcipher_decrypt()
        that were removed.
   - Support AES block cipher context saving with sx_blkcipher_save_state()
   and sx_blkcipher_resume_state().
   - Updated AES CCM and GCM and ChaCha20Poly1305 AEAD functions:
        - moved the operation direction from sx_aead_encrypt() and
        sx_aead_decrypt() to the create functions.
        Ex: sx_aead_create_aesgcm_enc(), sx_aead_create_aesgcm_dec()
        - added sx_aead_feed_aad(), sx_aead_crypt(), sx_aead_produce_tag() and
        sx_aead_verify_tag() functions which replace the functionality of
        sx_aead_encrypt() and sx_aead_decrypt().
   - Added AES CMAC with context saving support
   - Updated HMAC to use same API as AES CMAC, based on MAC. All HMAC functions
   were updated from sx_hash_create_hmac* to sx_mac_create_hmac*
   - Support AEAD context saving with sx_aead_save_state() and
   sx_aead_resume_state().
   - Added AEAD AAD fed using context saving for AES GCM/GMAC and ChaCha20Poly1305

3.1
   - Added SM4 XTS, GCM, CCM and CMAC modes
   - Added SM4 context saving for all modes

3.2
   - Introduced sxhashalg hash algorithm definitions
   - Added sx_hash_get_alg_digestsz()
   - Added sx_hash_get_alg_blocksz()
   - Added sx_hash_create() to create a hash context from a hash algorithm
     defininition. That's now the preferred way to use sxhash.

4.0
  - Added support for hardware keyed HMAC
     - The hmac class factories (sx_mac_create_hmac_sha2_224 ea) were
       relabeled and now take an sxkeyref structure to pass the key, or
       selected hardware key.

4.1
  - Preliminary support of 3GPP algorithms: Snow3G, ZUC, Kasumi

4.2
  - Added support for ARIA ECB, CBC, OFB, CFB, CTR, GCM, CCM and CMAC including
  context saving for all modes.
  - Added support for ChaCha20 including context saving.
  - Added support for transfer(copy of data from source to destination)

4.3
  - Added support for HW keys for BA415

4.4
  - Updated TRNG to support new HW changes
  - Extend API to support TRNG save/restore state
  - Extend API to support TRNG restart

4.5
  - Extended API with support for TRNG initialization with self-tests

4.6
  - Added TRNG proportion and repetition self-tests
  - Added support for HW keys for BA417
  - Extend maximum number of HW keys for BA411, BA413, BA415, BA417 and BA418

4.7
  - Enabled HW keys in BA419(SM4)

4.8
  - Added engine for OpenSSL V1.1w, supporting SHA3 and AES GCM/XTS

4.9
  - Added support for hardware AES temporal redundancy
  - Added generic AES functions with configuration parameter

4.10
  - Added support for CCM*
