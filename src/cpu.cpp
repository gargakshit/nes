#include <spdlog/sinks/stdout_color_sinks.h>

#include "config.h"
#include "cpu.h"
#include "utils.h"

#define SANITY_PANIC(msg)                                                      \
  do {                                                                         \
    dump_state();                                                              \
    PANIC(msg);                                                                \
  } while (false);

#define read16(addr)                                                           \
  (((uint16_t)read(addr)) | ((uint16_t)(read(addr + 1)) << 8))

namespace nes::cpu {
auto logger = spdlog::stderr_color_mt("nes::cpu");

CPU::CPU(ReadFunction read, WriteFunction write) noexcept
    : Registers(), read(std::move(read)), write(std::move(write)) {
  logger->trace("Constructing the CPU.");
  logger->trace("M6502 forever!");
  rst();
}

void CPU::rst() noexcept {
  logger->trace("Resetting the CPU.");

  a = 0;
  x = 0;
  y = 0;
  sp = 0xfd;
  pc = 0;
  p = 0;
  status._ = 1;

  const uint16_t reset_vec_location = 0xFFFC;
  pc = read16(reset_vec_location);

  dump_state();
  sanity();

  // Well reset took 8 cycles on actual hardware.
  pending_cycles = 8;
}

void CPU::dump_reg() noexcept {
  logger->debug("Register dump");
  logger->debug("  a  = {:#04x}", a);
  logger->debug("  x  = {:#04x}", x);
  logger->debug("  y  = {:#04x}", y);
  logger->debug("  sp = {:#04x}", sp);
  logger->debug("  pc = {:#06x}", pc);
  logger->debug("  p  = {:#010b} (0bNV1BDIZC)", p);
  logger->debug("    p.C = {:b}", static_cast<uint8_t>(status.C));
  logger->debug("    p.Z = {:b}", static_cast<uint8_t>(status.Z));
  logger->debug("    p.I = {:b}", static_cast<uint8_t>(status.I));
  logger->debug("    p.D = {:b}", static_cast<uint8_t>(status.D));
  logger->debug("    p.B = {:b}", static_cast<uint8_t>(status.B));
  logger->debug("    p.1 = {:b}", static_cast<uint8_t>(status._));
  logger->debug("    p.V = {:b}", static_cast<uint8_t>(status.V));
  logger->debug("    p.N = {:b}", static_cast<uint8_t>(status.N));
}

void CPU::sanity() noexcept {
#ifdef NES_CPU_RT_SANITY
  if ((p & 0b00100000) != 0b00100000) {
    SANITY_PANIC("Sanity: always-set status bit is not set.");
  }

  if (status._ != 1) {
    SANITY_PANIC("Sanity: always-set status bit is not set, invalid status "
                 "bitfield pack.");
  }
#endif
}

void CPU::dump_state() noexcept { dump_reg(); }

void CPU::addressing_mode(op::AddressingMode mode) noexcept {
  switch (mode) {
  case op::AddressingMode::Implicit: {
    logger->trace("Addressing mode: Implicit");
    // We use the accumulator.
    fetched = a;
  } break;

  case op::AddressingMode::Immediate: {
    logger->trace("Addressing mode: Immediate");
    // Set the jump address to PC.
    addr_abs = pc;
    // Increment the program counter;
    pc++;
  } break;

  case op::AddressingMode::ZeroPage: {
    logger->trace("Addressing mode: ZeroPage");
    // Set the jump address to the value read from PC, and wrap it to 256.
    addr_abs = read(pc) & 0xff;
    // Increment the program counter;
    pc++;
  } break;

  case op::AddressingMode::Absolute: {
    logger->trace("Addressing mode: Absolute");
    // Yeah, little endian.
    addr_abs = read16(pc);
    // We read 2 bytes.
    pc += 2;
  } break;

  case op::AddressingMode::Relative: {
    logger->trace("Addressing mode: Relative");
    addr_rel = read(pc);
    // In case we have a negative offset, set the first 8 bits.
    if (addr_rel & 0x80)
      addr_rel |= 0xff00;

    // No need to set the value here as relative is only used for jumps.
    // Increment the PC again.
    pc++;
  } break;

  case op::AddressingMode::Indirect: {
    logger->trace("Addressing mode: Indirect");
    // Read the 16-bit pointer.
    uint16_t ptr = read16(pc);
    pc += 2;

    // Now read the actual address to jump to.

    // Hardware bug: we read the wrong address if we are on the page boundary.
    if (ptr & 0xff)
      addr_abs = ((uint16_t)read(ptr)) | (((uint16_t)read(ptr & 0xff00)) << 8);
    else
      addr_abs = read16(ptr);
  } break;

  case op::AddressingMode::ZeroPageX: {
    logger->trace("Addressing mode: ZeroPageX");
    // Zero page + x, wrapped to 256.
    addr_abs = ((uint16_t)read(pc) + x) & 0xff;
    // Increment the PC as usual.
    pc++;
  } break;

  case op::AddressingMode::ZeroPageY: {
    logger->trace("Addressing mode: ZeroPageY");
    // Zero page + y, wrapped to 256.
    addr_abs = ((uint16_t)read(pc) + y) & 0xff;
    // Increment the PC as usual.
    pc++;
  } break;

  case op::AddressingMode::AbsoluteX: {
    logger->trace("Addressing mode: AbsoluteX");
    // Usual drill.
    uint16_t abs = read16(pc);
    pc += 2;

    // ABS + X.
    addr_abs = abs + x;

    // Check if we crossed the page boundary.
    if ((addr_abs & 0xff00) != (abs & 0xff00))
      pending_cycles++;
  } break;

  case op::AddressingMode::AbsoluteY: {
    logger->trace("Addressing mode: AbsoluteY");
    // Usual drill.
    uint16_t abs = read16(pc);
    pc += 2;

    // ABS + Y.
    addr_abs = abs + y;

    // Check if we crossed the page boundary.
    if ((addr_abs & 0xff00) != (abs & 0xff00))
      pending_cycles++;
  } break;

  case op::AddressingMode::IndirectX: {
    logger->trace("Addressing mode: IndirectX");
    // Read the immediate value.
    uint16_t addr = read(pc);
    pc++;

    // We won't use read16 as we need to wrap the addresses.
    uint16_t lo = read((addr + (uint16_t)x) & 0xff);
    uint16_t hi = read((addr + (uint16_t)x + 1) & 0xff);

    addr_abs = (hi << 8) | lo;
  } break;

  case op::AddressingMode::IndirectY: {
    logger->trace("Addressing mode: IndirectY");
    // Read the immediate value.
    uint16_t addr = read(pc);
    pc++;

    // We won't use read16 as we need to wrap the addresses.
    uint16_t lo = read((addr)&0xff);
    uint16_t hi = read((addr + 1) & 0xff);

    addr_abs = ((hi << 8) | lo) + y;

    // Check if we crossed the page boundary.
    if (addr_abs >> 8 != hi)
      pending_cycles++;
  } break;
  }
}

void CPU::flag_overflow(uint16_t result, uint16_t value) noexcept {
  // Don't ask...
  status.V =
      (~((uint16_t)a ^ (uint16_t)value) & ((uint16_t)a ^ (uint16_t)result)) &
      0x80;
}

void CPU::flag_negative(uint16_t result) noexcept { status.N = result & 0x80; }

void CPU::flag_zero(uint16_t result) noexcept {
  status.Z = (result & 0xff) == 0;
}

void CPU::flag_carry(uint16_t result) noexcept { status.C = result > 0xff; }

void CPU::fetch() noexcept {
  if (decoded_opcode->mode != op::AddressingMode::Implicit)
    fetched = read(addr_abs);
}

void CPU::execute(op::Op op) noexcept {
  switch (op) {
  case op::Op::ADC: {
    fetch();

    // A + fetched + carry.
    uint16_t result = (uint16_t)a + (uint16_t)fetched + (uint16_t)status.C;

    // Set the flags.
    flag_overflow(result, fetched);
    flag_negative(result);
    flag_zero(result);
    flag_carry(result);

    // Put the result in the accumulator.
    a = result & 0xff;

    // TODO(AG): Check if we need an extra cycle here.
  } break;

  case op::Op::SBC: {
    fetch();

    // Bless binary. Inverting the digits makes this same as ADC.
    uint16_t value = fetched ^ 0xff;

    // A + fetched + carry.
    uint16_t result = (uint16_t)a + (uint16_t)value + (uint16_t)status.C;

    // Set the flags.
    flag_overflow(result, value);
    flag_negative(result);
    flag_zero(result);
    flag_carry(result);

    // Put the result in the accumulator.
    a = result & 0xff;

    // TODO(AG): Check if we need an extra cycle here.
  } break;

  case op::Op::AND: {
    fetch();
    a &= fetched;

    flag_zero(a);
    flag_negative(a);

    // TODO(AG): Check if we need an extra cycle here.
  } break;

  case op::Op::ASL: {
    fetch();
    uint16_t result = ((uint16_t)fetched) << 1;

    flag_carry(result);
    flag_negative(result);
    flag_zero(result);

    if (decoded_opcode->mode == op::AddressingMode::Implicit)
      a = result & 0xff;
    else
      write(addr_abs, result & 0xff);
  } break;

  case op::Op::BCC: {
    if (status.C == 0)
      branch();
  } break;

  case op::Op::BCS: {
    if (status.C != 0)
      branch();
  } break;

  case op::Op::BEQ: {
    if (status.Z != 0)
      branch();
  } break;

  case op::Op::BIT: {
    fetch();
    uint16_t result = ((uint16_t)a) & ((uint16_t)fetched);

    flag_zero(result);
    status.N = (fetched & (1 << 7)) == 0;
    status.V = (fetched & (1 << 6)) == 0;
  } break;

  case op::Op::BMI: {
    if (status.N != 0)
      branch();
  } break;

  case op::Op::BNE: {
    if (status.Z == 0)
      branch();
  } break;

  case op::Op::BPL: {
    if (status.N == 0)
      branch();
  } break;

  case op::Op::BRK: {
    interrupt(0xfffe);
  } break;

  case op::Op::BVC: {
    if (status.V == 0)
      branch();
  } break;

  case op::Op::BVS: {
    if (status.V != 0)
      branch();
  } break;

  case op::Op::CLC: {
    status.C = 0;
  } break;

  case op::Op::CLD: {
    status.D = 0;
  } break;

  case op::Op::CLI: {
    status.I = 0;
  } break;

  case op::Op::CLV: {
    status.V = 0;
  } break;

  case op::Op::CMP: {
    fetch();

    uint16_t result = (uint16_t)a - (uint16_t)fetched;

    status.C = a >= fetched;
    flag_zero(result);
    flag_negative(result);
  } break;

  case op::Op::CPX: {
    fetch();

    uint16_t result = (uint16_t)x - (uint16_t)fetched;

    status.C = x >= fetched;
    flag_zero(result);
    flag_negative(result);
  } break;

  case op::Op::CPY: {
    fetch();

    uint16_t result = (uint16_t)y - (uint16_t)fetched;

    status.C = y >= fetched;
    flag_zero(result);
    flag_negative(result);
  } break;

  case op::Op::DEC: {
    fetch();

    uint16_t result = fetched - 1;
    write(addr_abs, result & 0xff);

    flag_zero(result);
    flag_negative(result);
  } break;

  case op::Op::DEX: {
    x--;
    flag_zero(x);
    flag_negative(x);
  } break;

  case op::Op::DEY: {
    y--;
    flag_zero(y);
    flag_negative(y);
  } break;

  case op::Op::EOR: {
    fetch();
    a ^= fetched;

    flag_zero(a);
    flag_negative(a);

    // TODO(AG): Check for cycle penalty here.
  } break;

  case op::Op::INC: {
    fetch();

    uint16_t result = fetched + 1;
    write(addr_abs, result & 0xff);

    flag_zero(result);
    flag_negative(result);
  } break;

  case op::Op::INX: {
    x++;
    flag_zero(x);
    flag_negative(x);
  } break;

  case op::Op::INY: {
    y++;
    flag_zero(y);
    flag_negative(y);
  } break;

  case op::Op::JMP: {
    pc = addr_abs;
  } break;

  case op::Op::JSR: {
    // We push the current PC due to how 6502 works.
    pc--;
    push_pc();
    pc = addr_abs;
  } break;

  case op::Op::LDA: {
    fetch();
    a = fetched;
    flag_zero(a);
    flag_negative(a);
  } break;

  case op::Op::LDX: {
    fetch();
    x = fetched;
    flag_zero(x);
    flag_negative(x);
  } break;

  case op::Op::LDY: {
    fetch();
    y = fetched;
    flag_zero(y);
    flag_negative(y);
  } break;

  case op::Op::NOP: {
    // TODO(AG): Add aliased NOPs (unknown instructions).
  } break;

  case op::Op::ORA: {
    fetch();
    a |= fetched;
    flag_zero(a);
    flag_negative(a);
  } break;

  case op::Op::PHA: {
    push(a);
  } break;

  case op::Op::PHP: {
    uint8_t old_flags = p;

    status._ = 1;
    status.B = 1;
    push(p);

    p = old_flags;
  } break;

  case op::Op::PLA: {
    a = pop();
    flag_zero(a);
    flag_negative(a);
  } break;

  case op::Op::PLP: {
    p = pop();
    status._ = 1;
  } break;

  case op::Op::ROL: {
    fetch();
    uint16_t result = (((uint16_t)fetched) << 1) | status.C;

    flag_zero(result);
    flag_negative(result);
    flag_carry(result);

    if (decoded_opcode->mode == op::AddressingMode::Implicit)
      a = result & 0xff;
    else
      write(addr_abs, result & 0xff);
  } break;

  case op::Op::ROR: {
    fetch();
    uint16_t result = (((uint16_t)fetched) >> 1) | (((uint16_t)status.C) << 7);

    flag_zero(result);
    flag_negative(result);
    flag_carry(result);

    if (decoded_opcode->mode == op::AddressingMode::Implicit)
      a = result & 0xff;
    else
      write(addr_abs, result & 0xff);
  } break;

  case op::Op::RTI: {
    p = pop();
    status._ = 1;
    status.B = 1;
    pop_pc();
  } break;

  case op::Op::RTS: {
    pop_pc();
    pc++;
  } break;

  case op::Op::SEC: {
    status.C = 1;
  } break;

  case op::Op::SED: {
    status.D = 1;
  } break;

  case op::Op::SEI: {
    status.I = 1;
  } break;

  case op::Op::STA: {
    write(addr_abs, a);
  } break;

  case op::Op::STX: {
    write(addr_abs, x);
  } break;

  case op::Op::STY: {
    write(addr_abs, y);
  } break;

  case op::Op::TAX: {
    x = a;
    flag_zero(x);
    flag_negative(x);
  } break;

  case op::Op::TAY: {
    y = a;
    flag_zero(y);
    flag_negative(y);
  } break;

  case op::Op::TSX: {
    x = sp;
    flag_zero(x);
    flag_negative(x);
  } break;

  case op::Op::TXA: {
    a = x;
    flag_zero(a);
    flag_negative(a);
  } break;

  case op::Op::TXS: {
    sp = x;
  } break;

  case op::Op::TYA: {
    a = y;
    flag_zero(a);
    flag_negative(a);
  } break;

  default: {
    PANIC("Unimplemented instruction");
  } break;
  }
}

void CPU::branch() noexcept { // Branch!
  pending_cycles++;

  // Branches are relative.
  addr_abs = pc + addr_rel;

  // Check if we have a page cross penalty.
  if ((addr_abs & 0xff00) != (pc & 0xff00))
    pending_cycles++;

  pc = addr_abs;
}

void CPU::interrupt(uint16_t vector) noexcept {
  logger->trace("Performing CPU interrupt with the vector {:#06x}", vector);

  status.I = 1;
  push_pc();

  // NOTE(AG): Decimal mode shenanigans on the NES 6502.
  //           Should change this while porting this CPU to other platforms.
  status.B = 1;
  push(p);
  status.B = 0;

  pc = read16(vector);
  dump_reg();
}

void CPU::push(uint8_t value) noexcept {
  logger->trace("Pushing {:#04x} on stack pos {:#04x}", value, sp);

  write(0x0100 + sp, value);
  sp--;
}

void CPU::push_pc() noexcept {
  push((pc >> 8) & 0xff);
  push(pc & 0xff);
}

uint8_t CPU::pop() noexcept {
  sp++;
  uint8_t value = read(0x0100 + sp);

  logger->trace("Popped {:#04x} from stack pos {:#04x}", value, sp);
  return value;
}

void CPU::pop_pc() noexcept {
  auto lo = pop();
  auto hi = pop();

  pc = ((uint16_t)lo) | (((uint16_t)hi) << 8);
}

void CPU::tick() noexcept {
  logger->trace("Tick.");

  if (pending_cycles > 0) {
    pending_cycles--;
    return;
  }

  // Perform a sanity check.
  sanity();

  // Read the opcode and increment the PC by 1.
  opcode = read(pc);
  pc++;

  decoded_opcode = &op::opcodes[opcode];
  logger->trace("Executing opcode {}", *decoded_opcode);

  if (decoded_opcode->unknown) {
    PANIC("Unknown opcode.");
  }

  // Set the pending cycles.
  pending_cycles = decoded_opcode->cycles;
  // Calculate stuff using the addressing mode.
  addressing_mode(decoded_opcode->mode);
  // Perform the actual instruction.
  execute(decoded_opcode->operation);

  status._ = 1;

  // Perform a sanity check.
  sanity();

  // Well this was also a cycle.
  pending_cycles--;
}

uint8_t CPU::irq() noexcept {
  logger->trace("External IRQ received.");
  if (!status.I) {
    logger->trace("Ignoring external IRQ (p.i is set)");
    return 0;
  }

  interrupt(0xFFFE);
  return pending_cycles;
}

uint8_t CPU::nmi() noexcept {
  logger->trace("External NMI received.");
  interrupt(0xFFFA);

  return pending_cycles;
}

CPU::~CPU() noexcept { logger->trace("Destructed the CPU."); }
} // namespace nes::cpu
