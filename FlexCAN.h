#pragma once
// Drop-in replacement for the Teensy FlexCAN_Library API, backed by the ESP32-S3's
// built-in TWAI controller. BMSModule/BMSModuleManager/VWBMSV2.ino are unmodified -
// only the underlying CAN peripheral driver changes.
#include <Arduino.h>
#include <stdint.h>

#if !defined(ARDUINO_ARCH_ESP32)
#error "This FlexCAN compatibility shim only supports ESP32 (M5Dial) builds."
#endif

// Field layout matches Teensy FlexCAN_Library's CAN_message_t so all existing
// msg.id / msg.ext / msg.len / msg.buf[] call sites work unchanged.
struct CAN_message_t {
  uint32_t id = 0;
  uint8_t ext = 0;
  uint8_t len = 8;
  uint8_t buf[8] = { 0 };
};

// Only flags.extended is used anywhere in this project; both filter mailboxes
// are left at id=0/mask=0 (accept-all), matching the original code's behavior.
struct CAN_filter_t {
  struct {
    uint8_t extended = 0;
    uint8_t remote = 0;
  } flags;
  uint32_t id = 0;
};

class FlexCAN {
  public:
    void begin(uint32_t baud, const CAN_filter_t &filter = CAN_filter_t());
    void setFilter(const CAN_filter_t &filter, uint8_t slot);
    int available();
    int read(CAN_message_t &msg);
    int write(const CAN_message_t &msg);

  private:
    bool started = false;
};

extern FlexCAN Can0;
