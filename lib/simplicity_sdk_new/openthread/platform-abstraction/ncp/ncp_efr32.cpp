/*******************************************************************************
 * @file
 * @brief Definitions for a spinel extension to support the efr32/platform API
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

#include "ncp_efr32.hpp"
#include "radio_counters.h"
#include "radio_extension.h"
#include "vendor_spinel.hpp"

#include "common/code_utils.hpp"

namespace Efr32Cmd = ot::Vendor::Efr32;

namespace ot {
namespace Ncp {
namespace Vendor {
namespace Efr32 {

static otError getRadioCounters(Spinel::Encoder &aEncoder);
static otError clearRadioCounters(void);

otError getEfr32Property(Spinel::Decoder &aDecoder, Spinel::Encoder &aEncoder)
{
    otError error = OT_ERROR_NOT_FOUND;
    uint8_t cmdKey;

    SuccessOrExit(aDecoder.ReadUint8(cmdKey));

    switch (cmdKey)
    {
    case Efr32Cmd::EFR32_RADIO_COUNTERS_COMMAND:
        error = getRadioCounters(aEncoder);
        break;
    }

exit:
    return error;
}

otError setEfr32Property(Spinel::Decoder &aDecoder)
{
    otError error = OT_ERROR_NOT_FOUND;
    uint8_t cmdKey;

    SuccessOrExit(aDecoder.ReadUint8(cmdKey));

    switch (cmdKey)
    {
    case Efr32Cmd::EFR32_RADIO_COUNTERS_COMMAND:
        error = clearRadioCounters();
        break;
    }

exit:
    return error;
}

otError getRadioCounters(Spinel::Encoder &aEncoder)
{
    otError            error = OT_ERROR_NONE;
    efr32RadioCounters radioCounters;

    SuccessOrExit(error = otPlatRadioExtensionGetRadioCounters(&radioCounters));
    SuccessOrExit(error = aEncoder.WriteDataWithLen((const uint8_t *)&radioCounters, sizeof(efr32RadioCounters)));
exit:
    return error;
}

otError clearRadioCounters(void)
{
    return otPlatRadioExtensionClearRadioCounters();
}

} // namespace Efr32
} // namespace Vendor
} // namespace Ncp
} // namespace ot
