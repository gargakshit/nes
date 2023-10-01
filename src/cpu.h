#ifndef NES_CPU_H
#define NES_CPU_H

#include <cstdint>
#include <string>

namespace nes::cpu {
// 7  bit  0
// ---- ----
// NV1B DIZC
// |||| ||||
// |||| |||+- Carry
// |||| ||+-- Zero
// |||| |+--- Interrupt Disable
// |||| +---- Decimal
// |||+------ (No CPU effect; see: the B flag)
// ||+------- (No CPU effect; always pushed as 1)
// |+-------- Overflow
// +--------- Negative
// Reference: https://www.nesdev.org/wiki/Status_flags#Flags
struct Status {
  uint8_t C: 1;
  uint8_t Z: 1;
  uint8_t I: 1;
  uint8_t D: 1;
  uint8_t B: 1;
  uint8_t _: 1;
  uint8_t V: 1;
  uint8_t N: 1;
};

struct Registers {
  uint8_t a; // Accumulator.
  uint8_t x; // X register.
  uint8_t y; // Y register.
  uint8_t sp; // Stack pointer.
  uint16_t pc; // Program counter.
  union {
    uint8_t p;
    Status status;
  }; // Status register
};

class CPU : private Registers {
  void dump_reg() noexcept;
  void dump_state() noexcept;
  void sanity() noexcept;

public:
  CPU() noexcept;
  ~CPU() noexcept;

  // Reset the CPU.
  void rst() noexcept;
  // Simulate a clock tick. Maybe a NOP depending on the number of pending
  // cycles.
  void tick() noexcept;
  // Send an interrupt request. Maybe ignored based on status.I. It will drain
  // the pending cycles and will return the number of cycles drained.
  uint8_t irq() noexcept;
  // Send a non-maskable interrupt. Will never be ignored. It will drain the
  // pending cycles and will return the number of cycles drained.
  uint8_t nmi() noexcept;
};
} // namespace nes::cpu

#endif // NES_CPU_H
