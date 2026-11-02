#include "spff/core/Event.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace spff
{

void setEventName(Event &event, const char *name)
{
  const size_t maxLength = sizeof(event.name) - 1;
  size_t length = name != nullptr ? strlen(name) : 0;
  if (length > maxLength)
  {
    length = maxLength;
  }

  if (length > 0)
  {
    memcpy(event.name, name, length);
  }
  event.name[length] = '\0';
}

float roundEventValue(float value)
{
  return roundf(value * 100.0f) / 100.0f;
}

void formatEventValue(float value, char *buffer, size_t size)
{
  if (buffer == nullptr || size == 0)
  {
    return;
  }
  if (!std::isfinite(value))
  {
    value = 0.0f;
  }

  snprintf(buffer, size, "%.2f", static_cast<double>(value));

  // No máximo 2 casas: remove zeros à direita (100.00 -> 100, 31.60 -> 31.6)
  char *dot = strchr(buffer, '.');
  if (dot != nullptr)
  {
    char *last = buffer + strlen(buffer) - 1;
    while (last > dot && *last == '0')
    {
      *last-- = '\0';
    }
    if (last == dot)
    {
      *last = '\0';
    }
  }
}

} // namespace spff
