#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "controller.h"

namespace nes::controller {
auto logger = spdlog::stderr_color_mt("nes::controller");

auto format_as(nes::controller::Button button) {
  switch (button) {
  case Button::A: return "A";
  case Button::B: return "B";
  case Button::Select: return "Select";
  case Button::Start: return "Start";
  case Button::Up: return "Up";
  case Button::Down: return "Down";
  case Button::Left: return "Left";
  case Button::Right: return "Right";
  }
}

void StandardController::set_key(const nes::controller::Button button,
                                 const bool pressed) noexcept {
  logger->debug("Button {} = {}", button, pressed);

  switch (button) {
  case Button::A: a = pressed; break;
  case Button::B: b = pressed; break;
  case Button::Select: select = pressed; break;
  case Button::Start: start = pressed; break;
  case Button::Up: up = pressed; break;
  case Button::Down: down = pressed; break;
  case Button::Left: left = pressed; break;
  case Button::Right: right = pressed; break;
  }
}
} // namespace nes::controller
