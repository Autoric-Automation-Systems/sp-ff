#pragma once

#include <Preferences.h>
#include <cstdint>

#include "spff/core/IntervalLimits.h"

namespace spff
{

// Persistência de configuração em NVS (sobrevive a reboot, queda de energia
// e regravação de firmware).
class SettingsStore
{
public:
  SettingsStore(const IntervalLimits &limits, const char *nameSpace);

  uint32_t heartbeatInterval();
  void saveHeartbeatInterval(uint32_t valueMs);

private:
  IntervalLimits limits_;
  const char *nameSpace_;
  Preferences preferences_;
};

} // namespace spff
