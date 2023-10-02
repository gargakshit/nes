#ifndef NES_OPCODE_TEST_H
#define NES_OPCODE_TEST_H

#include <array>
#include <string>
#include <tuple>
#include <vector>

#include <nlohmann/json.hpp>

namespace nes::cpu::test {
struct OpcodeTestState {
  uint16_t pc;
  uint8_t sp;
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t p;
  std::vector<std::pair<uint16_t, uint8_t>> ram;
};

using OpcodeTestCycles = std::tuple<uint16_t, uint8_t, std::string>;

struct OpcodeTest {
  std::string name;
  OpcodeTestState initial;
  OpcodeTestState final;
  std::vector<OpcodeTestCycles> cycles;

  bool run();
};

void from_json(const nlohmann::json &, OpcodeTest &);
void from_json(const nlohmann::json &, OpcodeTestState &);
} // namespace nes::cpu::test

#endif // NES_OPCODE_TEST_H
