#pragma once

#include <lib/support/Span.h>

#include <furi_hal_crypto_storage.h>

namespace chip {
namespace DeviceLayer {
namespace BSB {

template <typename T>
constexpr MutableByteSpan ToMutableByteSpan(T& data) {
    return {reinterpret_cast<uint8_t*>(&data), sizeof(data)};
}

MutableByteSpan ToMutableByteSpan(char* buf, size_t bufSize);

CHIP_ERROR
LoadCryptoStorageKey(FuriHalCryptoKeyType key_type, uint32_t key_id, MutableByteSpan& out_span);

CHIP_ERROR SignWithECDSA256Key(
    FuriHalCryptoKeyType key_type,
    uint32_t key_id,
    const ByteSpan& message,
    MutableByteSpan& out_span);

} // namespace BSB
} // namespace DeviceLayer
} // namespace Chip
