#ifndef NES_CART_H
#define NES_CART_H

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "mappers/mapper.h"

namespace nes::cart {
enum class MirroringMode {
  Horizontal,
  Vertical,
  OneScreenLo,
  OneScreenHi,
};

class Cart {
  std::vector<uint8_t> prg_rom;
  const uint8_t prg_banks;

  std::vector<uint8_t> chr_rom;
  const uint8_t chr_banks;

  std::unique_ptr<mappers::Mapper> mapper;

public:
  Cart(std::vector<uint8_t> prg_rom, uint8_t prg_banks,
       std::vector<uint8_t> chr_rom, uint8_t chr_banks,
       std::unique_ptr<mappers::Mapper> mapper,
       MirroringMode mirroring_mode) noexcept;
  ~Cart() noexcept;

  MirroringMode mirroring_mode;

  bool bus_read(uint16_t address, uint8_t &value) noexcept;
  bool bus_write(uint16_t address, uint8_t value) noexcept;
  bool ppu_read(uint16_t address, uint8_t &value) noexcept;
  bool ppu_write(uint16_t address, uint8_t value) noexcept;
};

std::optional<std::shared_ptr<Cart>>
load(const std::string &file_path) noexcept;
} // namespace nes::cart

#endif // NES_CART_H
