#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "board/main_declarations.h"

// hardware type
uint8_t hw_type = 0;
const board *current_board = NULL;

// uptime
uint32_t uptime_cnt = 0;

// bootloader mode
uint32_t enter_bootloader_mode = 0;

// heartbeat state
uint32_t heartbeat_counter = 0;
bool heartbeat_lost = false;
bool heartbeat_disabled = false;
