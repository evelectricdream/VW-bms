#include "FlexCAN.h"
#include "driver/twai.h"

FlexCAN Can0;

namespace {
  // M5Dial Grove Port A pins - only GPIO1/GPIO2 are broken out on this board.
  // Swap these two if the bus doesn't communicate (transceiver TX/RX reversed).
  constexpr gpio_num_t CAN_TX_PIN = GPIO_NUM_2;
  constexpr gpio_num_t CAN_RX_PIN = GPIO_NUM_1;
}

void FlexCAN::begin(uint32_t baud, const CAN_filter_t &filter) {
  (void)filter;
  if (started) {
    return;
  }

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
  g_config.rx_queue_len = 64;
  g_config.tx_queue_len = 32;

  twai_timing_config_t t_config;
  switch (baud) {
    case 1000000: t_config = TWAI_TIMING_CONFIG_1MBITS(); break;
    case 500000:  t_config = TWAI_TIMING_CONFIG_500KBITS(); break;
    case 250000:  t_config = TWAI_TIMING_CONFIG_250KBITS(); break;
    case 125000:  t_config = TWAI_TIMING_CONFIG_125KBITS(); break;
    default:      t_config = TWAI_TIMING_CONFIG_500KBITS(); break;
  }

  // Accept-all, matching the original code's zero-initialized id/mask filters.
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    twai_start();
    started = true;
  }
}

void FlexCAN::setFilter(const CAN_filter_t &filter, uint8_t slot) {
  // Hardware filter stays accept-all; module/PGN identification already
  // happens in software (canread()/decodecan()), matching original behavior.
  (void)filter;
  (void)slot;
}

int FlexCAN::available() {
  if (!started) {
    return 0;
  }
  twai_status_info_t status;
  if (twai_get_status_info(&status) != ESP_OK) {
    return 0;
  }
  return status.msgs_to_rx;
}

int FlexCAN::read(CAN_message_t &msg) {
  if (!started) {
    return 0;
  }
  twai_message_t rxMsg;
  if (twai_receive(&rxMsg, 0) != ESP_OK) {
    return 0;
  }

  msg.id = rxMsg.identifier;
  msg.ext = rxMsg.extd;
  msg.len = rxMsg.data_length_code;
  memcpy(msg.buf, rxMsg.data, sizeof(msg.buf));
  return 1;
}

int FlexCAN::write(const CAN_message_t &msg) {
  if (!started) {
    return 0;
  }
  twai_message_t txMsg = {};
  txMsg.identifier = msg.id;
  txMsg.extd = msg.ext;
  txMsg.data_length_code = msg.len;
  memcpy(txMsg.data, msg.buf, sizeof(txMsg.data));

  return twai_transmit(&txMsg, pdMS_TO_TICKS(10)) == ESP_OK ? 1 : 0;
}
