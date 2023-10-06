#include <spdlog/fmt/bin_to_hex.h>
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

  if (scanline == 241 && cycle == 1) {
    // We started the vblank.
    status.vblank = 1;
    if (control.nmi) {
      nmi = true;
    }
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

size_t PPU::get_palette_idx(size_t index, uint8_t pixel) {
  // Palettes have 4 entries. palette << 2 == palette * 4.
  return (ppu_read(0x3f00 + (index << 2) + pixel) & 0x3f) * 3;
}

std::array<uint8_t, 128 * 128 * 3> PPU::pattern_table(uint8_t index) noexcept {
  auto table = rendered_pattern_tables[index & 0x1];

  for (uint16_t tile_y = 0; tile_y < 16; tile_y++) {
    for (uint16_t tile_x = 0; tile_x < 16; tile_x++) {
      uint16_t tile_offset = (tile_y * 16 + tile_x) * 16;

      for (uint16_t row = 0; row < 8; row++) {
        uint16_t addr = (index * 0x1000) + tile_offset + row;

        uint8_t lo = ppu_read(addr);
        uint8_t hi = ppu_read(addr + 0x08);

        for (uint16_t col = 0; col < 8; col++) {
          uint8_t pixel = (lo & 0x1) + (hi & 0x1);

          lo >>= 1;
          hi >>= 1;

          auto pixel_idx =
              (((tile_y * 8 + row) * 128) + (tile_x * 8 + (7 - col))) * 3;

          const static auto palette = 4;
          auto color_idx = get_palette_idx(palette, pixel);

          table[pixel_idx + 0] = colors[color_idx + 0];
          table[pixel_idx + 1] = colors[color_idx + 1];
          table[pixel_idx + 2] = colors[color_idx + 2];
        }
      }
    }
  }

  return table;
}

std::array<uint8_t, 8 * 4 * 3> PPU::get_rendered_palettes() noexcept {
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t pixel = 0; pixel < 4; pixel++) {
      auto index = (i * 4 + pixel) * 3;
      auto color_idx = get_palette_idx(i, pixel);

      rendered_palettes[index + 0] = colors[color_idx + 0];
      rendered_palettes[index + 1] = colors[color_idx + 1];
      rendered_palettes[index + 2] = colors[color_idx + 2];
    }
  }

  return rendered_palettes;
}

uint8_t PPU::bus_read(uint16_t addr) noexcept {
  switch (addr) {
  case 0x00: break; // Control.
  case 0x01: break; // Mask.
  case 0x02: {
    status.vblank = 1;
    uint8_t data = (status.reg & 0xe0) | (data_buffer & 0x1f);
    // Reading the status clears vblank (???).
    status.vblank = 0;
    // And the address latch (???). Bruh.
    address_latch = 0;
    return data;
  } break;          // Status.
  case 0x03: break; // OAM address.
  case 0x04: break; // OAM data.
  case 0x05: break; // Scroll.
  case 0x06: break; // PPU address.
  case 0x07: {
    // Reads are delayed by one cycle (???).
    uint8_t data = data_buffer;
    //    data_buffer = ppu_read(active_rendering.reg);
    data_buffer = ppu_read(address);
    // Except when we read the palette.
    //    if (active_rendering.reg >= 0x3f00)
    //      data = data_buffer;
    if (address >= 0x3f00)
      data = data_buffer;
    // All reads increment the active address by 1 or 32 depending on the mode.
    active_rendering.reg += control.vram_increment_mode ? 32 : 1;
    address += 1;
    return data;
  } break; // PPU data.
  default: break;
  }

  return 0;
}

void PPU::bus_write(uint16_t addr, uint8_t val) noexcept {
  switch (addr) {
  case 0x00: {
    control.reg = val;
    temp_rendering.nametable_x = control.nametable_x;
    temp_rendering.nametable_y = control.nametable_y;
  } break;                          // Control.
  case 0x01: mask.reg = val; break; // Mask.
  case 0x02: break;                 // Status.
  case 0x03: break;                 // OAM address.
  case 0x04: break;                 // OAM data.
  case 0x05: {
  } break; // Scroll.
  case 0x06: {
    if (address_latch == 0) {
      address = (address & 0x00ff) | (((uint16_t)val) << 8);
      address_latch = 1;
    } else {
      address = (address & 0xff00) | val;
      address_latch = 0;
    }
  } break; // PPU address.
  case 0x07: {
    ppu_write(address, val);
    address += 1;
    // All reads increment the active address by 1 or 32 depending on the mode.
    active_rendering.reg += control.vram_increment_mode ? 32 : 1;
  } break; // PPU data.
  default: break;
  }
}

uint8_t PPU::ppu_read(uint16_t address) noexcept {
  uint8_t data = 0;
  auto addr = address & 0x3fff;

  if (cart->ppu_read(address, data))
    return data;

  switch (addr) {
  case 0x0000 ... 0x1fff: return 0;

  case 0x3f00 ... 0x3fff: {
    addr &= 0x001f;
    switch (addr) {
    case 0x0010: addr = 0x0000; break;
    case 0x0014: addr = 0x0004; break;
    case 0x0018: addr = 0x0008; break;
    case 0x001C: addr = 0x000C; break;
    default: break;
    }

    return palette_memory[addr];
  }

  default: return 0;
  }
}

void PPU::ppu_write(uint16_t address, uint8_t value) noexcept {
  auto addr = address & 0x3fff;

  if (cart->ppu_write(addr, value))
    return;

  switch (addr) {
  case 0x0000 ... 0x1fff: break;

  case 0x3f00 ... 0x3fff: {
    addr &= 0x001f;
    switch (addr) {
    case 0x0010: addr = 0x0000; break;
    case 0x0014: addr = 0x0004; break;
    case 0x0018: addr = 0x0008; break;
    case 0x001C: addr = 0x000C; break;
    default: break;
    }

    palette_memory[addr] = value;
  } break;

  default: break;
  }
}

PPU::~PPU() noexcept { logger->trace("Destructed the PPU."); }
} // namespace nes::ppu
