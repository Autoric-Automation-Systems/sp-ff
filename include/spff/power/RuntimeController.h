#pragma once

#include "spff/power/IPowerController.h"

namespace spff
{

// SP-FF fica em loop contínuo (sem deep sleep): o controlador cuida apenas do
// reboot e da leitura do slot, que é o que o ciclo precisa.
class RuntimeController : public IPowerController
{
public:
  void restart() override;
  const char *firmwareSlot() override;
};

} // namespace spff
