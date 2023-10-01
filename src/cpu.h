#ifndef NES_CPU_H
#define NES_CPU_H

#include <cstdint>
#include <functional>
#include <string>

#include "opcode.h"

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

using ReadFunction = std::function<uint8_t (uint16_t)>;
using WriteFunction = std::function<void (uint16_t, uint8_t)>;

class CPU : private Registers {
  void dump_reg() noexcept;
  void dump_state() noexcept;
  void sanity() noexcept;

  ReadFunction read;
  WriteFunction write;

  // Number of cycles waiting to execute before we can execute the next opcode.
  int pending_cycles = 0;
  // Fetched value for the ALU.
  uint8_t fetched = 0;
  // Absolute address to jump to / fetch a value from.
  uint16_t addr_abs = 0;
  // Relative address to jump to.
  uint16_t addr_rel = 0;
  // Opcode that is being currently executed.
  uint8_t opcode = 0;

  void addressing_mode(op::AddressingMode mode) noexcept;

public:
  CPU(ReadFunction read, WriteFunction write) noexcept;
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
