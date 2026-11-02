#pragma once

#include <cstdint>

namespace spff
{

// Agenda períodos sem estourar no rollover do millis().
class IntervalTimer
{
public:
  explicit IntervalTimer(uint32_t periodMs = 0) : periodMs_(periodMs) {}

  void setPeriod(uint32_t periodMs) { periodMs_ = periodMs; }
  uint32_t period() const { return periodMs_; }

  void start(uint32_t now) { deadline_ = now; }
  void scheduleNext(uint32_t now) { deadline_ = now + periodMs_; }

  bool due(uint32_t now) const { return static_cast<int32_t>(now - deadline_) >= 0; }

private:
  uint32_t periodMs_;
  uint32_t deadline_ = 0;
};

} // namespace spff
