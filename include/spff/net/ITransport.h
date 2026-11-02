#pragma once

#include <cstddef>

namespace spff
{

// Abstração de transporte HTTP (DIP: a API não conhece TLS, HTTPClient nem WiFi).
class ITransport
{
public:
  virtual ~ITransport() = default;

  // Retorna o status HTTP (>0) ou valor negativo em falha de transporte.
  virtual int post(const char *url, const char *body, char *response, size_t responseSize) = 0;
};

} // namespace spff
