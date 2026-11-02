#include "spff/core/SignalMonitor.h"

#include <cstdlib>

namespace spff
{

void SignalMonitor::seed(int rssi)
{
  if (isValid(rssi))
  {
    hasReading_ = true;
    lastReportedRssi_ = rssi;
  }
}

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
