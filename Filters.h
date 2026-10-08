#pragma once
// Portable reimplementation of JonHub/Filters' FilterOnePole (one-pole RC
// low-pass), matching its class name/API exactly. Pure float math, no MCU-
// specific code, so behavior is identical to the original Teensy build.
#include <Arduino.h>

enum FilterTypes { LOWPASS, HIGHPASS };

class FilterOnePole {
  public:
    FilterOnePole(FilterTypes filterType, float freq) {
      setAsFilter(filterType, freq);
    }

    void setAsFilter(FilterTypes filterType, float freq) {
      _filter_type = filterType;
      _rc = 1.0f / (2.0f * PI * freq);
      _output_value = 0.0f;
      _input_value_last = 0.0f;
      _last_time = micros() / 1000000.0f;
    }

    void input(float input) {
      float current_time = micros() / 1000000.0f;
      float dt = current_time - _last_time;
      _last_time = current_time;

      if (_filter_type == LOWPASS) {
        float a = dt / (_rc + dt);
        _output_value = a * input + (1.0f - a) * _output_value;
      } else {
        float a = _rc / (_rc + dt);
        _output_value = a * (_output_value + input - _input_value_last);
        _input_value_last = input;
      }
    }

    float output() const { return _output_value; }

  private:
    FilterTypes _filter_type;
    float _rc;
    float _output_value;
    float _input_value_last;
    float _last_time;
};
