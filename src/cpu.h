#ifndef NES_CPU_H
#define NES_CPU_H

#include <cstdint>

namespace nes::cpu {
enum struct AddressingMode {
  Implicit, // Implicitly a part of the instruction. Say BRK.
  Accumulator, // Operating on the accumulator.
  Immediate, // Immediate 8-bit value after the opcode.
  ZeroPage, // Immediate 8-bit value mapped to the zero page.
  Absolute, // Immediate 16-bit address after the opcode (LE).
  Relative, // Immediate 8-bit signed offset.
  Indirect, // 16-bit (LE) address stored in memory.
  ZeroPageX, // Zero Page + X (wraps).
  ZeroPageY, // Zero Page + Y (wraps).
  AbsoluteX, // Immediate 16-bit + X.
  AbsoluteY, // Immediate 16-bit + Y.
  IndirectX, // 16-bit address stored at (IMM16 + X).
  IndirectY, // (16-bit address stored at IMM16) + Y.
};

enum struct Operation {
  ADC, // Add with carry.
  AND, // Logical AND.
  ASL, // Arithmetic shift left.
  BCC, // Branch if carry clear.
  BCS, // Branch if carry set.
  BEQ, // Branch if equal (zero flag).
  BIT, // Bit test.
  BMI, // Branch if minus.
  BNQ, // Branch if not equal (zero flag).
  BPL, // Branch if positive.
  BRK, // Software IRQ.
  BVC, // Branch if overflow clear.
  BVS, // Branch if overflow set.
  CLC, // Clear carry flag.
  CLD, // Clear decimal mode.
  CLI, // Clear interrupt disable.
  CLV, // Clear overflow flag.
  CMP, // Compare.
  CPX, // Compare X register.
  CPY, // Compare Y register.
  DEC, // Decrement memory.
  DEX, // Decrement X register.
  DEY, // Decrement Y register.
  EOR, // Exclusive OR.
  INC, // Increment memory.
  INX, // Increment X register.
  INY, // Increment Y register.
  JMP, // Jump.
  JSR, // Jump to subroutine.
  LDA, // Load accumulator.
  LDX, // Load X register.
  LDY, // Load Y register.
  LSR, // Logical shift left.
  NOP, // No-op.
  ORA, // Logical inclusive OR.
  PHA, // Push accumulator.
  PHP, // Push status register.
  PLA, // Pull accumulator.
  PLP, // Pull status register.
  ROL, // Rotate left.
  ROR, // Rotate right.
  RTI, // Return from interrupt.
  RTS, // Return from subroutine.
  SBC, // Subtract with carry.
  SEC, // Set carry flag.
  SED, // Set decimal flag.
  SEI, // Disable IRQ.
  STA, // Store accumulator.
  STX, // Store X register.
  STY, // Store Y register.
  TAX, // Transfer accumulator to X register.
  TAY, // Transfer accumulator to Y register.
  TSX, // Transfer stack pointer to X register.
  TXA, // Transfer X register to accumulator.
  TXS, // Transfer X register to stack pointer.
  TYA, // Transfer Y register to accumulator.
};

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

  void rst() noexcept;
};
} // namespace nes::cpu

#endif // NES_CPU_H
