#include "board/config.h"
#include "board/main_declarations.h"

#if defined(PANDA_JUNGLE)
  #include "board/jungle/boards/board_declarations.h"
#elif defined(PANDA_BODY)
  #include "board/body/boards/board_declarations.h"
#else
  #include "board/boards/board_declarations.h"
#endif

#include "board/drivers/fake_siren.h"
#include "board/drivers/pwm.h"

bool siren_enabled = false;
uint32_t siren_countdown = 0;

void fake_siren_set(bool enabled) {
  if (enabled) {
    if (!siren_enabled) {
      siren_enabled = true;
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
      current_board->set_amp_enabled(true);
#endif
    }
    siren_countdown = 40; // 5 seconds at 8Hz
  } else {
    if (siren_enabled) {
      siren_enabled = false;
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
      current_board->set_amp_enabled(false);
#endif
    }
    siren_countdown = 0;
  }
}

void fake_siren_tick(void) {
  if (siren_countdown > 0U) {
    siren_countdown--;
    if (siren_countdown == 0U) {
      fake_siren_set(false);
    }
  }
}
