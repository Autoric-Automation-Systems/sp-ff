#pragma once

#include "spff/net/INetworkLink.h"

namespace spff
{

class WifiLink : public INetworkLink
{
public:
  explicit WifiLink(const char *apName) : apName_(apName) {}

  bool autoConnect(uint16_t portalTimeoutSeconds) override;
  bool connect(uint16_t retries) override;
  void disconnect() override;
  bool connected() override;
  int rssi() override;
  const char *localIp() override;
  const char *macAddress() override;
  void openPortal(uint16_t timeoutSeconds) override;

private:
  const char *apName_;
  char ip_[20] = {0};
  char mac_[18] = {0};
};

} // namespace spff
