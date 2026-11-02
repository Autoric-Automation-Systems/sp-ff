#pragma once

#include <Arduino.h>
#include <cstdint>

namespace spff
{

// Sensor de vazão por pulsos (YF-S201). A ISR só incrementa um contador;
// a conversão para litros acontece no contexto da aplicação.
class FlowSensor
{
public:
  FlowSensor(uint8_t pin, uint16_t pulsesPerLiter);

  void begin();

  // Litros completos acumulados desde a última chamada (o resto é preservado).
  uint32_t consumeLiters();

private:
  static void IRAM_ATTR handlePulse();

  static volatile uint32_t pulseCount_;

  uint8_t pin_;
  uint16_t pulsesPerLiter_;
  uint32_t pendingPulses_ = 0;
};

} // namespace spff
