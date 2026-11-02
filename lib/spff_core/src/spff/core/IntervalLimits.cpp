#include "spff/core/IntervalLimits.h"

namespace spff
{

bool isIntervalAllowed(uint32_t value, const IntervalLimits &limits)
{
  return value >= limits.minMs && value <= limits.maxMs;
}

uint32_t sanitizeInterval(uint32_t value, const IntervalLimits &limits)
{
  return isIntervalAllowed(value, limits) ? value : limits.fallbackMs;
}

} // namespace spff
