#pragma once

#include <cstdint>

namespace spff
{

// Versão no formato "MAJOR.MINOR[.PATCH]" (ex.: "1.2", "1.2.3").
// Comparação numérica componente a componente: texto puro compara errado
// ("1.10" < "1.9"), então o servidor não pode ditar a ordem dos dígitos.
struct FirmwareVersion
{
  uint16_t major;
  uint16_t minor;
  uint16_t patch;
};

// Componentes não numéricos viram 0 ("1.2-beta" -> 1.2.0); texto vazio é inválido.
FirmwareVersion parseFirmwareVersion(const char *text);

// true quando `candidate` é mais nova que `current`.
bool isNewerVersion(const char *candidate, const char *current);

} // namespace spff
