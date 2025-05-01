#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "controller/video_card.h"
#include "controller/i8042.h"
#include "controller/i8254.h"
#include "controller/kbc.h"

#include "main.h"
#include "lcom/timer.h"
#include "sprite.h"
#include "menu.h"
#include "macros.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;

Menu *mainMenu = NULL;

typedef enum {
  MENU,
  PLAY,
  GAMEOVER,
  QUIT
} MainState;

typedef void (*InterruptHandler)();

InterruptHandler interruptHandlers[NUM_EVENTS] = {
    NULL,
    timer_int_handler,
    kbc_ih,
    // handleMouseInterrupt,
    // handleSerialInterrupt,
};

static MainState current_stat;
bool running;

uint8_t irq_set_timer;
uint8_t irq_set_keyboard;


int (main)(int argc, char *argv[]) {
    // Set the language of LCF messages
    lcf_set_language("EN-US");

    // Enable tracing of function calls
    lcf_trace_calls("/home/lcom/labs/Labs/proj/trace.txt");

    // Enable logging of printf output
    lcf_log_output("/home/lcom/labs/Labs/proj/output.txt");

    // Start LCF, which handles command line arguments and invokes the right function
    if (lcf_start(argc, argv))
        return 1;

    // Clean up LCF resources
    lcf_cleanup();

    return 0;
}

int (initial_setup)() {

    if (start_VBE_mode(0x115) != 0) {
        return 1;
    }

    if (change_VBE_mode(0x115) != 0) {
        return 1;
    }

    if (timer_subscribe_int(&irq_set_timer) != 0) {
        return 1;
    }

    if (kbc_subscribe_int(&irq_set_keyboard) != 0) {
        return 1;
    }

    // Initialize the menu
    mainMenu = menu_create();
    if (!mainMenu) {
        return 1;
    }
    menu_draw(mainMenu);

    current_stat = MENU;
    running = true;

    return 0;
}

int (restore_system)() {

    // Destroy the menu object
    if (mainMenu) {
        menu_destroy(mainMenu);
        mainMenu = NULL;
    }

    // unsubscribe timer interrupts
    if (timer_unsubscribe_int() != 0) {
      return 1;
    }

    // unsubscribe keyboard interrupts
    if (kbc_unsubscribe_int() != 0) {
    return 1;
    }

    // restore the original video mode
    if (vg_exit() != 0) {
    return 1;
    }

    return 0;
}

MainState stateMachineUpdate(MainState currentState, EventType event) {
    MainState nextState = currentState;

    switch (currentState) {
        case MENU:
            menu_process_event(mainMenu, event);
            MenuSubstate currentMenuSubstate = menu_get_current_substate(mainMenu);
            if (currentMenuSubstate == MENU_FINISHED_PLAY) {
                nextState = PLAY;
            } else if (currentMenuSubstate == MENU_FINISHED_QUIT) {
                nextState = QUIT;
            } else if (currentMenuSubstate == MENU_EXITED) {
                nextState = QUIT;
            }
            break;

        /*
        case PLAY:
            game.processEvent(event);
            GameSubstate currentGameSubstate = game.getCurrentSubstate();
            if (currentGameSubstate == GAME_FINISHED) {
                nextState = GAMEOVER;
            }
            break;

        case GAMEOVER:
            gameover.processEvent(event);
            GameOverSubstate currentGameOverSubstate = gameover.getCurrentSubstate();
            if (currentGameOverSubstate == GAMEOVER_FINISHED_QUIT) {
                nextState = QUIT;
            }
            break;

         */

        case QUIT:
            running = false;
            break;

        default:
            break;
    }

    return nextState;
}

int (proj_main_loop)(int argc, char *argv[]) {

    if (initial_setup() != 0) {
        return 1;
    }
    printf("Screen resolution: %dx%d\n", vbe_mode_info.XResolution, vbe_mode_info.YResolution);

    int ipc_status;
    message msg;

    EventType pendingEvent = EVENT_NONE;

    while (running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("error");
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:

                    if (msg.m_notify.interrupts & BIT(irq_set_timer)) {
                        // pendingEvent = EVENT_TIMER;
                    }

                    if (msg.m_notify.interrupts & BIT(irq_set_keyboard)) {
                        pendingEvent = EVENT_KEYBOARD;
                    }

                    break;
                default:
                    break;
            }
        }

        if (pendingEvent != EVENT_NONE) {
            interruptHandlers[pendingEvent]();
            current_stat = stateMachineUpdate(current_stat, pendingEvent);

            pendingEvent = EVENT_NONE;
        }
    }

    if (restore_system() != 0) {
        printf("Error restoring system state.\n");
        return 1;
    }

    return 0;
}



