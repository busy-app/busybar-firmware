/*******************************************************************************
 * @file
 * @brief OpenThread vendor extension for EFR32
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

#include "openthread-core-config.h"

#include "common/code_utils.hpp"
#include "common/new.hpp"

#include "instance/extension.hpp"

#if defined(SL_COMPONENT_CATALOG_PRESENT)
#include "sl_component_catalog.h"
#endif

#include "platform-efr32.h"

namespace ot {
namespace Extension {

/**
 * Defines the vendor extension object.
 *
 */
class Extension : public ExtensionBase
{
public:
    explicit Extension(Instance &aInstance)
        : ExtensionBase(aInstance)
    {
    }
};

// ----------------------------------------------------------------------------
// `ExtensionBase` API
// ----------------------------------------------------------------------------

static OT_DEFINE_ALIGNED_VAR(sExtensionRaw, sizeof(Extension), uint64_t);

ExtensionBase &ExtensionBase::Init(Instance &aInstance)
{
    ExtensionBase *ext = reinterpret_cast<ExtensionBase *>(&sExtensionRaw);

    VerifyOrExit(!ext->mIsInitialized);

    ext = new (&sExtensionRaw) Extension(aInstance);

exit:
    return *ext;
}

void ExtensionBase::SignalInstanceInit(void)
{
    // OpenThread instance is initialized and ready.
}

void ExtensionBase::SignalNcpInit(Ncp::NcpBase &aNcpBase)
{
    // NCP instance is initialized and ready.
    OT_UNUSED_VARIABLE(aNcpBase);
}

void ExtensionBase::HandleNotifierEvents(Events aEvents)
{
    OT_UNUSED_VARIABLE(aEvents);
}

} // namespace Extension
} // namespace ot
