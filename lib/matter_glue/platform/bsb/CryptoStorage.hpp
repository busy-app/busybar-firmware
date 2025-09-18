#pragma once

#include <lib/support/Span.h>

#include <furi_hal_crypto_storage.h>

namespace chip {
namespace DeviceLayer {
namespace BSB {

CHIP_ERROR
LoadCryptoStorageItem(FuriHalCryptoKeyType key_type, uint32_t key_id, MutableByteSpan& out_buf);

CHIP_ERROR SignWithECDSA256Key(
    FuriHalCryptoKeyType key_type,
    uint32_t key_id,
    const ByteSpan& message,
    MutableByteSpan& out_buf);

} // namespace BSB
} // namespace DeviceLayer
} // namespace Chip
