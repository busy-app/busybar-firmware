/*******************************************************************************
 * @file
 * @brief Definitions for a spinel extension to support antenna diversity
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

#include "ncp_ant_div.hpp"
#include "radio_extension.h"
#include "vendor_spinel.hpp"

#include "common/code_utils.hpp"

namespace AntennaCmd = ot::Vendor::Antenna;

namespace ot {
namespace Ncp {
namespace Vendor {
namespace Antenna {

static otError getTxAntennaMode(Spinel::Encoder &aEncoder);
static otError setTxAntennaMode(Spinel::Decoder &aDecoder);
static otError getRxAntennaMode(Spinel::Encoder &aEncoder);
static otError setRxAntennaMode(Spinel::Decoder &aDecoder);
static otError getActivePhy(Spinel::Encoder &aEncoder);

otError getAntennaProperty(Spinel::Decoder &aDecoder, Spinel::Encoder &aEncoder)
{
    otError error = OT_ERROR_NOT_FOUND;
    uint8_t cmdKey;

    SuccessOrExit(aDecoder.ReadUint8(cmdKey));

    switch (cmdKey)
    {
    case AntennaCmd::ANT_TX_MODE_COMMAND:
        error = getTxAntennaMode(aEncoder);
        break;
    case AntennaCmd::ANT_RX_MODE_COMMAND:
        error = getRxAntennaMode(aEncoder);
        break;
    case AntennaCmd::ANT_ACTIVE_PHY_COMMAND:
        error = getActivePhy(aEncoder);
        break;
    }

exit:
    return error;
}

otError setAntennaProperty(Spinel::Decoder &aDecoder)
{
    otError error = OT_ERROR_NOT_FOUND;
    uint8_t cmdKey;

    SuccessOrExit(aDecoder.ReadUint8(cmdKey));

    switch (cmdKey)
    {
    case AntennaCmd::ANT_TX_MODE_COMMAND:
        error = setTxAntennaMode(aDecoder);
        break;
    case AntennaCmd::ANT_RX_MODE_COMMAND:
        error = setRxAntennaMode(aDecoder);
        break;
    }

exit:
    return error;
}

otError getTxAntennaMode(Spinel::Encoder &aEncoder)
{
    uint8_t mode = 0;

    IgnoreError(otPlatRadioExtensionGetTxAntennaMode(&mode));

    return (aEncoder.WriteUint8(mode));
}

otError setTxAntennaMode(Spinel::Decoder &aDecoder)
{
    uint8_t mode  = 0;
    otError error = OT_ERROR_NONE;

    SuccessOrExit(error = aDecoder.ReadUint8(mode));

    error = otPlatRadioExtensionSetTxAntennaMode(mode);

exit:
    return error;
}

otError getRxAntennaMode(Spinel::Encoder &aEncoder)
{
    uint8_t mode = 0;

    IgnoreError(otPlatRadioExtensionGetRxAntennaMode(&mode));

    return (aEncoder.WriteUint8(mode));
}

otError setRxAntennaMode(Spinel::Decoder &aDecoder)
{
    uint8_t mode  = 0;
    otError error = OT_ERROR_NONE;

    SuccessOrExit(error = aDecoder.ReadUint8(mode));

    error = otPlatRadioExtensionSetRxAntennaMode(mode);

exit:
    return error;
}

otError getActivePhy(Spinel::Encoder &aEncoder)
{
    uint8_t activePhy = 0;

    IgnoreError(otPlatRadioExtensionGetActivePhy(&activePhy));

    return (aEncoder.WriteUint8(activePhy));
}

} // namespace Antenna
} // namespace Vendor
} // namespace Ncp
} // namespace ot
