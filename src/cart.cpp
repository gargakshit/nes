#include <fstream>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "cart.h"

namespace nes::cart {
auto logger = spdlog::stderr_color_mt("nes::cart");

Cart::Cart() noexcept = default;

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

  uint8_t mapper =
      header.flags_1.mapper_lower | (header.flags_2.mapper_upper << 4);
  logger->info("Cart {} uses mapper {:03d}", file_path, mapper);

  file.close();
  return std::nullopt;
}
} // namespace nes::cart
