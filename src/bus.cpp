#include <functional>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "bus.h"

namespace nes::bus {
auto logger = spdlog::stderr_color_mt("nes::bus");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-avoid-bind"
Bus::Bus(const std::shared_ptr<cart::Cart> &cart) noexcept
    : wram(std::array<uint8_t, wram_size>()), cart(cart),
      cpu(cpu::CPU(std::bind(&Bus::read, this, std::placeholders::_1),
                   std::bind(&Bus::write, this, std::placeholders::_1,
                             std::placeholders::_2))),
      ppu(cart) {
  logger->trace("Creating a new bus.");
  logger->trace("Created a wram of size {0:#x} ({0}) bytes.", wram.size());
}
#pragma clang diagnostic pop

uint8_t Bus::read(uint16_t address) noexcept {
  logger->trace("Reading from address {:#06x}", address);

  uint8_t value;
  if (cart->bus_read(address, value)) {
    return value;
  }

  switch (address) {
  case 0x0000 ... 0x1fff: return wram[address & 0x07ff];
  case 0x2000 ... 0x3fff: return ppu.bus_read(address & 0x7);
  default: logger->trace("Ignoring read from {:#06x}", address); return 0;
  }
}

void Bus::write(uint16_t address, uint8_t value) noexcept {
  logger->trace("Writing to address {:#06x} = {:#04x}", address, value);
  if (cart->bus_write(address, value)) {
    return;
  }

  switch (address) {
  case 0x0000 ... 0x1fff: wram[address & 0x07ff] = value; break;
  case 0x2000 ... 0x3fff: ppu.bus_write(address & 0x7, value); break;
  default: logger->trace("Ignoring write to {:#06x}", address); break;
  }
}

void Bus::tick() noexcept {
  ppu.tick();

  // The CPU runs 3x slower than the PPU.
  if (elapsed_cycles % 3 == 0)
    cpu.tick();

  if (ppu.nmi) {
    ppu.nmi = false;
    cpu.nmi();
  }

  elapsed_cycles++;
}

Bus::~Bus() noexcept { logger->trace("Destructed the bus."); }
} // namespace nes::bus
