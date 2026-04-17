# Storage Bootloader (Single OTA Image of size 1024kB)

This sample configuration of the Gecko bootloader configures the bootloader to use the external flash to store firmware update images. The storage configuration is set up to store a single firmware update image at a time, in a single storage slot. The storage slot is configured to start at address 0x011F8000, and have a size of 1024 kB. This can be configured on the *"Bootloader Storage Slot Setup"*.

When modifying the storage layout of the bootloader, make sure that the code region does not overlap with the data region or the NVM3 region within the data region.

        ----------------------------------------------------------
        | REGION NAME  | REGION START | REGION END  | REGION SIZE |
        -----------------------------------------------------------
        | CODE REGION  | 0x0100 0000  | 0x011F 8000 | 0x001F 0000 |
        | DATA REGION  | 0x011F 8000  | 0x0139 A000 | 0x001E A000 |
        | OTA          | 0x011F 8000  | 0x012F 8000 | 0x0010 0000 |
        | NVM          | 0x0137 E000  | 0x0139 A000 | 0x0001 4000 |
        | LOCK BITS    | 0x0139 2000  | 0x0139 A000 | 0x0000 8000 | 
        -----------------------------------------------------------
**Note**: This sample configuration will not use all available flash space for bootloader storage. As the memory layout may differ between different application projects, this bootloader  project has a safe configuration that ensures that it doesn't overlap with any application sample configuration. This comes at the expense of leaving some unused flash space. Always make sure to configure your bootloader to use the same area for storage as is set aside by your application.
