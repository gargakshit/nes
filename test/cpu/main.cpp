#include <fstream>

#include <fmt/format.h>

#include <nlohmann/json.hpp>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>

#include "opcode-test.h"
#include "opcode.h"

void setup_spdlog() {
  auto logger = spdlog::stderr_color_mt("nes::cpu::test");
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::info);
}

bool test_opcode(uint8_t);

int main() {
  setup_spdlog();

  uint8_t opcode_idx = -1;
  spdlog::stopwatch sw;
  for (const auto &opcode : nes::cpu::op::opcodes) {
    opcode_idx++;

    if (opcode.unknown) {
      continue;
    }

    if (!test_opcode(opcode_idx)) {
      spdlog::error("Tests failed for opcode {:#04x}", opcode_idx);
      return 1;
    }
  }

  spdlog::info(
      "All tests passed! You are good to go :) (total time elapsed = {}s)", sw);
  return 0;
}

bool test_opcode(uint8_t opcode) {
  auto test_file = fmt::format("test/cpu/tests/{:02x}.json", opcode);
  auto prefix = fmt::format("[{:#04x}]", opcode);

  spdlog::info("{} Loading test", prefix);
  spdlog::debug("{} Loading {}", prefix, test_file);

  std::ifstream f(test_file);
  nlohmann::json j;
  f >> j;

  f.close();

  auto tests = j.template get<std::vector<nlohmann::json>>();
  spdlog::info("{} Starting test", prefix);

  using namespace nes::cpu::test;

  uint64_t test_idx = 0;

  spdlog::stopwatch sw;
  for (const auto &test_ : tests) {
    auto test = test_.template get<OpcodeTest>();
    test_idx++;

    spdlog::debug("{} [{:08}] Running ({})", prefix, test_idx, test.name);

    if (test.run()) {
      spdlog::debug("{} [{:08}] Passed  ({})", prefix, test_idx, test.name);
    } else {
      spdlog::error("{} [{:08}] Failed  ({})", prefix, test_idx, test.name);
      return false;
    }
  }

  spdlog::info("{} Passed, elapsed = {}s", prefix, sw);
  return true;
}
