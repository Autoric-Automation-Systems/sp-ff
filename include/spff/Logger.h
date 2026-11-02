#pragma once

#include <Arduino.h>
#include <stdarg.h>

namespace spff
{
namespace log
{

inline void begin(uint32_t baud, uint32_t waitMs = 3000)
{
  Serial.begin(baud);
  const uint32_t start = millis();
  while (!Serial && millis() - start < waitMs)
  {
  }
}

inline void info(const char *format, ...)
{
  va_list args;
  va_start(args, format);
  Serial.vprintf(format, args);
  Serial.println();
  va_end(args);
}

inline void blank()
{
  Serial.println();
}

} // namespace log
} // namespace spff
