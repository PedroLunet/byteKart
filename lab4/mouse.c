#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "mouse.h"

uint8_t current_byte;
int hook_id_mouse = 3;
struct packet pp;
uint8_t index_packet = 0;

int (mouse_subscribe_int)(uint8_t *bit_no) {
  if (bit_no == NULL) {
    printf("Null pointer.");
    return 1;
  }
  *bit_no = (uint8_t) hook_id_mouse;

  int ret = sys_irqsetpolicy(IRQ_MOUSE, (IRQ_REENABLE | IRQ_EXCLUSIVE), &hook_id_mouse);
  if (ret != 0) {
    printf("Error subscribing mouse.");
    return 1;
  }
  return 0;
}

int (mouse_unsubscribe_int)() {
  int ret = sys_irqrmpolicy(&hook_id_mouse);
  if (ret != 0) {
    printf("Error unsubscribing mouse.");
    return 1;
  }
  return 0;
}

void (mouse_ih)() {
  if (read_output_KBC(WRITE_COMMAND_BYTE, &current_byte, 1)) {
    printf("Error reading byte from mouse.");
  }
}

void (mouse_bytes)() {
  if (index_packet == 0 && (current_byte & BIT(3))) {
    pp.bytes[index_packet] = current_byte;
    index_packet++;
  } else if (index_packet > 0) {
    pp.bytes[index_packet] = current_byte;
    index_packet++;
  }
}

int (mouse_write_command)(uint8_t mouse_command) {

  int ret = write_command_KBC(KBC_CMD_REG, MOUSE_COMMAND); // request command 0xD4 to 0x64
  if (ret != 0) {
    printf("Error requesting command 0xD4.\n");
    return 1;
  }

  ret = write_command_KBC(OUT_BUFFER, mouse_command); // write command to 0x60
  if (ret != 0) {
    printf("Error writing mouse_command to 0x60.\n");
    return 1;
  }

  uint8_t ackowledgement_byte;
  ret = read_output_KBC(OUT_BUFFER, &ackowledgement_byte, 1);
  if (ret != 0) {
    printf("Ackowledgement byte not successful.\n");
    return 1;
  }

  if (ackowledgement_byte != 0xFA) {
    return mouse_write_command(mouse_command);
  }

  return 0;
}

void (mouse_struct_packet)(struct packet* pp) {
  pp->y_ov = (pp->bytes[0] & MOUSE_Y_OVFL);
  pp->x_ov = (pp->bytes[0] & MOUSE_X_OVFL);
  pp->rb = (pp->bytes[0] & MOUSE_RIGHT_BUTTON);
  pp->lb = (pp->bytes[0] & MOUSE_LEFT_BUTTON);
  pp->mb = (pp->bytes[0] & MOUSE_MIDDLE_BUTTON);
  pp->delta_x = (pp->bytes[0] & MSB_X_DELTA) ? (0xFF00 | ((uint16_t) pp->bytes[1])) : ((uint16_t) pp->bytes[1]);
  pp->delta_y = (pp->bytes[0] & MSB_Y_DELTA) ? (0xFF00 | ((uint16_t) pp->bytes[2])) : ((uint16_t) pp->bytes[2]);
}
