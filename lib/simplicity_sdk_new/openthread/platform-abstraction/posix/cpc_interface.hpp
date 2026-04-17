/*******************************************************************************
 * @file
 * @brief This file includes the implementation for the CPCd interface to radio (RCP).
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

#include "platform-posix.h"
#include "vendor_interface.hpp"

#include "sl_cpc.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <syslog.h>
#include <termios.h>
#include <unistd.h>

#include "common/code_utils.hpp"
#include "common/logging.hpp"
#include "common/new.hpp"
#include "lib/spinel/spinel.h"

#if OPENTHREAD_POSIX_CONFIG_SPINEL_VENDOR_INTERFACE_ENABLE

namespace ot {
namespace Posix {

/**
 * This class defines an implementation for a CPC interface to the Radio Co-processor (RCP)
 *
 */

class CpcInterfaceImpl : public ot::Spinel::SpinelInterface
{
public:
    /**
     * This constructor initializes the object.
     *
     * @param[in] aRadioUrl RadioUrl parsed from radio url.
     *
     */
    CpcInterfaceImpl(const Url::Url &aRadioUrl);

    /**
     * This destructor deinitializes the object.
     *
     */
    ~CpcInterfaceImpl(void);

    /**
     * This method initializes the interface to the Radio Co-processor (RCP).
     *
     * @note This method should be called before reading and sending spinel frames to the interface.
     *
     * @param[in] aCallback         Callback on frame received
     * @param[in] aCallbackContext  Callback context
     * @param[in] aFrameBuffer      A reference to a `RxFrameBuffer` object.
     *
     * @retval OT_ERROR_NONE       The interface is initialized successfully
     * @retval OT_ERROR_ALREADY    The interface is already initialized.
     * @retval OT_ERROR_FAILED     Failed to initialize the interface.
     *
     */
    otError Init(ReceiveFrameCallback aCallback, void *aCallbackContext, RxFrameBuffer &aFrameBuffer);

    /**
     * This method deinitializes the interface to the RCP.
     *
     */
    void Deinit(void);

    /**
     * This method encodes and sends a spinel frame to Radio Co-processor (RCP) over the socket.
     *
     * @param[in] aFrame   A pointer to buffer containing the spinel frame to send.
     * @param[in] aLength  The length (number of bytes) in the frame.
     *
     * @retval OT_ERROR_NONE     Successfully encoded and sent the spinel frame.
     * @retval OT_ERROR_BUSY     Failed due to another operation is on going.
     * @retval OT_ERROR_NO_BUFS  Insufficient buffer space available to encode the frame.
     * @retval OT_ERROR_FAILED   Failed to call the SPI driver to send the frame.
     *
     */
    otError SendFrame(const uint8_t *aFrame, uint16_t aLength);

    /**
     * This method waits for receiving part or all of spinel frame within specified interval.
     *
     * @param[in] aTimeoutUs  The timeout value in microseconds.
     *
     * @retval OT_ERROR_NONE             Part or all of spinel frame is received.
     * @retval OT_ERROR_RESPONSE_TIMEOUT No spinel frame is received within @p aTimeout.
     *
     */
    otError WaitForFrame(uint64_t aTimeoutUs);

    /**
     * This method updates the file descriptor sets with file descriptors used by the radio driver.
     *
     * @param[in,out] aMainloopContext  A pointer to the mainloop context.
     *
     */
    void UpdateFdSet(void *aMainloopContext);

    /**
     * This method performs radio driver processing.
     *
     * @param[in] aMainloopContext  A pointer to the mainloop context.
     *
     */
    void Process(const void *aMainloopContext);

    /**
     * This method returns the bus speed between the host and the radio.
     *
     * @returns  Bus speed in bits/second.
     *
     */
    uint32_t GetBusSpeed(void) const { return mCpcBusSpeed; }

    /**
     * This method hardware resets the RCP.
     *
     * @retval OT_ERROR_NONE             Reset the connection successfully.
     * @retval OT_ERROR_NOT_IMPLEMENTED  The hardware reset is not implemented..
     *
     */
    otError HardwareReset(void) { return OT_ERROR_NOT_IMPLEMENTED; }

    /**
     * This method returns the RCP interface metrics.
     *
     * @returns The RCP interface metrics.
     *
     */
    const otRcpInterfaceMetrics *GetRcpInterfaceMetrics(void) const { return &mInterfaceMetrics; }

    /**
     * Indicates whether or not the given interface matches this interface name.
     *
     * @param[in] aInterfaceName A pointer to the interface name.
     *
     * @retval TRUE   The given interface name matches this interface name.
     * @retval FALSE  The given interface name doesn't match this interface name.
     */
    static bool IsInterfaceNameMatch(const char *aInterfaceName)
    {
        static const char *kInterfaceName = OPENTHREAD_POSIX_CONFIG_SPINEL_VENDOR_INTERFACE_URL_PROTOCOL_NAME;
        return (strncmp(aInterfaceName, kInterfaceName, strlen(kInterfaceName)) == 0);
    }

    /**
     * This method is called reinitialise the CPC interface if sCpcResetReq indicates that a restart
     * is required.
     */
    void CheckAndReInitCpc(void);

private:
    /**
     * This method instructs `CpcInterface` to read data from radio over the socket.
     *
     * The mReceiveFrameCallback is used to pass the received frame to be processed.
     *
     */
    void Read(uint64_t aTimeoutUs);

    /**
     * This method waits for the socket file descriptor associated with the HDLC interface to become writable within
     * `kMaxWaitTime` interval.
     *
     * @retval OT_ERROR_NONE   Socket is writable.
     * @retval OT_ERROR_FAILED Socket did not become writable within `kMaxWaitTime`.
     *
     */
    otError WaitForWritable(void);

    /**
     * This method writes a given frame to the socket.
     *
     * This is blocking call, i.e., if the socket is not writable, this method waits for it to become writable for
     * up to `kMaxWaitTime` interval.
     *
     * @param[in] aFrame  A pointer to buffer containing the frame to write.
     * @param[in] aLength The length (number of bytes) in the frame.
     *
     * @retval OT_ERROR_NONE    Frame was written successfully.
     * @retval OT_ERROR_FAILED  Failed to write due to socket not becoming writable within `kMaxWaitTime`.
     *
     */
    otError Write(const uint8_t *aFrame, uint16_t aLength);

    /**
     * This method generates and sends a reset response back to OT.
     *
     * This method is called after the CPC layer catches the SPINEL reset command. This is done so that
     * CPC can handle resets of the RCP and OT is tricked into thinking it handles resets.
     *
     */
    void SendResetResponse(void);

    enum
    {
        kMaxFrameSize = SL_CPC_READ_MINIMUM_SIZE,
        kMaxWaitTime  = 2000, ///< Maximum wait time in Milliseconds for socket to become writable (see `SendFrame`).
        kMaxSleepDuration   = 100000, ///< Sleep duration in micro seconds before restarting cpc connection/endpoint.
        kMaxRestartAttempts = 300,
        kResetCMDSize       = 4,
        kCpcBusSpeed        = 115200,
    };

    ReceiveFrameCallback mReceiveFrameCallback;
    void                *mReceiveFrameContext;
    RxFrameBuffer       *mReceiveFrameBuffer;
    const Url::Url      &mRadioUrl;

    int            mSockFd;
    cpc_handle_t   mHandle;
    cpc_endpoint_t mEndpoint;
    uint32_t       mCpcBusSpeed;

    static void HandleSecondaryReset(void);
    static void SetCpcResetReq(bool state) { sCpcResetReq = state; }

    // Hard Coded Reset Response
    // 0x72 -> STATUS_RESET_SOFTWARE
    uint8_t mResetResponse[kResetCMDSize] = {0x80, 0x06, 0x00, 0x72};

    const uint8_t                mId = SL_CPC_ENDPOINT_15_4;
    typedef uint8_t              cpcError;
    static volatile sig_atomic_t sCpcResetReq;
    static bool                  sIsCpcInitialized;

    otRcpInterfaceMetrics mInterfaceMetrics;

    // Non-copyable, intentionally not implemented.
    CpcInterfaceImpl(const CpcInterfaceImpl &);
    CpcInterfaceImpl &operator=(const CpcInterfaceImpl &);

protected:
    /**
     * Indicates whether or not the frame is the Spinel SPINEL_CMD_RESET frame.
     *
     * @param[in] aFrame   A pointer to buffer containing the spinel frame.
     * @param[in] aLength  The length (number of bytes) in the frame.
     *
     * @retval true  If the frame is a Spinel SPINEL_CMD_RESET frame.
     * @retval false If the frame is not a Spinel SPINEL_CMD_RESET frame.
     *
     */
    bool IsSpinelResetCommand(const uint8_t *aFrame, uint16_t aLength)
    {
        const uint8_t kSpinelResetCommandLength = 2;

        if (aLength >= kSpinelResetCommandLength)
        {
            // Validate the header flag by masking out the iid bits as it is validated above.
            VerifyOrExit(((aFrame[0] & ~SPINEL_HEADER_IID_MASK) == SPINEL_HEADER_FLAG));

            // Validate the reset command.
            VerifyOrExit(aFrame[1] == SPINEL_CMD_RESET);

            return true;
        }

    exit:
        return false;
    }
};

} // namespace Posix
} // namespace ot
#endif // OPENTHREAD_POSIX_CONFIG_SPINEL_VENDOR_INTERFACE_ENABLE
