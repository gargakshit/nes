#include "platform.h"

namespace nes::platform {
#ifndef __APPLE__
float font_backing_scale_factor() { return 1; }
#endif

void imgui_begin(const char *name) {
  ImGui::Begin(name);
  ImGui::SetWindowFontScale(1 / font_backing_scale_factor());
}
} // namespace nes::platform
