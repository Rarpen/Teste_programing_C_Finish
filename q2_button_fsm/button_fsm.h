#ifndef BUTTON_FSM_H
#define BUTTON_FSM_H
#include <stdint.h>
typedef enum {
    BTN_STATE_OFF = 0,
    BTN_STATE_ON,
    BTN_STATE_PROTECTED
} button_state_t;
typedef enum {
    BTN_RELEASED = 0,
    BTN_PRESSED  = 1
} button_level_t;

typedef struct {
    button_state_t state;
    uint32_t protected_ms;     
    uint32_t press_start_ms;   
    uint32_t last_press_ms;              */
    uint32_t protected_start;  
} button_fsm_t;

void button_fsm_init(button_fsm_t *fsm, uint32_t protected_ms);

button_state_t button_fsm_update(button_fsm_t *fsm,
                                 button_level_t level,
                                 uint32_t now_ms);
                                 
uint32_t button_fsm_last_press_duration(const button_fsm_t *fsm);

const char *button_state_name(button_state_t s);

#endif
