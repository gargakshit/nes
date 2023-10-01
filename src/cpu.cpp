#include <iostream>

#include "cpu.h"

namespace nes::cpu {
CPU::CPU() {
  std::cout << "new cpu!" << std::endl;
}

CPU::~CPU() {
  std::cout << "delete cpu" << std::endl;
}
} // namespace nes::cpu
