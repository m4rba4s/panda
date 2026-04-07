#ifndef MAIN_DECLARATIONS_H
#define MAIN_DECLARATIONS_H

#include <stdint.h>
#include <stdbool.h>

// Forward declaration to avoid redefinition conflicts between Panda and Jungle
typedef struct board board;

typedef enum {
  BOOT_STANDBY,
  BOOT_BOOTKICK,
  BOOT_RESET,
} BootState;

extern void __initialize_hardware_early(void);
extern void __initialize_hardware(void);

extern uint8_t hw_type;
extern const board *current_board;
extern uint32_t uptime_cnt;
extern uint32_t enter_bootloader_mode;

// heartbeat state
extern uint32_t heartbeat_counter;
extern bool heartbeat_lost;
extern bool heartbeat_disabled;

// siren state
extern bool siren_enabled;
extern uint32_t siren_countdown;

#endif
