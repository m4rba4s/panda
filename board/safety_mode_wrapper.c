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
#include "board/drivers/can_common.h"
#include "board/drivers/fdcan.h"
#include "board/can_comms.h"
#include "board/drivers/uart.h"
#include "board/libc.h"
#include "board/sys/faults.h"
#include "board/safety_mode_wrapper.h"
#include "opendbc/safety/declarations.h"

bool is_car_safety_mode(uint16_t mode) {
  return (mode != SAFETY_SILENT) &&
         (mode != SAFETY_NOOUTPUT) &&
         (mode != SAFETY_ALLOUTPUT) &&
         (mode != SAFETY_ELM327);
}

void set_safety_mode(uint16_t mode, uint32_t param) {
  uint16_t mode_copy = mode;
  int err = set_safety_hooks(mode_copy, param);
  if (err == -1) {
    print("Error: safety set mode failed. Falling back to SILENT\n");
    mode_copy = SAFETY_SILENT;
    err = set_safety_hooks(mode_copy, 0U);
    assert_fatal(err == 0, "Error: Failed setting SILENT mode. Hanging\n");
  }

  switch (mode_copy) {
    case SAFETY_SILENT:
      set_intercept_relay(false, false);
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
      current_board->set_can_mode(CAN_MODE_NORMAL);
#endif
      can_silent = true;
      break;
    case SAFETY_NOOUTPUT:
      set_intercept_relay(false, false);
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
      current_board->set_can_mode(CAN_MODE_NORMAL);
#endif
      can_silent = true;
      break;
    case SAFETY_ALLOUTPUT:
      set_intercept_relay(false, false);
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
      current_board->set_can_mode(CAN_MODE_NORMAL);
#endif
      can_silent = false;
      break;
    default:
      set_intercept_relay(true, false);
      heartbeat_counter = 0U;
      heartbeat_lost = false;
#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
      current_board->set_can_mode(CAN_MODE_NORMAL);
#endif
      can_silent = false;
      break;
  }
  can_init_all();
}

void set_safety_param(uint32_t param) {
  (void)set_safety_hooks(current_safety_mode, param);
}
