#include "button_fsm.h"
#include <stdio.h>

int main(void)
{
    button_fsm_t fsm;
    button_fsm_init(&fsm, 10000);   

    struct { uint32_t t; button_level_t lvl; } steps[] = {
        {   0, BTN_RELEASED},
        { 100, BTN_PRESSED },  
        {1000, BTN_PRESSED },
        {1600, BTN_RELEASED},   
        {5000, BTN_RELEASED},   
        {5001, BTN_PRESSED },  
        {5500, BTN_RELEASED},
        {11599, BTN_RELEASED},  
        {11600, BTN_RELEASED},  
        {12000, BTN_PRESSED },  
    };

    button_state_t prev = BTN_STATE_OFF;
    printf("t(ms)\tlevel\tstate\n");
    for (size_t i = 0; i < sizeof(steps)/sizeof(steps[0]); ++i) {
        button_state_t st = button_fsm_update(&fsm, steps[i].lvl, steps[i].t);
        printf("%u\t%s\t%s%s\n",
               steps[i].t,
               steps[i].lvl == BTN_PRESSED ? "PRESS" : "REL  ",
               button_state_name(st),
               (st != prev) ? "  <- transicao" : "");
        if (prev == BTN_STATE_ON && st == BTN_STATE_PROTECTED) {
            printf("   >> duracao medida do pressionamento = %u ms\n",
                   button_fsm_last_press_duration(&fsm));
        }
        prev = st;
    }
    return 0;
}
