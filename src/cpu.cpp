#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "cpu.h"
#include "utils.h"

// Enable runtime CPU sanity checks.
// If we reach an impossible state, we panic.
#define NES_CPU_RT_SANITY

#define SANITY_PANIC(msg) \
  do {                    \
    dump_state();         \
    PANIC(msg);           \
  } while (false);

namespace nes::cpu {
auto logger = spdlog::stderr_color_mt("cpu");

CPU::CPU() noexcept: Registers() {
  logger->trace("Constructing the CPU.");
  logger->trace("M6502 forever!");
  rst();
}

void CPU::rst() noexcept {
  logger->trace("Resetting the CPU.");

  a = 0;
  x = 0;
  y = 0;
  sp = 0;
  pc = 0;
  p = 0;
  status._ = 1;

  dump_state();
  sanity();
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

void CPU::dump_state() noexcept {
  dump_reg();
}

void CPU::sanity() noexcept {
#ifdef NES_CPU_RT_SANITY
  if ((p & 0b00100000) != 0b00100000) {
    SANITY_PANIC("Sanity: always-set status bit is not set.");
  }

  if (status._ != 1) {
    SANITY_PANIC("Sanity: always-set status bit is not set, invalid status bitfield pack.");
  }
#endif
}

CPU::~CPU() noexcept {
  logger->trace("Destructed the CPU.");
}
} // namespace nes::cpu
