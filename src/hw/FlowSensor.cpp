#include "spff/hw/FlowSensor.h"

namespace spff
{

volatile uint32_t FlowSensor::pulseCount_ = 0;

FlowSensor::FlowSensor(uint8_t pin, uint16_t pulsesPerLiter)
    : pin_(pin), pulsesPerLiter_(pulsesPerLiter == 0 ? 1 : pulsesPerLiter)
{
}

void FlowSensor::begin()
{
  pinMode(pin_, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin_), handlePulse, RISING);
}

uint32_t FlowSensor::consumeLiters()
{
  noInterrupts();
  const uint32_t pulses = pulseCount_;
  pulseCount_ = 0;
  interrupts();

  pendingPulses_ += pulses;
  const uint32_t liters = pendingPulses_ / pulsesPerLiter_;
  pendingPulses_ -= liters * pulsesPerLiter_;
  return liters;
}

void IRAM_ATTR FlowSensor::handlePulse()
{
  pulseCount_ = pulseCount_ + 1;
}

} // namespace spff
