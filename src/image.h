#ifndef NES_IMAGE_H
#define NES_IMAGE_H

#include <array>

#include <GLFW/glfw3.h>
#include <imgui_impl_opengl3.h>

namespace nes::image {
class Image {
  GLuint texture = -1;

public:
  Image() noexcept;

  void set_data(const GLuint *data, int width, int height) const noexcept;
  template <size_t size>
  void set_data(std::array<uint32_t, size> data, int width,
                int height) const noexcept {
    set_data((GLuint *)data.data(), width, height);
  }

  [[nodiscard]] ImTextureID imgui_image() const noexcept;
};
} // namespace nes::image

#endif // NES_IMAGE_H
