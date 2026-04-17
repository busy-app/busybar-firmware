/*
 *  Copyright (c) 2025, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file implements the radio security for the EFR32 platform.
 */
#include "radio_security.h"
#include "security_manager.h"

#include <openthread-core-config.h>
#include <openthread/platform/radio.h>
#include <openthread/platform/time.h>

#include "common/code_utils.hpp"
#include "common/debug.hpp"
#include "common/logging.hpp"
#include "utils/code_utils.h"
#include "utils/mac_frame.h"

#include "platform-efr32.h"
#include "radio_instance.h"

extern "C" {
#include "sl_core.h"
#include "sl_packet_utils.h"
}

#if (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)

// Security key management
enum class MacKeyType
{
    PREV,
    CURRENT,
    NEXT,
    COUNT
};

struct securityMaterial
{
    uint8_t           ackKeyId;
    uint8_t           keyId;
    volatile uint32_t macFrameCounter;
    volatile uint32_t ackFrameCounter;
    otMacKeyMaterial  keys[static_cast<int>(MacKeyType::COUNT)];
    // KSU slot index for each stored key. 0xFF means "not stored in KSU".
    uint8_t ksuSlot[static_cast<int>(MacKeyType::COUNT)];
};

// Per-instance security material
static securityMaterial sMacKeys[RADIO_INTERFACE_COUNT];

// External declarations
extern otExtAddress sExtAddress[RADIO_EXT_ADDR_COUNT];

extern "C" {

void sli_ot_radio_security_init(void)
{
    // Initialize security material for all instances
    memset(sMacKeys, 0, sizeof(sMacKeys));
    // Mark KSU slots as "not present"
    for (size_t i = 0; i < RADIO_INTERFACE_COUNT; ++i)
    {
        for (int k = 0; k < static_cast<int>(MacKeyType::COUNT); ++k)
        {
            sMacKeys[i].ksuSlot[k] = 0xFF;
        }
    }
}

void sli_ot_radio_security_deinit(void)
{
#if (OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_PSA)
#ifdef LPWAES
#if defined(KSU_PRESENT)
    // Unregister all KSU keys before clearing memory
    for (size_t i = 0; i < RADIO_INTERFACE_COUNT; ++i)
    {
        for (int k = 0; k < static_cast<int>(MacKeyType::COUNT); ++k)
        {
            psa_key_id_t key_ref = sMacKeys[i].keys[k].mKeyMaterial.mKeyRef;
            if (key_ref != 0)
            {
                sl_sec_man_unregister_ksu_key(key_ref);
            }
        }
    }
#endif // KSU_PRESENT
#endif // LPWAES
#endif // PSA crypto lib

    // Clear security material for all instances
    memset(sMacKeys, 0, sizeof(sMacKeys));
}

otError sli_ot_radio_security_process_transmit(otRadioFrame *aFrame, otInstance *aInstance)
{
    otError         error = OT_ERROR_NONE;
    uint8_t         keyId;
    uint8_t         keyToUse;
    instanceIndex_t instanceIndex = sli_ot_radio_instance_get_index(aInstance);

    otEXPECT(otMacFrameIsSecurityEnabled(aFrame) && otMacFrameIsKeyIdMode1(aFrame)
             && !aFrame->mInfo.mTxInfo.mIsSecurityProcessed);

    if (otMacFrameIsAck(aFrame))
    {
        keyId = otMacFrameGetKeyId(aFrame);

        otEXPECT_ACTION(keyId != 0, error = OT_ERROR_FAILED);

        if (keyId == sMacKeys[instanceIndex].keyId - 1)
        {
            keyToUse = static_cast<uint8_t>(MacKeyType::PREV);
        }
        else if (keyId == sMacKeys[instanceIndex].keyId)
        {
            keyToUse = static_cast<uint8_t>(MacKeyType::CURRENT);
        }
        else if (keyId == sMacKeys[instanceIndex].keyId + 1)
        {
            keyToUse = static_cast<uint8_t>(MacKeyType::NEXT);
        }
        else
        {
            error = OT_ERROR_SECURITY;
            otEXPECT(false);
        }
    }
    else
    {
        keyId    = sMacKeys[instanceIndex].keyId;
        keyToUse = static_cast<uint8_t>(MacKeyType::CURRENT);
    }

    aFrame->mInfo.mTxInfo.mAesKey = &sMacKeys[instanceIndex].keys[keyToUse];

    if (!aFrame->mInfo.mTxInfo.mIsHeaderUpdated)
    {
        if (otMacFrameIsAck(aFrame))
        {
            // Store ack frame counter and ack key ID for receive frame
            sMacKeys[instanceIndex].ackKeyId        = keyId;
            sMacKeys[instanceIndex].ackFrameCounter = sMacKeys[instanceIndex].macFrameCounter;
        }

        otMacFrameSetKeyId(aFrame, keyId);
        otMacFrameSetFrameCounter(aFrame, sMacKeys[instanceIndex].macFrameCounter++);
    }

    efr32PlatProcessTransmitAesCcm(aFrame, &sExtAddress[instanceIndex]);

exit:
    return error;
}
#ifdef LPWAES
#if defined(KSU_PRESENT)
static void sli_ot_radio_security_copy_key_to_ksu(instanceIndex_t index)
{
    // Declare all variables at the top of the function to prevent jumping over initialization
    int          prevIdx = static_cast<int>(MacKeyType::PREV);
    int          currIdx = static_cast<int>(MacKeyType::CURRENT);
    int          nextIdx = static_cast<int>(MacKeyType::NEXT);
    psa_key_id_t idPrev  = 0;
    psa_key_id_t idCurr  = 0;
    psa_key_id_t idNext  = 0;
    // Unregister old KSU keys before they are replaced
    for (int k = 0; k < static_cast<int>(MacKeyType::COUNT); ++k)
    {
        psa_key_id_t old_key_ref = sMacKeys[index].keys[k].mKeyMaterial.mKeyRef;
        if (old_key_ref != 0)
        {
            sl_sec_man_unregister_ksu_key(old_key_ref);
        }
    }
    // Copy previous, current and next keys to KSU using the generalized security manager API
    for (int k = 0; k < static_cast<int>(MacKeyType::COUNT); ++k)
    {
        psa_key_id_t source_key_id = sMacKeys[index].keys[k].mKeyMaterial.mKeyRef;
        psa_key_id_t ksu_key_id    = 0;
        uint8_t      ksu_slot      = 0xFF;

        psa_status_t status = sl_sec_man_copy_key_to_ksu(source_key_id, &ksu_key_id, &ksu_slot);

        if (status == PSA_SUCCESS && ksu_key_id != 0)
        {
            // Update the key reference to the new KSU key
            sMacKeys[index].keys[k].mKeyMaterial.mKeyRef = ksu_key_id;
            sMacKeys[index].ksuSlot[k]                   = ksu_slot;
        }
        else
        {
            // Failed to copy to KSU - this is unexpected, assert in debug builds
            OT_ASSERT(status == PSA_SUCCESS);
        }
    }

    // Get PSA key ids and verify they are unique.
    idPrev = sMacKeys[index].keys[prevIdx].mKeyMaterial.mKeyRef;
    idCurr = sMacKeys[index].keys[currIdx].mKeyMaterial.mKeyRef;
    idNext = sMacKeys[index].keys[nextIdx].mKeyMaterial.mKeyRef;
    // If any two PSA key ids are equal, that's unexpected — fail loudly in
    // debug builds so the issue can be investigated.
    if (idPrev == idCurr || idPrev == idNext || idCurr == idNext)
    {
        // Duplicate PSA key id detected when copying keys to KSU. This is
        // unexpected — assert so the issue can be investigated.
        OT_ASSERT(false);
    }
}
#endif // KSU_PRESENT
#endif // LPWAES
void sli_ot_radio_security_set_mac_key(otInstance             *aInstance,
                                       uint8_t                 aKeyIdMode,
                                       uint8_t                 aKeyId,
                                       const otMacKeyMaterial *aPrevKey,
                                       const otMacKeyMaterial *aCurrKey,
                                       const otMacKeyMaterial *aNextKey,
                                       otRadioKeyType          aKeyType)
{
    OT_UNUSED_VARIABLE(aKeyIdMode);
    OT_UNUSED_VARIABLE(aKeyType);

    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);

    otEXPECT(sl_ot_rtos_task_can_access_pal());
    OT_ASSERT(aPrevKey != nullptr && aCurrKey != nullptr && aNextKey != nullptr);

    // MAC frame counters are reset before updating keys. This order
    // safeguards against issues that can arise when the radio
    // platform handles TX security and counter assignment.  The
    // radio platform might prepare an enhanced ACK to a received
    // frame from an parallel (e.g., ISR) context, which consumes
    // a MAC frame counter value.
    //
    // If the MAC key is updated before the frame counter is cleared,
    // the radio could receive and send an enhanced ACK between these
    // two actions, possibly using the new MAC key with a larger
    // (current) frame counter value. This could then prevent the
    // receiver from accepting subsequent transmissions after the
    // frame counter reset for a long time.
    //
    // While resetting counters first might briefly cause an enhanced
    // ACK to be sent with the old key and a zero counter (which might
    // be rejected by the receiver), this is a transient issue that
    // quickly resolves itself.
    sli_ot_radio_security_set_mac_frame_counter(aInstance, 0);

    sMacKeys[index].keyId = aKeyId;
    memcpy(&sMacKeys[index].keys[static_cast<int>(MacKeyType::PREV)], aPrevKey, sizeof(otMacKeyMaterial));
    memcpy(&sMacKeys[index].keys[static_cast<int>(MacKeyType::CURRENT)], aCurrKey, sizeof(otMacKeyMaterial));
    memcpy(&sMacKeys[index].keys[static_cast<int>(MacKeyType::NEXT)], aNextKey, sizeof(otMacKeyMaterial));
    // Reset recorded KSU slot markers for the new keys.
    for (int k = 0; k < static_cast<int>(MacKeyType::COUNT); ++k)
    {
        sMacKeys[index].ksuSlot[k] = 0xFF;
    }

#if (OPENTHREAD_CONFIG_CRYPTO_LIB == OPENTHREAD_CONFIG_CRYPTO_LIB_PSA)
    // Under LPWAES: copy current key into KSU (if available) using psa_copy_key, export only prev/next.
    // Otherwise export all three keys as before.
#if defined(LPWAES) && defined(KSU_PRESENT)
    sli_ot_radio_security_copy_key_to_ksu(index);
#else  // !LPWAES || !KSU_PRESENT
    size_t  aKeyLen;
    otError error;

    error = otPlatCryptoExportKey(sMacKeys[index].keys[static_cast<int>(MacKeyType::PREV)].mKeyMaterial.mKeyRef,
                                  sMacKeys[index].keys[static_cast<int>(MacKeyType::PREV)].mKeyMaterial.mKey.m8,
                                  sizeof(sMacKeys[index].keys[static_cast<int>(MacKeyType::PREV)]),
                                  &aKeyLen);
    OT_ASSERT(error == OT_ERROR_NONE);

    error = otPlatCryptoExportKey(sMacKeys[index].keys[static_cast<int>(MacKeyType::CURRENT)].mKeyMaterial.mKeyRef,
                                  sMacKeys[index].keys[static_cast<int>(MacKeyType::CURRENT)].mKeyMaterial.mKey.m8,
                                  sizeof(sMacKeys[index].keys[static_cast<int>(MacKeyType::CURRENT)]),
                                  &aKeyLen);
    OT_ASSERT(error == OT_ERROR_NONE);

    error = otPlatCryptoExportKey(sMacKeys[index].keys[static_cast<int>(MacKeyType::NEXT)].mKeyMaterial.mKeyRef,
                                  sMacKeys[index].keys[static_cast<int>(MacKeyType::NEXT)].mKeyMaterial.mKey.m8,
                                  sizeof(sMacKeys[index].keys[static_cast<int>(MacKeyType::NEXT)]),
                                  &aKeyLen);
    OT_ASSERT(error == OT_ERROR_NONE);
#endif // LPWAES
#endif // PSA crypto lib

exit:
    return;
}

void sli_ot_radio_security_set_mac_frame_counter(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);

    otEXPECT(sl_ot_rtos_task_can_access_pal());

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    sMacKeys[index].macFrameCounter = aMacFrameCounter;

    CORE_EXIT_ATOMIC();

exit:
    return;
}

void sli_ot_radio_security_set_mac_frame_counter_if_larger(otInstance *aInstance, uint32_t aMacFrameCounter)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    otEXPECT(sl_ot_rtos_task_can_access_pal());

    CORE_DECLARE_IRQ_STATE;
    CORE_ENTER_ATOMIC();

    if (aMacFrameCounter > sMacKeys[index].macFrameCounter)
    {
        sMacKeys[index].macFrameCounter = aMacFrameCounter;
    }

    CORE_EXIT_ATOMIC();

exit:
    return;
}

uint8_t sli_ot_radio_security_get_ack_key_id(otInstance *aInstance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    return sMacKeys[index].ackKeyId;
}

uint32_t sli_ot_radio_security_get_ack_frame_counter(otInstance *aInstance)
{
    instanceIndex_t index = sli_ot_radio_instance_get_index(aInstance);
    return sMacKeys[index].ackFrameCounter;
}

} // extern

#endif // (OPENTHREAD_CONFIG_THREAD_VERSION >= OT_THREAD_VERSION_1_2)
