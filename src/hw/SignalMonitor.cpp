#include "spff/hw/SignalMonitor.h"

#include <cstdlib>

namespace spff
{

bool SignalMonitor::shouldReport(int rssi)
{
  if (!hasReading_ || abs(rssi - lastReportedRssi_) >= reportDeltaDbm_)
  {
    hasReading_ = true;
    lastReportedRssi_ = rssi;
    return true;
  }
  return false;
}

} // namespace spff
