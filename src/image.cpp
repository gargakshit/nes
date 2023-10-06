#include "image.h"

namespace nes::image {
Image::Image() noexcept {
  // Create a new texture.
  glGenTextures(1, &texture);
  // Bind that texture.
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set some texture parameters.
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Image::set_data(const GLuint *data, int width, int height) const noexcept {
  // Bind the texture.
  glBindTexture(GL_TEXTURE_2D, texture);
  // Blit the image to the texture.
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, data);
}

ImTextureID Image::imgui_image() const noexcept {
  return (void *)(intptr_t)texture;
}
} // namespace nes::image
