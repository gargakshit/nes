#ifndef NES_SRC_OPCODE_H
#define NES_SRC_OPCODE_H

#include <fmt/format.h>

namespace nes::cpu::op {
enum struct AddressingMode {
  Implicit,  // Implicitly a part of the instruction, or on the accumulator.
  Immediate, // Immediate 8-bit value after the opcode.
  ZeroPage,  // Immediate 8-bit value mapped to the zero page.
  Absolute,  // Immediate 16-bit address after the opcode (LE).
  Relative,  // Immediate 8-bit signed offset.
  Indirect,  // 16-bit (LE) address stored in wram.
  ZeroPageX, // Zero Page + X (wraps).
  ZeroPageY, // Zero Page + Y (wraps).
  AbsoluteX, // Immediate 16-bit + X.
  AbsoluteY, // Immediate 16-bit + Y.
  IndirectX, // 16-bit address stored at (IMM + X).
  IndirectY, // (16-bit address stored at IMM16) + Y.
};

enum struct Op {
  ADC, // Add with carry.
  AND, // Logical AND.
  ASL, // Arithmetic shift left.
  BCC, // Branch if carry clear.
  BCS, // Branch if carry set.
  BEQ, // Branch if equal (zero flag).
  BIT, // Bit test.
  BMI, // Branch if minus.
  BNE, // Branch if not equal (zero flag).
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
  DEC, // Decrement wram.
  DEX, // Decrement X register.
  DEY, // Decrement Y register.
  EOR, // Exclusive OR.
  INC, // Increment wram.
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

struct Opcode {
  const Op operation;
  const AddressingMode mode;
  // We set the base number of cycles required here, and then compute the
  // penalty during runtime.
  const int cycles;
  const bool page_penalty = true;
  const bool unknown = false;
};

// Idea is to crash when we encounter an unknown opcode.
const auto unknown_op = Opcode{
    .operation = Op::NOP,
    .mode = AddressingMode::Implicit,
    .cycles = 2,
    .page_penalty = false,
    .unknown = true,
};

// An alias for brevity.
using AM = AddressingMode;

const Opcode opcodes[256] = {
    {Op::BRK, AM::Implicit, 7},
    {Op::ORA, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::ORA, AM::ZeroPage, 3},
    {Op::ASL, AM::ZeroPage, 5},
    unknown_op,
    {Op::PHP, AM::Implicit, 3},
    {Op::ORA, AM::Immediate, 2},
    {Op::ASL, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {Op::ORA, AM::Absolute, 4},
    {Op::ASL, AM::Absolute, 6},
    unknown_op,
    {Op::BPL, AM::Relative, 2},
    {Op::ORA, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::ORA, AM::ZeroPageX, 4},
    {Op::ASL, AM::ZeroPageX, 6},
    unknown_op,
    {Op::CLC, AM::Implicit, 2},
    {Op::ORA, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::ORA, AM::AbsoluteX, 4},
    {Op::ASL, AM::AbsoluteX, 7, false},
    unknown_op,
    {Op::JSR, AM::Absolute, 6},
    {Op::AND, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {Op::BIT, AM::ZeroPage, 3},
    {Op::AND, AM::ZeroPage, 3},
    {Op::ROL, AM::ZeroPage, 5},
    unknown_op,
    {Op::PLP, AM::Implicit, 4},
    {Op::AND, AM::Immediate, 2},
    {Op::ROL, AM::Implicit, 2},
    unknown_op,
    {Op::BIT, AM::Absolute, 4},
    {Op::AND, AM::Absolute, 4},
    {Op::ROL, AM::Absolute, 6},
    unknown_op,
    {Op::BMI, AM::Relative, 2},
    {Op::AND, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::AND, AM::ZeroPageX, 4},
    {Op::ROL, AM::ZeroPageX, 6},
    unknown_op,
    {Op::SEC, AM::Implicit, 2},
    {Op::AND, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::AND, AM::AbsoluteX, 4},
    {Op::ROL, AM::AbsoluteX, 7, false},
    unknown_op,
    {Op::RTI, AM::Implicit, 6},
    {Op::EOR, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::EOR, AM::ZeroPage, 3},
    {Op::LSR, AM::ZeroPage, 5},
    unknown_op,
    {Op::PHA, AM::Implicit, 3},
    {Op::EOR, AM::Immediate, 2},
    {Op::LSR, AM::Implicit, 2},
    unknown_op,
    {Op::JMP, AM::Absolute, 3},
    {Op::EOR, AM::Absolute, 4},
    {Op::LSR, AM::Absolute, 6},
    unknown_op,
    {Op::BVC, AM::Relative, 2},
    {Op::EOR, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::EOR, AM::ZeroPageX, 4},
    {Op::LSR, AM::ZeroPageX, 6},
    unknown_op,
    {Op::CLI, AM::Implicit, 2},
    {Op::EOR, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::EOR, AM::AbsoluteX, 4},
    {Op::LSR, AM::AbsoluteX, 7, false},
    unknown_op,
    {Op::RTS, AM::Implicit, 6},
    {Op::ADC, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::ADC, AM::ZeroPage, 3},
    {Op::ROR, AM::ZeroPage, 5},
    unknown_op,
    {Op::PLA, AM::Implicit, 4},
    {Op::ADC, AM::Immediate, 2},
    {Op::ROR, AM::Implicit, 2},
    unknown_op,
    {Op::JMP, AM::Indirect, 5},
    {Op::ADC, AM::Absolute, 4},
    {Op::ROR, AM::Absolute, 6},
    unknown_op,
    {Op::BVS, AM::Relative, 2},
    {Op::ADC, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::ADC, AM::ZeroPageX, 4},
    {Op::ROR, AM::ZeroPageX, 6},
    unknown_op,
    {Op::SEI, AM::Implicit, 2},
    {Op::ADC, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::ADC, AM::AbsoluteX, 4},
    {Op::ROR, AM::AbsoluteX, 7, false},
    unknown_op,
    unknown_op,
    {Op::STA, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {Op::STY, AM::ZeroPage, 3},
    {Op::STA, AM::ZeroPage, 3},
    {Op::STX, AM::ZeroPage, 3},
    unknown_op,
    {Op::DEY, AM::Implicit, 2},
    unknown_op,
    {Op::TXA, AM::Implicit, 2},
    unknown_op,
    {Op::STY, AM::Absolute, 4},
    {Op::STA, AM::Absolute, 4},
    {Op::STX, AM::Absolute, 4},
    unknown_op,
    {Op::BCC, AM::Relative, 2},
    {Op::STA, AM::IndirectY, 6, false},
    unknown_op,
    unknown_op,
    {Op::STY, AM::ZeroPageX, 4},
    {Op::STA, AM::ZeroPageX, 4},
    {Op::STX, AM::ZeroPageY, 4},
    unknown_op,
    {Op::TYA, AM::Implicit, 2},
    {Op::STA, AM::AbsoluteY, 5, false},
    {Op::TXS, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {Op::STA, AM::AbsoluteX, 5, false},
    unknown_op,
    unknown_op,
    {Op::LDY, AM::Immediate, 2},
    {Op::LDA, AM::IndirectX, 6},
    {Op::LDX, AM::Immediate, 2},
    unknown_op,
    {Op::LDY, AM::ZeroPage, 3},
    {Op::LDA, AM::ZeroPage, 3},
    {Op::LDX, AM::ZeroPage, 3},
    unknown_op,
    {Op::TAY, AM::Implicit, 2},
    {Op::LDA, AM::Immediate, 2},
    {Op::TAX, AM::Implicit, 2},
    unknown_op,
    {Op::LDY, AM::Absolute, 4},
    {Op::LDA, AM::Absolute, 4},
    {Op::LDX, AM::Absolute, 4},
    unknown_op,
    {Op::BCS, AM::Relative, 2},
    {Op::LDA, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    {Op::LDY, AM::ZeroPageX, 4},
    {Op::LDA, AM::ZeroPageX, 4},
    {Op::LDX, AM::ZeroPageY, 4},
    unknown_op,
    {Op::CLV, AM::Implicit, 2},
    {Op::LDA, AM::AbsoluteY, 4},
    {Op::TSX, AM::Implicit, 2},
    unknown_op,
    {Op::LDY, AM::AbsoluteX, 4},
    {Op::LDA, AM::AbsoluteX, 4},
    {Op::LDX, AM::AbsoluteY, 4},
    unknown_op,
    {Op::CPY, AM::Immediate, 2},
    {Op::CMP, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {Op::CPY, AM::ZeroPage, 3},
    {Op::CMP, AM::ZeroPage, 3},
    {Op::DEC, AM::ZeroPage, 5},
    unknown_op,
    {Op::INY, AM::Implicit, 2},
    {Op::CMP, AM::Immediate, 2},
    {Op::DEX, AM::Implicit, 2},
    unknown_op,
    {Op::CPY, AM::Absolute, 4},
    {Op::CMP, AM::Absolute, 4},
    {Op::DEC, AM::Absolute, 6},
    unknown_op,
    {Op::BNE, AM::Relative, 2},
    {Op::CMP, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::CMP, AM::ZeroPageX, 4},
    {Op::DEC, AM::ZeroPageX, 6},
    unknown_op,
    {Op::CLD, AM::Implicit, 2},
    {Op::CMP, AM::AbsoluteY, 4},
    {Op::NOP, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {Op::CMP, AM::AbsoluteX, 4},
    {Op::DEC, AM::AbsoluteX, 7, false},
    unknown_op,
    {Op::CPX, AM::Immediate, 2},
    {Op::SBC, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {Op::CPX, AM::ZeroPage, 3},
    {Op::SBC, AM::ZeroPage, 3},
    {Op::INC, AM::ZeroPage, 5},
    unknown_op,
    {Op::INX, AM::Implicit, 2},
    {Op::SBC, AM::Immediate, 2},
    {Op::NOP, AM::Implicit, 2},
    unknown_op,
    {Op::CPX, AM::Absolute, 4},
    {Op::SBC, AM::Absolute, 4},
    {Op::INC, AM::Absolute, 6},
    unknown_op,
    {Op::BEQ, AM::Relative, 2},
    {Op::SBC, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {Op::SBC, AM::ZeroPageX, 4},
    {Op::INC, AM::ZeroPageX, 6},
    unknown_op,
    {Op::SED, AM::Implicit, 2},
    {Op::SBC, AM::AbsoluteY, 4},
    {Op::NOP, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {Op::SBC, AM::AbsoluteX, 4},
    {Op::INC, AM::AbsoluteX, 7, false},
    unknown_op,
};
} // namespace nes::cpu::op

template <> struct fmt::formatter<nes::cpu::op::Opcode> {
  static constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const nes::cpu::op::Opcode &op, format_context &ctx)
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "(op = {} am = {} cyc = {} un = {})",
                          fmt::underlying(op.operation),
                          fmt::underlying(op.mode), op.cycles, op.unknown);
  }
};

#endif // NES_SRC_OPCODE_H
