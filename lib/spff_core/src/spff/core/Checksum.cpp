#include "spff/core/Checksum.h"

namespace spff
{

uint32_t checksumOf(uint32_t seed, const void *data, size_t length)
{
  const uint8_t *bytes = static_cast<const uint8_t *>(data);
  uint32_t hash = seed;
  for (size_t i = 0; i < length; i++)
  {
    hash ^= bytes[i];
    hash *= 16777619UL;
  }
  return hash;
}

} // namespace spff
