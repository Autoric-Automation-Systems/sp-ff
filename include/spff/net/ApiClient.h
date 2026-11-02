#pragma once

#include <cstddef>
#include <cstdint>

#include "spff/core/Event.h"
#include "spff/net/ITransport.h"

namespace spff
{

struct HeartbeatStatus
{
  const char *mac;
  const char *deviceType;
  const char *version;
  int rssi;
  const char *ip;
  uint32_t uptimeMs;
  uint32_t boots;
  uint32_t freeHeap;
};

// Contrato com a API do Smart Plant. Não sabe nada sobre WiFi nem sobre a fila.
class ApiClient
{
public:
  ApiClient(ITransport &transport, const char *baseUrl);

  // Envia o heartbeat. Retorna true em 2xx. `newIntervalMs` volta 0 quando o
  // servidor não informou um intervalo.
  bool sendHeartbeat(const HeartbeatStatus &status, uint32_t &newIntervalMs);

  // Envia os eventos em lotes e devolve quantos o servidor confirmou
  // (o chamador remove exatamente essa quantidade da fila).
  size_t sendEvents(const char *mac, const Event *events, size_t count, size_t batchSize, uint32_t nowEpoch);

private:
  bool postJson(const char *url, const char *body);

  ITransport &transport_;
  char heartbeatUrl_[96];
  char eventsUrl_[96];
  char body_[2048];
  char response_[512];
};

} // namespace spff
