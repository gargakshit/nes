#ifndef NES_APU_H
#define NES_APU_H

#include <array>

namespace nes::apu {
class APU {
  std::array<float, 31> pulse_lut{};
  std::array<float, 203> tnd_lut{};

public:
  APU() noexcept;
  ~APU() noexcept;
};
} // namespace nes::apu

#endif // NES_APU_H
