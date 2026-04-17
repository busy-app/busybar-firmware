# Storage Bootloader (Multiple OTA Image of size 756kB)
This sample configuration of the Gecko bootloader configures the bootloader to use the flash to store firmware update images to store up to two firmware update images at a time, in two storage slots. The storage slots are configured to start at address 0x011FA000 and 0x012B7000 of the flash respectively, and have a size of 756kB. This can be configured on the "Bootloader Storage Slot Setup".

The "bootload info" meta-information, which tells the bootloader which storage slot to install the next firmware update from, is configured under "Bootloader Storage Slot Setup" to be placed at address 0x0. The first storage slot is configured to start at address 0x2000 (8 kB) because the bootload info takes up 2 flash pages of 4 kB

When modifying the storage layout of the bootloader, make sure that the code region does not overlap with the data region or the NVM3 region within the data region.


        ----------------------------------------------------------
        | REGION NAME  | REGION START | REGION END  | REGION SIZE |
        -----------------------------------------------------------
        | CODE REGION  | 0x0100 0000  | 0x011F 8000 | 0x001F 0000 |
        | DATA REGION  | 0x011F 8000  | 0x0139 A000 | 0x001E A000 |
        | OTA1         | 0x011F A000  | 0x012B 7000 | 0x000B D000 |
        | OTA2         | 0x012D 1000  | 0x013A 0000 | 0x000B D000 |
        | NVM          | 0x0137 E000  | 0x0139 A000 | 0x0001 4000 |
        | LOCK BITS    | 0x0139 2000  | 0x0139 A000 | 0x0000 8000 |
        -----------------------------------------------------------                