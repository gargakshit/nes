#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "apu.h"

namespace nes::apu {
auto logger = spdlog::stderr_color_mt("nes::apu");

static constexpr uint8_t duty_cycle_lut[][8] = {
    {0, 1, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 0, 0, 0},
    {1, 0, 0, 1, 1, 1, 1, 1},
};

// https://wiki.nesdev.com/w/index.php/APU_Length_Counter
static constexpr uint8_t len_counter_lut[] = {
    10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
    12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30,
};

// Pulse channel.
void Pulse::tick_sweep() noexcept {
  if (sweep_reset) {
    sweep_value = sweep_period + 1;
    sweep_reset = false;
  } else {
    if (sweep_value > 0)
      sweep_value--;
    else {
      sweep_value = sweep_period + 1;
      if (sweep_enabled) {
        uint16_t new_value = timer_value >> sweep_shift;
        uint16_t timer_period =
            (uint16_t)timer_low | ((uint16_t)timer_high << 8);

        if (!sweep_negate)
          timer_period += new_value;
        else if (!pulse_2)
          timer_period -= new_value + 1;
        else if (pulse_2)
          timer_period -= new_value;

        timer_low = timer_period & 0xff;
        timer_high = (timer_period & 0x700) >> 8;
      }
    }
  }
}

void Pulse::tick_timer() noexcept {
  if (timer_value > 0)
    timer_value--;
  else {
    timer_value = (uint16_t)timer_low | ((uint16_t)timer_high << 8);
    duty_value = (duty_value + 1) & 0x3;
  }
}

uint8_t Pulse::unmixed() noexcept {
  auto active = duty_cycle_lut[duty_cycle][duty_value];
  if (!enabled || !active || counter.value == 0 || timer_value < 8)
    return 0;

  return constant_volume ? envelope.period : envelope.value;
}

void Pulse::write(uint16_t addr, uint8_t value) noexcept {
  switch (addr & 0b11) {
  case 0: {
    r0 = value;
    envelope.start = envelope_loop;
    envelope.loop = envelope_loop;
    envelope.period = envelope_period;
    counter.enabled = !envelope_loop;
  } break;

  case 1: {
    r1 = value;
    sweep_reset = true;
  } break;

  case 2: r2 = value; break;

  case 3: {
    r3 = value;
    counter.value = len_counter_lut[length_counter];
    duty_value = 0;
    envelope.rst = true;
  } break;
  }
}

// APU.

APU::APU() noexcept : pulse1(false), pulse2(true), mixer() {}

void APU::bus_write(uint16_t addr, uint8_t value) noexcept {
  switch (addr) {
  // Pulse 1.
  case 0x4000 ... 0x4003: pulse1.write(addr - 0x4000, value); break;

  // Pulse 2.
  case 0x4004 ... 0x4007: pulse2.write(addr - 0x4004, value); break;

  case 0x4015: {
    auto reg = StatusReg{.val = value};
    pulse1.enabled = reg.pulse_1;
    pulse2.enabled = reg.pulse_2;
  } break;

  case 0x4017: frame_counter_reg = value; break;

  default: break;
  }
}

uint8_t APU::bus_read(uint16_t addr) noexcept {
  if (addr == 0x4015) {
    // TODO: return the status register.
    return 0;
  }

  return 0;
}

void APU::tick() noexcept {
  auto tick_timers = [&]() {
    // Most timers only tick on even cycles.
    if (ticks % 2 == 0) {
      pulse1.tick_timer();
      pulse2.tick_timer();
    }
  };

  auto tick_len_counters = [&]() {
    pulse1.counter.tick();
    pulse2.counter.tick();
  };

  auto tick_sweeps = [&]() {
    pulse1.tick_sweep();
    pulse2.tick_sweep();
  };

  auto tick_envelopes = [&]() {
    pulse1.envelope.tick();
    pulse2.envelope.tick();
  };

  auto tick_sequencer = [&]() {
    // https://www.nesdev.org/wiki/APU#Frame_Counter_($4017)

    if (five_step_mode) {
      // Five-step mode.
      switch (sequencer_ticks % 5) {
      case 0: tick_envelopes(); break;
      case 1:
        tick_envelopes();
        tick_len_counters();
        tick_sweeps();
        break;
      case 2: tick_envelopes(); break;
      case 3: break;
      case 4:
        tick_envelopes();
        tick_len_counters();
        tick_sweeps();
        break;
      }
    } else {
      // Four-step mode.
      switch (sequencer_ticks % 4) {
      case 0: tick_envelopes(); break;
      case 1:
        tick_envelopes();
        tick_len_counters();
        tick_sweeps();
        break;
      case 2: tick_envelopes(); break;
      case 3:
        tick_envelopes();
        tick_len_counters();
        tick_sweeps();
        // TODO: Send IRQ.
        break;
      }
    }

    sequencer_ticks++;
  };

  ticks++;
  tick_timers();

  // Tick the sequencer at 240 Hz.
  if (ticks % (clock_speed / 240) == 0)
    tick_sequencer();

  // Sample the output.
  if (ticks % (clock_speed / sample_rate) == 0) {
    auto sample = mixer.mix(pulse1.unmixed(), pulse2.unmixed(), 0, 0, 0);

    samples[sample_idx] = sample;
    sample_idx = (sample_idx + 1) % 512;
  }
}
} // namespace nes::apu
