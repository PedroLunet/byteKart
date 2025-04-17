#include <lcom/lcf.h>
#include "mouse.h"

int hook_id_mouse = 3;

struct packet pp;

uint8_t scanbyte;
uint32_t count_mouse_packets = 0;
uint8_t packet[3];

int (mouse_subscribe_int)(uint8_t *bit_no) {
    if (bit_no == NULL) return 1;
    *bit_no = BIT(hook_id_mouse);
    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse) != 0) {
        return 1;
    }
    return 0;
}

int (mouse_unsubscribe_int)() {
    if (sys_irqrmpolicy(&hook_id_mouse) != 0) {
        return 1;
    }
    return 0;
}

void (mouse_ih)() {
    if (kbc_read_output(KBC_OUT_CMD, &scanbyte, 1) != 0) {
        printf("Error in reading byte from mouse\n");
    }
}

void (mouse_process_scanbyte)() {
    switch (count_mouse_packets % 3) {
        case 0:
            if (scanbyte & FIRST_BYTE) {
                packet[0] = scanbyte;
                count_mouse_packets++;
            }
            break;
        case 1:
            packet[1] = scanbyte;
            count_mouse_packets++;
            break;
        case 2:
            packet[2] = scanbyte;
            count_mouse_packets++;
            lcf_print_packet(packet);
            break;
        default:
            break;
    }
}

int (mouse_write_command)(uint8_t command) {

    uint8_t attempts = MAX_ATTEMPS;
    uint8_t response;

    do {
        attempts--;

        // Write 0xD4 to the KBC command byte
        if (kbc_write_command(KBC_IN_CMD, MOUSE_WRITE_CMD) != 0) {
            return 1;
        }

        // Write 0xF5 to the KBC command byte
        if (kbc_write_command(KBC_OUT_CMD, command) != 0) {
            return 1;
        }

        tickdelay(micros_to_ticks(DELAY));

        // Read acknowledgement byte on the output buffer
        if (util_sys_inb(KBC_OUT_CMD, &response) != 0) {
            return 1;
        }
        if (response == ACK) {
            return 0;
        }

    } while (response != NACK && attempts);

    return 1;
}


void (lcf_print_packet)(uint8_t *packet) {

    pp.bytes[0] = packet[0];
    pp.bytes[1] = packet[1];
    pp.bytes[2] = packet[2];
    pp.lb = packet[0] & MOUSE_LB;
    pp.rb = packet[0] & MOUSE_RB;
    pp.mb = packet[0] & MOUSE_MB;
    pp.delta_x = packet[1];
    pp.delta_y = packet[2];
    pp.x_ov = packet[0] & MOUSE_X_OVERFLOW;
    pp.y_ov = packet[0] & MOUSE_Y_OVERFLOW;

    if (packet[0] & MOUSE_X_SIGNAL) {
        pp.delta_x |= 0xFF00;
    }
    if (packet[0] & MOUSE_Y_SIGNAL) {
        pp.delta_y |= 0xFF00;
    }

    # ifdef LAB4
        mouse_print_packet(&pp);
    # endif
}
