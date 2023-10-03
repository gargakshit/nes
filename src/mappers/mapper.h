#ifndef NES_MAPPER_H
#define NES_MAPPER_H

#include <cstdint>

namespace nes::cart::mappers {
class Mapper {
public:
  virtual bool should_bus_read(uint16_t addr, uint16_t &mapped) = 0;
  virtual bool should_bus_write(uint16_t addr, uint16_t &mapped) = 0;
  virtual bool should_ppu_read(uint16_t addr, uint16_t &mapped) = 0;
  virtual bool should_ppu_write(uint16_t addr, uint16_t &mapped) = 0;
};
} // namespace nes::cart::mappers

#endif // NES_MAPPER_H
