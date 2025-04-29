#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "video_card.h"
#include "i8042.h"
#include "i8254.h"
#include "kbc.h"
#include "sprite.h"
#include "menu.h"
#include "macros.h"


extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;

Sprite *struct_left_line_tile;
Sprite *struct_right_line_tile;
Sprite *struct_outspace_tile;
Sprite *struct_track_tile;
Sprite *struct_track_line_tile;


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

int (ESC_key_wait)() {
    int ipc_status;
    message msg;
    uint8_t irq_set_keyboard;

    if (kbc_subscribe_int(&irq_set_keyboard) != 0) {
        return 1;
    }
    
    while (scancode != ESC_BREAKCODE) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("error");
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(irq_set_keyboard)) {
                        kbc_ih();
                        unsigned char size;
                        if (scancode == 0xE0) { // check if key is pressed or released
                            size = 2;
                        } else {
                            size = 1;
                        }
                        kbd_print_scancode(!(scancode & BIT(7)), size, &scancode);
                    }
                break;
                default:
                    break;
            }
        }
    }

    if (kbc_unsubscribe_int() != 0) {
        return 1;
    }
    
    return 0;
}

int (initial_setup)() {

    if (start_VBE_mode(0x115) != 0) {
        return 1;
    }

    if (change_VBE_mode(0x115) != 0) {
        return 1;
    }

    return 0;
}

int (restore_system)() {
    if (vg_exit() != 0) {
        return 1;
    }
    return 0;
}

int (exit_program)() {
    printf("Exiting the program...\n");

    if (restore_system() != 0) {
        printf("Error restoring system state.\n");
        return 1;
    }

    return 0;
}

int (proj_main_loop)(int argc, char *argv[]) {

    if (initial_setup() != 0) {
        return 1;
    }
    printf("Screen resolution: %dx%d\n", vbe_mode_info.XResolution, vbe_mode_info.YResolution);


    int selected_option = navigate_main_menu();
    switch (selected_option) {
        case 0:
            printf("Play selected.\n");
            // start game
            break;
        case 1:
            printf("Leaderboard selected.\n");
            // leaderboard
            break;
        case 2:
            printf("Quit selected.\n");
            exit_program();
            break;
        default:
            printf("Menu exited without selection.\n");
    }

    if (restore_system() != 0) {
        printf("Error restoring system state.\n");
        return 1;
    }

    return 0;
}



