#include "spff/core/EventQueue.h"

#include <cstring>

#include "spff/core/Checksum.h"

namespace spff
{
namespace
{

// Nome de evento aceito: ASCII imprimível, não vazio e terminado dentro do slot.
bool isValidName(const char *name)
{
  for (size_t i = 0; i < kEventNameLength; i++)
  {
    const unsigned char character = static_cast<unsigned char>(name[i]);
    if (character == '\0')
    {
      return i > 0;
    }
    if (character < 0x20 || character > 0x7E)
    {
      return false;
    }
  }
  return false;
}

} // namespace

EventQueue::EventQueue(QueueStorage &storage) : storage_(storage)
{
}

void EventQueue::restore()
{
  if (isConsistent())
  {
    return;
  }

  storage_.count = 0;
  storage_.magic = QueueStorage::kValidMagic;
  updateChecksum();
}

bool EventQueue::isConsistent() const
{
  if (storage_.magic != QueueStorage::kValidMagic || storage_.count > QueueStorage::kCapacity)
  {
    return false;
  }
  if (storage_.checksum != computeChecksum())
  {
    return false;
  }

  for (uint16_t i = 0; i < storage_.count; i++)
  {
    if (!isValidName(storage_.slots[i].name))
    {
      return false;
    }
  }
  return true;
}

uint32_t EventQueue::computeChecksum() const
{
  const uint32_t hash = checksumOf(kChecksumSeed, &storage_.count, sizeof(storage_.count));
  return checksumOf(hash, storage_.slots, sizeof(Event) * storage_.count);
}

void EventQueue::updateChecksum()
{
  storage_.checksum = computeChecksum();
}

bool EventQueue::push(const Event &event)
{
  bool kept = true;
  if (storage_.count >= QueueStorage::kCapacity)
  {
    dropFront(1);
    dropped_++;
    kept = false;
  }

  storage_.slots[storage_.count++] = event;
  updateChecksum();
  return kept;
}

void EventQueue::dropFront(size_t count)
{
  if (count >= storage_.count)
  {
    storage_.count = 0;
    updateChecksum();
    return;
  }

  const size_t remaining = storage_.count - count;
  memmove(&storage_.slots[0], &storage_.slots[count], sizeof(Event) * remaining);
  storage_.count = static_cast<uint16_t>(remaining);
  updateChecksum();
}

} // namespace spff
