#pragma once

#include <cstdint>

namespace spff
{

// Fonte de tempo injetável (permite testar a aplicação com relógio falso).
class IClock
{
public:
  virtual ~IClock() = default;

  // Epoch UTC em segundos; 0 quando ainda não existe referência de tempo.
  virtual uint32_t epochNow() = 0;

  // Garante a sincronização (bloqueante, respeitando o timeout).
  virtual bool ensureSynced(uint32_t timeoutMs) = 0;
};

} // namespace spff
