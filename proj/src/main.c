#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "controller/video_card.h"
#include "controller/i8042.h"
#include "controller/i8254.h"
#include "controller/kbc.h"
#include "sprite.h"
#include "menu.h"
#include "macros.h"

extern vbe_mode_info_t vbe_mode_info;
extern uint8_t scancode;
extern int timer_counter;

enum MainState {
  MENU,
  PLAY,
  GAMEOVER,
};
static enum MainState current_stat;
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



    current_stat = MENU;
    running = true;

    return 0;
}

int (restore_system)() {

  // unsubscribe timer interrupts
  if (timer_unsubscribe_int)() != 0) {
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

int (proj_main_loop)(int argc, char *argv[]) {

    if (initial_setup() != 0) {
        return 1;
    }
    printf("Screen resolution: %dx%d\n", vbe_mode_info.XResolution, vbe_mode_info.YResolution);

    int ipc_status;
    message msg;

    while (running) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("error");
            continue;
        }
        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:

                    if (msg.m_notify.interrupts & BIT(irq_set_timer)) {
                        timer_int_handler();
                    }

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

    /*
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
        restore_system();
    }
    */

    if (restore_system() != 0) {
        printf("Error restoring system state.\n");
        return 1;
    }

    return 0;
}



