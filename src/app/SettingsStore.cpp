#include "spff/app/SettingsStore.h"

namespace spff
{
namespace
{

constexpr const char *kHeartbeatKey = "hb_ms";

} // namespace

SettingsStore::SettingsStore(const IntervalLimits &limits, const char *nameSpace)
    : limits_(limits), nameSpace_(nameSpace)
{
}

uint32_t SettingsStore::heartbeatInterval()
{
  preferences_.begin(nameSpace_, true);
  const uint32_t stored = preferences_.getUInt(kHeartbeatKey, limits_.fallbackMs);
  preferences_.end();
  return sanitizeInterval(stored, limits_);
}

void SettingsStore::saveHeartbeatInterval(uint32_t valueMs)
{
  preferences_.begin(nameSpace_, false);
  preferences_.putUInt(kHeartbeatKey, sanitizeInterval(valueMs, limits_));
  preferences_.end();
}

} // namespace spff
