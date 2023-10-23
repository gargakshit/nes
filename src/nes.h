#ifndef NES_NES_H
#define NES_NES_H

#include <memory>

#include <RtAudio.h>

#include "bus.h"
#include "cart.h"
#include "controller.h"
#include "gui.h"

namespace nes {
bool load(const char *file_path) noexcept;
void destroy() noexcept;

class System {
public:
  bus::Bus bus;
  gui::GUI gui;
  RtAudio audio;
  RtAudio::StreamParameters parameters;

public:
  bool error_init = false;

  explicit System(const std::shared_ptr<cart::Cart> &cart) noexcept;
  ~System() noexcept;

  System(const System &other) = delete;
  System(const System &&other) = delete;

  void tick(unsigned int n) noexcept;
  void render() noexcept;
  void set_key(const controller::Button key, const bool pressed) noexcept;
};
} // namespace nes

#endif // NES_NES_H
