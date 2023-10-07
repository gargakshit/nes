#include <imgui.h>

#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "gui.h"
#include "platform.h"
#include "ppu.h"

namespace nes::gui {
const static auto label_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
auto logger = spdlog::stderr_color_mt("nes::gui");

GUI::GUI(bus::Bus &bus) noexcept
    : bus(bus), screen(), pattern_table_left(), pattern_table_right(),
      rendered_palette() {
  logger->debug("Initializing GUI.");
}

void GUI::render_system_metrics() noexcept {
  auto now = high_resolution_clock::now();
  auto difference = now - last_clock_capture;

  using namespace std::literals;
  if ((difference / 1us) >= 1000000) {
    elapsed_clocks_second = bus.elapsed_cycles - clocks_second_snapshot;
    clocks_second_snapshot = bus.elapsed_cycles;
    last_clock_capture = now;
  }

  platform::imgui_begin("Metrics");

  ImGui::TextColored(label_color, "Cycles");
  ImGui::SameLine();
  ImGui::Text("%llu", bus.elapsed_cycles);

  auto io = ImGui::GetIO();

  ImGui::TextColored(label_color, "FPS");
  ImGui::SameLine();
  ImGui::Text("%f", io.Framerate);

  ImGui::TextColored(label_color, "Speed");
  ImGui::SameLine();
  ImGui::Text("%llu Hz", elapsed_clocks_second);

  ImGui::TextColored(label_color, "Frames");
  ImGui::SameLine();
  ImGui::Text("%d", ImGui::GetFrameCount());

  ImGui::End();
}

void GUI::render_cpu_state() noexcept {
  platform::imgui_begin("CPU");

  ImGui::TextColored(label_color, "Registers");

  ImGui::TextColored(label_color, "a   ");
  ImGui::SameLine();
  ImGui::Text("%02x", bus.cpu.a);

  ImGui::TextColored(label_color, "x   ");
  ImGui::SameLine();
  ImGui::Text("%02x", bus.cpu.x);

  ImGui::TextColored(label_color, "y   ");
  ImGui::SameLine();
  ImGui::Text("%02x", bus.cpu.y);

  ImGui::TextColored(label_color, "sp  ");
  ImGui::SameLine();
  ImGui::Text("%02x", bus.cpu.sp);

  ImGui::TextColored(label_color, "pc  ");
  ImGui::SameLine();
  ImGui::Text("%04x", bus.cpu.pc);

  ImGui::TextColored(label_color, "p   ");
  ImGui::SameLine();
  ImGui::Text("%02x", bus.cpu.p);

  ImGui::TextColored(label_color, "p.C ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.C);

  ImGui::TextColored(label_color, "p.Z ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.Z);

  ImGui::TextColored(label_color, "p.I ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.I);

  ImGui::TextColored(label_color, "p.D ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.D);

  ImGui::TextColored(label_color, "p.B ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.B);

  ImGui::TextColored(label_color, "p.1 ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status._);

  ImGui::TextColored(label_color, "p.V ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.V);

  ImGui::TextColored(label_color, "p.N ");
  ImGui::SameLine();
  ImGui::Text("%x", bus.cpu.status.N);

  ImGui::NewLine();
  ImGui::TextColored(label_color, "System");

  ImGui::TextColored(label_color, "Pending cycles");
  ImGui::SameLine();
  ImGui::Text("%d", bus.cpu.pending_cycles);

  ImGui::TextColored(label_color, "Opcode");
  ImGui::SameLine();
  ImGui::Text("%02x", bus.cpu.opcode);

  ImGui::End();
}

void GUI::render_ppu_state() noexcept {
  pattern_table_left.set_data(bus.ppu.pattern_table(0), 128, 128);
  pattern_table_right.set_data(bus.ppu.pattern_table(1), 128, 128);
  rendered_palette.set_data(bus.ppu.get_rendered_palettes(), 16, 2);

  platform::imgui_begin("PPU State");

  const static auto patterntable_resolution = ImVec2(
      128 * display_resolution_multiplier, 128 * display_resolution_multiplier);

  ImGui::TextColored(label_color, "Patterntable 0");
  ImGui::Image(pattern_table_left.imgui_image(), patterntable_resolution);

  ImGui::TextColored(label_color, "Patterntable 1");
  ImGui::Image(pattern_table_right.imgui_image(), patterntable_resolution);

  ImGui::TextColored(label_color, "Palette");
  ImGui::Image(rendered_palette.imgui_image(),
               ImVec2(16 * 8 * display_resolution_multiplier,
                      2 * 8 * display_resolution_multiplier));

  ImGui::End();
}

void GUI::render_screen() const noexcept {
  using namespace nes::ppu;

  screen.set_data(bus.ppu.screen, PPU::screen_width, PPU::screen_height);
  const static auto size =
      ImVec2(PPU::screen_width * display_resolution_multiplier,
             PPU::screen_height * display_resolution_multiplier);

  platform::imgui_begin("Screen (NTSC)");
  ImGui::Image(screen.imgui_image(), size);
  ImGui::End();
}

void GUI::render_controller_input() const noexcept {
  platform::imgui_begin("Controllers");

  ImGui::TextColored(label_color, "Controller 0");

  ImGui::TextColored(label_color, "Up");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.up);

  ImGui::TextColored(label_color, "Down");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.down);

  ImGui::TextColored(label_color, "Left");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.left);

  ImGui::TextColored(label_color, "Right");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.right);

  ImGui::TextColored(label_color, "Select");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.select);

  ImGui::TextColored(label_color, "Start");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.start);

  ImGui::TextColored(label_color, "A");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.a);

  ImGui::TextColored(label_color, "B");
  ImGui::SameLine();
  ImGui::Text("%d", bus.controller_1.b);

  ImGui::End();
}

void GUI::render() noexcept {
  render_system_metrics();
  render_cpu_state();
  render_screen();
  render_ppu_state();
  render_controller_input();
}

GUI::~GUI() noexcept { logger->debug("Destructing the GUI."); }
} // namespace nes::gui
