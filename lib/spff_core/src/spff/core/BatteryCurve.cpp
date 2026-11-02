#include "spff/core/BatteryCurve.h"

#include <cmath>

namespace spff
{
namespace battery
{
namespace
{

struct Point
{
  float volts;
  int percent;
};

// Tensão em repouso x estado de carga aproximado de uma célula Li-ion.
constexpr Point kCurve[] = {
    {4.20f, 100},
    {4.10f, 90},
    {4.00f, 80},
    {3.93f, 70},
    {3.87f, 60},
    {3.82f, 50},
    {3.79f, 40},
    {3.75f, 30},
    {3.70f, 20},
    {3.60f, 10},
    {3.45f, 5},
    {3.30f, 0},
};

constexpr int kPointCount = sizeof(kCurve) / sizeof(kCurve[0]);

} // namespace

int percentFromVoltage(float volts)
{
  if (volts >= kCurve[0].volts)
  {
    return 100;
  }
  if (volts <= kCurve[kPointCount - 1].volts)
  {
    return 0;
  }

  for (int i = 0; i < kPointCount - 1; i++)
  {
    const Point &high = kCurve[i];
    const Point &low = kCurve[i + 1];
    if (volts <= high.volts && volts >= low.volts)
    {
      const float ratio = (volts - low.volts) / (high.volts - low.volts);
      const float percent = low.percent + ratio * (high.percent - low.percent);
      return static_cast<int>(lroundf(percent));
    }
  }
  return 0;
}

} // namespace battery
} // namespace spff
