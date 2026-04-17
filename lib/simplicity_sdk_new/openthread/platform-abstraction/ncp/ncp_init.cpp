/*******************************************************************************
 * @file
 * @brief Initialization routines for non-CPC NCP interfaces
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

#if !OPENTHREAD_CONFIG_NCP_CPC_ENABLE

#include "openthread-core-config.h"

#include <openthread/ncp.h>

#include "common/code_utils.hpp"
#include "common/new.hpp"

#include "ncp/ncp_config.h"

#if OPENTHREAD_CONFIG_NCP_SPI_ENABLE
#include "ncp_spi.hpp"
#else
#include "ncp_hdlc.hpp"
#include "utils/uart.h"

void otPlatUartReceived(const uint8_t *aBuf, uint16_t aBufLength)
{
    otNcpHdlcReceive(aBuf, aBufLength);
}
void otPlatUartSendDone(void)
{
    otNcpHdlcSendDone();
}
#endif

/**
 * otNcpSpiInit and otNcpHdlcInit are not defined when OPENTHREAD_ENABLE_NCP_VENDOR_HOOK
 * is configured. In addition, otAppNcpInit must be implemented when OPENTHREAD_ENABLE_NCP_VENDOR_HOOK
 * is configured.
 */
#if OPENTHREAD_ENABLE_NCP_VENDOR_HOOK

namespace ot {
namespace Ncp {

#if OPENTHREAD_CONFIG_NCP_HDLC_ENABLE
static int NcpSend(const uint8_t *aBuf, uint16_t aBufLength)
{
    IgnoreError(otPlatUartSend(aBuf, aBufLength));
    return aBufLength;
}
#endif

#if OPENTHREAD_CONFIG_NCP_SPI_ENABLE
static OT_DEFINE_ALIGNED_VAR(sNcpRaw, sizeof(NcpSpi), uint64_t);

extern "C" void otNcpSpiInit(otInstance *aInstance)
{
    NcpSpi   *ncpSpi   = nullptr;
    Instance *instance = static_cast<Instance *>(aInstance);

    ncpSpi = new (&sNcpRaw) NcpSpi(instance);

    if (ncpSpi == nullptr || ncpSpi != NcpBase::GetNcpInstance())
    {
        OT_ASSERT(false);
    }
}

#elif OPENTHREAD_CONFIG_NCP_HDLC_ENABLE
static OT_DEFINE_ALIGNED_VAR(sNcpRaw, sizeof(NcpHdlc), uint64_t);

extern "C" void otNcpHdlcInit(otInstance *aInstance, otNcpHdlcSendCallback aSendCallback)
{
    NcpHdlc  *ncpHdlc  = nullptr;
    Instance *instance = static_cast<Instance *>(aInstance);

    ncpHdlc = new (&sNcpRaw) NcpHdlc(instance, aSendCallback);

    if (ncpHdlc == nullptr || ncpHdlc != NcpBase::GetNcpInstance())
    {
        OT_ASSERT(false);
    }
}

#endif // OPENTHREAD_CONFIG_NCP_SPI_ENABLE

extern "C" void otAppNcpInit(otInstance *aInstance)
{
#if OPENTHREAD_CONFIG_NCP_SPI_ENABLE
    otNcpSpiInit(aInstance);
#elif OPENTHREAD_CONFIG_NCP_HDLC_ENABLE
    IgnoreError(otPlatUartEnable());
    otNcpHdlcInit(aInstance, NcpSend);
#endif
}

} // namespace Ncp
} // namespace ot

#endif // OPENTHREAD_ENABLE_NCP_VENDOR_HOOK
#endif // !OPENTHREAD_CONFIG_NCP_CPC_ENABLE
