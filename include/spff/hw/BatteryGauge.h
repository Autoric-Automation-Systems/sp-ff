#pragma once

#include <cstdint>

namespace spff
{

struct BatteryReading
{
  float volts;
  int percent;
};

// Leitura da bateria pelo divisor resistivo da placa medidora (0-25 VDC).
class BatteryGauge
{
public:
  BatteryGauge(uint8_t pin, float divider, uint8_t samples);

  // Uma única passada de amostragem: tensão média e percentual pela curva Li-ion.
  BatteryReading read();

private:
  float voltage();

  uint8_t pin_;
  float divider_;
  uint8_t samples_;
};

} // namespace spff
