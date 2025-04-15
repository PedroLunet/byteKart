// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include "mouse.h"
#include "../lab3/keyboard.h"
#include "lcom/timer.h"
#include "i8254.h"

#include <stdint.h>
#include <stdio.h>

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


int (mouse_test_packet)(uint32_t cnt) {

    int ipc_status;
    message msg;
    extern uint32_t count_mouse_packets;

    uint8_t bitno_mouse = 0;

    int irq_set_mouse = BIT(bitno_mouse);

    // Enable stream mode using LCF function
    // mouse_enable_data_reporting();

    // Enable data reporting
    if (send_mouse_command(MOUSE_ENABLE_CMD) != 0) {
        printf("Error in send_mouse_command\n");
        return 1;
    }

    // Subscribe mouse interrupts
    if (mouse_subscribe_int(&bitno_mouse) != 0) {
        printf("Error in mouse_subscribe_int\n");
        return 1;
    }

    while (count_mouse_packets / 3 < cnt) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("Error in driver_receive\n");
            return 1;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set_mouse) {
                        mouse_ih();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // Unsubscribe mouse interrupts
    if (mouse_unsubscribe_int() != 0) {
        printf("Error in mouse_unsubscribe_int\n");
        return 1;
    }

    // Disable data reporting
    if (send_mouse_command(MOUSE_DISABLE_CMD) != 0) {
        printf("Error in send_mouse_command\n");
        return 1;
    }

    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {

    int ipc_status;
    message msg;
    extern uint32_t count_mouse_packets;
    extern int counter;

    uint8_t bitno_mouse = 1;
    uint8_t bitno_timer = 0;

    int irq_set_mouse = BIT(bitno_mouse);
    int irq_set_timer = BIT(bitno_timer);

    // Enable data reporting
    if (send_mouse_command(MOUSE_ENABLE_CMD) != 0) {
        printf("Error in send_mouse_command\n");
        return 1;
    }

    // Subscribe timer interrupts
    if (timer_subscribe_int(&bitno_timer) != 0) {
        printf("Error in timer_subscribe_int\n");
        return 1;
    }

    // Subscribe mouse interrupts
    if (mouse_subscribe_int(&bitno_mouse) != 0) {
        printf("Error in mouse_subscribe_int\n");
        return 1;
    }

    while (counter / 60 < idle_time) {
        if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("Error in driver_receive\n");
            return 1;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & irq_set_mouse) {
                        mouse_ih();
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

    // Unsubscribe mouse interrupts
    if (mouse_unsubscribe_int() != 0) {
        printf("Error in mouse_unsubscribe_int\n");
        return 1;
    }

    // Unsubscribe timer interrupts
    if (timer_unsubscribe_int() != 0) {
        printf("Error in timer_unsubscribe_int\n");
        return 1;
    }

    // Disable data reporting
    if (send_mouse_command(MOUSE_DISABLE_CMD) != 0) {
        printf("Error in send_mouse_command\n");
        return 1;
    }

    return 0;

}

typedef enum mouse_states {
  	IDLE,
  	DRAWING_FIRST_LINE,
  	WAITING_FOR_SECOND_LINE,
    DRAWING_SECOND_LINE,
	GESTURE_COMPLETE
};

typedef enum mouse_events {
  	LEFT_BUTTON,
  	RIGHT_BUTTON,
  	MOVE_UP,
  	MOVE_DOWN,
  	MOVE_LEFT,
  	MOVE_RIGHT
};

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {

   	int ipc_status;
    message msg;
    uint8_t bitno_mouse = 1;
    extern uint8_t packet[3];

  	// Enable data reporting
    if (send_mouse_command(MOUSE_ENABLE_CMD) != 0) {
        printf("Error in send_mouse_command\n");
        return 1;
    }

    // Subscribe mouse interrupts
    if (mouse_subscribe_int(&bitno_mouse) != 0) {
        printf("Error in mouse_subscribe_int\n");
        return 1;
    }

    // Set up state machine
    enum mouse_states state = IDLE;

    while (state != GESTURE_COMPLETE) {
    	if (driver_receive(ANY, &msg, &ipc_status) != 0) {
            printf("Error in driver_receive\n");
            return 1;
        }

        if (is_ipc_notify(ipc_status)) {
            switch (_ENDPOINT_P(msg.m_source)) {
                case HARDWARE:
                    if (msg.m_notify.interrupts & bitno_mouse) {
                        mouse_ih();
                        switch() {
                            case IDLE:
                                if (packet[0] & BIT(3)) {
                                    state = DRAWING_FIRST_LINE;
                                }
                                break;
                            case DRAWING_FIRST_LINE:
                                if (abs(packet[1]) > x_len) {
                                    state = WAITING_FOR_SECOND_LINE;
                                }
                                break;
                            case WAITING_FOR_SECOND_LINE:
                                if (abs(packet[2]) > tolerance) {
                                    state = DRAWING_SECOND_LINE;
                                }
                                break;
                            case DRAWING_SECOND_LINE:
                                if (abs(packet[1]) > x_len) {
                                    state = GESTURE_COMPLETE;
                                }
                                break;
                            default:
                                break;
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
        printf("Error in mouse_unsubscribe_int\n");
        return 1;
    }

    // Disable data reporting
    if (send_mouse_command(MOUSE_DISABLE_CMD) != 0) {
        printf("Error in send_mouse_command\n");
        return 1;
    }

    return 0;

}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    /* This year you need not implement this. */
    printf("%s(%u, %u): under construction\n", __func__, period, cnt);
    return 1;
}
