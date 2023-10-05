#ifndef NES_BUS_H
#define NES_BUS_H

#include <array>
#include <cstdint>
#include <memory>

#include "cart.h"
#include "cpu.h"
#include "ppu.h"

namespace nes::bus {
using namespace nes;

class Bus {
  const static auto wram_size = 1 << 11;
  // Allocate 2kB (2048 bytes) of wram (working RAM, as nintendo calls it).
  std::array<uint8_t, wram_size> wram;

public:
  std::shared_ptr<cart::Cart> cart;
  cpu::CPU cpu;
  ppu::PPU ppu;

  // System metrics.
  uint64_t elapsed_cycles = 0;

  explicit Bus(std::shared_ptr<cart::Cart> cart) noexcept;
  ~Bus() noexcept;

  // Our "system bus" is essentially a combination of the CPU (main) bus and the
  // PPU bus.
  //
  // Bus also manages clocking each component, so essentially it is the whole
  // system.
  //
  // The bus owns its peripherals (yes the CPU is a peripheral to the bus).
  // Bus is the government, it moves the cart to itself.

  void write(uint16_t address, uint8_t value) noexcept;
  uint8_t read(uint16_t address) noexcept;

  void tick() noexcept;
};
} // namespace nes::bus

#endif // NES_BUS_H
