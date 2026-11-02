#pragma once

#include <cstddef>
#include <cstdint>

namespace spff
{

// FNV-1a: checksum barato para validar blocos persistidos em memória RTC,
// que podem vir indefinidos (primeira energização) ou de outro firmware.
constexpr uint32_t kChecksumSeed = 2166136261UL;

uint32_t checksumOf(uint32_t seed, const void *data, size_t length);

} // namespace spff
