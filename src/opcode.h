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
  Indirect,  // 16-bit (LE) address stored in memory.
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

struct Opcode {
  const std::string mnemonic;
  const Op operation;
  const AddressingMode mode;
  // We set the base number of cycles required here, and then compute the
  // penalty during runtime.
  const int cycles;
  const bool unknown = false;
};

// Idea is to crash when we encounter an unknown opcode.
const auto unknown_op = Opcode{
    .mnemonic = "???",
    .operation = Op::NOP,
    .mode = AddressingMode::Implicit,
    .cycles = 2,
    .unknown = true,
};

// An alias for brevity.
using AM = AddressingMode;

const Opcode opcodes[256] = {
    {"BRK", Op::BRK, AM::Implicit, 7},
    {"ORA", Op::ORA, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    unknown_op,
    {"ORA", Op::ORA, AM::ZeroPage, 3},
    {"ASL", Op::ASL, AM::ZeroPage, 5},
    unknown_op,
    {"PHP", Op::PHP, AM::Implicit, 3},
    {"ORA", Op::ORA, AM::Immediate, 2},
    {"ASL", Op::ASL, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {"ORA", Op::ORA, AM::Absolute, 4},
    {"ASL", Op::ASL, AM::Absolute, 6},
    unknown_op,
    {"BPL", Op::BPL, AM::Relative, 2},
    {"ORA", Op::ORA, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {"ORA", Op::ORA, AM::ZeroPageX, 4},
    {"ASL", Op::ASL, AM::ZeroPageX, 6},
    unknown_op,
    {"CLC", Op::CLC, AM::Implicit, 2},
    {"ORA", Op::ORA, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {"ORA", Op::ORA, AM::AbsoluteX, 4},
    {"ASL", Op::ASL, AM::AbsoluteX, 7},
    unknown_op,
    {"JSR", Op::JSR, AM::Absolute, 6},
    {"AND", Op::AND, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {"BIT", Op::BIT, AM::ZeroPage, 3},
    {"AND", Op::AND, AM::ZeroPage, 3},
    {"ROL", Op::ROL, AM::ZeroPage, 5},
    unknown_op,
    {"PLP", Op::PLP, AM::Implicit, 4},
    {"AND", Op::AND, AM::Immediate, 2},
    {"ROL", Op::ROL, AM::Implicit, 2},
    unknown_op,
    {"BIT", Op::BIT, AM::Absolute, 4},
    {"AND", Op::AND, AM::Absolute, 4},
    {"ROL", Op::ROL, AM::Absolute, 6},
    unknown_op,
    {"BMI", Op::BMI, AM::Relative, 2},
    {"AND", Op::AND, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {"AND", Op::AND, AM::ZeroPageX, 4},
    {"ROL", Op::ROL, AM::ZeroPageX, 6},
    unknown_op,
    {"SEC", Op::SEC, AM::Implicit, 2},
    {"AND", Op::AND, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {"AND", Op::AND, AM::AbsoluteX, 4},
    {"ROL", Op::ROL, AM::AbsoluteX, 7},
    unknown_op,
    {"RTI", Op::RTI, AM::Implicit, 6},
    {"EOR", Op::EOR, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    unknown_op,
    {"EOR", Op::EOR, AM::ZeroPage, 3},
    {"LSR", Op::LSR, AM::ZeroPage, 5},
    unknown_op,
    {"PHA", Op::PHA, AM::Implicit, 3},
    {"EOR", Op::EOR, AM::Immediate, 2},
    {"LSR", Op::LSR, AM::Implicit, 2},
    unknown_op,
    {"JMP", Op::JMP, AM::Absolute, 3},
    {"EOR", Op::EOR, AM::Absolute, 4},
    {"LSR", Op::LSR, AM::Absolute, 6},
    unknown_op,
    {"BVC", Op::BVC, AM::Relative, 2},
    {"EOR", Op::EOR, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {"EOR", Op::EOR, AM::ZeroPageX, 4},
    {"LSR", Op::LSR, AM::ZeroPageX, 6},
    unknown_op,
    {"CLI", Op::CLI, AM::Implicit, 2},
    {"EOR", Op::EOR, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {"EOR", Op::EOR, AM::AbsoluteX, 4},
    {"LSR", Op::LSR, AM::AbsoluteX, 7},
    unknown_op,
    {"RTS", Op::RTS, AM::Implicit, 6},
    {"ADC", Op::ADC, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    unknown_op,
    {"ADC", Op::ADC, AM::ZeroPage, 3},
    {"ROR", Op::ROR, AM::ZeroPage, 5},
    unknown_op,
    {"PLA", Op::PLA, AM::Implicit, 4},
    {"ADC", Op::ADC, AM::Immediate, 2},
    {"ROR", Op::ROR, AM::Implicit, 2},
    unknown_op,
    {"JMP", Op::JMP, AM::Indirect, 5},
    {"ADC", Op::ADC, AM::Absolute, 4},
    {"ROR", Op::ROR, AM::Absolute, 6},
    unknown_op,
    {"BVS", Op::BVS, AM::Relative, 2},
    {"ADC", Op::ADC, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {"ADC", Op::ADC, AM::ZeroPageX, 4},
    {"ROR", Op::ROR, AM::ZeroPageX, 6},
    unknown_op,
    {"SEI", Op::SEI, AM::Implicit, 2},
    {"ADC", Op::ADC, AM::AbsoluteY, 4},
    unknown_op,
    unknown_op,
    unknown_op,
    {"ADC", Op::ADC, AM::AbsoluteX, 4},
    {"ROR", Op::ROR, AM::AbsoluteX, 7},
    unknown_op,
    unknown_op,
    {"STA", Op::STA, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {"STY", Op::STY, AM::ZeroPage, 3},
    {"STA", Op::STA, AM::ZeroPage, 3},
    {"STX", Op::STX, AM::ZeroPage, 3},
    unknown_op,
    {"DEY", Op::DEY, AM::Implicit, 2},
    unknown_op,
    {"TXA", Op::TXA, AM::Implicit, 2},
    unknown_op,
    {"STY", Op::STY, AM::Absolute, 4},
    {"STA", Op::STA, AM::Absolute, 4},
    {"STX", Op::STX, AM::Absolute, 4},
    unknown_op,
    {"BCC", Op::BCC, AM::Relative, 2},
    {"STA", Op::STA, AM::IndirectY, 6},
    unknown_op,
    unknown_op,
    {"STY", Op::STY, AM::ZeroPageX, 4},
    {"STA", Op::STA, AM::ZeroPageX, 4},
    {"STX", Op::STX, AM::ZeroPageY, 4},
    unknown_op,
    {"TYA", Op::TYA, AM::Implicit, 2},
    {"STA", Op::STA, AM::AbsoluteY, 5},
    {"TXS", Op::TXS, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {"STA", Op::STA, AM::AbsoluteX, 5},
    unknown_op,
    unknown_op,
    {"LDY", Op::LDY, AM::Immediate, 2},
    {"LDA", Op::LDA, AM::IndirectX, 6},
    {"LDX", Op::LDX, AM::Immediate, 2},
    unknown_op,
    {"LDY", Op::LDY, AM::ZeroPage, 3},
    {"LDA", Op::LDA, AM::ZeroPage, 3},
    {"LDX", Op::LDX, AM::ZeroPage, 3},
    unknown_op,
    {"TAY", Op::TAY, AM::Implicit, 2},
    {"LDA", Op::LDA, AM::Immediate, 2},
    {"TAX", Op::TAX, AM::Implicit, 2},
    unknown_op,
    {"LDY", Op::LDY, AM::Absolute, 4},
    {"LDA", Op::LDA, AM::Absolute, 4},
    {"LDX", Op::LDX, AM::Absolute, 4},
    unknown_op,
    {"BCS", Op::BCS, AM::Relative, 2},
    {"LDA", Op::LDA, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    {"LDY", Op::LDY, AM::ZeroPageX, 4},
    {"LDA", Op::LDA, AM::ZeroPageX, 4},
    {"LDX", Op::LDX, AM::ZeroPageY, 4},
    unknown_op,
    {"CLV", Op::CLV, AM::Implicit, 2},
    {"LDA", Op::LDA, AM::AbsoluteY, 4},
    {"TSX", Op::TSX, AM::Implicit, 2},
    unknown_op,
    {"LDY", Op::LDY, AM::AbsoluteX, 4},
    {"LDA", Op::LDA, AM::AbsoluteX, 4},
    {"LDX", Op::LDX, AM::AbsoluteY, 4},
    unknown_op,
    {"CPY", Op::CPY, AM::Immediate, 2},
    {"CMP", Op::CMP, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {"CPY", Op::CPY, AM::ZeroPage, 3},
    {"CMP", Op::CMP, AM::ZeroPage, 3},
    {"DEC", Op::DEC, AM::ZeroPage, 5},
    unknown_op,
    {"INY", Op::INY, AM::Implicit, 2},
    {"CMP", Op::CMP, AM::Immediate, 2},
    {"DEX", Op::DEX, AM::Implicit, 2},
    unknown_op,
    {"CPY", Op::CPY, AM::Absolute, 4},
    {"CMP", Op::CMP, AM::Absolute, 4},
    {"DEC", Op::DEC, AM::Absolute, 6},
    unknown_op,
    {"BNE", Op::BNE, AM::Relative, 2},
    {"CMP", Op::CMP, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {"CMP", Op::CMP, AM::ZeroPageX, 4},
    {"DEC", Op::DEC, AM::ZeroPageX, 6},
    unknown_op,
    {"CLD", Op::CLD, AM::Implicit, 2},
    {"CMP", Op::CMP, AM::AbsoluteY, 4},
    {"NOP", Op::NOP, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {"CMP", Op::CMP, AM::AbsoluteX, 4},
    {"DEC", Op::DEC, AM::AbsoluteX, 7},
    unknown_op,
    {"CPX", Op::CPX, AM::Immediate, 2},
    {"SBC", Op::SBC, AM::IndirectX, 6},
    unknown_op,
    unknown_op,
    {"CPX", Op::CPX, AM::ZeroPage, 3},
    {"SBC", Op::SBC, AM::ZeroPage, 3},
    {"INC", Op::INC, AM::ZeroPage, 5},
    unknown_op,
    {"INX", Op::INX, AM::Implicit, 2},
    {"SBC", Op::SBC, AM::Immediate, 2},
    {"NOP", Op::NOP, AM::Implicit, 2},
    unknown_op,
    {"CPX", Op::CPX, AM::Absolute, 4},
    {"SBC", Op::SBC, AM::Absolute, 4},
    {"INC", Op::INC, AM::Absolute, 6},
    unknown_op,
    {"BEQ", Op::BEQ, AM::Relative, 2},
    {"SBC", Op::SBC, AM::IndirectY, 5},
    unknown_op,
    unknown_op,
    unknown_op,
    {"SBC", Op::SBC, AM::ZeroPageX, 4},
    {"INC", Op::INC, AM::ZeroPageX, 6},
    unknown_op,
    {"SED", Op::SED, AM::Implicit, 2},
    {"SBC", Op::SBC, AM::AbsoluteY, 4},
    {"NOP", Op::NOP, AM::Implicit, 2},
    unknown_op,
    unknown_op,
    {"SBC", Op::SBC, AM::AbsoluteX, 4},
    {"INC", Op::INC, AM::AbsoluteX, 7},
    unknown_op,
};
} // namespace nes::cpu::op

template <> struct fmt::formatter<nes::cpu::op::Opcode> {
  static constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }

  static auto format(const nes::cpu::op::Opcode &op, format_context &ctx)
      -> format_context::iterator {
    return fmt::format_to(ctx.out(), "{} (cyc = {})", op.mnemonic, op.cycles);
  }
};

#endif // NES_SRC_OPCODE_H
