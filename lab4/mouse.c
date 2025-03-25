#include <lcom/lcf.h>
#include "mouse.h"
#include "../lab3/keyboard.h"

#include "i8254.h"

int hook_id_mouse = 0;

uint8_t scanbyte;

uint32_t count_mouse_packets = 0;
uint8_t packet[3];

int (mouse_subscribe_int)(uint8_t *bit_no) {
    hook_id_mouse = *bit_no;
    if (sys_irqsetpolicy(MOUSE_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_mouse) != OK) {
        printf("Error in sys_irqsetpolicy()\n");
        return 1;
    }
    return 0;
}

int (mouse_unsubscribe_int)() {
    if (sys_irqrmpolicy(&hook_id_mouse) != OK) {
        printf("Error in sys_irqrmpolicy()\n");
        return 1;
    }
    return 0;
}

void (mouse_ih)() {

    uint16_t data = kbc_read_output_buffer();

    if (data & 0x100) {
        // Data is from the mouse
        scanbyte = data;
        mouse_process_scanbyte();
    }
}

void (mouse_process_scanbyte)() {
    switch (count_mouse_packets % 3) {
        case 0:
            if (scanbyte & BIT(3)) {
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
            # ifdef LAB4
                lcf_print_packet(packet);
            # endif
            break;
        default:
            break;
    }
}

int (send_mouse_command)(uint8_t command) {

    uint8_t response;

    do {
        // Write 0xD4 to the KBC command byte
        if (kbc_write(MOUSE_WRITE_CMD, KBC_ST_IBF, KBC_CMD_REG) != 0) {
            printf("Error in kbc_write\n");
            return 1;
        }

        // Write 0xF5 to the KBC input buffer
        if (kbc_write(command, KBC_ST_IBF, KBC_IN_BUF) != 0) {
            printf("Error in kbc_write\n");
            return 1;
        }

        // Read acknowledgement byte on the output buffer
        response = kbc_read_output_buffer();
        if (response == ACK) {
            return 0;
        } else if (response != NACK) {
            printf("Unexpected response: 0x%x\n", response);
            return 1;
        }

    } while (response == NACK);

    return 1;
}


void (lcf_print_packet)(uint8_t *packet) {

    struct packet pp;

    pp.bytes[0] = packet[0];
    pp.bytes[1] = packet[1];
    pp.bytes[2] = packet[2];
    pp.rb = packet[0] & BIT(1);
    pp.mb = packet[0] & BIT(2);
    pp.lb = packet[0] & BIT(0);
    pp.delta_x = packet[1];
    pp.delta_y = packet[2];
    pp.x_ov = packet[0] & BIT(6);
    pp.y_ov = packet[0] & BIT(7);

    if (packet[0] & BIT(4)) {
        pp.delta_x |= 0xFF00;
    }
    if (packet[0] & BIT(5)) {
        pp.delta_y |= 0xFF00;
    }

    mouse_print_packet(&pp);
}
