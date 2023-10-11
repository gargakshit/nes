#ifndef NES_PPU_H
#define NES_PPU_H

#include <array>
#include <memory>

#include "cart.h"

namespace nes::ppu {
using namespace nes;

union Control {
  struct {
    uint8_t nametable_x : 1;
    uint8_t nametable_y : 1;
    uint8_t vram_increment_mode : 1;
    uint8_t sprite_pattern_table : 1;
    uint8_t background_pattern_table : 1;
    uint8_t sprite_16x8_mode : 1;
    uint8_t slave_mode : 1;
    uint8_t nmi : 1;
  };
  uint8_t reg;
};

union Mask {
  struct {
    uint8_t grayscale : 1;
    uint8_t left_background : 1;
    uint8_t left_sprite : 1;
    uint8_t show_background : 1;
    uint8_t show_sprites : 1;
    uint8_t emp_red : 1;
    uint8_t emp_green : 1;
    uint8_t emp_blue : 1;
  };
  uint8_t reg;
};

union Status {
  struct {
    uint8_t _ : 5;
    uint8_t sprite_overflow : 1;
    uint8_t sprite_0_hit : 1;
    uint8_t vblank : 1;
  };
  uint8_t reg;
};

union InternalRendering {
  struct {
    uint16_t coarse_x : 5;
    uint16_t coarse_y : 5;
    uint16_t nametable_x : 1;
    uint16_t nametable_y : 1;
    uint16_t fine_y : 3;
    uint16_t _ : 1;
  };
  uint16_t reg;
};

struct Registers {
  Control control;
  Mask mask;
  Status status;
  InternalRendering v;
  InternalRendering t;
};

struct OAMEntry {
  uint8_t y;
  uint8_t id;
  uint8_t attribute;
  uint8_t x;
};

class PPU : public Registers {
  int16_t scanline = 0;
  int16_t cycle = 0;

  uint8_t data_buffer = 0;
  uint8_t address_latch = 0;
  uint8_t fine_x = 0; // Supposed to be 3 bits, but I am not making an unaligned
                      // bitfield here.

  std::array<std::array<uint8_t, 4096>, 2> pattern{};
  std::array<uint8_t, 32> palette_memory{};

  // 64 NES colors stored as RGB.
  constexpr const static std::array<uint32_t, 64> colors{
      0x626262FF, 0x001FB2FF, 0x2404C8FF, 0x5200B2FF, 0x730076FF, 0x800024FF,
      0x730B00FF, 0x522800FF, 0x244400FF, 0x005700FF, 0x005C00FF, 0x005324FF,
      0x003C76FF, 0x000000FF, 0x000000FF, 0x000000FF, 0xABABABFF, 0x0D57FFFF,
      0x4B30FFFF, 0x8A13FFFF, 0xBC08D6FF, 0xD21269FF, 0xC72E00FF, 0x9D5400FF,
      0x607B00FF, 0x209800FF, 0x00A300FF, 0x009942FF, 0x007DB4FF, 0x000000FF,
      0x000000FF, 0x000000FF, 0xFFFFFFFF, 0x53AEFFFF, 0x9085FFFF, 0xD365FFFF,
      0xFF57FFFF, 0xFF5DCFFF, 0xFF7757FF, 0xFA9E00FF, 0xBDC700FF, 0x7AE700FF,
      0x43F611FF, 0x26EF7EFF, 0x2CD5F6FF, 0x4E4E4EFF, 0x000000FF, 0x000000FF,
      0xFFFFFFFF, 0xB6E1FFFF, 0xCED1FFFF, 0xE9C3FFFF, 0xFFBCFFFF, 0xFFBDF4FF,
      0xFFC6C3FF, 0xFFD59AFF, 0xE9E681FF, 0xCEF481FF, 0xB6FB9AFF, 0xA9FAC3FF,
      0xA9F0F4FF, 0xB8B8B8FF, 0x000000FF, 0x000000FF};

  std::shared_ptr<cart::Cart> cart;

  [[nodiscard]] size_t get_color(size_t index, uint8_t pixel) const noexcept;

  std::array<std::array<uint32_t, 128 * 128>, 2> rendered_pattern_tables{};
  // 8 palettes, 4 colors, 3 channels. Use GL NEAREST_NEIGHBOUR to upscale. I
  // can't spare more CPU cycles on rendering debug information.
  std::array<uint32_t, 8 * 4> rendered_palettes{};

  // We will use the PPU frame timing diagram as a reference.
  // https://www.nesdev.org/w/images/default/4/4f/Ppu.svg
  // To accurately represent how a PPU renders, we will need some internal
  // state.

  uint8_t bg_next_tile_id = 0;
  uint8_t bg_next_attrib = 0;
  uint8_t bg_next_tile_lo = 0;
  uint8_t bg_next_tile_hi = 0;

  // See
  // https://www.nesdev.org/wiki/PPU_rendering#:~:text=contains%20the%20following%3A-,Background,-VRAM%20address%2C%20temporary

  uint16_t sr_bg_pattern_lo = 0;
  uint16_t sr_bg_pattern_hi = 0;
  uint16_t sr_bg_attrib_lo = 0;
  uint16_t sr_bg_attrib_hi = 0;

  std::array<uint8_t, 8> sr_sprite_pattern_lo;
  std::array<uint8_t, 8> sr_sprite_pattern_hi;

  std::array<OAMEntry, 64> oam{};
  std::array<OAMEntry, 8> secondary_oam{};
  uint8_t sprite_count = 0;

  bool sprite_0_hit_possible = false;
  bool sprite_0_hit_rendered = false;

public:
  const static auto screen_width = 256;
  const static auto screen_height = 240;

  std::array<std::array<uint8_t, 1024>, 2> nametables{};
  uint8_t *oam_memory = (uint8_t *)oam.data();

  bool nmi = false;

  // 256px width, 240px height.
  std::array<uint32_t, screen_width * screen_height> screen_1{};
  std::array<uint32_t, screen_width * screen_height> screen_2{};
  std::array<uint32_t, screen_width * screen_height> *active_buffer = &screen_1;
  std::array<uint32_t, screen_width * screen_height> *draw_buffer = &screen_2;

  bool swapped = false;
  bool frame_complete = false;

  explicit PPU(std::shared_ptr<cart::Cart> cart) noexcept;
  ~PPU() noexcept;

  void tick() noexcept;

  inline void swap() noexcept {
    swapped = !swapped;
    if (swapped) {
      active_buffer = &screen_2;
      draw_buffer = &screen_1;
    } else {
      active_buffer = &screen_1;
      draw_buffer = &screen_2;
    }
  }

  std::array<uint32_t, 128 * 128> pattern_table(uint8_t index) noexcept;
  std::array<uint32_t, 8 * 4> get_rendered_palettes() noexcept;

  void bus_write(uint16_t addr, uint8_t val) noexcept;
  uint8_t bus_read(uint16_t addr) noexcept;

  void ppu_write(uint16_t address, uint8_t value) noexcept;
  [[nodiscard]] uint8_t ppu_read(uint16_t address) const noexcept;
};
} // namespace nes::ppu

#endif // NES_PPU_H
