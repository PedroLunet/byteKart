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
extern uint32_t counter;
extern State state;

int (mouse_test_packet)(uint32_t cnt) {

    int ipc_status;
    message msg;
    uint8_t mouse_mask;

    int ret = mouse_subscribe_int(&mouse_mask);
    if (ret != 0) {
        printf("Error subscribing mouse in main.");
        return 1;
    } 
    
    ret = mouse_write_command(MOUSE_ENABLE);
    if (ret != 0) {
        printf("Error enabling mouse.\n");
        return 1;
    }

    while (cnt) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
          printf("Error in driver_receive\n");
          return 1;
        }
  
        if (is_ipc_notify(ipc_status)) {
          switch (_ENDPOINT_P(msg.m_source)) {
            case HARDWARE:
              if (msg.m_notify.interrupts & BIT(mouse_mask)) {
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

    ret = mouse_write_command(MOUSE_DISABLE);
    if (ret != 0) {
        printf("Error disabling mouse.\n");
        return 1;
    }

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

    uint8_t mouse_mask = 0;
    uint8_t timer_mask = 0;

    uint16_t timer_frequency = sys_hz();

    // Subscribe mouse interrupts
    int ret = mouse_subscribe_int(&mouse_mask);
    if (ret != 0) {
        printf("Error subscribing mouse in main.\n");
        return 1;
    } 

    // Subscribe timer interrupts
    ret = timer_subscribe_int(&timer_mask);
    if (ret != 0) {
        printf("Error subscribing timer in main.\n");
        return 1;
    } 

    // Enable data reporting
    if (mouse_write_command(MOUSE_ENABLE) != 0) {
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
                    if (msg.m_notify.interrupts & BIT(mouse_mask)) {
                        mouse_ih();
                        mouse_bytes();
                        if (index_packet == 3) {
                            mouse_struct_packet(&pp);
                            mouse_print_packet(&pp);
                            index_packet = 0;
                        }
                        seconds = 0;
                        counter = 0;
                    }
                    if (msg.m_notify.interrupts & BIT(timer_mask)) {
                        timer_int_handler();
                        if (counter % timer_frequency == 0) {
                            seconds++;
                        }
                    }
                break;
                default:
                    break;
            }
        }
    }

    // Disable data reporting
    ret = mouse_write_command(MOUSE_DISABLE);
    if (ret != 0) {
        printf("Error disabling mouse.\n");
        return 1;
    }

     // Unsubscribe timer interrupts
    ret = timer_unsubscribe_int();
    if (ret != 0) {
        printf("Error unsubscribing timer in main.");
        return 1;
    }

    // Unsubscribe mouse interrupts
    ret = mouse_unsubscribe_int();
    if (ret != 0) {
        printf("Error unsubscribing mouse in main.");
        return 1;
    }

    return 0;
}


int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

   	int ipc_status;
    message msg;

    bool gesture_done = true;

    uint8_t mouse_mask = 0;
  	// Subscribe mouse interrupts
      int ret = mouse_subscribe_int(&mouse_mask);
      if (ret != 0) {
          printf("Error subscribing mouse in main.\n");
          return 1;
      } 
  
      // Enable data reporting
      if (mouse_write_command(MOUSE_ENABLE) != 0) {
          return 1;
      }

    while (gesture_done) { 
    	if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("Error in driver_receive\n");
            continue;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & BIT(mouse_mask)) {
                        mouse_ih();
                        mouse_bytes();
                        if (index_packet == 3) {
                            mouse_struct_packet(&pp);
                            mouse_print_packet(&pp);
                            state_machine(x_len, tolerance);

                            if (state == END) {
                                gesture_done = false;
                            }
                            index_packet = 0;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // Disable data reporting
    ret = mouse_write_command(MOUSE_DISABLE);
    if (ret != 0) {
        printf("Error disabling mouse.\n");
        return 1;
    }

    // Unsubscribe mouse interrupts
    ret = mouse_unsubscribe_int();
    if (ret != 0) {
        printf("Error unsubscribing mouse in main.");
        return 1;
    }

    return 0;
}

