#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "bus.h"

namespace nes::bus {
auto logger = spdlog::stderr_color_mt("bus");

Bus::Bus() noexcept {
  logger->trace("Creating a new bus.");

  memory = std::move(std::make_unique<std::array<uint8_t, memory_size>>());
  logger->trace("Creating memory of size {} bytes.", memory->size());

  cpu = std::move(std::make_unique<cpu::CPU>());
}

Bus::~Bus() noexcept {
  logger->trace("Destructed the bus.");
}
} // namespace nes::bus
