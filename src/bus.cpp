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
      ppu(cart), controller_1(), apu() {
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

  case 0x4015: return apu.bus_read(address);

  case 0x4016 ... 0x4017: {
    auto val = (captured_controller_1 & 0x80) > 0;
    captured_controller_1 <<= 1;
    return val;
  }

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

  // Uhh, weird overlap with controller registers.
  case 0x4000 ... 0x4013:
  case 0x4015:
  case 0x4017: apu.bus_write(address, value); break;

  case 0x4014:
    logger->debug("Starting OAM DMA on page {:#04x}.", value);

    oam_page = value;
    oam_addr = 0x00;
    oam_dma = true;

    break;

  case 0x4016: captured_controller_1 = controller_1.state; break;
  default: logger->trace("Ignoring write to {:#06x}", address); break;
  }
}

void Bus::tick() noexcept {
  ppu.tick();

  // The CPU and APU runs 3x slower than the PPU.
  if (elapsed_cycles % 3 == 0) {
    apu.tick();
    if (!oam_dma) {
      cpu.tick();
    } else {
      // Waiting for DMA to sync. 513 / 514.
      if (dma_wait) {
        if (elapsed_cycles % 2 == 1) {
          dma_wait = false;
        }
      } else {
        if (elapsed_cycles % 2 == 0) {
          dma_data = read(((uint16_t)oam_page << 8) | ((uint16_t)oam_addr));
        } else {
          ppu.oam_memory[oam_addr] = dma_data;
          oam_addr++;

          if (oam_addr == 0) {
            dma_data = 0x00;
            dma_wait = true;
            oam_dma = false;
          }
        }
      }
    }
  }

  if (ppu.nmi) {
    ppu.nmi = false;
    cpu.nmi();
  }

  elapsed_cycles++;
}

Bus::~Bus() noexcept { logger->trace("Destructed the bus."); }
} // namespace nes::bus
