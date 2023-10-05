#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "ppu.h"

namespace nes::ppu {
auto logger = spdlog::stderr_color_mt("nes::ppu");

PPU::PPU(std::shared_ptr<cart::Cart> cart) noexcept : cart(std::move(cart)) {
  logger->trace("Constructed the PPU.");
}

void PPU::tick() noexcept {
  logger->trace("Tick.");

  if (cycle < screen_width && scanline < screen_height && scanline >= 0) {
    auto screen_idx = ((scanline * screen_width) + cycle) * 3;
    auto color_idx = (rand() % 2 ? 0x3f : 0x30) * 3; // Don't ask me, its noise.

    screen[screen_idx + 0] = colors[color_idx + 0];
    screen[screen_idx + 1] = colors[color_idx + 1];
    screen[screen_idx + 2] = colors[color_idx + 2];
  }

  cycle++;
  if (cycle >= 341) {
    cycle = 0;
    scanline++;

    if (scanline >= 261) {
      scanline = -1;
      frame_complete = true;
    }
  }
}

uint8_t PPU::bus_read(uint16_t addr) noexcept { return 0; }

void PPU::bus_write(uint16_t addr, uint8_t val) noexcept {}

uint8_t PPU::ppu_read(uint16_t address) noexcept { return 0; }

void PPU::ppu_write(uint16_t address, uint8_t value) noexcept {}

PPU::~PPU() noexcept { logger->trace("Destructed the PPU."); }
} // namespace nes::ppu
