#pragma once

#include <cstdint>

#include "spff/net/IClock.h"

namespace spff
{

// Relógio do dispositivo: SNTP quando há rede, com extrapolação por millis()
// entre as sincronizações (o relógio interno continua contando).
class SystemClock : public IClock
{
public:
  SystemClock(const char *ntpServer, long gmtOffsetSeconds, int daylightOffsetSeconds);

  uint32_t epochNow() override;
  bool ensureSynced(uint32_t timeoutMs) override;

  static constexpr uint32_t kMinValidEpoch = 100000UL;

private:
  const char *ntpServer_;
  long gmtOffsetSeconds_;
  int daylightOffsetSeconds_;
  uint32_t lastValidEpoch_ = 0;
  uint32_t lastValidMillis_ = 0;
};

} // namespace spff
