#include <chrono>

#include <imgui.h>
#include <imgui_impl_glfw.h>
// TODO(AG): Add support for Metal.
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "bus.h"
#include "cart.h"
#include "gui.h"
#include "platform.h"

#ifdef __APPLE__
#warning Using OpenGL on an Apple platform.
#endif

void setup_spdlog() {
  auto logger = spdlog::stderr_color_mt("nes");
  spdlog::set_default_logger(logger);

#ifdef NES_CPU_RT
  spdlog::set_level(spdlog::level::debug);
#else
  spdlog::set_level(spdlog::level::info);
#endif
}

using namespace nes;

static void glfw_error_callback(int error, const char *description) {
  spdlog::error("GLFW Error: code = {}, description = {}", error, description);
}

int main([[maybe_unused]] const int argc, [[maybe_unused]] const char **argv) {
  setup_spdlog();

  auto loaded_cart = cart::load("carts/nestest.nes");
  if (!loaded_cart) {
    return 1;
  }

  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) {
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);    // Required on Mac
  glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_TRUE); // Render on high-dpi.
#else
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

  // Create window with graphics context
  auto window = glfwCreateWindow(1280, 720, "NES", nullptr, nullptr);
  if (window == nullptr) {
    return 1;
  }

  auto scale = platform::font_backing_scale_factor();
  spdlog::info("Platform display scaling: {}", scale);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize ImGui.
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  auto io = ImGui::GetIO();
  (void)io;

  // Setup high-dpi font on the platform.
  ImFontConfig font_cfg;
  font_cfg.SizePixels = 13.0f * scale;
  io.Fonts->AddFontDefault(&font_cfg);
  io.DisplayFramebufferScale = ImVec2(scale, scale);
  io.FontGlobalScale = 1.0f / scale;

  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;           // Enable keyboard controls.
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking.

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  auto &style = ImGui::GetStyle();
  style.FrameRounding = 2;
  style.WindowRounding = 4;
  style.WindowPadding = ImVec2(16, 12);
  style.Colors[ImGuiCol_WindowBg] =
      ImVec4(0.094f, 0.094f, 0.101f, 1); // ~#18181A

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  auto bus = bus::Bus(std::move(*loaded_cart));

  GLuint screen_texture;

  gui::GUI gui(bus);

  auto clear_color = ImVec4(0.024f, 0.024f, 0.03f, 1.00f);

  using std::chrono::high_resolution_clock;
  using namespace std::literals;

  // Get the remaining time until the next burst of ticks.
  int64_t bus_residual_time_us = 0;
  auto old_frame_start = high_resolution_clock::now();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    auto frame_start = high_resolution_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
                          frame_start - old_frame_start)
                          .count();
    old_frame_start = frame_start;

    if (bus_residual_time_us > 0) {
      bus_residual_time_us -= elapsed_us;
      spdlog::trace("Residual time: {}, elapsed_us = {}", bus_residual_time_us,
                    elapsed_us);
    } else {
      bus_residual_time_us += (1000000 / 60) - elapsed_us;

      // Drain the bus.
      while (!bus.ppu.frame_complete)
        bus.tick();
      // Prepare for the next frame.
      bus.ppu.frame_complete = false;
    }

    bus.ppu.frame_complete = false;
    spdlog::debug("Elapsed {}, FC = {}", bus.elapsed_cycles,
                  bus.ppu.frame_complete);

    // Initialize a new frame.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    gui.render();

    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
