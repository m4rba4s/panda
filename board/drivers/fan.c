#include "board/config.h"
#include "board/main_declarations.h"

#if defined(PANDA_JUNGLE)
  #include "board/jungle/boards/board_declarations.h"
#elif defined(PANDA_BODY)
  #include "board/body/boards/board_declarations.h"
#else
  #include "board/boards/board_declarations.h"
#endif

#include "board/utils.h"
#include "board/drivers/fan.h"
#include "board/drivers/pwm.h"

#define FAN_TICK_FREQ 8U

#ifdef STM32H7
#include "board/stm32h7/llfan.h"
#endif

struct fan_state_t fan_state;

void fan_set_power(uint8_t percentage) {
  if (percentage == 0U) {
    fan_state.power = 0U;
  } else {
    fan_state.power = CLAMP(percentage, 20U, 100U);
  }
}

void fan_init(void) {
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
  fan_state.cooldown_counter = current_board->fan_enable_cooldown_time * FAN_TICK_FREQ;
#else
  fan_state.cooldown_counter = 0;
#endif
  fan_set_power(0);
}

void fan_tick(void) {
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
  if (current_board->has_fan) {
    // rpm control
    if (fan_state.power > 0U) {
      fan_state.cooldown_counter = current_board->fan_enable_cooldown_time * FAN_TICK_FREQ;
    } else {
      if (fan_state.cooldown_counter > 0U) {
        fan_state.cooldown_counter--;
      }
    }

    // Set PWM and enable line
    pwm_set(TIM3, 3, fan_state.power);
    current_board->set_fan_enabled((fan_state.power > 0U) || (fan_state.cooldown_counter > 0U));
  }
#endif
}
