#pragma once

#include <cstddef>
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

// Lote máximo enviado por requisição (o servidor aceita até 200).
constexpr size_t kMaxEventsPerBatch = 20;

void setEventName(Event &event, const char *name);

// Arredonda para 2 casas: mantém o valor estável no armazenamento e no log.
float roundEventValue(float value);

// Formata o valor do evento para o JSON: sempre 2 casas ("29.64").
// O ArduinoJson imprime float com a expansão completa (29.63999939), por isso
// o número é escrito como texto formatado na hora de serializar.
void formatEventValue(float value, char *buffer, size_t size);

} // namespace spff
