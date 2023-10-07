#ifndef NES_CONTROLLER_H
#define NES_CONTROLLER_H

#include <cstdint>

namespace nes::controller {
enum class Button { A, B, Select, Start, Up, Down, Left, Right };

class StandardController {
public:
  union {
    struct {
      uint8_t right : 1;
      uint8_t left : 1;
      uint8_t down : 1;
      uint8_t up : 1;
      uint8_t start : 1;
      uint8_t select : 1;
      uint8_t b : 1;
      uint8_t a : 1;
    };

    uint8_t state;
  };

  StandardController() noexcept = default;
  void set_key(const Button button, const bool pressed) noexcept;
};
} // namespace nes::controller

#endif // NES_CONTROLLER_H
