#ifndef NES_BUS_H
#define NES_BUS_H

#include <array>
#include <cstdint>
#include <memory>

#include "cart.h"
#include "controller.h"
#include "cpu.h"
#include "ppu.h"

namespace nes::bus {
using namespace nes;

class Bus {
  const static auto wram_size = 1 << 11;
  // Allocate 2kB (2048 bytes) of wram (working RAM, as nintendo calls it).
  std::array<uint8_t, wram_size> wram;

  bool oam_dma = false;
  bool dma_wait = true; // Wait for DMA to start.
  uint8_t oam_page = 0x00;
  uint8_t oam_addr = 0x00;
  uint8_t dma_data = 0x00;

public:
  std::shared_ptr<cart::Cart> cart;
  controller::StandardController controller_1;
  cpu::CPU cpu;
  ppu::PPU ppu;

  // System metrics.
  uint64_t elapsed_cycles = 0;
  uint8_t captured_controller_1 = 0;

  explicit Bus(const std::shared_ptr<cart::Cart> &cart) noexcept;
  ~Bus() noexcept;

  // Copy, move and default constructors.
  Bus() = delete;
  Bus(const Bus &) = delete;
  Bus(const Bus &&) = delete;

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
