#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "apu.h"

namespace nes::apu {
auto logger = spdlog::stderr_color_mt("nes::apu");

APU::APU() noexcept {
  // https://www.nesdev.org/wiki/APU_Mixer#Emulation.
  // We are going to do a LUT-based synthesis.

  for (auto i = 0; i < pulse_lut.size(); i++)
    pulse_lut[i] = 95.52f / (8128.f / (float)i + 100.f);

  for (auto i = 0; i < tnd_lut.size(); i++)
    tnd_lut[i] = 163.67f / (24329.f / (float)i + 100.f);
}
} // namespace nes::apu
