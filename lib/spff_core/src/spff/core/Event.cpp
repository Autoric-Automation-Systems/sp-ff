#include "spff/core/Event.h"

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

} // namespace spff
