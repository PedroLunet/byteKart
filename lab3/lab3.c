#include <lcom/lcf.h>
#include <lcom/lab3.h>

#include <stdbool.h>
#include <stdint.h>

#include "i8254.h"
#include "i8042.h"
#include "keyboard.h"
#include "kbc.c"
#include "../lab2/timer.c"

extern int timer_counter;
extern int syslib_count_lab3;
extern uint8_t scancode;

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    // lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    // lcf_log_output("/home/lcom/labs/lab3/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
        return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}

int(kbd_test_scan)() {

    int ipc_status;
    message msg;
    uint8_t irq_set;

    if (keyboard_subscribe_int(&irq_set) != 0) {
        return 1;
    }

    while (scancode != ESC_BREAKCODE) {

      if (driver_receive(ANY, &msg, &ipc_status) != 0) {
        printf("Error in driver_receive\n");
        return 1;
      }

      if (is_ipc_notify(ipc_status)) {
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE:
            if (msg.m_notify.interrupts & irq_set) {
              kbc_ih();
              keyboard_process_scancode();
            }
            break;
          default:
            break;
        }
      }
    }

    if (keyboard_unsubscribe_int() != 0) {
        return 1;
    }

    if (kbd_print_no_sysinb(syslib_count_lab3) != 0) {
      return 1;
    }

    return 0;
}

int(kbd_test_poll)() {

    while(scancode != ESC_BREAKCODE) {
      	if (kbc_read_output(KBC_OUT_CMD,&scancode, 0) == 0) {
      	    keyboard_process_scancode();
        }
    }

    return keyboard_restore();
}

int(kbd_test_timed_scan)(uint8_t n) {

    int ipc_status;
    message msg;
    uint8_t irq_set_timer, irq_set_keyboard;

    if (keyboard_subscribe_int(&irq_set_keyboard) != 0) {
        return 1;
    }
    if (timer_subscribe_int(&irq_set_timer) != 0) {
        return 1;
    }

    int seconds = 0;

    while (scancode != ESC_BREAKCODE && seconds < n) {

      if (driver_receive(ANY, &msg, &ipc_status) != 0) {
        printf("Error in driver_receive\n");
        continue;
      }

      if (is_ipc_notify(ipc_status)) {
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE:
            if (msg.m_notify.interrupts & irq_set_keyboard) {
              kbc_ih();
              keyboard_process_scancode();
              seconds = 0;
              timer_counter = 0;
            }
            if (msg.m_notify.interrupts & irq_set_timer) {
              timer_int_handler();
              if (timer_counter % 60 == 0) {
                seconds++;
              }
            }
            break;
          default:
            break;
        }
      }
    }

    if (keyboard_unsubscribe_int() != 0) {
        return 1;
    }

    if (timer_unsubscribe_int() != 0) {
        return 1;
    }

    if (kbd_print_no_sysinb(syslib_count_lab3) != 0) {
        return 1;
    }

    return 0;
}
