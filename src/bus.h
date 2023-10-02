#ifndef NES_BUS_H
#define NES_BUS_H

#include <array>
#include <cstdint>

#include "cpu.h"

namespace nes::bus {
using namespace nes;

class Bus {
  const static auto memory_size = 1 << 16;

  // For now, all the address space is mapped to memory.
  std::array<uint8_t, memory_size> memory;

public:
  cpu::CPU cpu;
  // System metrics.
  uint64_t elapsed_cycles = 0;

  Bus() noexcept;
  ~Bus() noexcept;

  void write(uint16_t address, uint8_t value) noexcept;
  uint8_t read(uint16_t address) noexcept;

  void tick() noexcept;
};
} // namespace nes::bus

#endif // NES_BUS_H
