#ifndef NES_APU_H
#define NES_APU_H

#include <array>

namespace nes::apu {
struct LenCounter {
  bool enabled = false;
  uint8_t value = 0;

  inline void tick() noexcept {
    if (enabled && value > 0)
      value--;
  }
};

struct Envelope {
  bool start = false;
  uint8_t decay = 0;
  bool loop = false;

  // Divider.

  bool rst = false;
  uint8_t period = 0;
  uint8_t value = 0;

  inline void tick() noexcept {
    // https://www.nesdev.org/wiki/APU_Envelope.

    // if the start flag is clear, the divider is clocked, otherwise the start
    // flag is cleared, the decay level counter is loaded with 15, and the
    // divider's sweep_period is immediately reloaded.

    if (rst) {
      // Reset the envelope.

      rst = false;
      value = 15;
      period = decay;
    } else {
      // Clock the divider.

      if (period > 0)
        period--;
      else {
        // When the divider is clocked while at 0, it is loaded with V and
        // clocks the decay level counter. Then one of two actions occurs: If
        // the counter is non-zero, it is decremented, otherwise if the loop
        // flag is set, the decay level counter is loaded with 15.

        period = decay;
        if (value > 0)
          value--;
        else if (loop)
          value = 15;
      }
    }
  }
};

struct Pulse {
  // Registers.

  // Config.
  union {
    struct {
      uint8_t envelope_period : 4;
      uint8_t constant_volume : 1;
      uint8_t envelope_loop : 1;
      uint8_t duty_cycle : 2;
    };
    uint8_t r0;
  };

  // Sweep unit.
  union {
    struct {
      uint8_t sweep_shift : 3;
      uint8_t sweep_negate : 1;
      uint8_t sweep_period : 3;
      uint8_t sweep_enabled : 1;
    };
    uint8_t r1;
  };

  union {
    uint8_t timer_low;
    uint8_t r2;
  };

  union {
    struct {
      uint8_t timer_high : 3;
      uint8_t length_counter : 5;
    };
    uint8_t r3;
  };

  // State.
  Envelope envelope;
  LenCounter counter;

  bool sweep_reset = false;
  uint8_t sweep_value;
  uint8_t duty_value;
  uint16_t timer_value;
  bool enabled = false;

  bool pulse_2;

  Pulse(const Pulse &) = delete;
  Pulse(const Pulse &&) = delete;

  explicit Pulse(bool pulse_2) noexcept // NOLINT(*-pro-type-member-init)
      : r0(0), r1(0), r2(0), r3(0), envelope(), pulse_2(pulse_2) {}

  void tick_timer() noexcept;
  void tick_sweep() noexcept;
  uint8_t unmixed() noexcept;

  void write(uint16_t addr, uint8_t value) noexcept;
};

struct Mixer {
  std::array<float, 31> pulse_lut{};
  std::array<float, 203> tnd_lut{};

  Mixer() noexcept {
    // https://www.nesdev.org/wiki/APU_Mixer#Emulation.
    // We are going to do a LUT-based synthesis.

    for (auto i = 0; i < pulse_lut.size(); i++)
      pulse_lut[i] = 95.52f / (8128.f / (float)i + 100.f);

    for (auto i = 0; i < tnd_lut.size(); i++)
      tnd_lut[i] = 163.67f / (24329.f / (float)i + 100.f);
  }

  inline float mix(uint8_t pulse1, uint8_t pulse2, uint8_t triangle,
                   uint8_t noise, uint8_t dmc) noexcept {
    return pulse_lut[pulse1 + pulse2] + tnd_lut[3 * triangle + 2 * noise + dmc];
  }
};

union StatusReg {
  struct {
    uint8_t pulse_1 : 1;
    uint8_t pulse_2 : 1;
    uint8_t triangle : 1;
    uint8_t noise : 1;
    uint8_t dmc : 1;
    uint8_t _ : 3;
  };
  uint8_t val;
};

class APU {
  Pulse pulse1;
  Pulse pulse2;
  Mixer mixer;

  uint64_t ticks = 0;
  uint64_t sequencer_ticks = 0;

  union {
    struct {
      uint8_t _ : 6;
      // TODO: Implement IRQ.
      uint8_t inhibit_irq : 1;
      uint8_t five_step_mode : 1;
    };
    uint8_t frame_counter_reg;
  };

public:
  uint64_t clock_speed = 5369318 / 3;

  const static uint sample_rate = 44100;
  std::array<float, 512> samples;
  uint sample_idx = 0;

  APU() noexcept;
  ~APU() noexcept = default;

  void bus_write(uint16_t addr, uint8_t value) noexcept;
  uint8_t bus_read(uint16_t addr) noexcept;

  void tick() noexcept;
};
} // namespace nes::apu

#endif // NES_APU_H
