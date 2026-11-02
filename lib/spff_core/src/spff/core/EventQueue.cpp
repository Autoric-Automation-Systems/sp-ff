#include "spff/core/EventQueue.h"

#include <cstring>

namespace spff
{

EventQueue::EventQueue(QueueStorage &storage) : storage_(storage)
{
}

void EventQueue::restore()
{
  if (storage_.magic != QueueStorage::kValidMagic || storage_.count > QueueStorage::kCapacity)
  {
    storage_.count = 0;
    storage_.magic = QueueStorage::kValidMagic;
  }
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
  return kept;
}

void EventQueue::dropFront(size_t count)
{
  if (count >= storage_.count)
  {
    storage_.count = 0;
    return;
  }
  const size_t remaining = storage_.count - count;
  memmove(&storage_.slots[0], &storage_.slots[count], sizeof(Event) * remaining);
  storage_.count = static_cast<uint16_t>(remaining);
}

} // namespace spff
