#pragma once

#include <cstdint>

namespace spff
{

// Tipo POD: vive também em memória RTC, então não pode ter construtor,
// destrutor nem inicializadores de membro.
struct Event
{
  char name[12];
  float value;
  uint32_t ts; // 0 = hora ainda desconhecida (carimbada no envio)
};

constexpr uint16_t kEventNameLength = sizeof(Event::name);

void setEventName(Event &event, const char *name);

} // namespace spff
