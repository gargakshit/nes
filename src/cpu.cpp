#include <spdlog/sinks/stdout_color_sinks.h>

#include <utility>

#include "cpu.h"
#include "utils.h"

// Enable runtime CPU sanity checks.
// If we reach an impossible state, we panic. It is recommended that we don't
// build with sanity checks enabled as those cost a lot of performance.
#define NES_CPU_RT_SANITY

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

  auto op = op::opcodes[opcode];
  logger->trace("Executing opcode {}", op);

  if (op.unknown) {
    PANIC("Unknown opcode.");
  }

  // Set the pending cycles.
  pending_cycles = op.cycles;
  // Calculate stuff using the addressing mode.
  addressing_mode(op.mode);

  // Perform a sanity check.
  sanity();

  // Well this was also a cycle.
  pending_cycles--;
}

uint8_t CPU::irq() noexcept {
  logger->trace("External IRQ received.");
  return 0;
}

uint8_t CPU::nmi() noexcept {
  logger->trace("External NMI received.");
  return 0;
}

CPU::~CPU() noexcept { logger->trace("Destructed the CPU."); }
} // namespace nes::cpu
