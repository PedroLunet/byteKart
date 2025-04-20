#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "../lab5/graphics.h"
#include "../xpm/tiles.h"
#include "../src/xpm.h"
#include "../lab3/keyboard.h"
#include "lcom/timer.h"
#include "../lab5/vbe.h"
#include "../lab3/kbc.h"

extern vbe_mode_info_t mode_info;
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

    if (keyboard_subscribe_int(&irq_set_keyboard) != 0) {
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
                    if (msg.m_notify.interrupts & irq_set_keyboard)
                        kbc_ih();
                keyboard_process_scancode();
                break;
                default:
                    break;
            }
        }
    }

    if (keyboard_unsubscribe_int() != 0) {
        return 1;
    }
    return 0;
}

int (initial_setup)() {

    if (vg_set_frame_buffer(0x115) != 0) {
        return 1;
    }

    if (vg_set_mode(0x115) != 0) {
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

int (proj_main_loop)(int argc, char *argv[]) {

    if (initial_setup() != 0) {
        return 1;
    }

    if (load_tiles() != 0) {
        restore_system();
        return 1;
    }

    uint32_t drawable_height = (mode_info.YResolution / 64) * 64;
    uint32_t y_offset = (mode_info.YResolution - drawable_height) / 2;

    uint32_t height_drawn = y_offset;
    for (uint32_t i = 0; i < mode_info.YResolution / 64 ; i++) {
        if (draw_horizontal_track(height_drawn) != 0) {
            restore_system();
            return 1;
        }
        height_drawn += 64;
    }
    swap_buffers();

    // cycle through the game loop
    if (ESC_key_wait() != 0) {
        return 1;
    }

    // restore system
    if (restore_system() != 0) {
        return 1;
    }

    return 0;
}

