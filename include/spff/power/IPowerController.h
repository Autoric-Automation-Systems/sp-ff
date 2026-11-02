#pragma once

namespace spff
{

// Controle de execução do firmware: reiniciar para ativar uma imagem OTA e
// informar em qual slot ela está rodando.
class IPowerController
{
public:
  virtual ~IPowerController() = default;

  // Reinicia para ativar uma imagem recém-gravada na partição inativa.
  virtual void restart() = 0;

  // Slot de firmware em execução ("app0"/"app1"), para confirmar o OTA.
  virtual const char *firmwareSlot() = 0;
};

} // namespace spff
