#include "spff/hw/BatteryGauge.h"

#include <Arduino.h>

#include "spff/core/BatteryCurve.h"

namespace spff
{

BatteryGauge::BatteryGauge(uint8_t pin, float divider, uint8_t samples)
    : pin_(pin), divider_(divider), samples_(samples == 0 ? 1 : samples)
{
}

BatteryReading BatteryGauge::read()
{
  const float measured = voltage();
  BatteryReading reading{};
  reading.volts = measured;
  reading.percent = battery::percentFromVoltage(measured);
  return reading;
}

float BatteryGauge::voltage()
{
  uint32_t sum = 0;
  for (uint8_t i = 0; i < samples_; i++)
  {
    sum += analogReadMilliVolts(pin_);
    delay(5);
  }
  const float pinVolts = (sum / static_cast<float>(samples_)) / 1000.0f;
  return pinVolts * divider_;
}

} // namespace spff
