# Security Features of the SiWx91x™

The SiWx91x™ chipset family comes with several features that can be utilized to develop secure applications. For more information, contact [Silicon Labs Sales](https://www.silabs.com/about-us/contact-sales).

The following are the security features of the SiWx91x:
- **Immutable First-Stage Bootloader** in read-only memory (ROM)
- **Secure Boot**: Authenticating and/or encrypting the boot image to prevent the writing of a malicious image onto the device.
- **Anti-Rollback**: A configurable option to prevent downgrading the firmware version.
- **Secure Debug**: Preventing the use of the debug ports which can be enabled in software using cryptographically secure host interface commands validated by the immutable first-stage bootloader.
- **Secure XIP**: XIP images with Advanced Encryption Standard (AES) encryption with physically unclonable function (PUF) keys held in an isolated key store on the device.
- **Wi-Fi Protected Access 2 and 3** (**WPA2** and **WPA3**) for secure Wi-Fi connections between access points (APs) and stations.
- **Transport Layer Security (TLS) version 1.3** for security at the network's transport layer network.
- **True Random Number Generator** (TRNG) based on radio frequency (RF) noise.
- **Secure Isolation** between the application and network processors.
- **Secure Attestation** of a SiWx91x device using a cryptographically signed entity attestation token (EAT).