#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "cpu.h"

auto logger = spdlog::stderr_color_mt("cpu");

namespace nes::cpu {
CPU::CPU() {
  logger->trace("Constructed a new CPU");
}

CPU::~CPU() {
  logger->trace("Deleted the CPU");
}
} // namespace nes::cpu
