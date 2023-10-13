#ifndef NES_DSP_H
#define NES_DSP_H

#define _PI 3.141592835f

#include <memory>

namespace nes::dsp {
class AudioFilter {
public:
  float dt;
  float rc;

  explicit AudioFilter(unsigned int sample_rate, unsigned int frequency)
      : dt(1.f / (float)sample_rate), rc(1.f / (2.f * _PI * (float)frequency)) {
  }

  virtual ~AudioFilter() = default;
  inline virtual float filter(float sample) = 0;
};

class HighPassFilter : public AudioFilter {
  float alpha;
  float prev_x;
  float prev_y;

public:
  explicit HighPassFilter(unsigned int sample_rate, unsigned int frequency)
      : AudioFilter(sample_rate, frequency), alpha(rc / (rc + dt)), prev_x(0),
        prev_y(0) {}

  inline float filter(float sample) override {
    auto y = alpha * prev_y + alpha * (sample - prev_x);
    prev_x = sample;
    prev_y = y;
    return y;
  }
};

class LowPassFilter : public AudioFilter {
  float alpha;
  float prev_y;

public:
  explicit LowPassFilter(unsigned int sample_rate, unsigned int frequency)
      : AudioFilter(sample_rate, frequency), alpha(dt / (rc + dt)), prev_y(0) {}

  inline float filter(float sample) override {
    auto y = alpha * sample + (1 - alpha) * prev_y;
    prev_y = y;
    return y;
  }
};
} // namespace nes::dsp

#endif // NES_DSP_H
