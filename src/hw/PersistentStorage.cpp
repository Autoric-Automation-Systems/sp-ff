#include "spff/hw/PersistentStorage.h"

#include <Arduino.h>

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
};

RTC_NOINIT_ATTR QueueStorage g_queueStorage;
RTC_NOINIT_ATTR BootStorage g_bootStorage;

} // namespace

QueueStorage &persistentQueueStorage()
{
  return g_queueStorage;
}

uint32_t nextBootCount()
{
  if (g_bootStorage.magic != kBootMagic || g_bootStorage.count > kMaxPlausibleBoots)
  {
    g_bootStorage.magic = kBootMagic;
    g_bootStorage.count = 0;
  }
  return ++g_bootStorage.count;
}

} // namespace spff
