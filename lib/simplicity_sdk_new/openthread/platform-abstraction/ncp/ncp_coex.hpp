/*******************************************************************************
 * @file
 * @brief Definitions for a spinel extension to support coex
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

#ifndef _NCP_COEX_HPP
#define _NCP_COEX_HPP

#include "spinel_decoder.hpp"
#include "spinel_encoder.hpp"

namespace ot {
namespace Ncp {
namespace Vendor {
namespace Coex {

otError getCoexProperty(Spinel::Decoder &aDecoder, Spinel::Encoder &aEncoder);
otError setCoexProperty(Spinel::Decoder &aDecoder);

} // namespace Coex
} // namespace Vendor
} // namespace Ncp
} // namespace ot

#endif
