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

// Último RSSI reportado (0 = nunca), para a histerese sobreviver ao restart.
int32_t lastReportedRssi();
void saveLastReportedRssi(int32_t rssi);

} // namespace spff
