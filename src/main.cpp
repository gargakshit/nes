#include <iostream>
#include <memory>

#include "cpu.h"

using namespace nes;

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char **argv) {
  std::cout << "Hello, World!" << std::endl;
  auto cpu = std::make_unique<cpu::CPU>();

  return 0;
}
