#pragma once

#include <cstdint>

namespace spff
{

struct IntervalLimits
{
  uint32_t minMs;
  uint32_t maxMs;
  uint32_t fallbackMs;
};

bool isIntervalAllowed(uint32_t value, const IntervalLimits &limits);

// Valor fora da faixa vira o padrão: um 0 deixaria o aparelho em loop de WiFi
// e um valor absurdo o deixaria mudo até alguém reiniciar.
uint32_t sanitizeInterval(uint32_t value, const IntervalLimits &limits);

} // namespace spff
