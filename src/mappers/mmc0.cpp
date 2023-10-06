#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "mmc0.h"

namespace nes::cart::mappers {
namespace mmc0 {
auto logger = spdlog::stderr_color_mt("nes::cart::mappers::mmc0");
}

MMC0::MMC0(uint8_t num_prg_chunks, uint8_t num_chr_chunks) noexcept
    : is_32k(num_prg_chunks > 1), is_chr_writable(num_chr_chunks == 0) {
  mmc0::logger->info(
      "PRG Chunks: {}, CHR Chunks: {}, is 32k: {}, CHR writable: {}",
      num_prg_chunks, num_chr_chunks, is_32k, is_chr_writable);
}

bool MMC0::should_bus_read(uint16_t addr, uint16_t &mapped) {
  // For 32k, 0x8000 -> 0xffff = prg[0x0000 -> 0x7fff].
  // For 16k, 0x8000 -> 0xffff = prg[0x0000 -> 0x3fff] (mirrored twice).

  if (addr < 0x8000)
    return false;

  mapped = addr & (is_32k ? 0x7fff : 0x3fff);
  return true;
}

bool MMC0::should_bus_write(uint16_t addr, uint16_t &mapped) {
  if (addr < 0x8000)
    return false;

  mapped = addr & (is_32k ? 0x7fff : 0x3fff);
  return true;
}

bool MMC0::should_ppu_read(uint16_t addr, uint16_t &mapped) {
  // 0x0000 -> 0x1fff -> chr[0x0000 -> 0x1fff];
  if (addr > 0x1fff)
    return false;

  mapped = addr;
  return true;
}

bool MMC0::should_ppu_write(uint16_t addr, uint16_t &mapped) {
  // If num_chr_chunks is zero, we treat this as RAM.
  if (addr > 0x1fff || !is_chr_writable)
    return false;

  mapped = addr;
  return true;
}

MMC0::~MMC0() noexcept {
  mmc0::logger->info("Destructed the mapper");
}
} // namespace nes::cart::mappers
