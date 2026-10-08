#pragma once
// Drop-in replacement for the Teensy pedvide/ADC library API used by VWBMSV2.ino
// for current-sensor sampling, backed by the ESP32 Arduino core's analogRead().
#include <Arduino.h>

#if !defined(ARDUINO_ARCH_ESP32)
#error "This ADC compatibility shim only supports ESP32 (M5Dial) builds."
#endif

// Accepted for API compatibility; ESP32's analogRead() has no equivalent knob.
enum class ADC_CONVERSION_SPEED { VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED, VERY_HIGH_SPEED };
enum class ADC_SAMPLING_SPEED { VERY_LOW_SPEED, LOW_SPEED, MED_SPEED, HIGH_SPEED, VERY_HIGH_SPEED };

class ADCModule {
  public:
    void setAveraging(uint8_t count) { averaging = count > 0 ? count : 1; }
    void setResolution(uint8_t bits) {
      resolutionBits = bits;
      analogReadResolution(resolutionBits);
    }
    void setConversionSpeed(ADC_CONVERSION_SPEED) {}
    void setSamplingSpeed(ADC_SAMPLING_SPEED) {}

    void startContinuous(uint8_t pin) { activePin = pin; }

    uint32_t analogReadContinuous() {
      uint32_t total = 0;
      for (uint8_t i = 0; i < averaging; i++) {
        total += analogRead(activePin);
      }
      return total / averaging;
    }

    uint32_t getMaxValue() const { return (1u << resolutionBits) - 1; }

  private:
    uint8_t averaging = 1;
    uint8_t resolutionBits = 12;
    uint8_t activePin = 0;
};

class ADC {
  public:
    ADC() : adc0(new ADCModule()) {}

    // pedvide/ADC's ADC class forwards single-arg startContinuous() to adc0.
    void startContinuous(uint8_t pin) { adc0->startContinuous(pin); }

    ADCModule *adc0;
};
