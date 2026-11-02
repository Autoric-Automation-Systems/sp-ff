#pragma once

namespace spff
{

// Abstração da atualização de firmware: o orquestrador não conhece HTTP nem
// as partições OTA do ESP32.
class IFirmwareUpdater
{
public:
  virtual ~IFirmwareUpdater() = default;

  // Baixa a imagem e grava na partição inativa. Retorna true quando a imagem
  // foi gravada e validada; a troca de partição acontece no reboot, que é
  // responsabilidade do chamador.
  virtual bool install(const char *url) = 0;
};

} // namespace spff
