#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include "keyboard.h"

#include "i8254.h"

#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/lab3/output.txt");

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
    uint8_t bitno = 1;
    extern uint8_t scancode;

    uint8_t bytes[2];
    uint8_t size = 0;
    bool make = true;

    int irq_set = BIT(bitno);

    if (kbc_subscribe_int(&bitno) != 0) {
        printf("Error in kbd_subscribe_int\n");
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
              if (bytes[0] != 0xE0) {
                bytes[0] = scancode;
                size = 1;
                if (scancode != 0xE0) {
                  if (scancode & BREAK_BIT) {
                    make = false;
                  }
                  kbd_print_scancode(make, size, bytes);
                  make = true;
                }
              } else {
                bytes[1] = scancode;
                size = 2;
                if (scancode & BREAK_BIT) {
                  make = false;
                }
              }
            }
            break;
          default:
            break;
        }
      }
    }

    if (kbc_unsubscribe_int() != 0) {
        printf("Error in kbd_unsubscribe_int\n");
        return 1;
    }

    return 0;
}

int(kbd_test_poll)() {
    /* To be completed by the students */
    printf("%s is not yet implemented!\n", __func__);

    return 1;
}

int(kbd_test_timed_scan)(uint8_t n) {
    /* To be completed by the students */
    printf("%s is not yet implemented!\n", __func__);

    return 1;
}
