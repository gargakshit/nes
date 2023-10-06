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
  uint8_t C : 1;
  uint8_t Z : 1;
  uint8_t I : 1;
  uint8_t D : 1;
  uint8_t B : 1;
  uint8_t _ : 1;
  uint8_t V : 1;
  uint8_t N : 1;
};

struct Registers {
  uint8_t a;   // Accumulator.
  uint8_t x;   // X register.
  uint8_t y;   // Y register.
  uint8_t sp;  // Stack pointer.
  uint16_t pc; // Program counter.
  union {
    uint8_t p;
    Status status;
  }; // Status register
};

using ReadFunction = std::function<uint8_t(uint16_t)>;
using WriteFunction = std::function<void(uint16_t, uint8_t)>;

class CPU : public Registers {
  void dump_reg() noexcept;
  void sanity() noexcept;

  ReadFunction read;
  WriteFunction write;
  // Fetched value for the ALU.
  uint8_t fetched = 0;
  // Absolute address to jump to / fetch a value from.
  uint16_t addr_abs = 0;
  // Relative address to jump to.
  uint16_t addr_rel = 0;
  const op::Opcode *decoded_opcode;

  // Compute addresses and stuff using the addressing mode.
  void addressing_mode(op::AddressingMode mode) noexcept;
  // Execute the actual instruction.
  void execute(op::Op op) noexcept;

  // Compute the overflow for an arithmetic operation. Must be called before
  // setting the accumulator.
  void flag_overflow(uint16_t result, uint16_t value) noexcept;
  // Compute the negative flag.
  void flag_negative(uint16_t result) noexcept;
  // Compute the zero flag.
  void flag_zero(uint16_t result) noexcept;
  // Compute the carry flag.
  void flag_carry(uint16_t result) noexcept;

  // Fetch the value from the addr_abs.
  void fetch() noexcept;

  // Branch using addr_rel.
  void branch() noexcept;

  // Performs an interrupt using the provided interrupt vector.
  void interrupt(uint16_t vector) noexcept;

  void push(uint8_t value) noexcept;
  void push_pc() noexcept;
  uint8_t pop() noexcept;
  void pop_pc() noexcept;

public:
  CPU(ReadFunction read, WriteFunction write) noexcept;
  ~CPU() noexcept;

  void dump_state() noexcept;

  // Number of cycles waiting to execute before we can execute the next opcode.
  int pending_cycles = 0;
  // Opcode that is being currently executed.
  uint8_t opcode = 0;

  // Reset the CPU.
  void rst() noexcept;
  // Simulate a clock tick. Maybe a NOP depending on the number of pending
  // cycles.
  void tick() noexcept;
  // Send an interrupt request.
  void irq() noexcept;
  // Send a non-maskable interrupt.
  void nmi() noexcept;
};
} // namespace nes::cpu

#endif // NES_CPU_H
