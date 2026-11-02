#include "spff/core/FirmwareVersion.h"

#include <cctype>

namespace spff
{
namespace
{

// Lê um componente numérico e avança o cursor. Sem dígitos, vale 0.
uint16_t readComponent(const char *&cursor)
{
  uint32_t value = 0;
  while (isdigit(static_cast<unsigned char>(*cursor)))
  {
    // Satura em 65535: versão absurda não estoura nem volta a ser menor.
    if (value < 65535U)
    {
      value = value * 10U + static_cast<uint32_t>(*cursor - '0');
    }
    cursor++;
  }
  return static_cast<uint16_t>(value > 65535U ? 65535U : value);
}

} // namespace

FirmwareVersion parseFirmwareVersion(const char *text)
{
  FirmwareVersion version{0, 0, 0};
  if (text == nullptr)
  {
    return version;
  }

  const char *cursor = text;
  while (*cursor != '\0' && !isdigit(static_cast<unsigned char>(*cursor)))
  {
    cursor++;
  }

  version.major = readComponent(cursor);
  if (*cursor == '.')
  {
    cursor++;
    version.minor = readComponent(cursor);
    if (*cursor == '.')
    {
      cursor++;
      version.patch = readComponent(cursor);
    }
  }
  return version;
}

bool isNewerVersion(const char *candidate, const char *current)
{
  if (candidate == nullptr || candidate[0] == '\0')
  {
    return false;
  }

  const FirmwareVersion remote = parseFirmwareVersion(candidate);
  const FirmwareVersion local = parseFirmwareVersion(current);

  if (remote.major != local.major)
  {
    return remote.major > local.major;
  }
  if (remote.minor != local.minor)
  {
    return remote.minor > local.minor;
  }
  return remote.patch > local.patch;
}

} // namespace spff
