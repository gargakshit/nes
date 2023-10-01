#ifndef NES_BUS_H
#define NES_BUS_H

#include <array>
#include <cstdint>
#include <memory>

#include "cpu.h"

namespace nes::bus {
using namespace nes;

class Bus {
  const static auto memory_size = 1 << 16;

  // For now, all the address space is mapped to memory.
  std::unique_ptr<std::array<uint8_t, memory_size>> memory;
  std::unique_ptr<cpu::CPU> cpu;

public:
  Bus() noexcept;
  ~Bus() noexcept;
};
} // namespace nes::bus

#endif //NES_BUS_H
