#ifndef NES_MMC0_H
#define NES_MMC0_H

#include "mapper.h"

namespace nes::cart::mappers {
class MMC0 : public Mapper {
  bool is_32k;
  bool is_chr_writable;

public:
  MMC0(uint8_t num_prg_chunks, uint8_t num_chr_chunks) noexcept;
  ~MMC0() noexcept override;

  bool should_bus_read(uint16_t addr, uint16_t &mapped) override;
  bool should_bus_write(uint16_t addr, uint16_t &mapped) override;
  bool should_ppu_read(uint16_t addr, uint16_t &mapped) override;
  bool should_ppu_write(uint16_t addr, uint16_t &mapped) override;
};
} // namespace nes::cart::mappers

#endif // NES_MMC0_H
