#ifndef NES_GUI_H
#define NES_GUI_H

#include <chrono>

#include "bus.h"
#include "image.h"

namespace nes::gui {
using std::chrono::high_resolution_clock;
using time_point = std::chrono::high_resolution_clock::time_point;

using namespace nes;

class GUI {
  const static auto display_resolution_multiplier = 2;

  bus::Bus &bus;

  image::Image screen;
  image::Image pattern_table_left;
  image::Image pattern_table_right;
  image::Image rendered_palette;

  uint64_t elapsed_clocks_second = 0;
  uint64_t clocks_second_snapshot = 0;
  time_point last_clock_capture = high_resolution_clock::now();

  void render_cpu_state() noexcept;
  void render_ppu_state() noexcept;
  void render_system_metrics() noexcept;
  void render_screen() const noexcept;

public:
  GUI(bus::Bus &bus) noexcept;
  ~GUI() noexcept;

  void render() noexcept;
};
} // namespace nes::gui

#endif // NES_GUI_H
