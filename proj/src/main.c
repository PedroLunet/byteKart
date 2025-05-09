#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "main.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;
extern uint8_t index_packet;
extern struct packet pp;

Menu *mainMenu = NULL;
SelectDifficulty *selectDifficulty = NULL;

static MainState current_state;
bool running;
Font *gameFont = NULL;

uint8_t irq_set_timer;
uint8_t irq_set_keyboard;
uint8_t irq_set_mouse;


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

    if (mouse_subscribe_int(&irq_set_mouse) != 0) {
        return 1;
    }

    if (mouse_write_command(MOUSE_ENABLE) != 0) {
      return 1;
    }

    // Load font
    gameFont = font_load_xpm((xpm_map_t) font_2_0, "/home/lcom/labs/proj/src/fonts/font_2.fnt");
    if (!gameFont) {
        printf("Error loading font.\n");
        return 1;
    }

    // Initialize the menu
    mainMenu = menu_create();
    if (!mainMenu) {
        menu_destroy(mainMenu);
        return 1;
    }
    menu_draw(mainMenu);

    // Initialize select difficulty
    selectDifficulty = select_difficulty_create();
    if (!selectDifficulty) {
        select_difficulty_destroy(selectDifficulty);
        return 1;
    }

    current_state = MENU;
    running = true;

    return 0;
}

int (restore_system)() {

    // Destroy the font
    if (gameFont) {
        font_destroy(gameFont);
        gameFont = NULL;
    }

    // Destroy the menu object
    if (mainMenu) {
        menu_destroy(mainMenu);
        mainMenu = NULL;
    }

    // Destroy the select difficulty object
    if (selectDifficulty) {
        select_difficulty_destroy(selectDifficulty);
        selectDifficulty = NULL;
    }

    // unsubscribe timer interrupts
    if (timer_unsubscribe_int() != 0) {
      return 1;
    }

    // unsubscribe keyboard interrupts
    if (kbc_unsubscribe_int() != 0) {
    return 1;
    }

    if (mouse_write_command(MOUSE_DISABLE) != 0) {
      return 1;
    }

    // unsubscribe mouse interrupts
    if (mouse_unsubscribe_int() != 0) {
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
                menu_reset_state(mainMenu);
                nextState = SELECT_DIFFICULTY;
            } else if (currentMenuSubstate == MENU_FINISHED_QUIT) {
                nextState = QUIT;
            } else if (currentMenuSubstate == MENU_EXITED) {
                nextState = QUIT;
            }
            break;

        case SELECT_DIFFICULTY:
            select_difficulty_process_event(selectDifficulty, event);
            DifficultyLevel chosenLevel = select_difficulty_get_chosen_level(selectDifficulty);
            if (chosenLevel == DIFFICULTY_EASY || chosenLevel == DIFFICULTY_MEDIUM || chosenLevel == DIFFICULTY_HARD) {
                // game_set_difficulty(game, chosenLevel);
                nextState = SELECT_CAR;
            } else if (chosenLevel == DIFFICULTY_EXITED) {
                nextState = QUIT;
            }
            break;

        case SELECT_CAR:
            // Handle car selection
            break;

        case SELECT_TRACK:
            // Handle track selection
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
                        kbc_ih();
                        pendingEvent = EVENT_KEYBOARD;
                    }

                    if (msg.m_notify.interrupts & BIT(irq_set_mouse)) {
                        mouse_ih();
                        mouse_bytes();
                        if (index_packet == 3) {
                            mouse_struct_packet(&pp);
                            index_packet = 0;
                            pendingEvent = EVENT_MOUSE;
                        }
                    }

                    break;
                default:
                    break;
            }
        }

        if (pendingEvent != EVENT_NONE) {
            // interruptHandlers[pendingEvent]();
            current_state = stateMachineUpdate(current_state, pendingEvent);

            pendingEvent = EVENT_NONE;
        }

        if (swap_buffers() != 0) {
            printf("Error swapping buffers.\n");
            return 1;
        }
    }

    if (restore_system() != 0) {
        printf("Error restoring system state.\n");
        return 1;
    }

    return 0;
}



