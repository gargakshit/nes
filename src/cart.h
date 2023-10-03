#ifndef NES_CART_H
#define NES_CART_H

#include <memory>
#include <optional>
#include <string>

namespace nes::cart {
class Cart {
public:
  Cart() noexcept;
};

std::optional<std::shared_ptr<Cart>>
load(const std::string &file_path) noexcept;
} // namespace nes::cart

#endif // NES_CART_H
