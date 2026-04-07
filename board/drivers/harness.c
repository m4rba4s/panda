#include "board/config.h"
#include "board/main_declarations.h"

#if defined(PANDA_JUNGLE)
  #include "board/jungle/boards/board_declarations.h"
#elif defined(PANDA_BODY)
  #include "board/body/boards/board_declarations.h"
#else
  #include "board/boards/board_declarations.h"
#endif

#include "board/drivers/harness.h"
#include "board/drivers/fdcan.h"
#include "board/drivers/led.h"
#include "board/stm32h7/lladc.h"
#include "board/drivers/gpio.h"

struct harness_t harness = {
  .status = HARNESS_STATUS_NC,
};

void harness_init(void) {
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
  if (current_board->harness_config != NULL) {
    set_gpio_mode(current_board->harness_config->GPIO_relay_SBU1, current_board->harness_config->pin_relay_SBU1, MODE_OUTPUT);
    set_gpio_mode(current_board->harness_config->GPIO_relay_SBU2, current_board->harness_config->pin_relay_SBU2, MODE_OUTPUT);
    set_gpio_output(current_board->harness_config->GPIO_relay_SBU1, current_board->harness_config->pin_relay_SBU1, 0);
    set_gpio_output(current_board->harness_config->GPIO_relay_SBU2, current_board->harness_config->pin_relay_SBU2, 0);
  }
#endif
}

uint8_t harness_check_ignition(void) {
  uint8_t ret = HARNESS_STATUS_NC;
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
  if (current_board->harness_config != NULL) {
    uint16_t sbu1_voltage = (uint16_t)adc_get_mV(&(current_board->harness_config->adc_signal_SBU1));
    uint16_t sbu2_voltage = (uint16_t)adc_get_mV(&(current_board->harness_config->adc_signal_SBU2));

    if ((sbu1_voltage > 500U) || (sbu2_voltage > 500U)) {
      if (sbu1_voltage > sbu2_voltage) {
        ret = HARNESS_STATUS_NORMAL;
      } else {
        ret = HARNESS_STATUS_FLIPPED;
      }
    }
  }
#endif
  harness.status = ret;
  return ret;
}

void harness_set_orientation(uint8_t orientation) {
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
  if (current_board->harness_config != NULL) {
    if (orientation == HARNESS_ORIENTATION_NORMAL) {
      set_gpio_output(current_board->harness_config->GPIO_relay_SBU1, current_board->harness_config->pin_relay_SBU1, 1);
      set_gpio_output(current_board->harness_config->GPIO_relay_SBU2, current_board->harness_config->pin_relay_SBU2, 0);
    } else if (orientation == HARNESS_ORIENTATION_FLIPPED) {
      set_gpio_output(current_board->harness_config->GPIO_relay_SBU1, current_board->harness_config->pin_relay_SBU1, 0);
      set_gpio_output(current_board->harness_config->GPIO_relay_SBU2, current_board->harness_config->pin_relay_SBU2, 1);
    } else {
      set_gpio_output(current_board->harness_config->GPIO_relay_SBU1, current_board->harness_config->pin_relay_SBU1, 0);
      set_gpio_output(current_board->harness_config->GPIO_relay_SBU2, current_board->harness_config->pin_relay_SBU2, 0);
    }
  }
#else
  UNUSED(orientation);
#endif
}

void set_intercept_relay(bool intercept, bool ignition_relay) {
  UNUSED(intercept);
  UNUSED(ignition_relay);
}

void harness_tick(void) {
  // TODO: implement
}
