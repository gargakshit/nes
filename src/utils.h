#ifndef NES_SRC_UTILS_H
#define NES_SRC_UTILS_H

#include <cstdlib>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace nes::utils {
auto global_panic_logger = spdlog::stderr_color_mt("PANIC");
} // namespace nes::utils

#define PANIC(msg) \
  do {             \
    nes::utils::global_panic_logger->critical("PANIC ({}:{}): {}", __FILE__, __LINE__, msg); \
    nes::utils::global_panic_logger->flush();                                                \
    exit(1);       \
  } while(false)

#endif // NES_SRC_UTILS_H
