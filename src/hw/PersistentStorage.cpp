#include "spff/hw/PersistentStorage.h"

#include <Arduino.h>
#include <cstddef>

#include "spff/core/Checksum.h"
#include "spff/core/SignalMonitor.h"

namespace spff
{
namespace
{

constexpr uint32_t kBootMagic = 0x424F4F54UL; // "BOOT"
constexpr uint32_t kMaxPlausibleBoots = 1000000UL;

struct BootStorage
{
  uint32_t magic;
  uint32_t count;
  int32_t lastRssi; // 0 = nunca reportado (0 não é RSSI válido)
  uint32_t checksum;
};

RTC_NOINIT_ATTR QueueStorage g_queueStorage;
RTC_NOINIT_ATTR BootStorage g_bootStorage;

uint32_t computeStateChecksum()
{
  return checksumOf(kChecksumSeed, &g_bootStorage, offsetof(BootStorage, checksum));
}

void updateStateChecksum()
{
  g_bootStorage.checksum = computeStateChecksum();
}

void resetState()
{
  g_bootStorage.magic = kBootMagic;
  g_bootStorage.count = 0;
  g_bootStorage.lastRssi = 0;
  updateStateChecksum();
}

} // namespace

QueueStorage &persistentQueueStorage()
{
  return g_queueStorage;
}

uint32_t nextBootCount()
{
  // Sem checksum o bloco não é confiável: memória RTC pode vir indefinida
  // (primeira energização) ou de outro firmware.
  if (g_bootStorage.magic != kBootMagic || g_bootStorage.count > kMaxPlausibleBoots ||
      g_bootStorage.checksum != computeStateChecksum())
  {
    resetState();
  }

  g_bootStorage.count++;
  updateStateChecksum();
  return g_bootStorage.count;
}

int32_t lastReportedRssi()
{
  return SignalMonitor::isValid(g_bootStorage.lastRssi) ? g_bootStorage.lastRssi : 0;
}

void saveLastReportedRssi(int32_t rssi)
{
  g_bootStorage.lastRssi = rssi;
  updateStateChecksum();
}

} // namespace spff
