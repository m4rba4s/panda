#include "board/config.h"
#include "board/drivers/uart.h"

void debug_ring_callback(uart_ring *ring) {
  char rcv;
  while (get_char(ring, &rcv)) {
    (void)put_char(ring, rcv);
  }
}
