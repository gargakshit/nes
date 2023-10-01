#include <memory>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "cpu.h"

using namespace nes;

void setup_spdlog() {
  auto logger = spdlog::stderr_color_mt("nes");
  spdlog::set_default_logger(logger);

  spdlog::set_level(spdlog::level::trace);
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char **argv) {
  setup_spdlog();

  auto cpu = std::make_unique<cpu::CPU>();

  return 0;
}
