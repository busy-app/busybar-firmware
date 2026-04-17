/*******************************************************************************
 * @file
 * @brief Definitions for an extended NCP spinel interface to the OpenThread stack
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

#ifndef _NCP_SPINEL_HPP
#define _NCP_SPINEL_HPP

#include "lib/spinel/spinel.h"

namespace ot {
namespace Spinel {
namespace Vendor {

enum
{
    SPINEL_PROP_VENDOR_ANTENNA      = (SPINEL_PROP_VENDOR__BEGIN + 0),
    SPINEL_PROP_VENDOR_COEX         = (SPINEL_PROP_VENDOR__BEGIN + 1),
    SPINEL_PROP_VENDOR_TEST         = (SPINEL_PROP_VENDOR__BEGIN + 2),
    SPINEL_PROP_VENDOR_EFR32        = (SPINEL_PROP_VENDOR__BEGIN + 3),
    SPINEL_PROP_VENDOR_MEMORY_USAGE = (SPINEL_PROP_VENDOR__BEGIN + 4),
};
} // namespace Vendor
} // namespace Spinel

namespace Vendor {
namespace Antenna {
enum
{
    ANT_TX_MODE_COMMAND,
    ANT_RX_MODE_COMMAND,
    ANT_ACTIVE_PHY_COMMAND,
};

} // namespace Antenna

namespace Coex {
enum
{
    COEX_DP_STATE_COMMAND,
    COEX_GPIO_INPUT_OVERRIDE_COMMAND,
    COEX_ACTIVE_RADIO_COMMAND,
    COEX_PHY_SELECT_TIMEOUT_COMMAND,
    COEX_PTA_OPTIONS_COMMAND,
    COEX_CONSTANT_OPTIONS_COMMAND,
    COEX_PTA_STATE_COMMAND,
    COEX_PWM_STATE_COMMAND,
    COEX_COUNTERS_COMMAND,
    COEX_RADIO_HOLDOFF_COMMAND,
};

} // namespace Coex

namespace Test {
enum
{
    GEN_PTI_RADIO_CONFIG_COMMAND,
    GEN_CCA_MODE_COMMAND,
};

} // namespace Test

namespace Efr32 {
enum
{
    EFR32_RADIO_COUNTERS_COMMAND,
};

} // namespace Efr32

namespace MemoryUsage {
enum
{
    MEMORY_USAGE_USED_HEAP_COMMAND,
    MEMORY_USAGE_HIGH_WATERMARK_COMMAND,
};

} // namespace MemoryUsage
} // namespace Vendor
} // namespace ot
#endif // _NCP_SPINEL_HPP
