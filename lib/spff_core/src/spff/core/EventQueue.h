#pragma once

#include <cstddef>
#include <cstdint>

#include "spff/core/Event.h"

namespace spff
{

// Bloco de memória da fila. A instância real fica em RTC (ver PersistentStorage),
// por isso é POD e o tamanho é fixo em tempo de compilação.
struct QueueStorage
{
  static constexpr uint16_t kCapacity = 64;
  static constexpr uint32_t kValidMagic = 0x53504651UL; // "SPFQ"

  Event slots[kCapacity];
  uint16_t count;
  uint32_t magic;
  uint32_t checksum;
};

// Fila FIFO de capacidade fixa, sem alocação dinâmica.
class EventQueue
{
public:
  explicit EventQueue(QueueStorage &storage);

  // Valida magic, contagem, checksum e os nomes: memória RTC pode vir
  // indefinida (primeira energização) ou de outro firmware, e nada disso é
  // confiável sem checksum — bloco inconsistente é descartado.
  void restore();

  size_t size() const { return storage_.count; }
  bool empty() const { return storage_.count == 0; }
  const Event &at(size_t index) const { return storage_.slots[index]; }
  const Event *data() const { return storage_.slots; }

  // Retorna false quando a fila estava cheia: o evento mais antigo é descartado
  // para preservar as leituras recentes.
  bool push(const Event &event);

  void dropFront(size_t count);

  uint32_t dropped() const { return dropped_; }
  static constexpr size_t capacity() { return QueueStorage::kCapacity; }

private:
  bool isConsistent() const;
  uint32_t computeChecksum() const;
  void updateChecksum();

  QueueStorage &storage_;
  uint32_t dropped_ = 0;
};

} // namespace spff
