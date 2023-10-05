#include <fstream>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "cart.h"
#include "mappers/mmc0.h"

namespace nes::cart {
auto logger = spdlog::stderr_color_mt("nes::cart");

Cart::Cart(std::vector<uint8_t> prg_rom, uint8_t prg_banks,
           std::vector<uint8_t> chr_rom, uint8_t chr_banks,
           std::unique_ptr<mappers::Mapper> mapper) noexcept
    : prg_rom(std::move(prg_rom)), prg_banks(prg_banks),
      chr_rom(std::move(chr_rom)), chr_banks(chr_banks),
      mapper(std::move(mapper)) {
  logger->debug("PRG Size: {}", this->prg_rom.size());
  logger->debug("CHR Size: {}", this->chr_rom.size());
}

bool Cart::bus_read(uint16_t address, uint8_t &value) noexcept {
  uint16_t mapped_addr = 0;
  if (mapper->should_bus_read(address, mapped_addr)) {
    value = prg_rom[mapped_addr];
    return true;
  }

  return false;
}

bool Cart::bus_write(uint16_t address, uint8_t value) noexcept {
  uint16_t mapped_addr = 0;
  if (mapper->should_bus_write(address, mapped_addr)) {
    prg_rom[mapped_addr] = value;
    return true;
  }

  return false;
}

bool Cart::ppu_read(uint16_t address, uint8_t &value) noexcept {
  uint16_t mapped_addr = 0;
  if (mapper->should_ppu_read(address, mapped_addr)) {
    value = chr_rom[mapped_addr];
    return true;
  }

  return false;
}

bool Cart::ppu_write(uint16_t address, uint8_t value) noexcept {
  uint16_t mapped_addr = 0;
  if (mapper->should_ppu_write(address, mapped_addr)) {
    chr_rom[mapped_addr] = value;
    return true;
  }

  return false;
}

Cart::~Cart() noexcept { logger->trace("Destructed the cart."); }

struct Header {
  char magic[4];
  uint8_t num_prg_chunks;
  uint8_t num_chr_chunks;

  union {
    struct {
      uint8_t _ : 3;
      uint8_t trainer : 1;
      uint8_t mapper_lower : 4;
    };
    uint8_t raw;
  } flags_1;

  union {
    struct {
      uint8_t _ : 4;
      uint8_t mapper_upper : 4;
    };
    uint8_t raw;
  } flags_2;

  char padding[8];
};

std::optional<std::unique_ptr<mappers::Mapper>>
select_mapper(uint8_t id, uint8_t num_prg_chunks,
              uint8_t num_chr_chunks) noexcept {
  switch (id) { // NOLINT(*-multiway-paths-covered)
  case 0:
    return std::make_unique<mappers::MMC0>(num_prg_chunks, num_chr_chunks);

  default: return std::nullopt;
  }
}

std::optional<std::shared_ptr<Cart>>
load(const std::string &file_path) noexcept {
  std::ifstream file(file_path);
  if (!file) {
    logger->error("Unable to open the cart from {}", file_path);
    file.close();
    return std::nullopt;
  }

  // Initialize a new header with the magic set as "INV".
  Header header{.magic = "INV"};
  file.read((char *)&header, sizeof(header));

  if (header.magic[0] != 'N' || header.magic[1] != 'E' ||
      header.magic[2] != 'S' || header.magic[3] != 0x1a) {
    logger->error("Invalid magic header for the cart file at {}", file_path);
    file.close();
    return std::nullopt;
  }

  // Skip the trainer if there is any.
  if (header.flags_1.trainer) {
    logger->debug("Skipping trainer");
    file.seekg(512, std::ios_base::cur);
  }

  uint8_t mapper_id =
      header.flags_1.mapper_lower | (header.flags_2.mapper_upper << 4);
  logger->info("Cart {} uses mapper_id {:03d}", file_path, mapper_id);

  // We are only handling file type 1 for now.
  std::vector<uint8_t> prg_rom((1 << 14) * header.num_prg_chunks);
  file.read((char *)prg_rom.data(), (long)prg_rom.size());

  std::vector<uint8_t> chr_rom((1 << 13) * header.num_chr_chunks);
  file.read((char *)chr_rom.data(), (long)chr_rom.size());

  file.close();

  auto mapper =
      select_mapper(mapper_id, header.num_prg_chunks, header.num_chr_chunks);
  if (!mapper) {
    return std::nullopt;
  }

  return std::make_shared<Cart>(prg_rom, header.num_prg_chunks, chr_rom,
                                header.num_chr_chunks, std::move(*mapper));
}
} // namespace nes::cart
