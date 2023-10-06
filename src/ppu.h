#ifndef NES_PPU_H
#define NES_PPU_H

#include <array>
#include <memory>
#include <span>

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
    uint8_t sprite_size : 1;
    uint8_t slave_mode : 1;
    uint8_t nmi : 1;
  };
  uint8_t reg;
};

union Mask {
  struct {
    uint8_t grayscale : 1;
    uint8_t left_sprite : 1;
    uint8_t left_background : 1;
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
    uint8_t unused : 5;
    uint8_t sprite_overflow : 1;
    uint8_t sprite_0_hit : 1;
    uint8_t vblank : 1;
  };
  uint8_t reg;
};

union InternalRendering {
  struct {
    uint16_t coarse_x_scroll : 5;
    uint16_t coarse_y_scroll : 5;
    uint16_t nametable_x : 1;
    uint16_t nametable_y : 1;
    uint16_t fine_y_scroll : 3;
    uint16_t _ : 1;
  };
  uint16_t reg;
};

struct Registers {
  Control control;
  Mask mask;
  Status status;
  InternalRendering active_rendering;
  InternalRendering temp_rendering;
};

class PPU : public Registers {
  int16_t scanline = 0;
  int16_t cycle = 0;

  uint8_t data_buffer = 0;
  uint8_t address_latch = 0;
  uint8_t scroll_fine_x = 0;
  uint16_t address = 0;

  std::array<std::array<uint8_t, 1024>, 2> nametables{};
  std::array<std::array<uint8_t, 4096>, 2> pattern{};
  std::array<uint8_t, 32> palette_memory{};
  std::array<uint8_t, 256> oam_memory{};

  // 64 NES colors stored as RGB.
  std::array<uint8_t, 64 * 3> colors{
      84,  84,  84,  0,   30,  116, 8,   16,  144, 48,  0,   136, 68,  0,   100,
      92,  0,   48,  84,  4,   0,   60,  24,  0,   32,  42,  0,   8,   58,  0,
      0,   64,  0,   0,   60,  0,   0,   50,  60,  0,   0,   0,   0,   0,   0,
      0,   0,   0,   152, 150, 152, 8,   76,  196, 48,  50,  236, 92,  30,  228,
      136, 20,  176, 160, 20,  100, 152, 34,  32,  120, 60,  0,   84,  90,  0,
      40,  114, 0,   8,   124, 0,   0,   118, 40,  0,   102, 120, 0,   0,   0,
      0,   0,   0,   0,   0,   0,   236, 238, 236, 76,  154, 236, 120, 124, 236,
      176, 98,  236, 228, 84,  236, 236, 88,  180, 236, 106, 100, 212, 136, 32,
      160, 170, 0,   116, 196, 0,   76,  208, 32,  56,  204, 108, 56,  180, 204,
      60,  60,  60,  0,   0,   0,   0,   0,   0,   236, 238, 236, 168, 204, 236,
      188, 188, 236, 212, 178, 236, 236, 174, 236, 236, 174, 212, 236, 180, 176,
      228, 196, 144, 204, 210, 120, 180, 222, 120, 168, 226, 144, 152, 226, 180,
      160, 214, 228, 160, 162, 160, 0,   0,   0,   0,   0,   0};

  std::shared_ptr<cart::Cart> cart;

  size_t get_palette_idx(size_t index, uint8_t pixel);

  std::array<std::array<uint8_t, 128 * 128 * 3>, 2> rendered_pattern_tables{};
  // 8 palettes, 4 colors, 3 channels. Use GL NEAREST_NEIGHBOUR to upscale. I
  // can't spare more CPU cycles on rendering debug information.
  std::array<uint8_t, 8 * 4 * 3> rendered_palettes;

public:
  const static auto screen_width = 256;
  const static auto screen_height = 240;

  bool nmi = false;

  // 256px width, 240px height, 3 channels.
  std::array<uint8_t, screen_width * screen_height * 3> screen{};
  bool frame_complete = false;

  explicit PPU(std::shared_ptr<cart::Cart> cart) noexcept;
  ~PPU() noexcept;

  void tick() noexcept;

  std::array<uint8_t, 128 * 128 * 3> pattern_table(uint8_t index) noexcept;
  std::array<uint8_t, 8 * 4 * 3> get_rendered_palettes() noexcept;

  void bus_write(uint16_t addr, uint8_t val) noexcept;
  uint8_t bus_read(uint16_t addr) noexcept;

  void ppu_write(uint16_t address, uint8_t value) noexcept;
  uint8_t ppu_read(uint16_t address) noexcept;
};
} // namespace nes::ppu

#endif // NES_PPU_H
