#ifndef BOOTSTUB_DECLARATIONS_H
#define BOOTSTUB_DECLARATIONS_H

#include <stdint.h>
#include <stdbool.h>

// Forward declarations
typedef struct board board;
typedef struct uart_ring uart_ring;

void uart_init(uart_ring *q, unsigned int baud);

// ********************* Globals **********************
extern uint8_t hw_type;
extern uint32_t enter_bootloader_mode;
extern const board *current_board;

#endif
