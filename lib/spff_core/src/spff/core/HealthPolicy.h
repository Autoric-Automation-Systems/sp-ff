#pragma once

#include <cstdint>

namespace spff
{

// Decide quando o dispositivo deve se reconfigurar ou reiniciar sozinho,
// a partir do histórico de falhas de rede e de envio.
class HealthPolicy
{
public:
  HealthPolicy(uint8_t portalAfterWifiFailures, uint8_t restartAfterSendFailures)
      : portalAfterWifiFailures_(portalAfterWifiFailures),
        restartAfterSendFailures_(restartAfterSendFailures) {}

  void recordSend(bool ok);
  void recordWifi(bool ok);

  uint8_t sendFailures() const { return sendFailures_; }
  uint8_t wifiFailures() const { return wifiFailures_; }

  bool requiresRestart() const { return sendFailures_ >= restartAfterSendFailures_; }

  // Consome o pedido: devolve true uma única vez a cada bloco de falhas.
  bool consumeReconfigurationRequest();

private:
  uint8_t portalAfterWifiFailures_;
  uint8_t restartAfterSendFailures_;
  uint8_t sendFailures_ = 0;
  uint8_t wifiFailures_ = 0;
  bool reconfigurationPending_ = false;
};

} // namespace spff
