#include <functional>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "bus.h"

namespace nes::bus {
auto logger = spdlog::stderr_color_mt("nes::bus");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-avoid-bind"
Bus::Bus() noexcept
    : cpu(cpu::CPU(std::bind(&Bus::read, this, std::placeholders::_1),
                   std::bind(&Bus::write, this, std::placeholders::_1,
                             std::placeholders::_2))),
      memory(std::array<uint8_t, memory_size>()) {
  logger->trace("Creating a new bus.");
  logger->trace("Created a memory of size {0:#x} ({0}) bytes.", memory.size());

  memory[0x10] = 0x02;
}
#pragma clang diagnostic pop

uint8_t Bus::read(uint16_t address) noexcept {
  auto value = memory[address];
  logger->trace("Reading from address {:#06x} = {:#04x}", address, value);
  return value;
}

void Bus::write(uint16_t address, uint8_t value) noexcept {
  logger->trace("Writing to address {:#06x} = {:#04x}", address, value);
  memory[address] = value;
}

void Bus::tick() noexcept {
  elapsed_cycles++;
  cpu.tick();
}

Bus::~Bus() noexcept { logger->trace("Destructed the bus."); }
} // namespace nes::bus
