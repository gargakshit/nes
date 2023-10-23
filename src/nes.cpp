#include <RtAudio.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "cart.h"
#include "nes.h"

namespace nes {
// We'll keep a global reference. I don't like it, but yea...
auto logger = spdlog::stderr_color_mt("nes::system");
static unsigned int sample_buf_size = 512;

static int audio_callback(void *output, [[maybe_unused]] void *input,
                          unsigned int frames, double time,
                          RtAudioStreamStatus status, void *userData) {
  auto buffer = (float *)output;
  auto system = (System *)userData;

  unsigned int ticks = 5369318 * frames / apu::APU::sample_rate;
  system->tick(ticks);

  if (sample_buf_size != 512 && frames != sample_buf_size) {
    logger->error("Can't buffer 512 samples");
  }

  if (status) {
    logger->error("Stream underflow");
  }

  for (int i = 0; i < frames; i++) {
    auto sample = system->bus.apu.samples[i];
    buffer[i] = sample;
    buffer[i + 512] = sample;
  }

  return 0;
}

static RtAudio::StreamOptions options;

System::System(const std::shared_ptr<cart::Cart> &cart) noexcept
    : bus(cart), gui(bus), audio() {
  auto deviceId = audio.getDefaultOutputDevice();
  auto device = audio.getDeviceInfo(deviceId);

  logger->info("Audio device: {}", device.name);

  parameters.deviceId = deviceId;
  parameters.nChannels = 2;
  parameters.firstChannel = 0;

  logger->info("Audio params: channels = {}, sample_rate = {}, buffer_size = "
               "{}, format = float32",
               parameters.nChannels, apu::APU::sample_rate, sample_buf_size);

  options.numberOfBuffers = 3;
  options.flags |= RTAUDIO_NONINTERLEAVED;

  if (audio.openStream(&parameters, nullptr, RTAUDIO_FLOAT32,
                       apu::APU::sample_rate, &sample_buf_size, &audio_callback,
                       this, &options)) {
    logger->error("Unable to open an audio stream, err = {}.",
                  audio.getErrorText());
    error_init = true;
    return;
  }

  if (audio.startStream()) {
    logger->error("Unable to start the audio stream, err = {}.",
                  audio.getErrorText());
    error_init = true;
    return;
  }
}

void System::tick(unsigned int n) noexcept {
  for (auto i = 0; i < n; i++)
    bus.tick();
}

void System::render() noexcept { gui.render(); }

void System::set_key(const controller::Button key,
                     const bool pressed) noexcept {
  bus.controller_1.set_key(key, pressed);
}

System::~System() noexcept {
  if (audio.isStreamRunning())
    audio.stopStream();

  if (audio.isStreamRunning())
    audio.closeStream();
}
} // namespace nes
