#ifndef NES_GUI_H
#define NES_GUI_H

#include "bus.h"

namespace nes::gui {
using namespace nes;

class GUI {
  bus::Bus &bus;

  void render_cpu_state() noexcept;
  void render_system_metrics() noexcept;

public:
  GUI(bus::Bus &bus) noexcept;
  ~GUI() noexcept;

  void render() noexcept;
};
} // namespace nes::gui

#endif // NES_GUI_H
