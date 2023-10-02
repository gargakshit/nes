#ifndef NES_PLATFORM_H
#define NES_PLATFORM_H

#include <imgui.h>

namespace nes::platform {
float font_backing_scale_factor();
void imgui_begin(const char* name);
} // namespace nes::platform

#endif // NES_PLATFORM_H
