/*******************************************************************************
 * @file
 * @brief Provides definitions for indirect code classification of functions
 * in link_raw API module.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifndef OPENTHREAD_LINK_RAW_H_
#define OPENTHREAD_LINK_RAW_H_
#include "sl_code_classification.h"
#include <openthread/platform/radio.h>

typedef void (*otLinkRawEnergyScanDone)(otInstance *aInstance, int8_t aEnergyScanMaxRssi);
typedef void (*otLinkRawReceiveDone)(otInstance *aInstance, otRadioFrame *aFrame, otError aError);
typedef void (*otLinkRawTransmitDone)(otInstance   *aInstance,
                                      otRadioFrame *aFrame,
                                      otRadioFrame *aAckFrame,
                                      otError       aError);

// Classify all the otLinkRaw<APIs>
SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
bool    otLinkRawGetPromiscuous(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
bool    otLinkRawIsEnabled(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
int8_t  otLinkRawGetRssi(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawEnergyScan(otInstance             *aInstance,
                                                     uint8_t                 aScanChannel,
                                                     uint16_t                aScanDuration,
                                                     otLinkRawEnergyScanDone aCallback);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawReceive(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetAlternateShortAddress(otInstance *aInstance, otShortAddress aShortAddress);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetMacFrameCounter(otInstance *aInstance, uint32_t aMacFrameCounter);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetMacFrameCounterIfLarger(otInstance *aInstance, uint32_t aMacFrameCounter);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetMacKey(otInstance     *aInstance,
                                                    uint8_t         aKeyIdMode,
                                                    uint8_t         aKeyId,
                                                    const otMacKey *aPrevKey,
                                                    const otMacKey *aCurrKey,
                                                    const otMacKey *aNextKey);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetPromiscuous(otInstance *aInstance, bool aEnable);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetReceiveDone(otInstance *aInstance, otLinkRawReceiveDone aCallback);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSetShortAddress(otInstance *aInstance, uint16_t aShortAddress);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSleep(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchAddExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchAddShortEntry(otInstance *aInstance, uint16_t aShortAddress);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchClearExtEntries(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchClearExtEntry(otInstance *aInstance, const otExtAddress *aExtAddress);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchClearShortEntries(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchClearShortEntry(otInstance *aInstance, uint16_t aShortAddress);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawSrcMatchEnable(otInstance *aInstance, bool aEnable);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otError otLinkRawTransmit(otInstance *aInstance, otLinkRawTransmitDone aCallback);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otRadioCaps   otLinkRawGetCaps(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
otRadioFrame *otLinkRawGetTransmitBuffer(otInstance *aInstance);

SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
uint64_t      otLinkRawGetRadioTime(otInstance *aInstance);

#endif //OPENTHREAD_LINK_RAW_H_
