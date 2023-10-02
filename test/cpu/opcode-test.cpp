#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "cpu.h"
#include "opcode-test.h"

namespace nes::cpu::test {
void from_json(const nlohmann::json &j, OpcodeTest &t) {
  j.at("name").get_to(t.name);
  j.at("initial").get_to(t.initial);
  j.at("final").get_to(t.final);
  j.at("cycles").get_to(t.cycles);
}

void from_json(const nlohmann::json &j, OpcodeTestState &s) {
  j.at("pc").get_to(s.pc);
  j.at("s").get_to(s.sp);
  j.at("a").get_to(s.a);
  j.at("x").get_to(s.x);
  j.at("y").get_to(s.y);
  j.at("p").get_to(s.p);
  j.at("ram").get_to(s.ram);
}

auto logger = spdlog::stderr_color_mt("nes::test::cpu::opcode");

bool OpcodeTest::run() {
  using namespace nes::cpu;

  auto memory = std::array<uint8_t, 1 << 16>();

  CPU cpu(
      [&memory](uint16_t addr) {
        auto value = memory[addr];
        logger->trace("Reading from {:#06x} = {:#04x}", addr, value);
        return value;
      },
      [&memory](uint16_t addr, uint8_t val) {
        logger->trace("Writing to {:#06x} ({:#04x})", addr, val);
        memory[addr] = val;
      });

  // Set the registers.
  cpu.pc = initial.pc;
  cpu.sp = initial.sp;
  cpu.a = initial.a;
  cpu.x = initial.x;
  cpu.y = initial.y;
  cpu.p = initial.p;

  // Set the pending cycles to zero.
  cpu.pending_cycles = 0;

  // Initialize the memory.
  for (const auto &ram_contents : initial.ram)
    memory[ram_contents.first] = ram_contents.second;

  cpu.dump_state();

  auto num_cycles = cycles.size();
  for (auto i = 0; i < num_cycles; i++) {
    cpu.tick();
  }

  cpu.dump_state();

  if (final.pc != cpu.pc) {
    logger->error("final.pc ({:#06x}) != cpu.pc ({:#06x})", final.pc, cpu.pc);
    return false;
  }

  if (final.sp != cpu.sp) {
    logger->error("final.sp ({:#04x}) != cpu.sp ({:#04x})", final.sp, cpu.sp);
    return false;
  }

  if (final.a != cpu.a) {
    logger->error("final.a ({:#04x}) != cpu.a ({:#04x})", final.a, cpu.a);
    return false;
  }

  if (final.x != cpu.x) {
    logger->error("final.x ({:#04x}) != cpu.x ({:#04x})", final.x, cpu.x);
    return false;
  }

  if (final.y != cpu.y) {
    logger->error("final.y ({:#04x}) != cpu.y ({:#04x})", final.y, cpu.y);
    return false;
  }

  // Ignore bit 5 (it is hardwired to 1).
  if ((final.p & 0b00100000) != (cpu.p & 0b00100000)) {
    logger->error("final.p ({:#04x}) != cpu.p ({:#04x})", final.p, cpu.p);
    return false;
  }

  for (const auto &ram_contents : final.ram) {
    auto &addr = ram_contents.first;
    if (memory[addr] != ram_contents.second) {
      logger->error("ram[{:#06x}] ({:#04x}) != final.ram[{:#06x}] ({:#04x})",
                    addr, memory[addr], addr, ram_contents.second);
      return false;
    }
  }

  if (cpu.pending_cycles != 0) {
    logger->error("cpu.pending_cycles = {} (!= 0)", cpu.pending_cycles);
    return false;
  }

  return true;
}
} // namespace nes::cpu::test
