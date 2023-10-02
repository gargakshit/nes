#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "bus.h"

using namespace nes;

void setup_spdlog() {
  auto logger = spdlog::stderr_color_mt("nes");
  spdlog::set_default_logger(logger);

  spdlog::set_level(spdlog::level::info);
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char **argv) {
  setup_spdlog();

  bus::Bus bus;

  for (auto i = 0; i < 128; i++) {
    bus.tick();
  }

  return 0;
}
