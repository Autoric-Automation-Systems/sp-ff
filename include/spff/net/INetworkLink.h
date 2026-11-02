#pragma once

#include <cstdint>

namespace spff
{

// Abstração da conectividade: a aplicação não conhece WiFi, WiFiManager nem AP.
class INetworkLink
{
public:
  virtual ~INetworkLink() = default;

  // Boot: credenciais salvas ou portal de configuração.
  virtual bool autoConnect(uint16_t portalTimeoutSeconds) = 0;

  // Ciclo: apenas credenciais já salvas.
  virtual bool connect(uint16_t retries) = 0;

  virtual void disconnect() = 0;
  virtual bool connected() = 0;
  virtual int rssi() = 0;
  virtual const char *localIp() = 0;
  virtual const char *macAddress() = 0;
  virtual void openPortal(uint16_t timeoutSeconds) = 0;
};

} // namespace spff
