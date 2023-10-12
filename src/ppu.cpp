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

  // https://www.nesdev.org/wiki/PPU_scrolling is your friend.
  // And bless C++ lambdas... for existing...

  auto increment_coarse_x = [&]() {
    if (!mask.show_background && !mask.show_sprites) {
      return;
    }

    if (v.coarse_x == 31) {
      // We overflowed. Time to reset.
      v.coarse_x = 0;
      // And switch nametables.
      v.nametable_x = ~v.nametable_x;
    } else {
      v.coarse_x++;
    }
  };

  auto increment_y = [&]() {
    if (!mask.show_background && !mask.show_sprites) {
      return;
    }

    if (v.fine_y < 7) {
      // We can get-by by incrementing the fine y value.
      v.fine_y++;
    } else {
      // Set fine_y to zero.
      v.fine_y = 0;

      if (v.coarse_y == 29) {
        // Remember we have 30 tiles (30 * 8 = 240 dots) in the y direction.
        v.coarse_y = 0;
        // Switching the vertical nametable.
        v.nametable_y = ~v.nametable_y;
      } else if (v.coarse_y == 31) {
        // How did the coarse_y overflow in the attribute memory?
        v.coarse_y = 0;
      } else {
        // Ugh increment.
        v.coarse_y++;
      }
    }
  };

  auto transfer_x_addr = [&]() {
    if (!mask.show_background && !mask.show_sprites) {
      return;
    }

    v.coarse_x = t.coarse_x;
    v.nametable_x = t.nametable_x;
  };

  auto transfer_y_addr = [&]() {
    if (!mask.show_background && !mask.show_sprites) {
      return;
    }

    v.fine_y = t.fine_y;
    v.coarse_y = t.coarse_y;
    v.nametable_y = t.nametable_y;
  };

  auto load_sr_bg = [&]() {
    sr_bg_pattern_lo = (sr_bg_pattern_lo & 0xff00) | bg_next_tile_lo;
    sr_bg_pattern_hi = (sr_bg_pattern_hi & 0xff00) | bg_next_tile_hi;

    // Ummm inflation. We just saved some computation here.
    sr_bg_attrib_lo =
        (sr_bg_attrib_lo & 0xff00) | ((bg_next_attrib & 0b01) ? 0xff : 0x00);
    sr_bg_attrib_hi =
        (sr_bg_attrib_hi & 0xff00) | ((bg_next_attrib & 0b10) ? 0xff : 0x00);
  };

  auto update_sr = [&]() {
    if (mask.show_background) {
      sr_bg_pattern_lo <<= 1;
      sr_bg_pattern_hi <<= 1;

      sr_bg_attrib_lo <<= 1;
      sr_bg_attrib_hi <<= 1;
    }

    if (mask.show_sprites && cycle >= 1 && cycle < 258) {
      for (auto i = 0; i < sprite_count; i++) {
        if (i >= 8)
          break;

        if (secondary_oam[i].x > 0)
          secondary_oam[i].x--;
        else {
          sr_sprite_pattern_lo[i] <<= 1;
          sr_sprite_pattern_hi[i] <<= 1;
        }
      }
    }
  };

  // Visible scanlines.

  if (scanline >= -1 && scanline < 240) {
    if (scanline == 0 && cycle == 0) {
      // Skip the "odd frame". This is essentially the same as time compression
      // without actually affecting the rendering. There is a reason we jump
      // back to cycle 0 and scanline -1.
      // Also RIP branch predictor.
      cycle = 1;
    }

    if (scanline == -1 && cycle == 1) {
      // New frame. I ain't in the vblank no more.
      status.vblank = 0;
      status.sprite_overflow = 0;
      status.sprite_0_hit = 0;

      sr_sprite_pattern_lo.fill(0);
      sr_sprite_pattern_hi.fill(0);
    }

    // Essentially skipping the HBLANK now.
    if ((cycle >= 2 && cycle < 258) || (cycle >= 321 && cycle < 338)) {
      update_sr();

      // Let's synchronize to the NES PPU timings.
      switch ((cycle - 1) & 0x07) {
      case 0: {
        load_sr_bg();
        // Cycle zero means we are in a new coarse x / y. To get the next tile
        // ID, we read from 0x2000 (nametable base) + (v.reg's first 12 bits).
        // v.reg's first 12 bits are (bless loopy)
        //  offset = (coarse_x <> coarse_y <> nametable_x <> nametable_y).
        // 0x2000 + offset. Remember we have nametable mirroring set-up. This
        // will give the whole tile ID. Pretty clever!
        bg_next_tile_id = ppu_read(0x2000 | (v.reg & 0x0fff));
        logger->trace("Reading next tile ID from {:#06x} = {:#04x}",
                      0x2000 | (v.reg & 0x0fff), bg_next_tile_id);
      } break;

      case 1: break;

      case 2: {
        // 0x03c0 is where the attribute memory starts in a nametable. So we
        // just get the nametable address from the "v" register, divide coarse
        // values by 4 (remember you can have 64 attributes) per nametable, and
        // a 4x4 tile grid will have the same attribute. This is some clever
        // bitwise trickery.
        auto address = 0x23c0 | (v.nametable_y << 11) | (v.nametable_x << 10) |
                       ((v.coarse_y >> 2) << 3) | (v.coarse_x >> 2);
        bg_next_attrib = ppu_read(address);

        logger->debug("Reading next tile attribute from {:#06x} = {:#04x}",
                      address, bg_next_attrib);

        // We read an 8-bit value. Remember we can have 4 active palettes for
        // the background. That means, each palette is 2-bit, and the attribute
        // is further split into 2x2 tile groups with the same palette. NES was
        // a good system.

        // Some more bitwise trickery.
        // We essentially need to swizzle the attribute into the correct order.
        // Tiles are 2D, and palette IDs are not in-order inside the attribute
        // memory.

        if (v.coarse_y & 0x02)
          bg_next_attrib >>= 4;
        if (v.coarse_x & 0x02)
          bg_next_attrib >>= 2;
        bg_next_attrib &= 0x03;
      } break;

      case 3: break;

      case 4: {
        // Fetch the lo-bit plane. This should be easy to understand, I guess.
        bg_next_tile_lo = ppu_read((control.background_pattern_table << 12) +
                                   ((uint16_t)bg_next_tile_id << 4) + v.fine_y);
      } break;

      case 5: break;

      case 6: {
        // Fetch the hi-bit plane.
        bg_next_tile_hi =
            ppu_read((control.background_pattern_table << 12) +
                     ((uint16_t)bg_next_tile_id << 4) + v.fine_y + 8);
      } break;

      case 7: increment_coarse_x(); break;
      }
    }

    // Increment the y on the end of a scanline.
    if (cycle == 256)
      increment_y();

    // Time to reset some stuff.
    if (cycle == 257) {
      load_sr_bg();
      transfer_x_addr();
    }

    // I love random hardware behaviours.
    if (cycle == 338 || cycle == 340)
      bg_next_tile_id = ppu_read(0x2000 | (v.reg & 0x0FFF));

    // New frame, get ready for the next frame.
    if (scanline == -1 && cycle >= 280 && cycle < 305)
      transfer_y_addr();

    // Foreground rendering.
    // May not be 100% accurate.

    if (scanline >= 0 && cycle == 257) {
      // Clear the secondary OAM with 0xff.
      secondary_oam.fill({0xff, 0xff, 0xff, 0xff});
      sr_sprite_pattern_lo.fill(0);
      sr_sprite_pattern_hi.fill(0);
      sprite_count = 0;
      sprite_0_hit_possible = false;

      // Evaluate sprites for the next scanline.

      // Iterate through OAM memory.
      for (auto i = 0; i < 64; i++) {
        auto oam_entry = oam[i];

        int sprite_y_diff = (int)scanline - (int)oam_entry.y;
        auto visible = sprite_y_diff >= 0 &&
                       sprite_y_diff < (control.sprite_16x8_mode ? 16 : 8);

        if (!visible)
          continue;

        if (sprite_count < 8) {
          if (i == 0)
            sprite_0_hit_possible = true;

          secondary_oam[sprite_count] = OAMEntry{
              .y = oam_entry.y,
              .id = oam_entry.id,
              .attribute = oam_entry.attribute,
              .x = oam_entry.x,
          };
          sprite_count++;
        }

        // Break as soon as we reach the 9th sprite.
        if (sprite_count >= 9)
          break;
      }

      status.sprite_overflow = sprite_count > 8;
    }

    if (cycle == 340) {
      for (auto i = 0; i < sprite_count; i++) {
        if (i >= 8)
          break;

        auto sprite = secondary_oam[i];
        auto flipped = sprite.attribute & 0x80;

        uint16_t addr_low = 0;

        if (control.sprite_16x8_mode == 0) {
          // 8x8.
          if (!flipped) {
            addr_low = (control.sprite_pattern_table << 12) | (sprite.id << 4) |
                       (scanline - sprite.y);
          } else {
            addr_low = (control.sprite_pattern_table << 12) | (sprite.id << 4) |
                       (7 - (scanline - sprite.y));
          }
        } else {
          // 16x8.
          if (!flipped) {
            if (scanline - sprite.y < 8) {
              // Top half.
              addr_low = ((sprite.id & 1) << 12) | ((sprite.id & 0xfe) << 4) |
                         ((scanline - sprite.y) & 0x7);
            } else {
              // Bottom half.
              addr_low = ((sprite.id & 1) << 12) |
                         (((sprite.id & 0xfe) + 1) << 4) |
                         ((scanline - sprite.y) & 0x7);
            }
          } else {
            if (scanline - sprite.y < 8) {
              // Top half.
              addr_low = ((sprite.id & 1) << 12) |
                         (((sprite.id & 0xfe) + 1) << 4) |
                         (7 - (scanline - sprite.y) & 0x7);
            } else {
              // Bottom half.
              addr_low = ((sprite.id & 1) << 12) | ((sprite.id & 0xfe) << 4) |
                         (7 - (scanline - sprite.y) & 0x7);
            }
          }
        }

        auto low_bitplane = ppu_read(addr_low);
        auto high_bitplane = ppu_read(addr_low + 8);

        // Flipped horizontally.
        if (sprite.attribute & 0x40) {
          auto flip_byte = [](uint8_t b) -> uint8_t {
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            return b;
          };

          low_bitplane = flip_byte(low_bitplane);
          high_bitplane = flip_byte(high_bitplane);
        }

        sr_sprite_pattern_lo[i] = low_bitplane;
        sr_sprite_pattern_hi[i] = high_bitplane;
      }
    }
  }

  if (scanline == 240) {
    // Scanline 240 is essentially a NOP (post-render line).
  }

  if (scanline == 241 && cycle == 1) {
    // We started the vblank.
    status.vblank = 1;
    if (control.nmi)
      nmi = true;
  }

  uint8_t bg_pixel = 0;   // The bg_pixel (2b).
  uint8_t bg_palette = 0; // Palette ID (3b).

  if (mask.show_background) {
    uint16_t mux = 0x8000 >> fine_x;

    uint8_t pixel_lo = (sr_bg_pattern_lo & mux) > 0;
    uint8_t pixel_hi = (sr_bg_pattern_hi & mux) > 0;
    bg_pixel = (pixel_hi << 1) | pixel_lo;

    uint8_t palette_lo = (sr_bg_attrib_lo & mux) > 0;
    uint8_t palette_hi = (sr_bg_attrib_hi & mux) > 0;
    bg_palette = (palette_hi << 1) | palette_lo;
  }

  uint8_t spr_pixel = 0;
  uint8_t spr_palette = 0;
  uint8_t spr_priority = 0;

  if (mask.show_sprites) {
    sprite_0_hit_rendered = false;

    for (auto i = 0; i < sprite_count; i++) {
      if (i >= 8)
        break;

      // We haven't really started rendering this sprite.
      if (secondary_oam[i].x != 0)
        continue;

      uint8_t pixel_lo = (sr_sprite_pattern_lo[i] & 0x80) > 0;
      uint8_t pixel_hi = (sr_sprite_pattern_hi[i] & 0x80) > 0;
      spr_pixel = (pixel_hi << 1) | pixel_lo;

      spr_palette = (secondary_oam[i].attribute & 0x03) + 0x04;
      spr_priority = (secondary_oam[i].attribute & 0x20) == 0;

      // Break if we found something to draw.
      if (spr_pixel != 0) {
        if (i == 0)
          sprite_0_hit_rendered = true;
        break;
      }
    }
  }

  uint8_t pixel = 0;
  uint8_t palette = 0;

  // Finally, get what pixel won.
  if (bg_pixel != 0 && spr_pixel == 0) {
    pixel = bg_pixel;
    palette = bg_palette;
  } else if (bg_pixel == 0 && spr_pixel != 0) {
    pixel = spr_pixel;
    palette = spr_palette;
  } else if (bg_pixel != 0 && spr_pixel != 0) {
    pixel = spr_priority ? spr_pixel : bg_pixel;
    palette = spr_priority ? spr_palette : bg_palette;

    if (sprite_0_hit_possible && sprite_0_hit_rendered && mask.show_sprites &&
        mask.show_background) {
      if (~(mask.left_sprite | mask.left_background) && cycle >= 9 &&
          cycle < 258)
        status.sprite_0_hit = 1;
      else if (cycle >= 1 && cycle < 258)
        status.sprite_0_hit = 1;
    }
  }

  // Time to draw.
  auto x = cycle - 1;
  auto y = scanline;

  // Yes.
  if (x >= 0 && x < 256 && y >= 0 && y < 240)
    // We will double-buffer;
    draw_buffer[y * screen_width + x] = get_color(palette, pixel);

  cycle++;
  if (cycle >= 341) {
    cycle = 0;
    scanline++;

    if (scanline >= 261) {
      scanline = -1;
      frame_complete = true;

      // Swap the active buffers;
      swap();
    }
  }
}

size_t PPU::get_color(size_t index, uint8_t pixel) const noexcept {
  // Palettes have 4 entries. palette << 2 == palette * 4.
  return colors[ppu_read(0x3f00 + (index << 2) + pixel) & 0x3f];
}

std::array<uint32_t, 128 * 128> PPU::pattern_table(uint8_t index) noexcept {
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
              ((tile_y * 8 + row) * 128) + (tile_x * 8 + (7 - col));

          const static auto palette = 4;
          table[pixel_idx] = get_color(palette, pixel);
        }
      }
    }
  }

  return table;
}

std::array<uint32_t, 8 * 4> PPU::get_rendered_palettes() noexcept {
  for (uint8_t i = 0; i < 8; i++) {
    for (uint8_t pixel = 0; pixel < 4; pixel++) {
      rendered_palettes[i * 4 + pixel] = get_color(i, pixel);
    }
  }

  return rendered_palettes;
}

uint8_t PPU::bus_read(uint16_t addr) noexcept {
  switch (addr) {
  case 0x00: break; // Control.
  case 0x01: break; // Mask.
  case 0x02: {
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
    data_buffer = ppu_read(v.reg);
    // Except when we read the palette.
    if (v.reg >= 0x3f00)
      data = data_buffer;
    // All reads increment the active address by 1 or 32 depending on the mode.
    v.reg += control.vram_increment_mode ? 32 : 1;
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
    t.nametable_x = control.nametable_x;
    t.nametable_y = control.nametable_y;
  } break;                          // Control.
  case 0x01: mask.reg = val; break; // Mask.
  case 0x02: break;                 // Status.
  case 0x03: break;                 // OAM address.
  case 0x04: break;                 // OAM data.

  case 0x05: {
    if (address_latch == 0) {
      fine_x = val & 0x07;
      t.coarse_x = val >> 3;
      address_latch = 1;
    } else {
      t.fine_y = val & 0x07;
      t.coarse_y = val >> 3;
      address_latch = 0;
    }
  } break; // Scroll.

  case 0x06: {
    if (address_latch == 0) {
      t.reg = (t.reg & 0x00ff) | (((uint16_t)val & 0x3f) << 8);
      address_latch = 1;
    } else {
      t.reg = (t.reg & 0xff00) | (uint16_t)val;
      v.reg = t.reg;
      address_latch = 0;
    }
  } break; // PPU address.

  case 0x07: {
    ppu_write(v.reg, val);
    // All reads increment the active address by 1 or 32 depending on the mode.
    v.reg += control.vram_increment_mode ? 32 : 1;
  } break; // PPU data.
  default: break;
  }
}

uint8_t PPU::ppu_read(uint16_t addr) const noexcept {
  uint8_t data = 0;
  addr &= 0x3fff;

  if (cart->ppu_read(addr, data))
    return data;

  switch (addr) {
  case 0x0000 ... 0x1fff: return pattern[(addr & 0x1000) >> 12][addr & 0x0fff];

  case 0x2000 ... 0x3eff:
    addr &= 0x0fff;

    if (cart->mirroring_mode == cart::MirroringMode::Vertical) {
      return nametables[(addr & (1 << 0xa)) >> 0xa][addr & 0x3ff];
    } else if (cart->mirroring_mode == cart::MirroringMode::Horizontal) {
      return nametables[(addr & (1 << 0xb)) >> 0xb][addr & 0x3ff];
    }

    return 0;

  case 0x3f00 ... 0x3fff: {
    addr &= 0x001f;
    switch (addr) {
    case 0x0010: addr = 0x0000; break;
    case 0x0014: addr = 0x0004; break;
    case 0x0018: addr = 0x0008; break;
    case 0x001C: addr = 0x000C; break;
    default: break;
    }

    return palette_memory[addr] & (mask.grayscale ? 0x30 : 0x3f);
  }

  default: return 0;
  }
}

void PPU::ppu_write(uint16_t addr, uint8_t value) noexcept {
  addr &= 0x3fff;

  if (cart->ppu_write(addr, value))
    return;

  switch (addr) {
  case 0x0000 ... 0x1fff:
    // Address is between 0x0000 and 0x1fff. To get "which patterntable to
    // address", we take the MSB. And then to get the index inside the
    // patterntable, we mask it with 0x0fff (the patterntable size).
    // Bless hex and bitwise.
    pattern[(addr & 0x1000) >> 12][addr & 0x0fff] = value;
    break;

  case 0x2000 ... 0x3eff:
    addr &= 0x0fff;

    if (cart->mirroring_mode == cart::MirroringMode::Vertical) {
      auto nametable_idx = (addr & (1 << 0xa)) >> 0xa;
      nametables[nametable_idx][addr & 0x3ff] = value;
    } else if (cart->mirroring_mode == cart::MirroringMode::Horizontal) {
      auto nametable_idx = (addr & (1 << 0xb)) >> 0xb;
      nametables[nametable_idx][addr & 0x3ff] = value;
    }

    break;

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
