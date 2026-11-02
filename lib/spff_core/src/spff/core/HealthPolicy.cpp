#include "spff/core/HealthPolicy.h"

namespace spff
{

void HealthPolicy::recordSend(bool ok)
{
  if (ok)
  {
    sendFailures_ = 0;
    return;
  }
  if (sendFailures_ < UINT8_MAX)
  {
    sendFailures_++;
  }
}

void HealthPolicy::recordWifi(bool ok)
{
  if (ok)
  {
    wifiFailures_ = 0;
    return;
  }
  if (wifiFailures_ < UINT8_MAX)
  {
    wifiFailures_++;
  }
  if (portalAfterWifiFailures_ > 0 && wifiFailures_ % portalAfterWifiFailures_ == 0)
  {
    reconfigurationPending_ = true;
  }
}

bool HealthPolicy::consumeReconfigurationRequest()
{
  const bool pending = reconfigurationPending_;
  reconfigurationPending_ = false;
  return pending;
}

} // namespace spff
