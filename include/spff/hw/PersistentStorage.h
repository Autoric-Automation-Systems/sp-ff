#pragma once

#include <cstdint>

#include "spff/core/EventQueue.h"

namespace spff
{

// Bloco da fila de eventos em memória RTC: sobrevive a reboot por software,
// panic e watchdog (queda de energia limpa a memória).
QueueStorage &persistentQueueStorage();

// Contador de boots em RTC, já sanitizado contra memória indefinida.
uint32_t nextBootCount();

} // namespace spff
