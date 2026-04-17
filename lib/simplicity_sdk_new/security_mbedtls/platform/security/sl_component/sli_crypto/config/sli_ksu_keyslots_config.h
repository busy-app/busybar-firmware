#ifndef SLI_KSU_KEYSLOTS_CONFIG_H
#define SLI_KSU_KEYSLOTS_CONFIG_H

// -----------------------------------------------------------------------------
// User exposed config options

// <<< Use Configuration Wizard in Context Menu >>>

// <h> KSU KEY slot reservation

// <o SL_KSU_KEY_SLOTS_RPA> Number of KSU key slots reserved for RPA.
// <i> Default: 16
// <i> The value of this configuration should be updated for the application needs.
#define SL_KSU_KEY_SLOTS_RPA         2

// </h>

// <<< end of configuration section >>>

#define SLI_KSU_MAX_KEY_SLOTS              KSU_MAX_KEY_SLOTS
#define SLI_KSU_KEY_SLOT_NVM3                 0
#if defined(SL_CATALOG_BLUETOOTH_FEATURE_RESOLVING_LIST_PRESENT)
#define SLI_KSU_KEY_SLOT_RPA_START            1
#define SLI_KSU_KEY_SLOT_RPA_END           (SLI_KSU_KEY_SLOT_RPA_START + SL_KSU_KEY_SLOTS_RPA - 1)
#define SLI_KSU_KEY_SLOT_USER_START        (SLI_KSU_KEY_SLOT_RPA_END + 1)
#else
#define SLI_KSU_KEY_SLOT_USER_START           1
#endif

#endif // SLI_KSU_KEYSLOTS_CONFIG_H
