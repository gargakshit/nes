#include <imgui.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "gui.h"
#include "platform.h"

namespace nes::gui {
const static auto label_color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
auto logger = spdlog::stderr_color_mt("nes::gui");

GUI::GUI(bus::Bus &bus) noexcept : bus(bus) {
  logger->debug("Initializing GUI.");
}

void GUI::render_system_metrics() noexcept {
  platform::imgui_begin("Bus Metrics");

  ImGui::TextColored(label_color, "Cycles");
  ImGui::SameLine();
  ImGui::Text("%llu", bus.elapsed_cycles);

  if (ImGui::Button("Debug: Tick")) {
    bus.tick();
  }

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

void GUI::render() noexcept {
  render_system_metrics();
  render_cpu_state();
}

GUI::~GUI() noexcept { logger->debug("Destructing the GUI."); }
} // namespace nes::gui
