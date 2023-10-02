#include <AppKit/NSScreen.h>

#include "platform.h"

namespace nes::platform {
float font_backing_scale_factor() {
  return (float)NSScreen.mainScreen.backingScaleFactor;
}
} // namespace nes::platform
