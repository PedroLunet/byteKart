#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "video_card.h"
#include "i8042.h"
#include "i8254.h"
#include "kbc.h"
#include "xpm.h"
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

    if (draw_main_screen(0) != 0) {
        printf("Error drawing main screen.\n");
        return 1;
    }

    int selected_option = navigate_main_menu();
    if (selected_option == -1) {
        printf("Menu exited without selection.\n");
    } else if (selected_option == 0) {
        printf("Play selected.\n");
        // start the game - por implementar
    } else if (selected_option == 1) {
        printf("Quit selected.\n");
        exit_program();
    }


    /*
    if (load_tiles() != 0) {
        restore_system();
        return 1;
    }

    uint32_t drawable_height = (vbe_mode_info.YResolution / 64) * 64;
    uint32_t y_offset = (vbe_mode_info.YResolution - drawable_height) / 2;

    uint32_t height_drawn = y_offset;
    for (uint32_t i = 0; i < vbe_mode_info.YResolution / 64 ; i++) {
        if (draw_horizontal_track(height_drawn) != 0) {
            restore_system();
            return 1;
        }
        height_drawn += 64;
    }

    if (ESC_key_wait() != 0) {
        return 1;
    }
    */

    if (restore_system() != 0) {
        printf("Error restoring system state.\n");
        return 1;
    }

    return 0;
}



