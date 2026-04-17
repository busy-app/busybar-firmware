/*******************************************************************************
 * @file
 * @brief Provides definitions for indirect code classification of functions
 * in factory_diags.hpp
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

#ifndef FACTORY_DIAGS_HPP_
#define FACTORY_DIAGS_HPP_
 
#include "sl_code_classification.h"
#include "openthread-core-config.h"
 
#if OPENTHREAD_CONFIG_DIAG_ENABLE
 
#include <string.h>
 
#include <openthread/diag.h>
#include <openthread/platform/radio.h>
 
#include "common/clearable.hpp"
#include "common/error.hpp"
#include "common/locator.hpp"
#include "common/non_copyable.hpp"
#include "common/string.hpp"
#include "mac/mac_types.hpp"

namespace ot {
namespace FactoryDiags {
 
class Diags : public InstanceLocator, private NonCopyable
{
public:
    /**
     * Constructor.
     *
     * @param[in]  aInstance  The OpenThread instance.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    explicit Diags(Instance &aInstance);

    /**
     * Processes a factory diagnostics command line.
     *
     * @param[in]   aString        A null-terminated input string.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessLine(const char *aString);

    /**
     * Processes a factory diagnostics command line.
     *
     * @param[in]   aArgsLength    The number of args in @p aArgs.
     * @param[in]   aArgs          The arguments of diagnostics command line.
     *
     * @retval  kErrorInvalidArgs       The command is supported but invalid arguments provided.
     * @retval  kErrorNone              The command is successfully process.
     * @retval  kErrorNotImplemented    The command is not supported.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessCmd(uint8_t aArgsLength, char *aArgs[]);

    /**
     * Indicates whether or not the factory diagnostics mode is enabled.
     *
     * @retval TRUE if factory diagnostics mode is enabled
     * @retval FALSE if factory diagnostics mode is disabled.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool IsEnabled(void);

    /**
     * The platform driver calls this method to notify OpenThread diagnostics module that the alarm has fired.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void AlarmFired(void);

    /**
     * The radio driver calls this method to notify OpenThread diagnostics module of a received frame.
     *
     * @param[in]  aFrame  A pointer to the received frame or `nullptr` if the receive operation failed.
     * @param[in]  aError  kErrorNone when successfully received a frame,
     *                     kErrorAbort when reception was aborted and a frame was not received,
     *                     kErrorNoBufs when a frame could not be received due to lack of rx buffer space.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ReceiveDone(otRadioFrame *aFrame, Error aError);

    /**
     * The radio driver calls this method to notify OpenThread diagnostics module that the transmission has completed.
     *
     * @param[in]  aError  kErrorNone when the frame was transmitted,
     *                     kErrorChannelAccessFailure tx could not take place due to activity on channel,
     *                     kErrorAbort when transmission was aborted for other reasons.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void TransmitDone(Error aError);

    /**
     * Sets the diag output callback.
     *
     * @param[in]  aCallback   A callback method called to output diag messages.
     * @param[in]  aContext    A user context pointer.
     */
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void SetOutputCallback(otDiagOutputCallback aCallback, void *aContext);

private:
    static constexpr uint8_t kMaxArgs = OPENTHREAD_CONFIG_DIAG_CMD_LINE_ARGS_MAX;

    struct Command
    {
        const char *mName;
        Error (Diags::*mCommand)(uint8_t aArgsLength, char *aArgs[]);
    };

    struct Stats : public Clearable<Stats>
    {
        uint32_t mReceivedPackets;
        uint32_t mSentSuccessPackets;
        uint32_t mSentFailedPackets;
        uint32_t mSentErrorCcaPackets;
        uint32_t mSentErrorAbortPackets;
        uint32_t mSentErrorInvalidStatePackets;
        uint32_t mSentErrorOthersPackets;
        int8_t   mFirstRssi;
        uint8_t  mFirstLqi;
        int8_t   mLastRssi;
        uint8_t  mLastLqi;
    };

    struct RawPowerSetting
    {
        static constexpr uint16_t       kMaxDataSize    = OPENTHREAD_CONFIG_POWER_CALIBRATION_RAW_POWER_SETTING_SIZE;
        static constexpr uint16_t       kInfoStringSize = kMaxDataSize * 2 + 1;
        typedef String<kInfoStringSize> InfoString;

        InfoString ToString(void) const
        {
            InfoString string;

            string.AppendHexBytes(mData, mLength);

            return string;
        }

        bool operator!=(const RawPowerSetting &aOther) const
        {
            return (mLength != aOther.mLength) || (memcmp(mData, aOther.mData, mLength) != 0);
        }

        uint8_t  mData[kMaxDataSize];
        uint16_t mLength;
    };

    struct PowerSettings
    {
        bool operator!=(const PowerSettings &aOther) const
        {
            return (mTargetPower != aOther.mTargetPower) || (mActualPower != aOther.mActualPower) ||
                   (mRawPowerSetting != aOther.mRawPowerSetting);
        }

        int16_t         mTargetPower;
        int16_t         mActualPower;
        RawPowerSetting mRawPowerSetting;
    };

    struct ReceiveConfig
    {
        ReceiveConfig(void)
            : mIsEnabled(false)
            , mIsAsyncCommand(false)
            , mShowRssi(true)
            , mShowLqi(true)
            , mShowPsdu(false)
            , mIsFilterEnabled(false)
            , mReceiveCount(0)
            , mNumFrames(0)
            , mFilterAddress()
        {
        }

        bool mIsEnabled : 1;
        bool mIsAsyncCommand : 1;
        bool mShowRssi : 1;
        bool mShowLqi : 1;
        bool mShowPsdu : 1;
        bool mIsFilterEnabled : 1;

        uint16_t     mReceiveCount;
        uint16_t     mNumFrames;
        Mac::Address mFilterAddress;
    };
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ParseCmd(char *aString, uint8_t &aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessChannel(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessFrame(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessContinuousWave(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessGpio(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessPower(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessRadio(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessRepeat(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessPowerSettings(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessRawPowerSetting(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessSend(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessStart(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessStats(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessStop(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessStream(uint8_t aArgsLength, char *aArgs[]);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessSweep(uint8_t aArgsLength, char *aArgs[]);
#if OPENTHREAD_RADIO && !OPENTHREAD_RADIO_CLI
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ProcessEcho(uint8_t aArgsLength, char *aArgs[]);
#endif
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error GetRawPowerSetting(RawPowerSetting &aRawPowerSetting);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error GetPowerSettings(uint8_t aChannel, PowerSettings &aPowerSettings);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error ParseReceiveConfigFormat(const char *aFormat, ReceiveConfig &aConfig);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error RadioReceive(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    Error TransmitPacket(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void  OutputReceivedFrame(const otRadioFrame *aFrame);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    bool  ShouldHandleReceivedFrame(const otRadioFrame &aFrame) const;
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void Output(const char *aFormat, ...);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void ResetTxPacket(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void OutputStats(void);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    void UpdateTxStats(Error aError);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static bool IsChannelValid(uint8_t aChannel);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static bool IsFrameLengthValid(uint16_t aLength);
    SL_CODE_CLASSIFY(SL_CODE_COMPONENT_OPENTHREAD, SL_CODE_CLASS_TIME_CRITICAL)
    static const struct Command sCommands[];

#if OPENTHREAD_FTD || OPENTHREAD_MTD || (OPENTHREAD_RADIO && OPENTHREAD_RADIO_CLI)
    enum TxCmd : uint8_t
    {
        kTxCmdNone,
        kTxCmdRepeat,
        kTxCmdSend,
        kTxCmdSweep,
    };

    Stats mStats;

    otRadioFrame *mTxPacket;
    uint32_t      mTxPeriod;
    uint32_t      mTxPackets;
    uint8_t       mChannel;
    int8_t        mTxPower;
    uint16_t      mTxLen;
    TxCmd         mCurTxCmd;
    bool          mIsHeaderUpdated : 1;
    bool          mIsSecurityProcessed : 1;
    bool          mIsTxPacketSet : 1;
    bool          mIsAsyncSend : 1;
    bool          mDiagSendOn : 1;
    bool          mIsSleepOn : 1;
    bool          mIsAsyncSweep : 1;
#endif

    ReceiveConfig        mReceiveConfig;
    otDiagOutputCallback mOutputCallback;
    void                *mOutputContext;
};

} // namespace FactoryDiags
} // namespace ot

#endif // #if OPENTHREAD_CONFIG_DIAG_ENABLE

#endif // FACTORY_DIAGS_HPP_
