// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>

#include "mouse.h"

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");

    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need/ it]
    // lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    // lcf_log_output("/home/lcom/labs/lab4/output.txt");

    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
        return 1;

    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();

    return 0;
}

extern struct packet pp;
extern uint8_t index_packet;

int (mouse_test_packet)(uint32_t cnt) {

    //int ipc_status, r;
    //message msg;
    uint8_t mouse_mask;

    int ret = mouse_subscribe_int(&mouse_mask);
    if (ret != 0) {
        printf("Error subscribing mouse in main.");
        return 1;
    }

    // enable data report 

    int ipc_status;
    message msg;

    while (cnt) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
          printf("Error in driver_receive\n");
          return 1;
        }
  
        if (is_ipc_notify(ipc_status)) {
          switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE:
              if (msg.m_notify.interrupts & mouse_mask) {
                mouse_ih();
                mouse_bytes();
                if (index_packet == 3) {
                  mouse_struct_packet(&pp);
                  mouse_print_packet(&pp);
                  index_packet = 0;
                  cnt--;
                }
              }
              break;
            default:
              break;
          }
        }
    }

    // disable data report 

    ret = mouse_unsubscribe_int();
    if (ret != 0) {
        printf("Error unsubscribing mouse in main.");
        return 1;
    }

    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {

    int ipc_status;
    message msg;
    uint8_t seconds = 0;

    uint8_t irq_set_mouse = 0;
    uint8_t irq_set_timer = 0;

    uint16_t timer_frequency = sys_hz();

    // Enable data reporting
    if (mouse_write_command(MOUSE_ENABLE_CMD) != 0) {
        return 1;
    }

    // Subscribe timer interrupts
    if (timer_subscribe_int(&irq_set_timer) != 0) {
        return 1;
    }

    // Subscribe mouse interrupts
    if (mouse_subscribe_int(&irq_set_mouse) != 0) {
        return 1;
    }

    while (seconds < idle_time) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("Error in driver_receive\n");
            continue;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set_mouse) {
                        mouse_ih();
                        mouse_process_scanbyte();
                        seconds = 0;
                        timer_counter = 0;
                    }
                    if (msg.m_notify.interrupts & irq_set_timer) {
                        timer_int_handler();
                        if (timer_counter % timer_frequency == 0) {
                            seconds++;
                        }
                    }
                break;
                default:
                    break;
            }
        }
    }

    // Unsubscribe mouse interrupts
    if (mouse_unsubscribe_int() != 0) {
        return 1;
    }

    // Unsubscribe timer interrupts
    if (timer_unsubscribe_int() != 0) {
        return 1;
    }

    // Disable data reporting
    if (mouse_write_command(MOUSE_DISABLE_CMD) != 0) {
        return 1;
    }

    return 0;

}

void (update_state_machine)(uint8_t x_len, uint8_t tolerance) {
    switch (state) {
        case START:
            if (pp.lb && !pp.mb && !pp.rb) {
                state = UP;
                x_len_total = 0;
            }
            break;

        case UP:
            if (!pp.lb && !pp.mb && !pp.rb) {
                state = VERTEX;
            } else if ((pp.delta_x >= -tolerance && abs(pp.delta_y) > abs(pp.delta_x) - tolerance) ||
                       (abs(pp.delta_y) <= tolerance)) {
                x_len_total += pp.delta_x;
            } else {
               state = START;
            }
            break;

        case VERTEX:
            if (pp.rb && !pp.lb && !pp.mb) {
                state = DOWN;
            } else if (abs(pp.delta_x) > tolerance || abs(pp.delta_y) > tolerance) {
                state = START;
            }
        break;

        case DOWN:
            if (!pp.rb && !pp.mb && !pp.lb && x_len_total >= x_len) {
                state = END;
            } else if ((pp.delta_x >= -tolerance && abs(pp.delta_y) > abs(pp.delta_x) - tolerance) ||
                       (abs(pp.delta_y) <= tolerance)) {
                x_len_total += pp.delta_x;
            } else {
               state = START;
            }
        break;

        case END:
            break;

        default:
            break;
    }


    x_len_total = max(0, x_len_total + pp.delta_x);
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

   	int ipc_status;
    message msg;
    uint8_t irq_set_mouse;

  	// Enable data reporting
    if (mouse_write_command(MOUSE_ENABLE_CMD) != 0) {
        return 1;
    }

    // Subscribe mouse interrupts
    if (mouse_subscribe_int(&irq_set_mouse) != 0) {
        return 1;
    }


    while (state != END) {
    	if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("Error in driver_receive\n");
            continue;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set_mouse) {
                        mouse_ih();
                        mouse_process_scanbyte();
                        if (count_mouse_packets % 3 == 0) {
                            update_state_machine(x_len, tolerance);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // Unsubscribe mouse interrupts
    if (mouse_unsubscribe_int() != 0) {
        return 1;
    }

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
    /* To be completed */
    printf("%s: under construction\n", __func__);
    return 1;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    /* This year you need not implement this. */
    printf("%s(%u, %u): under construction\n", __func__, period, cnt);
    return 1;
}

