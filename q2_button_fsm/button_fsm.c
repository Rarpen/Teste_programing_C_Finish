#include "button_fsm.h"

void button_fsm_init(button_fsm_t *fsm, uint32_t protected_ms)
{
    if (fsm == NULL) return;
    fsm->state          = BTN_STATE_OFF;
    fsm->protected_ms   = protected_ms;
    fsm->press_start_ms = 0;
    fsm->last_press_ms  = 0;
    fsm->protected_start = 0;
}

button_state_t button_fsm_update(button_fsm_t *fsm,
                                 button_level_t level,
                                 uint32_t now_ms)
{
    if (fsm == NULL) return BTN_STATE_OFF;

    switch (fsm->state) {
    case BTN_STATE_OFF:
        if (level == BTN_PRESSED) {
            fsm->press_start_ms = now_ms;
            fsm->state = BTN_STATE_ON;
        }
        break;

    case BTN_STATE_ON:
        if (level == BTN_RELEASED) {
            fsm->last_press_ms   = now_ms - fsm->press_start_ms;
            fsm->protected_start = now_ms;
            fsm->state = BTN_STATE_PROTECTED;
        }
        break;

    case BTN_STATE_PROTECTED:
        if ((now_ms - fsm->protected_start) >= fsm->protected_ms) {
            fsm->state = BTN_STATE_OFF;
            if (level == BTN_PRESSED) {
                fsm->press_start_ms = now_ms;
                fsm->state = BTN_STATE_ON;
            }
        }
        break;

    default:
        fsm->state = BTN_STATE_OFF;
        break;
    }

    return fsm->state;
}

uint32_t button_fsm_last_press_duration(const button_fsm_t *fsm)
{
    return (fsm != NULL) ? fsm->last_press_ms : 0;
}

const char *button_state_name(button_state_t s)
{
    switch (s) {
    case BTN_STATE_OFF:       return "OFF";
    case BTN_STATE_ON:        return "ON";
    case BTN_STATE_PROTECTED: return "PROTECTED";
    default:                  return "?";
    }
}
