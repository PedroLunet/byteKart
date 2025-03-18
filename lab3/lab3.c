#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include "keyboard.h"
#include "lcom/timer.h"

#include "i8254.h"

#include <stdbool.h>
#include <stdint.h>

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
    uint8_t bitno = 1;
    extern uint8_t scancode;

    extern int count;

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

    kbd_print_no_sysinb(count);

    return 0;
}

int(kbd_test_poll)() {

    extern int count;
    extern uint8_t scancode;

    while(scancode != ESC_BREAKCODE) {
      	scancode = kbc_read_output_buffer();
        kbd_process_scancode();
    }

    if (kbc_write(KBC_READ_CMD, KBC_ST_IBF, KBC_CMD_REG) != 0) {
        printf("Error in kbc_input()\n");
        return 1;
    }

  	uint8_t command_byte = kbc_read_output_buffer();

    if (kbc_write(KBC_WRITE_CMD, KBC_ST_IBF, KBC_CMD_REG) != 0) {
        printf("Error in kbc_input()\n");
        return 1;
    }

    if (kbc_write((command_byte | KBC_ST_CBYTE), KBC_ST_OBF, KBC_IN_BUF) != 0) {
        printf("Error in kbc_input()\n");
        return 1;
    }

    kbd_print_no_sysinb(count);

    return 1;
}

int(kbd_test_timed_scan)(uint8_t n) {

  	extern int counter;
    int ipc_status;
    message msg;

    uint8_t bitno_keyboard = 1;
    uint8_t bitno_timer = 0;

    extern uint8_t scancode;

    int irq_set_keyboard = BIT(bitno_keyboard);
    int irq_set_timer = BIT(bitno_timer);

    if (kbc_subscribe_int(&bitno_keyboard) != 0) {
        printf("Error in kbd_subscribe_int\n");
        return 1;
    }
    if (timer_subscribe_int(&bitno_timer) != 0) {
        printf("Error in timer_subscribe_int\n");
        return 1;
    }

    while (scancode != ESC_BREAKCODE && counter / 60 < n) {
      if (driver_receive(ANY, &msg, &ipc_status) != 0) {
        printf("Error in driver_receive\n");
        return 1;
      }

      if (is_ipc_notify(ipc_status)) {
        switch (_ENDPOINT_P(msg.m_source)) {
          case HARDWARE:
            if (msg.m_notify.interrupts & irq_set_keyboard) {
              kbc_ih();
              counter = 0;
            }
            if (msg.m_notify.interrupts & irq_set_timer) {
              timer_int_handler();
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

    if (timer_unsubscribe_int() != 0) {
        printf("Error in timer_unsubscribe_int\n");
        return 1;
    }

    return 0;
}
