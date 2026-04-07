#include "board/config.h"
#include "board/main_declarations.h"

#if defined(PANDA_JUNGLE)
  #include "board/jungle/boards/board_declarations.h"
#elif defined(PANDA_BODY)
  #include "board/body/boards/board_declarations.h"
#else
  #include "board/boards/board_declarations.h"
#endif

#include "board/drivers/bootkick.h"
#include "board/drivers/harness.h"
#include "board/drivers/uart.h"

static bool bootkick_reset_triggered = false;

void bootkick_tick(bool ignition, bool recent_heartbeat) {
  static uint16_t bootkick_last_serial_ptr = 0;
  static uint8_t boot_reset_countdown = 0;
  static BootState boot_state = BOOT_BOOTKICK;

#if !defined(PANDA_JUNGLE) && !defined(PANDA_BODY)
  if (current_board->set_bootkick != NULL) {
    if (boot_reset_countdown > 0U) {
      boot_reset_countdown--;
      if (boot_reset_countdown == 0U) {
        boot_state = BOOT_BOOTKICK;
      }
    }

    // if we haven't seen a heartbeat recently, and the ignition is off, reset the SOM
    if (!recent_heartbeat && !ignition) {
      if (boot_state != BOOT_STANDBY) {
        boot_state = BOOT_STANDBY;
        current_board->set_bootkick(BOOT_STANDBY);
      }
    } else {
      if (boot_state != BOOT_BOOTKICK) {
        boot_state = BOOT_BOOTKICK;
        current_board->set_bootkick(BOOT_BOOTKICK);
      }
    }

    // handle manual reset
    if (uart_ring_debug.w_ptr_rx != bootkick_last_serial_ptr) {
      // check for reset command
      uint16_t ptr = bootkick_last_serial_ptr;
      while (ptr != uart_ring_debug.w_ptr_rx) {
        if (uart_ring_debug.elems_rx[ptr] == (uint8_t)'R') {
          boot_state = BOOT_RESET;
          current_board->set_bootkick(BOOT_RESET);
          boot_reset_countdown = 40; // 5 seconds
          bootkick_reset_triggered = true;
        }
        ptr = (ptr + 1U) % uart_ring_debug.rx_fifo_size;
      }
      bootkick_last_serial_ptr = uart_ring_debug.w_ptr_rx;
    }
  }
#else
  UNUSED(ignition);
  UNUSED(recent_heartbeat);
  UNUSED(bootkick_last_serial_ptr);
  UNUSED(boot_reset_countdown);
  UNUSED(boot_state);
  UNUSED(bootkick_reset_triggered);
#endif
}
