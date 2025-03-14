#ifndef SL_MATTER_PROVISION_H
#define SL_MATTER_PROVISION_H

// <<< Use Configuration Wizard in Context Menu >>>

// <q ENABLE_EXAMPLE_CREDENTIALS> With this flag enabled if the attestation certificates are not present in non-volatile memory, the device will use the hardcoded test certificates. The flag should be disabled in production builds. 
// <i> Default: 1
#define ENABLE_EXAMPLE_CREDENTIALS 1

// <<< end of configuration section >>> 

#if ENABLE_EXAMPLE_CREDENTIALS
    #define CHIP_DEVICE_CONFIG_ENABLE_EXAMPLE_CREDENTIALS 1
#endif

#endif // SL_MATTER_PROVISION_H
